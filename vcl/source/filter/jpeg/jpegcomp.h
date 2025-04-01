/*
 * jpegcomp.h
 *
 * Copyright (C) 2010, D. R. Commander
 * For conditions of distribution and use, see the accompanying README file.
 *
 * JPEG compatibility macros
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */

#if JPEG_LIB_VERSION >= 70
 min_DCT_h_scaled_size
 min_DCT_v_scaled_size
#else
 min_DCT_scaled_size
 min_DCT_scaled_size
#endif
