/*************************************************************************
 *
 *  $RCSfile: dxf2mtf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <math.h>
#include <string.h>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <vcl/poly.hxx>
#include "dxf2mtf.hxx"


ULONG DXF2GDIMetaFile::CountEntities(const DXFEntities & rEntities)
{
    const DXFBasicEntity * pBE;
    ULONG nRes;

    nRes=0;
    for (pBE=rEntities.pFirst; pBE!=NULL; pBE=pBE->pSucc) nRes++;
    return nRes;
}


void DXF2GDIMetaFile::MayCallback(ULONG nMainEntitiesProcessed)
{
    ULONG nPercent;

    if (pCallback!=NULL && nMainEntitiesCount!=0) {
        nPercent=nMinPercent+(nMaxPercent-nMinPercent)*nMainEntitiesProcessed/nMainEntitiesCount;
        if (nPercent>=nLastPercent+4) {
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE) bStatus=FALSE;
            nLastPercent=nPercent;
        }
    }
}

#ifndef VCL
Color DXF2GDIMetaFile::ConvertColor(BYTE nColor)
{
    return Color(
        ((USHORT)pDXF->aPalette.GetRed(nColor))<<8,
        ((USHORT)pDXF->aPalette.GetGreen(nColor))<<8,
        ((USHORT)pDXF->aPalette.GetBlue(nColor))<<8
    );
}
#else
Color DXF2GDIMetaFile::ConvertColor(BYTE nColor)
{
    return Color(
        pDXF->aPalette.GetRed( nColor ),
        pDXF->aPalette.GetGreen( nColor ),
        pDXF->aPalette.GetBlue( nColor ) );
}
#endif

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


PenStyle DXF2GDIMetaFile::LTypeToPStyle(const char * sLineType)
{
    const DXFLType * pLT;
    PenStyle ePStyle;
    pLT=pDXF->aTables.SearchLType(sLineType);
    if (pLT==NULL) ePStyle=PEN_SOLID;
    else if (pLT->nDashCount<=1) ePStyle=PEN_SOLID;
    else if (pLT->nDashCount==2) {
        if (fabs(pLT->fDash[0])*4<fabs(pLT->fPatternLength)) ePStyle=PEN_DOT;
        else ePStyle=PEN_DASH;
    }
    else ePStyle=PEN_DASHDOT;
    return ePStyle;
}


PenStyle DXF2GDIMetaFile::GetEntityPStyle(const DXFBasicEntity & rE)
{
    PenStyle ePStyle;
    const DXFLayer * pLayer;

    if (strcmp(rE.sLineType,"BYLAYER")==0) {
        if (rE.sLayer[0]=='0' && rE.sLayer[1]==0) ePStyle=eParentLayerPStyle;
        else {
            pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) ePStyle=LTypeToPStyle(pLayer->sLineType);
            else ePStyle=eParentLayerPStyle;
        }
    }
    else if (strcmp(rE.sLineType,"BYBLOCK")==0) {
        ePStyle=eBlockPStyle;
    }
    else ePStyle=LTypeToPStyle(rE.sLineType);
    return ePStyle;
}


BOOL DXF2GDIMetaFile::SetLineAttribute(const DXFBasicEntity & rE, ULONG nWidth)
{
    long nColor;
    Color aColor;
    PenStyle ePStyle;

    nColor=GetEntityColor(rE);
    if (nColor<0) return FALSE;
    aColor=ConvertColor((BYTE)nColor);
    ePStyle=GetEntityPStyle(rE);

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


BOOL DXF2GDIMetaFile::SetFontAttribute(const DXFBasicEntity & rE, short nAngle, USHORT nHeight, double fWidthScale)
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
        pVirDev->DrawLine(aP0,aP1);
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
    ByteString  aStr( rE.sText );
    DXFTransform aT( DXFTransform(rE.fXScale,rE.fHeight,1.0,rE.fRotAngle,rE.aP0), rTransform );
    aT.TransDir(DXFVector(0,1,0),aV);
    nHeight=(USHORT)(aV.Abs()+0.5);
    fA=aT.CalcRotAngle();
    nAng=(short)(fA*10.0+0.5);
    aT.TransDir(DXFVector(1,0,0),aV);
    if ( SetFontAttribute( rE,nAng, nHeight, aV. Abs() ) )
    {
        String aUString( aStr, RTL_TEXTENCODING_IBM_437 );
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
        PenStyle eSavedBlockPStyle, eSavedParentLayerPStyle;
        nSavedBlockColor=nBlockColor;
        nSavedParentLayerColor=nParentLayerColor;
        eSavedBlockPStyle=eBlockPStyle;
        eSavedParentLayerPStyle=eParentLayerPStyle;
        nBlockColor=GetEntityColor(rE);
        eBlockPStyle=GetEntityPStyle(rE);
        if (rE.sLayer[0]!='0' || rE.sLayer[1]!=0) {
            DXFLayer * pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) {
                nParentLayerColor=pLayer->nColor;
                eParentLayerPStyle=LTypeToPStyle(pLayer->sLineType);
            }
        }
        DrawEntities(*pB,aT,FALSE);
        eBlockPStyle=eSavedBlockPStyle;
        eParentLayerPStyle=eSavedParentLayerPStyle;
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
            String aUString( aStr, RTL_TEXTENCODING_IBM_437 );
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
        if (rE.nIEFlags&0x0f==0) pVirDev->DrawPolygon(aPoly);
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
        PenStyle eSavedBlockPStyle, eSavedParentLayerPStyle;
        nSavedBlockColor=nBlockColor;
        nSavedParentLayerColor=nParentLayerColor;
        eSavedBlockPStyle=eBlockPStyle;
        eSavedParentLayerPStyle=eParentLayerPStyle;
        nBlockColor=GetEntityColor(rE);
        eBlockPStyle=GetEntityPStyle(rE);
        if (rE.sLayer[0]!='0' || rE.sLayer[1]!=0) {
            DXFLayer * pLayer=pDXF->aTables.SearchLayer(rE.sLayer);
            if (pLayer!=NULL) {
                nParentLayerColor=pLayer->nColor;
                eParentLayerPStyle=LTypeToPStyle(pLayer->sLineType);
            }
        }
        DrawEntities(*pB,aT,FALSE);
        eBlockPStyle=eSavedBlockPStyle;
        eParentLayerPStyle=eSavedParentLayerPStyle;
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
            case DXF_3DFACE:
                Draw3DFaceEntity((DXF3DFaceEntity&)*pE,*pT);
                break;
            case DXF_DIMENSION:
                DrawDimensionEntity((DXFDimensionEntity&)*pE,*pT);
                break;
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


BOOL DXF2GDIMetaFile::Convert(const DXFRepresentation & rDXF, GDIMetaFile & rMTF,
                              PFilterCallback pcallback, void * pcallerdata,
                              USHORT nminpercent, USHORT nmaxpercent)
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

    pCallback=pcallback;
    pCallerData=pcallerdata;
    nMinPercent=(ULONG)nminpercent;
    nMaxPercent=(ULONG)nmaxpercent;
    nLastPercent=nMinPercent;
    nMainEntitiesCount=CountEntities(pDXF->aEntities);

    nBlockColor=7;
    eBlockPStyle=PEN_SOLID;

    pLayer=pDXF->aTables.SearchLayer("0");
    if (pLayer!=NULL) {
        nParentLayerColor=pLayer->nColor & 0xff;
        eParentLayerPStyle=LTypeToPStyle(pLayer->sLineType);
    }
    else {
        nParentLayerColor=7;
        eParentLayerPStyle=PEN_SOLID;
    }

    pVirDev->EnableOutput(FALSE);
    rMTF.Record(pVirDev);

    aActLineColor = pVirDev->GetLineColor();
    aActFillColor = pVirDev->GetFillColor();
    aActFont = pVirDev->GetFont();

    pVPort=pDXF->aTables.SearchVPort("*ACTIVE");
    if (pVPort!=NULL) {
        if (pVPort->aDirection.fx==0 && pVPort->aDirection.fy==0) pVPort=NULL;
    }

    if (pVPort==NULL) {
        if (pDXF->aBoundingBox.bEmpty==TRUE) bStatus=FALSE;
        else {
            fWidth=pDXF->aBoundingBox.fMaxX-pDXF->aBoundingBox.fMinX;
            fHeight=pDXF->aBoundingBox.fMaxY-pDXF->aBoundingBox.fMinY;
            if (fWidth<=0 || fHeight<=0) bStatus=FALSE;
            else {
                if (fWidth<500.0 || fHeight<500.0 || fWidth>32767.0 || fHeight>32767.0) {
                    if (fWidth>fHeight) fScale=10000.0/fWidth;
                    else fScale=10000.0/fHeight;
                }
                else fScale=1.0;
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
        if (fWidth<500.0 || fHeight<500.0 || fWidth>32767.0 || fHeight>32767.0) {
            if (fWidth>fHeight) fScale=10000.0/fWidth;
            else fScale=10000.0/fHeight;
        }
        else fScale=1.0;
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

    if (bStatus==TRUE) DrawEntities(pDXF->aEntities,aTransform,TRUE);

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



