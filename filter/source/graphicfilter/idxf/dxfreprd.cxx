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


#include <string.h>
#include <dxfreprd.hxx>
#include "osl/nlsupport.h"

//------------------DXFBoundingBox--------------------------------------------


void DXFBoundingBox::Union(const DXFVector & rVector)
{
    if (bEmpty) {
        fMinX=rVector.fx;
        fMinY=rVector.fy;
        fMinZ=rVector.fz;
        fMaxX=rVector.fx;
        fMaxY=rVector.fy;
        fMaxZ=rVector.fz;
        bEmpty=false;
    }
    else {
        if (fMinX>rVector.fx) fMinX=rVector.fx;
        if (fMinY>rVector.fy) fMinY=rVector.fy;
        if (fMinZ>rVector.fz) fMinZ=rVector.fz;
        if (fMaxX<rVector.fx) fMaxX=rVector.fx;
        if (fMaxY<rVector.fy) fMaxY=rVector.fy;
        if (fMaxZ<rVector.fz) fMaxZ=rVector.fz;
    }
}


//------------------DXFPalette------------------------------------------------


DXFPalette::DXFPalette()
{
    short i,j,nHue,nNSat,nVal,nC[3],nmax,nmed,nmin;
    sal_uInt8 nV;

    pRed  =new sal_uInt8[256];
    pGreen=new sal_uInt8[256];
    pBlue =new sal_uInt8[256];

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
                SetColor((sal_uInt8)(i++),(sal_uInt8)nC[0],(sal_uInt8)nC[1],(sal_uInt8)nC[2]);
            }
        }
    }

    // Farben 250 - 255 (shades of gray)
    for (i=0; i<6; i++) {
        nV=(sal_uInt8)(i*38+65);
        SetColor((sal_uInt8)(250+i),nV,nV,nV);
    }
}


DXFPalette::~DXFPalette()
{
    delete[] pBlue;
    delete[] pGreen;
    delete[] pRed;
}


void DXFPalette::SetColor(sal_uInt8 nIndex, sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
{
    pRed[nIndex]=nRed;
    pGreen[nIndex]=nGreen;
    pBlue[nIndex]=nBlue;
}


//------------------DXFRepresentation-----------------------------------------


DXFRepresentation::DXFRepresentation()
    : bUseUTF8(false)
    , mbInCalc(false)
{
    setTextEncoding(osl_getTextEncodingFromLocale(nullptr)); // Use default encoding if none specified
    setGlobalLineTypeScale(1.0);
}

DXFRepresentation::~DXFRepresentation()
{
}


bool DXFRepresentation::Read( SvStream & rIStream, sal_uInt16 /*nMinPercent*/, sal_uInt16 /*nMaxPercent*/)
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

    if (bRes && aBoundingBox.bEmpty)
        CalcBoundingBox(aEntities,aBoundingBox);

    return bRes;
}

void DXFRepresentation::ReadHeader(DXFGroupReader & rDGR)
{
    while (rDGR.GetG()!=0 || (rDGR.GetS() != "EOF" && rDGR.GetS() != "ENDSEC") )
    {
        if (rDGR.GetG()==9) {
            if (rDGR.GetS() == "$EXTMIN" ||
                rDGR.GetS() == "$EXTMAX")
            {
                DXFVector aVector;
                while (rDGR.Read()!=9 && rDGR.GetG()!=0) {
                    switch (rDGR.GetG()) {
                    case 10: aVector.fx = rDGR.GetF(); break;
                    case 20: aVector.fy = rDGR.GetF(); break;
                    case 30: aVector.fz = rDGR.GetF(); break;
                    }
                }
                aBoundingBox.Union(aVector);
            }
            else if (rDGR.GetS() == "$ACADVER")
            {
                if (!rDGR.Read(1))
                    continue;
                if (rDGR.GetS() >= "AC1021")
                    bUseUTF8 = true;
            }
            else if (rDGR.GetS() == "$DWGCODEPAGE")
            {
                if (!rDGR.Read(3))
                    continue;

                // FIXME: we really need a whole table of
                // $DWGCODEPAGE to encodings mappings
                if ( (rDGR.GetS().equalsIgnoreAsciiCase("ANSI_932")) ||
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
            else rDGR.Read();
        }
        else rDGR.Read();
    }
}

void DXFRepresentation::CalcBoundingBox(const DXFEntities & rEntities,
                                        DXFBoundingBox & rBox)
{
    if (mbInCalc)
        return;
    mbInCalc = true;

    DXFBasicEntity * pBE=rEntities.pFirst;
    while (pBE!=nullptr) {
        switch (pBE->eType) {
            case DXF_LINE: {
                const DXFLineEntity * pE = static_cast<const DXFLineEntity*>(pBE);
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                break;
            }
            case DXF_POINT: {
                const DXFPointEntity * pE = static_cast<const DXFPointEntity*>(pBE);
                rBox.Union(pE->aP0);
                break;
            }
            case DXF_CIRCLE: {
                const DXFCircleEntity * pE = static_cast<const DXFCircleEntity*>(pBE);
                DXFVector aP;
                aP=pE->aP0;
                aP.fx-=pE->fRadius;
                aP.fy-=pE->fRadius;
                rBox.Union(aP);
                aP=pE->aP0;
                aP.fx+=pE->fRadius;
                aP.fy+=pE->fRadius;
                rBox.Union(aP);
                break;
            }
            case DXF_ARC: {
                const DXFArcEntity * pE = static_cast<const DXFArcEntity*>(pBE);
                DXFVector aP;
                aP=pE->aP0;
                aP.fx-=pE->fRadius;
                aP.fy-=pE->fRadius;
                rBox.Union(aP);
                aP=pE->aP0;
                aP.fx+=pE->fRadius;
                aP.fy+=pE->fRadius;
                rBox.Union(aP);
                break;
            }
            case DXF_TRACE: {
                const DXFTraceEntity * pE = static_cast<const DXFTraceEntity*>(pBE);
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                rBox.Union(pE->aP2);
                rBox.Union(pE->aP3);
                break;
            }
            case DXF_SOLID: {
                const DXFSolidEntity * pE = static_cast<const DXFSolidEntity*>(pBE);
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                rBox.Union(pE->aP2);
                rBox.Union(pE->aP3);
                break;
            }
            case DXF_TEXT: {
                //const DXFTextEntity * pE = (DXFTextEntity*)pBE;
                //???
                break;
            }
            case DXF_SHAPE: {
                //const DXFShapeEntity * pE = (DXFShapeEntity*)pBE;
                //???
                break;
            }
            case DXF_INSERT: {
                const DXFInsertEntity * pE = static_cast<const DXFInsertEntity*>(pBE);
                DXFBlock * pB;
                DXFBoundingBox aBox;
                DXFVector aP;
                pB=aBlocks.Search(pE->m_sName);
                if (pB==nullptr) break;
                CalcBoundingBox(*pB,aBox);
                if (aBox.bEmpty) break;
                aP.fx=(aBox.fMinX-pB->aBasePoint.fx)*pE->fXScale+pE->aP0.fx;
                aP.fy=(aBox.fMinY-pB->aBasePoint.fy)*pE->fYScale+pE->aP0.fy;
                aP.fz=(aBox.fMinZ-pB->aBasePoint.fz)*pE->fZScale+pE->aP0.fz;
                rBox.Union(aP);
                aP.fx=(aBox.fMaxX-pB->aBasePoint.fx)*pE->fXScale+pE->aP0.fx;
                aP.fy=(aBox.fMaxY-pB->aBasePoint.fy)*pE->fYScale+pE->aP0.fy;
                aP.fz=(aBox.fMaxZ-pB->aBasePoint.fz)*pE->fZScale+pE->aP0.fz;
                rBox.Union(aP);
                break;
            }
            case DXF_ATTDEF: {
                //const DXFAttDefEntity * pE = (DXFAttDefEntity*)pBE;
                //???
                break;
            }
            case DXF_ATTRIB: {
                //const DXFAttribEntity * pE = (DXFAttribEntity*)pBE;
                //???
                break;
            }
            case DXF_VERTEX: {
                const DXFVertexEntity * pE = static_cast<const DXFVertexEntity*>(pBE);
                rBox.Union(pE->aP0);
                break;
            }
            case DXF_3DFACE: {
                const DXF3DFaceEntity * pE = static_cast<const DXF3DFaceEntity*>(pBE);
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                rBox.Union(pE->aP2);
                rBox.Union(pE->aP3);
                break;
            }
            case DXF_DIMENSION: {
                const DXFDimensionEntity * pE = static_cast<const DXFDimensionEntity*>(pBE);
                DXFBlock * pB;
                DXFBoundingBox aBox;
                DXFVector aP;
                pB = aBlocks.Search(pE->m_sPseudoBlock);
                if (pB==nullptr) break;
                CalcBoundingBox(*pB,aBox);
                if (aBox.bEmpty) break;
                aP.fx=aBox.fMinX-pB->aBasePoint.fx;
                aP.fy=aBox.fMinY-pB->aBasePoint.fy;
                aP.fz=aBox.fMinZ-pB->aBasePoint.fz;
                rBox.Union(aP);
                aP.fx=aBox.fMaxX-pB->aBasePoint.fx;
                aP.fy=aBox.fMaxY-pB->aBasePoint.fy;
                aP.fz=aBox.fMaxZ-pB->aBasePoint.fz;
                rBox.Union(aP);
                break;
            }
            case DXF_POLYLINE: {
                //const DXFAttribEntity * pE = (DXFAttribEntity*)pBE;
                //???
                break;
            }
            case DXF_SEQEND: {
                //const DXFAttribEntity * pE = (DXFAttribEntity*)pBE;
                //???
                break;
            }
            case DXF_HATCH :
                break;
            case DXF_LWPOLYLINE :
                break;
        }
        pBE=pBE->pSucc;
    }
    mbInCalc = false;
}

namespace {
    inline bool lcl_isDec(sal_Unicode ch)
    {
        return ch >= L'0' && ch <= L'9';
    }
    inline bool lcl_isHex(sal_Unicode ch)
    {
        return lcl_isDec(ch) || (ch >= L'A' && ch <= L'F') || (ch >= L'a' && ch <= L'f');
    }
}

OUString DXFRepresentation::ToOUString(const OString& s, bool bSpecials) const
{
    OUString result = OStringToOUString(s, getTextEncoding());
    if (bSpecials) {
        result = result.replaceAll("%%o", "")                     // Overscore - simply remove
                       .replaceAll("%%u", "")                     // Underscore - simply remove
                       .replaceAll("%%d", OUString(sal_Unicode(L'\u00B0'))) // Degrees symbol (°)
                       .replaceAll("%%p", OUString(sal_Unicode(L'\u00B1'))) // Tolerance symbol (±)
                       .replaceAll("%%c", OUString(sal_Unicode(L'\u2205'))) // Diameter symbol
                       .replaceAll("%%%", "%");                   // Percent symbol

        sal_Int32 pos = result.indexOf("%%"); // %%nnn, where nnn - 3-digit decimal ASCII code
        while (pos != -1 && pos <= result.getLength() - 5) {
            OUString asciiNum = result.copy(pos + 2, 3);
            if (lcl_isDec(asciiNum[0]) &&
                lcl_isDec(asciiNum[1]) &&
                lcl_isDec(asciiNum[2]))
            {
                char ch = static_cast<char>(asciiNum.toUInt32());
                OUString codePt(&ch, 1, mEnc);
                result = result.replaceAll(result.copy(pos, 5), codePt, pos);
            }
            pos = result.indexOf("%%", pos + 1);
        }

        pos = result.indexOf("\\U+"); // \U+XXXX, where XXXX - 4-digit hex unicode
        while (pos != -1 && pos <= result.getLength() - 7) {
            OUString codePtNum = result.copy(pos + 3, 4);
            if (lcl_isHex(codePtNum[0]) &&
                lcl_isHex(codePtNum[1]) &&
                lcl_isHex(codePtNum[2]) &&
                lcl_isHex(codePtNum[3]))
            {
                OUString codePt(static_cast<sal_Unicode>(codePtNum.toUInt32(16)));
                result = result.replaceAll(result.copy(pos, 7), codePt, pos);
            }
            pos = result.indexOf("\\U+", pos + 1);
        }
    }
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
