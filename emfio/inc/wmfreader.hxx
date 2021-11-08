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

#ifndef INCLUDED_EMFIO_INC_WMFREADER_HXX
#define INCLUDED_EMFIO_INC_WMFREADER_HXX

#include "mtftools.hxx"
#include <tools/stream.hxx>

// predefines
struct WmfExternal;

namespace  {
    // MS Windows defines
    enum WMFRecords
    {
        W_META_SETBKCOLOR           = 0x0201,
        W_META_SETBKMODE            = 0x0102,
        W_META_SETMAPMODE           = 0x0103,
        W_META_SETROP2              = 0x0104,
        W_META_SETRELABS            = 0x0105,
        W_META_SETPOLYFILLMODE      = 0x0106,
        W_META_SETSTRETCHBLTMODE    = 0x0107,
        W_META_SETTEXTCHAREXTRA     = 0x0108,
        W_META_SETTEXTCOLOR         = 0x0209,
        W_META_SETTEXTJUSTIFICATION = 0x020A,
        W_META_SETWINDOWORG         = 0x020B,
        W_META_SETWINDOWEXT         = 0x020C,
        W_META_SETVIEWPORTORG       = 0x020D,
        W_META_SETVIEWPORTEXT       = 0x020E,
        W_META_OFFSETWINDOWORG      = 0x020F,
        W_META_SCALEWINDOWEXT       = 0x0410,
        W_META_OFFSETVIEWPORTORG    = 0x0211,
        W_META_SCALEVIEWPORTEXT     = 0x0412,
        W_META_LINETO               = 0x0213,
        W_META_MOVETO               = 0x0214,
        W_META_EXCLUDECLIPRECT      = 0x0415,
        W_META_INTERSECTCLIPRECT    = 0x0416,
        W_META_ARC                  = 0x0817,
        W_META_ELLIPSE              = 0x0418,
        W_META_FLOODFILL            = 0x0419,
        W_META_PIE                  = 0x081A,
        W_META_RECTANGLE            = 0x041B,
        W_META_ROUNDRECT            = 0x061C,
        W_META_PATBLT               = 0x061D,
        W_META_SAVEDC               = 0x001E,
        W_META_SETPIXEL             = 0x041F,
        W_META_OFFSETCLIPRGN        = 0x0220,
        W_META_TEXTOUT              = 0x0521,
        W_META_BITBLT               = 0x0922,
        W_META_STRETCHBLT           = 0x0B23,
        W_META_POLYGON              = 0x0324,
        W_META_POLYLINE             = 0x0325,
        W_META_ESCAPE               = 0x0626,
        W_META_RESTOREDC            = 0x0127,
        W_META_FILLREGION           = 0x0228,
        W_META_FRAMEREGION          = 0x0429,
        W_META_INVERTREGION         = 0x012A,
        W_META_PAINTREGION          = 0x012B,
        W_META_SELECTCLIPREGION     = 0x012C,
        W_META_SELECTOBJECT         = 0x012D,
        W_META_SETTEXTALIGN         = 0x012E,
        W_META_DRAWTEXT             = 0x062F,
        W_META_CHORD                = 0x0830,
        W_META_SETMAPPERFLAGS       = 0x0231,
        W_META_EXTTEXTOUT           = 0x0a32,
        W_META_SETDIBTODEV          = 0x0d33,
        W_META_SELECTPALETTE        = 0x0234,
        W_META_REALIZEPALETTE       = 0x0035,
        W_META_ANIMATEPALETTE       = 0x0436,
        W_META_SETPALENTRIES        = 0x0037,
        W_META_POLYPOLYGON          = 0x0538,
        W_META_RESIZEPALETTE        = 0x0139,
        W_META_DIBBITBLT            = 0x0940,
        W_META_DIBSTRETCHBLT        = 0x0b41,
        W_META_DIBCREATEPATTERNBRUSH = 0x0142,
        W_META_STRETCHDIB           = 0x0f43,
        W_META_EXTFLOODFILL         = 0x0548,
        W_META_RESETDC              = 0x014C,
        W_META_STARTDOC             = 0x014D,
        W_META_STARTPAGE            = 0x004F,
        W_META_ENDPAGE              = 0x0050,
        W_META_ABORTDOC             = 0x0052,
        W_META_ENDDOC               = 0x005E,
        W_META_DELETEOBJECT         = 0x01f0,
        W_META_CREATEPALETTE        = 0x00f7,
        W_META_CREATEBRUSH          = 0x00F8,
        W_META_CREATEPATTERNBRUSH   = 0x01F9,
        W_META_CREATEPENINDIRECT    = 0x02FA,
        W_META_CREATEFONTINDIRECT   = 0x02FB,
        W_META_CREATEBRUSHINDIRECT  = 0x02FC,
        W_META_CREATEBITMAPINDIRECT = 0x02FD,
        W_META_CREATEBITMAP         = 0x06FE,
        W_META_CREATEREGION         = 0x06FF
    };
}

namespace emfio
{

    class WmfReader : public MtfTools
    {
    private:
        sal_uInt16      mnUnitsPerInch;
        sal_uInt32      mnRecSize;

        // embedded EMF data
        std::optional<std::vector<sal_uInt8>> mpEMFStream;

        // total number of comment records containing EMF data
        sal_uInt32      mnEMFRecCount;

        // number of EMF records read
        sal_uInt32      mnEMFRec;

        // total size of embedded EMF data
        sal_uInt32      mnEMFSize;

        sal_uInt32      mnSkipActions;

        // eventually handed over external header
        const WmfExternal* mpExternalHeader;

        bool mbEnableEMFPlus = true;

        // reads header of the WMF-Datei
        bool            ReadHeader();

        // reads parameters of the record with the functionnumber nFunction.
        void            ReadRecordParams(sal_uInt32 nRecordSize, sal_uInt16 nFunction);

        Point           ReadPoint();                // reads and converts a point (first X then Y)
        Point           ReadYX();                   // reads and converts a point (first Y then X)
        tools::Rectangle       ReadRectangle();            // reads and converts a rectangle
        Size            ReadYXExt();
        void            GetPlaceableBound(tools::Rectangle& rSize, SvStream* pStrm);

    public:
        WmfReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile, const WmfExternal* pExternalHeader);

        // read WMF file from stream and fill the GDIMetaFile
        void ReadWMF();

        // Allows disabling EMF+ if EMF is embedded in this WMF.
        void SetEnableEMFPlus(bool bEnableEMFPlus) { mbEnableEMFPlus = bEnableEMFPlus; }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
