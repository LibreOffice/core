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
    String aName;
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
    sal_uLong               nLastPercent; // with which number pCallback has been called the last time
    SvStream*           pMET;
    Rectangle           aPictureRect;
    MapMode             aPictureMapMode;
    MapMode             aTargetMapMode;
    sal_uLong               nActualFieldStartPos;     // start position of the current 'Field'
    sal_uLong               nNumberOfDataFields;  // number of commenced 'Graphcis Data Fields'
    Color               aGDILineColor;
    Color               aGDIFillColor;
    RasterOp            eGDIRasterOp;
    Font                aGDIFont;
    MapMode             aGDIMapMode;   // currently ununsed!
    Rectangle           aGDIClipRect; // currently ununsed!
    METGDIStackMember*  pGDIStack;
    Color               aMETColor;
    Color               aMETBackgroundColor;
    Color               aMETPatternSymbol;
    RasterOp            eMETMix ;
    long                nMETStrokeLineWidth;
    Size                aMETChrCellSize;
    short               nMETChrAngle;
    sal_uInt8               nMETChrSet;
    METChrSet*          pChrSetList; // list of Character-Sets
    sal_uInt8               nNextChrSetId; // the first unused ChrSet-Id
    sal_uLong               nActBitmapId; // Field-Id of the next Bitmap
    sal_uLong               nNumberOfActions; // number of Actions in the GDIMetafile
    sal_uLong               nNumberOfBitmaps; // number of Bitmaps
    sal_uLong               nWrittenActions;  // number of already processed actions during the writing of the orders
    sal_uLong               nWrittenBitmaps;  // number of already written Bitmaps
    sal_uLong               nActBitmapPercent; // percentage of the next bitmap that's already written

    ::std::auto_ptr< VirtualDevice >    apDummyVDev;
    OutputDevice*                       pCompDev;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

    void MayCallback();
        // calculates a percentage based on the 5 parameters above and then does a
        // Callback as the case may be. Sets bStatus to sal_False if the user wants to cancel

    void CountActionsAndBitmaps(const GDIMetaFile * pMTF);
        // Counts the bitmaps and actions (nNumberOfActions and nNumberOfBitmaps have to
        // be set to 0 at the beginning, since this method is recursive)

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
    void METChrStr(Point aPt, String aStr);
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

    METWriter() :
        pMET(NULL), pGDIStack(NULL), nMETStrokeLineWidth(0), nMETChrAngle(0), pChrSetList(NULL), pCompDev(NULL)
    {
#ifndef NO_GETAPPWINDOW
        pCompDev = reinterpret_cast< OutputDevice* >( Application::GetAppWindow() );
#endif
        if( !pCompDev )
        {
            apDummyVDev.reset( new VirtualDevice );
            pCompDev = apDummyVDev.get();
        }
    }

    sal_Bool WriteMET( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                        FilterConfigItem* pConfigItem );
};


//========================== Methods of METWriter ==========================

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
    *pMET << (sal_uInt8)0xb4 << (sal_uInt8)6
          << (sal_uInt8)0x00 << (sal_uInt8)0 << nPathId;
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
    *pMET << ((sal_uInt8)(nWord>>8)) << ((sal_uInt8)(nWord&0x00ff));
}


void METWriter::WriteBigEndianLong(sal_uLong nLong)
{
    WriteBigEndianShort((sal_uInt16)(nLong>>16));
    WriteBigEndianShort((sal_uInt16)(nLong&0x0000ffff));
}


void METWriter::WritePoint(Point aPt)
{
    Point aNewPt = pCompDev->LogicToLogic( aPt, aPictureMapMode, aTargetMapMode );

    *pMET << (sal_Int32) ( aNewPt.X() - aPictureRect.Left() )
          << (sal_Int32) ( aPictureRect.Bottom() - aNewPt.Y() );
}


void METWriter::WriteFieldIntroducer(sal_uInt16 nFieldSize, sal_uInt16 nFieldType,
                                     sal_uInt8 nFlags, sal_uInt16 nSegSeqNum)
{
    nActualFieldStartPos=pMET->Tell();
    WriteBigEndianShort(nFieldSize);
    *pMET << (sal_uInt8)0xd3 << nFieldType << nFlags << nSegSeqNum;
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
        *pMET << nbyte;
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

        *pMET << (sal_uInt8)0x0c << (sal_uInt8)0x02 << (sal_uInt8)0x84 << (sal_uInt8)0x00;
        *pMET << (sal_uInt8)0xa4 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x01;
        *pMET << (sal_uInt8)0x01 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;

        *pMET << (sal_uInt8)0x04 << (sal_uInt8)0x24 << (sal_uInt8)0x05 << (sal_uInt8)pCS->nSet;

        *pMET << (sal_uInt8)0x14 << (sal_uInt8)0x1f;
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
        *pMET << nbyte;
        *pMET << (sal_uInt8)0x05;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x0c;

        *pMET << (sal_uInt8)0x06 << (sal_uInt8)0x20 << (sal_uInt8)0x03 << (sal_uInt8)0xd4;
        *pMET << (sal_uInt8)0x03 << (sal_uInt8)0x52;

        *pMET << (sal_uInt8)0x24 << (sal_uInt8)0x02 << (sal_uInt8)0x08 << (sal_uInt8)0x00;
        OString n(OUStringToOString(pCS->aName,
            osl_getThreadTextEncoding()));
        for (i=0; i<32; i++)
        {
            if ( i == 0 || c != 0 )
                c = n[i];
            *pMET << c;
        }
    }
}


void METWriter::WriteColorAttributeTable(sal_uLong nFieldId, BitmapPalette* pPalette, sal_uInt8 nBasePartFlags, sal_uInt8 nBasePartLCTID)
{
    sal_uInt16 nIndex,nNumI,i;

    if (bStatus==sal_False) return;

    //--- The Field 'Begin Color Attribute Table':
    WriteFieldIntroducer(16,BegColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    //--- The Field 'Color Attribute Table':
    WriteFieldIntroducer(0,BlkColAtrMagic,0,0);
    *pMET << nBasePartFlags << (sal_uInt8)0x00 << nBasePartLCTID; // 'Base Part'
    if (pPalette!=NULL)
    {
        nIndex=0;
        while (nIndex<pPalette->GetEntryCount())
        {
            nNumI=pPalette->GetEntryCount()-nIndex;
            if (nNumI>81) nNumI=81;
            *pMET << (sal_uInt8)(11+nNumI*3);                   // length of the parameter
            *pMET << (sal_uInt8)1 << (sal_uInt8)0 << (sal_uInt8)1;        // typ: element list, Reserved, Format: RGB
            *pMET << (sal_uInt8)0; WriteBigEndianShort(nIndex); // start-Index (3 Bytes)
            *pMET << (sal_uInt8)8 << (sal_uInt8)8 << (sal_uInt8)8;        // Bits per component R,G,B
            *pMET << (sal_uInt8)3;                              // number of bytes per entry
            for (i=0; i<nNumI; i++)
            {
                const BitmapColor& rCol = (*pPalette)[ nIndex ];

                *pMET << (sal_uInt8) rCol.GetRed();
                *pMET << (sal_uInt8) rCol.GetGreen();
                *pMET << (sal_uInt8) rCol.GetBlue();
                nIndex++;
            }
        }
    }
    else
    {
        // 'Trible Generating'
        *pMET << (sal_uInt8)0x0a << (sal_uInt8)0x02 << (sal_uInt8)0x00 << (sal_uInt8)0x01 << (sal_uInt8)0x00;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x08 << (sal_uInt8)0x08 << (sal_uInt8)0x08;
    }
    UpdateFieldSize();

    //--- The Field 'End Color Attribute Table':
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

    //--- The Field 'Begin Image Object':
    WriteFieldIntroducer(16,BegImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    // generate Windows-BMP file
    WriteDIB(rBitmap, aTemp, false, true);

    // read header of the Windows-BMP file:
    aTemp.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    aTemp.Seek(18);
    aTemp >> nWidth >> nHeight;
    aTemp.SeekRel(2);
    aTemp >> nBitsPerPixel;
    aTemp.SeekRel(8);
    aTemp >> nResX >> nResY;
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
            aTemp >> nb >> ng >> nr; aTemp.SeekRel(1);
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
        *pMET << (sal_uInt8)0x0c << (sal_uInt8)0x02 << (sal_uInt8)0x84 << (sal_uInt8)0x00;
        WriteFieldId(nActColMapId);
        *pMET << (sal_uInt8)0x04 << (sal_uInt8)0x24 << (sal_uInt8)0x07 << (sal_uInt8)0x01;

        //--- The Field 'End Object Environment Group':
        WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);
    }

    //--- The Field 'Image Data Descriptor':
    WriteFieldIntroducer(17,DscImgObjMagic,0,0);
    *pMET << (sal_uInt8)0x01; // Unit of measure: tens of centimeters
    WriteBigEndianShort((sal_uInt16)nResX);
    WriteBigEndianShort((sal_uInt16)nResY);
    WriteBigEndianShort((sal_uInt16)nWidth);
    WriteBigEndianShort((sal_uInt16)nHeight);

    //--- The first Field 'Image Picture Data':
    WriteFieldIntroducer(0,DatImgObjMagic,0,0);

    // Begin Segment:
    *pMET << (sal_uInt8)0x70 << (sal_uInt8)0x00;

    // Begin Image Content:
    *pMET << (sal_uInt8)0x91 << (sal_uInt8)0x01 << (sal_uInt8)0xff;

    // Image Size:
    *pMET << (sal_uInt8)0x94 << (sal_uInt8)0x09 << (sal_uInt8)0x02;
    *pMET << (sal_uInt16) 0 << (sal_uInt16) 0;
    WriteBigEndianShort((sal_uInt16)nHeight);
    WriteBigEndianShort((sal_uInt16)nWidth);

    // Image Encoding:
    *pMET << (sal_uInt8)0x95 << (sal_uInt8)0x02 << (sal_uInt8)0x03 << (sal_uInt8)0x03;

    // Image IDE-Size:
    *pMET << (sal_uInt8)0x96 << (sal_uInt8)0x01 << (sal_uInt8)nBitsPerPixel;

    if (nBitsPerPixel<=8) {
        // Image LUT-ID
        *pMET << (sal_uInt8)0x97 << (sal_uInt8)0x01 << (sal_uInt8)0x01;
    }
    else {
        // IDE Structure
        *pMET << (sal_uInt8)0x9b << (sal_uInt8)0x08 << (sal_uInt8)0x00 << (sal_uInt8)0x01;
        *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x08;
        *pMET << (sal_uInt8)0x08 << (sal_uInt8)0x08;
    }

    pBuf=new sal_uInt8[nBytesPerLine];
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

    // End Image Content:
    *pMET << (sal_uInt8)0x93 << (sal_uInt8)0x00;

    // End Segment:
    *pMET << (sal_uInt8)0x71 << (sal_uInt8)0x00;

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

    //------------------------------------------------------------------------------
    // The following is the OS2 original documentation and the associated implementation
    //------------------------------------------------------------------------------

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
    *pMET << (sal_uInt8)0xf7 << (sal_uInt8)0x07 << (sal_uInt8)0xb0 << (sal_uInt8)0x00
          << (sal_uInt8)0x00 << (sal_uInt8)0x23 << (sal_uInt8)0x01 << (sal_uInt8)0x01
          << (sal_uInt8)0x05;

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
    *pMET << (sal_uInt8)0xf6 << (sal_uInt8)0x28 << (sal_uInt8)0x40 << (sal_uInt8)0x00
          << (sal_uInt8)0x05 << (sal_uInt8)0x01
          << (sal_uInt32)(aUnitsPerDecimeter.Width())
          << (sal_uInt32)(aUnitsPerDecimeter.Height())
          << (sal_uInt32)0
          << (sal_uInt32)0 << (sal_uInt32)aPictureRect.GetWidth()
          << (sal_uInt32)0 << (sal_uInt32)aPictureRect.GetHeight()
          << (sal_uInt32)0 << (sal_uInt32)0;

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
    *pMET << (sal_uInt8)0x21 << (sal_uInt8)0x07 << (sal_uInt8)0x08 << (sal_uInt8)0xe0
          << (sal_uInt8)0x00 << (sal_uInt8)0x8f << (sal_uInt8)0x00 << (sal_uInt8)0x05
          << (sal_uInt8)0x05;

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set default viewing transform 0x07
    //  3-4       Mask 0xCC0C
    //  5         Names 0x8F
    //  6-n       M11, M12, M21, M22, M41, M42   Matrix elements
    *pMET << (sal_uInt8)0x21 << (sal_uInt8)0x1c << (sal_uInt8)0x07 << (sal_uInt8)0xcc
          << (sal_uInt8)0x0c << (sal_uInt8)0x8f
          << (sal_uInt32)0x00010000 << (sal_uInt32)0x00000000 << (sal_uInt32)0x00000000
          << (sal_uInt32)0x00010000 << (sal_uInt32)0x00000000 << (sal_uInt32)0x00000000;

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
    *pMET << (sal_uInt8)0x21 << (sal_uInt8)0x10 << (sal_uInt8)0x02 << (sal_uInt8)0x40
          << (sal_uInt8)0x00 << (sal_uInt8)0x8f
          << (sal_uInt8)0xaa << (sal_uInt8)0x02 << (sal_uInt8)0x00 << (sal_uInt8)0x00
          << (sal_uInt8)0x44 << (sal_uInt8)0x04 << (sal_uInt8)0x00 << (sal_uInt8)0x00
          << (sal_uInt8)0xa8 << (sal_uInt8)0xaa << (sal_uInt8)0x40 << (sal_uInt8)0x44;

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
    *pMET << (sal_uInt8)0x21 << (sal_uInt8)0x0c << (sal_uInt8)0x03 << (sal_uInt8)0x40
          << (sal_uInt8)0x00 << (sal_uInt8)0x8f
          << (sal_uInt8)0x66 << (sal_uInt8)0x02 << (sal_uInt8)0x00 << (sal_uInt8)0x00
          << (sal_uInt8)0x66 << (sal_uInt8)0x02 << (sal_uInt8)0x00 << (sal_uInt8)0x00;

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
        *pMET << (sal_uInt8)0xe7 << (sal_uInt8)0x07 << (sal_uInt8)0x80 << (sal_uInt8)0x00;
        WriteBigEndianLong(nActBitmapId);
        *pMET << (sal_uInt8)0xfe;
    }

    UpdateFieldSize();

    if (pMET->GetError()) bStatus=sal_False;
}


void METWriter::WillWriteOrder(sal_uLong nNextOrderMaximumLength)
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
    *pMET << (sal_uInt8)0xd6 << (sal_uInt8)44 << (sal_uInt16)0 << (sal_uInt16) 0x00cc;
    WriteBigEndianLong(nActBitmapId++);
    *pMET << (sal_uInt8)0x02 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
    WritePoint(Point(aPt.X(),aPt.Y()+aSize.Height()));
    WritePoint(Point(aPt.X()+aSize.Width(),aPt.Y()));
    *pMET << (sal_uInt32)0 << (sal_uInt32)0
          << (sal_uInt32)(rBmpSizePixel.Width())
          << (sal_uInt32)(rBmpSizePixel.Height());
}

void METWriter::METSetAndPushLineInfo( const LineInfo& rLineInfo )
{
    sal_Int32 nWidth = pCompDev->LogicToLogic( Size( rLineInfo.GetWidth(),0 ), aPictureMapMode, aTargetMapMode ).Width();

    WillWriteOrder( 8 );            // set stroke linewidth
    *pMET   << (sal_uInt8)0x15
            << (sal_uInt8)6
            << (sal_uInt8)0             // Flags
            << (sal_uInt8)0
            << nWidth;

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
        *pMET << (sal_uInt8)0x18 << nStyle;     // set LineType
    }
}

void METWriter::METPopLineInfo( const LineInfo& rLineInfo )
{
    WillWriteOrder( 8 );            // set stroke linewidth
    *pMET   << (sal_uInt8)0x15
            << (sal_uInt8)6
            << (sal_uInt8)0             // Flags
            << (sal_uInt8)0
            << (sal_uInt32)1;

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        WillWriteOrder( 2 );
        *pMET << (sal_uInt8)0x18 << (sal_uInt8)0;       // set LineType
    }
}

void METWriter::METBeginArea(sal_Bool bBoundaryLine)
{
    WillWriteOrder(2);
    *pMET << (sal_uInt8)0x68;
    if (bBoundaryLine) *pMET << (sal_uInt8)0xc0;
    else               *pMET << (sal_uInt8)0x80;
}


void METWriter::METEndArea()
{
    WillWriteOrder(2);
    *pMET << (sal_uInt8)0x60 << (sal_uInt8)0;
}


void METWriter::METBeginPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    *pMET << (sal_uInt8)0xd0 << (sal_uInt8)6 << (sal_uInt16) 0 << nPathId;
}


void METWriter::METEndPath()
{
    WillWriteOrder(2);
    *pMET << (sal_uInt8)0x7f << (sal_uInt8)0;
}


void METWriter::METFillPath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    *pMET << (sal_uInt8)0xd7 << (sal_uInt8)6
          << (sal_uInt8)0x00 << (sal_uInt8)0 << nPathId;
}


void METWriter::METOutlinePath(sal_uInt32 nPathId)
{
    WillWriteOrder(8);
    *pMET << (sal_uInt8)0xd4 << (sal_uInt8)6
          << (sal_uInt8)0 << (sal_uInt8)0 << nPathId;
}


void METWriter::METCloseFigure()
{
    WillWriteOrder(2);
    *pMET << (sal_uInt8)0x7d << (sal_uInt8)0;
}


void METWriter::METMove(Point aPt)
{
    WillWriteOrder(10);
    *pMET << (sal_uInt8)0x21 << (sal_uInt8)8;
    WritePoint(aPt);
}


void METWriter::METLine(Point aPt1, Point aPt2)
{
    WillWriteOrder(18);
    *pMET << (sal_uInt8)0xc1 << (sal_uInt8)16;
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
            *pMET << (sal_uInt8)0xc1; // Line at given pos
            bFirstOrder=sal_False;
        }
        else {
            *pMET << (sal_uInt8)0x81; // Line at current pos
        }
        *pMET << (sal_uInt8)(nOrderPoints*8);
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
    *pMET << (sal_uInt8)0x81 << (sal_uInt8)8;
    WritePoint(aPt);
}


void METWriter::METBox(sal_Bool bFill, sal_Bool bBoundary,
                       Rectangle aRect, sal_uInt32 nHAxis, sal_uInt32 nVAxis)
{
    sal_uInt8 nFlags=0;
    if (bFill)     nFlags|=0x40;
    if (bBoundary) nFlags|=0x20;

    WillWriteOrder(28);
    *pMET << (sal_uInt8)0xc0 << (sal_uInt8)26 << nFlags << (sal_uInt8)0;
    WritePoint(aRect.BottomLeft());
    WritePoint(aRect.TopRight());
    *pMET << nHAxis << nVAxis;
}


void METWriter::METFullArc(Point aCenter, double fMultiplier)
{
    WillWriteOrder(14);
    *pMET << (sal_uInt8)0xc7 << (sal_uInt8)12;
    WritePoint(aCenter);
    *pMET << (sal_Int32)(fMultiplier*65536.0+0.5);
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
    *pMET << (sal_uInt8)0xa3 << (sal_uInt8)20;
    WritePoint(aCenter);
    *pMET << (sal_Int32)(fMultiplier*65536.0+0.5);
    *pMET << (sal_Int32)(fStartAngle*65536.0+0.5);
    *pMET << (sal_Int32)(fSweepAngle*65536.0+0.5);
}


void METWriter::METChrStr( Point aPt, String aUniStr )
{
    OString aStr(OUStringToOString(aUniStr,
        osl_getThreadTextEncoding()));
    sal_uInt16 nLen = aStr.getLength();
    WillWriteOrder( 11 + nLen );
    *pMET << (sal_uInt8)0xc3 << (sal_uInt8)( 9 + nLen );
    WritePoint(aPt);
    for (sal_uInt16 i = 0; i < nLen; ++i)
        *pMET << aStr[i];
    *pMET << (sal_uInt8)0;
}


void METWriter::METSetArcParams(sal_Int32 nP, sal_Int32 nQ, sal_Int32 nR, sal_Int32 nS)
{
    WillWriteOrder(18);
    *pMET << (sal_uInt8)0x22 << (sal_uInt8)16 << nP << nQ << nR << nS;
}


void METWriter::METSetColor(Color aColor)
{
    if (aColor==aMETColor) return;
    aMETColor=aColor;

    WillWriteOrder(6);
    *pMET << (sal_uInt8)0xa6 << (sal_uInt8)4 << (sal_uInt8)0
          << (sal_uInt8)(aColor.GetBlue())
          << (sal_uInt8)(aColor.GetGreen())
          << (sal_uInt8)(aColor.GetRed());
}


void METWriter::METSetBackgroundColor(Color aColor)
{
    if (aColor==aMETBackgroundColor) return;
    aMETBackgroundColor=aColor;

    WillWriteOrder(6);
    *pMET << (sal_uInt8)0xa7 << (sal_uInt8)4 << (sal_uInt8)0
          << (sal_uInt8)(aColor.GetBlue())
          << (sal_uInt8)(aColor.GetGreen())
          << (sal_uInt8)(aColor.GetRed());
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
    *pMET << (sal_uInt8)0x0c << nMix;
}


void METWriter::METSetChrCellSize(Size aSize)
{
    if (aMETChrCellSize==aSize)
        return;

    aMETChrCellSize=aSize;
    WillWriteOrder(10);
    if (aSize.Width()==0) aSize.Width()=aSize.Height();
    *pMET << (sal_uInt8)0x33 << (sal_uInt8)8 << (sal_Int32)aSize.Width() << (sal_Int32)aSize.Height();
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
    *pMET << (sal_uInt8)0x34 << (sal_uInt8)8 << nax << nay;
}


void METWriter::METSetChrSet(sal_uInt8 nSet)
{
    if (nMETChrSet==nSet)
        return;

    nMETChrSet=nSet;
    WillWriteOrder(2);
    *pMET << (sal_uInt8)0x38 << nSet;
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
                METChrStr(aPt, String(pA->GetText(),pA->GetIndex(),pA->GetLen()));
            }
            break;

            case META_TEXTARRAY_ACTION:
            {
                const MetaTextArrayAction*  pA = (const MetaTextArrayAction*) pMA;
                sal_uInt16                      i;
                String                      aStr;
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
                aStr=String(pA->GetText(),pA->GetIndex(),pA->GetLen());

                if( pA->GetDXArray()!=NULL )
                {
                    Point aPt2;

                    for( i=0; i < aStr.Len(); i++ )
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
                        METChrStr( aPt2, OUString( aStr.GetChar( i ) ) );
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
                String                          aStr;
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
                aStr=String(pA->GetText(),pA->GetIndex(),pA->GetLen());
                pDXAry=new sal_Int32[aStr.Len()];
                nNormSize = aVDev.GetTextArray( aStr, pDXAry );

                for ( i = 0; i < aStr.Len(); i++ )
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
                    METChrStr( aPt2, OUString( aStr.GetChar( i ) ) );
                }

                delete[] pDXAry;
            }
            break;

            case META_TEXTRECT_ACTION:
            {
//              OSL_FAIL( "Unsupported MET-Action: META_TEXTRECT_ACTION!" );
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

    //--- The Field 'Begin Object Environment Group':
    WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
    WriteFieldId(7);

    //--- The Field 'Map Color Attribute Table':
    WriteFieldIntroducer(22,MapColAtrMagic,0,0);
    WriteBigEndianShort(0x000e);
    *pMET << (sal_uInt8)0x0c << (sal_uInt8)0x02 << (sal_uInt8)0x84 << (sal_uInt8)0x00;
    WriteFieldId(4);

    //--- The first Field 'Map Coded Font':
    WriteFieldIntroducer(32,MapCodFntMagic,0,0);
    WriteBigEndianShort(0x0018);
    *pMET << (sal_uInt8)0x0c << (sal_uInt8)0x02 << (sal_uInt8)0x84 << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0xff << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00 << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0x04 << (sal_uInt8)0x24 << (sal_uInt8)0x05 << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0x06 << (sal_uInt8)0x20;
    *pMET << (sal_uInt8)0x03 << (sal_uInt8)0x97 << (sal_uInt8)0x01 << (sal_uInt8)0xb5;

    //--- The additional Fields 'Map Coded Font':
    CreateChrSets(pMTF);
    WriteChrSets();

    //--- The Fields 'Map Data Resource':
    nId=nActBitmapId;
    for (i=0; i<nNumberOfBitmaps; i++)
    {
        WriteFieldIntroducer(29,MapDatResMagic,0,0);
        WriteBigEndianShort(0x0015);
        *pMET << (sal_uInt8)0x0c << (sal_uInt8)0x02 << (sal_uInt8)0x84 << (sal_uInt8)0x00;
        WriteFieldId(nId);
        *pMET << (sal_uInt8)0x07 << (sal_uInt8)0x22 << (sal_uInt8)0x10;
        *pMET << (sal_uInt32)nId;
        nId++;
    }

    //--- Das Feld 'End Object Environment Group':
    WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
    WriteFieldId(7);
}


void METWriter::WriteGraphicsObject(const GDIMetaFile * pMTF)
{
    sal_uLong nSegmentSize,nPos,nDataFieldsStartPos;

    if( bStatus==sal_False )
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
    *pMET << (sal_uInt8)0x70 << (sal_uInt8)0x0e << (sal_uInt32)0;
    *pMET << (sal_uInt8)0x70 << (sal_uInt8)0x10; // Flags
    *pMET << (sal_uInt16)0; // Lo-Word of the length of the segment data  (Big Endian)
    *pMET << (sal_uInt32)0;  // Reserved
    *pMET << (sal_uInt16)0; // Hi-Word of the length of the segment (Big Endian) (Ohh Ohh OS2)
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
        bStatus=sal_False;
}


void METWriter::WriteResourceGroup(const GDIMetaFile * pMTF)
{
    if( bStatus==sal_False )
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
        bStatus=sal_False;
}


void METWriter::WriteDocument(const GDIMetaFile * pMTF)
{
    if( bStatus==sal_False )
        return;

    //--- The Field 'Begin Document':
    WriteFieldIntroducer(0,BegDocumnMagic,0,0);
    WriteFieldId(1);
    *pMET << (sal_uInt8)0x00 << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0x05 << (sal_uInt8)0x18 << (sal_uInt8)0x03 << (sal_uInt8)0x0c << (sal_uInt8)0x00;
    *pMET << (sal_uInt8)0x06 << (sal_uInt8)0x01 << (sal_uInt8)0x03 << (sal_uInt8)0xd4 << (sal_uInt8)0x03 << (sal_uInt8)0x52;
    *pMET << (sal_uInt8)0x03 << (sal_uInt8)0x65 << (sal_uInt8)0x00;
    UpdateFieldSize();

    //--- The Content:
    WriteResourceGroup(pMTF);

    //--- The Field 'End Document':
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

//================== GraphicExport - the exported Function ================

// this needs to be kept in sync with
// ImpFilterLibCacheEntry::GetImportFunction() from
// vcl/source/filter/graphicfilter.cxx
#if defined(DISABLE_DYNLOADING)
#define GraphicExport emeGraphicExport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicExport( SvStream & rStream, Graphic & rGraphic, FilterConfigItem* pFilterConfigItem, sal_Bool )
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
