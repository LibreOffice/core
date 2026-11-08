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

#include <sal/config.h>

#include <string_view>

#include "dxfreprd.hxx"
#include "dxfmtext.hxx"
#include <osl/nlsupport.h>
#include <unotools/defaultencoding.hxx>
#include <unotools/wincodepage.hxx>

//------------------DXFPalette------------------------------------------------


DXFPalette::DXFPalette()
{
    short i,j,nHue,nNSat,nVal,nC[3],nmax,nmed,nmin;
    sal_uInt8 nV;

    // colors 0 - 9 (normal colors)
    SetColor(0, 0x00, 0x00, 0x00); // actually never being used
    SetColor(1, 0xff, 0x00, 0x00);
    SetColor(2, 0xff, 0xff, 0x00);
    SetColor(3, 0x00, 0xff, 0x00);
    SetColor(4, 0x00, 0xff, 0xff);
    SetColor(5, 0x00, 0x00, 0xff);
    SetColor(6, 0xff, 0x00, 0xff);
    SetColor(7, 0x0f, 0x0f, 0x0f); // actually white???
    SetColor(8, 0x80, 0x80, 0x80);
    SetColor(9, 0xc0, 0xc0, 0xc0);

    // colors 10 - 249
    // (Universal-Palette: 24 hues * 5 lightnesses * 2 saturations )
    i=10;
    for (nHue=0; nHue<24; nHue++) {
        for (nVal=5; nVal>=1; nVal--) {
            for (nNSat=0; nNSat<2; nNSat++) {
                nmax=((nHue+3)>>3)%3;
                j=nHue-(nmax<<3); if (j>4) j=j-24;
                if (j>=0) {
                    nmed=(nmax+1)%3;
                    nmin=(nmax+2)%3;
                }
                else {
                    nmed=(nmax+2)%3;
                    nmin=(nmax+1)%3;
                    j=-j;
                }
                nC[nmin]=0;
                nC[nmed]=255*j/4;
                nC[nmax]=255;
                if (nNSat!=0) {
                    for (j=0; j<3; j++) nC[j]=(nC[j]>>1)+128;
                }
                for (j=0; j<3; j++) nC[j]=nC[j]*nVal/5;
                SetColor(static_cast<sal_uInt8>(i++),static_cast<sal_uInt8>(nC[0]),static_cast<sal_uInt8>(nC[1]),static_cast<sal_uInt8>(nC[2]));
            }
        }
    }

    // Farben 250 - 255 (shades of gray)
    for (i=0; i<6; i++) {
        nV=static_cast<sal_uInt8>(i*38+65);
        SetColor(static_cast<sal_uInt8>(250+i),nV,nV,nV);
    }
}


void DXFPalette::SetColor(sal_uInt8 nIndex, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
{
    pRed[nIndex]=nRed;
    pGreen[nIndex]=nGreen;
    pBlue[nIndex]=nBlue;
}


//------------------DXFRepresentation-----------------------------------------


DXFRepresentation::DXFRepresentation()
    : mEnc(RTL_TEXTENCODING_DONTKNOW)
    , mfGlobalLineTypeScale(1.0)
    , mbLineWeightDisplay(false)
    , mfDimArrowSize(0.18)
    , mfDimScale(1.0)
    , mnPointMode(0)
    , mfPointSize(0.0)
{
}

DXFRepresentation::~DXFRepresentation()
{
}

rtl_TextEncoding DXFRepresentation::getTextEncoding() const
{
    return (isTextEncodingSet()) ?
        mEnc :
        osl_getTextEncodingFromLocale(nullptr); // Use default encoding if none specified
}

bool DXFRepresentation::Read( SvStream & rIStream )
{
    bool bRes;

    aTables.Clear();
    aBlocks.Clear();
    aEntities.Clear();

    DXFGroupReader DGR( rIStream );

    DGR.Read();
    while (DGR.GetG()!=0 || (DGR.GetS() != "EOF")) {
        if (DGR.GetG()==0 && DGR.GetS() == "SECTION") {
            if (DGR.Read()!=2) {
                DGR.SetError();
                break;
            }
            if      (DGR.GetS() == "HEADER")   ReadHeader(DGR);
            else if (DGR.GetS() == "TABLES")   aTables.Read(DGR);
            else if (DGR.GetS() == "BLOCKS")   aBlocks.Read(DGR);
            else if (DGR.GetS() == "ENTITIES") aEntities.Read(DGR);
            else DGR.Read();
        }
        else DGR.Read();
    }

    bRes=DGR.GetStatus();

    return bRes;
}

void DXFRepresentation::ReadHeader(DXFGroupReader & rDGR)
{
    while (rDGR.GetG()!=0 || (rDGR.GetS() != "EOF" && rDGR.GetS() != "ENDSEC") )
    {
        if (rDGR.GetG()==9) {
            if (rDGR.GetS() == "$ACADVER")
            {
                if (!rDGR.Read(1))
                    continue;
                // Versions of AutoCAD up to Release 12 (inclusive, AC1009)
                // were DOS software and used OEM encoding for storing strings.
                // Release 13 (AC1012) had both DOS and Windows variants.
                // Its Windows variant, and later releases used ANSI encodings for
                // strings (up to version 2006, which was the last one to do so).
                // Later versions (2007+, AC1021+) use UTF-8 for that.
                // Other (non-Autodesk) implementations may have used different
                // encodings for storing to corresponding formats, but there's
                // no way to know that.
                // See http://autodesk.blogs.com/between_the_lines/autocad-release-history.html
                if ((rDGR.GetS() <= std::string_view("AC1009")) || (rDGR.GetS() == "AC2.22") || (rDGR.GetS() == "AC2.21") || (rDGR.GetS() == "AC2.10") ||
                    (rDGR.GetS() == "AC1.50") || (rDGR.GetS() == "AC1.40") || (rDGR.GetS() == "AC1.2")  || (rDGR.GetS() == "MC0.0"))
                {
                    // Set OEM encoding for old DOS formats
                    // only if the encoding is not set yet
                    // e.g. by previous $DWGCODEPAGE
                    if (!isTextEncodingSet())
                        setTextEncoding(utl_getWinTextEncodingFromLangStr(
                            utl_getLocaleForGlobalDefaultEncoding(), true));
                }
                else if (rDGR.GetS() >= std::string_view("AC1021"))
                    setTextEncoding(RTL_TEXTENCODING_UTF8);
                else
                {
                    // Set ANSI encoding for old Windows formats
                    // only if the encoding is not set yet
                    // e.g. by previous $DWGCODEPAGE
                    if (!isTextEncodingSet())
                        setTextEncoding(utl_getWinTextEncodingFromLangStr(
                            utl_getLocaleForGlobalDefaultEncoding()));
                }
            }
            else if (rDGR.GetS() == "$DWGCODEPAGE")
            {
                if (!rDGR.Read(3))
                    continue;

                // If we already use UTF8, then don't update encoding anymore
                if (mEnc == RTL_TEXTENCODING_UTF8)
                    continue;
                // FIXME: we really need a whole table of
                // $DWGCODEPAGE to encodings mappings
                else if ( (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_932")) ||
                          (rDGR.GetS().equalsIgnoreAsciiCase("DOS932")) )
                {
                    setTextEncoding(RTL_TEXTENCODING_MS_932);
                }
                else if (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_936"))
                {
                    setTextEncoding(RTL_TEXTENCODING_MS_936);
                }
                else if (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_949"))
                {
                    setTextEncoding(RTL_TEXTENCODING_MS_949);
                }
                else if (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_950"))
                {
                    setTextEncoding(RTL_TEXTENCODING_MS_950);
                }
                else if (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_1251"))
                {
                    setTextEncoding(RTL_TEXTENCODING_MS_1251);
                }
            }
            else if (rDGR.GetS() == "$LTSCALE")
            {
                if (!rDGR.Read(40))
                    continue;
                setGlobalLineTypeScale(getGlobalLineTypeScale() * rDGR.GetF());
            }
            else if (rDGR.GetS() == "$DIMASZ")
            {
                if (!rDGR.Read(40))
                    continue;
                mfDimArrowSize = rDGR.GetF();
            }
            else if (rDGR.GetS() == "$DIMSCALE")
            {
                if (!rDGR.Read(40))
                    continue;
                mfDimScale = rDGR.GetF();
            }
            else if (rDGR.GetS() == "$PDMODE")
            {
                if (!rDGR.Read(70))
                    continue;
                mnPointMode = rDGR.GetI();
            }
            else if (rDGR.GetS() == "$PDSIZE")
            {
                if (!rDGR.Read(40))
                    continue;
                mfPointSize = rDGR.GetF();
            }
            else if (rDGR.GetS() == "$LWDISPLAY")
            {
                if (!rDGR.Read(290))
                    continue;
                mbLineWeightDisplay = rDGR.GetI() != 0;
            }
            else rDGR.Read();
        }
        else rDGR.Read();
    }
}

OUString DXFRepresentation::ToOUString(std::string_view s, bool bEscapes) const
{
    const OUString aText = OStringToOUString(s, getTextEncoding(),
                                             RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                                           | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                                           | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR);
    return dxf::decodeSpecialCharacters(aText, getTextEncoding(), bEscapes);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
