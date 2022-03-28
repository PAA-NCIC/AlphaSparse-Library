#include <alphasparse/opt.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "alphasparse/format.h"
#include "alphasparse/util.h"

alphasparse_status_t ONAME(const ALPHA_SPMAT_CSR *source, ALPHA_SPMAT_COO **dest) {
  ALPHA_SPMAT_COO *mat = alpha_malloc(sizeof(ALPHA_SPMAT_COO));
  *dest = mat;
  ALPHA_INT m = source->rows;
  ALPHA_INT n = source->cols;
  ALPHA_INT nnz = source->rows_end[m - 1];
  ALPHA_INT num_threads = alpha_get_thread_num();
  mat->rows = m;
  mat->cols = n;
  ALPHA_INT *rows_indx = alpha_memalign((uint64_t)(nnz) * sizeof(ALPHA_INT), DEFAULT_ALIGNMENT);
  ALPHA_INT *cols_indx = alpha_memalign((uint64_t)(nnz) * sizeof(ALPHA_INT), DEFAULT_ALIGNMENT);
  mat->values = alpha_memalign((uint64_t)nnz * sizeof(ALPHA_Number), DEFAULT_ALIGNMENT);
  mat->row_indx = rows_indx;
  mat->col_indx = cols_indx;
#ifdef _OPENMP
#pragma omp parallel for num_threads(num_threads)
#endif
  for (ALPHA_INT r = 0; r < m; r++) {
    for (ALPHA_INT ai = source->rows_start[r]; ai < source->rows_end[r]; ai++) {
      rows_indx[ai] = r;
    }
  }

  // ALPHA_INT cur_row = 0;
  // for (ALPHA_INT ai = 0; ai < nnz; ai++)
  // {
  //     while (source->rows_end[cur_row] == source->rows_start[cur_row] && cur_row < m)
  //     {
  //         cur_row++;
  //     }
  //     cur_row += source->rows_start[cur_row] < ai ? 1 : 0;
  //     rows_indx[ai] = cur_row;
  //     // if (source->rows_start[cur_row] > ai)
  //     //     r++;
  // }
  memcpy(cols_indx, source->col_indx, (uint64_t)sizeof(ALPHA_INT) * nnz);
  memcpy(mat->values, source->values, (uint64_t)sizeof(ALPHA_Number) * nnz);
  mat->nnz = nnz;
#ifdef __DCU__
  mat->ordered = source->ordered;
  coo_order(mat);
#endif
  mat->d_rows_indx = NULL;
  mat->d_cols_indx = NULL;
  mat->d_values    = NULL;

  return ALPHA_SPARSE_STATUS_SUCCESS;
}