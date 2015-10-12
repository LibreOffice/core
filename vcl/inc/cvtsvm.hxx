/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_CVTSVM_HXX
#define INCLUDED_VCL_INC_CVTSVM_HXX

#include <stack>
#include <vcl/dllapi.h>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>

#define CONVERT_TO_SVM1                 0x00000001UL
#define CONVERT_FROM_SVM1               0x00000002UL

#define GDI_PIXEL_ACTION                1
#define GDI_POINT_ACTION                2
#define GDI_LINE_ACTION                 3
#define GDI_RECT_ACTION                 4
#define GDI_ELLIPSE_ACTION              5
#define GDI_ARC_ACTION                  6
#define GDI_PIE_ACTION                  7
#define GDI_INVERTRECT_ACTION           8
#define GDI_HIGHLIGHTRECT_ACTION        9
#define GDI_POLYLINE_ACTION             10
#define GDI_POLYGON_ACTION              11
#define GDI_POLYPOLYGON_ACTION          12
#define GDI_TEXT_ACTION                 13
#define GDI_TEXTARRAY_ACTION            14
#define GDI_STRETCHTEXT_ACTION          15
#define GDI_BITMAP_ACTION               17
#define GDI_BITMAPSCALE_ACTION          18
#define GDI_PEN_ACTION                  19
#define GDI_FONT_ACTION                 20
#define GDI_FILLBRUSH_ACTION            22
#define GDI_MAPMODE_ACTION              23
#define GDI_CLIPREGION_ACTION           24
#define GDI_RASTEROP_ACTION             25
#define GDI_PUSH_ACTION                 26
#define GDI_POP_ACTION                  27
#define GDI_MOVECLIPREGION_ACTION       28
#define GDI_ISECTCLIPREGION_ACTION      29
#define GDI_BITMAPSCALEPART_ACTION      32
#define GDI_GRADIENT_ACTION             33

#define GDI_TRANSPARENT_COMMENT         1024
#define GDI_HATCH_COMMENT               1025
#define GDI_REFPOINT_COMMENT            1026
#define GDI_TEXTLINECOLOR_COMMENT       1027
#define GDI_TEXTLINE_COMMENT            1028
#define GDI_FLOATTRANSPARENT_COMMENT    1029
#define GDI_GRADIENTEX_COMMENT          1030
#define GDI_COMMENT_COMMENT             1031
#define GDI_UNICODE_COMMENT             1032

#define GDI_LINEJOIN_ACTION             1033
#define GDI_EXTENDEDPOLYGON_ACTION      1034
#define GDI_LINEDASHDOT_ACTION          1035

#define GDI_LINECAP_ACTION              1036

class VCL_PLUGIN_PUBLIC SVMConverter
{
private:
    SAL_DLLPRIVATE void             ImplConvertFromSVM1( SvStream& rIStm, GDIMetaFile& rMtf );
    SAL_DLLPRIVATE void             ImplConvertToSVM1( SvStream& rOStm, GDIMetaFile& rMtf );
    SAL_DLLPRIVATE sal_uLong            ImplWriteActions( SvStream& rOStm, GDIMetaFile& rMtf,
                                          VirtualDevice& rSaveVDev, bool& rRop_0_1,
                                          Color& rLineCol, ::std::stack<Color*>& rLineColStack,
                                          rtl_TextEncoding& rActualCharSet );

public:
                        SVMConverter( SvStream& rIStm, GDIMetaFile& rMtf, sal_uLong nConvertMode  );
                        ~SVMConverter() {}

private:
                        SVMConverter( const SVMConverter& ) = delete;
    SVMConverter&       operator=( const SVMConverter& ) = delete;
};

#endif // INCLUDED_VCL_INC_CVTSVM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
