/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _SVX_ZOOM_DEF_HXX
#define _SVX_ZOOM_DEF_HXX

#define ZOOMBTN_OPTIMAL     ((sal_uInt16)0x0001)
#define ZOOMBTN_PAGEWIDTH   ((sal_uInt16)0x0002)
#define ZOOMBTN_WHOLEPAGE   ((sal_uInt16)0x0004)

#endif

// Zoom factor for Calc, Writer, Draw and Impress
// We use 2^(1/4) so that 4 steps lead to double / half size
#define ZOOM_FACTOR    1.1892071150027210667175

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
