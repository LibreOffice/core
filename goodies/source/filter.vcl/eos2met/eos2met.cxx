/*************************************************************************
 *
 *  $RCSfile: eos2met.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:12 $
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

#if SUPD <= 364
#include <tools/mtf.hxx>
#include <fltcall.hxx>
#else
#include <svtools/fltcall.hxx>
#endif

#include <math.h>
#include <tools/stream.hxx>
#include <tools/bigint.hxx>
#include <vcl/metaact.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/poly.hxx>
#include <vcl/graph.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/solar.hrc>
#include "strings.hrc"
#include "dlgeos2.hxx"

// -----------------------------Feld-Typen-------------------------------

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
    BYTE nSet;
    ByteString aName;
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
    Region                      aClipRegion;
};

class METWriter
{
private:

    BOOL                bStatus;
    PFilterCallback     pCallback;
    void*               pCallerData;
    ULONG               nLastPercent; // Mit welcher Zahl pCallback zuletzt aufgerufen wurde.
    SvStream*           pMET;
    Rectangle           aPictureRect;
    MapMode             aPictureMapMode;
    MapMode             aTargetMapMode;
    ULONG               nActualFieldStartPos;     // Anfangs-Position des aktuellen 'Field'
    ULONG               nNumberOfDataFields;  // Anzahl der angefangenen 'Graphcis Data Fields'
    Color               aGDILineColor;
    Color               aGDIFillColor;
    RasterOp            eGDIRasterOp;
    Font                aGDIFont;
    MapMode             aGDIMapMode;   // derzeit unbenutzt!
    Region              aGDIClipRegion; // derzeit unbenutzt!
    METGDIStackMember*  pGDIStack;
    Color               aMETColor;
    Color               aMETBackgroundColor;
    Color               aMETPatternSymbol;
    RasterOp            eMETMix ;
    long                nMETStrokeLineWidth;
    Size                aMETChrCellSize;
    short               nMETChrAngle;
    BYTE                nMETChrSet;
    METChrSet*          pChrSetList; // Liste der Character-Sets
    BYTE                nNextChrSetId; // die erste unbenutzte ChrSet-Id
    ULONG               nActBitmapId; // Field-Id der naechsten Bitmap
    ULONG               nNumberOfActions; // Anzahl der Actions im GDIMetafile
    ULONG               nNumberOfBitmaps; // Anzahl der Bitmaps
    ULONG               nWrittenActions;  // Anzahl der bereits verarbeiteten Actions beim Schreiben der Orders
    ULONG               nWrittenBitmaps;  // Anzahl der bereits geschriebenen Bitmaps
    ULONG               nActBitmapPercent; // Wieviel Prozent die naechste Bitmap schon geschrieben ist.

    void MayCallback();
        // Berechnet anhand der obigen 5 Parameter eine Prozentzahl
        // und macht dann ggf. einen Callback. Setzt bStatus auf FALSE wenn User abbrechen
        // moechte.

    void CountActionsAndBitmaps(const GDIMetaFile * pMTF);
        // Zaehlt die Bitmaps und Actions (nNumberOfActions und nNumberOfBitmaps muessen
        // zu Anfang auf 0 gesetzt werden, weil diese Methode rekursiv ist)

    void WriteBigEndianShort(USHORT nWord);
    void WriteBigEndianLong(ULONG nLong);

    void WritePoint(Point aPt);

    void WriteFieldIntroducer(USHORT nFieldSize, USHORT nFieldType,
                              BYTE nFlags, USHORT nSegSeqNum);
    void UpdateFieldSize();

    void WriteFieldId(ULONG nId);

    void CreateChrSets(const GDIMetaFile * pMTF);
    void CreateChrSet(const Font & rFont);
    void WriteChrSets();
    BYTE FindChrSet(const Font & rFont);

    void WriteColorAttributeTable(ULONG nFieldId=4, BitmapPalette* pPalette=NULL,
                                  BYTE nBasePartFlags=0x40, BYTE nBasePartLCTID=0);

    void WriteImageObject(const Bitmap & rBitmap);
    void WriteImageObjects(const GDIMetaFile * pMTF);

    void WriteDataDescriptor(const GDIMetaFile * pMTF);

    void WillWriteOrder(ULONG nNextOrderMaximumLength);

    void METSetAndPushLineInfo( const LineInfo& rLineInfo );
    void METPopLineInfo( const LineInfo& rLineInfo );
    void METBitBlt(Point aPt, Size aSize, const Bitmap & rBitmap);
    void METBeginArea(BOOL bBoundaryLine);
    void METEndArea();
    void METBeginPath(ULONG nPathId);
    void METEndPath();
    void METFillPath(ULONG nPathId);
    void METOutlinePath(ULONG nPathId);
    void METCloseFigure();
    void METMove(Point aPt);
    void METLine(Point aPt1, Point aPt2);
    void METLine(const Polygon & rPolygon);
    void METLine(const PolyPolygon & rPolyPolygon);
    void METLineAtCurPos(Point aPt);
    void METBox(BOOL bFill, BOOL bBoundary,
                Rectangle aRect, ULONG nHAxis, ULONG nVAxis);
    void METArc(Point aP0, Point aP1, Point aP2);
    void METArcAtCurPos(Point aP1, Point aP2);
    void METFullArc(Point aCenter, double fMultiplier);
    void METPartialArcAtCurPos(Point aCenter, double fMultiplier,
                               double fStartAngle, double fSweepAngle);
    void METChrStr(Point aPt, String aStr);
    void METSetArcParams(long nP, long nQ, long nR, long nS);
    void METSetColor(Color aColor);
    void METSetBackgroundColor(Color aColor);
    void METSetMix(RasterOp eROP);
    void METSetChrCellSize(Size aSize);
    void METSetChrAngle(short nAngle);
    void METSetChrSet(BYTE nSet);

    void WriteOrders(const GDIMetaFile * pMTF);

    void WriteObjectEnvironmentGroup(const GDIMetaFile * pMTF);

    void WriteGraphicsObject(const GDIMetaFile * pMTF);

    void WriteResourceGroup(const GDIMetaFile * pMTF);

    void WriteDocument(const GDIMetaFile * pMTF);

public:

    METWriter() {}

    BOOL WriteMET(const GDIMetaFile & rMTF, SvStream & rTargetStream,
                  PFilterCallback pcallback, void * pcallerdata);
};


//========================== Methoden von METWriter ==========================

void METWriter::MayCallback()
{
    ULONG nPercent;

    // Wir gehen mal einfach so davon aus, dass 16386 Actions einer Bitmap entsprechen
    // (in der Regel wird ein Metafile entweder nur Actions oder einige Bitmaps und fast
    // keine Actions enthalten. Dann ist das Verhaeltnis ziemlich unwichtig)

    nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
             *100/((nNumberOfBitmaps<<14)+nNumberOfActions);

    if (nPercent>=nLastPercent+3)
    {
        nLastPercent=nPercent;
        if(pCallback!=NULL && nPercent<=100)
        {
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE)
                bStatus=FALSE;
        }
    }
}

void METWriter::CountActionsAndBitmaps(const GDIMetaFile * pMTF)
{
    const MetaAction* pMA;

    for( ULONG nAction = 0, nCount=pMTF->GetActionCount(); nAction < nCount; nAction++ )
    {
        pMA =  pMTF->GetAction(nAction);

        switch (pMA->GetType())
        {
            case META_EPS_ACTION :
            {
                const GDIMetaFile aGDIMetaFile( ((const MetaEPSAction*)pMA)->GetSubstitute() );
                INT32 nCount = aGDIMetaFile.GetActionCount();
                INT32 i;
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


void METWriter::WriteBigEndianShort(USHORT nWord)
{
    *pMET << ((BYTE)(nWord>>8)) << ((BYTE)(nWord&0x00ff));
}


void METWriter::WriteBigEndianLong(ULONG nLong)
{
    WriteBigEndianShort((USHORT)(nLong>>16));
    WriteBigEndianShort((USHORT)(nLong&0x0000ffff));
}


void METWriter::WritePoint(Point aPt)
{
    Point aNewPt = OutputDevice::LogicToLogic( aPt, aPictureMapMode, aTargetMapMode );

    *pMET << (long) ( aNewPt.X() - aPictureRect.Left() )
          << (long) ( aPictureRect.Bottom() - aNewPt.Y() );
}


void METWriter::WriteFieldIntroducer(USHORT nFieldSize, USHORT nFieldType,
                                     BYTE nFlags, USHORT nSegSeqNum)
{
    nActualFieldStartPos=pMET->Tell();
    WriteBigEndianShort(nFieldSize);
    *pMET << (BYTE)0xd3 << nFieldType << nFlags << nSegSeqNum;
}


void METWriter::UpdateFieldSize()
{
    ULONG nPos;

    nPos=pMET->Tell();
    pMET->Seek(nActualFieldStartPos);
    WriteBigEndianShort((USHORT)(nPos-nActualFieldStartPos));
    pMET->Seek(nPos);
}


void METWriter::WriteFieldId(ULONG nId)
{
    BYTE nbyte;
    short i;

    for (i=1; i<=8; i++) {
        nbyte= '0' + (BYTE)((nId >> (32-i*4)) & 0x0f);
        *pMET << nbyte;
    }
}


void METWriter::CreateChrSets(const GDIMetaFile * pMTF)
{
    ULONG nAction, nActionCount;
    const MetaAction * pMA;

    if (bStatus==FALSE)
        return;

    nActionCount=pMTF->GetActionCount();

    for (nAction=0; nAction<nActionCount; nAction++)
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
        pCS->aName = ByteString( rFont.GetName(), RTL_TEXTENCODING_UTF8 );
        pCS->eWeight = rFont.GetWeight();
    }
}


BYTE METWriter::FindChrSet(const Font & rFont)
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
    USHORT i;
    char c;
    METChrSet * pCS;
    BYTE nbyte;

    for (pCS=pChrSetList; pCS!=NULL; pCS=pCS->pSucc)
    {

        WriteFieldIntroducer(0x58,MapCodFntMagic,0,0);

        WriteBigEndianShort(0x0050);

        *pMET << (BYTE)0x0c << (BYTE)0x02 << (BYTE)0x84 << (BYTE)0x00;
        *pMET << (BYTE)0xa4 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x01;
        *pMET << (BYTE)0x01 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;

        *pMET << (BYTE)0x04 << (BYTE)0x24 << (BYTE)0x05 << (BYTE)pCS->nSet;

        *pMET << (BYTE)0x14 << (BYTE)0x1f;
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
        *pMET << (BYTE)0x05;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x0c;

        *pMET << (BYTE)0x06 << (BYTE)0x20 << (BYTE)0x03 << (BYTE)0xd4;
        *pMET << (BYTE)0x03 << (BYTE)0x52;

        *pMET << (BYTE)0x24 << (BYTE)0x02 << (BYTE)0x08 << (BYTE)0x00;
        for (i=0; i<32; i++)
        {
            if ( i == 0 || c != 0 )
                c = pCS->aName.GetChar( i );
            *pMET << c;
        }
    }
}


void METWriter::WriteColorAttributeTable(ULONG nFieldId, BitmapPalette* pPalette, BYTE nBasePartFlags, BYTE nBasePartLCTID)
{
    USHORT nIndex,nNumI,i;

    if (bStatus==FALSE) return;

    //--- Das Feld 'Begin Color Attribute Table':
    WriteFieldIntroducer(16,BegColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    //--- Das Feld 'Color Attribute Table':
    WriteFieldIntroducer(0,BlkColAtrMagic,0,0);
    *pMET << nBasePartFlags << (BYTE)0x00 << nBasePartLCTID; // 'Base Part'
    if (pPalette!=NULL)
    {
        nIndex=0;
        while (nIndex<pPalette->GetEntryCount())
        {
            nNumI=pPalette->GetEntryCount()-nIndex;
            if (nNumI>81) nNumI=81;
            *pMET << (BYTE)(11+nNumI*3);                   // Laenge des Parameters
            *pMET << (BYTE)1 << (BYTE)0 << (BYTE)1;        // typ: element list, Reserved, Format: RGB
            *pMET << (BYTE)0; WriteBigEndianShort(nIndex); // Start-Index (3 Bytes)
            *pMET << (BYTE)8 << (BYTE)8 << (BYTE)8;        // Bits je Komponente R,G,B
            *pMET << (BYTE)3;                              // Anzahl Bytes je Eintrag
            for (i=0; i<nNumI; i++)
            {
                const BitmapColor& rCol = (*pPalette)[ nIndex ];

                *pMET << (BYTE) rCol.GetRed();
                *pMET << (BYTE) rCol.GetGreen();
                *pMET << (BYTE) rCol.GetBlue();
                nIndex++;
            }
        }
    }
    else
    {
        // 'Trible Generating'
        *pMET << (BYTE)0x0a << (BYTE)0x02 << (BYTE)0x00 << (BYTE)0x01 << (BYTE)0x00;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x08 << (BYTE)0x08 << (BYTE)0x08;
    }
    UpdateFieldSize();

    //--- Das Feld 'End Color Attribute Table':
    WriteFieldIntroducer(16,EndColAtrMagic,0,0);
    WriteFieldId(nFieldId);

    if (pMET->GetError())
        bStatus=FALSE;
}


void METWriter::WriteImageObject(const Bitmap & rBitmap)
{
    SvMemoryStream aTemp(0x00010000,0x00010000);
    ULONG nWidth,nHeight,nResX,nResY,nBytesPerLine,i,j,nNumColors,ny,nLines;
    ULONG nActColMapId;
    USHORT nBitsPerPixel;
    BYTE nbyte, * pBuf;

    if (bStatus==FALSE)
        return;

    nActColMapId=((nActBitmapId>>24)&0x000000ff) | ((nActBitmapId>> 8)&0x0000ff00) |
                 ((nActBitmapId<< 8)&0x00ff0000) | ((nActBitmapId<<24)&0xff000000);

    //--- Das Feld 'Begin Image Object':
    WriteFieldIntroducer(16,BegImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    // Windows-BMP-Datei erzeugen:
    aTemp << rBitmap;

    // Header der Windows-BMP-Datei einlesen:
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

    // ggf. Farbpalette einlesen und in die MET-Datei schreiben:
    if (nBitsPerPixel<=8)
    {
        BitmapPalette   aPal( (USHORT) nNumColors );
        BYTE            nr,ng,nb;

        for (i=0; i<nNumColors; i++)
        {
            aTemp >> nb >> ng >> nr; aTemp.SeekRel(1);
            aPal[ (USHORT) i ] = BitmapColor( nr, ng, nb );
        }

        //--- Das Feld 'Begin Resource Group':
        WriteFieldIntroducer(16,BegResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        //--- Farbtabelle schreiben:
        WriteColorAttributeTable(nActColMapId,&aPal,0,1);

        //--- Das Feld 'End Resource Group':
        WriteFieldIntroducer(16,EndResGrpMagic,0,0);
        WriteFieldId(nActColMapId);

        //--- Das Feld 'Begin Object Environment Group':
        WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);

        //--- Das Feld 'Map Color Attribute Table':
        WriteFieldIntroducer(26,MapColAtrMagic,0,0);
        WriteBigEndianShort(0x0012);
        *pMET << (BYTE)0x0c << (BYTE)0x02 << (BYTE)0x84 << (BYTE)0x00;
        WriteFieldId(nActColMapId);
        *pMET << (BYTE)0x04 << (BYTE)0x24 << (BYTE)0x07 << (BYTE)0x01;

        //--- Das Feld 'End Object Environment Group':
        WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
        WriteFieldId(nActBitmapId);
    }

    //--- Das Feld 'Image Data Descriptor':
    WriteFieldIntroducer(17,DscImgObjMagic,0,0);
    *pMET << (BYTE)0x01; // Unit of measure: tens of centimeters
    WriteBigEndianShort((USHORT)nResX);
    WriteBigEndianShort((USHORT)nResY);
    WriteBigEndianShort((USHORT)nWidth);
    WriteBigEndianShort((USHORT)nHeight);

    //--- Das erste Feld 'Image Picture Data':
    WriteFieldIntroducer(0,DatImgObjMagic,0,0);

    // Begin Segment:
    *pMET << (BYTE)0x70 << (BYTE)0x00;

    // Begin Image Content:
    *pMET << (BYTE)0x91 << (BYTE)0x01 << (BYTE)0xff;

    // Image Size:
    *pMET << (BYTE)0x94 << (BYTE)0x09 << (BYTE)0x02;
    *pMET << (USHORT) 0 << (USHORT) 0;
    WriteBigEndianShort((USHORT)nHeight);
    WriteBigEndianShort((USHORT)nWidth);

    // Image Encoding:
    *pMET << (BYTE)0x95 << (BYTE)0x02 << (BYTE)0x03 << (BYTE)0x03;

    // Image IDE-Size:
    *pMET << (BYTE)0x96 << (BYTE)0x01 << (BYTE)nBitsPerPixel;

    if (nBitsPerPixel<=8) {
        // Image LUT-ID
        *pMET << (BYTE)0x97 << (BYTE)0x01 << (BYTE)0x01;
    }
    else {
        // IDE Structure
        *pMET << (BYTE)0x9b << (BYTE)0x08 << (BYTE)0x00 << (BYTE)0x01;
        *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x08;
        *pMET << (BYTE)0x08 << (BYTE)0x08;
    }

    pBuf=new BYTE[nBytesPerLine];
    ny=0;
    while (ny<nHeight) {

        // Abschliessen des vorherigen Feldes 'Image Picture Data':
        UpdateFieldSize();

        // Und ein neues Feld 'Image Picture Data' anfangen:
        WriteFieldIntroducer(0,DatImgObjMagic,0,0);

        // Einige Scanlines lesen und schreiben:
        nLines=nHeight-ny;
        if (nLines*nBytesPerLine>30000) nLines=30000/nBytesPerLine;
        if (nLines<1) nLines=1;
        WriteBigEndianShort(0xfe92);
        WriteBigEndianShort((USHORT)(nLines*nBytesPerLine));
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
        if (aTemp.GetError() || pMET->GetError()) bStatus=FALSE;
        nActBitmapPercent=(ny+1)*100/nHeight;
        MayCallback();
        if (bStatus==FALSE) { delete pBuf; return; }
    }
    delete pBuf;

    // End Image Content:
    *pMET << (BYTE)0x93 << (BYTE)0x00;

    // End Segment:
    *pMET << (BYTE)0x71 << (BYTE)0x00;

    // Abschliessen des letzten Feldes 'Image Picture Data':
    UpdateFieldSize();

    //--- Das Feld 'End Image Object':
    WriteFieldIntroducer(16,EndImgObjMagic,0,0);
    WriteFieldId(nActBitmapId);

    // Ids erhoehen:
    nActBitmapId++;

    // Bitmaps zaehlen:
    nWrittenBitmaps++;
    nActBitmapPercent=0;

    if (pMET->GetError()) bStatus=FALSE;
}


void METWriter::WriteImageObjects(const GDIMetaFile * pMTF)
{
    const MetaAction*   pMA;

    if (bStatus==FALSE)
        return;

    for ( ULONG nAction = 0, nCount = pMTF->GetActionCount(); nAction < nCount; nAction++)
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

                INT32 nCount = aGDIMetaFile.GetActionCount();
                for ( INT32 i = 0; i < nCount; i++ )
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

        if (bStatus==FALSE)
            break;
    }

    if (pMET->GetError())
        bStatus=FALSE;
}

void METWriter::WriteDataDescriptor(const GDIMetaFile *)
{
    if (bStatus==FALSE)
        return;

    WriteFieldIntroducer(0,DscGrfObjMagic,0,0);

    //------------------------------------------------------------------------------
    // Im Folgenden die OS2-Orginal-Dokumentation und die Implementation dazu (uff)
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
    *pMET << (BYTE)0xf7 << (BYTE)0x07 << (BYTE)0xb0 << (BYTE)0x00
          << (BYTE)0x00 << (BYTE)0x23 << (BYTE)0x01 << (BYTE)0x01
          << (BYTE)0x05;

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
    *pMET << (BYTE)0xf6 << (BYTE)0x28 << (BYTE)0x40 << (BYTE)0x00
          << (BYTE)0x05 << (BYTE)0x01
          << (ULONG)(aUnitsPerDecimeter.Width())
          << (ULONG)(aUnitsPerDecimeter.Height())
          << (ULONG)0
          << (ULONG)0 << (ULONG)aPictureRect.GetWidth()
          << (ULONG)0 << (ULONG)aPictureRect.GetHeight()
          << (ULONG)0 << (ULONG)0;

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
    *pMET << (BYTE)0x21 << (BYTE)0x07 << (BYTE)0x08 << (BYTE)0xe0
          << (BYTE)0x00 << (BYTE)0x8f << (BYTE)0x00 << (BYTE)0x05
          << (BYTE)0x05;

    //  0         0x21 Set Current Defaults
    //  1         Length of following data
    //  2         Set default viewing transform 0x07
    //  3-4       Mask 0xCC0C
    //  5         Names 0x8F
    //  6-n       M11, M12, M21, M22, M41, M42   Matrix elements
    *pMET << (BYTE)0x21 << (BYTE)0x1c << (BYTE)0x07 << (BYTE)0xcc
          << (BYTE)0x0c << (BYTE)0x8f
          << (ULONG)0x00010000 << (ULONG)0x00000000 << (ULONG)0x00000000
          << (ULONG)0x00010000 << (ULONG)0x00000000 << (ULONG)0x00000000;

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
    *pMET << (BYTE)0x21 << (BYTE)0x10 << (BYTE)0x02 << (BYTE)0x40
          << (BYTE)0x00 << (BYTE)0x8f
          << (BYTE)0xaa << (BYTE)0x02 << (BYTE)0x00 << (BYTE)0x00
          << (BYTE)0x44 << (BYTE)0x04 << (BYTE)0x00 << (BYTE)0x00
          << (BYTE)0xa8 << (BYTE)0xaa << (BYTE)0x40 << (BYTE)0x44;

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
    *pMET << (BYTE)0x21 << (BYTE)0x0c << (BYTE)0x03 << (BYTE)0x40
          << (BYTE)0x00 << (BYTE)0x8f
          << (BYTE)0x66 << (BYTE)0x02 << (BYTE)0x00 << (BYTE)0x00
          << (BYTE)0x66 << (BYTE)0x02 << (BYTE)0x00 << (BYTE)0x00;

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
        *pMET << (BYTE)0xe7 << (BYTE)0x07 << (BYTE)0x80 << (BYTE)0x00;
        WriteBigEndianLong(nActBitmapId);
        *pMET << (BYTE)0xfe;
    }

    UpdateFieldSize();

    if (pMET->GetError()) bStatus=FALSE;
}


void METWriter::WillWriteOrder(ULONG nNextOrderMaximumLength)
{
    // Die Parameter eines 'Graphics Data Fields' duerfen (laut OS2-Doku)
    // hoechstens 32759 Bytes umfassen. Gemeint ist die Laenge des Feldes minus
    // dem 'Structured Field Introducer' (groesse: 8). Also darf die Groesse
    // des ganzen Fields hoechstens 8+32759=32767=0x7fff sein.
    // Zur Sicherheit nehmen wir lieber 30000 als Grenze.

    if (pMET->Tell()-nActualFieldStartPos+nNextOrderMaximumLength>30000)
    {
        UpdateFieldSize();
        WriteFieldIntroducer(0,DatGrfObjMagic,0,0);
        nNumberOfDataFields++;
    }
}



void METWriter::METBitBlt(Point aPt, Size aSize, const Bitmap & rBitmap)
{
    WillWriteOrder(46);
    *pMET << (BYTE)0xd6 << (BYTE)44 << (USHORT)0 << (USHORT) 0x00cc;
    WriteBigEndianLong(nActBitmapId++);
    *pMET << (BYTE)0x02 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
    WritePoint(Point(aPt.X(),aPt.Y()+aSize.Height()));
    WritePoint(Point(aPt.X()+aSize.Width(),aPt.Y()));
    *pMET << (ULONG)0 << (ULONG)0
          << (ULONG)(rBitmap.GetSizePixel().Width())
          << (ULONG)(rBitmap.GetSizePixel().Height());
}

void METWriter::METSetAndPushLineInfo( const LineInfo& rLineInfo )
{
    INT32 nWidth = OutputDevice::LogicToLogic( Size( rLineInfo.GetWidth(),0 ), aPictureMapMode, aTargetMapMode ).Width();

    WillWriteOrder( 8 );            // set stroke linewidth
    *pMET   << (BYTE)0x15
            << (BYTE)6
            << (BYTE)0              // Flags
            << (BYTE)0
            << nWidth;

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        BYTE nStyle = 0;            // LineDefault;

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
        }
        WillWriteOrder( 2 );
        *pMET << (BYTE)0x18 << nStyle;      // set LineType
    }
}

void METWriter::METPopLineInfo( const LineInfo& rLineInfo )
{
    WillWriteOrder( 8 );            // set stroke linewidth
    *pMET   << (BYTE)0x15
            << (BYTE)6
            << (BYTE)0              // Flags
            << (BYTE)0
            << (UINT32)1;

    if ( rLineInfo.GetStyle() != LINE_SOLID )
    {
        WillWriteOrder( 2 );
        *pMET << (BYTE)0x18 << (BYTE)0;     // set LineType
    }
}

void METWriter::METBeginArea(BOOL bBoundaryLine)
{
    WillWriteOrder(2);
    *pMET << (BYTE)0x68;
    if (bBoundaryLine) *pMET << (BYTE)0xc0;
    else               *pMET << (BYTE)0x80;
}


void METWriter::METEndArea()
{
    WillWriteOrder(2);
    *pMET << (BYTE)0x60 << (BYTE)0;
}


void METWriter::METBeginPath(ULONG nPathId)
{
    WillWriteOrder(8);
    *pMET << (BYTE)0xd0 << (BYTE)6 << (USHORT) 0 << nPathId;
}


void METWriter::METEndPath()
{
    WillWriteOrder(2);
    *pMET << (BYTE)0x7f << (BYTE)0;
}


void METWriter::METFillPath(ULONG nPathId)
{
    WillWriteOrder(8);
    *pMET << (BYTE)0xd7 << (BYTE)6
          << (BYTE)0x00 << (BYTE)0 << nPathId;
}


void METWriter::METOutlinePath(ULONG nPathId)
{
    WillWriteOrder(8);
    *pMET << (BYTE)0xd4 << (BYTE)6
          << (BYTE)0 << (BYTE)0 << nPathId;
}


void METWriter::METCloseFigure()
{
    WillWriteOrder(2);
    *pMET << (BYTE)0x7d << (BYTE)0;
}


void METWriter::METMove(Point aPt)
{
    WillWriteOrder(10);
    *pMET << (BYTE)0x21 << (BYTE)8;
    WritePoint(aPt);
}


void METWriter::METLine(Point aPt1, Point aPt2)
{
    WillWriteOrder(18);
    *pMET << (BYTE)0xc1 << (BYTE)16;
    WritePoint(aPt1); WritePoint(aPt2);
}


void METWriter::METLine(const Polygon & rPolygon)
{
    USHORT nNumPoints,i,j,nOrderPoints;
    BOOL bFirstOrder;

    bFirstOrder=TRUE;
    i=0; nNumPoints=rPolygon.GetSize();
    while (i<nNumPoints) {
        nOrderPoints=nNumPoints-i;
        if (nOrderPoints>30) nOrderPoints=30;
        WillWriteOrder(nOrderPoints*8+2);
        if (bFirstOrder==TRUE) {
            *pMET << (BYTE)0xc1; // Line at given pos
            bFirstOrder=FALSE;
        }
        else {
            *pMET << (BYTE)0x81; // Line at current pos
        }
        *pMET << (BYTE)(nOrderPoints*8);
        for (j=0; j<nOrderPoints; j++) WritePoint(rPolygon.GetPoint(i++));
    }
}


void METWriter::METLine(const PolyPolygon & rPolyPolygon)
{
    USHORT i,nCount;
    nCount=rPolyPolygon.Count();
    for (i=0; i<nCount; i++) {
        METLine(rPolyPolygon.GetObject(i));
        METCloseFigure();
    }
}


void METWriter::METLineAtCurPos(Point aPt)
{
    WillWriteOrder(10);
    *pMET << (BYTE)0x81 << (BYTE)8;
    WritePoint(aPt);
}


void METWriter::METBox(BOOL bFill, BOOL bBoundary,
                       Rectangle aRect, ULONG nHAxis, ULONG nVAxis)
{
    BYTE nFlags=0;
    if (bFill)     nFlags|=0x40;
    if (bBoundary) nFlags|=0x20;

    WillWriteOrder(28);
    *pMET << (BYTE)0xc0 << (BYTE)26 << nFlags << (BYTE)0;
    WritePoint(aRect.BottomLeft());
    WritePoint(aRect.TopRight());
    *pMET << nHAxis << nVAxis;
}


void METWriter::METArc(Point aP0, Point aP1, Point aP2)
{
    WillWriteOrder(26);
    *pMET << (BYTE)0xc6 << (BYTE)24;
    WritePoint(aP0);
    WritePoint(aP1);
    WritePoint(aP2);
}


void METWriter::METArcAtCurPos(Point aP1, Point aP2)
{
    WillWriteOrder(18);
    *pMET << (BYTE)0x86 << (BYTE)16;
    WritePoint(aP1);
    WritePoint(aP2);
}


void METWriter::METFullArc(Point aCenter, double fMultiplier)
{
    WillWriteOrder(14);
    *pMET << (BYTE)0xc7 << (BYTE)12;
    WritePoint(aCenter);
    *pMET << (long)(fMultiplier*65536.0+0.5);
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
    *pMET << (BYTE)0xa3 << (BYTE)20;
    WritePoint(aCenter);
    *pMET << (long)(fMultiplier*65536.0+0.5);
    *pMET << (long)(fStartAngle*65536.0+0.5);
    *pMET << (long)(fSweepAngle*65536.0+0.5);
}


void METWriter::METChrStr( Point aPt, String aUniStr )
{
    USHORT nLen,i;
    ByteString aStr( aUniStr, RTL_TEXTENCODING_UTF8 );
    nLen = aStr.Len();
    WillWriteOrder( 11 + nLen );
    *pMET << (BYTE)0xc3 << (BYTE)( 9 + nLen );
    WritePoint(aPt);
    for ( i = 0; i < nLen; i++ )
        *pMET << aStr.GetChar( i );
    *pMET << (BYTE)0;
}


void METWriter::METSetArcParams(long nP, long nQ, long nR, long nS)
{
    WillWriteOrder(18);
    *pMET << (BYTE)0x22 << (BYTE)16 << nP << nQ << nR << nS;
}


void METWriter::METSetColor(Color aColor)
{
    if (aColor==aMETColor) return;
    aMETColor=aColor;

    WillWriteOrder(6);
    *pMET << (BYTE)0xa6 << (BYTE)4 << (BYTE)0
          << (BYTE)(aColor.GetBlue())
          << (BYTE)(aColor.GetGreen())
          << (BYTE)(aColor.GetRed());
}


void METWriter::METSetBackgroundColor(Color aColor)
{
    if (aColor==aMETBackgroundColor) return;
    aMETBackgroundColor=aColor;

    WillWriteOrder(6);
    *pMET << (BYTE)0xa7 << (BYTE)4 << (BYTE)0
          << (BYTE)(aColor.GetBlue())
          << (BYTE)(aColor.GetGreen())
          << (BYTE)(aColor.GetRed());
}

void METWriter::METSetMix(RasterOp eROP)
{
    BYTE nMix;

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
    *pMET << (BYTE)0x0c << nMix;
}


void METWriter::METSetChrCellSize(Size aSize)
{
    if (aMETChrCellSize==aSize)
        return;

    aMETChrCellSize=aSize;
    WillWriteOrder(10);
    if (aSize.Width()==0) aSize.Width()=aSize.Height();
    *pMET << (BYTE)0x33 << (BYTE)8 << (long)aSize.Width() << (long)aSize.Height();
}


void METWriter::METSetChrAngle(short nAngle)
{
    double fa;
    long nax,nay;

    if (nMETChrAngle==nAngle) return;
    nMETChrAngle=nAngle;

    if (nAngle==0)
    {
        nax=256;
        nay=0;
    }
    else
    {
        fa=((double)nAngle)/1800.0*3.14159265359;
        nax=(long)(256.0*cos(fa)+0.5);
        nay=(long)(256.0*sin(fa)+0.5);
    }

    WillWriteOrder(10);
    *pMET << (BYTE)0x34 << (BYTE)8 << nax << nay;
}


void METWriter::METSetChrSet(BYTE nSet)
{
    if (nMETChrSet==nSet)
        return;

    nMETChrSet=nSet;
    WillWriteOrder(2);
    *pMET << (BYTE)0x38 << nSet;
}


void METWriter::WriteOrders( const GDIMetaFile* pMTF )
{
    if(bStatus==FALSE)
        return;

    for( ULONG nA = 0, nACount = pMTF->GetActionCount(); nA < nACount; nA++ )
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
                METBeginArea(FALSE);
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
                    METBox( TRUE, FALSE, pA->GetRect(), 0, 0 );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( FALSE, TRUE, pA->GetRect(), 0, 0 );
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
                    METBox( TRUE, FALSE, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix( eGDIRasterOp );
                    METSetColor( aGDILineColor );
                    METBox( FALSE, TRUE, pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
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
                    METBeginArea(FALSE);
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
                    METLine(pA->GetPolygon());
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

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor );
                    METSetBackgroundColor(aGDIFillColor );
                    METBeginPath(1);
                    METLine(pA->GetPolygon());
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor );
                    METBeginPath(1);
                    METLine(pA->GetPolygon());
                    METEndPath();
                    METOutlinePath(1);
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;

                if( aGDIFillColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDIFillColor);
                    METSetBackgroundColor(aGDIFillColor);
                    METBeginPath(1);
                    METLine(pA->GetPolyPolygon());
                    METEndPath();
                    METFillPath(1);
                }

                if( aGDILineColor != Color( COL_TRANSPARENT ) )
                {
                    METSetMix(eGDIRasterOp);
                    METSetColor(aGDILineColor);
                    METBeginPath(1);
                    METLine(pA->GetPolyPolygon());
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
                USHORT                      i;
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
                        METChrStr( aPt2, String( aStr.GetChar( i ) ) );
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
                USHORT                          i;
                long*                           pDXAry;
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
                pDXAry=new long[aStr.Len()];
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
                    METChrStr( aPt2, String( aStr.GetChar( i ) ) );
                }

                delete[] pDXAry;
            }
            break;

            case META_TEXTRECT_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_TEXTRECT_ACTION!" );
            }
            break;

            case META_BMP_ACTION:
            {
                const MetaBmpAction* pA = (const MetaBmpAction*) pMA;

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pA->GetBitmap().GetSizePixel(), pA->GetBitmap() );
            }
            break;

            case META_BMPSCALE_ACTION:
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pMA;

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
            }
            break;

            case META_BMPSCALEPART_ACTION:
            {
                const MetaBmpScalePartAction*   pA = (const MetaBmpScalePartAction*) pMA;
                Bitmap                          aTmp( pA->GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix( eGDIRasterOp );
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), aTmp );
            }
            break;

            case META_BMPEX_ACTION:
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction*) pMA;
                Bitmap                  aTmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), aTmp.GetSizePixel(), aTmp );
            }
            break;

            case META_BMPEXSCALE_ACTION:
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pMA;
                Bitmap                      aTmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetPoint(), pA->GetSize(), aTmp );
            }
            break;

            case META_BMPEXSCALEPART_ACTION:
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pMA;
                Bitmap                          aTmp( Graphic( pA->GetBitmapEx() ).GetBitmap() );

                aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                METSetMix(eGDIRasterOp);
                METBitBlt( pA->GetDestPoint(), pA->GetDestSize(), aTmp );
            }
            break;

            case META_EPS_ACTION :
            {
                const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                INT32 nCount = aGDIMetaFile.GetActionCount();
                for ( INT32 i = 0; i < nCount; i++ )
                {
                    const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                    if ( pMetaAct->GetType() == META_BMPSCALE_ACTION )
                    {
                        const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*)pMetaAct;
                        METSetMix(eGDIRasterOp);
                        METBitBlt( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap() );
                        break;
                    }
                }
            }
            break;

            case META_MASK_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_MASK_ACTION!" );
            }
            break;

            case META_MASKSCALE_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_MASKSCALE_ACTION!" );
            }
            break;

            case META_MASKSCALEPART_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_MASKSCALEPART_ACTION!" );
            }
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
            {
//              DBG_ERROR( "Unsupported MET-Action: META_WALLPAPER_ACTION!" );
            }
            break;

            case META_CLIPREGION_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_CLIPREGION_ACTION!" );
            }
            break;

            case META_ISECTRECTCLIPREGION_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_ISECTRECTCLIPREGION_ACTION!" );
            }
            break;

            case META_ISECTREGIONCLIPREGION_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_ISECTREGIONCLIPREGION_ACTION!" );
            }
            break;

            case META_MOVECLIPREGION_ACTION:
            {
//              DBG_ERROR( "Unsupported MET-Action: META_MOVECLIPREGION_ACTION!" );
            }
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
            {
//              DBG_ERROR( "Unsupported MET-Action: META_TEXTALIGN_ACTION!" );
            }
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
                pGS->aClipRegion=aGDIClipRegion;
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
                    aGDIClipRegion=pGS->aClipRegion;
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
        bStatus=FALSE;

      if( bStatus == FALSE )
        break;
    }
}

void METWriter::WriteObjectEnvironmentGroup(const GDIMetaFile * pMTF)
{
    ULONG i, nId;

    //--- Das Feld 'Begin Object Environment Group':
    WriteFieldIntroducer(16,BegObjEnvMagic,0,0);
    WriteFieldId(7);

    //--- Das Feld 'Map Color Attribute Table':
    WriteFieldIntroducer(22,MapColAtrMagic,0,0);
    WriteBigEndianShort(0x000e);
    *pMET << (BYTE)0x0c << (BYTE)0x02 << (BYTE)0x84 << (BYTE)0x00;
    WriteFieldId(4);

    //--- Das erste Feld 'Map Coded Font':
    WriteFieldIntroducer(32,MapCodFntMagic,0,0);
    WriteBigEndianShort(0x0018);
    *pMET << (BYTE)0x0c << (BYTE)0x02 << (BYTE)0x84 << (BYTE)0x00;
    *pMET << (BYTE)0xff << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
    *pMET << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00 << (BYTE)0x00;
    *pMET << (BYTE)0x04 << (BYTE)0x24 << (BYTE)0x05 << (BYTE)0x00;
    *pMET << (BYTE)0x06 << (BYTE)0x20;
    *pMET << (BYTE)0x03 << (BYTE)0x97 << (BYTE)0x01 << (BYTE)0xb5;

    //--- Die weiteren Felder 'Map Coded Font':
    CreateChrSets(pMTF);
    WriteChrSets();

    //--- Die Felder 'Map Data Resource':
    nId=nActBitmapId;
    for (i=0; i<nNumberOfBitmaps; i++)
    {
        WriteFieldIntroducer(29,MapDatResMagic,0,0);
        WriteBigEndianShort(0x0015);
        *pMET << (BYTE)0x0c << (BYTE)0x02 << (BYTE)0x84 << (BYTE)0x00;
        WriteFieldId(nId);
        *pMET << (BYTE)0x07 << (BYTE)0x22 << (BYTE)0x10;
        *pMET << (ULONG)nId;
        nId++;
    }

    //--- Das Feld 'End Object Environment Group':
    WriteFieldIntroducer(16,EndObjEnvMagic,0,0);
    WriteFieldId(7);
}


void METWriter::WriteGraphicsObject(const GDIMetaFile * pMTF)
{
    ULONG nSegmentSize,nPos,nDataFieldsStartPos;

    if( bStatus==FALSE )
        return;

    //--- Das Feld 'Begin Graphics Object':
    WriteFieldIntroducer(16,BegGrfObjMagic,0,0);
    WriteFieldId(7);

    // Map Color Attribute Table, Fonts und anderes:
    WriteObjectEnvironmentGroup(pMTF);

    //--- Das Feld 'Graphics Data Descriptor':
    WriteDataDescriptor(pMTF);

    // Zaehler fuer Data Fields initialisieren:
    nNumberOfDataFields=0;

    // Und Position des ersten Data Fields merken:
    nDataFieldsStartPos=pMET->Tell();

    //--- Anfang des ersten Feldes 'Graphics Data'
    WriteFieldIntroducer(0,DatGrfObjMagic,0,0);
    nNumberOfDataFields++;

    // Nun schreiben wir zunaechst den Kopf des Segments:
    *pMET << (BYTE)0x70 << (BYTE)0x0e << (ULONG)0;
    *pMET << (BYTE)0x70 << (BYTE)0x10; // Flags
    *pMET << (USHORT)0; // Lo-Wort der Laenge der Segementdaten (Big Endian)
    *pMET << (ULONG)0;  // Reserved
    *pMET << (USHORT)0; // Hi-Wort der Laenge der Segementdaten (Big Endian) (Ohh Ohh OS2)
    // Anmerkung: die richtige Daten-Laenge schreiben wir weiter unten nochmal

    // Jetzt werden alle Orders rausgeschrieben:
    // (wobei die Sache ggf. in mehrere 'Graphics Data Fields' aufgeteilt
    // wird, per Methode WillWriteOrder(..))
    WriteOrders(pMTF);

    //--- Das letzte Feld 'Graphic Data' beenden:
    UpdateFieldSize();

    //--- Und schliesslich die Segmentgroesse richtigstellen:
    nPos=pMET->Tell();
    nSegmentSize=nPos-nDataFieldsStartPos;
    nSegmentSize-=nNumberOfDataFields*8; // Structured Field Introducers zaehlen nicht mit
    pMET->Seek(nDataFieldsStartPos+16); // Zum Lo-Wort der Segmentgroesse seeken
    WriteBigEndianShort((USHORT)(nSegmentSize&0x0000ffff)); // Und schreiben
    pMET->Seek(nDataFieldsStartPos+22); // Zum Hi-Wort der Segmentgroesse seeken
    WriteBigEndianShort((USHORT)(nSegmentSize>>16)); // Und schreiben
    pMET->Seek(nPos); // Zurueck zur Tagesordnung

    //--- Das Feld 'End Graphic Objects':
    WriteFieldIntroducer(16,EndGrfObjMagic,0,0);
    WriteFieldId(7);

    if( pMET->GetError() )
        bStatus=FALSE;
}


void METWriter::WriteResourceGroup(const GDIMetaFile * pMTF)
{
    if( bStatus==FALSE )
        return;

    //--- Das Feld 'Begin Resource Group':
    WriteFieldIntroducer(16,BegResGrpMagic,0,0);
    WriteFieldId(2);

    //--- Der Inhalt:
    WriteColorAttributeTable();
    nActBitmapId=0x77777700;
    WriteImageObjects(pMTF);
    nActBitmapId=0x77777700;
    WriteGraphicsObject(pMTF);

    //--- Das Feld 'End Resource Group':
    WriteFieldIntroducer(16,EndResGrpMagic,0,0);
    WriteFieldId(2);

    if( pMET->GetError() )
        bStatus=FALSE;
}


void METWriter::WriteDocument(const GDIMetaFile * pMTF)
{
    if( bStatus==FALSE )
        return;

    //--- Das Feld 'Begin Document':
    WriteFieldIntroducer(0,BegDocumnMagic,0,0);
    WriteFieldId(1);
    *pMET << (BYTE)0x00 << (BYTE)0x00;
    *pMET << (BYTE)0x05 << (BYTE)0x18 << (BYTE)0x03 << (BYTE)0x0c << (BYTE)0x00;
    *pMET << (BYTE)0x06 << (BYTE)0x01 << (BYTE)0x03 << (BYTE)0xd4 << (BYTE)0x03 << (BYTE)0x52;
    *pMET << (BYTE)0x03 << (BYTE)0x65 << (BYTE)0x00;
    UpdateFieldSize();

    //--- Der Inhalt:
    WriteResourceGroup(pMTF);

    //--- Das Feld 'End Document':
    WriteFieldIntroducer(16,EndDocumnMagic,0,0);
    WriteFieldId(1);

    if( pMET->GetError() )
        bStatus=FALSE;
}


BOOL METWriter::WriteMET( const GDIMetaFile& rMTF, SvStream& rTargetStream,
                          PFilterCallback pcallback, void* pcallerdata )
{
    METChrSet*          pCS;
    METGDIStackMember*  pGS;

    bStatus=TRUE;

    pCallback=pcallback;
    pCallerData=pcallerdata;
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
    aGDIClipRegion=Region();
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

    return bStatus;
}

//================== GraphicExport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#else
extern "C" BOOL GraphicExport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#endif
{
    METWriter aMETWriter;

    if (rGraphic.GetType()==GRAPHIC_GDIMETAFILE) {
        return aMETWriter.WriteMET(rGraphic.GetGDIMetaFile(),rStream,pCallback,pCallerData);
    }
    else {
        Bitmap aBmp=rGraphic.GetBitmap();
        GDIMetaFile aMTF;
        VirtualDevice aVirDev;

        aMTF.Record(&aVirDev);
        aVirDev.DrawBitmap(Point(),aBmp);
        aMTF.Stop();
        aMTF.SetPrefSize(aBmp.GetSizePixel());
        return aMETWriter.WriteMET(aMTF,rStream,pCallback,pCallerData);
    }
}

//================== GraphicDialog - die exportierte Funktion ================

extern "C" BOOL SAL_CALL DoExportDialog( FltCallDialogParameter& rPara )
{
    BOOL    bRet = FALSE;

    if ( rPara.pWindow && rPara.pCfg )
    {
        ByteString aResMgrName( "eme" );
        ResMgr* pResMgr;

        aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
        pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );

        if( pResMgr )
        {
            rPara.pResMgr = pResMgr;
            bRet = ( DlgExportEMET( rPara ).Execute() == RET_OK );
            delete pResMgr;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

//================== ein bischen Muell fuer Windows ==========================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif
