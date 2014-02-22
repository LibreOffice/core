/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <vcl/fltcall.hxx>

#include <math.h>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>
#include <vcl/metaact.hxx>
#include <vcl/salbtype.hxx>
#include <tools/poly.hxx>
#include <vcl/graph.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svl/solar.hrc>
#include <vcl/gdimetafiletools.hxx>
#include <vcl/dibtools.hxx>



#define BegDocumnMagic 0xA8A8 /* Begin Document */
#define EndDocumnMagic 0xA8A9 /* End Document   */

#define BegResGrpMagic 0xC6A8 /* Begin Resource Group */
#define EndResGrpMagic 0xC6A9 /* End Resource Group   */

#define BegColAtrMagic 0x77A8 /* Begin Color Attribute Table */
#define EndColAtrMagic 0x77A9 /* End Color Attribute Table   */
#define BlkColAtrMagic 0x77B0 /* Color Attribute Table       */
#define MapColAtrMagic 0x77AB /* Map Color Attribute Table   */

#define BegImgObjMagic 0xFBA8 /* Begin Image Object    */
#define EndImgObjMagic 0xFBA9 /* End Image Object      */
#define DscImgObjMagic 0xFBA6 /* Image Data Descriptor */
#define DatImgObjMagic 0xFBEE /* Image Picture Data    */

#define BegObjEnvMagic 0xC7A8 /* Begin Object Environment Group */
#define EndObjEnvMagic 0xC7A9 /* End Object Environment Group   */

#define BegGrfObjMagic 0xBBA8 /* Begin Graphics Object   */
#define EndGrfObjMagic 0xBBA9 /* End Graphics Object     */
#define DscGrfObjMagic 0xBBA6 /* Graphics Data Descritor */
#define DatGrfObjMagic 0xBBEE /* Graphics Data           */

#define MapCodFntMagic 0x8AAB /* Map Coded Font    */
#define MapDatResMagic 0xC3AB /* Map Data Resource */



































struct METChrSet
{
    struct METChrSet * pSucc;
    sal_uInt8 nSet;
    OUString aName;
    FontWeight eWeight;
};

struct METGDIStackMember
{
    struct METGDIStackMember *  pSucc;
    Color                       aLineColor;
    Color                       aFillColor;
    RasterOp                    eRasterOp;
    Font                        aFont;
    MapMode                     aMapMode;
    Rectangle                   aClipRect;
};

class METWriter
{
private:

    sal_Bool                bStatus;
    sal_uLong               nLastPercent; 
    SvStream*           pMET;
    Rectangle           aPictureRect;
    MapMode             aPictureMapMode;
    MapMode             aTargetMapMode;
    sal_uLong               nActualFieldStartPos;     
    sal_uLong               nNumberOfDataFields;  
    Color               aGDILineColor;
    Color               aGDIFillColor;
    RasterOp            eGDIRasterOp;
    Font                aGDIFont;
    MapMode             aGDIMapMode;   
    Rectangle           aGDIClipRect; 
    METGDIStackMember*  pGDIStack;
    Color               aMETColor;
    Color               aMETBackgroundColor;
    Color               aMETPatternSymbol;
    RasterOp            eMETMix ;
    long                nMETStrokeLineWidth;
    Size                aMETChrCellSize;
    short               nMETChrAngle;
    sal_uInt8               nMETChrSet;
    METChrSet*          pChrSetList; 
    sal_uInt8               nNextChrSetId; 
    sal_uLong               nActBitmapId; 
    sal_uLong               nNumberOfActions; 
    sal_uLong               nNumberOfBitmaps; 
    sal_uLong               nWrittenActions;  
    sal_uLong               nWrittenBitmaps;  
    sal_uLong               nActBitmapPercent; 

    ::std::auto_ptr< VirtualDevice >    apDummyVDev;
    OutputDevice*                       pCompDev;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void MayCallback();
        
        

    void CountActionsAndBitmaps(const GDIMetaFile * pMTF);
        
        

    void WriteBigEndianShort(sal_uInt16 nWord);
    void WriteBigEndianLong(sal_uLong nLong);

    void WritePoint(Point aPt);
    void WriteClipRect( const Rectangle& rRect );
    void WriteFieldIntroducer(sal_uInt16 nFieldSize, sal_uInt16 nFieldType,
                              sal_uInt8 nFlags, sal_uInt16 nSegSeqNum);
    void UpdateFieldSize();

    void WriteFieldId(sal_uLong nId);

    void CreateChrSets(const GDIMetaFile * pMTF);
    void CreateChrSet(const Font & rFont);
    void WriteChrSets();
    sal_uInt8 FindChrSet(const Font & rFont);

    void WriteColorAttributeTable(sal_uLong nFieldId=4, BitmapPalette* pPalette=NULL,
                                  sal_uInt8 nBasePartFlags=0x40, sal_uInt8 nBasePartLCTID=0);

    void WriteImageObject(const Bitmap & rBitmap);
    void WriteImageObjects(const GDIMetaFile * pMTF);

    void WriteDataDescriptor(const GDIMetaFile * pMTF);

    void WillWriteOrder(sal_uLong nNextOrderMaximumLength);

    void METSetAndPushLineInfo( const LineInfo& rLineInfo );
    void METPopLineInfo( const LineInfo& rLineInfo );
    void METBitBlt(Point aPt, Size aSize, const Size& rSizePixel);
    void METBeginArea(sal_Bool bBoundaryLine);
    void METEndArea();
    void METBeginPath(sal_uInt32 nPathId);
    void METEndPath();
    void METFillPath(sal_uInt32 nPathId);
    void METOutlinePath(sal_uInt32 nPathId);
    void METCloseFigure();
    void METMove(Point aPt);
    void METLine(Point aPt1, Point aPt2);
    void METLine(const Polygon & rPolygon);
    void METLine(const PolyPolygon & rPolyPolygon);
    void METLineAtCurPos(Point aPt);
    void METBox(sal_Bool bFill, sal_Bool bBoundary,
                Rectangle aRect, sal_uInt32 nHAxis, sal_uInt32 nVAxis);
    void METFullArc(Point aCenter, double fMultiplier);
    void METPartialArcAtCurPos(Point aCenter, double fMultiplier,
                               double fStartAngle, double fSweepAngle);
    void METChrStr(Point aPt, OUString aStr);
    void METSetArcParams(sal_Int32 nP, sal_Int32 nQ, sal_Int32 nR, sal_Int32 nS);
    void METSetColor(Color aColor);
    void METSetBackgroundColor(Color aColor);
    void METSetMix(RasterOp eROP);
    void METSetChrCellSize(Size aSize);
    void METSetChrAngle(short nAngle);
    void METSetChrSet(sal_uInt8 nSet);

    void WriteOrders(const GDIMetaFile * pMTF);

    void WriteObjectEnvironmentGroup(const GDIMetaFile * pMTF);

    void WriteGraphicsObject(const GDIMetaFile * pMTF);

    void WriteResourceGroup(const GDIMetaFile * pMTF);

    void WriteDocument(const GDIMetaFile * pMTF);

public:

    METWriter()
        : bStatus(sal_False)
        , nLastPercent( 0 )
        , pMET(NULL)
        , nActualFieldStartPos( 0 )
        , nNumberOfDataFields( 0 )
        , eGDIRasterOp( ROP_OVERPAINT )
        , pGDIStack(NULL)
        , eMETMix( ROP_OVERPAINT )
        , nMETStrokeLineWidth(0)
        , nMETChrAngle(0)
        , nMETChrSet( 0 )
        , pChrSetList(NULL)
        , nNextChrSetId( 0 )
        , nActBitmapId( 0 )
        , nNumberOfActions( 0 )
        , nNumberOfBitmaps( 0 )
        , nWrittenActions( 0 )
        , nWrittenBitmaps( 0 )
        , nActBitmapPercent( 0 )
        , pCompDev(NULL)
    {
        pCompDev = reinterpret_cast< OutputDevice* >( Application::GetAppWindow() );
        if( !pCompDev )
        {
            apDummyVDev.reset( new VirtualDevice );
            pCompDev = apDummyVDev.get();
        }
    }

    sal_Bool WriteMET( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                        FilterConfigItem* pConfigItem );
};




void METWriter::MayCallback()
{
    if ( xStatusIndicator.is() )
    {
        sal_uLong nPercent;
        nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
                *100/((nNumberOfBitmaps<<14)+nNumberOfActions);

        if (nPercent>=nLastPercent+3)
        {
            nLastPercent = nPercent;
            if ( nPercent <= 100 )
                xStatusIndicator->setValue( nPercent );
        }
    }
}

void METWriter::WriteClipRect( const Rectangle& rRect )
{
    aGDIClipRect = rRect;
    sal_uInt32 nPathId = ( rRect.IsEmpty() ) ? 0 : 1;
    if ( nPathId )
    {
        Polygon aPoly( rRect );
        METBeginPath( nPathId );
        METLine( aPoly );
        METEndPath();
    }
    WillWriteOrder(8);
    pMET->WriteUChar( (sal_uInt8)0xb4 ).WriteUChar( (sal_uInt8)6 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0 ).WriteUInt32( nPathId );
}

void METWriter::CountActionsAndBitmaps(const GDIMetaFile * pMTF)
{
    const MetaAction* pMA;

    for( size_t nAction = 0, nActionCount=pMTF->GetActionSize(); nAction < nActionCount; nAction++ )
    {
        pMA =  pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case META_EPS_ACTION :
            {
                const GDIMetaFile aGDIMetaFile( ((const MetaEPSAction*)pMA)->GetSubstitute() );
                size_t nCount = aGDIMetaFile.GetActionSize();
                size_t i;
                for ( i = 0; i < nCount; i++ )
                    if ( ((const MetaAction*)aGDIMetaFile.GetAction( i ))->GetType() == META_BMPSCALE_ACTION )
                        break;
                if ( i == nCount)
                    break;
            }
            case META_BMP_ACTION:
            case META_BMPSCALE_ACTION:
            case META_BMPSCALEPART_ACTION:
            case META_BMPEX_ACTION:
            case META_BMPEXSCALE_ACTION:
            case META_BMPEXSCALEPART_ACTION:
                nNumberOfBitmaps++;
            break;
        }
        nNumberOfActions++;
    }
}


void METWriter::WriteBigEndianShort(sal_uInt16 nWord)
{
    pMET->WriteUChar( (sal_uInt8)(nWord>>8) ).WriteUChar( (sal_uInt8)(nWord&0x00ff) );
}


void METWriter::WriteBigEndianLong(sal_uLong nLong)
{
    WriteBigEndianShort((sal_uInt16)(nLong>>16));
    WriteBigEndianShort((sal_uInt16)(nLong&0x0000ffff));
}


void METWriter::WritePoint(Point aPt)
{
    Point aNewPt = pCompDev->LogicToLogic( aPt, aPictureMapMode, aTargetMapMode );

    pMET->WriteInt32( (sal_Int32) ( aNewPt.X() - aPictureRect.Left() ) )
         .WriteInt32( (sal_Int32) ( aPictureRect.Bottom() - aNewPt.Y() ) );
}


void METWriter::WriteFieldIntroducer(sal_uInt16 nFieldSize, sal_uInt16 nFieldType,
                                     sal_uInt8 nFlags, sal_uInt16 nSegSeqNum)
{
    nActualFieldStartPos=pMET->Tell();
    WriteBigEndianShort(nFieldSize);
    pMET->WriteUChar( (sal_uInt8)0xd3 ).WriteUInt16( nFieldType ).WriteUChar( nFlags ).WriteUInt16( nSegSeqNum );
}


void METWriter::UpdateFieldSize()
{
    sal_uLong nPos;

    nPos=pMET->Tell();
    pMET->Seek(nActualFieldStartPos);
    WriteBigEndianShort((sal_uInt16)(nPos-nActualFieldStartPos));
    pMET->Seek(nPos);
}


void METWriter::WriteFieldId(sal_uLong nId)
{
    sal_uInt8 nbyte;
    short i;

    for (i=1; i<=8; i++) {
        nbyte= '0' + (sal_uInt8)((nId >> (32-i*4)) & 0x0f);
        pMET->WriteUChar( nbyte );
    }
}


void METWriter::CreateChrSets(const GDIMetaFile * pMTF)
{
    size_t nAction, nActionCount;
    const MetaAction * pMA;

    if (bStatus==sal_False)
        return;

    nActionCount = pMTF->GetActionSize();

    for (nAction = 0; nAction < nActionCount; nAction++)
    {
        pMA = pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case META_FONT_ACTION:
            {
                const MetaFontAction* pA = (const MetaFontAction*) pMA;
                CreateChrSet( pA->GetFont() );
            }
            break;
        }
    }
}


void METWriter::CreateChrSet(const Font & rFont)
{
    METChrSet * pCS;

    if ( FindChrSet( rFont ) == 0 )
    {
        pCS = new METChrSet;
        pCS->pSucc = pChrSetList; pChrSetList=pCS;
        pCS->nSet = nNextChrSetId++;
        pCS->aName = rFont.GetName();
        pCS->eWeight = rFont.GetWeight();
    }
}


sal_uInt8 METWriter::FindChrSet(const Font & rFont)
{
    METChrSet* pCS;

    for (pCS=pChrSetList; pCS!=NULL; pCS=pCS->pSucc)
    {
        if (pCS->aName==rFont.GetName() && pCS->eWeight==rFont.GetWeight() )
            return pCS->nSet;
    }

    return 0;
}


void METWriter::WriteChrSets()
{
    sal_uInt16 i;
    char c = 0;
    METChrSet * pCS;
    sal_uInt8 nbyte;

    for (pCS=pChrSetList; pCS!=NULL; pCS=pCS->pSucc)
    {

        WriteFieldIntroducer(0x58,MapCodFntMagic,0,0);

        WriteBigEndianShort(0x0050);

        pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x84 ).WriteUChar( (sal_uInt8)0x00 );
        pMET->WriteUChar( (sal_uInt8)0xa4 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x01 );
        pMET->WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );

        pMET->WriteUChar( (sal_uInt8)0x04 ).WriteUChar( (sal_uInt8)0x24 ).WriteUChar( (sal_uInt8)0x05 ).WriteUChar( (sal_uInt8)pCS->nSet );

        pMET->WriteUChar( (sal_uInt8)0x14 ).WriteUChar( (sal_uInt8)0x1f );
        switch (pCS->eWeight)
        {
            case WEIGHT_THIN:       nbyte=1; break;
            case WEIGHT_ULTRALIGHT: nbyte=2; break;
            case WEIGHT_LIGHT:      nbyte=3; break;
            case WEIGHT_SEMILIGHT:  nbyte=4; break;
            case WEIGHT_NORMAL:     nbyte=5; break;
            case WEIGHT_SEMIBOLD:   nbyte=6; break;
            case WEIGHT_BOLD:       nbyte=7; break;
            case WEIGHT_ULTRABOLD:  nbyte=8; break;
            case WEIGHT_BLACK:      nbyte=9; break;
            default:                nbyte=5;
        }
        pMET->WriteUChar( nbyte );
        pMET->WriteUChar( (sal_uInt8)0x05 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x0c );

        pMET->WriteUChar( (sal_uInt8)0x06 ).WriteUChar( (sal_uInt8)0x20 ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0xd4 );
        pMET->WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x52 );

        pMET->WriteUChar( (sal_uInt8)0x24 ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0x00 );
        OString n(OUStringToOString(pCS->aName,
            osl_getThreadTextEncoding()));
        for (i=0; i<32; i++)
        {
            if ( i == 0 || c != 0 )
                c = n[i];
            pMET->WriteChar( c );
        }
    }
}


void METWriter::WriteColorAttributeTable(sal_uLong nFieldId, BitmapPalette* pPalette, sal_uInt8 nBasePartFlags, sal_uInt8 nBasePartLCTID)
{
    sal_uInt16 nIndex,nNumI,i;

    if (bStatus==sal_False) return;

    
    WriteFieldIntroducer(16,BegColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    
    WriteFieldIntroducer(0,BlkColAtrMagic,0,0);
    pMET->WriteUChar( nBasePartFlags ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( nBasePartLCTID ); 
    if (pPalette!=NULL)
    {
        nIndex=0;
        while (nIndex<pPalette->GetEntryCount())
        {
            nNumI=pPalette->GetEntryCount()-nIndex;
            if (nNumI>81) nNumI=81;
            pMET->WriteUChar( (sal_uInt8)(11+nNumI*3) );                   
            pMET->WriteUChar( (sal_uInt8)1 ).WriteUChar( (sal_uInt8)0 ).WriteUChar( (sal_uInt8)1 );        
            pMET->WriteUChar( (sal_uInt8)0 ); WriteBigEndianShort(nIndex); 
            pMET->WriteUChar( (sal_uInt8)8 ).WriteUChar( (sal_uInt8)8 ).WriteUChar( (sal_uInt8)8 );        
            pMET->WriteUChar( (sal_uInt8)3 );                              
            for (i=0; i<nNumI; i++)
            {
                const BitmapColor& rCol = (*pPalette)[ nIndex ];

                pMET->WriteUChar( (sal_uInt8) rCol.GetRed() );
                pMET->WriteUChar( (sal_uInt8) rCol.GetGreen() );
                pMET->WriteUChar( (sal_uInt8) rCol.GetBlue() );
                nIndex++;
            }
        }
    }
    else
    {
        
        pMET->WriteUChar( (sal_uInt8)0x0a ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0x00 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0x08 );
    }
    UpdateFieldSize();

    
    WriteFieldIntroducer(16,EndColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    if (pMET->GetError())
        bStatus=sal_False;
}


void METWriter::WriteImageObject(const Bitmap & rBitmap)
{
    SvMemoryStream aTemp(0x00010000,0x00010000);
    sal_uInt32 nWidth,nHeight,nResX,nResY;
    sal_uLong nBytesPerLine,i,j,nNumColors,ny,nLines;
    sal_uLong nActColMapId;
    sal_uInt16 nBitsPerPixel;
    sal_uInt8 nbyte, * pBuf;

    if (bStatus==sal_False)
        return;

    nActColMapId=((nActBitmapId>>24)&0x000000ff) | ((nActBitmapId>> 8)&0x0000ff00) |
                 ((nActBitmapId<< 8)&0x00ff0000) | ((nActBitmapId<<24)&0xff000000);

    
    WriteFieldIntroducer(16,BegImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    
    WriteDIB(rBitmap, aTemp, false, true);

    
    aTemp.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    aTemp.Seek(18);
    aTemp.ReadUInt32( nWidth ).ReadUInt32( nHeight );
    aTemp.SeekRel(2);
    aTemp.ReadUInt16( nBitsPerPixel );
    aTemp.SeekRel(8);
    aTemp.ReadUInt32( nResX ).ReadUInt32( nResY );
    aTemp.SeekRel(8);

    nNumColors=1<<nBitsPerPixel;
    nBytesPerLine=((nWidth*nBitsPerPixel+0x0000001f) & 0xffffffe0 ) >> 3;

    
    if (nBitsPerPixel<=8)
    {
        BitmapPalette   aPal( (sal_uInt16) nNumColors );
        sal_uInt8           nr,ng,nb;

        for (i=0; i<nNumColors; i++)
        {
            aTemp.ReadUChar( nb ).ReadUChar( ng ).ReadUChar( nr ); aTemp.SeekRel(1);
            aPal[ (sal_uInt16) i ] = BitmapColor( nr, ng, nb );
        }

        
        WriteFieldIntroducer(16,BegResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        
        WriteColorAttributeTable(nActColMapId,&aPal,0,1);

        
        WriteFieldIntroducer(16,EndResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        
        WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);

        
        WriteFieldIntroducer(26,MapColAtrMagic,0,0);
        WriteBigEndianShort(0x0012);
        pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x84 ).WriteUChar( (sal_uInt8)0x00 );
        WriteFieldId(nActColMapId);
        pMET->WriteUChar( (sal_uInt8)0x04 ).WriteUChar( (sal_uInt8)0x24 ).WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0x01 );

        
        WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);
    }

    
    WriteFieldIntroducer(17,DscImgObjMagic,0,0);
    pMET->WriteUChar( (sal_uInt8)0x01 ); 
    WriteBigEndianShort((sal_uInt16)nResX);
    WriteBigEndianShort((sal_uInt16)nResY);
    WriteBigEndianShort((sal_uInt16)nWidth);
    WriteBigEndianShort((sal_uInt16)nHeight);

    
    WriteFieldIntroducer(0,DatImgObjMagic,0,0);

    
    pMET->WriteUChar( (sal_uInt8)0x70 ).WriteUChar( (sal_uInt8)0x00 );

    
    pMET->WriteUChar( (sal_uInt8)0x91 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0xff );

    
    pMET->WriteUChar( (sal_uInt8)0x94 ).WriteUChar( (sal_uInt8)0x09 ).WriteUChar( (sal_uInt8)0x02 );
    pMET->WriteUInt16( (sal_uInt16) 0 ).WriteUInt16( (sal_uInt16) 0 );
    WriteBigEndianShort((sal_uInt16)nHeight);
    WriteBigEndianShort((sal_uInt16)nWidth);

    
    pMET->WriteUChar( (sal_uInt8)0x95 ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x03 );

    
    pMET->WriteUChar( (sal_uInt8)0x96 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)nBitsPerPixel );

    if (nBitsPerPixel<=8) {
        
        pMET->WriteUChar( (sal_uInt8)0x97 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0x01 );
    }
    else {
        
        pMET->WriteUChar( (sal_uInt8)0x9b ).WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x01 );
        pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x08 );
        pMET->WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0x08 );
    }

    pBuf=new sal_uInt8[nBytesPerLine];
    ny=0;
    while (ny<nHeight) {

        
        UpdateFieldSize();

        
        WriteFieldIntroducer(0,DatImgObjMagic,0,0);

        
        nLines=nHeight-ny;
        if (nLines*nBytesPerLine>30000) nLines=30000/nBytesPerLine;
        if (nLines<1) nLines=1;
        WriteBigEndianShort(0xfe92);
        WriteBigEndianShort((sal_uInt16)(nLines*nBytesPerLine));
        for (i=0; i<nLines; i++) {
            aTemp.Read(pBuf,nBytesPerLine);
            if (nBitsPerPixel==24) {
                for (j=2; j<nBytesPerLine; j+=3) {
                    nbyte=pBuf[j]; pBuf[j]=pBuf[j-2]; pBuf[j-2]=nbyte;
                }
            }
            pMET->Write(pBuf,nBytesPerLine);
            ny++;
        }
        if (aTemp.GetError() || pMET->GetError()) bStatus=sal_False;
        nActBitmapPercent=(ny+1)*100/nHeight;
        MayCallback();
        if (bStatus==sal_False) { delete[] pBuf; return; }
    }
    delete[] pBuf;

    
    pMET->WriteUChar( (sal_uInt8)0x93 ).WriteUChar( (sal_uInt8)0x00 );

    
    pMET->WriteUChar( (sal_uInt8)0x71 ).WriteUChar( (sal_uInt8)0x00 );

    
    UpdateFieldSize();

    
    WriteFieldIntroducer(16,EndImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    
    nActBitmapId++;

    
    nWrittenBitmaps++;
    nActBitmapPercent=0;

    if (pMET->GetError()) bStatus=sal_False;
}


void METWriter::WriteImageObjects(const GDIMetaFile * pMTF)
{
    const MetaAction*   pMA;

    if (bStatus==sal_False)
        return;

    for ( size_t nAction = 0, nActionCount = pMTF->GetActionSize(); nAction < nActionCount; nAction++)
    {
        pMA = pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case META_BMP_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( ( (MetaBmpAction*) pMA )->GetBitmap() );
            }
            break;

            case META_BMPSCALE_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( ( (MetaBmpScaleAction*) pMA )->GetBitmap() );
            }
            break;

            case META_BMPSCALEPART_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( ( (MetaBmpScalePartAction*) pMA )->GetBitmap() );
            }
            break;

            case META_BMPEX_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( ( (MetaBmpExAction*) pMA )->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case META_BMPEXSCALE_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( ( (MetaBmpExScaleAction*) pMA )->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case META_BMPEXSCALEPART_ACTION:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( ( (MetaBmpExScalePartAction*) pMA )->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case META_EPS_ACTION :
            {
                const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*)pMetaAct;
                        METSetMix( eGDIRasterOp );
                        WriteImageObject( pBmpScaleAction->GetBitmap() );
                        break;
                    }
                }
            }
            break;
        }

        if (bStatus==sal_False)
            break;
    }

    if (pMET->GetError())
        bStatus=sal_False;
}

void METWriter::WriteDataDescriptor(const GDIMetaFile *)
{
    if (bStatus==sal_False)
        return;

    WriteFieldIntroducer(0,DscGrfObjMagic,0,0);

    
    
    

    

    
    
    
    
    
    
    
    
    
    
    pMET->WriteUChar( (sal_uInt8)0xf7 ).WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0xb0 ).WriteUChar( (sal_uInt8)0x00 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x23 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0x01 )
         .WriteUChar( (sal_uInt8)0x05 );

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    Size aUnitsPerDecimeter=OutputDevice::LogicToLogic(Size(10,10),MapMode(MAP_CM),aPictureMapMode);
    pMET->WriteUChar( (sal_uInt8)0xf6 ).WriteUChar( (sal_uInt8)0x28 ).WriteUChar( (sal_uInt8)0x40 ).WriteUChar( (sal_uInt8)0x00 )
         .WriteUChar( (sal_uInt8)0x05 ).WriteUChar( (sal_uInt8)0x01 )
         .WriteUInt32( (sal_uInt32)(aUnitsPerDecimeter.Width()) )
         .WriteUInt32( (sal_uInt32)(aUnitsPerDecimeter.Height()) )
         .WriteUInt32( (sal_uInt32)0 )
         .WriteUInt32( (sal_uInt32)0 ).WriteUInt32( (sal_uInt32)aPictureRect.GetWidth() )
         .WriteUInt32( (sal_uInt32)0 ).WriteUInt32( (sal_uInt32)aPictureRect.GetHeight() )
         .WriteUInt32( (sal_uInt32)0 ).WriteUInt32( (sal_uInt32)0 );

    
    
    
    
    
    
    
    
    
    
    
    
    
    pMET->WriteUChar( (sal_uInt8)0x21 ).WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0x08 ).WriteUChar( (sal_uInt8)0xe0 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x8f ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x05 )
         .WriteUChar( (sal_uInt8)0x05 );

    
    
    
    
    
    
    pMET->WriteUChar( (sal_uInt8)0x21 ).WriteUChar( (sal_uInt8)0x1c ).WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0xcc )
         .WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x8f )
         .WriteUInt32( (sal_uInt32)0x00010000 ).WriteUInt32( (sal_uInt32)0x00000000 ).WriteUInt32( (sal_uInt32)0x00000000 )
         .WriteUInt32( (sal_uInt32)0x00010000 ).WriteUInt32( (sal_uInt32)0x00000000 ).WriteUInt32( (sal_uInt32)0x00000000 );

    
    
    
    
    
    
    
    
    
    
    
    
    //
    
    
    
    
    
    
    //
    
    
    
    
    
    
    
    
    

    
    
    
    
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    pMET->WriteUChar( (sal_uInt8)0x21 ).WriteUChar( (sal_uInt8)0x10 ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x40 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x8f )
         .WriteUChar( (sal_uInt8)0xaa ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 )
         .WriteUChar( (sal_uInt8)0x44 ).WriteUChar( (sal_uInt8)0x04 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 )
         .WriteUChar( (sal_uInt8)0xa8 ).WriteUChar( (sal_uInt8)0xaa ).WriteUChar( (sal_uInt8)0x40 ).WriteUChar( (sal_uInt8)0x44 );

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    pMET->WriteUChar( (sal_uInt8)0x21 ).WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x40 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x8f )
         .WriteUChar( (sal_uInt8)0x66 ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 )
         .WriteUChar( (sal_uInt8)0x66 ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );

    
    
    
    
    
    
    
    
    
    
    
    
    //
    
    
    
    
    
    
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    
    
    if (nNumberOfBitmaps>0) {
        pMET->WriteUChar( (sal_uInt8)0xe7 ).WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0x80 ).WriteUChar( (sal_uInt8)0x00 );
        WriteBigEndianLong(nActBitmapId);
        pMET->WriteUChar( (sal_uInt8)0xfe );
    }

    UpdateFieldSize();

    if (pMET->GetError()) bStatus=sal_False;
}


void METWriter::WillWriteOrder(sal_uLong nNextOrderMaximumLength)
{
    
    
    
    
    
    if (pMET->Tell()-nActualFieldStartPos+nNextOrderMaximumLength>30000)
    {
        UpdateFieldSize();
        WriteFieldIntroducer(0,DatGrfObjMagic,0,0);
        nNumberOfDataFields++;
    }
}



void METWriter::METBitBlt(Point aPt, Size aSize, const Size& rBmpSizePixel)
{
    WillWriteOrder(46);
    pMET->WriteUChar( (sal_uInt8)0xd6 ).WriteUChar( (sal_uInt8)44 ).WriteUInt16( (sal_uInt16)0 ).WriteUInt16( (sal_uInt16) 0x00cc );
    WriteBigEndianLong(nActBitmapId++);
    pMET->WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
    WritePoint(Point(aPt.X(),aPt.Y()+aSize.Height()));
    WritePoint(Point(aPt.X()+aSize.Width(),aPt.Y()));
    pMET->WriteUInt32( (sal_uInt32)0 ).WriteUInt32( (sal_uInt32)0 )
         .WriteUInt32( (sal_uInt32)(rBmpSizePixel.Width()) )
         .WriteUInt32( (sal_uInt32)(rBmpSizePixel.Height()) );
}

void METWriter::METSetAndPushLineInfo( const LineInfo& rLineInfo )
{
    sal_Int32 nWidth = pCompDev->LogicToLogic( Size( rLineInfo.GetWidth(),0 ), aPictureMapMode, aTargetMapMode ).Width();

    WillWriteOrder( 8 );            
    pMET  ->WriteUChar( (sal_uInt8)0x15 )
           .WriteUChar( (sal_uInt8)6 )
           .WriteUChar( (sal_uInt8)0 )             
           .WriteUChar( (sal_uInt8)0 )
           .WriteInt32( nWidth );

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        sal_uInt8 nStyle = 0;           

        switch ( rLineInfo.GetStyle() )
        {
            case LINE_NONE :
                nStyle = 8;
            break;

            case LINE_DASH :
            {
                if ( rLineInfo.GetDotCount() )
                {
                    if ( !rLineInfo.GetDashCount() )
                        nStyle = 1; 
                    else
                        nStyle = 3; 
                }
                else
                    nStyle = 2;     
            }
            break;
            case LineStyle_SOLID:
            case LineStyle_FORCE_EQUAL_SIZE:
                break;  
        }
        WillWriteOrder( 2 );
        pMET->WriteUChar( (sal_uInt8)0x18 ).WriteUChar( nStyle );     
    }
}

void METWriter::METPopLineInfo( const LineInfo& rLineInfo )
{
    WillWriteOrder( 8 );            
    pMET  ->WriteUChar( (sal_uInt8)0x15 )
           .WriteUChar( (sal_uInt8)6 )
           .WriteUChar( (sal_uInt8)0 )             
           .WriteUChar( (sal_uInt8)0 )
           .WriteUInt32( (sal_uInt32)1 );

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        WillWriteOrder( 2 );
        pMET->WriteUChar( (sal_uInt8)0x18 ).WriteUChar( (sal_uInt8)0 );       
    }
}

void METWriter::METBeginArea(sal_Bool bBoundaryLine)
{
    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x68 );
    if (bBoundaryLine) pMET->WriteUChar( (sal_uInt8)0xc0 );
    else               pMET->WriteUChar( (sal_uInt8)0x80 );
}


void METWriter::METEndArea()
{
    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x60 ).WriteUChar( (sal_uInt8)0 );
}


void METWriter::METBeginPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( (sal_uInt8)0xd0 ).WriteUChar( (sal_uInt8)6 ).WriteUInt16( (sal_uInt16) 0 ).WriteUInt32( nPathId );
}


void METWriter::METEndPath()
{
    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x7f ).WriteUChar( (sal_uInt8)0 );
}


void METWriter::METFillPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( (sal_uInt8)0xd7 ).WriteUChar( (sal_uInt8)6 )
         .WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0 ).WriteUInt32( nPathId );
}


void METWriter::METOutlinePath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( (sal_uInt8)0xd4 ).WriteUChar( (sal_uInt8)6 )
         .WriteUChar( (sal_uInt8)0 ).WriteUChar( (sal_uInt8)0 ).WriteUInt32( nPathId );
}


void METWriter::METCloseFigure()
{
    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x7d ).WriteUChar( (sal_uInt8)0 );
}


void METWriter::METMove(Point aPt)
{
    WillWriteOrder(10);
    pMET->WriteUChar( (sal_uInt8)0x21 ).WriteUChar( (sal_uInt8)8 );
    WritePoint(aPt);
}


void METWriter::METLine(Point aPt1, Point aPt2)
{
    WillWriteOrder(18);
    pMET->WriteUChar( (sal_uInt8)0xc1 ).WriteUChar( (sal_uInt8)16 );
    WritePoint(aPt1); WritePoint(aPt2);
}


void METWriter::METLine(const Polygon & rPolygon)
{
    sal_uInt16 nNumPoints,i,j,nOrderPoints;
    sal_Bool bFirstOrder;

    bFirstOrder=sal_True;
    i=0; nNumPoints=rPolygon.GetSize();
    while (i<nNumPoints) {
        nOrderPoints=nNumPoints-i;
        if (nOrderPoints>30) nOrderPoints=30;
        WillWriteOrder(nOrderPoints*8+2);
        if (bFirstOrder==sal_True) {
            pMET->WriteUChar( (sal_uInt8)0xc1 ); 
            bFirstOrder=sal_False;
        }
        else {
            pMET->WriteUChar( (sal_uInt8)0x81 ); 
        }
        pMET->WriteUChar( (sal_uInt8)(nOrderPoints*8) );
        for (j=0; j<nOrderPoints; j++) WritePoint(rPolygon.GetPoint(i++));
    }
}


void METWriter::METLine(const PolyPolygon & rPolyPolygon)
{
    sal_uInt16 i,nCount;
    nCount=rPolyPolygon.Count();
    for (i=0; i<nCount; i++) {
        METLine(rPolyPolygon.GetObject(i));
        METCloseFigure();
    }
}


void METWriter::METLineAtCurPos(Point aPt)
{
    WillWriteOrder(10);
    pMET->WriteUChar( (sal_uInt8)0x81 ).WriteUChar( (sal_uInt8)8 );
    WritePoint(aPt);
}


void METWriter::METBox(sal_Bool bFill, sal_Bool bBoundary,
                       Rectangle aRect, sal_uInt32 nHAxis, sal_uInt32 nVAxis)
{
    sal_uInt8 nFlags=0;
    if (bFill)     nFlags|=0x40;
    if (bBoundary) nFlags|=0x20;

    WillWriteOrder(28);
    pMET->WriteUChar( (sal_uInt8)0xc0 ).WriteUChar( (sal_uInt8)26 ).WriteUChar( nFlags ).WriteUChar( (sal_uInt8)0 );
    WritePoint(aRect.BottomLeft());
    WritePoint(aRect.TopRight());
    pMET->WriteUInt32( nHAxis ).WriteUInt32( nVAxis );
}


void METWriter::METFullArc(Point aCenter, double fMultiplier)
{
    WillWriteOrder(14);
    pMET->WriteUChar( (sal_uInt8)0xc7 ).WriteUChar( (sal_uInt8)12 );
    WritePoint(aCenter);
    pMET->WriteInt32( (sal_Int32)(fMultiplier*65536.0+0.5) );
}


void METWriter::METPartialArcAtCurPos(Point aCenter, double fMultiplier,
                                      double fStartAngle, double fSweepAngle)
{
    fStartAngle*=180.0/3.14159265359;
    while (fStartAngle>360.0) fStartAngle-=360.0;
    while (fStartAngle<0.0) fStartAngle+=360.0;
    fSweepAngle*=180.0/3.14159265359;
    while (fSweepAngle>360.0) fSweepAngle-=360.0;
    while (fSweepAngle<.00) fSweepAngle+=360.0;
    WillWriteOrder(22);
    pMET->WriteUChar( (sal_uInt8)0xa3 ).WriteUChar( (sal_uInt8)20 );
    WritePoint(aCenter);
    pMET->WriteInt32( (sal_Int32)(fMultiplier*65536.0+0.5) );
    pMET->WriteInt32( (sal_Int32)(fStartAngle*65536.0+0.5) );
    pMET->WriteInt32( (sal_Int32)(fSweepAngle*65536.0+0.5) );
}


void METWriter::METChrStr( Point aPt, OUString aUniStr )
{
    OString aStr(OUStringToOString(aUniStr,
        osl_getThreadTextEncoding()));
    sal_uInt16 nLen = aStr.getLength();
    WillWriteOrder( 11 + nLen );
    pMET->WriteUChar( (sal_uInt8)0xc3 ).WriteUChar( (sal_uInt8)( 9 + nLen ) );
    WritePoint(aPt);
    for (sal_uInt16 i = 0; i < nLen; ++i)
        pMET->WriteChar( aStr[i] );
    pMET->WriteUChar( (sal_uInt8)0 );
}


void METWriter::METSetArcParams(sal_Int32 nP, sal_Int32 nQ, sal_Int32 nR, sal_Int32 nS)
{
    WillWriteOrder(18);
    pMET->WriteUChar( (sal_uInt8)0x22 ).WriteUChar( (sal_uInt8)16 ).WriteInt32( nP ).WriteInt32( nQ ).WriteInt32( nR ).WriteInt32( nS );
}


void METWriter::METSetColor(Color aColor)
{
    if (aColor==aMETColor) return;
    aMETColor=aColor;

    WillWriteOrder(6);
    pMET->WriteUChar( (sal_uInt8)0xa6 ).WriteUChar( (sal_uInt8)4 ).WriteUChar( (sal_uInt8)0 )
         .WriteUChar( (sal_uInt8)(aColor.GetBlue()) )
         .WriteUChar( (sal_uInt8)(aColor.GetGreen()) )
         .WriteUChar( (sal_uInt8)(aColor.GetRed()) );
}


void METWriter::METSetBackgroundColor(Color aColor)
{
    if (aColor==aMETBackgroundColor) return;
    aMETBackgroundColor=aColor;

    WillWriteOrder(6);
    pMET->WriteUChar( (sal_uInt8)0xa7 ).WriteUChar( (sal_uInt8)4 ).WriteUChar( (sal_uInt8)0 )
         .WriteUChar( (sal_uInt8)(aColor.GetBlue()) )
         .WriteUChar( (sal_uInt8)(aColor.GetGreen()) )
         .WriteUChar( (sal_uInt8)(aColor.GetRed()) );
}

void METWriter::METSetMix(RasterOp eROP)
{
    sal_uInt8 nMix;

    if (eMETMix==eROP)
        return;

    eMETMix=eROP;

    switch (eROP)
    {
        case ROP_INVERT: nMix=0x0c; break;
        case ROP_XOR:    nMix=0x04; break;
        default:         nMix=0x02;
    }

    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( nMix );
}


void METWriter::METSetChrCellSize(Size aSize)
{
    if (aMETChrCellSize==aSize)
        return;

    aMETChrCellSize=aSize;
    WillWriteOrder(10);
    if (aSize.Width()==0) aSize.Width()=aSize.Height();
    pMET->WriteUChar( (sal_uInt8)0x33 ).WriteUChar( (sal_uInt8)8 ).WriteInt32( (sal_Int32)aSize.Width() ).WriteInt32( (sal_Int32)aSize.Height() );
}


void METWriter::METSetChrAngle(short nAngle)
{
    sal_Int32 nax, nay;

    if (nMETChrAngle==nAngle) return;
    nMETChrAngle=nAngle;

    if (nAngle==0)
    {
        nax=256;
        nay=0;
    }
    else
    {
        double fa=((double)nAngle)/1800.0*3.14159265359;
        nax=(long)(256.0*cos(fa)+0.5);
        nay=(long)(256.0*sin(fa)+0.5);
    }

    WillWriteOrder(10);
    pMET->WriteUChar( (sal_uInt8)0x34 ).WriteUChar( (sal_uInt8)8 ).WriteInt32( nax ).WriteInt32( nay );
}


void METWriter::METSetChrSet(sal_uInt8 nSet)
{
    if (nMETChrSet==nSet)
        return;

    nMETChrSet=nSet;
    WillWriteOrder(2);
    pMET->WriteUChar( (sal_uInt8)0x38 ).WriteUChar( nSet );
}


void METWriter::WriteOrders( const GDIMetaFile* pMTF )
{
    if(bStatus==sal_False)
        return;

    for( size_t nA = 0, nACount = pMTF->GetActionSize(); nA < nACount; nA++ )
    {
        const MetaAction* pMA = pMTF->GetAction( nA );

        switch (pMA->GetType())
        {
            case META_PIXEL_ACTION:
            {
                const MetaPixelAction* pA = (const MetaPixelAction*) pMA;
                METSetMix( eGDIRasterOp );
                METSetColor( pA->GetColor() );
                METLine( pA->GetPoint(),pA->GetPoint() );
            }
            break;

            case META_POINT_ACTION:
            {
                const MetaPointAction* pA = (const MetaPointAction*) pMA;

                METSetArcParams(1,1,0,0);
                METSetMix(eGDIRasterOp);
                METSetColor(aGDILineColor);
                METBeginArea(sal_False);
                METFullArc(pA->GetPoint(),0.5);
                METEndArea();
            }
            break;

            case META_LINE_ACTION:
            {
                const MetaLineAction* pA = (const MetaLineAction*) pMA;

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    LineInfo aLineInfo( pA->GetLineInfo() );
                    if ( ! ( aLineInfo.IsDefault() ) )
                        METSetAndPushLineInfo( aLineInfo );

                    METSetMix( eGDIRasterOp );
                    METSetColor(aGDILineColor);
                    METBeginPath( 1 );
                    METLine( pA->GetStartPoint(), pA->GetEndPoint() );
                    METEndPath();
                    METOutlinePath( 1 );

                    if ( ! ( aLineInfo.IsDefault() ) )
                        METPopLineInfo( aLineInfo );
                }
            }
            break;

            case META_RECT_ACTION:
            {
                const MetaRectAction* pA = (const MetaRectAction*) pMA;

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBox( sal_True, sal_False, pA->GetRect(), 0, 0 );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( sal_False, sal_True, pA->GetRect(), 0, 0 );
                }
            }
            break;

            case META_ROUNDRECT_ACTION:
            {
                const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pMA;

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBox( sal_True, sal_False, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( sal_False, sal_True, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
            }
            break;

            case META_ELLIPSE_ACTION:
            {
                const MetaEllipseAction*    pA = (const MetaEllipseAction*) pMA;
                Point                       aCenter;

                aCenter.X()=(pA->GetRect().Left()+pA->GetRect().Right())/2;
                aCenter.Y()=(pA->GetRect().Top()+pA->GetRect().Bottom())/2;

                METSetArcParams(pA->GetRect().GetWidth(), pA->GetRect().GetHeight(),0,0);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBeginArea(sal_False);
                    METFullArc(aCenter,0.5);
                    METEndArea();
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METFullArc( aCenter,0.5 );
                }
            }
            break;

            case META_ARC_ACTION:
            {
                const MetaArcAction*    pA = (const MetaArcAction*) pMA;
                Point                   aStartPos,aCenter;
                double                  fdx,fdy,fa1,fa2;

                aCenter.X()=(pA->GetRect().Left()+pA->GetRect().Right())/2;
                aCenter.Y()=(pA->GetRect().Top()+pA->GetRect().Bottom())/2;
                fdx=(double)(pA->GetStartPoint().X()-aCenter.X());
                fdy=(double)(pA->GetStartPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa1=atan2(-fdy,fdx);
                fdx=(double)(pA->GetEndPoint().X()-aCenter.X());
                fdy=(double)(pA->GetEndPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa2=atan2(-fdy,fdx);
                aStartPos.X()=aCenter.X()+(long)(((double)pA->GetRect().GetWidth())*cos(fa1)/2.0+0.5);
                aStartPos.Y()=aCenter.Y()-(long)(((double)pA->GetRect().GetHeight())*sin(fa1)/2.0+0.5);

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METSetArcParams(pA->GetRect().GetWidth(), pA->GetRect().GetHeight(),0,0);
                    METBeginPath(1);
                    METMove(aStartPos);
                    METPartialArcAtCurPos(aCenter,0.5,fa1,fa2-fa1);
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_PIE_ACTION:
            {
                const MetaPieAction*    pA = (const MetaPieAction*) pMA;
                Point                   aCenter;
                double                  fdx,fdy,fa1,fa2;

                aCenter.X()=(pA->GetRect().Left()+pA->GetRect().Right())/2;
                aCenter.Y()=(pA->GetRect().Top()+pA->GetRect().Bottom())/2;
                fdx=(double)(pA->GetStartPoint().X()-aCenter.X());
                fdy=(double)(pA->GetStartPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa1=atan2(-fdy,fdx);
                fdx=(double)(pA->GetEndPoint().X()-aCenter.X());
                fdy=(double)(pA->GetEndPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa2=atan2(-fdy,fdx);

                METSetArcParams(pA->GetRect().GetWidth(), pA->GetRect().GetHeight(),0,0);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBeginPath(1);
                    METMove(aCenter);
                    METPartialArcAtCurPos(aCenter,0.5,fa1,fa2-fa1);
                    METLineAtCurPos(aCenter);
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBeginPath(1);
                    METMove(aCenter);
                    METPartialArcAtCurPos(aCenter,0.5,fa1,fa2-fa1);
                    METLineAtCurPos(aCenter);
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_CHORD_ACTION:
            {
                const MetaChordAction*  pA = (const MetaChordAction*) pMA;
                Point                   aStartPos,aCenter;
                double                  fdx,fdy,fa1,fa2;

                aCenter.X()=(pA->GetRect().Left()+pA->GetRect().Right())/2;
                aCenter.Y()=(pA->GetRect().Top()+pA->GetRect().Bottom())/2;
                fdx=(double)(pA->GetStartPoint().X()-aCenter.X());
                fdy=(double)(pA->GetStartPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa1=atan2(-fdy,fdx);
                fdx=(double)(pA->GetEndPoint().X()-aCenter.X());
                fdy=(double)(pA->GetEndPoint().Y()-aCenter.Y());
                fdx*=(double)pA->GetRect().GetHeight();
                fdy*=(double)pA->GetRect().GetWidth();
                if (fdx==0.0 && fdy==0.0) fdx=1.0;
                fa2=atan2(-fdy,fdx);
                aStartPos.X()=aCenter.X()+(long)(((double)pA->GetRect().GetWidth())*cos(fa1)/2.0+0.5);
                aStartPos.Y()=aCenter.Y()-(long)(((double)pA->GetRect().GetHeight())*sin(fa1)/2.0+0.5);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBeginPath(1);
                    METMove(aStartPos);
                    METPartialArcAtCurPos(aCenter,0.5,fa1,fa2-fa1);
                    METLineAtCurPos(aStartPos);
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBeginPath(1);
                    METMove(aStartPos);
                    METPartialArcAtCurPos(aCenter,0.5,fa1,fa2-fa1);
                    METLineAtCurPos(aStartPos);
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_POLYLINE_ACTION:
            {
                const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pMA;

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    LineInfo aLineInfo( pA->GetLineInfo() );
                    if ( ! ( aLineInfo.IsDefault() ) )
                        METSetAndPushLineInfo( aLineInfo );

                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    Polygon aSimplePoly;
                    const Polygon& rPoly = pA->GetPolygon();
                    if ( rPoly.HasFlags() )
                        rPoly.AdaptiveSubdivide( aSimplePoly );
                    else
                        aSimplePoly = rPoly;
                    METLine( aSimplePoly );
                    METEndPath();
                    METOutlinePath(1);

                    if ( ! ( aLineInfo.IsDefault() ) )
                        METPopLineInfo( aLineInfo );
                }
            }
            break;

            case META_POLYGON_ACTION:
            {
                const MetaPolygonAction* pA = (const MetaPolygonAction*) pMA;
                Polygon aSimplePoly;
                const Polygon& rPoly = pA->GetPolygon();
                if ( rPoly.HasFlags() )
                    rPoly.AdaptiveSubdivide( aSimplePoly );
                else
                    aSimplePoly = rPoly;
                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor );
                    METSetBackgroundColor(aGDIFillColor );
                    METBeginPath(1);
                    METLine( aSimplePoly );
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor );
                    METBeginPath(1);
                    METLine( aSimplePoly );
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;

                PolyPolygon aSimplePolyPoly( pA->GetPolyPolygon() );
                sal_uInt16 i, nCount = aSimplePolyPoly.Count();
                for ( i = 0; i < nCount; i++ )
                {
                    if ( aSimplePolyPoly[ i ].HasFlags() )
                    {
                        Polygon aSimplePoly;
                        aSimplePolyPoly[ i ].AdaptiveSubdivide( aSimplePoly );
                        aSimplePolyPoly[ i ] = aSimplePoly;
                    }
                }
                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor);
                    METSetBackgroundColor(aGDIFillColor);
                    METBeginPath(1);
                    METLine( aSimplePolyPoly );
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    METLine( aSimplePolyPoly );
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_TEXT_ACTION:
            {
                const MetaTextAction*   pA = (const MetaTextAction*) pMA;
                Point                   aPt( pA->GetPoint() );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE)
                {
                    VirtualDevice aVDev;

                    if( aGDIFont.GetAlign()==ALIGN_TOP )
                        aPt.Y()+=(long)aVDev.GetFontMetric( aGDIFont ).GetAscent();
                    else
                        aPt.Y()-=(long)aVDev.GetFontMetric( aGDIFont ).GetDescent();
                }

                METSetMix(eGDIRasterOp);
                METSetColor(aGDIFont.GetColor());
                METSetBackgroundColor(aGDIFont.GetFillColor());
                METSetChrCellSize(aGDIFont.GetSize());
                METSetChrAngle(aGDIFont.GetOrientation());
                METSetChrSet(FindChrSet(aGDIFont));
                METChrStr(aPt, pA->GetText().copy(pA->GetIndex(),pA->GetLen()));
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pMA;
                sal_uInt16                      i;
                OUString                    aStr;
                Polygon                     aPolyDummy(1);
                short                       nOrientation;
                Point                       aPt( pA->GetPoint() );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE )
                {
                    VirtualDevice aVDev;
                    if( aGDIFont.GetAlign() == ALIGN_TOP )
                        aPt.Y()+=(long)aVDev.GetFontMetric(aGDIFont).GetAscent();
                    else
                        aPt.Y()-=(long)aVDev.GetFontMetric(aGDIFont).GetDescent();
                }

                METSetMix(eGDIRasterOp);
                METSetColor(aGDIFont.GetColor());
                METSetBackgroundColor(aGDIFont.GetFillColor());
                METSetChrCellSize(aGDIFont.GetSize());
                METSetChrAngle( nOrientation = aGDIFont.GetOrientation() );
                METSetChrSet(FindChrSet(aGDIFont));
                aStr = pA->GetText().copy(pA->GetIndex(),pA->GetLen());

                if( pA->GetDXArray()!=NULL )
                {
                    Point aPt2;

                    for( i=0; i < aStr.getLength(); i++ )
                    {
                        aPt2 = aPt;
                        if ( i > 0 )
                        {
                            aPt2.X() += pA->GetDXArray()[i-1];
                            if ( nOrientation )
                            {
                                aPolyDummy.SetPoint( aPt2, 0 );
                                aPolyDummy.Rotate( aPt, nOrientation );
                                aPt2 = aPolyDummy.GetPoint( 0 );
                            }
                        }
                        METChrStr( aPt2, OUString( aStr[ i ] ) );
                    }
                }
                else
                    METChrStr( aPt, aStr );
            }
            break;

            case META_STRETCHTEXT_ACTION:
            {
                const MetaStretchTextAction*    pA = (const MetaStretchTextAction*) pMA;
                VirtualDevice                   aVDev;
                sal_uInt16                          i;
                sal_Int32*                  pDXAry;
                sal_Int32                       nNormSize;
                OUString                        aStr;
                Polygon                         aPolyDummy(1);
                short                           nOrientation;
                Point                           aPt( pA->GetPoint() );
                Point                           aPt2;

                aVDev.SetFont( aGDIFont );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE)
                {
                    if( aGDIFont.GetAlign() == ALIGN_TOP )
                        aPt.Y()+=(long)aVDev.GetFontMetric().GetAscent();
                    else
                        aPt.Y()-=(long)aVDev.GetFontMetric().GetDescent();
                }

                METSetMix(eGDIRasterOp);
                METSetColor(aGDIFont.GetColor());
                METSetBackgroundColor(aGDIFont.GetFillColor());
                METSetChrCellSize(aGDIFont.GetSize());
                METSetChrAngle( nOrientation = aGDIFont.GetOrientation() );
                METSetChrSet(FindChrSet(aGDIFont));
                aStr = pA->GetText().copy(pA->GetIndex(),pA->GetLen());
                pDXAry = new sal_Int32[aStr.getLength()];
                nNormSize = aVDev.GetTextArray( aStr, pDXAry );

                for ( i = 0; i < aStr.getLength(); i++ )
                {
                    aPt2 = aPt;
                    if ( i > 0 )
                    {
                        aPt2.X() += pDXAry[i-1]*((long)pA->GetWidth())/ nNormSize;
                        if ( nOrientation )
                        {
                            aPolyDummy.SetPoint( aPt2, 0 );
                            aPolyDummy.Rotate( aPt, nOrientation );
                            aPt2 = aPolyDummy.GetPoint( 0 );
                        }
                    }
                    METChrStr( aPt2, OUString( aStr[ i ] ) );
                }

                delete[] pDXAry;
            }
            break;

            case META_TEXTRECT_ACTION:
            {

            }
            break;

            case META_BMP_ACTION:
            {
                const MetaBmpAction*    pA = (const MetaBmpAction*) pMA;
                const Size              aSizePixel( pA->GetBitmap().GetSizePixel() );

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pCompDev->PixelToLogic( aSizePixel, aPictureMapMode ), aSizePixel );
            }
            break;

            case META_BMPSCALE_ACTION:
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pMA;

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pA->GetSize(), pA->GetBitmap().GetSizePixel() );
            }
            break;

            case META_BMPSCALEPART_ACTION:
            {
                const MetaBmpScalePartAction*   pA = (const MetaBmpScalePartAction*) pMA;
                Bitmap                          aTmp( pA->GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), pA->GetBitmap().GetSizePixel() );
            }
            break;

            case META_BMPEX_ACTION:
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction*) pMA;
                const Size              aSizePixel( pA->GetBitmapEx().GetSizePixel() );

                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetPoint(), pCompDev->PixelToLogic( aSizePixel, aPictureMapMode ), aSizePixel );
            }
            break;

            case META_BMPEXSCALE_ACTION:
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pMA;
                const Size                  aSizePixel( pA->GetBitmapEx().GetSizePixel() );

                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetPoint(), pA->GetSize(), aSizePixel );
            }
            break;

            case META_BMPEXSCALEPART_ACTION:
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pMA;
                Bitmap                          aTmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), aTmp.GetSizePixel() );
            }
            break;

            case META_EPS_ACTION :
            {
                const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*)pMetaAct;
                        METSetMix(eGDIRasterOp);
                        METBitBlt( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap().GetSizePixel() );
                        break;
                    }
                }
            }
            break;

            case META_MASK_ACTION:
            break;

            case META_MASKSCALE_ACTION:
            break;

            case META_MASKSCALEPART_ACTION:
            break;

            case META_GRADIENT_ACTION:
            {
                VirtualDevice               aVDev;
                GDIMetaFile                 aTmpMtf;
                const MetaGradientAction*   pA = (const MetaGradientAction*) pMA;

                aVDev.SetMapMode( aTargetMapMode );
                aVDev.AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                WriteOrders( &aTmpMtf );
            }
            break;

            case META_HATCH_ACTION:
            {
                VirtualDevice           aVDev;
                GDIMetaFile             aTmpMtf;
                const MetaHatchAction*  pA = (const MetaHatchAction*) pMA;

                aVDev.SetMapMode( aTargetMapMode );
                aVDev.AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                WriteOrders( &aTmpMtf );
            }
            break;

            case META_WALLPAPER_ACTION:
            break;

            case META_CLIPREGION_ACTION:
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
                const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pMA;
                WriteClipRect( pA->GetRect() );
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            break;

            case META_MOVECLIPREGION_ACTION:
            break;

            case META_LINECOLOR_ACTION:
            {
                const MetaLineColorAction* pA = (const MetaLineColorAction*) pMA;

                if( pA->IsSetting() )
                    aGDILineColor = pA->GetColor();
                else
                    aGDILineColor = Color( COL_TRANSPARENT );
            }
            break;

            case META_FILLCOLOR_ACTION:
            {
                const MetaFillColorAction* pA = (const MetaFillColorAction*) pMA;

                if( pA->IsSetting() )
                    aGDIFillColor = pA->GetColor();
                else
                    aGDIFillColor = Color( COL_TRANSPARENT );
            }
            break;

            case META_TEXTCOLOR_ACTION:
            {
                const MetaTextColorAction* pA = (const MetaTextColorAction*) pMA;
                aGDIFont.SetColor( pA->GetColor() );
            }
            break;

            case META_TEXTFILLCOLOR_ACTION:
            {
                const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pMA;

                if( pA->IsSetting() )
                    aGDIFont.SetFillColor( pA->GetColor() );
                else
                    aGDIFont.SetFillColor( Color( COL_TRANSPARENT ) );
            }
            break;

            case META_TEXTALIGN_ACTION:
            break;

            case META_MAPMODE_ACTION:
            {
                const MetaMapModeAction* pA = (const MetaMapModeAction*) pMA;

                if( aPictureMapMode != pA->GetMapMode() )
                {
                    if ( pA->GetMapMode().GetMapUnit() == MAP_RELATIVE )
                    {
                        MapMode aMM = pA->GetMapMode();
                        Fraction aScaleX = aMM.GetScaleX();
                        Fraction aScaleY = aMM.GetScaleY();

                        Point aOrigin = aPictureMapMode.GetOrigin();
                        BigInt aX( aOrigin.X() );
                        aX *= BigInt( aScaleX.GetDenominator() );

                        if( aOrigin.X() >= 0 )
                        {
                            if( aScaleX.GetNumerator() >= 0 )
                                aX += BigInt( aScaleX.GetNumerator()/2 );
                            else
                                aX -= BigInt( (aScaleX.GetNumerator()+1)/2 );
                        }
                        else
                        {
                            if( aScaleX.GetNumerator() >= 0 )
                                aX -= BigInt( (aScaleX.GetNumerator()-1)/2 );
                            else
                                aX += BigInt( aScaleX.GetNumerator()/2 );
                        }

                        aX /= BigInt( aScaleX.GetNumerator() );
                        aOrigin.X() = (long) aX + aMM.GetOrigin().X();

                        BigInt aY( aOrigin.Y() );
                        aY *= BigInt( aScaleY.GetDenominator() );

                        if( aOrigin.Y() >= 0 )
                        {
                            if( aScaleY.GetNumerator() >= 0 )
                                aY += BigInt( aScaleY.GetNumerator()/2 );
                            else
                                aY -= BigInt( (aScaleY.GetNumerator()+1)/2 );
                        }
                        else
                        {
                            if( aScaleY.GetNumerator() >= 0 )
                                aY -= BigInt( (aScaleY.GetNumerator()-1)/2 );
                            else
                                aY += BigInt( aScaleY.GetNumerator()/2 );
                        }

                        aY /= BigInt( aScaleY.GetNumerator() );
                        aOrigin.Y() = (long)aY + aMM.GetOrigin().Y();
                        aPictureMapMode.SetOrigin( aOrigin );

                        aScaleX *= aPictureMapMode.GetScaleX();
                        aScaleY *= aPictureMapMode.GetScaleY();
                        aPictureMapMode.SetScaleX( aScaleX );
                        aPictureMapMode.SetScaleY( aScaleY );
                    }
                    else
                        aPictureMapMode=pA->GetMapMode();
                }
            }
            break;

            case META_FONT_ACTION:
            {
                aGDIFont = ( (const MetaFontAction*) pMA )->GetFont();
            }
            break;

            case META_PUSH_ACTION:
            {
                METGDIStackMember* pGS = new METGDIStackMember;

                pGS->pSucc=pGDIStack; pGDIStack=pGS;
                pGS->aLineColor=aGDILineColor;
                pGS->aFillColor=aGDIFillColor;
                pGS->eRasterOp=eGDIRasterOp;
                pGS->aFont=aGDIFont;
                pGS->aMapMode=aPictureMapMode;
                pGS->aClipRect=aGDIClipRect;
            }
            break;

            case META_POP_ACTION:
            {
                METGDIStackMember* pGS;

                if( pGDIStack )
                {
                    pGS=pGDIStack; pGDIStack=pGS->pSucc;
                    aGDILineColor=pGS->aLineColor;
                    aGDIFillColor=pGS->aFillColor;
                    eGDIRasterOp=pGS->eRasterOp;
                    aGDIFont=pGS->aFont;
                    if ( pGS->aClipRect != aGDIClipRect )
                        WriteClipRect( pGS->aClipRect );
                    aPictureMapMode=pGS->aMapMode;
                    delete pGS;
                }
            }
            break;

            case META_RASTEROP_ACTION:
            {
                eGDIRasterOp = ( (const MetaRasterOpAction*) pMA )->GetRasterOp();
            }
            break;

            case META_TRANSPARENT_ACTION:
            {
                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor);
                    METSetBackgroundColor(aGDIFillColor);
                    METBeginPath(1);
                    METLine(( (const MetaTransparentAction*) pMA )->GetPolyPolygon());
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    METLine(( (const MetaTransparentAction*) pMA )->GetPolyPolygon());
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_FLOATTRANSPARENT_ACTION:
            {
                const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pMA;

                GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                Point           aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                const Size      aSrcSize( aTmpMtf.GetPrefSize() );
                const Point     aDestPt( pA->GetPoint() );
                const Size      aDestSize( pA->GetSize() );
                const double    fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                const double    fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                long            nMoveX, nMoveY;

                if( fScaleX != 1.0 || fScaleY != 1.0 )
                {
                    aTmpMtf.Scale( fScaleX, fScaleY );
                    aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                }

                nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                if( nMoveX || nMoveY )
                    aTmpMtf.Move( nMoveX, nMoveY );

                WriteOrders( &aTmpMtf );
            }
            break;
      }

      nWrittenActions++;
      MayCallback();

      if( pMET->GetError() )
        bStatus=sal_False;

      if( bStatus == sal_False )
        break;
    }
}

void METWriter::WriteObjectEnvironmentGroup(const GDIMetaFile * pMTF)
{
    sal_uLong i, nId;

    
    WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
    WriteFieldId(7);

    
    WriteFieldIntroducer(22,MapColAtrMagic,0,0);
    WriteBigEndianShort(0x000e);
    pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x84 ).WriteUChar( (sal_uInt8)0x00 );
    WriteFieldId(4);

    
    WriteFieldIntroducer(32,MapCodFntMagic,0,0);
    WriteBigEndianShort(0x0018);
    pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x84 ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0xff ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0x04 ).WriteUChar( (sal_uInt8)0x24 ).WriteUChar( (sal_uInt8)0x05 ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0x06 ).WriteUChar( (sal_uInt8)0x20 );
    pMET->WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x97 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0xb5 );

    
    CreateChrSets(pMTF);
    WriteChrSets();

    
    nId=nActBitmapId;
    for (i=0; i<nNumberOfBitmaps; i++)
    {
        WriteFieldIntroducer(29,MapDatResMagic,0,0);
        WriteBigEndianShort(0x0015);
        pMET->WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x02 ).WriteUChar( (sal_uInt8)0x84 ).WriteUChar( (sal_uInt8)0x00 );
        WriteFieldId(nId);
        pMET->WriteUChar( (sal_uInt8)0x07 ).WriteUChar( (sal_uInt8)0x22 ).WriteUChar( (sal_uInt8)0x10 );
        pMET->WriteUInt32( (sal_uInt32)nId );
        nId++;
    }

    
    WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
    WriteFieldId(7);
}


void METWriter::WriteGraphicsObject(const GDIMetaFile * pMTF)
{
    sal_uLong nSegmentSize,nPos,nDataFieldsStartPos;

    if( bStatus==sal_False )
        return;

    
    WriteFieldIntroducer(16,BegGrfObjMagic,0,0);
    WriteFieldId(7);

    
    WriteObjectEnvironmentGroup(pMTF);

    
    WriteDataDescriptor(pMTF);

    
    nNumberOfDataFields=0;

    
    nDataFieldsStartPos=pMET->Tell();

    
    WriteFieldIntroducer(0,DatGrfObjMagic,0,0);
    nNumberOfDataFields++;

    
    pMET->WriteUChar( (sal_uInt8)0x70 ).WriteUChar( (sal_uInt8)0x0e ).WriteUInt32( (sal_uInt32)0 );
    pMET->WriteUChar( (sal_uInt8)0x70 ).WriteUChar( (sal_uInt8)0x10 ); 
    pMET->WriteUInt16( (sal_uInt16)0 ); 
    pMET->WriteUInt32( (sal_uInt32)0 );  
    pMET->WriteUInt16( (sal_uInt16)0 ); 
    

    
    
    
    WriteOrders(pMTF);

    
    UpdateFieldSize();

    
    nPos=pMET->Tell();
    nSegmentSize=nPos-nDataFieldsStartPos;
    nSegmentSize-=nNumberOfDataFields*8; 
    pMET->Seek(nDataFieldsStartPos+16); 
    WriteBigEndianShort((sal_uInt16)(nSegmentSize&0x0000ffff)); 
    pMET->Seek(nDataFieldsStartPos+22); 
    WriteBigEndianShort((sal_uInt16)(nSegmentSize>>16)); 
    pMET->Seek(nPos); 

    
    WriteFieldIntroducer(16,EndGrfObjMagic,0,0);
    WriteFieldId(7);

    if( pMET->GetError() )
        bStatus=sal_False;
}


void METWriter::WriteResourceGroup(const GDIMetaFile * pMTF)
{
    if( bStatus==sal_False )
        return;

    
    WriteFieldIntroducer(16,BegResGrpMagic,0,0);
    WriteFieldId(2);

    
    WriteColorAttributeTable();
    nActBitmapId=0x77777700;
    WriteImageObjects(pMTF);
    nActBitmapId=0x77777700;
    WriteGraphicsObject(pMTF);

    
    WriteFieldIntroducer(16,EndResGrpMagic,0,0);
    WriteFieldId(2);

    if( pMET->GetError() )
        bStatus=sal_False;
}


void METWriter::WriteDocument(const GDIMetaFile * pMTF)
{
    if( bStatus==sal_False )
        return;

    
    WriteFieldIntroducer(0,BegDocumnMagic,0,0);
    WriteFieldId(1);
    pMET->WriteUChar( (sal_uInt8)0x00 ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0x05 ).WriteUChar( (sal_uInt8)0x18 ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x0c ).WriteUChar( (sal_uInt8)0x00 );
    pMET->WriteUChar( (sal_uInt8)0x06 ).WriteUChar( (sal_uInt8)0x01 ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0xd4 ).WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x52 );
    pMET->WriteUChar( (sal_uInt8)0x03 ).WriteUChar( (sal_uInt8)0x65 ).WriteUChar( (sal_uInt8)0x00 );
    UpdateFieldSize();

    
    WriteResourceGroup(pMTF);

    
    WriteFieldIntroducer(16,EndDocumnMagic,0,0);
    WriteFieldId(1);

    if( pMET->GetError() )
        bStatus=sal_False;
}

sal_Bool METWriter::WriteMET( const GDIMetaFile& rMTF, SvStream& rTargetStream, FilterConfigItem* pFilterConfigItem )
{
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }

    METChrSet*          pCS;
    METGDIStackMember*  pGS;

    bStatus=sal_True;
    nLastPercent=0;

    pMET=&rTargetStream;
    pMET->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

    aPictureRect = Rectangle( Point(), rMTF.GetPrefSize() );
    aTargetMapMode = aPictureMapMode = rMTF.GetPrefMapMode();

    aGDILineColor=Color( COL_BLACK );
    aGDIFillColor=Color( COL_WHITE );
    eGDIRasterOp=ROP_OVERPAINT;
    aGDIFont=Font();
    aGDIMapMode=MapMode();
    aGDIClipRect=Rectangle();
    pGDIStack=NULL;
    aMETColor=Color(COL_BLACK);
    aMETBackgroundColor=Color(COL_WHITE);
    eMETMix=ROP_OVERPAINT;
    nMETStrokeLineWidth=1;
    aMETChrCellSize=Size(0,0);
    nMETChrAngle=0;
    nMETChrSet=0x00;
    pChrSetList=NULL;
    nNextChrSetId=1;
    nNumberOfActions=0;
    nNumberOfBitmaps=0;
    nWrittenActions=0;
    nWrittenBitmaps=0;
    nActBitmapPercent=0;

    CountActionsAndBitmaps(&rMTF);

    WriteDocument(&rMTF);

    while( pChrSetList )
    {
        pCS=pChrSetList;
        pChrSetList=pCS->pSucc;
        delete pCS;
    }

    while( pGDIStack )
    {
        pGS=pGDIStack;
        pGDIStack=pGS->pSucc;
        delete pGS;
    }

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bStatus;
}






#if defined(DISABLE_DYNLOADING)
#define GraphicExport emeGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
{
    METWriter aMETWriter;

    
    GDIMetaFile aMetafile(rGraphic.GetGDIMetaFile());

    if(usesClipActions(aMetafile))
    {
        
        
        
        clipMetafileContentAgainstOwnRegions(aMetafile);
    }

    return aMETWriter.WriteMET( aMetafile, rStream, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
