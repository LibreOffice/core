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


#include <iterator>
#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <editeng/opaqitem.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <grfatr.hxx>           // class SwCropGrf
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <fltshell.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>            // class SwTxtNode
#include <mdiexp.hxx>           // Progress
#include "writerwordglue.hxx"
#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "ww8par.hxx"           // class SwWWImplReader
#include "ww8par2.hxx"          // struct WWFlyPara
#include "ww8graf.hxx"
#include <svtools/filter.hxx>

using namespace ::com::sun::star;
using namespace sw::types;

wwZOrderer::wwZOrderer(const sw::util::SetLayer &rSetLayer, SdrPage* pDrawPg,
    const SvxMSDffShapeOrders *pShapeOrders)
    : maSetLayer(rSetLayer), mnInlines(0), mpDrawPg(pDrawPg),
    mpShapeOrders(pShapeOrders)
{
    mnNoInitialObjects = mpDrawPg->GetObjCount();
    OSL_ENSURE(mpDrawPg,"Missing draw page impossible!");
}

void wwZOrderer::InsideEscher(sal_uLong nSpId)
{
    maIndexes.push(GetEscherObjectIdx(nSpId));
}

void wwZOrderer::OutsideEscher()
{
    maIndexes.pop();
}

// consider new parameter <_bInHeaderFooter>
void wwZOrderer::InsertEscherObject( SdrObject* pObject,
                                     sal_uLong nSpId,
                                     const bool _bInHeaderFooter )
{
    sal_uLong nInsertPos = GetEscherObjectPos( nSpId, _bInHeaderFooter );
    InsertObject(pObject, nInsertPos + mnNoInitialObjects + mnInlines);
}

wwZOrderer::myeiter wwZOrderer::MapEscherIdxToIter(sal_uLong nIdx)
{
    myeiter aIter = maEscherLayer.begin();
    myeiter aEnd = maEscherLayer.end();
    while (aIter != aEnd)
    {
        if (aIter->mnEscherShapeOrder == nIdx)
            break;
        ++aIter;
    }
    return aIter;
}

sal_uInt16 wwZOrderer::GetEscherObjectIdx(sal_uLong nSpId)
{
    sal_uInt16 nFound=0;
    sal_uInt16 nShapeCount = mpShapeOrders ? mpShapeOrders->size() : 0;
    // First, find out what position this shape is in in the Escher order.
    for (sal_uInt16 nShapePos=0; nShapePos < nShapeCount; nShapePos++)
    {
        const SvxMSDffShapeOrder& rOrder = *(*mpShapeOrders)[nShapePos];
        if (rOrder.nShapeId == nSpId)
        {
            nFound = nShapePos;
            break;
        }
    }
    return nFound;
}

// consider new parameter <_bInHeaderFooter>
sal_uLong wwZOrderer::GetEscherObjectPos( sal_uLong nSpId,
                                      const bool _bInHeaderFooter )
{
    /*
    EscherObjects have their own ordering which needs to be matched to
    the actual ordering that should be used when inserting them into the
    document.
    */
    sal_uInt16 nFound = GetEscherObjectIdx(nSpId);
    // Match the ordering position from the ShapeOrders to the ordering of all
    // objects in the document, there is a complexity when escherobjects
    // contain inlines objects, we need to consider thsose as part of the
    // escher count
    sal_uLong nRet=0;
    myeiter aIter = maEscherLayer.begin();
    myeiter aEnd = maEscherLayer.end();
    // skip objects in page header|footer,
    // if current object isn't in page header|footer
    if ( !_bInHeaderFooter )
    {
        while ( aIter != aEnd )
        {
            if ( !aIter->mbInHeaderFooter )
            {
                break;
            }
            nRet += aIter->mnNoInlines + 1;
            ++aIter;
        }
    }
    while (aIter != aEnd)
    {
        // insert object in page header|footer
        // before objects in page body
        if ( _bInHeaderFooter && !aIter->mbInHeaderFooter )
        {
            break;
        }
        if ( aIter->mnEscherShapeOrder > nFound )
            break;
        nRet += aIter->mnNoInlines+1;
        ++aIter;
    }
    maEscherLayer.insert(aIter, EscherShape( nFound, _bInHeaderFooter ) );
    return nRet;
}

// InsertObj() fuegt das Objekt in die Sw-Page ein und merkt sich die Z-Pos in
// einem VarArr
void wwZOrderer::InsertDrawingObject(SdrObject* pObj, short nWwHeight)
{
    sal_uLong nPos = GetDrawingObjectPos(nWwHeight);
    if (nWwHeight & 0x2000)                 // Heaven ?
        maSetLayer.SendObjectToHeaven(*pObj);
    else
        maSetLayer.SendObjectToHell(*pObj);

    InsertObject(pObj, nPos + mnNoInitialObjects + mnInlines);
}

void wwZOrderer::InsertTextLayerObject(SdrObject* pObject)
{
    maSetLayer.SendObjectToHeaven(*pObject);
    if (maIndexes.empty())
    {
        InsertObject(pObject, mnNoInitialObjects + mnInlines);
        ++mnInlines;
    }
    else
    {
        //If we are inside an escher objects, place us just after that
        //escher obj, and increment its inline count
        sal_uInt16 nIdx = maIndexes.top();
        myeiter aEnd = MapEscherIdxToIter(nIdx);

        sal_uLong nInsertPos=0;
        myeiter aIter = maEscherLayer.begin();
        while (aIter != aEnd)
        {
            nInsertPos += aIter->mnNoInlines+1;
            ++aIter;
        }

        OSL_ENSURE(aEnd != maEscherLayer.end(), "Something very wrong here");
        if (aEnd != maEscherLayer.end())
        {
            aEnd->mnNoInlines++;
            nInsertPos += aEnd->mnNoInlines;
        }

        InsertObject(pObject, mnNoInitialObjects + mnInlines + nInsertPos);
    }
}

// Parallel zu dem Obj-Array im Dokument baue ich ein Array auf,
// dass die Ww-Height ( -> Wer ueberdeckt wen ) beinhaltet.
// Anhand dieses VARARR wird die Einfuegeposition ermittelt.
// Der Offset bei Datei in bestehendes Dokument mit Grafiklayer einfuegen
// muss der Aufrufer den Index um mnNoInitialObjects erhoeht werden, damit die
// neuen Objekte am Ende landen ( Einfuegen ist dann schneller )
sal_uLong wwZOrderer::GetDrawingObjectPos(short nWwHeight)
{
    myditer aIter = maDrawHeight.begin();
    myditer aEnd = maDrawHeight.end();

    while (aIter != aEnd)
    {
        if ((*aIter & 0x1fff) > (nWwHeight & 0x1fff))
            break;
        ++aIter;
    }

    aIter = maDrawHeight.insert(aIter, nWwHeight);
    return std::distance(maDrawHeight.begin(), aIter);
}

bool wwZOrderer::InsertObject(SdrObject* pObject, sal_uLong nPos)
{
    if (!pObject->IsInserted())
    {
        mpDrawPg->InsertObject(pObject, nPos);
        return true;
    }
    return false;
}

#ifdef __WW8_NEEDS_COPY
extern void WW8PicShadowToReal(  WW8_PIC_SHADOW*  pPicS,  WW8_PIC*  pPic );
#endif // defined __WW8_NEEDS_COPY

bool SwWW8ImplReader::GetPictGrafFromStream(Graphic& rGraphic, SvStream& rSrc)
{
    return 0 == GraphicFilter::GetGraphicFilter().ImportGraphic(rGraphic, aEmptyStr, rSrc,
        GRFILTER_FORMAT_DONTKNOW);
}

bool SwWW8ImplReader::ReadGrafFile(String& rFileName, Graphic*& rpGraphic,
    const WW8_PIC& rPic, SvStream* pSt, sal_uLong nFilePos, bool* pbInDoc)
{                                                  // Grafik in File schreiben
    *pbInDoc = true;                               // default

    sal_uLong nPosFc = nFilePos + rPic.cbHeader;

    switch (rPic.MFP.mm)
    {
        case 94: // BMP-File ( nicht embeddet ) oder GIF
        case 99: // TIFF-File ( nicht embeddet )
            pSt->Seek(nPosFc);
            // Name als P-String einlesen
            rFileName = read_uInt8_PascalString(*pSt, eStructCharSet);
            if (rFileName.Len())
                rFileName = URIHelper::SmartRel2Abs(
                    INetURLObject(sBaseURL), rFileName,
                    URIHelper::GetMaybeFileHdl());
            *pbInDoc = false;       // Datei anschliessend nicht loeschen
            return rFileName.Len() != 0;        // Einlesen OK
    }

    GDIMetaFile aWMF;
    pSt->Seek( nPosFc );
    bool bOk = ReadWindowMetafile( *pSt, aWMF, NULL ) ? true : false;

    if (!bOk || pSt->GetError() || !aWMF.GetActionSize())
        return false;

    if (pWwFib->envr != 1) // !MAC als Creator
    {
        rpGraphic = new Graphic( aWMF );
        return true;
    }

    // MAC - Word als Creator
    // im WMF steht nur "Benutzen sie Word 6.0c" Mac-Pict steht dahinter
    // allerdings ohne die ersten 512 Bytes, bei einem MAC-PICT egal sind (
    // werden nicht ausgewertet )
    bOk = false;
    long nData = rPic.lcb - ( pSt->Tell() - nPosFc );
    if (nData > 0)
    {
        rpGraphic = new Graphic();
        if (0 == (bOk = SwWW8ImplReader::GetPictGrafFromStream(*rpGraphic, *pSt)))
            DELETEZ(rpGraphic);
    }
    return bOk; // Grafik drin
}

struct WW8PicDesc
{
    sal_Int16 nCL, nCR, nCT, nCB;
    long nWidth, nHeight;

    WW8PicDesc( const WW8_PIC& rPic );
};

WW8PicDesc::WW8PicDesc( const WW8_PIC& rPic )
{
    //See #i21190# before fiddling with this method
    long nOriWidth = rPic.dxaGoal;        //Size in 1/100 mm before crop
    long nOriHeight = rPic.dyaGoal;

    nCL = rPic.dxaCropLeft;
    nCR = rPic.dxaCropRight;
    nCT = rPic.dyaCropTop;
    nCB = rPic.dyaCropBottom;

    long nAktWidth  = nOriWidth - (nCL + nCR);  // Size after crop
    long nAktHeight = nOriHeight - (nCT + nCB);
    if (!nAktWidth)
        nAktWidth  = 1;
    if (!nAktHeight)
        nAktHeight = 1;
    nWidth = nAktWidth * rPic.mx / 1000;        // Writer Size
    nHeight = nAktHeight * rPic.my / 1000;
}

void SwWW8ImplReader::ReplaceObj(const SdrObject &rReplaceObj,
    SdrObject &rSubObj)
{
    // SdrGrafObj anstatt des SdrTextObj in dessen Gruppe einsetzen
    if (SdrObject* pGroupObject = rReplaceObj.GetUpGroup())
    {
        SdrObjList* pObjectList = pGroupObject->GetSubList();

        rSubObj.SetLogicRect(rReplaceObj.GetCurrentBoundRect());
        rSubObj.SetLayer(rReplaceObj.GetLayer());

        // altes Objekt raus aus Gruppen-Liste und neues rein
        // (dies tauscht es ebenfalls in der Drawing-Page aus)
        pObjectList->ReplaceObject(&rSubObj, rReplaceObj.GetOrdNum());
    }
    else
    {
        OSL_ENSURE( !this, "Impossible!");
    }
}

// MakeGrafNotInCntnt setzt eine nicht-Zeichengebundene Grafik
// ( bGrafApo == true)
SwFlyFrmFmt* SwWW8ImplReader::MakeGrafNotInCntnt(const WW8PicDesc& rPD,
    const Graphic* pGraph, const String& rFileName, const SfxItemSet& rGrfSet)
{

    sal_uInt32 nWidth = rPD.nWidth;
    sal_uInt32 nHeight = rPD.nHeight;

    // Vertikale Verschiebung durch Zeilenabstand
    sal_Int32 nNetHeight = nHeight + rPD.nCT + rPD.nCB;
    if( pSFlyPara->nLineSpace && pSFlyPara->nLineSpace > nNetHeight )
        pSFlyPara->nYPos =
            (sal_uInt16)( pSFlyPara->nYPos + pSFlyPara->nLineSpace - nNetHeight );

    WW8FlySet aFlySet(*this, pWFlyPara, pSFlyPara, true);

    SwFmtAnchor aAnchor(pSFlyPara->eAnchor);
    aAnchor.SetAnchor(pPaM->GetPoint());
    aFlySet.Put(aAnchor);

    aFlySet.Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth, nHeight ) );

    SwFlyFrmFmt* pFlyFmt = rDoc.Insert(*pPaM, rFileName, aEmptyStr, pGraph,
        &aFlySet, &rGrfSet, NULL);

    // Damit die Frames bei Einfuegen in existierendes Doc erzeugt werden:
    if (rDoc.GetCurrentViewShell() &&   //swmod 071108//swmod 071225
        (FLY_AT_PARA == pFlyFmt->GetAnchor().GetAnchorId()))
    {
        pFlyFmt->MakeFrms();
    }
    return pFlyFmt;
}


// MakeGrafInCntnt fuegt zeichengebundene Grafiken ein
SwFrmFmt* SwWW8ImplReader::MakeGrafInCntnt(const WW8_PIC& rPic,
    const WW8PicDesc& rPD, const Graphic* pGraph, const String& rFileName,
    const SfxItemSet& rGrfSet)
{
    WW8FlySet aFlySet(*this, pPaM, rPic, rPD.nWidth, rPD.nHeight);

    SwFrmFmt* pFlyFmt = 0;

    if (!rFileName.Len() && nObjLocFc)      // dann sollte ists ein OLE-Object
        pFlyFmt = ImportOle(pGraph, &aFlySet, &rGrfSet);

    if( !pFlyFmt )                          // dann eben als Graphic
    {

        pFlyFmt = rDoc.Insert( *pPaM, rFileName, aEmptyStr, pGraph, &aFlySet,
            &rGrfSet, NULL);
    }

    // Grafik im Rahmen ? ok, Rahmen auf Bildgroesse vergroessern
    //  ( nur wenn Auto-Breite )
    if( pSFlyPara )
        pSFlyPara->BoxUpWidth( rPD.nWidth );
    return pFlyFmt;
}

SwFrmFmt* SwWW8ImplReader::ImportGraf1(WW8_PIC& rPic, SvStream* pSt,
    sal_uLong nFilePos )
{
    SwFrmFmt* pRet = 0;
    if( pSt->IsEof() || rPic.fError || rPic.MFP.mm == 99 )
        return 0;

    String aFileName;
    bool bInDoc;
    Graphic* pGraph = 0;
    bool bOk = ReadGrafFile(aFileName, pGraph, rPic, pSt, nFilePos, &bInDoc);

    if (!bOk)
    {
        delete pGraph;
        return 0;                       // Grafik nicht korrekt eingelesen
    }

    WW8PicDesc aPD( rPic );

    SwAttrSet aGrfSet( rDoc.GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);
    if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
    {
        SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB) ;
        aGrfSet.Put( aCrop );
    }

    if( pWFlyPara && pWFlyPara->bGrafApo )
        pRet = MakeGrafNotInCntnt(aPD,pGraph,aFileName,aGrfSet);
    else
        pRet = MakeGrafInCntnt(rPic,aPD,pGraph,aFileName,aGrfSet);
    delete pGraph;
    return pRet;
}

void SwWW8ImplReader::PicRead(SvStream *pDataStream, WW8_PIC *pPic,
    bool bVer67)
{
    //Only the first 0x2e bytes are the same between version 6/7 and 8+
#ifdef __WW8_NEEDS_COPY
    WW8_PIC_SHADOW aPicS;
    pDataStream->Read( &aPicS, sizeof( aPicS ) );
    WW8PicShadowToReal( &aPicS, pPic );
#else
    pDataStream->Read( pPic, 0x2E);
#endif // defined __WW8_NEEDS_COPY
    for (int i=0;i<4;i++)
        pDataStream->Read( &pPic->rgbrc[i], bVer67 ? 2 : 4);
    *pDataStream >> pPic->dxaOrigin;
    *pDataStream >> pPic->dyaOrigin;
    if (!bVer67)
        pDataStream->SeekRel(2);  //cProps
}

SwFrmFmt* SwWW8ImplReader::ImportGraf(SdrTextObj* pTextObj,
    SwFrmFmt* pOldFlyFmt)
{
    SwFrmFmt* pRet = 0;
    if (
        ((pStrm == pDataStream ) && !nPicLocFc) ||
        (nIniFlags & WW8FL_NO_GRAF)
       )
    {
        return 0;
    }

    ::SetProgressState(nProgress, mpDocShell);         // Update

    GrafikCtor();

    /*
        kleiner Spass von Microsoft: manchmal existiert ein Stream Namens DATA
        Dieser enthaelt dann den PICF und die entsprechende Grafik !!!
        Wir mappen ansonsten die Variable pDataStream auf pStream.
    */

    sal_uLong nOldPos = pDataStream->Tell();
    WW8_PIC aPic;
    pDataStream->Seek( nPicLocFc );
    PicRead( pDataStream, &aPic, bVer67);

        // Plausibilitaetstest ist noetig, da z.B. bei CheckBoxen im
        // Feld-Result ein WMF-aehnliches Struct vorkommt.
    if ((aPic.lcb >= 58) && !pDataStream->GetError())
    {
        if( pFlyFmtOfJustInsertedGraphic )
        {
            // Soeben haben wir einen Grafik-Link ins Doc inserted.
            // Wir muessen ihn jetzt noch Positioniern und Skalieren.
            //
            WW8PicDesc aPD( aPic );

            WW8FlySet aFlySet( *this, pPaM, aPic, aPD.nWidth, aPD.nHeight );

            // the correct anchor is set in Read_F_IncludePicture and the current PaM point's
            // behind the position if it is anchored in content; because this anchor add
            // a character into the textnode. IussueZilla task 2806
            if (FLY_AS_CHAR ==
                pFlyFmtOfJustInsertedGraphic->GetAnchor().GetAnchorId() )
            {
                aFlySet.ClearItem( RES_ANCHOR );
            }

            pFlyFmtOfJustInsertedGraphic->SetFmtAttr( aFlySet );

            pFlyFmtOfJustInsertedGraphic = 0;
        }
        else if((0x64 == aPic.MFP.mm) || (0x66 == aPic.MFP.mm))
        {
            // verlinkte Grafik im Escher-Objekt
            SdrObject* pObject = 0;

            WW8PicDesc aPD( aPic );
            if (!pMSDffManager)
                pMSDffManager = new SwMSDffManager(*this);
            /*
            ##835##
            Disable use of main stream as fallback stream for inline direct
            blips as it is known that they are directly after the record
            header, testing for existance in main stream may lead to an
            incorrect fallback graphic being found if other escher graphics
            have been inserted in the document
            */
            pMSDffManager->DisableFallbackStream();
            if( !pMSDffManager->GetModel() )
                pMSDffManager->SetModel(pDrawModel, 1440);

            if (0x66 == aPic.MFP.mm)
            {
                //These ones have names prepended
                sal_uInt8 nNameLen=0;
                *pDataStream >> nNameLen;
                pDataStream->SeekRel( nNameLen );
            }

            Rectangle aChildRect;
            Rectangle aClientRect( 0,0, aPD.nWidth,  aPD.nHeight);
            SvxMSDffImportData aData( aClientRect );
            pObject = pMSDffManager->ImportObj(*pDataStream, &aData, aClientRect, aChildRect );
            if (pObject)
            {
                // fuer den Rahmen
                SfxItemSet aAttrSet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN,
                    RES_FRMATR_END-1 );

                SvxMSDffImportRec const*const pRecord = (1 == aData.size())
                    ? &*aData.begin() : 0;

                if( pRecord )
                {

                    // Horizontal rule may have its width given as % of page width
                    // (-1 is used if not given, 0 means the object has fixed width).
                    // Additionally, if it's a horizontal rule without width given,
                    // assume 100.0% width.
                    int relativeWidth = pRecord->relativeHorizontalWidth;
                    if( relativeWidth == -1 )
                        relativeWidth = pRecord->isHorizontalRule ? 1000 : 0;
                    if( relativeWidth != 0 )
                    {
                        aPic.mx = msword_cast<sal_uInt16>(
                            maSectionManager.GetPageWidth() -
                            maSectionManager.GetPageRight() -
                            maSectionManager.GetPageLeft()) * relativeWidth / 1000;
                        aPD = WW8PicDesc( aPic );
                        // This SetSnapRect() call adjusts the size of the object itself,
                        // no idea why it's this call (or even what the call actually does),
                        // but that's what ImportGraf() (called by ImportObj()) uses.
                        pObject->SetSnapRect( Rectangle( 0, 0, aPD.nWidth, aPD.nHeight ));
                    }

                    //A graphic of this type in this location is always
                    //inline, and uses the pic in the same mould as ww6
                    //graphics.
                    if (pWFlyPara && pWFlyPara->bGrafApo)
                    {
                        WW8FlySet aFlySet(*this, pWFlyPara, pSFlyPara, true);

                        SwFmtAnchor aAnchor(pSFlyPara->eAnchor);
                        aAnchor.SetAnchor(pPaM->GetPoint());
                        aFlySet.Put(aAnchor);

                        aAttrSet.Put(aFlySet);
                    }
                    else
                    {
                        WW8FlySet aFlySet( *this, pPaM, aPic, aPD.nWidth,
                            aPD.nHeight );

                        aAttrSet.Put(aFlySet);
                    }

                    Rectangle aInnerDist(   pRecord->nDxTextLeft,
                        pRecord->nDyTextTop, pRecord->nDxTextRight,
                        pRecord->nDyTextBottom  );

                    MatchSdrItemsIntoFlySet( pObject, aAttrSet,
                        pRecord->eLineStyle, pRecord->eLineDashing,
                        pRecord->eShapeType, aInnerDist );

                    //Groesse aus der WinWord PIC-Struktur als
                    //Grafik-Groesse nehmen
                    aAttrSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPD.nWidth,
                        aPD.nHeight ) );
                }

                // for the Grafik
                SfxItemSet aGrSet( rDoc.GetAttrPool(), RES_GRFATR_BEGIN,
                    RES_GRFATR_END-1 );

                if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
                {
                    SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB );
                    aGrSet.Put( aCrop );
                }

                if (pRecord)
                    MatchEscherMirrorIntoFlySet(*pRecord, aGrSet);

                // ggfs. altes AttrSet uebernehmen und
                // horiz. Positionierungs-Relation korrigieren
                if( pOldFlyFmt )
                {
                    aAttrSet.Put( pOldFlyFmt->GetAttrSet() );
                    const SwFmtHoriOrient &rHori = pOldFlyFmt->GetHoriOrient();
                    if( text::RelOrientation::FRAME == rHori.GetRelationOrient() )
                    {
                        aAttrSet.Put( SwFmtHoriOrient( rHori.GetPos(),
                            text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ) );
                    }
                }

                bool bTextObjWasGrouped = false;
                if (pOldFlyFmt && pTextObj && pTextObj->GetUpGroup())
                    bTextObjWasGrouped = true;

                if (bTextObjWasGrouped)
                    ReplaceObj(*pTextObj, *pObject);
                else
                {
                    if (sal_uInt16(OBJ_OLE2) == pObject->GetObjIdentifier())
                    {
                        // the size from BLIP, if there is any, should be already set
                        pRet = InsertOle(*((SdrOle2Obj*)pObject), aAttrSet, aGrSet);
                    }
                    else
                    {
                        if (SdrGrafObj* pGraphObject = PTR_CAST(SdrGrafObj, pObject))
                        {
                            // Nun den Link bzw. die Grafik ins Doc stopfen
                            const Graphic& rGraph = pGraphObject->GetGraphic();

                            if (nObjLocFc)  // is it a OLE-Object?
                                pRet = ImportOle(&rGraph, &aAttrSet, &aGrSet, pObject->GetBLIPSizeRectangle());

                            if (!pRet)
                            {
                                pRet = rDoc.Insert(*pPaM, aEmptyStr, aEmptyStr,
                                    &rGraph, &aAttrSet, &aGrSet, NULL );
                            }
                        }
                        else
                            pRet = rDoc.Insert(*pPaM, *pObject, &aAttrSet, NULL);
                    }
                }

                // also nur, wenn wir ein *Insert* gemacht haben
                if (pRet)
                {
                    if (pRecord)
                        SetAttributesAtGrfNode(pRecord, pRet, 0);

                    // #i68101#
                    // removed pObject->HasSetName() usage since always returned true,
                    // also removed else-part and wrote an informing mail to Henning Brinkmann
                    // about this to clarify.
                    pRet->SetName(pObject->GetName());

                    // Zeiger auf neues Objekt ermitteln und Z-Order-Liste
                    // entsprechend korrigieren (oder Eintrag loeschen)
                    if (SdrObject* pOurNewObject = CreateContactObject(pRet))
                    {
                        if (pOurNewObject != pObject)
                        {
                            pMSDffManager->ExchangeInShapeOrder( pObject, 0, 0,
                                pOurNewObject );

                            // altes SdrGrafObj aus der Page loeschen und
                            // zerstoeren
                            if (pObject->GetPage())
                                pDrawPg->RemoveObject(pObject->GetOrdNum());
                            SdrObject::Free( pObject );
                        }
                    }
                    else
                        pMSDffManager->RemoveFromShapeOrder( pObject );
                }
                else
                    pMSDffManager->RemoveFromShapeOrder( pObject );

                // auch das ggfs.  Page loeschen, falls nicht gruppiert,
                if (pTextObj && !bTextObjWasGrouped && pTextObj->GetPage())
                    pDrawPg->RemoveObject( pTextObj->GetOrdNum() );
            }
            pMSDffManager->EnableFallbackStream();
        }
        else if (aPic.lcb >= 58)
            pRet = ImportGraf1(aPic, pDataStream, nPicLocFc);
    }
    pDataStream->Seek( nOldPos );

    if (pRet)
    {
        SdrObject* pOurNewObject = CreateContactObject(pRet);
        pWWZOrder->InsertTextLayerObject(pOurNewObject);
    }

    return AddAutoAnchor(pRet);
}

#ifdef __WW8_NEEDS_COPY

void WW8PicShadowToReal( WW8_PIC_SHADOW * pPicS, WW8_PIC * pPic )
{
    pPic->lcb = SVBT32ToUInt32( pPicS->lcb );
    pPic->cbHeader = SVBT16ToShort( pPicS->cbHeader );
    pPic->MFP.mm = SVBT16ToShort( pPicS->MFP.mm );
    pPic->MFP.xExt = SVBT16ToShort( pPicS->MFP.xExt );
    pPic->MFP.yExt = SVBT16ToShort( pPicS->MFP.yExt );
    pPic->MFP.hMF = SVBT16ToShort( pPicS->MFP.hMF );
    for( sal_uInt16 i = 0; i < 14 ; i++ )
        pPic->rcWinMF[i] = SVBT8ToByte( pPicS->rcWinMF[i] );
    pPic->dxaGoal = SVBT16ToShort( pPicS->dxaGoal );
    pPic->dyaGoal = SVBT16ToShort( pPicS->dyaGoal );
    pPic->mx = SVBT16ToShort( pPicS->mx );
    pPic->my = SVBT16ToShort( pPicS->my );
    pPic->dxaCropLeft = SVBT16ToShort( pPicS->dxaCropLeft );
    pPic->dyaCropTop = SVBT16ToShort( pPicS->dyaCropTop );
    pPic->dxaCropRight = SVBT16ToShort( pPicS->dxaCropRight );
    pPic->dyaCropBottom = SVBT16ToShort( pPicS->dyaCropBottom );
    pPic->brcl = pPicS->aBits1[0] & 0x0f;
    pPic->fFrameEmpty = (pPicS->aBits1[0] & 0x10) >> 4;
    pPic->fBitmap = (pPicS->aBits1[0] & 0x20) >> 5;
    pPic->fDrawHatch = (pPicS->aBits1[0] & 0x40) >> 6;
    pPic->fError = (pPicS->aBits1[0] & 0x80) >> 7;
    pPic->bpp = pPicS->aBits2[0];
}

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW * pFSPAS, WW8_FSPA * pFSPA )
{
    pFSPA->nSpId        = SVBT32ToUInt32( pFSPAS->nSpId );
    pFSPA->nXaLeft      = SVBT32ToUInt32( pFSPAS->nXaLeft );
    pFSPA->nYaTop       = SVBT32ToUInt32( pFSPAS->nYaTop );
    pFSPA->nXaRight     = SVBT32ToUInt32( pFSPAS->nXaRight );
    pFSPA->nYaBottom    = SVBT32ToUInt32( pFSPAS->nYaBottom );

    sal_uInt16 nBits        = SVBT16ToShort( pFSPAS->aBits1 );

    pFSPA->bHdr         = 0 !=  ( nBits & 0x0001 );
    pFSPA->nbx          =       ( nBits & 0x0006 ) >> 1;
    pFSPA->nby          =       ( nBits & 0x0018 ) >> 3;
    pFSPA->nwr          =       ( nBits & 0x01E0 ) >> 5;
    pFSPA->nwrk         =       ( nBits & 0x1E00 ) >> 9;
    pFSPA->bRcaSimple   = 0 !=  ( nBits & 0x2000 );
    pFSPA->bBelowText   = 0 !=  ( nBits & 0x4000 );
    pFSPA->bAnchorLock  = 0 !=  ( nBits & 0x8000 );
    pFSPA->nTxbx = SVBT32ToUInt32( pFSPAS->nTxbx );
}
#endif // defined __WW8_NEEDS_COPY

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
