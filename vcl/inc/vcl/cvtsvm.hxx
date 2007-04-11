/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cvtsvm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:50:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_CVTMTF_HXX
#define _SV_CVTMTF_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

// -----------
// - Defines -
// -----------

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
#define GDI_ICON_ACTION                 16
#define GDI_BITMAP_ACTION               17
#define GDI_BITMAPSCALE_ACTION          18
#define GDI_PEN_ACTION                  19
#define GDI_FONT_ACTION                 20
#define GDI_BACKBRUSH_ACTION            21
#define GDI_FILLBRUSH_ACTION            22
#define GDI_MAPMODE_ACTION              23
#define GDI_CLIPREGION_ACTION           24
#define GDI_RASTEROP_ACTION             25
#define GDI_PUSH_ACTION                 26
#define GDI_POP_ACTION                  27
#define GDI_MOVECLIPREGION_ACTION       28
#define GDI_ISECTCLIPREGION_ACTION      29
#define GDI_MTF_ACTION                  30
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

// ----------------
// - SVMConverter -
// ----------------

class Stack;

class VCL_DLLPUBLIC SVMConverter
{
private:
//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void             ImplConvertFromSVM1( SvStream& rIStm, GDIMetaFile& rMtf );
    SAL_DLLPRIVATE void             ImplConvertToSVM1( SvStream& rOStm, GDIMetaFile& rMtf );
    SAL_DLLPRIVATE ULONG            ImplWriteActions( SvStream& rOStm, GDIMetaFile& rMtf,
                                          VirtualDevice& rSaveVDev, BOOL& rRop_0_1,
                                          Color& rLineCol, Stack& rLineColStack,
                                          rtl_TextEncoding& rActualCharSet );
//#endif

public:
                        SVMConverter( SvStream& rIStm, GDIMetaFile& rMtf, ULONG nConvertMode  );
                        ~SVMConverter() {}

private:
                        // Not implemented
                        SVMConverter( const SVMConverter& );
    SVMConverter&       operator=( const SVMConverter& );
};

#endif // _SV_CVTMTF_HXX
