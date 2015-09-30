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
#include <grfatr.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <fltshell.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>
#include "writerwordglue.hxx"
#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"
#include "ww8graf.hxx"
#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>

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
    // First, find out what position this shape is in the Escher order.
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
    // contain inlines objects, we need to consider those as part of the
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

// InsertObj() adds the object into the Sw-Page and memorize the Z-position
// in a VarArr
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

/* Parallel to the Obj-array in the document I also build an array which
 * contains the Ww-height (-> what covers what).
 * Based on this VARARR the position where the insertion happens is
 * determined.
 * When inserting the offset in an existing document with a graphic layer the
 * caller has to increment the index by mnNoInitialObjects, so that the new
 * objects are added at the end (inserting is faster then)
 */
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

extern void WW8PicShadowToReal(  WW8_PIC_SHADOW*  pPicS,  WW8_PIC*  pPic );

bool SwWW8ImplReader::GetPictGrafFromStream(Graphic& rGraphic, SvStream& rSrc)
{
    return 0 == GraphicFilter::GetGraphicFilter().ImportGraphic(rGraphic, OUString(), rSrc,
        GRFILTER_FORMAT_DONTKNOW);
}

bool SwWW8ImplReader::ReadGrafFile(OUString& rFileName, Graphic*& rpGraphic,
    const WW8_PIC& rPic, SvStream* pSt, sal_uLong nFilePos, bool* pbInDoc)
{                                                  // Write the graphic to the file
    *pbInDoc = true;                               // default

    sal_uLong nPosFc = nFilePos + rPic.cbHeader;

    switch (rPic.MFP.mm)
    {
        case 94: // BMP-file ( not embedded ) or GIF
        case 99: // TIFF-file ( not embedded )
            pSt->Seek(nPosFc);
            // read name as P-string
            rFileName = read_uInt8_PascalString(*pSt, m_eStructCharSet);
            if (!rFileName.isEmpty())
                rFileName = URIHelper::SmartRel2Abs(
                    INetURLObject(m_sBaseURL), rFileName,
                    URIHelper::GetMaybeFileHdl());
            *pbInDoc = false;       // Don't delete the file afterwards
            return !rFileName.isEmpty();        // read was successful
    }

    GDIMetaFile aWMF;
    pSt->Seek( nPosFc );
    bool bOk = ReadWindowMetafile( *pSt, aWMF, NULL );

    if (!bOk || pSt->GetError() || !aWMF.GetActionSize())
        return false;

    if (m_pWwFib->envr != 1) // !MAC as creator
    {
        rpGraphic = new Graphic( aWMF );
        return true;
    }

    // MAC - word as creator
    // The WMF only says "Please use Word 6.0c" and Mac-Pict follows but without
    // the first 512 Bytes which are not relevant in a MAC-PICT (they are not
    // interpreted)
    bOk = false;
    long nData = rPic.lcb - ( pSt->Tell() - nPosFc );
    if (nData > 0)
    {
        rpGraphic = new Graphic();
        if (!(bOk = SwWW8ImplReader::GetPictGrafFromStream(*rpGraphic, *pSt)))
            DELETEZ(rpGraphic);
    }
    return bOk; // Contains graphic
}

struct WW8PicDesc
{
    sal_Int16 nCL, nCR, nCT, nCB;
    long nWidth, nHeight;

    explicit WW8PicDesc( const WW8_PIC& rPic );
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
    // Insert SdrGrafObj instead of SdrTextObj into this group
    if (SdrObject* pGroupObject = rReplaceObj.GetUpGroup())
    {
        SdrObjList* pObjectList = pGroupObject->GetSubList();

        rSubObj.SetLogicRect(rReplaceObj.GetCurrentBoundRect());
        rSubObj.SetLayer(rReplaceObj.GetLayer());

        // remove old object from group-list and add new one
        // (this also exchanges it in the drwaing page)
        pObjectList->ReplaceObject(&rSubObj, rReplaceObj.GetOrdNum());
    }
    else
    {
        OSL_ENSURE( false, "Impossible!");
    }
}

// MakeGrafNotInContent inserts a non character bound graphic
// ( bGrafApo == true)
SwFlyFrameFormat* SwWW8ImplReader::MakeGrafNotInContent(const WW8PicDesc& rPD,
    const Graphic* pGraph, const OUString& rFileName, const SfxItemSet& rGrfSet)
{

    sal_uInt32 nWidth = rPD.nWidth;
    sal_uInt32 nHeight = rPD.nHeight;

    // Vertical shift through line spacing
    sal_Int32 nNetHeight = nHeight + rPD.nCT + rPD.nCB;
    if( m_pSFlyPara->nLineSpace && m_pSFlyPara->nLineSpace > nNetHeight )
        m_pSFlyPara->nYPos =
            (sal_uInt16)( m_pSFlyPara->nYPos + m_pSFlyPara->nLineSpace - nNetHeight );

    WW8FlySet aFlySet(*this, m_pWFlyPara, m_pSFlyPara, true);

    SwFormatAnchor aAnchor(m_pSFlyPara->eAnchor);
    aAnchor.SetAnchor(m_pPaM->GetPoint());
    aFlySet.Put(aAnchor);

    aFlySet.Put( SwFormatFrmSize( ATT_FIX_SIZE, nWidth, nHeight ) );

    SwFlyFrameFormat* pFlyFormat = m_rDoc.getIDocumentContentOperations().Insert(*m_pPaM, rFileName, OUString(), pGraph,
        &aFlySet, &rGrfSet, NULL);

    // So the frames are generated when inserted in an existing doc:
    if (m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() &&
        (FLY_AT_PARA == pFlyFormat->GetAnchor().GetAnchorId()))
    {
        pFlyFormat->MakeFrms();
    }
    return pFlyFormat;
}

// MakeGrafInContent inserts a character bound graphic
SwFrameFormat* SwWW8ImplReader::MakeGrafInContent(const WW8_PIC& rPic,
    const WW8PicDesc& rPD, const Graphic* pGraph, const OUString& rFileName,
    const SfxItemSet& rGrfSet)
{
    WW8FlySet aFlySet(*this, m_pPaM, rPic, rPD.nWidth, rPD.nHeight);

    SwFrameFormat* pFlyFormat = 0;

    if (rFileName.isEmpty() && m_nObjLocFc)      // dann sollte ists ein OLE-Object
        pFlyFormat = ImportOle(pGraph, &aFlySet, &rGrfSet);

    if( !pFlyFormat )                          // dann eben als Graphic
    {

        pFlyFormat = m_rDoc.getIDocumentContentOperations().Insert( *m_pPaM, rFileName, OUString(), pGraph, &aFlySet,
            &rGrfSet, NULL);
    }

    // Resize the frame to the size of the picture if graphic is inside a frame
    // (only if auto-width)
    if( m_pSFlyPara )
        m_pSFlyPara->BoxUpWidth( rPD.nWidth );
    return pFlyFormat;
}

SwFrameFormat* SwWW8ImplReader::ImportGraf1(WW8_PIC& rPic, SvStream* pSt,
    sal_uLong nFilePos )
{
    SwFrameFormat* pRet = 0;
    if( pSt->IsEof() || rPic.fError || rPic.MFP.mm == 99 )
        return 0;

    OUString aFileName;
    bool bInDoc;
    Graphic* pGraph = 0;
    bool bOk = ReadGrafFile(aFileName, pGraph, rPic, pSt, nFilePos, &bInDoc);

    if (!bOk)
    {
        delete pGraph;
        return 0;                       // Graphic could not be readed correctly
    }

    WW8PicDesc aPD( rPic );

    SwAttrSet aGrfSet( m_rDoc.GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);
    if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
    {
        SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB) ;
        aGrfSet.Put( aCrop );
    }

    if( m_pWFlyPara && m_pWFlyPara->bGrafApo )
        pRet = MakeGrafNotInContent(aPD,pGraph,aFileName,aGrfSet);
    else
        pRet = MakeGrafInContent(rPic,aPD,pGraph,aFileName,aGrfSet);
    delete pGraph;
    return pRet;
}

void SwWW8ImplReader::PicRead(SvStream *pDataStream, WW8_PIC *pPic,
    bool bVer67)
{
    //Only the first 0x2e bytes are the same between version 6/7 and 8+
    WW8_PIC_SHADOW aPicS;
    pDataStream->Read( &aPicS, sizeof( aPicS ) );
    WW8PicShadowToReal( &aPicS, pPic );
    for (int i=0;i<4;i++)
        pDataStream->Read( &pPic->rgbrc[i], bVer67 ? 2 : 4);
    pDataStream->ReadInt16( pPic->dxaOrigin );
    pDataStream->ReadInt16( pPic->dyaOrigin );
    if (!bVer67)
        pDataStream->SeekRel(2);  //cProps
}

SwFrameFormat* SwWW8ImplReader::ImportGraf(SdrTextObj* pTextObj,
    SwFrameFormat* pOldFlyFormat)
{
    SwFrameFormat* pRet = 0;
    if (
        ((m_pStrm == m_pDataStream ) && !m_nPicLocFc) ||
        (m_nIniFlags & WW8FL_NO_GRAF)
       )
    {
        return 0;
    }

    ::SetProgressState(m_nProgress, m_pDocShell);         // Update

    GrafikCtor();

    /*
     * Little joke from Microsoft: sometimes a stream named DATA exists. This
     * stream then contains the PICF and the corresponding graphic!
     * We otherwise map the variable pDataStream to pStream.
     */
    sal_uLong nOldPos = m_pDataStream->Tell();
    WW8_PIC aPic;
    m_pDataStream->Seek( m_nPicLocFc );
    PicRead( m_pDataStream, &aPic, m_bVer67);

    // Sanity check is needed because for example check boxes in field results
    // contain a WMF-like struct
    if ((aPic.lcb >= 58) && !m_pDataStream->GetError())
    {
        if( m_pFlyFormatOfJustInsertedGraphic )
        {
            // We just added a graphic-link into the doc. Now we need to set
            // its position and scale it.
            WW8PicDesc aPD( aPic );

            WW8FlySet aFlySet( *this, m_pPaM, aPic, aPD.nWidth, aPD.nHeight );

            // the correct anchor is set in Read_F_IncludePicture and the
            // current PaM point is after the position if it is anchored in
            // content; because this anchor add a character into the textnode.
            // #i2806#
            if (FLY_AS_CHAR ==
                m_pFlyFormatOfJustInsertedGraphic->GetAnchor().GetAnchorId() )
            {
                aFlySet.ClearItem( RES_ANCHOR );
            }

            m_pFlyFormatOfJustInsertedGraphic->SetFormatAttr( aFlySet );

            m_pFlyFormatOfJustInsertedGraphic = 0;
        }
        else if((0x64 == aPic.MFP.mm) || (0x66 == aPic.MFP.mm))
        {
            // verlinkte Grafik im Escher-Objekt
            SdrObject* pObject = 0;

            WW8PicDesc aPD( aPic );
            if (!m_pMSDffManager)
                m_pMSDffManager = new SwMSDffManager(*this, m_bSkipImages);
            /* ##835##
             * Disable use of main stream as fallback stream for inline direct
             * blips as it is known that they are directly after the record
             * header, testing for existence in main stream may lead to an
             * incorrect fallback graphic being found if other escher graphics
             * have been inserted in the document
             */
            m_pMSDffManager->DisableFallbackStream();
            if( !m_pMSDffManager->GetModel() )
                m_pMSDffManager->SetModel(m_pDrawModel, 1440);

            if (0x66 == aPic.MFP.mm)
            {
                //These ones have names prepended
                sal_uInt8 nNameLen=0;
                m_pDataStream->ReadUChar( nNameLen );
                m_pDataStream->SeekRel( nNameLen );
            }

            Rectangle aChildRect;
            Rectangle aClientRect( 0,0, aPD.nWidth,  aPD.nHeight);
            SvxMSDffImportData aData( aClientRect );
            pObject = m_pMSDffManager->ImportObj(*m_pDataStream, &aData, aClientRect, aChildRect );
            if (pObject)
            {
                // for the frame
                SfxItemSet aAttrSet( m_rDoc.GetAttrPool(), RES_FRMATR_BEGIN,
                    RES_FRMATR_END-1 );

                SvxMSDffImportRec const*const pRecord = (1 == aData.size())
                    ? aData.begin()->get() : nullptr;

                if( pRecord )
                {

                    // Horizontal rule may have its width given as % of page
                    // width (-1 is used if not given, 0 means the object has
                    // fixed width).
                    // Additionally, if it's a horizontal rule without width
                    // given, assume 100.0% width.
                    int relativeWidth = pRecord->relativeHorizontalWidth;
                    if( relativeWidth == -1 )
                        relativeWidth = pRecord->isHorizontalRule ? 1000 : 0;
                    if( relativeWidth != 0 )
                    {
                        aPic.mx = msword_cast<sal_uInt16>(
                            m_aSectionManager.GetPageWidth() -
                            m_aSectionManager.GetPageRight() -
                            m_aSectionManager.GetPageLeft()) * relativeWidth / 1000;
                        aPD = WW8PicDesc( aPic );
                        // This SetSnapRect() call adjusts the size of the
                        // object itself, no idea why it's this call (or even
                        // what the call actually does), but that's what
                        // ImportGraf() (called by ImportObj()) uses.
                        pObject->SetSnapRect( Rectangle( 0, 0, aPD.nWidth, aPD.nHeight ));
                    }

                    // A graphic of this type in this location is always
                    // inline, and uses the pic in the same module as ww6
                    // graphics.
                    if (m_pWFlyPara && m_pWFlyPara->bGrafApo)
                    {
                        WW8FlySet aFlySet(*this, m_pWFlyPara, m_pSFlyPara, true);

                        SwFormatAnchor aAnchor(m_pSFlyPara->eAnchor);
                        aAnchor.SetAnchor(m_pPaM->GetPoint());
                        aFlySet.Put(aAnchor);

                        aAttrSet.Put(aFlySet);
                    }
                    else
                    {
                        WW8FlySet aFlySet( *this, m_pPaM, aPic, aPD.nWidth,
                            aPD.nHeight );

                        aAttrSet.Put(aFlySet);
                    }
                    // Modified for i120716,for graf importing from MS Word 2003
                    // binary format, there is no border distance.
                    Rectangle aInnerDist(0,0,0,0);
                    MatchSdrItemsIntoFlySet( pObject, aAttrSet,
                        pRecord->eLineStyle, pRecord->eLineDashing,
                        pRecord->eShapeType, aInnerDist );

                    // Set the size from the WinWord PIC-structure as graphic
                    // size
                    aAttrSet.Put( SwFormatFrmSize( ATT_FIX_SIZE, aPD.nWidth,
                        aPD.nHeight ) );
                }

                // for the graphic
                SfxItemSet aGrSet( m_rDoc.GetAttrPool(), RES_GRFATR_BEGIN,
                    RES_GRFATR_END-1 );

                if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
                {
                    SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB );
                    aGrSet.Put( aCrop );
                }

                if (pRecord)
                    MatchEscherMirrorIntoFlySet(*pRecord, aGrSet);

                // if necessary adopt old AttrSet and correct horizontal
                // positioning relation
                if( pOldFlyFormat )
                {
                    aAttrSet.Put( pOldFlyFormat->GetAttrSet() );
                    const SwFormatHoriOrient &rHori = pOldFlyFormat->GetHoriOrient();
                    if( text::RelOrientation::FRAME == rHori.GetRelationOrient() )
                    {
                        aAttrSet.Put( SwFormatHoriOrient( rHori.GetPos(),
                            text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ) );
                    }
                }

                bool bTextObjWasGrouped = false;
                if (pOldFlyFormat && pTextObj && pTextObj->GetUpGroup())
                    bTextObjWasGrouped = true;

                if (bTextObjWasGrouped)
                    ReplaceObj(*pTextObj, *pObject);
                else
                {
                    if (sal_uInt16(OBJ_OLE2) == pObject->GetObjIdentifier())
                    {
                        // the size from BLIP, if there is any, should be already set
                        pRet = InsertOle(*static_cast<SdrOle2Obj*>(pObject), aAttrSet, &aGrSet);
                    }
                    else
                    {
                        if (SdrGrafObj* pGraphObject = dynamic_cast<SdrGrafObj*>( pObject) )
                        {
                            // Now add the link or rather the graphic to the doc
                            const Graphic& rGraph = pGraphObject->GetGraphic();

                            if (m_nObjLocFc)  // is it a OLE-Object?
                                pRet = ImportOle(&rGraph, &aAttrSet, &aGrSet, pObject->GetBLIPSizeRectangle());

                            if (!pRet)
                            {
                                pRet = m_rDoc.getIDocumentContentOperations().Insert(*m_pPaM, OUString(), OUString(),
                                    &rGraph, &aAttrSet, &aGrSet, NULL );
                            }
                        }
                        else
                            pRet = m_rDoc.getIDocumentContentOperations().InsertDrawObj(*m_pPaM, *pObject, aAttrSet );
                    }
                }

                // only if we made an *Insert*
                if (pRet)
                {
                    if (pRecord)
                        SetAttributesAtGrfNode(pRecord, pRet, 0);

                    // #i68101#
                    // removed pObject->HasSetName() usage since always returned
                    // true, also removed else-part and wrote an informing mail
                    // to Henning Brinkmann about this to clarify.
                    pRet->SetName(pObject->GetName());


                    // determine the pointer to the new object and update
                    // Z-order-list accordingly (or delete entry)
                    if (SdrObject* pOurNewObject = CreateContactObject(pRet))
                    {
                        if (pOurNewObject != pObject)
                        {
                            m_pMSDffManager->ExchangeInShapeOrder( pObject, 0, 0,
                                pOurNewObject );

                            // delete and destroy old SdrGrafObj from page
                            if (pObject->GetPage())
                                m_pDrawPg->RemoveObject(pObject->GetOrdNum());
                            SdrObject::Free( pObject );
                        }
                    }
                    else
                        m_pMSDffManager->RemoveFromShapeOrder( pObject );
                }
                else
                    m_pMSDffManager->RemoveFromShapeOrder( pObject );

                // also delete this from the page if not grouped
                if (pTextObj && !bTextObjWasGrouped && pTextObj->GetPage())
                    m_pDrawPg->RemoveObject( pTextObj->GetOrdNum() );
            }
            m_pMSDffManager->EnableFallbackStream();
        }
        else if (aPic.lcb >= 58)
            pRet = ImportGraf1(aPic, m_pDataStream, m_nPicLocFc);
    }
    m_pDataStream->Seek( nOldPos );

    if (pRet)
    {
        SdrObject* pOurNewObject = CreateContactObject(pRet);
        m_pWWZOrder->InsertTextLayerObject(pOurNewObject);
    }

    return AddAutoAnchor(pRet);
}

void WW8PicShadowToReal( WW8_PIC_SHADOW * pPicS, WW8_PIC * pPic )
{
    pPic->lcb = SVBT32ToUInt32( pPicS->lcb );
    pPic->cbHeader = SVBT16ToShort( pPicS->cbHeader );
    pPic->MFP.mm = SVBT16ToShort( pPicS->MFP.mm );
    pPic->MFP.xExt = SVBT16ToShort( pPicS->MFP.xExt );
    pPic->MFP.yExt = SVBT16ToShort( pPicS->MFP.yExt );
    pPic->MFP.hMF = SVBT16ToShort( pPicS->MFP.hMF );
    for( sal_uInt16 i = 0; i < 14 ; i++ )
        pPic->rcWinMF[i] = pPicS->rcWinMF[i];
    pPic->dxaGoal = SVBT16ToShort( pPicS->dxaGoal );
    pPic->dyaGoal = SVBT16ToShort( pPicS->dyaGoal );
    pPic->mx = SVBT16ToShort( pPicS->mx );
    pPic->my = SVBT16ToShort( pPicS->my );
    pPic->dxaCropLeft = SVBT16ToShort( pPicS->dxaCropLeft );
    pPic->dyaCropTop = SVBT16ToShort( pPicS->dyaCropTop );
    pPic->dxaCropRight = SVBT16ToShort( pPicS->dxaCropRight );
    pPic->dyaCropBottom = SVBT16ToShort( pPicS->dyaCropBottom );
    pPic->brcl = pPicS->aBits1 & 0x0f;
    pPic->fFrameEmpty = (pPicS->aBits1 & 0x10) >> 4;
    pPic->fBitmap = (pPicS->aBits1 & 0x20) >> 5;
    pPic->fDrawHatch = (pPicS->aBits1 & 0x40) >> 6;
    pPic->fError = (pPicS->aBits1 & 0x80) >> 7;
    pPic->bpp = pPicS->aBits2;
}

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW * pFSPAS, WW8_FSPA * pFSPA )
{
    pFSPA->nSpId        = SVBT32ToUInt32( pFSPAS->nSpId );
    pFSPA->nXaLeft      = SVBT32ToUInt32( pFSPAS->nXaLeft );
    pFSPA->nYaTop       = SVBT32ToUInt32( pFSPAS->nYaTop );
    pFSPA->nXaRight     = SVBT32ToUInt32( pFSPAS->nXaRight );
    pFSPA->nYaBottom    = SVBT32ToUInt32( pFSPAS->nYaBottom );

    sal_uInt16 nBits        = SVBT16ToShort( pFSPAS->aBits1 );

    pFSPA->bHdr         = sal_uInt16(0 !=  ( nBits & 0x0001 ));
    pFSPA->nbx          =       ( nBits & 0x0006 ) >> 1;
    pFSPA->nby          =       ( nBits & 0x0018 ) >> 3;
    pFSPA->nwr          =       ( nBits & 0x01E0 ) >> 5;
    pFSPA->nwrk         =       ( nBits & 0x1E00 ) >> 9;
    pFSPA->bRcaSimple   = sal_uInt16(0 !=  ( nBits & 0x2000 ));
    pFSPA->bBelowText   = sal_uInt16(0 !=  ( nBits & 0x4000 ));
    pFSPA->bAnchorLock  = sal_uInt16(0 !=  ( nBits & 0x8000 ));
    pFSPA->nTxbx = SVBT32ToUInt32( pFSPAS->nTxbx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
