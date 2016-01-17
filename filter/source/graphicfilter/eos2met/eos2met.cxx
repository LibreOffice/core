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

#include <vcl/fltcall.hxx>

#include <math.h>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>
#include <vcl/metaact.hxx>
#include <vcl/salbtype.hxx>
#include <tools/poly.hxx>
#include <tools/fract.hxx>
#include <vcl/graphic.hxx>
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
#include <memory>

// -----------------------------Field Types-------------------------------

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

// Struktur des Metafiles
// BegDocumn
//   BegResGrp
//     BegColAtr
//       BlkColAtr
//     EndColAtr
//     BegImgObj[0..n]
//       BegResGrp[]
//         BegColAtr[]
//           BlkColAtr
//         EndColAtr
//       EndResGrp
//       BegObjEnv[]
//         MapColAtr
//       EndObjEnv
//       DscImgObj
//       DatImgOb1
//       DatImgOb2[1..n]
//     EndImgObj
//     BegGrfObj
//       BegObjEnv[]
//         MapColAtr
//         MapCodFnt1
//         MapCodFnt2[0..n]
//         MapDatRes[0..n]
//       EndObjEnv
//       DscGrfObj
//       DatGrfObj[0..n]
//     EndGrfObj
//   EndResGrp
// EndDocumn

//============================== METWriter ===================================

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
    vcl::Font                   aFont;
    MapMode                     aMapMode;
    Rectangle                   aClipRect;
};

class METWriter
{
private:

    bool                bStatus;
    sal_uInt32          nLastPercent; // with which number pCallback has been called the last time
    SvStream*           pMET;
    Rectangle           aPictureRect;
    MapMode             aPictureMapMode;
    MapMode             aTargetMapMode;
    sal_uInt32          nActualFieldStartPos;     // start position of the current 'Field'
    sal_uInt32          nNumberOfDataFields;  // number of commenced 'Graphics Data Fields'
    Color               aGDILineColor;
    Color               aGDIFillColor;
    RasterOp            eGDIRasterOp;
    vcl::Font           aGDIFont;
    MapMode             aGDIMapMode;   // currently ununsed!
    Rectangle           aGDIClipRect; // currently ununsed!
    METGDIStackMember*  pGDIStack;
    Color               aMETColor;
    Color               aMETBackgroundColor;
    RasterOp            eMETMix ;
    long                nMETStrokeLineWidth;
    Size                aMETChrCellSize;
    short               nMETChrAngle;
    sal_uInt8           nMETChrSet;
    METChrSet*          pChrSetList; // list of Character-Sets
    sal_uInt8           nNextChrSetId; // the first unused ChrSet-Id
    sal_uInt32          nActBitmapId; // Field-Id of the next Bitmap
    sal_uInt32          nNumberOfActions; // number of Actions in the GDIMetafile
    sal_uInt32               nNumberOfBitmaps; // number of Bitmaps
    sal_uInt32               nWrittenActions;  // number of already processed actions during the writing of the orders
    sal_uInt32               nWrittenBitmaps;  // number of already written Bitmaps
    sal_uInt32               nActBitmapPercent; // percentage of the next bitmap that's already written

    ScopedVclPtr<VirtualDevice>  apDummyVDev;
    VclPtr<OutputDevice>         pCompDev;

    css::uno::Reference< css::task::XStatusIndicator > xStatusIndicator;

    void MayCallback();
        // calculates a percentage based on the 5 parameters above and then does a
        // Callback as the case may be. Sets bStatus to sal_False if the user wants to cancel

    void CountActionsAndBitmaps(const GDIMetaFile * pMTF);
        // Counts the bitmaps and actions (nNumberOfActions and nNumberOfBitmaps have to
        // be set to 0 at the beginning, since this method is recursive)

    void WriteBigEndianShort(sal_uInt16 nWord);
    void WriteBigEndianLong(sal_uInt32 nLong);

    void WritePoint(Point aPt);
    void WriteClipRect( const Rectangle& rRect );
    void WriteFieldIntroducer(sal_uInt16 nFieldSize, sal_uInt16 nFieldType,
                              sal_uInt8 nFlags, sal_uInt16 nSegSeqNum);
    void UpdateFieldSize();

    void WriteFieldId(sal_uInt32 nId);

    void CreateChrSets(const GDIMetaFile * pMTF);
    void CreateChrSet(const vcl::Font & rFont);
    void WriteChrSets();
    sal_uInt8 FindChrSet(const vcl::Font & rFont);

    void WriteColorAttributeTable(sal_uInt32 nFieldId=4, BitmapPalette* pPalette=nullptr,
                                  sal_uInt8 nBasePartFlags=0x40, sal_uInt8 nBasePartLCTID=0);

    void WriteImageObject(const Bitmap & rBitmap);
    void WriteImageObjects(const GDIMetaFile * pMTF);

    void WriteDataDescriptor(const GDIMetaFile * pMTF);

    void WillWriteOrder(sal_uInt32 nNextOrderMaximumLength);

    void METSetAndPushLineInfo( const LineInfo& rLineInfo );
    void METPopLineInfo( const LineInfo& rLineInfo );
    void METBitBlt(Point aPt, Size aSize, const Size& rSizePixel);
    void METBeginArea(bool bBoundaryLine);
    void METEndArea();
    void METBeginPath(sal_uInt32 nPathId);
    void METEndPath();
    void METFillPath(sal_uInt32 nPathId);
    void METOutlinePath(sal_uInt32 nPathId);
    void METCloseFigure();
    void METMove(Point aPt);
    void METLine(Point aPt1, Point aPt2);
    void METLine(const tools::Polygon & rPolygon);
    void METLine(const tools::PolyPolygon & rPolyPolygon);
    void METLineAtCurPos(Point aPt);
    void METBox(bool bFill, bool bBoundary,
                const Rectangle& rRect, sal_uInt32 nHAxis, sal_uInt32 nVAxis);
    void METFullArc(Point aCenter, double fMultiplier);
    void METPartialArcAtCurPos(Point aCenter, double fMultiplier,
                               double fStartAngle, double fSweepAngle);
    void METChrStr(Point aPt, const OUString& aStr);
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
        : bStatus(false)
        , nLastPercent( 0 )
        , pMET(nullptr)
        , nActualFieldStartPos( 0 )
        , nNumberOfDataFields( 0 )
        , eGDIRasterOp( ROP_OVERPAINT )
        , pGDIStack(nullptr)
        , eMETMix( ROP_OVERPAINT )
        , nMETStrokeLineWidth(0)
        , nMETChrAngle(0)
        , nMETChrSet( 0 )
        , pChrSetList(nullptr)
        , nNextChrSetId( 0 )
        , nActBitmapId( 0 )
        , nNumberOfActions( 0 )
        , nNumberOfBitmaps( 0 )
        , nWrittenActions( 0 )
        , nWrittenBitmaps( 0 )
        , nActBitmapPercent( 0 )
        , pCompDev(nullptr)
    {
        pCompDev = reinterpret_cast< OutputDevice* >( Application::GetAppWindow() );
        if( !pCompDev )
        {
            apDummyVDev.disposeAndReset(VclPtr<VirtualDevice>::Create());
            pCompDev = apDummyVDev.get();
        }
    }

    bool WriteMET( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                        FilterConfigItem* pConfigItem );
};


//========================== Methods of METWriter ==========================

void METWriter::MayCallback()
{
    if ( xStatusIndicator.is() )
    {
        sal_uInt32 nPercent;
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
        tools::Polygon aPoly( rRect );
        METBeginPath( nPathId );
        METLine( aPoly );
        METEndPath();
    }
    WillWriteOrder(8);
    pMET->WriteUChar( 0xb4 ).WriteUChar( 6 )
         .WriteUChar( 0x00 ).WriteUChar( 0 ).WriteUInt32( nPathId );
}

void METWriter::CountActionsAndBitmaps(const GDIMetaFile * pMTF)
{
    for( size_t nAction = 0, nActionCount=pMTF->GetActionSize(); nAction < nActionCount; nAction++ )
    {
        const MetaAction* pMA =  pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case MetaActionType::EPS :
            {
                const GDIMetaFile aGDIMetaFile( static_cast<const MetaEPSAction*>(pMA)->GetSubstitute() );
                size_t nCount = aGDIMetaFile.GetActionSize();
                size_t i;
                for ( i = 0; i < nCount; i++ )
                    if ( const_cast<const MetaAction*>(aGDIMetaFile.GetAction( i ))->GetType() == MetaActionType::BMPSCALE )
                        break;
                if ( i == nCount)
                    break;
            }
            case MetaActionType::BMP:
            case MetaActionType::BMPSCALE:
            case MetaActionType::BMPSCALEPART:
            case MetaActionType::BMPEX:
            case MetaActionType::BMPEXSCALE:
            case MetaActionType::BMPEXSCALEPART:
                nNumberOfBitmaps++;
            break;
            default: break;
        }
        nNumberOfActions++;
    }
}


void METWriter::WriteBigEndianShort(sal_uInt16 nWord)
{
    pMET->WriteUChar( nWord>>8 ).WriteUChar( nWord&0x00ff );
}


void METWriter::WriteBigEndianLong(sal_uInt32 nLong)
{
    WriteBigEndianShort((sal_uInt16)(nLong>>16));
    WriteBigEndianShort((sal_uInt16)(nLong&0x0000ffff));
}


void METWriter::WritePoint(Point aPt)
{
    Point aNewPt = OutputDevice::LogicToLogic( aPt, aPictureMapMode, aTargetMapMode );

    pMET->WriteInt32( aNewPt.X() - aPictureRect.Left() )
         .WriteInt32( aPictureRect.Bottom() - aNewPt.Y() );
}


void METWriter::WriteFieldIntroducer(sal_uInt16 nFieldSize, sal_uInt16 nFieldType,
                                     sal_uInt8 nFlags, sal_uInt16 nSegSeqNum)
{
    nActualFieldStartPos=pMET->Tell();
    WriteBigEndianShort(nFieldSize);
    pMET->WriteUChar( 0xd3 ).WriteUInt16( nFieldType ).WriteUChar( nFlags ).WriteUInt16( nSegSeqNum );
}


void METWriter::UpdateFieldSize()
{
    sal_uInt32 nPos;

    nPos=pMET->Tell();
    pMET->Seek(nActualFieldStartPos);
    WriteBigEndianShort((sal_uInt16)(nPos-nActualFieldStartPos));
    pMET->Seek(nPos);
}


void METWriter::WriteFieldId(sal_uInt32 nId)
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

    if (!bStatus)
        return;

    nActionCount = pMTF->GetActionSize();

    for (nAction = 0; nAction < nActionCount; nAction++)
    {
        const MetaAction * pMA = pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case MetaActionType::FONT:
            {
                const MetaFontAction* pA = static_cast<const MetaFontAction*>(pMA);
                CreateChrSet( pA->GetFont() );
            }
            break;
            default: break;
        }
    }
}


void METWriter::CreateChrSet(const vcl::Font & rFont)
{
    METChrSet * pCS;

    if ( FindChrSet( rFont ) == 0 )
    {
        pCS = new METChrSet;
        pCS->pSucc = pChrSetList; pChrSetList=pCS;
        pCS->nSet = nNextChrSetId++;
        pCS->aName = rFont.GetFamilyName();
        pCS->eWeight = rFont.GetWeight();
    }
}


sal_uInt8 METWriter::FindChrSet(const vcl::Font & rFont)
{
    METChrSet* pCS;

    for (pCS=pChrSetList; pCS!=nullptr; pCS=pCS->pSucc)
    {
        if (pCS->aName==rFont.GetFamilyName() && pCS->eWeight==rFont.GetWeight() )
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

    for (pCS=pChrSetList; pCS!=nullptr; pCS=pCS->pSucc)
    {

        WriteFieldIntroducer(0x58,MapCodFntMagic,0,0);

        WriteBigEndianShort(0x0050);

        pMET->WriteUChar( 0x0c ).WriteUChar( 0x02 ).WriteUChar( 0x84 ).WriteUChar( 0x00 );
        pMET->WriteUChar( 0xa4 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x01 );
        pMET->WriteUChar( 0x01 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );

        pMET->WriteUChar( 0x04 ).WriteUChar( 0x24 ).WriteUChar( 0x05 ).WriteUChar( pCS->nSet );

        pMET->WriteUChar( 0x14 ).WriteUChar( 0x1f );
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
        pMET->WriteUChar( 0x05 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x0c );

        pMET->WriteUChar( 0x06 ).WriteUChar( 0x20 ).WriteUChar( 0x03 ).WriteUChar( 0xd4 );
        pMET->WriteUChar( 0x03 ).WriteUChar( 0x52 );

        pMET->WriteUChar( 0x24 ).WriteUChar( 0x02 ).WriteUChar( 0x08 ).WriteUChar( 0x00 );
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


void METWriter::WriteColorAttributeTable(sal_uInt32 nFieldId, BitmapPalette* pPalette, sal_uInt8 nBasePartFlags, sal_uInt8 nBasePartLCTID)
{
    sal_uInt16 nIndex,nNumI,i;

    if (!bStatus) return;

    //--- The Field 'Begin Color Attribute Table':
    WriteFieldIntroducer(16,BegColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    //--- The Field 'Color Attribute Table':
    WriteFieldIntroducer(0,BlkColAtrMagic,0,0);
    pMET->WriteUChar( nBasePartFlags ).WriteUChar( 0x00 ).WriteUChar( nBasePartLCTID ); // 'Base Part'
    if (pPalette!=nullptr)
    {
        nIndex=0;
        while (nIndex<pPalette->GetEntryCount())
        {
            nNumI=pPalette->GetEntryCount()-nIndex;
            if (nNumI>81) nNumI=81;
            pMET->WriteUChar( (11+nNumI*3) );                   // length of the parameter
            pMET->WriteUChar( 1 ).WriteUChar( 0 ).WriteUChar( 1 );        // typ: element list, Reserved, Format: RGB
            pMET->WriteUChar( 0 ); WriteBigEndianShort(nIndex); // start-Index (3 Bytes)
            pMET->WriteUChar( 8 ).WriteUChar( 8 ).WriteUChar( 8 );        // Bits per component R,G,B
            pMET->WriteUChar( 3 );                              // number of bytes per entry
            for (i=0; i<nNumI; i++)
            {
                const BitmapColor& rCol = (*pPalette)[ nIndex ];

                pMET->WriteUChar( rCol.GetRed() );
                pMET->WriteUChar( rCol.GetGreen() );
                pMET->WriteUChar( rCol.GetBlue() );
                nIndex++;
            }
        }
    }
    else
    {
        // 'Trible Generating'
        pMET->WriteUChar( 0x0a ).WriteUChar( 0x02 ).WriteUChar( 0x00 ).WriteUChar( 0x01 ).WriteUChar( 0x00 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x08 ).WriteUChar( 0x08 ).WriteUChar( 0x08 );
    }
    UpdateFieldSize();

    //--- The Field 'End Color Attribute Table':
    WriteFieldIntroducer(16,EndColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    if (pMET->GetError())
        bStatus=false;
}


void METWriter::WriteImageObject(const Bitmap & rBitmap)
{
    SvMemoryStream aTemp(0x00010000,0x00010000);
    sal_uInt32 nWidth,nHeight,nResX,nResY;
    sal_uInt32 nBytesPerLine,i,j,nNumColors,ny,nLines;
    sal_uInt32 nActColMapId;
    sal_uInt16 nBitsPerPixel;
    sal_uInt8 nbyte;

    if (!bStatus)
        return;

    nActColMapId=((nActBitmapId>>24)&0x000000ff) | ((nActBitmapId>> 8)&0x0000ff00) |
                 ((nActBitmapId<< 8)&0x00ff0000) | ((nActBitmapId<<24)&0xff000000);

    //--- The Field 'Begin Image Object':
    WriteFieldIntroducer(16,BegImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    // generate Windows-BMP file
    WriteDIB(rBitmap, aTemp, false, true);

    // read header of the Windows-BMP file:
    aTemp.SetEndian(SvStreamEndian::LITTLE);
    aTemp.Seek(18);
    aTemp.ReadUInt32( nWidth ).ReadUInt32( nHeight );
    aTemp.SeekRel(2);
    aTemp.ReadUInt16( nBitsPerPixel );
    aTemp.SeekRel(8);
    aTemp.ReadUInt32( nResX ).ReadUInt32( nResY );
    aTemp.SeekRel(8);

    nNumColors=1<<nBitsPerPixel;
    nBytesPerLine=((nWidth*nBitsPerPixel+0x0000001f) & 0xffffffe0 ) >> 3;

    // read color palette as the case may be and write it to the MET file:
    if (nBitsPerPixel<=8)
    {
        BitmapPalette   aPal( (sal_uInt16) nNumColors );
        sal_uInt8           nr,ng,nb;

        for (i=0; i<nNumColors; i++)
        {
            aTemp.ReadUChar( nb ).ReadUChar( ng ).ReadUChar( nr ); aTemp.SeekRel(1);
            aPal[ (sal_uInt16) i ] = BitmapColor( nr, ng, nb );
        }

        //--- The Field 'Begin Resource Group':
        WriteFieldIntroducer(16,BegResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        //--- writer color table:
        WriteColorAttributeTable(nActColMapId,&aPal,0,1);

        //--- The Field 'End Resource Group':
        WriteFieldIntroducer(16,EndResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        //--- The Field 'Begin Object Environment Group':
        WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);

        //--- The Field 'Map Color Attribute Table':
        WriteFieldIntroducer(26,MapColAtrMagic,0,0);
        WriteBigEndianShort(0x0012);
        pMET->WriteUChar( 0x0c ).WriteUChar( 0x02 ).WriteUChar( 0x84 ).WriteUChar( 0x00 );
        WriteFieldId(nActColMapId);
        pMET->WriteUChar( 0x04 ).WriteUChar( 0x24 ).WriteUChar( 0x07 ).WriteUChar( 0x01 );

        //--- The Field 'End Object Environment Group':
        WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);
    }

    //--- The Field 'Image Data Descriptor':
    WriteFieldIntroducer(17,DscImgObjMagic,0,0);
    pMET->WriteUChar( 0x01 ); // Unit of measure: tens of centimeters
    WriteBigEndianShort((sal_uInt16)nResX);
    WriteBigEndianShort((sal_uInt16)nResY);
    WriteBigEndianShort((sal_uInt16)nWidth);
    WriteBigEndianShort((sal_uInt16)nHeight);

    //--- The first Field 'Image Picture Data':
    WriteFieldIntroducer(0,DatImgObjMagic,0,0);

    // Begin Segment:
    pMET->WriteUChar( 0x70 ).WriteUChar( 0x00 );

    // Begin Image Content:
    pMET->WriteUChar( 0x91 ).WriteUChar( 0x01 ).WriteUChar( 0xff );

    // Image Size:
    pMET->WriteUChar( 0x94 ).WriteUChar( 0x09 ).WriteUChar( 0x02 );
    pMET->WriteUInt16( 0 ).WriteUInt16( 0 );
    WriteBigEndianShort((sal_uInt16)nHeight);
    WriteBigEndianShort((sal_uInt16)nWidth);

    // Image Encoding:
    pMET->WriteUChar( 0x95 ).WriteUChar( 0x02 ).WriteUChar( 0x03 ).WriteUChar( 0x03 );

    // Image IDE-Size:
    pMET->WriteUChar( 0x96 ).WriteUChar( 0x01 ).WriteUChar( nBitsPerPixel );

    if (nBitsPerPixel<=8) {
        // Image LUT-ID
        pMET->WriteUChar( 0x97 ).WriteUChar( 0x01 ).WriteUChar( 0x01 );
    }
    else {
        // IDE Structure
        pMET->WriteUChar( 0x9b ).WriteUChar( 0x08 ).WriteUChar( 0x00 ).WriteUChar( 0x01 );
        pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x08 );
        pMET->WriteUChar( 0x08 ).WriteUChar( 0x08 );
    }

    std::unique_ptr<sal_uInt8[]> pBuf(new sal_uInt8[nBytesPerLine]);
    ny=0;
    while (ny<nHeight) {

        // finalize the previous field 'Image Picture Data':
        UpdateFieldSize();

        // and start a new field 'Image Picture Data':
        WriteFieldIntroducer(0,DatImgObjMagic,0,0);

        // read and write several Scanlines:
        nLines=nHeight-ny;
        if (nLines*nBytesPerLine>30000) nLines=30000/nBytesPerLine;
        if (nLines<1) nLines=1;
        WriteBigEndianShort(0xfe92);
        WriteBigEndianShort((sal_uInt16)(nLines*nBytesPerLine));
        for (i=0; i<nLines; i++) {
            aTemp.Read(pBuf.get(),nBytesPerLine);
            if (nBitsPerPixel==24) {
                for (j=2; j<nBytesPerLine; j+=3) {
                    nbyte=pBuf[j]; pBuf[j]=pBuf[j-2]; pBuf[j-2]=nbyte;
                }
            }
            pMET->Write(pBuf.get(),nBytesPerLine);
            ny++;
        }
        if (aTemp.GetError() || pMET->GetError()) bStatus=false;
        nActBitmapPercent=(ny+1)*100/nHeight;
        MayCallback();
        if (!bStatus) return;
    }
    pBuf.reset();

    // End Image Content:
    pMET->WriteUChar( 0x93 ).WriteUChar( 0x00 );

    // End Segment:
    pMET->WriteUChar( 0x71 ).WriteUChar( 0x00 );

    // finalize the last field 'Image Picture Data':
    UpdateFieldSize();

    //--- The Field 'End Image Object':
    WriteFieldIntroducer(16,EndImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    // increase Ids:
    nActBitmapId++;

    // count Bitmaps:
    nWrittenBitmaps++;
    nActBitmapPercent=0;

    if (pMET->GetError()) bStatus=false;
}


void METWriter::WriteImageObjects(const GDIMetaFile * pMTF)
{
    if (!bStatus)
        return;

    for ( size_t nAction = 0, nActionCount = pMTF->GetActionSize(); nAction < nActionCount; nAction++)
    {
        const MetaAction* pMA = pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case MetaActionType::BMP:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( static_cast<const MetaBmpAction*>(pMA)->GetBitmap() );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( static_cast<const MetaBmpScaleAction*>(pMA)->GetBitmap() );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( static_cast<const MetaBmpScalePartAction*>(pMA)->GetBitmap() );
            }
            break;

            case MetaActionType::BMPEX:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( static_cast<const MetaBmpExAction*>(pMA)->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( static_cast<const MetaBmpExScaleAction*>(pMA)->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                METSetMix( eGDIRasterOp );
                WriteImageObject( Graphic( static_cast<const MetaBmpExScalePartAction*>(pMA)->GetBitmapEx() ).GetBitmap() );
            }
            break;

            case MetaActionType::EPS :
            {
                const MetaEPSAction* pA = static_cast<const MetaEPSAction*>(pMA);
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == MetaActionType::BMPSCALE )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pMetaAct);
                        METSetMix( eGDIRasterOp );
                        WriteImageObject( pBmpScaleAction->GetBitmap() );
                        break;
                    }
                }
            }
            break;
            default: break;
        }

        if (!bStatus)
            break;
    }

    if (pMET->GetError())
        bStatus=false;
}

void METWriter::WriteDataDescriptor(const GDIMetaFile *)
{
    if (!bStatus)
        return;

    WriteFieldIntroducer(0,DscGrfObjMagic,0,0);


    // The following is the OS2 original documentation and the associated implementation


    //  Parameters (all required and in this order)

    //  0         0xF7 Specify GVM Subset
    //  1         Length of following data 0x07
    //  2         0xB0 drawing order subset
    //  3-4       0x0000
    //  5         0x23 Level 3.2
    //  6         0x01 Version 1
    //  7         Length of following field 0x01
    //  8         Coordinate types in data
    //       0x04Intel16
    //       0x05Intel32
    pMET->WriteUChar( 0xf7 ).WriteUChar( 0x07 ).WriteUChar( 0xb0 ).WriteUChar( 0x00 )
         .WriteUChar( 0x00 ).WriteUChar( 0x23 ).WriteUChar( 0x01 ).WriteUChar( 0x01 )
         .WriteUChar( 0x05 );

    //  0         0xF6 Set Picture Descriptor
    //  1         Length of following data
    //  2         Flags
    //       0    B'0' Picture in 2D
    //       1    Picture Dimensions
    //            B'0'  Not absolute (PU_ARBITRARY PS)
    //            B'1'  Absolute (example: PU_TWIPS PS)
    //       2    Picture Elements
    //            B'0'  Not pels
    //            B'1'  Pels (PU_PELS PS)
    //                  (Bit 1 must also be set)
    //       3-7  B'00000'
    //  3         0x00 Reserved
    //  4         Picture frame size coordinate type
    //       0x04  Intel16
    //       0x05  Intel32
    //  5         UnitsOfMeasure
    //       0x00  Ten inches
    //       0x01  Decimeter
    //  6-11 or 6-17(2 or 4 bytes) Resolution.
    //       GPS Units / UOM on x axis
    //       GPS Units / UOM on y axis
    //       GPS Units / UOM on z axis
    //  12-23 or 18-41(2 or 4 bytes) Window Size.
    //       GPS X left, X right
    //       GPS Y bottom, Y top
    //       GPS Z near, Z far
    Size aUnitsPerDecimeter=OutputDevice::LogicToLogic(Size(10,10),MapMode(MAP_CM),aPictureMapMode);
    pMET->WriteUChar( 0xf6 ).WriteUChar( 0x28 ).WriteUChar( 0x40 ).WriteUChar( 0x00 )
         .WriteUChar( 0x05 ).WriteUChar( 0x01 )
         .WriteUInt32( aUnitsPerDecimeter.Width() )
         .WriteUInt32( aUnitsPerDecimeter.Height() )
         .WriteUInt32( 0 )
         .WriteUInt32( 0 ).WriteUInt32( aPictureRect.GetWidth() )
         .WriteUInt32( 0 ).WriteUInt32( aPictureRect.GetHeight() )
         .WriteUInt32( 0 ).WriteUInt32( 0 );

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set Default Parameter Format 0x08
    //  3-4       Mask 0xE000
    //  5         Names 0x8F
    //  6         Coordinates
    //       0x00  Picture in 2D
    //  7         Transforms
    //       0x04  Intel16
    //       0x05  Intel32
    //  8         Geometrics
    //       0x04  Intel16
    //       0x05  Intel32
    pMET->WriteUChar( 0x21 ).WriteUChar( 0x07 ).WriteUChar( 0x08 ).WriteUChar( 0xe0 )
         .WriteUChar( 0x00 ).WriteUChar( 0x8f ).WriteUChar( 0x00 ).WriteUChar( 0x05 )
         .WriteUChar( 0x05 );

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set default viewing transform 0x07
    //  3-4       Mask 0xCC0C
    //  5         Names 0x8F
    //  6-n       M11, M12, M21, M22, M41, M42   Matrix elements
    pMET->WriteUChar( 0x21 ).WriteUChar( 0x1c ).WriteUChar( 0x07 ).WriteUChar( 0xcc )
         .WriteUChar( 0x0c ).WriteUChar( 0x8f )
         .WriteUInt32( 0x00010000 ).WriteUInt32( 0x00000000 ).WriteUInt32( 0x00000000 )
         .WriteUInt32( 0x00010000 ).WriteUInt32( 0x00000000 ).WriteUInt32( 0x00000000 );

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set default line attributes 0x01
    //  3-4       Mask - OR of as many of the following bits as are required:
    //       0x8000  Line type
    //       0x4000  Line width
    //       0x2000  Line end
    //       0x1000  Line join
    //       0x0800  Stroke width
    //       0x0008  Line color
    //       0x0002  Line mix
    //  5         Flags
    //
    //       0x0F Set indicated default attributes to initial values. (Data field is not present in this
    //             instance).
    //       0x8F Set indicated default attributes to specified values.
    //  6-n       Data - data values as required, in the following order if present.
    //            No space is reserved for attributes for which the corresponding mask flag was not
    //            set.
    //
    //       (1 byte)  - Line type
    //       (1 byte)  - Line width
    //       (1 byte)  - Line end
    //       (1 byte)  - Line join
    //       (G bytes) - Stroke width
    //       (4 bytes) - Line color
    //       (1 byte)  - Line mix (G=2 or 4 depending on the Geometrics parameter of Set Default
    //            Parameter Format)
    // Nanu! witziger-weise fehlt obiger Abschnitt in den Metadateien. Also lassen wir ihn auch weg

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set Default Character Attributes 0x02
    //  3-4       Mask - OR of as many of the following bits as are required:
    //
    //       0x8000  Character angle
    //       0x4000  Character box
    //       0x2000  Character direction
    //       0x1000  Character precision
    //       0x0800  Character set
    //       0x0400  Character shear
    //       0x0040  Character break extra
    //       0x0020  Character extra
    //       0x0008  Character color
    //       0x0004  Character background color
    //       0x0002  Character mix
    //       0x0001  Character background mix
    //  5         Flags
    //       0x0FSet indicated default attributes to initial values.  (Data field is not present in this
    //            case).
    //       0x8FSet indicated default attributes to specified values.
    //  6-n       Data - data values as required, in the following order if present.
    //            No space is reserved for attributes for which the corresponding Mask flag was not
    //            set.
    //       (2*G bytes)     - Character angle
    //       (2*G + 4 bytes)- Character box
    //       (1 byte)        - Character direction
    //       (1 byte)        - Character precision
    //       (1 byte)        - Character set
    //       (2*G bytes)     - Character shear
    //       (4 bytes)       - Character break extra
    //       (4 bytes)       - Character extra
    //       (4 bytes)       - Character color
    //       (4 bytes)       - Character background color
    //       (1 byte)        - Character mix
    //       (1 byte)        - Character background mix (G=2 or 4 depending on the Geometrics
    //            parameter of Set Default Parameter Format)
    pMET->WriteUChar( 0x21 ).WriteUChar( 0x10 ).WriteUChar( 0x02 ).WriteUChar( 0x40 )
         .WriteUChar( 0x00 ).WriteUChar( 0x8f )
         .WriteUChar( 0xaa ).WriteUChar( 0x02 ).WriteUChar( 0x00 ).WriteUChar( 0x00 )
         .WriteUChar( 0x44 ).WriteUChar( 0x04 ).WriteUChar( 0x00 ).WriteUChar( 0x00 )
         .WriteUChar( 0xa8 ).WriteUChar( 0xaa ).WriteUChar( 0x40 ).WriteUChar( 0x44 );

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set Default Marker Attributes 0x03
    //  3-4       Mask - OR of as many of the following bits as are required:
    //       0x4000  Marker box
    //       0x1000  Marker precision
    //       0x0800  Marker set
    //       0x0100  Marker symbol
    //       0x0008  Marker color
    //       0x0004  Marker background color
    //       0x0002  Marker mix
    //       0x0001  Marker background mix
    //  5         Flags
    //       0x0F  Set indicated default attributes to initial values.
    //             (Data field is not present in this instance)
    //       0x8F  Set indicated default attributes to specified values.
    //  6-n       Data - data values as required, in this order if present.
    //            No space is reserved for attributes for which the corresponding Mask flag was not
    //            set.
    //       (2*G bytes)    - Marker box
    //       (1 byte)       - Marker precision
    //       (1 byte)       - Marker set
    //       (1 byte)       - Marker symbol
    //       (4 bytes)      - Marker color
    //       (4 bytes)      - Marker background color
    //       (1 byte)       - Marker mix
    //       (1 byte)       - Marker background mix (G=2 or 4 depending on the Geometrics
    //            parameter of Set Default Parameter Format)
    pMET->WriteUChar( 0x21 ).WriteUChar( 0x0c ).WriteUChar( 0x03 ).WriteUChar( 0x40 )
         .WriteUChar( 0x00 ).WriteUChar( 0x8f )
         .WriteUChar( 0x66 ).WriteUChar( 0x02 ).WriteUChar( 0x00 ).WriteUChar( 0x00 )
         .WriteUChar( 0x66 ).WriteUChar( 0x02 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set Default Pattern Attributes 0x04
    //  3-4       Mask - OR of as many of the following bits as are required:
    //       0x0800  Pattern set
    //       0x0100  Pattern symbol
    //       0x0080  Pattern reference point
    //       0x0008  Pattern color
    //       0x0004  Pattern background color
    //       0x0002  Pattern mix
    //       0x0001  Pattern background mix
    //       5       Flags
    //
    //            0x0F Set indicated default attributes to initial values.
    //                  (Data field is not present in this instance)
    //            0x8F Set indicated default attributes to specified values.
    //       6-n     Data - data values as required, in this order if present.
    //               No space is reserved for attributes for which the corresponding Mask flag was
    //               not set.
    //
    //            (1 byte)     - Pattern set
    //            (1 byte)     - Pattern symbol
    //            (2*G bytes)  - Pattern reference point
    //            (4 bytes)    - Pattern color
    //            (4 bytes)    - Pattern background color
    //            (1 byte)     - Pattern mix
    //            (1 byte)     - Pattern background mix (G=2 or 4 depending on the Geometrics
    //               parameter of Set Default Parameter Format)
    //       0       0x21 Set Current Defaults
    //       1       Length of following data
    //       2       Set Default Image Attributes 0x06
    //       3-4     Mask - OR of as many of these bits as are required:
    //            0x0008  Image color
    //            0x0004  Image background color
    //            0x0002  Image mix
    //            0x0001  Image background mix
    //       5       Flags
    //            0x0F Set indicated default attributes to initial values. (Data field is not present in
    //                  this instance)
    //            0x8F Set indicated default attributes to specified values.
    //       6-n     Data - data values as required, in this order if present.
    //               No space is reserved for attributes for which the corresponding Mask flag was
    //               not set.
    //            (4 bytes)  - Image color
    //            (4 bytes)  - Image background color
    //            (1 byte)   - Image mix
    //            (1 byte)   - Image background mix
    //       0       0x21 Set Current Defaults
    //       1       Length of following data
    //       2       Set Default Viewing Window 0x05
    //       3-4     Mask - OR of as many of the following bits as are required:
    //            0x8000  x left limit
    //            0x4000  x right limit
    //            0x2000  y bottom limit
    //            0x1000  y top limit
    //       5       Flags
    //            0x0F Set indicated default attributes to initial values.
    //                  (Data field is not present in this case).
    //            0x8F Set indicated default attributes to specified values.
    //       6-n     Data - data values as required, in the following order if present.
    //               No space is reserved for attributes for which the corresponding Mask flag was
    //               not set.
    //            (2*G bytes)  - x left limit
    //            (2*G bytes)  - x right limit
    //            (2*G bytes)  - y bottom limit
    //            (2*G bytes)  - y top limit (G=2 or 4 depending on the Geometrics parameter of Set
    //               Default Parameter Format)
    //       0       0x21 Set Current Defaults
    //       1       Length of following data
    //       2       Set Default Arc Parameters 0x0B
    //       3-4     Mask - OR of as many of the following bits as are required:
    //            0x8000  P value
    //            0x4000  Q value
    //            0x2000  R value
    //            0x1000  S value
    //       5       Flags
    //            0x0F Set indicated default attributes to initial values.
    //                  (Data field is not present in this case).
    //            0x8F Set indicated default attributes to specified values.
    //       6-n     Data - data values as required, in the following order if present.
    //               No space is reserved for attributes for which the corresponding Mask flag was
    //               not set.
    //            (G bytes)  - P value
    //            (G bytes)  - Q value
    //            (G bytes)  - R value
    //            (G bytes)  - S value (G=2 or 4 depending on the Geometrics parameter of Set
    //               Default Parameter Format)
    //       0       0x21 Set Current Defaults
    //       1       Length of following data
    //       2       Set Default Pick Identifier 0x0C
    //       3-4     Mask - OR of as many of the following bits as are required:
    //            0x8000  Pick identifier
    //       5       Flags
    //            0x0F Set indicated default attributes to initial values.
    //                  (Data field is not present in this case).
    //            0x8F Set indicated default attributes to specified values.
    //       6-n     Data - data values as required, in the following order if present.
    //               No space is reserved for attributes for which the corresponding Mask flag was
    //               not set.
    //            (4 bytes)  - Pick identifier

    //       0       0xE7 Set Bit-map Identifier
    //       1       Length of following data 0x07
    //       2-3     Usage Flags 0x8000
    //       4-7     Bit-map handle
    //       8       Lcid
    if (nNumberOfBitmaps>0) {
        pMET->WriteUChar( 0xe7 ).WriteUChar( 0x07 ).WriteUChar( 0x80 ).WriteUChar( 0x00 );
        WriteBigEndianLong(nActBitmapId);
        pMET->WriteUChar( 0xfe );
    }

    UpdateFieldSize();

    if (pMET->GetError()) bStatus=false;
}


void METWriter::WillWriteOrder(sal_uInt32 nNextOrderMaximumLength)
{
    // The parameters of a 'Graphics Data Fields' can be (according to OS2
    // documentation) at most 32759 bytes long. Meant by this is the size
    // of the field minus the 'Structured Field Introducer' (size 8).
    // So the size of the whole field can be at most 8+32759=32767=0x7fff.
    // To be on the safe side whe use 30000 as the limit.
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
    pMET->WriteUChar( 0xd6 ).WriteUChar( 44 ).WriteUInt16( 0 ).WriteUInt16( 0x00cc );
    WriteBigEndianLong(nActBitmapId++);
    pMET->WriteUChar( 0x02 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
    WritePoint(Point(aPt.X(),aPt.Y()+aSize.Height()));
    WritePoint(Point(aPt.X()+aSize.Width(),aPt.Y()));
    pMET->WriteUInt32( 0 ).WriteUInt32( 0 )
         .WriteUInt32( rBmpSizePixel.Width() )
         .WriteUInt32( rBmpSizePixel.Height() );
}

void METWriter::METSetAndPushLineInfo( const LineInfo& rLineInfo )
{
    sal_Int32 nWidth = OutputDevice::LogicToLogic( Size( rLineInfo.GetWidth(),0 ), aPictureMapMode, aTargetMapMode ).Width();

    WillWriteOrder( 8 );            // set stroke linewidth
    pMET  ->WriteUChar( 0x15 )
           .WriteUChar( 6 )
           .WriteUChar( 0 )             // Flags
           .WriteUChar( 0 )
           .WriteInt32( nWidth );

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        sal_uInt8 nStyle = 0;           // LineDefault;

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
                        nStyle = 1; // LINE_DOT
                    else
                        nStyle = 3; // LINE_DASH_DOT
                }
                else
                    nStyle = 2;     // LINE_DASH
            }
            break;
            case LineStyle_SOLID:
            case LineStyle_FORCE_EQUAL_SIZE:
                break;  // not handled -Wall
        }
        WillWriteOrder( 2 );
        pMET->WriteUChar( 0x18 ).WriteUChar( nStyle );     // set LineType
    }
}

void METWriter::METPopLineInfo( const LineInfo& rLineInfo )
{
    WillWriteOrder( 8 );            // set stroke linewidth
    pMET  ->WriteUChar( 0x15 )
           .WriteUChar( 6 )
           .WriteUChar( 0 )             // Flags
           .WriteUChar( 0 )
           .WriteUInt32( 1 );

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        WillWriteOrder( 2 );
        pMET->WriteUChar( 0x18 ).WriteUChar( 0 );       // set LineType
    }
}

void METWriter::METBeginArea(bool bBoundaryLine)
{
    WillWriteOrder(2);
    pMET->WriteUChar( 0x68 );
    if (bBoundaryLine) pMET->WriteUChar( 0xc0 );
    else               pMET->WriteUChar( 0x80 );
}


void METWriter::METEndArea()
{
    WillWriteOrder(2);
    pMET->WriteUChar( 0x60 ).WriteUChar( 0 );
}


void METWriter::METBeginPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( 0xd0 ).WriteUChar( 6 ).WriteUInt16( 0 ).WriteUInt32( nPathId );
}


void METWriter::METEndPath()
{
    WillWriteOrder(2);
    pMET->WriteUChar( 0x7f ).WriteUChar( 0 );
}


void METWriter::METFillPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( 0xd7 ).WriteUChar( 6 )
         .WriteUChar( 0x00 ).WriteUChar( 0 ).WriteUInt32( nPathId );
}


void METWriter::METOutlinePath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    pMET->WriteUChar( 0xd4 ).WriteUChar( 6 )
         .WriteUChar( 0 ).WriteUChar( 0 ).WriteUInt32( nPathId );
}


void METWriter::METCloseFigure()
{
    WillWriteOrder(2);
    pMET->WriteUChar( 0x7d ).WriteUChar( 0 );
}


void METWriter::METMove(Point aPt)
{
    WillWriteOrder(10);
    pMET->WriteUChar( 0x21 ).WriteUChar( 8 );
    WritePoint(aPt);
}


void METWriter::METLine(Point aPt1, Point aPt2)
{
    WillWriteOrder(18);
    pMET->WriteUChar( 0xc1 ).WriteUChar( 16 );
    WritePoint(aPt1); WritePoint(aPt2);
}


void METWriter::METLine(const tools::Polygon & rPolygon)
{
    sal_uInt16 nNumPoints,i,j,nOrderPoints;
    bool bFirstOrder;

    bFirstOrder=true;
    i=0; nNumPoints=rPolygon.GetSize();
    while (i<nNumPoints) {
        nOrderPoints=nNumPoints-i;
        if (nOrderPoints>30) nOrderPoints=30;
        WillWriteOrder(nOrderPoints*8+2);
        if (bFirstOrder) {
            pMET->WriteUChar( 0xc1 ); // Line at given pos
            bFirstOrder=false;
        }
        else {
            pMET->WriteUChar( 0x81 ); // Line at current pos
        }
        pMET->WriteUChar( nOrderPoints*8 );
        for (j=0; j<nOrderPoints; j++) WritePoint(rPolygon.GetPoint(i++));
    }
}


void METWriter::METLine(const tools::PolyPolygon & rPolyPolygon)
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
    pMET->WriteUChar( 0x81 ).WriteUChar( 8 );
    WritePoint(aPt);
}

void METWriter::METBox(bool bFill, bool bBoundary,
                       const Rectangle& rRect, sal_uInt32 nHAxis, sal_uInt32 nVAxis)
{
    sal_uInt8 nFlags=0;
    if (bFill)     nFlags|=0x40;
    if (bBoundary) nFlags|=0x20;

    WillWriteOrder(28);
    pMET->WriteUChar( 0xc0 ).WriteUChar( 26 ).WriteUChar( nFlags ).WriteUChar( 0 );
    WritePoint(rRect.BottomLeft());
    WritePoint(rRect.TopRight());
    pMET->WriteUInt32( nHAxis ).WriteUInt32( nVAxis );
}

void METWriter::METFullArc(Point aCenter, double fMultiplier)
{
    WillWriteOrder(14);
    pMET->WriteUChar( 0xc7 ).WriteUChar( 12 );
    WritePoint(aCenter);
    pMET->WriteInt32( fMultiplier*65536.0+0.5 );
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
    pMET->WriteUChar( 0xa3 ).WriteUChar( 20 );
    WritePoint(aCenter);
    pMET->WriteInt32( fMultiplier*65536.0+0.5 );
    pMET->WriteInt32( fStartAngle*65536.0+0.5 );
    pMET->WriteInt32( fSweepAngle*65536.0+0.5 );
}


void METWriter::METChrStr( Point aPt, const OUString& aUniStr )
{
    OString aStr(OUStringToOString(aUniStr,
        osl_getThreadTextEncoding()));
    sal_uInt16 nLen = aStr.getLength();
    WillWriteOrder( 11 + nLen );
    pMET->WriteUChar( 0xc3 ).WriteUChar( 9 + nLen );
    WritePoint(aPt);
    for (sal_uInt16 i = 0; i < nLen; ++i)
        pMET->WriteChar( aStr[i] );
    pMET->WriteUChar( 0 );
}


void METWriter::METSetArcParams(sal_Int32 nP, sal_Int32 nQ, sal_Int32 nR, sal_Int32 nS)
{
    WillWriteOrder(18);
    pMET->WriteUChar( 0x22 ).WriteUChar( 16 ).WriteInt32( nP ).WriteInt32( nQ ).WriteInt32( nR ).WriteInt32( nS );
}


void METWriter::METSetColor(Color aColor)
{
    if (aColor==aMETColor) return;
    aMETColor=aColor;

    WillWriteOrder(6);
    pMET->WriteUChar( 0xa6 ).WriteUChar( 4 ).WriteUChar( 0 )
         .WriteUChar( aColor.GetBlue() )
         .WriteUChar( aColor.GetGreen() )
         .WriteUChar( aColor.GetRed() );
}


void METWriter::METSetBackgroundColor(Color aColor)
{
    if (aColor==aMETBackgroundColor) return;
    aMETBackgroundColor=aColor;

    WillWriteOrder(6);
    pMET->WriteUChar( 0xa7 ).WriteUChar( 4 ).WriteUChar( 0 )
         .WriteUChar( aColor.GetBlue() )
         .WriteUChar( aColor.GetGreen() )
         .WriteUChar( aColor.GetRed() );
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
    pMET->WriteUChar( 0x0c ).WriteUChar( nMix );
}


void METWriter::METSetChrCellSize(Size aSize)
{
    if (aMETChrCellSize==aSize)
        return;

    aMETChrCellSize=aSize;
    WillWriteOrder(10);
    if (aSize.Width()==0) aSize.Width()=aSize.Height();
    pMET->WriteUChar( 0x33 ).WriteUChar( 8 ).WriteInt32( aSize.Width() ).WriteInt32( aSize.Height() );
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
    pMET->WriteUChar( 0x34 ).WriteUChar( 8 ).WriteInt32( nax ).WriteInt32( nay );
}


void METWriter::METSetChrSet(sal_uInt8 nSet)
{
    if (nMETChrSet==nSet)
        return;

    nMETChrSet=nSet;
    WillWriteOrder(2);
    pMET->WriteUChar( 0x38 ).WriteUChar( nSet );
}


void METWriter::WriteOrders( const GDIMetaFile* pMTF )
{
    if(!bStatus)
        return;

    for( size_t nA = 0, nACount = pMTF->GetActionSize(); nA < nACount; nA++ )
    {
        const MetaAction* pMA = pMTF->GetAction( nA );

        switch (pMA->GetType())
        {
            case MetaActionType::PIXEL:
            {
                const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pMA);
                METSetMix( eGDIRasterOp );
                METSetColor( pA->GetColor() );
                METLine( pA->GetPoint(),pA->GetPoint() );
            }
            break;

            case MetaActionType::POINT:
            {
                const MetaPointAction* pA = static_cast<const MetaPointAction*>(pMA);

                METSetArcParams(1,1,0,0);
                METSetMix(eGDIRasterOp);
                METSetColor(aGDILineColor);
                METBeginArea(false);
                METFullArc(pA->GetPoint(),0.5);
                METEndArea();
            }
            break;

            case MetaActionType::LINE:
            {
                const MetaLineAction* pA = static_cast<const MetaLineAction*>(pMA);

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

            case MetaActionType::RECT:
            {
                const MetaRectAction* pA = static_cast<const MetaRectAction*>(pMA);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBox( true, false, pA->GetRect(), 0, 0 );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( false, true, pA->GetRect(), 0, 0 );
                }
            }
            break;

            case MetaActionType::ROUNDRECT:
            {
                const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pMA);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBox( true, false, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( false, true, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
            }
            break;

            case MetaActionType::ELLIPSE:
            {
                const MetaEllipseAction*    pA = static_cast<const MetaEllipseAction*>(pMA);
                Point                       aCenter;

                aCenter.X()=(pA->GetRect().Left()+pA->GetRect().Right())/2;
                aCenter.Y()=(pA->GetRect().Top()+pA->GetRect().Bottom())/2;

                METSetArcParams(pA->GetRect().GetWidth(), pA->GetRect().GetHeight(),0,0);

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDIFillColor );
                    METSetBackgroundColor( aGDIFillColor );
                    METBeginArea(false);
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

            case MetaActionType::ARC:
            {
                const MetaArcAction*    pA = static_cast<const MetaArcAction*>(pMA);
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

            case MetaActionType::PIE:
            {
                const MetaPieAction*    pA = static_cast<const MetaPieAction*>(pMA);
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

            case MetaActionType::CHORD:
            {
                const MetaChordAction*  pA = static_cast<const MetaChordAction*>(pMA);
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

            case MetaActionType::POLYLINE:
            {
                const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pMA);

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    LineInfo aLineInfo( pA->GetLineInfo() );
                    if ( ! ( aLineInfo.IsDefault() ) )
                        METSetAndPushLineInfo( aLineInfo );

                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    tools::Polygon aSimplePoly;
                    const tools::Polygon& rPoly = pA->GetPolygon();
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

            case MetaActionType::POLYGON:
            {
                const MetaPolygonAction* pA = static_cast<const MetaPolygonAction*>(pMA);
                tools::Polygon aSimplePoly;
                const tools::Polygon& rPoly = pA->GetPolygon();
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

            case MetaActionType::POLYPOLYGON:
            {
                const MetaPolyPolygonAction* pA = static_cast<const MetaPolyPolygonAction*>(pMA);

                tools::PolyPolygon aSimplePolyPoly( pA->GetPolyPolygon() );
                sal_uInt16 i, nCount = aSimplePolyPoly.Count();
                for ( i = 0; i < nCount; i++ )
                {
                    if ( aSimplePolyPoly[ i ].HasFlags() )
                    {
                        tools::Polygon aSimplePoly;
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

            case MetaActionType::TEXT:
            {
                const MetaTextAction*   pA = static_cast<const MetaTextAction*>(pMA);
                Point                   aPt( pA->GetPoint() );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE)
                {
                    ScopedVclPtrInstance< VirtualDevice > pVDev;

                    if( aGDIFont.GetAlign()==ALIGN_TOP )
                        aPt.Y()+=(long)pVDev->GetFontMetric( aGDIFont ).GetAscent();
                    else
                        aPt.Y()-=(long)pVDev->GetFontMetric( aGDIFont ).GetDescent();
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

            case MetaActionType::TEXTARRAY:
            {
                const MetaTextArrayAction*  pA = static_cast<const MetaTextArrayAction*>(pMA);
                sal_uInt16                  i;
                OUString                    aStr;
                tools::Polygon aPolyDummy(1);
                short                       nOrientation;
                Point                       aPt( pA->GetPoint() );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE )
                {
                    ScopedVclPtrInstance< VirtualDevice > pVDev;
                    if( aGDIFont.GetAlign() == ALIGN_TOP )
                        aPt.Y()+=(long)pVDev->GetFontMetric(aGDIFont).GetAscent();
                    else
                        aPt.Y()-=(long)pVDev->GetFontMetric(aGDIFont).GetDescent();
                }

                METSetMix(eGDIRasterOp);
                METSetColor(aGDIFont.GetColor());
                METSetBackgroundColor(aGDIFont.GetFillColor());
                METSetChrCellSize(aGDIFont.GetSize());
                METSetChrAngle( nOrientation = aGDIFont.GetOrientation() );
                METSetChrSet(FindChrSet(aGDIFont));
                aStr = pA->GetText().copy(pA->GetIndex(),pA->GetLen());

                if( pA->GetDXArray()!=nullptr )
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

            case MetaActionType::STRETCHTEXT:
            {
                const MetaStretchTextAction*    pA = static_cast<const MetaStretchTextAction*>(pMA);
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                sal_uInt16                          i;
                sal_Int32                       nNormSize;
                OUString                        aStr;
                tools::Polygon aPolyDummy(1);
                short                           nOrientation;
                Point                           aPt( pA->GetPoint() );
                Point                           aPt2;

                pVDev->SetFont( aGDIFont );

                if( aGDIFont.GetAlign() != ALIGN_BASELINE)
                {
                    if( aGDIFont.GetAlign() == ALIGN_TOP )
                        aPt.Y()+=(long)pVDev->GetFontMetric().GetAscent();
                    else
                        aPt.Y()-=(long)pVDev->GetFontMetric().GetDescent();
                }

                METSetMix(eGDIRasterOp);
                METSetColor(aGDIFont.GetColor());
                METSetBackgroundColor(aGDIFont.GetFillColor());
                METSetChrCellSize(aGDIFont.GetSize());
                METSetChrAngle( nOrientation = aGDIFont.GetOrientation() );
                METSetChrSet(FindChrSet(aGDIFont));
                aStr = pA->GetText().copy(pA->GetIndex(),pA->GetLen());
                std::unique_ptr<long[]> pDXAry(new long[aStr.getLength()]);
                nNormSize = pVDev->GetTextArray( aStr, pDXAry.get() );

                for ( i = 0; i < aStr.getLength(); i++ )
                {
                    aPt2 = aPt;
                    if (i > 0 && nNormSize)
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
            }
            break;

            case MetaActionType::TEXTRECT:
            {
//              OSL_FAIL( "Unsupported MET-Action: MetaActionType::TEXTRECT!" );
            }
            break;

            case MetaActionType::BMP:
            {
                const MetaBmpAction*    pA = static_cast<const MetaBmpAction*>(pMA);
                const Size              aSizePixel( pA->GetBitmap().GetSizePixel() );

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pCompDev->PixelToLogic( aSizePixel, aPictureMapMode ), aSizePixel );
            }
            break;

            case MetaActionType::BMPSCALE:
            {
                const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pMA);

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pA->GetSize(), pA->GetBitmap().GetSizePixel() );
            }
            break;

            case MetaActionType::BMPSCALEPART:
            {
                const MetaBmpScalePartAction*   pA = static_cast<const MetaBmpScalePartAction*>(pMA);
                Bitmap                          aTmp( pA->GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), pA->GetBitmap().GetSizePixel() );
            }
            break;

            case MetaActionType::BMPEX:
            {
                const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pMA);
                const Size              aSizePixel( pA->GetBitmapEx().GetSizePixel() );

                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetPoint(), pCompDev->PixelToLogic( aSizePixel, aPictureMapMode ), aSizePixel );
            }
            break;

            case MetaActionType::BMPEXSCALE:
            {
                const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pMA);
                const Size                  aSizePixel( pA->GetBitmapEx().GetSizePixel() );

                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetPoint(), pA->GetSize(), aSizePixel );
            }
            break;

            case MetaActionType::BMPEXSCALEPART:
            {
                const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pMA);
                Bitmap                          aTmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), aTmp.GetSizePixel() );
            }
            break;

            case MetaActionType::EPS :
            {
                const MetaEPSAction* pA = static_cast<const MetaEPSAction*>(pMA);
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                size_t nCount = aGDIMetaFile.GetActionSize();
                for ( size_t i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == MetaActionType::BMPSCALE )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pMetaAct);
                        METSetMix(eGDIRasterOp);
                        METBitBlt( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap().GetSizePixel() );
                        break;
                    }
                }
            }
            break;

            case MetaActionType::MASK:
            break;

            case MetaActionType::MASKSCALE:
            break;

            case MetaActionType::MASKSCALEPART:
            break;

            case MetaActionType::GRADIENT:
            {
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                GDIMetaFile                 aTmpMtf;
                const MetaGradientAction*   pA = static_cast<const MetaGradientAction*>(pMA);

                pVDev->SetMapMode( aTargetMapMode );
                pVDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                WriteOrders( &aTmpMtf );
            }
            break;

            case MetaActionType::HATCH:
            {
                ScopedVclPtrInstance< VirtualDevice > pVDev;
                GDIMetaFile             aTmpMtf;
                const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pMA);

                pVDev->SetMapMode( aTargetMapMode );
                pVDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                WriteOrders( &aTmpMtf );
            }
            break;

            case MetaActionType::WALLPAPER:
            break;

            case MetaActionType::CLIPREGION:
            break;

            case MetaActionType::ISECTRECTCLIPREGION:
            {
                const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pMA);
                WriteClipRect( pA->GetRect() );
            }
            break;

            case MetaActionType::ISECTREGIONCLIPREGION:
            break;

            case MetaActionType::MOVECLIPREGION:
            break;

            case MetaActionType::LINECOLOR:
            {
                const MetaLineColorAction* pA = static_cast<const MetaLineColorAction*>(pMA);

                if( pA->IsSetting() )
                    aGDILineColor = pA->GetColor();
                else
                    aGDILineColor = Color( COL_TRANSPARENT );
            }
            break;

            case MetaActionType::FILLCOLOR:
            {
                const MetaFillColorAction* pA = static_cast<const MetaFillColorAction*>(pMA);

                if( pA->IsSetting() )
                    aGDIFillColor = pA->GetColor();
                else
                    aGDIFillColor = Color( COL_TRANSPARENT );
            }
            break;

            case MetaActionType::TEXTCOLOR:
            {
                const MetaTextColorAction* pA = static_cast<const MetaTextColorAction*>(pMA);
                aGDIFont.SetColor( pA->GetColor() );
            }
            break;

            case MetaActionType::TEXTFILLCOLOR:
            {
                const MetaTextFillColorAction* pA = static_cast<const MetaTextFillColorAction*>(pMA);

                if( pA->IsSetting() )
                    aGDIFont.SetFillColor( pA->GetColor() );
                else
                    aGDIFont.SetFillColor( Color( COL_TRANSPARENT ) );
            }
            break;

            case MetaActionType::TEXTALIGN:
            break;

            case MetaActionType::MAPMODE:
            {
                const MetaMapModeAction* pA = static_cast<const MetaMapModeAction*>(pMA);

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

            case MetaActionType::FONT:
            {
                aGDIFont = static_cast<const MetaFontAction*>(pMA)->GetFont();
            }
            break;

            case MetaActionType::PUSH:
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

            case MetaActionType::POP:
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

            case MetaActionType::RASTEROP:
            {
                eGDIRasterOp = static_cast<const MetaRasterOpAction*>(pMA)->GetRasterOp();
            }
            break;

            case MetaActionType::Transparent:
            {
                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor);
                    METSetBackgroundColor(aGDIFillColor);
                    METBeginPath(1);
                    METLine( static_cast<const MetaTransparentAction*>(pMA)->GetPolyPolygon());
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    METLine( static_cast<const MetaTransparentAction*>(pMA)->GetPolyPolygon());
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case MetaActionType::FLOATTRANSPARENT:
            {
                const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pMA);

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
            default: break;
      }

      nWrittenActions++;
      MayCallback();

      if( pMET->GetError() )
        bStatus=false;

      if( !bStatus )
        break;
    }
}

void METWriter::WriteObjectEnvironmentGroup(const GDIMetaFile * pMTF)
{
    sal_uInt32 i, nId;

    //--- The Field 'Begin Object Environment Group':
    WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
    WriteFieldId(7);

    //--- The Field 'Map Color Attribute Table':
    WriteFieldIntroducer(22,MapColAtrMagic,0,0);
    WriteBigEndianShort(0x000e);
    pMET->WriteUChar( 0x0c ).WriteUChar( 0x02 ).WriteUChar( 0x84 ).WriteUChar( 0x00 );
    WriteFieldId(4);

    //--- The first Field 'Map Coded Font':
    WriteFieldIntroducer(32,MapCodFntMagic,0,0);
    WriteBigEndianShort(0x0018);
    pMET->WriteUChar( 0x0c ).WriteUChar( 0x02 ).WriteUChar( 0x84 ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0xff ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0x04 ).WriteUChar( 0x24 ).WriteUChar( 0x05 ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0x06 ).WriteUChar( 0x20 );
    pMET->WriteUChar( 0x03 ).WriteUChar( 0x97 ).WriteUChar( 0x01 ).WriteUChar( 0xb5 );

    //--- The additional Fields 'Map Coded Font':
    CreateChrSets(pMTF);
    WriteChrSets();

    //--- The Fields 'Map Data Resource':
    nId=nActBitmapId;
    for (i=0; i<nNumberOfBitmaps; i++)
    {
        WriteFieldIntroducer(29,MapDatResMagic,0,0);
        WriteBigEndianShort(0x0015);
        pMET->WriteUChar( 0x0c ).WriteUChar( 0x02 ).WriteUChar( 0x84 ).WriteUChar( 0x00 );
        WriteFieldId(nId);
        pMET->WriteUChar( 0x07 ).WriteUChar( 0x22 ).WriteUChar( 0x10 );
        pMET->WriteUInt32( nId );
        nId++;
    }

    //--- Das Feld 'End Object Environment Group':
    WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
    WriteFieldId(7);
}


void METWriter::WriteGraphicsObject(const GDIMetaFile * pMTF)
{
    sal_uInt32 nSegmentSize,nPos,nDataFieldsStartPos;

    if( !bStatus )
        return;

    //--- Das Feld 'Begin Graphics Object':
    WriteFieldIntroducer(16,BegGrfObjMagic,0,0);
    WriteFieldId(7);

    // Map Color Attribute Table, Fonts and other stuff:
    WriteObjectEnvironmentGroup(pMTF);

    //--- The Field 'Graphics Data Descriptor':
    WriteDataDescriptor(pMTF);

    // initialise the counter for Data Fields:
    nNumberOfDataFields=0;

    // and remember the position of the first Data Field:
    nDataFieldsStartPos=pMET->Tell();

    //--- start of the first Field 'Graphics Data'
    WriteFieldIntroducer(0,DatGrfObjMagic,0,0);
    nNumberOfDataFields++;

    // now at first we write the head of the segment:
    pMET->WriteUChar( 0x70 ).WriteUChar( 0x0e ).WriteUInt32( 0 );
    pMET->WriteUChar( 0x70 ).WriteUChar( 0x10 ); // Flags
    pMET->WriteUInt16( 0 ); // Lo-Word of the length of the segment data  (Big Endian)
    pMET->WriteUInt32( 0 );  // Reserved
    pMET->WriteUInt16( 0 ); // Hi-Word of the length of the segment (Big Endian) (Ohh Ohh OS2)
    // Annotation: we're writing the correct data length again below

    // now all orders are being written out:
    // (wobei die Sache ggf. in mehrere 'Graphics Data Fields' aufgeteilt
    // wird, per Methode WillWriteOrder(..))
    WriteOrders(pMTF);

    //--- terminate the last Field 'Graphic Data':
    UpdateFieldSize();

    //--- and finally correct the segment size:
    nPos=pMET->Tell();
    nSegmentSize=nPos-nDataFieldsStartPos;
    nSegmentSize-=nNumberOfDataFields*8; // Structured Field Introducers are not counted
    pMET->Seek(nDataFieldsStartPos+16); // seek to the Lo-Word of the segment size
    WriteBigEndianShort((sal_uInt16)(nSegmentSize&0x0000ffff)); // Und schreiben
    pMET->Seek(nDataFieldsStartPos+22); // seek to the Hi-Word of the segment size
    WriteBigEndianShort((sal_uInt16)(nSegmentSize>>16)); // and writing it
    pMET->Seek(nPos); // back to business as usual

    //--- The Field 'End Graphic Objects':
    WriteFieldIntroducer(16,EndGrfObjMagic,0,0);
    WriteFieldId(7);

    if( pMET->GetError() )
        bStatus=false;
}


void METWriter::WriteResourceGroup(const GDIMetaFile * pMTF)
{
    if( !bStatus )
        return;

    //--- The Field 'Begin Resource Group':
    WriteFieldIntroducer(16,BegResGrpMagic,0,0);
    WriteFieldId(2);

    //--- The Content:
    WriteColorAttributeTable();
    nActBitmapId=0x77777700;
    WriteImageObjects(pMTF);
    nActBitmapId=0x77777700;
    WriteGraphicsObject(pMTF);

    //--- The Field 'End Resource Group':
    WriteFieldIntroducer(16,EndResGrpMagic,0,0);
    WriteFieldId(2);

    if( pMET->GetError() )
        bStatus=false;
}


void METWriter::WriteDocument(const GDIMetaFile * pMTF)
{
    if( !bStatus )
        return;

    //--- The Field 'Begin Document':
    WriteFieldIntroducer(0,BegDocumnMagic,0,0);
    WriteFieldId(1);
    pMET->WriteUChar( 0x00 ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0x05 ).WriteUChar( 0x18 ).WriteUChar( 0x03 ).WriteUChar( 0x0c ).WriteUChar( 0x00 );
    pMET->WriteUChar( 0x06 ).WriteUChar( 0x01 ).WriteUChar( 0x03 ).WriteUChar( 0xd4 ).WriteUChar( 0x03 ).WriteUChar( 0x52 );
    pMET->WriteUChar( 0x03 ).WriteUChar( 0x65 ).WriteUChar( 0x00 );
    UpdateFieldSize();

    //--- The Content:
    WriteResourceGroup(pMTF);

    //--- The Field 'End Document':
    WriteFieldIntroducer(16,EndDocumnMagic,0,0);
    WriteFieldId(1);

    if( pMET->GetError() )
        bStatus=false;
}

bool METWriter::WriteMET( const GDIMetaFile& rMTF, SvStream& rTargetStream, FilterConfigItem* pFilterConfigItem )
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

    bStatus=true;
    nLastPercent=0;

    pMET=&rTargetStream;
    pMET->SetEndian(SvStreamEndian::LITTLE);

    aPictureRect = Rectangle( Point(), rMTF.GetPrefSize() );
    aTargetMapMode = aPictureMapMode = rMTF.GetPrefMapMode();

    aGDILineColor=Color( COL_BLACK );
    aGDIFillColor=Color( COL_WHITE );
    eGDIRasterOp=ROP_OVERPAINT;
    aGDIFont=vcl::Font();
    aGDIMapMode=MapMode();
    aGDIClipRect=Rectangle();
    pGDIStack=nullptr;
    aMETColor=Color(COL_BLACK);
    aMETBackgroundColor=Color(COL_WHITE);
    eMETMix=ROP_OVERPAINT;
    nMETStrokeLineWidth=1;
    aMETChrCellSize=Size(0,0);
    nMETChrAngle=0;
    nMETChrSet=0x00;
    pChrSetList=nullptr;
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

//================== GraphicExport - the exported Function ================


extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL
emeGraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem )
{
    METWriter aMETWriter;

    // #119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
    GDIMetaFile aMetafile(rGraphic.GetGDIMetaFile());

    if(usesClipActions(aMetafile))
    {
        // #i121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against it's own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aMetafile);
    }

    return aMETWriter.WriteMET( aMetafile, rStream, pFilterConfigItem );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
