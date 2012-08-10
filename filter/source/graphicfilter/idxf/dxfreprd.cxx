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


//------------------DXFBoundingBox--------------------------------------------


void DXFBoundingBox::Union(const DXFVector & rVector)
{
    if (bEmpty==sal_True) {
        fMinX=rVector.fx;
        fMinY=rVector.fy;
        fMinZ=rVector.fz;
        fMaxX=rVector.fx;
        fMaxY=rVector.fy;
        fMaxZ=rVector.fz;
        bEmpty=sal_False;
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
{
    setTextEncoding(RTL_TEXTENCODING_IBM_437);
        setGlobalLineTypeScale(1.0);
}


DXFRepresentation::~DXFRepresentation()
{
}


sal_Bool DXFRepresentation::Read( SvStream & rIStream, sal_uInt16 nMinPercent, sal_uInt16 nMaxPercent)
{
    DXFGroupReader * pDGR;
    sal_Bool bRes;

    aTables.Clear();
    aBlocks.Clear();
    aEntities.Clear();

    pDGR = new DXFGroupReader( rIStream, nMinPercent, nMaxPercent );

    pDGR->Read();
    while (pDGR->GetG()!=0 || strcmp(pDGR->GetS(),"EOF")!=0) {
        if (pDGR->GetG()==0 && strcmp(pDGR->GetS(),"SECTION")==0) {
            if (pDGR->Read()!=2) {
                pDGR->SetError();
                break;
            }
            if      (strcmp(pDGR->GetS(),"HEADER"  )==0) ReadHeader(*pDGR);
            else if (strcmp(pDGR->GetS(),"TABLES"  )==0) aTables.Read(*pDGR);
            else if (strcmp(pDGR->GetS(),"BLOCKS"  )==0) aBlocks.Read(*pDGR);
            else if (strcmp(pDGR->GetS(),"ENTITIES")==0) aEntities.Read(*pDGR);
            else pDGR->Read();
        }
        else pDGR->Read();
    }

    bRes=pDGR->GetStatus();

    delete pDGR;

    if (bRes==sal_True && aBoundingBox.bEmpty==sal_True)
        CalcBoundingBox(aEntities,aBoundingBox);

    return bRes;
}


void DXFRepresentation::ReadHeader(DXFGroupReader & rDGR)
{

         while (rDGR.GetG()!=0 || (strcmp(rDGR.GetS(),"EOF")!=0 &&  strcmp(rDGR.GetS(),"ENDSEC")!=0) )
         {
                 if (rDGR.GetG()==9) {
                         if (strcmp(rDGR.GetS(),"$EXTMIN")==0 ||
                                 strcmp(rDGR.GetS(),"$EXTMAX")==0)
                         {
                                 DXFVector aVector;
                                 rDGR.SetF(10,0.0);
                                 rDGR.SetF(20,0.0);
                                 rDGR.SetF(30,0.0);
                                 do {
                                         rDGR.Read();
                                 } while (rDGR.GetG()!=9 && rDGR.GetG()!=0);
                                 aVector.fx=rDGR.GetF(10);
                                 aVector.fy=rDGR.GetF(20);
                                 aVector.fz=rDGR.GetF(30);
                                 aBoundingBox.Union(aVector);
                         } else {
                                 if (strcmp(rDGR.GetS(),"$DWGCODEPAGE")==0)
                                 {
                                         rDGR.Read();

                                         // FIXME: we really need a whole table of
                                         // $DWGCODEPAGE to encodings mappings
                                         if ( (strcmp(rDGR.GetS(),"ANSI_932")==0) ||
                          (strcmp(rDGR.GetS(),"ansi_932")==0) ||
                                              (strcmp(rDGR.GetS(),"DOS932")==0) ||
                                              (strcmp(rDGR.GetS(),"dos932")==0) )
                                         {
                                                 setTextEncoding(RTL_TEXTENCODING_MS_932);
                                         }
                                 }
                 else if (strcmp(rDGR.GetS(),"$LTSCALE")==0)
                                 {
                                         rDGR.Read();
                                         setGlobalLineTypeScale(getGlobalLineTypeScale() * rDGR.GetF());
                                 }
                                 else rDGR.Read();
                         }
                 }
                 else rDGR.Read();
         }
}


void DXFRepresentation::CalcBoundingBox(const DXFEntities & rEntities,
                                        DXFBoundingBox & rBox)
{
    DXFBasicEntity * pBE=rEntities.pFirst;
    while (pBE!=NULL) {
        switch (pBE->eType) {
            case DXF_LINE: {
                const DXFLineEntity * pE = (DXFLineEntity*)pBE;
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                break;
            }
            case DXF_POINT: {
                const DXFPointEntity * pE = (DXFPointEntity*)pBE;
                rBox.Union(pE->aP0);
                break;
            }
            case DXF_CIRCLE: {
                const DXFCircleEntity * pE = (DXFCircleEntity*)pBE;
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
                const DXFArcEntity * pE = (DXFArcEntity*)pBE;
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
                const DXFTraceEntity * pE = (DXFTraceEntity*)pBE;
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                rBox.Union(pE->aP2);
                rBox.Union(pE->aP3);
                break;
            }
            case DXF_SOLID: {
                const DXFSolidEntity * pE = (DXFSolidEntity*)pBE;
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
                const DXFInsertEntity * pE = (DXFInsertEntity*)pBE;
                DXFBlock * pB;
                DXFBoundingBox aBox;
                DXFVector aP;
                pB=aBlocks.Search(pE->sName);
                if (pB==NULL) break;
                CalcBoundingBox(*pB,aBox);
                if (aBox.bEmpty==sal_True) break;
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
                const DXFVertexEntity * pE = (DXFVertexEntity*)pBE;
                rBox.Union(pE->aP0);
                break;
            }
            case DXF_3DFACE: {
                const DXF3DFaceEntity * pE = (DXF3DFaceEntity*)pBE;
                rBox.Union(pE->aP0);
                rBox.Union(pE->aP1);
                rBox.Union(pE->aP2);
                rBox.Union(pE->aP3);
                break;
            }
            case DXF_DIMENSION: {
                const DXFDimensionEntity * pE = (DXFDimensionEntity*)pBE;
                DXFBlock * pB;
                DXFBoundingBox aBox;
                DXFVector aP;
                pB=aBlocks.Search(pE->sPseudoBlock);
                if (pB==NULL) break;
                CalcBoundingBox(*pB,aBox);
                if (aBox.bEmpty==sal_True) break;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
