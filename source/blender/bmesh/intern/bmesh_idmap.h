#include "BLI_compiler_attrs.h"
#include "BLI_compiler_compat.h"
#include "BLI_sys_types.h"

#include "bmesh.h"

#ifndef WITH_BM_ID_FREELIST
#  define WITH_BM_ID_FREELIST
#endif

//#define DEBUG_BM_IDMAP /* Debug idmap; note: disables mempool deallocation */

#define USE_NEW_IDMAP

#define BM_ID_NONE 0  //-1

#ifdef __cplusplus
#  include "BLI_map.hh"
#  include "BLI_vector.hh"
#endif

typedef struct BMIdMap {
  int flag;

  uint maxid;
  int cd_id_off[15];
  BMesh *bm;

  BMElem **map;
  int map_size;

#ifdef __cplusplus
  blender::Vector<int> freelist;

  using FreeIdxMap = blender::Map<int, int>;

  /* maps ids to their position within the freelist
       only used if freelist is bigger then a certain size,
       see FREELIST_HASHMAP_THRESHOLD_HIGH in bmesh_construct.c.*/
  FreeIdxMap *free_idx_map;
#endif

#ifdef DEBUG_BM_IDMAP
#  ifdef __cplusplus
  blender::Map<BMElem *, int> *elem2id;
  blender::Map<int, BMElem *> *id2elem;
#  else
  void elem_idmap;
#  endif
#endif
} BMIdMap;

#ifdef __cplusplus
extern "C" {
#endif

BMIdMap *BM_idmap_new(BMesh *bm, int elem_mask);
void BM_idmap_check_attributes(BMIdMap *idmap);
void BM_idmap_check_ids(BMIdMap *idmap);
void BM_idmap_destroy(BMIdMap *idmap);

int BM_idmap_alloc(BMIdMap *idmap, BMElem *elem);
void BM_idmap_assign(BMIdMap *idmap, BMElem *elem, int id);
void BM_idmap_release(BMIdMap *idmap, BMElem *elem, bool clear_id);
int BM_idmap_check_assign(BMIdMap *idmap, BMElem *elem);

BLI_INLINE int BM_idmap_get_id(BMIdMap *map, BMElem *elem)
{
  return BM_ELEM_CD_GET_INT(elem, map->cd_id_off[(int)elem->head.htype]);
}

BLI_INLINE BMElem *BM_idmap_lookup(BMIdMap *map, int elem)
{
  return elem >= 0 ? map->map[elem] : NULL;
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
template<typename T> inline void BM_idmap_on_elem_moved(BMIdMap *map, T *old_elem, T *new_elem)
#else
BLI_INLINE void BM_idmap_on_elem_moved(BMIdMap *map, BMElem *old_elem, BMElem *new_elem)
#endif
{
  int id = BM_ELEM_CD_GET_INT(new_elem, map->cd_id_off[(int)new_elem->head.htype]);
  if (id < 0 || id >= map->map_size || map->map[id] != reinterpret_cast<BMElem *>(old_elem)) {
    //printf("%s: possible idmap corruption\n", __func__);
  }

  if (id >= 0 && id < map->map_size) {
    map->map[id] = reinterpret_cast<BMElem *>(new_elem);
  }
}
