/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <string.h>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include "dxf2mtf.hxx"

#include <math.h>


ULONG DXF2GDIMetaFile::CountEntities(const DXFEntities & rEntities)
{
    const DXFBasicEntity * pBE;
    ULONG nRes;

    nRes=0;
    for (pBE=rEntities.pFirst; pBE!=NULL; pBE=pBE->pSucc) nRes++;
    return nRes;
}


void DXF2GDIMetaFile::MayCallback(ULONG /*nMainEntitiesProcessed*/)
{
    // ULONG nPercent;
/*
    if (pCallback!=NULL && nMainEntitiesCount!=0) {
        nPercent=nMinPercent+(nMaxPercent-nMinPercent)*nMainEntitiesProcessed/nMainEntitiesCount;
        if (nPercent>=nLastPercent+4) {
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE) bStatus=FALSE;
            nLastPercent=nPercent;
        }
    }
*/
}

Color DXF2GDIMetaFile::ConvertColor(BYTE nColor)
{
    return Color(
        pDXF->aPalette.GetRed( nColor ),
        pDXF->aPalette.GetGreen( nColor ),
        pDXF->aPalette.GetBlue( nColor ) );
}

long DXF2GDIMetaFile::GetEntityColor(const DXFBasicEntity & rE)
{
    long nColor;
    const DXFLayer * pLayer;

    nColor=rE.nColor;
    if (nColor==256) {
        if (rE.sLayer[0]=='0' && rE.sLayer[1]==0) nColor=nParentLayerColor;
        else {
            pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) nColor=pLayer->nColor;
            else nColor=nParentLayerColor;
        }
    }
    else if (nColor==0) nColor=nBlockColor;
    return nColor;
}

DXFLineInfo DXF2GDIMetaFile::LTypeToDXFLineInfo(const char * sLineType)
{
    const DXFLType * pLT;
    DXFLineInfo aDXFLineInfo;

    pLT=pDXF->aTables.SearchLType(sLineType);
    if (pLT==NULL || pLT->nDashCount == 0) {
        aDXFLineInfo.eStyle = LINE_SOLID;
    }
    else {
        sal_Int32 i;
        double x;
        aDXFLineInfo.eStyle = LINE_DASH;
        for (i=0; i < (pLT->nDashCount); i++) {
            x = pLT->fDash[i] * pDXF->getGlobalLineTypeScale();
// ####
            // x = (sal_Int32) rTransform.TransLineWidth( pLT->fDash[i] * pDXF->getGlobalLineTypeScale() );
            if ( x >= 0.0 ) {
                if ( aDXFLineInfo.nDotCount == 0 ) { 
                    aDXFLineInfo.nDotCount ++;
                    aDXFLineInfo.fDotLen = x;
                }
                else if ( aDXFLineInfo.fDotLen == x ) {
                    aDXFLineInfo.nDotCount ++;
                }
                else if ( aDXFLineInfo.nDashCount == 0 ) {
                    aDXFLineInfo.nDashCount ++;
                    aDXFLineInfo.fDashLen = x;
                }
                else if ( aDXFLineInfo.fDashLen == x ) {
                    aDXFLineInfo.nDashCount ++;
                }
                else {
                    // It is impossible to be converted.
                }
            }
            else {
                if ( aDXFLineInfo.fDistance == 0 ) {
                    aDXFLineInfo.fDistance = -1 * x;
                }
                else {
                    // It is impossible to be converted.
                }
            }

        }
    }

#if 0
    if (aDXFLineInfo.DashCount > 0 && aDXFLineInfo.DashLen == 0.0)
        aDXFLineInfo.DashLen ( 1 );
    if (aDXFLineInfo.DotCount > 0 && aDXFLineInfo.DotLen() == 0.0)
        aDXFLineInfo.SetDotLen( 1 );
    if (aDXFLineInfo.GetDashCount > 0 || aDXFLineInfo.GetDotCount > 0)
        if (aDXFLineInfo.GetDistance() == 0)
            aDXFLineInfo.SetDistance( 1 );
#endif

    return aDXFLineInfo;
}

DXFLineInfo DXF2GDIMetaFile::GetEntityDXFLineInfo(const DXFBasicEntity & rE)
{
    DXFLineInfo aDXFLineInfo;
    const DXFLayer * pLayer;

    aDXFLineInfo.eStyle = LINE_SOLID;
    aDXFLineInfo.fWidth = 0;
    aDXFLineInfo.nDashCount = 0;
    aDXFLineInfo.fDashLen = 0;
    aDXFLineInfo.nDotCount = 0;
    aDXFLineInfo.fDotLen = 0;
    aDXFLineInfo.fDistance = 0;

    if (strcmp(rE.sLineType,"BYLAYER")==0) {
        if (rE.sLayer[0]=='0' && rE.sLayer[1]==0) aDXFLineInfo=aParentLayerDXFLineInfo;
        else {
            pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) aDXFLineInfo=LTypeToDXFLineInfo(pLayer->sLineType);
            else aDXFLineInfo=aParentLayerDXFLineInfo;
        }
    }
    else if (strcmp(rE.sLineType,"BYBLOCK")==0) {
        aDXFLineInfo=aBlockDXFLineInfo;
    }
    else aDXFLineInfo=LTypeToDXFLineInfo(rE.sLineType);
    return aDXFLineInfo;
}


BOOL DXF2GDIMetaFile::SetLineAttribute(const DXFBasicEntity & rE, ULONG /*nWidth*/)
{
    long nColor;
    Color aColor;

    nColor=GetEntityColor(rE);
    if (nColor<0) return FALSE;
    aColor=ConvertColor((BYTE)nColor);

    if (aActLineColor!=aColor) {
        pVirDev->SetLineColor( aActLineColor = aColor );
    }

    if (aActFillColor!=Color( COL_TRANSPARENT )) {
        pVirDev->SetFillColor(aActFillColor = Color( COL_TRANSPARENT ));
    }
    return TRUE;
}


BOOL DXF2GDIMetaFile::SetAreaAttribute(const DXFBasicEntity & rE)
{
    long nColor;
    Color aColor;

    nColor=GetEntityColor(rE);
    if (nColor<0) return FALSE;
    aColor=ConvertColor((BYTE)nColor);

    if (aActLineColor!=aColor) {
        pVirDev->SetLineColor( aActLineColor = aColor );
    }

    if ( aActFillColor == Color( COL_TRANSPARENT ) || aActFillColor != aColor) {
        pVirDev->SetFillColor( aActFillColor = aColor );
    }
    return TRUE;
}


BOOL DXF2GDIMetaFile::SetFontAttribute(const DXFBasicEntity & rE, short nAngle, USHORT nHeight, double /*fWidthScale*/)
{
    long nColor;
    Color aColor;
    Font aFont;

    nAngle=-nAngle;
    while (nAngle>3600) nAngle-=3600;
    while (nAngle<0) nAngle+=3600;

    nColor=GetEntityColor(rE);
    if (nColor<0) return FALSE;
    aColor=ConvertColor((BYTE)nColor);

    aFont.SetColor(aColor);
    aFont.SetTransparent(TRUE);
    aFont.SetFamily(FAMILY_SWISS);
    aFont.SetSize(Size(0,nHeight));
    aFont.SetAlign(ALIGN_BASELINE);
    aFont.SetOrientation(nAngle);
    if (aActFont!=aFont) {
        aActFont=aFont;
        pVirDev->SetFont(aActFont);
    }

    return TRUE;
}


void DXF2GDIMetaFile::DrawLineEntity(const DXFLineEntity & rE, const DXFTransform & rTransform)
{
    if (SetLineAttribute(rE)) {
        Point aP0,aP1;
        rTransform.Transform(rE.aP0,aP0);
        rTransform.Transform(rE.aP1,aP1);

        DXFLineInfo aDXFLineInfo;
        aDXFLineInfo=GetEntityDXFLineInfo(rE);
        LineInfo aLineInfo;
        aLineInfo = rTransform.Transform(aDXFLineInfo);

#if 0
        printf("%f\n", rTransform.TransLineWidth(1000.0));

        // LINE_NONE = 0, LINE_SOLID = 1, LINE_DASH = 2, LineStyle_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
        aLineInfo.SetStyle( LINE_DASH );
        aLineInfo.SetWidth( 300 );
        aLineInfo.SetDashCount( 2 );
        aLineInfo.SetDashLen( 100 );
        aLineInfo.SetDotCount( 1 );
        aLineInfo.SetDotLen( 0 );
        aLineInfo.SetDistance( 500 );
#endif

        pVirDev->DrawLine(aP0,aP1,aLineInfo);
        if (rE.fThickness!=0) {
            Point aP2,aP3;
            rTransform.Transform(rE.aP0+DXFVector(0,0,rE.fThickness),aP2);
            rTransform.Transform(rE.aP1+DXFVector(0,0,rE.fThickness),aP3);
            pVirDev->DrawLine(aP2,aP3);
            pVirDev->DrawLine(aP0,aP2);
            pVirDev->DrawLine(aP1,aP3);
        }
    }
}


void DXF2GDIMetaFile::DrawPointEntity(const DXFPointEntity & rE, const DXFTransform & rTransform)
{

    if (SetLineAttribute(rE)) {
        Point aP0;
        rTransform.Transform(rE.aP0,aP0);
        if (rE.fThickness==0) pVirDev->DrawPixel(aP0);
        else {
            Point aP1;
            rTransform.Transform(rE.aP0+DXFVector(0,0,rE.fThickness),aP1);
            pVirDev->DrawLine(aP0,aP1);
        }
    }
}


void DXF2GDIMetaFile::DrawCircleEntity(const DXFCircleEntity & rE, const DXFTransform & rTransform)
{
    double frx,fry,fAng;
    USHORT nPoints,i;
    DXFVector aC;

    if (SetLineAttribute(rE)==FALSE) return;
    rTransform.Transform(rE.aP0,aC);
    if (rE.fThickness==0 && rTransform.TransCircleToEllipse(rE.fRadius,frx,fry)==TRUE) {
        pVirDev->DrawEllipse(
            Rectangle((long)(aC.fx-frx+0.5),(long)(aC.fy-fry+0.5),
                      (long)(aC.fx+frx+0.5),(long)(aC.fy+fry+0.5)));
    }
    else {
        nPoints=OptPointsPerCircle;
        Polygon aPoly(nPoints);
        for (i=0; i<nPoints; i++) {
            fAng=2*3.14159265359/(double)(nPoints-1)*(double)i;
            rTransform.Transform(
                rE.aP0+DXFVector(rE.fRadius*cos(fAng),rE.fRadius*sin(fAng),0),
                aPoly[i]
            );
        }
        pVirDev->DrawPolyLine(aPoly);
        if (rE.fThickness!=0) {
            Polygon aPoly2(nPoints);
            for (i=0; i<nPoints; i++) {
                fAng=2*3.14159265359/(double)(nPoints-1)*(double)i;
                rTransform.Transform(
                    rE.aP0+DXFVector(rE.fRadius*cos(fAng),rE.fRadius*sin(fAng),rE.fThickness),
                    aPoly2[i]
                );

            }
            pVirDev->DrawPolyLine(aPoly2);
            for (i=0; i<nPoints-1; i++) pVirDev->DrawLine(aPoly[i],aPoly2[i]);
        }
    }
}


void DXF2GDIMetaFile::DrawArcEntity(const DXFArcEntity & rE, const DXFTransform & rTransform)
{
    double frx,fry,fA1,fdA,fAng;
    USHORT nPoints,i;
    DXFVector aC;
    Point aPS,aPE;

    if (SetLineAttribute(rE)==FALSE) return;
    fA1=rE.fStart;
    fdA=rE.fEnd-fA1;
    while (fdA>=360.0) fdA-=360.0;
    while (fdA<=0) fdA+=360.0;
    rTransform.Transform(rE.aP0,aC);
    if (rE.fThickness==0 && fdA>5.0 && rTransform.TransCircleToEllipse(rE.fRadius,frx,fry)==TRUE) {
        DXFVector aVS(cos(fA1/180.0*3.14159265359),sin(fA1/180.0*3.14159265359),0.0);
        aVS*=rE.fRadius;
        aVS+=rE.aP0;
        DXFVector aVE(cos((fA1+fdA)/180.0*3.14159265359),sin((fA1+fdA)/180.0*3.14159265359),0.0);
        aVE*=rE.fRadius;
        aVE+=rE.aP0;
        if (rTransform.Mirror()==TRUE) {
            rTransform.Transform(aVS,aPS);
            rTransform.Transform(aVE,aPE);
        }
        else {
            rTransform.Transform(aVS,aPE);
            rTransform.Transform(aVE,aPS);
        }
        pVirDev->DrawArc(
            Rectangle((long)(aC.fx-frx+0.5),(long)(aC.fy-fry+0.5),
                      (long)(aC.fx+frx+0.5),(long)(aC.fy+fry+0.5)),
            aPS,aPE
        );
    }
    else {
        nPoints=(USHORT)(fdA/360.0*(double)OptPointsPerCircle+0.5);
        if (nPoints<2) nPoints=2;
        Polygon aPoly(nPoints);
        for (i=0; i<nPoints; i++) {
            fAng=3.14159265359/180.0 * ( fA1 + fdA/(double)(nPoints-1)*(double)i );
            rTransform.Transform(
                rE.aP0+DXFVector(rE.fRadius*cos(fAng),rE.fRadius*sin(fAng),0),
                aPoly[i]
            );
        }
        pVirDev->DrawPolyLine(aPoly);
        if (rE.fThickness!=0) {
            Polygon aPoly2(nPoints);
            for (i=0; i<nPoints; i++) {
                fAng=3.14159265359/180.0 * ( fA1 + fdA/(double)(nPoints-1)*(double)i );
                rTransform.Transform(
                    rE.aP0+DXFVector(rE.fRadius*cos(fAng),rE.fRadius*sin(fAng),rE.fThickness),
                    aPoly2[i]
                );
            }
            pVirDev->DrawPolyLine(aPoly2);
            for (i=0; i<nPoints; i++) pVirDev->DrawLine(aPoly[i],aPoly2[i]);
        }
    }
}


void DXF2GDIMetaFile::DrawTraceEntity(const DXFTraceEntity & rE, const DXFTransform & rTransform)
{
    if (SetLineAttribute(rE)) {
        Polygon aPoly(4);
        rTransform.Transform(rE.aP0,aPoly[0]);
        rTransform.Transform(rE.aP1,aPoly[1]);
        rTransform.Transform(rE.aP3,aPoly[2]);
        rTransform.Transform(rE.aP2,aPoly[3]);
        pVirDev->DrawPolygon(aPoly);
        if (rE.fThickness!=0) {
            USHORT i;
            Polygon aPoly2(4);
            DXFVector aVAdd(0,0,rE.fThickness);
            rTransform.Transform(rE.aP0+aVAdd,aPoly2[0]);
            rTransform.Transform(rE.aP1+aVAdd,aPoly2[1]);
            rTransform.Transform(rE.aP3+aVAdd,aPoly2[2]);
            rTransform.Transform(rE.aP2+aVAdd,aPoly2[3]);
            pVirDev->DrawPolygon(aPoly2);
            for (i=0; i<4; i++) pVirDev->DrawLine(aPoly[i],aPoly2[i]);
        }
    }
}


void DXF2GDIMetaFile::DrawSolidEntity(const DXFSolidEntity & rE, const DXFTransform & rTransform)
{
    if (SetAreaAttribute(rE)) {
        USHORT nN;
        if (rE.aP2==rE.aP3) nN=3; else nN=4;
        Polygon aPoly(nN);
        rTransform.Transform(rE.aP0,aPoly[0]);
        rTransform.Transform(rE.aP1,aPoly[1]);
        rTransform.Transform(rE.aP3,aPoly[2]);
        if (nN>3) rTransform.Transform(rE.aP2,aPoly[3]);
        pVirDev->DrawPolygon(aPoly);
        if (rE.fThickness!=0) {
            Polygon aPoly2(nN);
            DXFVector aVAdd(0,0,rE.fThickness);
            rTransform.Transform(rE.aP0+aVAdd,aPoly2[0]);
            rTransform.Transform(rE.aP1+aVAdd,aPoly2[1]);
            rTransform.Transform(rE.aP3+aVAdd,aPoly2[2]);
            if (nN>3) rTransform.Transform(rE.aP2+aVAdd,aPoly2[3]);
            pVirDev->DrawPolygon(aPoly2);
            if (SetLineAttribute(rE)) {
                USHORT i;
                for (i=0; i<nN; i++) pVirDev->DrawLine(aPoly[i],aPoly2[i]);
            }
        }
    }
}


void DXF2GDIMetaFile::DrawTextEntity(const DXFTextEntity & rE, const DXFTransform & rTransform)
{
    DXFVector aV;
    Point aPt;
    double fA;
    USHORT nHeight;
    short nAng;
    ByteString	aStr( rE.sText );
    DXFTransform aT( DXFTransform(rE.fXScale,rE.fHeight,1.0,rE.fRotAngle,rE.aP0), rTransform );
    aT.TransDir(DXFVector(0,1,0),aV);
    nHeight=(USHORT)(aV.Abs()+0.5);
    fA=aT.CalcRotAngle();
    nAng=(short)(fA*10.0+0.5);
    aT.TransDir(DXFVector(1,0,0),aV);
    if ( SetFontAttribute( rE,nAng, nHeight, aV. Abs() ) )
    {
        String aUString( aStr, pDXF->getTextEncoding() );
        aT.Transform( DXFVector( 0, 0, 0 ), aPt );
        pVirDev->DrawText( aPt, aUString );
    }
}


void DXF2GDIMetaFile::DrawInsertEntity(const DXFInsertEntity & rE, const DXFTransform & rTransform)
{
    const DXFBlock * pB;
    pB=pDXF->aBlocks.Search(rE.sName);
    if (pB!=NULL) {
        DXFTransform aDXFTransform1(1.0,1.0,1.0,DXFVector(0.0,0.0,0.0)-pB->aBasePoint);
        DXFTransform aDXFTransform2(rE.fXScale,rE.fYScale,rE.fZScale,rE.fRotAngle,rE.aP0);
        DXFTransform aT(
            DXFTransform( aDXFTransform1, aDXFTransform2 ),
            rTransform
        );
        long nSavedBlockColor, nSavedParentLayerColor;
        DXFLineInfo aSavedBlockDXFLineInfo, aSavedParentLayerDXFLineInfo;
        nSavedBlockColor=nBlockColor;
        nSavedParentLayerColor=nParentLayerColor;
        aSavedBlockDXFLineInfo=aBlockDXFLineInfo;
        aSavedParentLayerDXFLineInfo=aParentLayerDXFLineInfo;
        nBlockColor=GetEntityColor(rE);
        aBlockDXFLineInfo=GetEntityDXFLineInfo(rE);
        if (rE.sLayer[0]!='0' || rE.sLayer[1]!=0) {
            DXFLayer * pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) {
                nParentLayerColor=pLayer->nColor;
                aParentLayerDXFLineInfo=LTypeToDXFLineInfo(pLayer->sLineType);
            }
        }
        DrawEntities(*pB,aT,FALSE);
        aBlockDXFLineInfo=aSavedBlockDXFLineInfo;
        aParentLayerDXFLineInfo=aSavedParentLayerDXFLineInfo;
        nBlockColor=nSavedBlockColor;
        nParentLayerColor=nSavedParentLayerColor;
    }
}


void DXF2GDIMetaFile::DrawAttribEntity(const DXFAttribEntity & rE, const DXFTransform & rTransform)
{
    if ((rE.nAttrFlags&1)==0) {
        DXFVector aV;
        Point aPt;
        double fA;
        USHORT nHeight;
        short nAng;
        ByteString aStr( rE.sText );
        DXFTransform aT( DXFTransform( rE.fXScale, rE.fHeight, 1.0, rE.fRotAngle, rE.aP0 ), rTransform ); 
        aT.TransDir(DXFVector(0,1,0),aV);
        nHeight=(USHORT)(aV.Abs()+0.5);
        fA=aT.CalcRotAngle();
        nAng=(short)(fA*10.0+0.5);
        aT.TransDir(DXFVector(1,0,0),aV);
        if (SetFontAttribute(rE,nAng,nHeight,aV.Abs()))
        {
            String aUString( aStr, pDXF->getTextEncoding() );
            aT.Transform( DXFVector( 0, 0, 0 ), aPt );
            pVirDev->DrawText( aPt, aUString );
        }
    }
}


void DXF2GDIMetaFile::DrawPolyLineEntity(const DXFPolyLineEntity & rE, const DXFTransform & rTransform)
{
    USHORT i,nPolySize;
    double fW;
    const DXFBasicEntity * pBE;

    nPolySize=0;
    pBE=rE.pSucc;
    while (pBE!=NULL && pBE->eType==DXF_VERTEX) {
        nPolySize++;
        pBE=pBE->pSucc;
    }
    if (nPolySize<2) return;
    Polygon aPoly(nPolySize);
    fW=0.0;
    pBE=rE.pSucc;
    for (i=0; i<nPolySize; i++) {
        rTransform.Transform(((DXFVertexEntity*)pBE)->aP0,aPoly[i]);
        if (i+1<nPolySize || (rE.nFlags&1)!=0) {
            if (((DXFVertexEntity*)pBE)->fSWidth>=0.0) fW+=((DXFVertexEntity*)pBE)->fSWidth;
            else fW+=rE.fSWidth;
            if (((DXFVertexEntity*)pBE)->fEWidth>=0.0) fW+=((DXFVertexEntity*)pBE)->fEWidth;
            else fW+=rE.fEWidth;
        }
        pBE=pBE->pSucc;
    }
    fW/=2.0;
    if ((rE.nFlags&1)!=0) fW/=(double)nPolySize;
    else fW/=(double)(nPolySize-1);
    if (SetLineAttribute(rE,rTransform.TransLineWidth(fW))) {
        if ((rE.nFlags&1)!=0) pVirDev->DrawPolygon(aPoly);
        else pVirDev->DrawPolyLine(aPoly);
        if (rE.fThickness!=0) {
            Polygon aPoly2(nPolySize);
            pBE=rE.pSucc;
            for (i=0; i<nPolySize; i++) {
                rTransform.Transform(
                   (((DXFVertexEntity*)pBE)->aP0)+DXFVector(0,0,rE.fThickness),
                   aPoly2[i]
                );
                pBE=pBE->pSucc;
            }
            if ((rE.nFlags&1)!=0) pVirDev->DrawPolygon(aPoly2);
            else pVirDev->DrawPolyLine(aPoly2);
            for (i=0; i<nPolySize; i++) pVirDev->DrawLine(aPoly[i],aPoly2[i]);
        }
    }
}

void DXF2GDIMetaFile::DrawLWPolyLineEntity(const DXFLWPolyLineEntity & rE, const DXFTransform & rTransform )
{
    sal_Int32 i, nPolySize = rE.nCount;
    if ( nPolySize && rE.pP )
    {
        Polygon aPoly( (sal_uInt16)nPolySize);
        for ( i = 0; i < nPolySize; i++ )
        {
            rTransform.Transform( rE.pP[ (sal_uInt16)i ], aPoly[ (sal_uInt16)i ] );
        }
        double fW = rE.fConstantWidth;
        if ( SetLineAttribute( rE, rTransform.TransLineWidth( fW ) ) )
        {
            if ( ( rE.nFlags & 1 ) != 0 )
                pVirDev->DrawPolygon( aPoly );
            else
                pVirDev->DrawPolyLine( aPoly );
                // ####
                //pVirDev->DrawPolyLine( aPoly, aDXFLineInfo );
        }
    }
}

void DXF2GDIMetaFile::DrawHatchEntity(const DXFHatchEntity & rE, const DXFTransform & rTransform )
{
    if ( rE.nBoundaryPathCount )
    {
        SetAreaAttribute( rE );
        sal_Int32 j = 0;
        PolyPolygon aPolyPoly;
        for ( j = 0; j < rE.nBoundaryPathCount; j++ )
        {
            DXFPointArray aPtAry;
            const DXFBoundaryPathData& rPathData = rE.pBoundaryPathData[ j ];
            if ( rPathData.bIsPolyLine )
            {
                sal_Int32 i;
                for( i = 0; i < rPathData.nPointCount; i++ )
                {
                    Point aPt;
                    rTransform.Transform( rPathData.pP[ i ], aPt );
                    aPtAry.push_back( aPt );
                }
            }
            else
            {
                sal_uInt32 i;
                for ( i = 0; i < rPathData.aEdges.size(); i++ )
                {
                    const DXFEdgeType* pEdge = rPathData.aEdges[ i ];
                    switch( pEdge->nEdgeType )
                    {
                        case 1 : 
                        {
                            Point aPt;
                            rTransform.Transform( ((DXFEdgeTypeLine*)pEdge)->aStartPoint, aPt );
                            aPtAry.push_back( aPt );
                            rTransform.Transform( ((DXFEdgeTypeLine*)pEdge)->aEndPoint, aPt );
                            aPtAry.push_back( aPt );
                        }
                        break;
                        case 2 :
                        {
/*
                            double frx,fry,fA1,fdA,fAng;
                            USHORT nPoints,i;
                            DXFVector aC;
                            Point aPS,aPE;
                            fA1=((DXFEdgeTypeCircularArc*)pEdge)->fStartAngle;
                            fdA=((DXFEdgeTypeCircularArc*)pEdge)->fEndAngle - fA1;
                            while ( fdA >= 360.0 )
                                fdA -= 360.0;
                            while ( fdA <= 0 )
                                fdA += 360.0;
                            rTransform.Transform(((DXFEdgeTypeCircularArc*)pEdge)->aCenter, aC);
                            if ( fdA > 5.0 && rTransform.TransCircleToEllipse(((DXFEdgeTypeCircularArc*)pEdge)->fRadius,frx,fry ) == TRUE )
                            {
                                DXFVector aVS(cos(fA1/180.0*3.14159265359),sin(fA1/180.0*3.14159265359),0.0);
                                aVS*=((DXFEdgeTypeCircularArc*)pEdge)->fRadius;
                                aVS+=((DXFEdgeTypeCircularArc*)pEdge)->aCenter;
                                DXFVector aVE(cos((fA1+fdA)/180.0*3.14159265359),sin((fA1+fdA)/180.0*3.14159265359),0.0);
                                aVE*=((DXFEdgeTypeCircularArc*)pEdge)->fRadius;
                                aVE+=((DXFEdgeTypeCircularArc*)pEdge)->aCenter;
                                if ( rTransform.Mirror() == TRUE )
                                {
                                    rTransform.Transform(aVS,aPS);
                                    rTransform.Transform(aVE,aPE);
                                }
                                else
                                {
                                    rTransform.Transform(aVS,aPE);
                                    rTransform.Transform(aVE,aPS);
                                }
                                pVirDev->DrawArc(
                                    Rectangle((long)(aC.fx-frx+0.5),(long)(aC.fy-fry+0.5),
                                              (long)(aC.fx+frx+0.5),(long)(aC.fy+fry+0.5)),
                                    aPS,aPE
                                );
                            }
*/
                        }
                        break;
                        case 3 :
                        case 4 :
                        break;
                    }
                }
            }
            sal_uInt16 i, nSize = (sal_uInt16)aPtAry.size();
            if ( nSize )
            {
                Polygon aPoly( nSize );
                for ( i = 0; i < nSize; i++ )
                    aPoly[ i ] = aPtAry[ i ];
                aPolyPoly.Insert( aPoly, POLYPOLY_APPEND );
            }
        }
        if ( aPolyPoly.Count() )
            pVirDev->DrawPolyPolygon( aPolyPoly );
    }
}

void DXF2GDIMetaFile::Draw3DFaceEntity(const DXF3DFaceEntity & rE, const DXFTransform & rTransform)
{
    USHORT nN,i;
    if (SetLineAttribute(rE)) {
        if (rE.aP2==rE.aP3) nN=3; else nN=4;
        Polygon aPoly(nN);
        rTransform.Transform(rE.aP0,aPoly[0]);
        rTransform.Transform(rE.aP1,aPoly[1]);
        rTransform.Transform(rE.aP2,aPoly[2]);
        if (nN>3) rTransform.Transform(rE.aP3,aPoly[3]);
        if ((rE.nIEFlags&0x0f)==0) pVirDev->DrawPolygon(aPoly);
        else {
            for (i=0; i<nN; i++) {
                if ( (rE.nIEFlags & (1<<i)) == 0 ) {
                    pVirDev->DrawLine(aPoly[i],aPoly[(i+1)%nN]);
                }
            }
        }
    }
}


void DXF2GDIMetaFile::DrawDimensionEntity(const DXFDimensionEntity & rE, const DXFTransform & rTransform)
{
    const DXFBlock * pB;
    pB=pDXF->aBlocks.Search(rE.sPseudoBlock);
    if (pB!=NULL) {
        DXFTransform aT(
            DXFTransform(1.0,1.0,1.0,DXFVector(0.0,0.0,0.0)-pB->aBasePoint),
            rTransform
        );
        long nSavedBlockColor, nSavedParentLayerColor;
        DXFLineInfo aSavedBlockDXFLineInfo, aSavedParentLayerDXFLineInfo;
        nSavedBlockColor=nBlockColor;
        nSavedParentLayerColor=nParentLayerColor;
        aSavedBlockDXFLineInfo=aBlockDXFLineInfo;
        aSavedParentLayerDXFLineInfo=aParentLayerDXFLineInfo;
        nBlockColor=GetEntityColor(rE);
        aBlockDXFLineInfo=GetEntityDXFLineInfo(rE);
        if (rE.sLayer[0]!='0' || rE.sLayer[1]!=0) {
            DXFLayer * pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) {
                nParentLayerColor=pLayer->nColor;
                aParentLayerDXFLineInfo=LTypeToDXFLineInfo(pLayer->sLineType);
            }
        }
        DrawEntities(*pB,aT,FALSE);
        aBlockDXFLineInfo=aSavedBlockDXFLineInfo;
        aParentLayerDXFLineInfo=aSavedParentLayerDXFLineInfo;
        nBlockColor=nSavedBlockColor;
        nParentLayerColor=nSavedParentLayerColor;
    }
}


void DXF2GDIMetaFile::DrawEntities(const DXFEntities & rEntities,
                                   const DXFTransform & rTransform,
                                   BOOL bTopEntities)
{
    ULONG nCount=0;
    DXFTransform aET;
    const DXFTransform * pT;

    const DXFBasicEntity * pE=rEntities.pFirst;

    while (pE!=NULL && bStatus==TRUE) {
        if (pE->nSpace==0) {
            if (pE->aExtrusion.fz==1.0) {
                pT=&rTransform;
            }
            else {
                aET=DXFTransform(DXFTransform(pE->aExtrusion),rTransform);
                pT=&aET;
            }
            switch (pE->eType) {
            case DXF_LINE:
                DrawLineEntity((DXFLineEntity&)*pE,*pT);
                break;
            case DXF_POINT:
                DrawPointEntity((DXFPointEntity&)*pE,*pT);
                break;
            case DXF_CIRCLE:
                DrawCircleEntity((DXFCircleEntity&)*pE,*pT);
                break;
            case DXF_ARC:
                DrawArcEntity((DXFArcEntity&)*pE,*pT);
                break;
            case DXF_TRACE:
                DrawTraceEntity((DXFTraceEntity&)*pE,*pT);
                break;
            case DXF_SOLID:
                DrawSolidEntity((DXFSolidEntity&)*pE,*pT);
                break;
            case DXF_TEXT:
                DrawTextEntity((DXFTextEntity&)*pE,*pT);
                break;
            case DXF_INSERT:
                DrawInsertEntity((DXFInsertEntity&)*pE,*pT);
                break;
            case DXF_ATTRIB:
                DrawAttribEntity((DXFAttribEntity&)*pE,*pT);
                break;
            case DXF_POLYLINE:
                DrawPolyLineEntity((DXFPolyLineEntity&)*pE,*pT);
                break;
            case DXF_LWPOLYLINE :
                DrawLWPolyLineEntity((DXFLWPolyLineEntity&)*pE, *pT);
                break;
            case DXF_HATCH :
                DrawHatchEntity((DXFHatchEntity&)*pE, *pT);
                break;
            case DXF_3DFACE:
                Draw3DFaceEntity((DXF3DFaceEntity&)*pE,*pT);
                break;
            case DXF_DIMENSION:
                DrawDimensionEntity((DXFDimensionEntity&)*pE,*pT);
                break;
            default:
                break;  // four other values not handled -Wall
            }
        }
        pE=pE->pSucc;
        nCount++;
        if (bTopEntities) MayCallback(nCount);
    }
}


DXF2GDIMetaFile::DXF2GDIMetaFile()
{
}


DXF2GDIMetaFile::~DXF2GDIMetaFile()
{
}


BOOL DXF2GDIMetaFile::Convert(const DXFRepresentation & rDXF, GDIMetaFile & rMTF, USHORT nminpercent, USHORT nmaxpercent)
{
    double fWidth,fHeight,fScale;
    DXFTransform aTransform;
    Size aPrefSize;
    const DXFLayer * pLayer;
    const DXFVPort * pVPort;

    pVirDev = new VirtualDevice;
    pDXF    = &rDXF;
    bStatus = TRUE;

    OptPointsPerCircle=50;

    nMinPercent=(ULONG)nminpercent;
    nMaxPercent=(ULONG)nmaxpercent;
    nLastPercent=nMinPercent;
    nMainEntitiesCount=CountEntities(pDXF->aEntities);

    nBlockColor=7;
    aBlockDXFLineInfo.eStyle = LINE_SOLID;
    aBlockDXFLineInfo.fWidth = 0;
    aBlockDXFLineInfo.nDashCount = 0;
    aBlockDXFLineInfo.fDashLen = 0;
    aBlockDXFLineInfo.nDotCount = 0;
    aBlockDXFLineInfo.fDotLen = 0;
    aBlockDXFLineInfo.fDistance = 0;

    pLayer=pDXF->aTables.SearchLayer("0");
    if (pLayer!=NULL) {
        nParentLayerColor=pLayer->nColor & 0xff;
        aParentLayerDXFLineInfo=LTypeToDXFLineInfo(pLayer->sLineType);
    }
    else {
        nParentLayerColor=7;
        aParentLayerDXFLineInfo.eStyle = LINE_SOLID;
        aParentLayerDXFLineInfo.fWidth = 0;
        aParentLayerDXFLineInfo.nDashCount = 0;
        aParentLayerDXFLineInfo.fDashLen = 0;
        aParentLayerDXFLineInfo.nDotCount = 0;
        aParentLayerDXFLineInfo.fDotLen = 0;
        aParentLayerDXFLineInfo.fDistance = 0;
    }

    pVirDev->EnableOutput(FALSE);
    rMTF.Record(pVirDev);

    aActLineColor = pVirDev->GetLineColor();
    aActFillColor = pVirDev->GetFillColor();
    aActFont = pVirDev->GetFont();

    pVPort=pDXF->aTables.SearchVPort("*ACTIVE");
    if (pVPort!=NULL) {
        if (pVPort->aDirection.fx==0 && pVPort->aDirection.fy==0)
            pVPort=NULL;
    }

    if (pVPort==NULL) {
        if (pDXF->aBoundingBox.bEmpty==TRUE)
            bStatus=FALSE;
        else {
            fWidth=pDXF->aBoundingBox.fMaxX-pDXF->aBoundingBox.fMinX;
            fHeight=pDXF->aBoundingBox.fMaxY-pDXF->aBoundingBox.fMinY;
            if (fWidth<=0 || fHeight<=0) {
                bStatus=FALSE;
                fScale = 0;  // -Wall added this...
            }
            else {
//				if (fWidth<500.0 || fHeight<500.0 || fWidth>32767.0 || fHeight>32767.0) {
                    if (fWidth>fHeight)
                        fScale=10000.0/fWidth;
                    else
                        fScale=10000.0/fHeight;
//				}
//				else
//					fScale=1.0;
                aTransform=DXFTransform(fScale,-fScale,fScale,
                                        DXFVector(-pDXF->aBoundingBox.fMinX*fScale,
                                                   pDXF->aBoundingBox.fMaxY*fScale,
                                                  -pDXF->aBoundingBox.fMinZ*fScale));
            }
            aPrefSize.Width() =(long)(fWidth*fScale+1.5);
            aPrefSize.Height()=(long)(fHeight*fScale+1.5);
        }
    }
    else {
        fHeight=pVPort->fHeight;
        fWidth=fHeight*pVPort->fAspectRatio;
//		if (fWidth<500.0 || fHeight<500.0 || fWidth>32767.0 || fHeight>32767.0) {
            if (fWidth>fHeight)
                fScale=10000.0/fWidth;
            else
                fScale=10000.0/fHeight;
//		}
//		else
//			fScale=1.0;
        aTransform=DXFTransform(
            DXFTransform(pVPort->aDirection,pVPort->aTarget),
            DXFTransform(
                DXFTransform(1.0,-1.0,1.0,DXFVector(fWidth/2-pVPort->fCenterX,fHeight/2+pVPort->fCenterY,0)),
                DXFTransform(fScale,fScale,fScale,DXFVector(0,0,0))
            )
        );
        aPrefSize.Width() =(long)(fWidth*fScale+1.5);
        aPrefSize.Height()=(long)(fHeight*fScale+1.5);
    }

    if (bStatus==TRUE)
        DrawEntities(pDXF->aEntities,aTransform,TRUE);

    rMTF.Stop();

    if ( bStatus==TRUE )
    {
        rMTF.SetPrefSize( aPrefSize );

        // MapMode einfach, falls Grafik dann nicht zu klein wird (<0,5cm),
        // auf 1/100-mm (1/10-mm) setzen
        if( ( aPrefSize.Width() < 500 ) && ( aPrefSize.Height() < 500 ) )
            rMTF.SetPrefMapMode( MapMode( MAP_10TH_MM ) );
        else
            rMTF.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
    }

    delete pVirDev;
    return bStatus;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
