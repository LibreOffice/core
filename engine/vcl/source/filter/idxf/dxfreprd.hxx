/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include "dxfblkrd.hxx"
#include "dxftblrd.hxx"
#include <array>
#include <string_view>

//--------------------Other stuff---------------------------------------------


//-------------------A 3D-Min/Max-Box-----------------------------------------

//-------------------The (constant) palette for DXF-------------------------

class DXFPalette {

public:

    DXFPalette();

    sal_uInt8 GetRed(sal_uInt8 nIndex) const;
    sal_uInt8 GetGreen(sal_uInt8 nIndex) const;
    sal_uInt8 GetBlue(sal_uInt8 nIndex) const;

private:
    std::array<sal_uInt8, 256> pRed;
    std::array<sal_uInt8, 256> pGreen;
    std::array<sal_uInt8, 256> pBlue;
    void SetColor(sal_uInt8 nIndex, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue);
};


//-----------------read and represent DXF file--------------------------------


class DXFRepresentation {

public:

    DXFPalette aPalette;
        // The always equal DXF color palette

    DXFTables aTables;
        // the tables of the DXF file

    DXFBlocks aBlocks;
        // the blocks of the DXF file

    DXFEntities aEntities;
        // the entities (from the Entities-Section) of the DXF file

    rtl_TextEncoding mEnc;  // $DWGCODEPAGE

    double mfGlobalLineTypeScale; // $LTSCALE
    bool mbLineWeightDisplay; // $LWDISPLAY

    DXFRepresentation();
    ~DXFRepresentation();

    rtl_TextEncoding getTextEncoding() const;
    void setTextEncoding(rtl_TextEncoding aEnc) { mEnc = aEnc; }
    // the text in the encoding of the drawing with its special characters decoded; without
    // bEscapes the \U+ and \M+ escapes are left for the MTEXT parser
    OUString ToOUString(std::string_view s, bool bEscapes = true) const;

    double getGlobalLineTypeScale() const { return mfGlobalLineTypeScale; }
    void setGlobalLineTypeScale(double fGlobalLineTypeScale) { mfGlobalLineTypeScale = fGlobalLineTypeScale; }

    bool Read( SvStream & rIStream );
        // Reads complete DXF file.

private:
    void ReadHeader(DXFGroupReader & rDGR);

    bool isTextEncodingSet() const { return mEnc != RTL_TEXTENCODING_DONTKNOW; }
};


//-------------------inlines--------------------------------------------------


inline sal_uInt8 DXFPalette::GetRed(sal_uInt8 nIndex) const { return pRed[nIndex]; }
inline sal_uInt8 DXFPalette::GetGreen(sal_uInt8 nIndex) const { return pGreen[nIndex]; }
inline sal_uInt8 DXFPalette::GetBlue(sal_uInt8 nIndex) const { return pBlue[nIndex]; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
