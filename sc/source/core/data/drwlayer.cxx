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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <scitems.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <sot/exchange.hxx>
#include <svx/objfac3d.hxx>
#include <svx/xtable.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/svxids.hrc>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/unolingu.hxx>
#include <svx/drawitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itempool.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/globname.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <drwlayer.hxx>
#include <drawpage.hxx>
#include <global.hxx>
#include <document.hxx>
#include <userdat.hxx>
#include <markdata.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <postit.hxx>
#include <attrib.hxx>
#include <charthelper.hxx>
#include <table.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vcl/fieldvalues.hxx>
#include <memory>
#include <algorithm>
#include <cstdlib>

namespace com::sun::star::embed { class XEmbeddedObject; }

#define DET_ARROW_OFFSET    1000

using namespace ::com::sun::star;

static E3dObjFactory* pF3d = nullptr;
static sal_uInt16 nInst = 0;

SfxObjectShell* ScDrawLayer::pGlobalDrawPersist = nullptr;

bool bDrawIsInUndo = false;         //TODO: Member

ScUndoObjData::ScUndoObjData( SdrObject* pObjP, const ScAddress& rOS, const ScAddress& rOE,
                                               const ScAddress& rNS, const ScAddress& rNE ) :
    SdrUndoObj( *pObjP ),
    aOldStt( rOS ),
    aOldEnd( rOE ),
    aNewStt( rNS ),
    aNewEnd( rNE )
{
}

ScUndoObjData::~ScUndoObjData()
{
}

void ScUndoObjData::Undo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    OSL_ENSURE(pData,"ScUndoObjData: Data missing");
    if (pData)
    {
        pData->maStart = aOldStt;
        pData->maEnd = aOldEnd;
    }

    // Undo also an untransformed anchor
    pData = ScDrawLayer::GetNonRotatedObjData( pObj );
    if (pData)
    {
        pData->maStart = aOldStt;
        pData->maEnd = aOldEnd;
    }
}

void ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    OSL_ENSURE(pData,"ScUndoObjData: Data missing");
    if (pData)
    {
        pData->maStart = aNewStt;
        pData->maEnd = aNewEnd;
    }

    // Redo also an untransformed anchor
    pData = ScDrawLayer::GetNonRotatedObjData( pObj );
    if (pData)
    {
        pData->maStart = aNewStt;
        pData->maEnd = aNewEnd;
    }
}

ScUndoAnchorData::ScUndoAnchorData( SdrObject* pObjP, ScDocument* pDoc, SCTAB nTab ) :
    SdrUndoObj( *pObjP ),
    mpDoc( pDoc ),
    mnTab( nTab )
{
    mbWasCellAnchored = ScDrawLayer::IsCellAnchored( *pObjP );
    mbWasResizeWithCell = ScDrawLayer::IsResizeWithCell( *pObjP );
}

ScUndoAnchorData::~ScUndoAnchorData()
{
}

void ScUndoAnchorData::Undo()
{
    // Trigger Object Change
    if (pObj->IsInserted() && pObj->getSdrPageFromSdrObject())
    {
        SdrHint aHint(SdrHintKind::ObjectChange, *pObj);
        pObj->getSdrModelFromSdrObject().Broadcast(aHint);
    }

    if (mbWasCellAnchored)
        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *mpDoc, mnTab, mbWasResizeWithCell);
    else
        ScDrawLayer::SetPageAnchored( *pObj );
}

void ScUndoAnchorData::Redo()
{
    if (mbWasCellAnchored)
        ScDrawLayer::SetPageAnchored( *pObj );
    else
        ScDrawLayer::SetCellAnchoredFromPosition(*pObj, *mpDoc, mnTab, mbWasResizeWithCell);

    // Trigger Object Change
    if (pObj->IsInserted() && pObj->getSdrPageFromSdrObject())
    {
        SdrHint aHint(SdrHintKind::ObjectChange, *pObj);
        pObj->getSdrModelFromSdrObject().Broadcast(aHint);
    }
}

ScTabDeletedHint::ScTabDeletedHint( SCTAB nTabNo ) :
    nTab( nTabNo )
{
}

ScTabDeletedHint::~ScTabDeletedHint()
{
}

ScTabSizeChangedHint::ScTabSizeChangedHint( SCTAB nTabNo ) :
    nTab( nTabNo )
{
}

ScTabSizeChangedHint::~ScTabSizeChangedHint()
{
}

#define MAXMM   10000000

static tools::Long TwipsToHmm (tools::Long nVal)
{
    return static_cast< tools::Long >( vcl::ConvertDoubleValue (static_cast<sal_Int64>(nVal), 0, 0,
            FieldUnit::TWIP, FieldUnit::MM_100TH) );
}

static tools::Long HmmToTwips (tools::Long nVal)
{
    return static_cast< tools::Long > ( vcl::ConvertDoubleValue (static_cast<sal_Int64>(nVal), 0, 0,
            FieldUnit::MM_100TH, FieldUnit::TWIP) );
}

static void lcl_ReverseTwipsToMM( tools::Rectangle& rRect )
{
    rRect.SetLeft( HmmToTwips( rRect.Left() ) );
    rRect.SetRight( HmmToTwips( rRect.Right() ) );
    rRect.SetTop( HmmToTwips( rRect.Top()) );
    rRect.SetBottom( HmmToTwips(rRect.Bottom()) );
}

static ScRange lcl_getClipRangeFromClipDoc(ScDocument* pClipDoc, SCTAB nClipTab)
{
    if (!pClipDoc)
        return ScRange();

    SCCOL nClipStartX;
    SCROW nClipStartY;
    SCCOL nClipEndX;
    SCROW nClipEndY;
    pClipDoc->GetClipStart(nClipStartX, nClipStartY);
    pClipDoc->GetClipArea(nClipEndX, nClipEndY, true);
    nClipEndX = nClipEndX + nClipStartX;
    nClipEndY += nClipStartY; // GetClipArea returns the difference

    return ScRange(nClipStartX, nClipStartY, nClipTab, nClipEndX, nClipEndY, nClipTab);
}

ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const OUString& rName ) :
    FmFormModel(
        nullptr,
        pGlobalDrawPersist ? pGlobalDrawPersist : (pDocument ? pDocument->GetDocumentShell() : nullptr)),
    aName( rName ),
    pDoc( pDocument ),
    bRecording( false ),
    bAdjustEnabled( true ),
    bHyphenatorSet( false )
{
    pGlobalDrawPersist = nullptr;          // Only use once

    SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : nullptr;
    XColorListRef pXCol = XColorList::GetStdColorList();
    if ( pObjSh )
    {
        SetObjectShell( pObjSh );

        // set color table
        const SvxColorListItem* pColItem = pObjSh->GetItem( SID_COLOR_TABLE );
        if ( pColItem )
            pXCol = pColItem->GetColorList();
    }
    SetPropertyList( static_cast<XPropertyList *> (pXCol.get()) );

    SetSwapGraphics();

    SetScaleUnit(MapUnit::Map100thMM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(MapUnit::Map100thMM);
    SvxFrameDirectionItem aModeItem( SvxFrameDirection::Environment, EE_PARA_WRITINGDIR );
    rPool.SetPoolDefaultItem( aModeItem );

    // #i33700#
    // Set shadow distance defaults as PoolDefaultItems. Details see bug.
    rPool.SetPoolDefaultItem(makeSdrShadowXDistItem(300));
    rPool.SetPoolDefaultItem(makeSdrShadowYDistItem(300));

    // default for script spacing depends on locale, see SdDrawDocument ctor in sd
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    if (MsLangId::isKorean(eOfficeLanguage) || eOfficeLanguage == LANGUAGE_JAPANESE)
    {
        // secondary is edit engine pool
        rPool.GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
    }

    rPool.FreezeIdRanges();                         // the pool is also used directly

    SdrLayerAdmin& rAdmin = GetLayerAdmin();
    rAdmin.NewLayer("vorne",    sal_uInt8(SC_LAYER_FRONT));
    rAdmin.NewLayer("hinten",   sal_uInt8(SC_LAYER_BACK));
    rAdmin.NewLayer("intern",   sal_uInt8(SC_LAYER_INTERN));
    rAdmin.NewLayer("Controls", sal_uInt8(SC_LAYER_CONTROLS));
    rAdmin.SetControlLayerName("Controls");
    rAdmin.NewLayer("hidden",   sal_uInt8(SC_LAYER_HIDDEN));
    // "Controls" is new - must also be created when loading

    // Set link for URL-Fields
    ScModule* pScMod = SC_MOD();
    Outliner& rOutliner = GetDrawOutliner();
    rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    Outliner& rHitOutliner = GetHitTestOutliner();
    rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    SfxItemPool* pOutlinerPool = rOutliner.GetEditTextObjectPool();
    if ( pOutlinerPool )
    {
         m_pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));           // 12Pt
         m_pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CJK ));           // 12Pt
         m_pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CTL ));           // 12Pt
    }
    SfxItemPool* pHitOutlinerPool = rHitOutliner.GetEditTextObjectPool();
    if ( pHitOutlinerPool )
    {
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));    // 12Pt
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CJK ));    // 12Pt
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CTL ));    // 12Pt
    }

    // initial undo mode as in Calc document
    if( pDoc )
        EnableUndo( pDoc->IsUndoEnabled() );

    //  URL-Buttons have no handler anymore, all is done by themselves

    if( !nInst++ )
    {
        pF3d = new E3dObjFactory;
    }
}

ScDrawLayer::~ScDrawLayer()
{
    Broadcast(SdrHint(SdrHintKind::ModelCleared));

    ClearModel(true);

    pUndoGroup.reset();
    if( !--nInst )
    {
        delete pF3d;
        pF3d = nullptr;
    }
}

void ScDrawLayer::UseHyphenator()
{
    if (!bHyphenatorSet)
    {
        css::uno::Reference< css::linguistic2::XHyphenator >
                                    xHyphenator = LinguMgr::GetHyphenator();

        GetDrawOutliner().SetHyphenator( xHyphenator );
        GetHitTestOutliner().SetHyphenator( xHyphenator );

        bHyphenatorSet = true;
    }
}

SdrPage* ScDrawLayer::AllocPage(bool bMasterPage)
{
    return new ScDrawPage(*this, bMasterPage);
}

bool ScDrawLayer::HasObjects() const
{
    bool bFound = false;

    sal_uInt16 nCount = GetPageCount();
    for (sal_uInt16 i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = true;

    return bFound;
}

SdrModel* ScDrawLayer::AllocModel() const
{
    //  Allocated model (for clipboard etc) must not have a pointer
    //  to the original model's document, pass NULL as document:

    return new ScDrawLayer( nullptr, aName );
}

bool ScDrawLayer::ScAddPage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return false;   // not inserted

    ScDrawPage* pPage = static_cast<ScDrawPage*>(AllocPage( false ));
    InsertPage(pPage, static_cast<sal_uInt16>(nTab));
    if (bRecording)
        AddCalcUndo(std::make_unique<SdrUndoNewPage>(*pPage));

    ResetTab(nTab, pDoc->GetTableCount()-1);
    return true;        // inserted
}

void ScDrawLayer::ScRemovePage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return;

    Broadcast( ScTabDeletedHint( nTab ) );
    if (bRecording)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        AddCalcUndo(std::make_unique<SdrUndoDelPage>(*pPage));        // Undo-Action becomes the page owner
        RemovePage( static_cast<sal_uInt16>(nTab) );    // just deliver, not deleting
    }
    else
        DeletePage( static_cast<sal_uInt16>(nTab) );    // just get rid of it

    ResetTab(nTab, pDoc->GetTableCount()-1);
}

void ScDrawLayer::ScRenamePage( SCTAB nTab, const OUString& rNewName )
{
    ScDrawPage* pPage = static_cast<ScDrawPage*>( GetPage(static_cast<sal_uInt16>(nTab)) );
    if (pPage)
        pPage->SetName(rNewName);
}

void ScDrawLayer::ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos )
{
    MovePage( nOldPos, nNewPos );
    sal_uInt16 nMinPos = std::min(nOldPos, nNewPos);
    ResetTab(nMinPos, pDoc->GetTableCount()-1);
}

void ScDrawLayer::ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos )
{
    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = GetPage(nNewPos);

    // Copying

    if (pOldPage && pNewPage)
    {
        SCTAB nOldTab = static_cast<SCTAB>(nOldPos);
        SCTAB nNewTab = static_cast<SCTAB>(nNewPos);

        SdrObjListIter aIter( pOldPage, SdrIterMode::Flat );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            ScDrawObjData* pOldData = GetObjData(pOldObject);
            if (pOldData)
            {
                pOldData->maStart.SetTab(nOldTab);
                pOldData->maEnd.SetTab(nOldTab);
            }

            // Clone to target SdrModel
            SdrObject* pNewObject(pOldObject->CloneSdrObject(*this));
            pNewObject->NbcMove(Size(0,0));
            pNewPage->InsertObject( pNewObject );
            ScDrawObjData* pNewData = GetObjData(pNewObject);
            if (pNewData)
            {
                pNewData->maStart.SetTab(nNewTab);
                pNewData->maEnd.SetTab(nNewTab);
            }

            if (bRecording)
                AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pNewObject ) );

            pOldObject = aIter.Next();
        }
    }

    ResetTab(static_cast<SCTAB>(nNewPos), pDoc->GetTableCount()-1);
}

void ScDrawLayer::ResetTab( SCTAB nStart, SCTAB nEnd )
{
    SCTAB nPageSize = static_cast<SCTAB>(GetPageCount());
    if (nPageSize < 0)
        // No drawing pages exist.
        return;

    if (nEnd >= nPageSize)
        // Avoid iterating beyond the last existing page.
        nEnd = nPageSize - 1;

    for (SCTAB i = nStart; i <= nEnd; ++i)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(i));
        if (!pPage)
            continue;

        SdrObjListIter aIter(pPage, SdrIterMode::Flat);
        for (SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next())
        {
            ScDrawObjData* pData = GetObjData(pObj);
            if (!pData)
                continue;

            pData->maStart.SetTab(i);
            pData->maEnd.SetTab(i);
        }
    }
}

static bool IsInBlock( const ScAddress& rPos, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2 )
{
    return rPos.Col() >= nCol1 && rPos.Col() <= nCol2 &&
           rPos.Row() >= nRow1 && rPos.Row() <= nRow2;
}

void ScDrawLayer::MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCCOL nDx,SCROW nDy, bool bUpdateNoteCaptionPos )
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return;

    bool bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    const size_t nCount = pPage->GetObjCount();
    for ( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = GetObjDataTab( pObj, nTab );
        if( pData )
        {
            const ScAddress aOldStt = pData->maStart;
            const ScAddress aOldEnd = pData->maEnd;
            bool bChange = false;
            if ( aOldStt.IsValid() && IsInBlock( aOldStt, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->maStart.IncCol( nDx );
                pData->maStart.IncRow( nDy );
                bChange = true;
            }
            if ( aOldEnd.IsValid() && IsInBlock( aOldEnd, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->maEnd.IncCol( nDx );
                pData->maEnd.IncRow( nDy );
                bChange = true;
            }
            if (bChange)
            {
                if ( dynamic_cast<const SdrRectObj*>( pObj) !=  nullptr && pData->maStart.IsValid() && pData->maEnd.IsValid() )
                    pData->maStart.PutInOrder( pData->maEnd );

                // Update also an untransformed anchor that's what we stored ( and still do ) to xml
                ScDrawObjData* pNoRotatedAnchor = GetNonRotatedObjData( pObj );
                if ( pNoRotatedAnchor )
                {
                    pNoRotatedAnchor->maStart.IncCol(nDx);
                    pNoRotatedAnchor->maStart.IncRow(nDy);
                    pNoRotatedAnchor->maEnd.IncCol(nDx);
                    pNoRotatedAnchor->maEnd.IncRow(nDy);
                }

                AddCalcUndo( std::make_unique<ScUndoObjData>( pObj, aOldStt, aOldEnd, pData->maStart, pData->maEnd ) );
                RecalcPos( pObj, *pData, bNegativePage, bUpdateNoteCaptionPos );
            }
        }
    }
}

void ScDrawLayer::SetPageSize(sal_uInt16 nPageNo, const Size& rSize, bool bUpdateNoteCaptionPos,
                              const ScObjectHandling eObjectHandling)
{
    SdrPage* pPage = GetPage(nPageNo);
    if (!pPage)
        return;

    if ( rSize != pPage->GetSize() )
    {
        pPage->SetSize( rSize );
        Broadcast( ScTabSizeChangedHint( static_cast<SCTAB>(nPageNo) ) );   // SetWorkArea() on the views
    }

    // Do not call RecalcPos while loading, because row height is not finished, when SetPageSize
    // is called first time. Instead the objects are initialized from ScXMLImport::endDocument() and
    // RecalcPos is called from there.
    if (!pDoc || pDoc->IsImportingXML())
        return;

    // Implement Detective lines (adjust to new heights / widths)
    //  even if size is still the same
    //  (individual rows/columns can have been changed))

    bool bNegativePage = pDoc && pDoc->IsNegativePage( static_cast<SCTAB>(nPageNo) );

    // Disable mass broadcasts from drawing objects' position changes.
    bool bWasLocked = isLocked();
    setLock(true);

    const size_t nCount = pPage->GetObjCount();
    for ( size_t i = 0; i < nCount; ++i )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = GetObjDataTab( pObj, static_cast<SCTAB>(nPageNo) );
        if( pData ) // cell anchored
        {
            if (pData->meType == ScDrawObjData::DrawingObject
                || pData->meType == ScDrawObjData::ValidationCircle)
            {
                switch (eObjectHandling)
                {
                    case ScObjectHandling::RecalcPosMode:
                        RecalcPos(pObj, *pData, bNegativePage, bUpdateNoteCaptionPos);
                        break;
                    case ScObjectHandling::MoveRTLMode:
                        MoveRTL(pObj);
                        break;
                    case ScObjectHandling::MirrorRTLMode:
                        MirrorRTL(pObj);
                        break;
                }
            }
            else // DetectiveArrow and CellNote
                RecalcPos(pObj, *pData, bNegativePage, bUpdateNoteCaptionPos);
        }
        else // page anchored
        {
            switch (eObjectHandling)
            {
                case ScObjectHandling::MoveRTLMode:
                    MoveRTL(pObj);
                    break;
                case ScObjectHandling::MirrorRTLMode:
                    MirrorRTL(pObj);
                    break;
                case ScObjectHandling::RecalcPosMode: // does not occur for page anchored shapes
                    break;
            }
        }
    }

    setLock(bWasLocked);
}

namespace
{
    //Can't have a zero width dimension
    tools::Rectangle lcl_makeSafeRectangle(const tools::Rectangle &rNew)
    {
        tools::Rectangle aRect = rNew;
        if (aRect.Bottom() == aRect.Top())
            aRect.SetBottom( aRect.Top()+1 );
        if (aRect.Right() == aRect.Left())
            aRect.SetRight( aRect.Left()+1 );
        return aRect;
    }

    Point lcl_calcAvailableDiff(const ScDocument &rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const Point &aWantedDiff)
    {
        Point aAvailableDiff(aWantedDiff);
        tools::Long nHeight = static_cast<tools::Long>(rDoc.GetRowHeight( nRow, nTab ) * HMM_PER_TWIPS);
        tools::Long nWidth  = static_cast<tools::Long>(rDoc.GetColWidth(  nCol, nTab ) * HMM_PER_TWIPS);
        if (aAvailableDiff.Y() > nHeight)
            aAvailableDiff.setY( nHeight );
        if (aAvailableDiff.X() > nWidth)
            aAvailableDiff.setX( nWidth );
        return aAvailableDiff;
    }

    tools::Rectangle lcl_UpdateCalcPoly(basegfx::B2DPolygon &rCalcPoly, int nWhichPoint, const Point &rPos)
    {
        rCalcPoly.setB2DPoint(nWhichPoint, basegfx::B2DPoint(rPos.X(), rPos.Y()));
        basegfx::B2DRange aRange(basegfx::utils::getRange(rCalcPoly));
        return tools::Rectangle(static_cast<tools::Long>(aRange.getMinX()), static_cast<tools::Long>(aRange.getMinY()),
            static_cast<tools::Long>(aRange.getMaxX()), static_cast<tools::Long>(aRange.getMaxY()));
    }

bool lcl_AreRectanglesApproxEqual(const tools::Rectangle& rRectA, const tools::Rectangle& rRectB)
{
    // Twips <-> Hmm conversions introduce +-1 differences although there are no real changes in the object.
    // Therefore test with == is not appropriate in some cases.
    if (std::abs(rRectA.Left() - rRectB.Left()) > 1)
        return false;
    if (std::abs(rRectA.Top() - rRectB.Top()) > 1)
        return false;
    if (std::abs(rRectA.Right() - rRectB.Right()) > 1)
        return false;
    if (std::abs(rRectA.Bottom() - rRectB.Bottom()) > 1)
        return false;
    return true;
}

bool lcl_NeedsMirrorYCorrection(SdrObject* pObj)
{
    return pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE
           && static_cast<SdrObjCustomShape*>(pObj)->IsMirroredY();
}

void lcl_SetLogicRectFromAnchor(SdrObject* pObj, ScDrawObjData& rAnchor, ScDocument* pDoc)
{
    // This is only used during initialization. At that time, shape handling is always LTR. No need
    // to consider negative page.
    if (!pObj || !pDoc || !rAnchor.maEnd.IsValid() || !rAnchor.maStart.IsValid())
        return;

    // In case of a vertical mirrored custom shape, LibreOffice uses internally an additional 180deg
    // in aGeo.nRotationAngle and in turn has a different logic rectangle position. We remove flip,
    // set the logic rectangle, and apply flip again. You cannot simple use a 180deg-rotated
    // rectangle, because custom shape mirroring is internally applied after the other
    // transformations.
    const bool bNeedsMirrorYCorrection = lcl_NeedsMirrorYCorrection(pObj); // remember state
    if (bNeedsMirrorYCorrection)
    {
        const tools::Rectangle aRect(pObj->GetSnapRect());
        const Point aLeft(aRect.Left(), (aRect.Top() + aRect.Bottom()) >> 1);
        const Point aRight(aLeft.X() + 1000, aLeft.Y());
        pObj->NbcMirror(aLeft, aRight);
    }

    // Build full sized logic rectangle from start and end given in anchor.
    const tools::Rectangle aStartCellRect(
        pDoc->GetMMRect(rAnchor.maStart.Col(), rAnchor.maStart.Row(), rAnchor.maStart.Col(),
                        rAnchor.maStart.Row(), rAnchor.maStart.Tab(), false /*bHiddenAsZero*/));
    Point aStartPoint(aStartCellRect.Left(), aStartCellRect.Top());
    aStartPoint.AdjustX(rAnchor.maStartOffset.getX());
    aStartPoint.AdjustY(rAnchor.maStartOffset.getY());

    const tools::Rectangle aEndCellRect(
        pDoc->GetMMRect(rAnchor.maEnd.Col(), rAnchor.maEnd.Row(), rAnchor.maEnd.Col(),
                        rAnchor.maEnd.Row(), rAnchor.maEnd.Tab(), false /*bHiddenAsZero*/));
    Point aEndPoint(aEndCellRect.Left(), aEndCellRect.Top());
    aEndPoint.AdjustX(rAnchor.maEndOffset.getX());
    aEndPoint.AdjustY(rAnchor.maEndOffset.getY());

    // Set this as new, full sized logical rectangle
    tools::Rectangle aNewRectangle(aStartPoint, aEndPoint);
    aNewRectangle.Justify();
    if (!lcl_AreRectanglesApproxEqual(pObj->GetLogicRect(), aNewRectangle))
        pObj->NbcSetLogicRect(lcl_makeSafeRectangle(aNewRectangle));

    // The shape has the correct logical rectangle now. Reapply the above removed mirroring.
    if (bNeedsMirrorYCorrection)
    {
        const tools::Rectangle aRect(pObj->GetSnapRect());
        const Point aLeft(aRect.Left(), (aRect.Top() + aRect.Bottom()) >> 1);
        const Point aRight(aLeft.X() + 1000, aLeft.Y());
        pObj->NbcMirror(aLeft, aRight);
    }
}

} // namespace

void ScDrawLayer::ResizeLastRectFromAnchor(const SdrObject* pObj, ScDrawObjData& rData,
                                           bool bUseLogicRect, bool bNegativePage, bool bCanResize)
{
    tools::Rectangle aRect = bUseLogicRect ? pObj->GetLogicRect() : pObj->GetSnapRect();
    SCCOL nCol1 = rData.maStart.Col();
    SCROW nRow1 = rData.maStart.Row();
    SCTAB nTab1 = rData.maStart.Tab();
    SCCOL nCol2 = rData.maEnd.Col();
    SCROW nRow2 = rData.maEnd.Row();
    SCTAB nTab2 = rData.maEnd.Tab();
    Point aPos(pDoc->GetColOffset(nCol1, nTab1, /*bHiddenAsZero*/true),
               pDoc->GetRowOffset(nRow1, nTab1, /*bHiddenAsZero*/true));
    aPos.setX(TwipsToHmm(aPos.X()));
    aPos.setY(TwipsToHmm(aPos.Y()));
    aPos += lcl_calcAvailableDiff(*pDoc, nCol1, nRow1, nTab1, rData.maStartOffset);

    // this sets the needed changed position (translation)
    aRect.SetPos(aPos);

    if (bCanResize)
    {
        // all this stuff is additional stuff to evtl. not only translate the
        // range (Rectangle), but also check for and evtl. do corrections for it's size
        const tools::Rectangle aLastCellRect(rData.getLastCellRect());

        // If the row was hidden before, or we don't have a valid cell rect, calculate the
        // new rect based on the end point.
        // Also when the end point is set, we need to consider it.
        if (rData.mbWasInHiddenRow || aLastCellRect.IsEmpty() || nRow1 != nRow2 || nCol1 != nCol2)
        {
            Point aEnd(pDoc->GetColOffset(nCol2, nTab2, /*bHiddenAsZero*/true),
                       pDoc->GetRowOffset(nRow2, nTab2, /*bHiddenAsZero*/true));
            aEnd.setX(TwipsToHmm(aEnd.X()));
            aEnd.setY(TwipsToHmm(aEnd.Y()));
            aEnd += lcl_calcAvailableDiff(*pDoc, nCol2, nRow2, nTab2, rData.maEndOffset);

            aRect = tools::Rectangle(aPos, aEnd);
        }
        else if (!aLastCellRect.IsEmpty())
        {
            // We calculate based on the last cell rect to be able to scale the image
            // as much as the cell was scaled.
            // Still, we keep the image in its current cell (to keep start anchor == end anchor)
            const tools::Rectangle aCurrentCellRect(GetCellRect(*GetDocument(), rData.maStart, true));
            tools::Long nCurrentWidth(aCurrentCellRect.GetWidth());
            tools::Long nCurrentHeight(aCurrentCellRect.GetHeight());
            const tools::Long nLastWidth(aLastCellRect.GetWidth());
            const tools::Long nLastHeight(aLastCellRect.GetHeight());

            // tdf#116931 Avoid and correct nifty numerical problems with the integer
            // based and converted values (GetCellRect uses multiplies with HMM_PER_TWIPS)
            if(nCurrentWidth + 1 == nLastWidth || nCurrentWidth == nLastWidth + 1)
            {
                nCurrentWidth = nLastWidth;
            }

            if(nCurrentHeight + 1 == nLastHeight || nCurrentHeight == nLastHeight + 1)
            {
                nCurrentHeight = nLastHeight;
            }

            // get initial ScalingFactors
            double fWidthFactor(nCurrentWidth == nLastWidth || 0 == nLastWidth
                ? 1.0
                : static_cast<double>(nCurrentWidth) / static_cast<double>(nLastWidth));
            double fHeightFactor(nCurrentHeight == nLastHeight || 0 == nLastHeight
                ? 1.0
                : static_cast<double>(nCurrentHeight) / static_cast<double>(nLastHeight));

            // check if we grow or shrink - and at all
            const bool bIsGrowing(nCurrentWidth > nLastWidth || nCurrentHeight > nLastHeight);
            const bool bIsShrinking(nCurrentWidth < nLastWidth || nCurrentHeight < nLastHeight);
            const bool bIsSizeChanged(bIsGrowing || bIsShrinking);

            // handle AspectRatio, only needed if size does change
            if(bIsSizeChanged && pObj->shouldKeepAspectRatio())
            {
                tools::Rectangle aRectIncludingOffset = aRect;
                aRectIncludingOffset.setWidth(aRect.GetWidth() + rData.maStartOffset.X());
                aRectIncludingOffset.setHeight(aRect.GetHeight() + rData.maStartOffset.Y());
                tools::Long nWidth = aRectIncludingOffset.GetWidth();
                assert(nWidth && "div-by-zero");
                double fMaxWidthFactor = static_cast<double>(nCurrentWidth)
                                         / static_cast<double>(nWidth);
                tools::Long nHeight = aRectIncludingOffset.GetHeight();
                assert(nHeight && "div-by-zero");
                double fMaxHeightFactor = static_cast<double>(nCurrentHeight)
                                          / static_cast<double>(nHeight);
                double fMaxFactor = std::min(fMaxHeightFactor, fMaxWidthFactor);

                if(bIsGrowing) // cell is growing larger
                {
                    // To actually grow the image, we need to take the max
                    fWidthFactor = std::max(fWidthFactor, fHeightFactor);
                }
                else if(bIsShrinking) // cell is growing smaller, take the min
                {
                    fWidthFactor = std::min(fWidthFactor, fHeightFactor);
                }

                // We don't want the image to become larger than the current cell
                fWidthFactor = fHeightFactor = std::min(fWidthFactor, fMaxFactor);
            }

            if(bIsSizeChanged)
            {
                // tdf#116931 re-organized scaling (if needed)
                // Check if we need to scale at all. Always scale on growing.
                bool bNeedToScale(bIsGrowing);

                if(!bNeedToScale && bIsShrinking)
                {
                    // Check if original still fits into space. Do *not* forget to
                    // compare with evtl. numerically corrected aCurrentCellRect
                    const bool bFitsInX(aRect.Right() <= aCurrentCellRect.Left() + nCurrentWidth);
                    const bool bFitsInY(aRect.Bottom() <= aCurrentCellRect.Top() + nCurrentHeight);

                    // If the image still fits in the smaller cell, don't resize it at all
                    bNeedToScale = (!bFitsInX || !bFitsInY);
                }

                if(bNeedToScale)
                {
                    // tdf#116931 use transformations now. Translation is already applied
                    // (see aRect.SetPos above), so only scale needs to be applied - relative
                    // to *new* CellRect (which is aCurrentCellRect).
                    // Prepare scale relative to top-left of aCurrentCellRect
                    basegfx::B2DHomMatrix aChange;

                    aChange.translate(-aCurrentCellRect.getX(), -aCurrentCellRect.getY());
                    aChange.scale(fWidthFactor, fHeightFactor);
                    aChange.translate(aCurrentCellRect.getX(), aCurrentCellRect.getY());

                    // create B2DRange and transform by prepared scale
                    basegfx::B2DRange aNewRange = vcl::unotools::b2DRectangleFromRectangle(aRect);

                    aNewRange.transform(aChange);

                    // apply to aRect
                    aRect = tools::Rectangle(
                        basegfx::fround(aNewRange.getMinX()), basegfx::fround(aNewRange.getMinY()),
                        basegfx::fround(aNewRange.getMaxX()), basegfx::fround(aNewRange.getMaxY()));
                }
            }
        }
    }

    if (bNegativePage)
        MirrorRectRTL(aRect);

    rData.setShapeRect(GetDocument(), lcl_makeSafeRectangle(aRect), pObj->IsVisible());
}

void ScDrawLayer::InitializeCellAnchoredObj(SdrObject* pObj, ScDrawObjData& rData)
{
    // This is called from ScXMLImport::endDocument()
    if (!pDoc || !pObj)
        return;
    if (!rData.getShapeRect().IsEmpty())
        return; // already initialized, should not happen
    if (rData.meType == ScDrawObjData::CellNote || rData.meType == ScDrawObjData::ValidationCircle
        || rData.meType == ScDrawObjData::DetectiveArrow)
        return; // handled in RecalcPos

    // Prevent multiple broadcasts during the series of changes.
    bool bWasLocked = pObj->getSdrModelFromSdrObject().isLocked();
    pObj->getSdrModelFromSdrObject().setLock(true);

    // rNoRotatedAnchor refers in its start and end addresses and its start and end offsets to
    // the logic rectangle of the object. The values are so, as if no hidden columns and rows
    // exists and if it is a LTR sheet. These values are directly used for XML in ODF file.
    ScDrawObjData& rNoRotatedAnchor = *GetNonRotatedObjData(pObj, true /*bCreate*/);

    // From XML import, rData contains temporarily the anchor information as they are given in
    // XML. Copy it to rNoRotatedAnchor, where it belongs. rData will later contain the anchor
    // of the transformed object as visible on screen.
    rNoRotatedAnchor.maStart = rData.maStart;
    rNoRotatedAnchor.maEnd = rData.maEnd;
    rNoRotatedAnchor.maStartOffset = rData.maStartOffset;
    rNoRotatedAnchor.maEndOffset = rData.maEndOffset;

    SCCOL nCol1 = rNoRotatedAnchor.maStart.Col();
    SCROW nRow1 = rNoRotatedAnchor.maStart.Row();
    SCTAB nTab1 = rNoRotatedAnchor.maStart.Tab(); // Used as parameter several times

    // Object has coordinates relative to left/top of containing cell in XML. Change object to
    // absolute coordinates as internally used.
    const tools::Rectangle aRect(
        pDoc->GetMMRect(nCol1, nRow1, nCol1, nRow1, nTab1, false /*bHiddenAsZero*/));
    const Size aShift(aRect.Left(), aRect.Top());
    pObj->NbcMove(aShift);

    const ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType(*pObj);
    if (aAnchorType == SCA_CELL_RESIZE)
    {
        if (pObj->GetObjIdentifier() == OBJ_LINE)
        {
            // Horizontal lines might have wrong start and end anchor because of erroneously applied
            // 180deg rotation (tdf#137446). Other lines have wrong end anchor. Coordinates in
            // object are correct. Use them for recreating the anchor.
            const basegfx::B2DPolygon aPoly(
                static_cast<SdrPathObj*>(pObj)->GetPathPoly().getB2DPolygon(0));
            const basegfx::B2DPoint aB2DPoint0(aPoly.getB2DPoint(0));
            const basegfx::B2DPoint aB2DPoint1(aPoly.getB2DPoint(1));
            const Point aPointLT(FRound(std::min(aB2DPoint0.getX(), aB2DPoint1.getX())),
                                 FRound(std::min(aB2DPoint0.getY(), aB2DPoint1.getY())));
            const Point aPointRB(FRound(std::max(aB2DPoint0.getX(), aB2DPoint1.getX())),
                                 FRound(std::max(aB2DPoint0.getY(), aB2DPoint1.getY())));
            const tools::Rectangle aObjRect(aPointLT, aPointRB);
            GetCellAnchorFromPosition(aObjRect, rNoRotatedAnchor, *pDoc, nTab1,
                                      false /*bHiddenAsZero*/);
        }
        else if (pObj->GetObjIdentifier() == OBJ_MEASURE)
        {
            // Measure lines might have got wrong start and end anchor from XML import. Recreate
            // anchor from start and end point.
            SdrMeasureObj* pMeasureObj = dynamic_cast<SdrMeasureObj*>(pObj);
            // tdf#137576. The logic rectangle has likely no current values here, but only the
            // 1cm x 1cm default size. The call of TakeUnrotatedSnapRect is currently (LO 7.2)
            // the only way to force a recalc of the logic rectangle.
            tools::Rectangle aObjRect;
            pMeasureObj->TakeUnrotatedSnapRect(aObjRect);
            GetCellAnchorFromPosition(aObjRect, rNoRotatedAnchor, *pDoc, rData.maStart.Tab(),
                                      false /*bHiddenAsZero*/);
        }
        else
        {
            // In case there are hidden rows or cols, versions 7.0 and earlier have written width and
            // height in file so that hidden row or col are count as zero. XML import bases the
            // logical rectangle of the object on it. Shapes have at least wrong size, when row or col
            // are shown. We try to regenerate the logic rectangle as far as possible from the anchor.
            // ODF specifies anyway, that the size has to be ignored, if end cell attributes exist.
            lcl_SetLogicRectFromAnchor(pObj, rNoRotatedAnchor, pDoc);
        }
    }
    else // aAnchorType == SCA_CELL, other types will not occur here.
    {
        // XML has no end cell address in this case. We generate it from position.
        UpdateCellAnchorFromPositionEnd(*pObj, rNoRotatedAnchor, *pDoc, nTab1,
                                        true /*bUseLogicRect*/);
    }

    // Make sure maShapeRect of rNoRotatedAnchor is not empty. Method ScDrawView::Notify()
    // needs it to detect a change in object geometry. For example a 180deg rotation effects only
    // logic rect.
    rNoRotatedAnchor.setShapeRect(GetDocument(), pObj->GetLogicRect(), true);

    // Start and end addresses and offsets in rData refer to the actual snap rectangle of the
    // shape. We initialize them here based on the "full" sized object. Adaptation to reduced size
    // (by hidden row/col) is done later in RecalcPos.
    GetCellAnchorFromPosition(pObj->GetSnapRect(), rData, *pDoc, nTab1, false /*bHiddenAsZero*/);

    // As of ODF 1.3 strict there is no attribute to store whether an object is hidden. So a "visible"
    // object might actually be hidden by being in hidden row or column. We detect it here.
    // Note, that visibility by hidden row or column refers to the snap rectangle.
    if (pObj->IsVisible()
        && (pDoc->RowHidden(rData.maStart.Row(), rData.maStart.Tab())
            || pDoc->ColHidden(rData.maStart.Col(), rData.maStart.Tab())))
        pObj->SetVisible(false);

    // Set visibility. ToDo: Really used?
    rNoRotatedAnchor.setShapeRect(GetDocument(), pObj->GetLogicRect(), pObj->IsVisible());

    // And set maShapeRect in rData. It stores not only the current rectangles, but currently,
    // existence of maShapeRect is the flag for initialization is done.
    rData.setShapeRect(GetDocument(), pObj->GetSnapRect(), pObj->IsVisible());

    pObj->getSdrModelFromSdrObject().setLock(bWasLocked);
}

void ScDrawLayer::RecalcPos( SdrObject* pObj, ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::RecalcPos - missing document" );
    if( !pDoc )
        return;

    if (rData.meType == ScDrawObjData::CellNote)
    {
        OSL_ENSURE( rData.maStart.IsValid(), "ScDrawLayer::RecalcPos - invalid position for cell note" );
        /*  #i109372# On insert/remove rows/columns/cells: Updating the caption
            position must not be done, if the cell containing the note has not
            been moved yet in the document. The calling code now passes an
            additional boolean stating if the cells are already moved. */
        if( bUpdateNoteCaptionPos )
            /*  When inside an undo action, there may be pending note captions
                where cell note is already deleted (thus document cannot find
                the note object anymore). The caption will be deleted later
                with drawing undo. */
            if( ScPostIt* pNote = pDoc->GetNote( rData.maStart ) )
                pNote->UpdateCaptionPos( rData.maStart );
        return;
    }

    bool bValid1 = rData.maStart.IsValid();
    SCCOL nCol1 = rData.maStart.Col();
    SCROW nRow1 = rData.maStart.Row();
    SCTAB nTab1 = rData.maStart.Tab();
    bool bValid2 = rData.maEnd.IsValid();
    SCCOL nCol2 = rData.maEnd.Col();
    SCROW nRow2 = rData.maEnd.Row();
    SCTAB nTab2 = rData.maEnd.Tab();

    if (rData.meType == ScDrawObjData::ValidationCircle)
    {
        // Validation circle for detective.
        rData.setShapeRect(GetDocument(), pObj->GetLogicRect());

        // rData.maStart should contain the address of the be validated cell.
        tools::Rectangle aRect = GetCellRect(*GetDocument(), rData.maStart, true);
        aRect.AdjustLeft( -250 );
        aRect.AdjustRight(250 );
        aRect.AdjustTop( -70 );
        aRect.AdjustBottom(70 );
        if ( bNegativePage )
            MirrorRectRTL( aRect );

        if ( pObj->GetLogicRect() != aRect )
        {
            if (bRecording)
                AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );
            rData.setShapeRect(GetDocument(), lcl_makeSafeRectangle(aRect));
            // maStart has the meaning of "to be validated cell" in a validation circle. For usual
            // drawing objects it has the meaning "left/top of logic/snap rect". Because the rectangle
            // is expanded above, SetLogicRect() will set maStart to one cell left and one cell above
            // of the to be validated cell. We need to backup the old value and restore it.
            ScAddress aBackup(rData.maStart);
            pObj->SetLogicRect(rData.getShapeRect());
            rData.maStart = aBackup;
        }
    }
    else if (rData.meType == ScDrawObjData::DetectiveArrow)
    {
        rData.setShapeRect(GetDocument(), pObj->GetLogicRect());
        basegfx::B2DPolygon aCalcPoly;
        Point aOrigStartPos(pObj->GetPoint(0));
        Point aOrigEndPos(pObj->GetPoint(1));
        aCalcPoly.append(basegfx::B2DPoint(aOrigStartPos.X(), aOrigStartPos.Y()));
        aCalcPoly.append(basegfx::B2DPoint(aOrigEndPos.X(), aOrigEndPos.Y()));
        //TODO: do not create multiple Undos for one object (last one can be omitted then)

        SCCOL nLastCol;
        SCROW nLastRow;
        if( bValid1 )
        {
            Point aPos( pDoc->GetColOffset( nCol1, nTab1 ), pDoc->GetRowOffset( nRow1, nTab1 ) );
            if (!pDoc->ColHidden(nCol1, nTab1, nullptr, &nLastCol))
                aPos.AdjustX(pDoc->GetColWidth( nCol1, nTab1 ) / 4 );
            if (!pDoc->RowHidden(nRow1, nTab1, nullptr, &nLastRow))
                aPos.AdjustY(pDoc->GetRowHeight( nRow1, nTab1 ) / 2 );
            aPos.setX(TwipsToHmm( aPos.X() ));
            aPos.setY(TwipsToHmm( aPos.Y() ));
            Point aStartPos = aPos;
            if ( bNegativePage )
                aStartPos.setX( -aStartPos.X() );     // don't modify aPos - used below
            if ( pObj->GetPoint( 0 ) != aStartPos )
            {
                if (bRecording)
                    AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );

                rData.setShapeRect(GetDocument(), lcl_UpdateCalcPoly(aCalcPoly, 0, aStartPos));
                pObj->SetPoint( aStartPos, 0 );
            }

            if( !bValid2 )
            {
                Point aEndPos( aPos.X() + DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aEndPos.Y() < 0)
                    aEndPos.AdjustY(2 * DET_ARROW_OFFSET);
                if ( bNegativePage )
                    aEndPos.setX( -aEndPos.X() );
                if ( pObj->GetPoint( 1 ) != aEndPos )
                {
                    if (bRecording)
                        AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );

                    rData.setShapeRect(GetDocument(), lcl_UpdateCalcPoly(aCalcPoly, 1, aEndPos));
                    pObj->SetPoint( aEndPos, 1 );
                }
            }
        }
        if( bValid2 )
        {
            Point aPos( pDoc->GetColOffset( nCol2, nTab2 ), pDoc->GetRowOffset( nRow2, nTab2 ) );
            if (!pDoc->ColHidden(nCol2, nTab2, nullptr, &nLastCol))
                aPos.AdjustX(pDoc->GetColWidth( nCol2, nTab2 ) / 4 );
            if (!pDoc->RowHidden(nRow2, nTab2, nullptr, &nLastRow))
                aPos.AdjustY(pDoc->GetRowHeight( nRow2, nTab2 ) / 2 );
            aPos.setX(TwipsToHmm( aPos.X() ));
            aPos.setY(TwipsToHmm( aPos.Y() ));
            Point aEndPos = aPos;
            if ( bNegativePage )
                aEndPos.setX( -aEndPos.X() );         // don't modify aPos - used below
            if ( pObj->GetPoint( 1 ) != aEndPos )
            {
                if (bRecording)
                    AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );

                rData.setShapeRect(GetDocument(), lcl_UpdateCalcPoly(aCalcPoly, 1, aEndPos));
                pObj->SetPoint( aEndPos, 1 );
            }

            if( !bValid1 )
            {
                Point aStartPos( aPos.X() - DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aStartPos.X() < 0)
                    aStartPos.AdjustX(2 * DET_ARROW_OFFSET);
                if (aStartPos.Y() < 0)
                    aStartPos.AdjustY(2 * DET_ARROW_OFFSET);
                if ( bNegativePage )
                    aStartPos.setX( -aStartPos.X() );
                if ( pObj->GetPoint( 0 ) != aStartPos )
                {
                    if (bRecording)
                        AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );

                    rData.setShapeRect(GetDocument(), lcl_UpdateCalcPoly(aCalcPoly, 0, aStartPos));
                    pObj->SetPoint( aStartPos, 0 );
                }
            }
        }
    } // end ScDrawObjData::DetectiveArrow
    else // start ScDrawObjData::DrawingObject
    {
        // Do not change hidden objects. That would produce zero height or width and loss of offsets.
        if (!pObj->IsVisible())
            return;

        // Prevent multiple broadcasts during the series of changes.
        bool bWasLocked = pObj->getSdrModelFromSdrObject().isLocked();
        pObj->getSdrModelFromSdrObject().setLock(true);

        bool bCanResize = bValid2 && !pObj->IsResizeProtect() && rData.mbResizeWithCell;

        // update anchor with snap rect
        ResizeLastRectFromAnchor( pObj, rData, false, bNegativePage, bCanResize );

        ScDrawObjData& rNoRotatedAnchor = *GetNonRotatedObjData( pObj, true /*bCreate*/);

        if( bCanResize )
        {
            tools::Rectangle aNew = rData.getShapeRect();
            tools::Rectangle aOld(pObj->GetSnapRect());
            if (!lcl_AreRectanglesApproxEqual(aNew, aOld))
            {
                if (bRecording)
                    AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );

                // ToDo: Implement NbcSetSnapRect of SdrMeasureObj. Then this can be removed.
                tools::Long nOldWidth = aOld.GetWidth();
                tools::Long nOldHeight = aOld.GetHeight();
                if (pObj->IsPolyObj() && nOldWidth && nOldHeight)
                {
                    // Polyline objects need special treatment.
                    Size aSizeMove(aNew.Left()-aOld.Left(), aNew.Top()-aOld.Top());
                    pObj->NbcMove(aSizeMove);

                    double fXFrac = static_cast<double>(aNew.GetWidth()) / static_cast<double>(nOldWidth);
                    double fYFrac = static_cast<double>(aNew.GetHeight()) / static_cast<double>(nOldHeight);
                    pObj->NbcResize(aNew.TopLeft(), Fraction(fXFrac), Fraction(fYFrac));
                }

                rData.setShapeRect(GetDocument(), lcl_makeSafeRectangle(rData.getShapeRect()), pObj->IsVisible());
                if (pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE)
                    pObj->AdjustToMaxRect(rData.getShapeRect());
                else
                    pObj->SetSnapRect(rData.getShapeRect());

                // The shape rectangle in the 'unrotated' anchor needs to be updated to the changed
                // object geometry. It is used in adjustAnchoredPosition() in ScDrawView::Notify().
                rNoRotatedAnchor.setShapeRect(pDoc, pObj->GetLogicRect(), pObj->IsVisible());
            }
        }
        else
        {
            Point aPos( rData.getShapeRect().getX(), rData.getShapeRect().getY() );
            if ( pObj->GetRelativePos() != aPos )
            {
                if (bRecording)
                    AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );
                pObj->SetRelativePos( aPos );
                rNoRotatedAnchor.setShapeRect(pDoc, pObj->GetLogicRect(), pObj->IsVisible());
            }
        }
        /*
         * If we were not allowed resize the object, then the end cell anchor
         * is possibly incorrect now, and if the object has no end-cell (e.g.
         * missing in original .xml) we are also forced to generate one
        */
        bool bEndAnchorIsBad = !bValid2 || pObj->IsResizeProtect();
        if (bEndAnchorIsBad)
        {
            // update 'rotated' anchor
            ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, rData, *pDoc, nTab1, false);
            // update 'unrotated' anchor
            ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, rNoRotatedAnchor, *pDoc, nTab1 );
        }

        // End prevent multiple broadcasts during the series of changes.
        pObj->getSdrModelFromSdrObject().setLock(bWasLocked);
        if (!bWasLocked)
            pObj->BroadcastObjectChange();
    } // end ScDrawObjData::DrawingObject
}

bool ScDrawLayer::GetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const
{
    OSL_ENSURE( pDoc, "ScDrawLayer::GetPrintArea without document" );
    if ( !pDoc )
        return false;

    SCTAB nTab = rRange.aStart.Tab();
    OSL_ENSURE( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab differ" );

    bool bNegativePage = pDoc->IsNegativePage( nTab );

    bool bAny = false;
    tools::Long nEndX = 0;
    tools::Long nEndY = 0;
    tools::Long nStartX = LONG_MAX;
    tools::Long nStartY = LONG_MAX;

    // Calculate borders

    if (!bSetHor)
    {
        nStartX = 0;
        SCCOL nStartCol = rRange.aStart.Col();
        SCCOL i;
        for (i=0; i<nStartCol; i++)
            nStartX +=pDoc->GetColWidth(i,nTab);
        nEndX = nStartX;
        SCCOL nEndCol = rRange.aEnd.Col();
        for (i=nStartCol; i<=nEndCol; i++)
            nEndX += pDoc->GetColWidth(i,nTab);
        nStartX = TwipsToHmm( nStartX );
        nEndX   = TwipsToHmm( nEndX );
    }
    if (!bSetVer)
    {
        nStartY = pDoc->GetRowHeight( 0, rRange.aStart.Row()-1, nTab);
        nEndY = nStartY + pDoc->GetRowHeight( rRange.aStart.Row(),
                rRange.aEnd.Row(), nTab);
        nStartY = TwipsToHmm( nStartY );
        nEndY   = TwipsToHmm( nEndY );
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     // positions are negative, swap start/end so the same comparisons work
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    const SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (pPage)
    {
        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
                            //TODO: test Flags (hidden?)

            tools::Rectangle aObjRect = pObject->GetCurrentBoundRect();
            bool bFit = true;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = false;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = false;
            // #i104716# don't include hidden note objects
            if ( bFit && pObject->GetLayer() != SC_LAYER_HIDDEN )
            {
                if (bSetHor)
                {
                    if (aObjRect.Left() < nStartX) nStartX = aObjRect.Left();
                    if (aObjRect.Right()  > nEndX) nEndX = aObjRect.Right();
                }
                if (bSetVer)
                {
                    if (aObjRect.Top()  < nStartY) nStartY = aObjRect.Top();
                    if (aObjRect.Bottom() > nEndY) nEndY = aObjRect.Bottom();
                }
                bAny = true;
            }

            pObject = aIter.Next();
        }
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     // reverse transformation, so the same cell address calculation works
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    if (bAny)
    {
        OSL_ENSURE( nStartX<=nEndX && nStartY<=nEndY, "Start/End wrong in ScDrawLayer::GetPrintArea" );

        if (bSetHor)
        {
            nStartX = HmmToTwips( nStartX );
            nEndX = HmmToTwips( nEndX );
            tools::Long nWidth;

            nWidth = 0;
            rRange.aStart.SetCol( 0 );
            if (nWidth <= nStartX)
            {
                for (SCCOL nCol : pDoc->GetColumnsRange(nTab, 0, MAXCOL))
                {
                    nWidth += pDoc->GetColWidth(nCol,nTab);
                    if (nWidth > nStartX)
                    {
                        rRange.aStart.SetCol( nCol );
                        break;
                    }
                }
            }

            nWidth = 0;
            rRange.aEnd.SetCol( 0 );
            if (nWidth <= nEndX)
            {
                for (SCCOL nCol : pDoc->GetColumnsRange(nTab, 0, MAXCOL)) //TODO: start at Start
                {
                    nWidth += pDoc->GetColWidth(nCol,nTab);
                    if (nWidth > nEndX)
                    {
                        rRange.aEnd.SetCol( nCol );
                        break;
                    }
                }
            }
        }

        if (bSetVer)
        {
            nStartY = HmmToTwips( nStartY );
            nEndY = HmmToTwips( nEndY );
            SCROW nRow = pDoc->GetRowForHeight( nTab, nStartY);
            rRange.aStart.SetRow( nRow>0 ? (nRow-1) : 0);
            nRow = pDoc->GetRowForHeight( nTab, nEndY);
            rRange.aEnd.SetRow( nRow == MAXROW ? MAXROW :
                    (nRow>0 ? (nRow-1) : 0));
        }
    }
    else
    {
        if (bSetHor)
        {
            rRange.aStart.SetCol(0);
            rRange.aEnd.SetCol(0);
        }
        if (bSetVer)
        {
            rRange.aStart.SetRow(0);
            rRange.aEnd.SetRow(0);
        }
    }
    return bAny;
}

void ScDrawLayer::AddCalcUndo( std::unique_ptr<SdrUndoAction> pUndo )
{
    if (bRecording)
    {
        if (!pUndoGroup)
            pUndoGroup.reset(new SdrUndoGroup(*this));

        pUndoGroup->AddAction( std::move(pUndo) );
    }
}

void ScDrawLayer::BeginCalcUndo(bool bDisableTextEditUsesCommonUndoManager)
{
    SetDisableTextEditUsesCommonUndoManager(bDisableTextEditUsesCommonUndoManager);
    pUndoGroup.reset();
    bRecording = true;
}

std::unique_ptr<SdrUndoGroup> ScDrawLayer::GetCalcUndo()
{
    std::unique_ptr<SdrUndoGroup> pRet = std::move(pUndoGroup);
    bRecording = false;
    SetDisableTextEditUsesCommonUndoManager(false);
    return pRet;
}

void ScDrawLayer::MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                            SCCOL nDx,SCROW nDy, bool bInsDel, bool bUpdateNoteCaptionPos )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::MoveArea without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    bool bNegativePage = pDoc->IsNegativePage( nTab );

    tools::Rectangle aRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );
    lcl_ReverseTwipsToMM( aRect );
    //TODO: use twips directly?

    Point aMove;

    if (nDx > 0)
        for (SCCOL s=0; s<nDx; s++)
            aMove.AdjustX(pDoc->GetColWidth(s+nCol1,nTab) );
    else
        for (SCCOL s=-1; s>=nDx; s--)
            aMove.AdjustX( -(pDoc->GetColWidth(s+nCol1,nTab)) );
    if (nDy > 0)
        aMove.AdjustY(pDoc->GetRowHeight( nRow1, nRow1+nDy-1, nTab) );
    else
        aMove.AdjustY( -sal_Int16(pDoc->GetRowHeight( nRow1+nDy, nRow1-1, nTab)) );

    if ( bNegativePage )
        aMove.setX( -aMove.X() );

    Point aTopLeft = aRect.TopLeft();       // Beginning when zoomed out
    if (bInsDel)
    {
        if ( aMove.X() != 0 && nDx < 0 )    // nDx counts cells, sign is independent of RTL
            aTopLeft.AdjustX(aMove.X() );
        if ( aMove.Y() < 0 )
            aTopLeft.AdjustY(aMove.Y() );
    }

        //      Detectiv arrows: Adjust cell position

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy, bUpdateNoteCaptionPos );
}

bool ScDrawLayer::HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
    if ( !pDoc )
        return false;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return false;

    // for an empty page, there's no need to calculate the row heights
    if (!pPage->GetObjCount())
        return false;

    tools::Rectangle aTestRect;

    aTestRect.AdjustTop(pDoc->GetRowHeight( 0, nStartRow-1, nTab) );

    if (nEndRow==MAXROW)
        aTestRect.SetBottom( MAXMM );
    else
    {
        aTestRect.SetBottom( aTestRect.Top() );
        aTestRect.AdjustBottom(pDoc->GetRowHeight( nStartRow, nEndRow, nTab) );
        aTestRect.SetBottom(TwipsToHmm( aTestRect.Bottom() ));
    }

    aTestRect.SetTop(TwipsToHmm( aTestRect.Top() ));

    aTestRect.SetLeft( 0 );
    aTestRect.SetRight( MAXMM );

    bool bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
        MirrorRectRTL( aTestRect );

    bool bFound = false;

    tools::Rectangle aObjRect;
    SdrObjListIter aIter( pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = pObject->GetSnapRect();  //TODO: GetLogicRect ?
        if (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft()))
            bFound = true;

        pObject = aIter.Next();
    }

    return bFound;
}

void ScDrawLayer::DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2, bool bAnchored )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::DeleteObjectsInArea without document" );
    if ( !pDoc )
        return;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return;

    pPage->RecalcObjOrdNums();

    const size_t nObjCount = pPage->GetObjCount();
    if (!nObjCount)
        return;

    size_t nDelCount = 0;
    tools::Rectangle aDelRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );
    tools::Rectangle aDelCircle = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );
    aDelCircle.AdjustLeft(-250);
    aDelCircle.AdjustRight(250);
    aDelCircle.AdjustTop(-70);
    aDelCircle.AdjustBottom(70);

    std::unique_ptr<SdrObject*[]> ppObj(new SdrObject*[nObjCount]);

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        // do not delete note caption, they are always handled by the cell note
        // TODO: detective objects are still deleted, is this desired?
        if (!IsNoteCaption( pObject ))
        {
            tools::Rectangle aObjRect;
            ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObject);
            if (pObjData && pObjData->meType == ScDrawObjData::ValidationCircle)
            {
                aObjRect = pObject->GetLogicRect();
                if(aDelCircle.IsInside(aObjRect))
                   ppObj[nDelCount++] = pObject;
            }
            else
            {
                aObjRect = pObject->GetCurrentBoundRect();
                if (aDelRect.IsInside(aObjRect))
                {
                    if (bAnchored)
                    {
                        ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType(*pObject);
                        if (aAnchorType == SCA_CELL || aAnchorType == SCA_CELL_RESIZE)
                            ppObj[nDelCount++] = pObject;
                    }
                    else
                        ppObj[nDelCount++] = pObject;
                }
            }
        }

        pObject = aIter.Next();
    }

    if (bRecording)
        for (size_t i=1; i<=nDelCount; ++i)
            AddCalcUndo( std::make_unique<SdrUndoRemoveObj>( *ppObj[nDelCount-i] ) );

    for (size_t i=1; i<=nDelCount; ++i)
        pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );
}

void ScDrawLayer::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::DeleteObjectsInSelection without document" );
    if ( !pDoc )
        return;

    if ( !rMark.IsMultiMarked() )
        return;

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

    SCTAB nTabCount = pDoc->GetTableCount();
    for (const SCTAB nTab : rMark)
    {
        if (nTab >= nTabCount)
            break;

        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        if (pPage)
        {
            pPage->RecalcObjOrdNums();
            const size_t nObjCount = pPage->GetObjCount();
            if (nObjCount)
            {
                size_t nDelCount = 0;
                //  Rectangle around the whole selection
                tools::Rectangle aMarkBound = pDoc->GetMMRect(
                            aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab );

                std::unique_ptr<SdrObject*[]> ppObj(new SdrObject*[nObjCount]);

                SdrObjListIter aIter( pPage, SdrIterMode::Flat );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    // do not delete note caption, they are always handled by the cell note
                    // TODO: detective objects are still deleted, is this desired?
                    if (!IsNoteCaption( pObject ))
                    {
                        tools::Rectangle aObjRect = pObject->GetCurrentBoundRect();
                        ScRange aRange = pDoc->GetRange(nTab, aObjRect);
                        bool bObjectInMarkArea =
                            aMarkBound.IsInside(aObjRect) && rMark.IsAllMarked(aRange);
                        const ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pObject);
                        ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType(*pObject);
                        bool bObjectAnchoredToMarkedCell
                            = ((aAnchorType == SCA_CELL || aAnchorType == SCA_CELL_RESIZE)
                               && pObjData && rMark.IsCellMarked(pObjData->maStart.Col(),
                                                                 pObjData->maStart.Row()));
                        if (bObjectInMarkArea || bObjectAnchoredToMarkedCell)
                        {
                            ppObj[nDelCount++] = pObject;
                        }
                    }

                    pObject = aIter.Next();
                }

                //  Delete objects (backwards)

                if (bRecording)
                    for (size_t i=1; i<=nDelCount; ++i)
                        AddCalcUndo( std::make_unique<SdrUndoRemoveObj>( *ppObj[nDelCount-i] ) );

                for (size_t i=1; i<=nDelCount; ++i)
                    pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );
            }
        }
        else
        {
            OSL_FAIL("pPage?");
        }
    }
}

void ScDrawLayer::CopyToClip( ScDocument* pClipDoc, SCTAB nTab, const tools::Rectangle& rRange )
{
    //  copy everything in the specified range into the same page (sheet) in the clipboard doc

    SdrPage* pSrcPage = GetPage(static_cast<sal_uInt16>(nTab));
    if (!pSrcPage)
        return;

    ScDrawLayer* pDestModel = nullptr;
    SdrPage* pDestPage = nullptr;

    SdrObjListIter aIter( pSrcPage, SdrIterMode::Flat );
    SdrObject* pOldObject = aIter.Next();
    while (pOldObject)
    {
        tools::Rectangle aObjRect = pOldObject->GetCurrentBoundRect();

        bool bObjectInArea = rRange.IsInside(aObjRect);
        const ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pOldObject);
        if (pObjData)
        {
            ScRange aClipRange = lcl_getClipRangeFromClipDoc(pClipDoc, nTab);
            bObjectInArea = bObjectInArea || aClipRange.In(pObjData->maStart);
        }

        // do not copy internal objects (detective) and note captions
        if (bObjectInArea && pOldObject->GetLayer() != SC_LAYER_INTERN
            && !IsNoteCaption(pOldObject))
        {
            if ( !pDestModel )
            {
                pDestModel = pClipDoc->GetDrawLayer();      // does the document already have a drawing layer?
                if ( !pDestModel )
                {
                    //  allocate drawing layer in clipboard document only if there are objects to copy

                    pClipDoc->InitDrawLayer();                  //TODO: create contiguous pages
                    pDestModel = pClipDoc->GetDrawLayer();
                }
                if (pDestModel)
                    pDestPage = pDestModel->GetPage( static_cast<sal_uInt16>(nTab) );
            }

            OSL_ENSURE( pDestPage, "no page" );
            if (pDestPage)
            {
                // Clone to target SdrModel
                SdrObject* pNewObject(pOldObject->CloneSdrObject(*pDestModel));

                uno::Reference< chart2::XChartDocument > xOldChart( ScChartHelper::GetChartFromSdrObject( pOldObject ) );
                if(!xOldChart.is())//#i110034# do not move charts as they lose all their data references otherwise
                    pNewObject->NbcMove(Size(0,0));
                pDestPage->InsertObject( pNewObject );

                //  no undo needed in clipboard document
                //  charts are not updated
            }
        }

        pOldObject = aIter.Next();
    }
}

static bool lcl_IsAllInRange( const ::std::vector< ScRangeList >& rRangesVector, const ScRange& rClipRange )
{
    //  check if every range of rRangesVector is completely in rClipRange

    for( const ScRangeList& rRanges : rRangesVector )
    {
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            const ScRange & rRange = rRanges[ i ];
            if ( !rClipRange.In( rRange ) )
            {
                return false;   // at least one range is not valid
            }
        }
    }

    return true;            // everything is fine
}

static bool lcl_MoveRanges( ::std::vector< ScRangeList >& rRangesVector, const ScRange& rSourceRange, const ScAddress& rDestPos )
{
    bool bChanged = false;

    ScRange aErrorRange( ScAddress::UNINITIALIZED );
    for( ScRangeList& rRanges : rRangesVector )
    {
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            ScRange & rRange = rRanges[ i ];
            if ( rSourceRange.In( rRange ) )
            {
                SCCOL nDiffX = rDestPos.Col() - rSourceRange.aStart.Col();
                SCROW nDiffY = rDestPos.Row() - rSourceRange.aStart.Row();
                SCTAB nDiffZ = rDestPos.Tab() - rSourceRange.aStart.Tab();
                if (!rRange.Move( nDiffX, nDiffY, nDiffZ, aErrorRange))
                {
                    assert(!"can't move range");
                }
                bChanged = true;
            }
        }
    }

    return bChanged;
}

void ScDrawLayer::CopyFromClip( ScDrawLayer* pClipModel, SCTAB nSourceTab, const tools::Rectangle& rSourceRange,
                                    const ScAddress& rDestPos, const tools::Rectangle& rDestRange )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::CopyFromClip without document" );
    if ( !pDoc )
        return;

    if (!pClipModel)
        return;

    if (bDrawIsInUndo)      //TODO: can this happen?
    {
        OSL_FAIL("CopyFromClip, bDrawIsInUndo");
        return;
    }

    bool bMirrorObj = ( rSourceRange.Left() < 0 && rSourceRange.Right() < 0 &&
                        rDestRange.Left()   > 0 && rDestRange.Right()   > 0 ) ||
                      ( rSourceRange.Left() > 0 && rSourceRange.Right() > 0 &&
                        rDestRange.Left()   < 0 && rDestRange.Right()   < 0 );
    tools::Rectangle aMirroredSource = rSourceRange;
    if ( bMirrorObj )
        MirrorRectRTL( aMirroredSource );

    SCTAB nDestTab = rDestPos.Tab();

    SdrPage* pSrcPage = pClipModel->GetPage(static_cast<sal_uInt16>(nSourceTab));
    SdrPage* pDestPage = GetPage(static_cast<sal_uInt16>(nDestTab));
    OSL_ENSURE( pSrcPage && pDestPage, "draw page missing" );
    if ( !pSrcPage || !pDestPage )
        return;

    SdrObjListIter aIter( pSrcPage, SdrIterMode::Flat );
    SdrObject* pOldObject = aIter.Next();

    ScDocument* pClipDoc = pClipModel->GetDocument();
    //  a clipboard document and its source share the same document item pool,
    //  so the pointers can be compared to see if this is copy&paste within
    //  the same document
    bool bSameDoc = pDoc && pClipDoc && pDoc->GetPool() == pClipDoc->GetPool();
    bool bDestClip = pDoc && pDoc->IsClipboard();

    //#i110034# charts need correct sheet names for xml range conversion during load
    //so the target sheet name is temporarily renamed (if we have any SdrObjects)
    OUString aDestTabName;
    bool bRestoreDestTabName = false;
    if( pOldObject && !bSameDoc && !bDestClip )
    {
        if( pDoc && pClipDoc )
        {
            OUString aSourceTabName;
            if( pClipDoc->GetName( nSourceTab, aSourceTabName )
                && pDoc->GetName( nDestTab, aDestTabName ) )
            {
                if( aSourceTabName != aDestTabName &&
                    pDoc->ValidNewTabName(aSourceTabName) )
                {
                    bRestoreDestTabName = pDoc->RenameTab( nDestTab, aSourceTabName );
                }
            }
        }
    }

    // first mirror, then move
    Size aMove( rDestRange.Left() - aMirroredSource.Left(), rDestRange.Top() - aMirroredSource.Top() );

    tools::Long nDestWidth = rDestRange.GetWidth();
    tools::Long nDestHeight = rDestRange.GetHeight();
    tools::Long nSourceWidth = rSourceRange.GetWidth();
    tools::Long nSourceHeight = rSourceRange.GetHeight();

    tools::Long nWidthDiff = nDestWidth - nSourceWidth;
    tools::Long nHeightDiff = nDestHeight - nSourceHeight;

    Fraction aHorFract(1,1);
    Fraction aVerFract(1,1);
    bool bResize = false;
    // sizes can differ by 1 from twips->1/100mm conversion for equal cell sizes,
    // don't resize to empty size when pasting into hidden columns or rows
    if ( std::abs(nWidthDiff) > 1 && nDestWidth > 1 && nSourceWidth > 1 )
    {
        aHorFract = Fraction( nDestWidth, nSourceWidth );
        bResize = true;
    }
    if ( std::abs(nHeightDiff) > 1 && nDestHeight > 1 && nSourceHeight > 1 )
    {
        aVerFract = Fraction( nDestHeight, nSourceHeight );
        bResize = true;
    }
    Point aRefPos = rDestRange.TopLeft();       // for resizing (after moving)

    while (pOldObject)
    {
        tools::Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
        // do not copy internal objects (detective) and note captions

        SCTAB nClipTab = bRestoreDestTabName ? nDestTab : nSourceTab;
        ScRange aClipRange = lcl_getClipRangeFromClipDoc(pClipDoc, nClipTab);

        bool bObjectInArea = rSourceRange.IsInside(aObjRect);
        const ScDrawObjData* pObjData = ScDrawLayer::GetObjData(pOldObject);
        if (pObjData) // Consider images anchored to the copied cell
            bObjectInArea = bObjectInArea || aClipRange.In(pObjData->maStart);
        if (bObjectInArea && (pOldObject->GetLayer() != SC_LAYER_INTERN)
            && !IsNoteCaption(pOldObject))
        {
            // Clone to target SdrModel
            SdrObject* pNewObject(pOldObject->CloneSdrObject(*this));

            if ( bMirrorObj )
                MirrorRTL( pNewObject );        // first mirror, then move

            pNewObject->NbcMove( aMove );
            if ( bResize )
                pNewObject->NbcResize( aRefPos, aHorFract, aVerFract );

            pDestPage->InsertObject( pNewObject );
            if (bRecording)
                AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pNewObject ) );

            //#i110034# handle chart data references (after InsertObject)

            if ( pNewObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                uno::Reference< embed::XEmbeddedObject > xIPObj = static_cast<SdrOle2Obj*>(pNewObject)->GetObjRef();
                uno::Reference< embed::XClassifiedObject > xClassified = xIPObj;
                SvGlobalName aObjectClassName;
                if ( xClassified.is() )
                {
                    try {
                        aObjectClassName = SvGlobalName( xClassified->getClassID() );
                    } catch( uno::Exception& )
                    {
                        // TODO: handle error?
                    }
                }

                if ( xIPObj.is() && SotExchange::IsChart( aObjectClassName ) )
                {
                    uno::Reference< chart2::XChartDocument > xNewChart( ScChartHelper::GetChartFromSdrObject( pNewObject ) );
                    if( xNewChart.is() && !xNewChart->hasInternalDataProvider() )
                    {
                        OUString aChartName = static_cast<SdrOle2Obj*>(pNewObject)->GetPersistName();
                        ::std::vector< ScRangeList > aRangesVector;
                        pDoc->GetChartRanges( aChartName, aRangesVector, *pDoc );
                        if( !aRangesVector.empty() )
                        {
                            bool bInSourceRange = false;
                            if ( pClipDoc )
                            {
                                bInSourceRange = lcl_IsAllInRange( aRangesVector, aClipRange );
                            }

                            // always lose references when pasting into a clipboard document (transpose)
                            if ( ( bInSourceRange || bSameDoc ) && !bDestClip )
                            {
                                if ( bInSourceRange )
                                {
                                    if ( rDestPos != aClipRange.aStart )
                                    {
                                        //  update the data ranges to the new (copied) position
                                        if ( lcl_MoveRanges( aRangesVector, aClipRange, rDestPos ) )
                                            pDoc->SetChartRanges( aChartName, aRangesVector );
                                    }
                                }
                                else
                                {
                                    //  leave the ranges unchanged
                                }
                            }
                            else
                            {
                                //  pasting into a new document without the complete source data
                                //  -> break connection to source data and switch to own data

                                uno::Reference< chart::XChartDocument > xOldChartDoc( ScChartHelper::GetChartFromSdrObject( pOldObject ), uno::UNO_QUERY );
                                uno::Reference< chart::XChartDocument > xNewChartDoc( xNewChart, uno::UNO_QUERY );
                                if( xOldChartDoc.is() && xNewChartDoc.is() )
                                    xNewChartDoc->attachData( xOldChartDoc->getData() );

                                //  (see ScDocument::UpdateChartListenerCollection, PastingDrawFromOtherDoc)
                            }
                        }
                    }
                }
            }
        }

        pOldObject = aIter.Next();
    }

    if( bRestoreDestTabName )
        pDoc->RenameTab( nDestTab, aDestTabName );
}

void ScDrawLayer::MirrorRTL( SdrObject* pObj )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::MirrorRTL - missing document" );
    if( !pDoc )
        return;

    sal_uInt16 nIdent = pObj->GetObjIdentifier();

    //  don't mirror OLE or graphics, otherwise ask the object
    //  if it can be mirrored
    bool bCanMirror = ( nIdent != OBJ_GRAF && nIdent != OBJ_OLE2 );
    if (bCanMirror)
    {
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo( aInfo );
        bCanMirror = aInfo.bMirror90Allowed;
    }

    if (bCanMirror)
    {
        ScDrawObjData* pData = GetObjData(pObj);
        if (pData) // cell anchored
        {
            // Remember values from positive side.
            const tools::Rectangle aOldSnapRect = pObj->GetSnapRect();
            const tools::Rectangle aOldLogicRect = pObj->GetLogicRect();
            // Generate noRotate anchor if missing.
            ScDrawObjData* pNoRotatedAnchor = GetNonRotatedObjData(pObj);
            if (!pNoRotatedAnchor)
            {
                ScDrawObjData aNoRotateAnchor;
                const tools::Rectangle aLogicRect(pObj->GetLogicRect());
                GetCellAnchorFromPosition(aLogicRect, aNoRotateAnchor,
                              *pDoc, pData->maStart.Tab());
                aNoRotateAnchor.mbResizeWithCell = pData->mbResizeWithCell;
                SetNonRotatedAnchor(*pObj, aNoRotateAnchor);
                pNoRotatedAnchor = GetNonRotatedObjData(pObj);
                assert(pNoRotatedAnchor);
            }
            // Mirror object at vertical axis
            Point aRef1( 0, 0 );
            Point aRef2( 0, 1 );
            if (bRecording)
                AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );
            pObj->Mirror( aRef1, aRef2 );

            // Adapt offsets in pNoRotatedAnchor so, that object will be moved to current position in
            // save and reload.
            const tools::Long nInverseShift = aOldSnapRect.Left() + aOldSnapRect.Right();
            const Point aLogicLT = pObj->GetLogicRect().TopLeft();
            const Point aMirroredLogicLT = aLogicLT + Point(nInverseShift, 0);
            const Point aOffsetDiff = aMirroredLogicLT - aOldLogicRect.TopLeft();
            // new Offsets
            pNoRotatedAnchor->maStartOffset += aOffsetDiff;
            pNoRotatedAnchor->maEndOffset += aOffsetDiff;
        }
        else // page anchored
        {
            Point aRef1( 0, 0 );
            Point aRef2( 0, 1 );
            if (bRecording)
                AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *pObj ) );
            pObj->Mirror( aRef1, aRef2 );
        }
    }
    else
    {
        //  Move instead of mirroring:
        //  New start position is negative of old end position
        //  -> move by sum of start and end position
        tools::Rectangle aObjRect = pObj->GetSnapRect();
        Size aMoveSize( -(aObjRect.Left() + aObjRect.Right()), 0 );
        if (bRecording)
            AddCalcUndo( std::make_unique<SdrUndoMoveObj>( *pObj, aMoveSize ) );
        pObj->Move( aMoveSize );
    }

    // for cell anchored objects adapt rectangles in anchors
    ScDrawObjData* pData = GetObjData(pObj);
    if (pData)
    {
        pData->setShapeRect(GetDocument(), pObj->GetSnapRect(), pObj->IsVisible());
        ScDrawObjData* pNoRotatedAnchor = GetNonRotatedObjData(pObj, true /*bCreate*/);
        pNoRotatedAnchor->setShapeRect(GetDocument(), pObj->GetLogicRect(), pObj->IsVisible());
    }
}

void ScDrawLayer::MoveRTL(SdrObject* pObj)
{
    tools::Rectangle aObjRect = pObj->GetSnapRect();
    Size aMoveSize( -(aObjRect.Left() + aObjRect.Right()), 0 );
    if (bRecording)
        AddCalcUndo( std::make_unique<SdrUndoMoveObj>( *pObj, aMoveSize ) );
    pObj->Move( aMoveSize );

    // for cell anchored objects adapt rectangles in anchors
    ScDrawObjData* pData = GetObjData(pObj);
    if (pData)
    {
        pData->setShapeRect(GetDocument(), pObj->GetSnapRect(), pObj->IsVisible());
        ScDrawObjData* pNoRotatedAnchor = GetNonRotatedObjData(pObj, true /*bCreate*/);
        pNoRotatedAnchor->setShapeRect(GetDocument(), pObj->GetLogicRect(), pObj->IsVisible());
    }
}

void ScDrawLayer::MirrorRectRTL( tools::Rectangle& rRect )
{
    //  mirror and swap left/right
    tools::Long nTemp = rRect.Left();
    rRect.SetLeft( -rRect.Right() );
    rRect.SetRight( -nTemp );
}

tools::Rectangle ScDrawLayer::GetCellRect( const ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell )
{
    tools::Rectangle aCellRect;
    OSL_ENSURE( rDoc.ValidColRowTab( rPos.Col(), rPos.Row(), rPos.Tab() ), "ScDrawLayer::GetCellRect - invalid cell address" );
    if( rDoc.ValidColRowTab( rPos.Col(), rPos.Row(), rPos.Tab() ) )
    {
        // find top left position of passed cell address
        Point aTopLeft;
        for( SCCOL nCol = 0; nCol < rPos.Col(); ++nCol )
            aTopLeft.AdjustX(rDoc.GetColWidth( nCol, rPos.Tab() ) );
        if( rPos.Row() > 0 )
            aTopLeft.AdjustY(rDoc.GetRowHeight( 0, rPos.Row() - 1, rPos.Tab() ) );

        // find bottom-right position of passed cell address
        ScAddress aEndPos = rPos;
        if( bMergedCell )
        {
            const ScMergeAttr* pMerge = rDoc.GetAttr( rPos, ATTR_MERGE );
            if( pMerge->GetColMerge() > 1 )
                aEndPos.IncCol( pMerge->GetColMerge() - 1 );
            if( pMerge->GetRowMerge() > 1 )
                aEndPos.IncRow( pMerge->GetRowMerge() - 1 );
        }
        Point aBotRight = aTopLeft;
        for( SCCOL nCol = rPos.Col(); nCol <= aEndPos.Col(); ++nCol )
            aBotRight.AdjustX(rDoc.GetColWidth( nCol, rPos.Tab() ) );
        aBotRight.AdjustY(rDoc.GetRowHeight( rPos.Row(), aEndPos.Row(), rPos.Tab() ) );

        // twips -> 1/100 mm
        aTopLeft.setX( static_cast< tools::Long >( aTopLeft.X() * HMM_PER_TWIPS ) );
        aTopLeft.setY( static_cast< tools::Long >( aTopLeft.Y() * HMM_PER_TWIPS ) );
        aBotRight.setX( static_cast< tools::Long >( aBotRight.X() * HMM_PER_TWIPS ) );
        aBotRight.setY( static_cast< tools::Long >( aBotRight.Y() * HMM_PER_TWIPS ) );

        aCellRect = tools::Rectangle( aTopLeft, aBotRight );
        if( rDoc.IsNegativePage( rPos.Tab() ) )
            MirrorRectRTL( aCellRect );
    }
    return aCellRect;
}

OUString ScDrawLayer::GetVisibleName( const SdrObject* pObj )
{
    OUString aName = pObj->GetName();
    if ( pObj->GetObjIdentifier() == OBJ_OLE2 )
    {
        //  For OLE, the user defined name (GetName) is used
        //  if it's not empty (accepting possibly duplicate names),
        //  otherwise the persist name is used so every object appears
        //  in the Navigator at all.

        if ( aName.isEmpty() )
            aName = static_cast<const SdrOle2Obj*>(pObj)->GetPersistName();
    }
    return aName;
}

static bool IsNamedObject( const SdrObject* pObj, const OUString& rName )
{
    //  sal_True if rName is the object's Name or PersistName
    //  (used to find a named object)

    return ( pObj->GetName() == rName ||
            ( pObj->GetObjIdentifier() == OBJ_OLE2 &&
              static_cast<const SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
}

SdrObject* ScDrawLayer::GetNamedObject( const OUString& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const
{
    sal_uInt16 nTabCount = GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
    {
        const SdrPage* pPage = GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( nId == 0 || pObject->GetObjIdentifier() == nId )
                    if ( IsNamedObject( pObject, rName ) )
                    {
                        rFoundTab = static_cast<SCTAB>(nTab);
                        return pObject;
                    }

                pObject = aIter.Next();
            }
        }
    }

    return nullptr;
}

OUString ScDrawLayer::GetNewGraphicName( tools::Long* pnCounter ) const
{
    OUString aBase = ScResId(STR_GRAPHICNAME) + " ";

    bool bThere = true;
    OUString aGraphicName;
    SCTAB nDummy;
    tools::Long nId = pnCounter ? *pnCounter : 0;
    while (bThere)
    {
        ++nId;
        aGraphicName = aBase + OUString::number( nId );
        bThere = ( GetNamedObject( aGraphicName, 0, nDummy ) != nullptr );
    }

    if ( pnCounter )
        *pnCounter = nId;

    return aGraphicName;
}

void ScDrawLayer::EnsureGraphicNames()
{
    //  make sure all graphic objects have names (after Excel import etc.)

    sal_uInt16 nTabCount = GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
    {
        SdrPage* pPage = GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
            SdrObject* pObject = aIter.Next();

            /* The index passed to GetNewGraphicName() will be set to
                the used index in each call. This prevents the repeated search
                for all names from 1 to current index. */
            tools::Long nCounter = 0;

            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_GRAF && pObject->GetName().isEmpty())
                    pObject->SetName( GetNewGraphicName( &nCounter ) );

                pObject = aIter.Next();
            }
        }
    }
}

namespace
{
    SdrObjUserData* GetFirstUserDataOfType(const SdrObject *pObj, sal_uInt16 nId)
    {
        sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObj->GetUserData( i );
            if( pData && pData->GetInventor() == SdrInventor::ScOrSwDraw && pData->GetId() == nId )
                return pData;
        }
        return nullptr;
    }

    void DeleteFirstUserDataOfType(SdrObject *pObj, sal_uInt16 nId)
    {
        sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
        for( sal_uInt16 i = nCount; i > 0; i-- )
        {
            SdrObjUserData* pData = pObj->GetUserData( i-1 );
            if( pData && pData->GetInventor() == SdrInventor::ScOrSwDraw && pData->GetId() == nId )
                pObj->DeleteUserData(i-1);
        }
    }
}

void ScDrawLayer::SetNonRotatedAnchor(SdrObject& rObj, const ScDrawObjData& rAnchor)
{
    ScDrawObjData* pAnchor = GetNonRotatedObjData( &rObj, true );
    pAnchor->maStart = rAnchor.maStart;
    pAnchor->maEnd = rAnchor.maEnd;
    pAnchor->maStartOffset = rAnchor.maStartOffset;
    pAnchor->maEndOffset = rAnchor.maEndOffset;
    pAnchor->mbResizeWithCell = rAnchor.mbResizeWithCell;
}

void ScDrawLayer::SetCellAnchored( SdrObject &rObj, const ScDrawObjData &rAnchor )
{
    ScDrawObjData* pAnchor = GetObjData( &rObj, true );
    pAnchor->maStart = rAnchor.maStart;
    pAnchor->maEnd = rAnchor.maEnd;
    pAnchor->maStartOffset = rAnchor.maStartOffset;
    pAnchor->maEndOffset = rAnchor.maEndOffset;
    pAnchor->mbResizeWithCell = rAnchor.mbResizeWithCell;
}

void ScDrawLayer::SetCellAnchoredFromPosition( SdrObject &rObj, const ScDocument &rDoc, SCTAB nTab,
                                               bool bResizeWithCell )
{
    if (!rObj.IsVisible())
        return;
    ScDrawObjData aAnchor;
    // set anchor in terms of the visual ( SnapRect )
    // object ( e.g. for when object is rotated )
    const tools::Rectangle aObjRect(rObj.GetSnapRect());
    GetCellAnchorFromPosition(
        aObjRect,
        aAnchor,
        rDoc,
        nTab);

    aAnchor.mbResizeWithCell = bResizeWithCell;
    SetCellAnchored( rObj, aAnchor );

    // absolutely necessary to set flag, ScDrawLayer::RecalcPos expects it.
    if ( ScDrawObjData* pAnchor = GetObjData( &rObj ) )
    {
        pAnchor->setShapeRect(&rDoc, rObj.GetSnapRect());
    }

    // - keep also an anchor in terms of the Logic ( untransformed ) object
    // because that's what we stored ( and still do ) to xml

    // Vertical flipped custom shapes need special treatment, see comment in
    // lcl_SetLogicRectFromAnchor
    tools::Rectangle aObjRect2;
    if (lcl_NeedsMirrorYCorrection(&rObj))
    {
        const tools::Rectangle aRect(rObj.GetSnapRect());
        const Point aLeft(aRect.Left(), (aRect.Top() + aRect.Bottom()) >> 1);
        const Point aRight(aLeft.X() + 1000, aLeft.Y());
        rObj.NbcMirror(aLeft, aRight);
        aObjRect2 = rObj.GetLogicRect();
        rObj.NbcMirror(aLeft, aRight);
    }
    else if (rObj.GetObjIdentifier() == OBJ_MEASURE)
    {
        // tdf#137576. A SdrMeasureObj might have a wrong logic rect here. TakeUnrotatedSnapRect
        // calculates the current unrotated snap rectangle, sets logic rectangle and returns it.
        static_cast<SdrMeasureObj*>(&rObj)->TakeUnrotatedSnapRect(aObjRect2);
    }
    else
        aObjRect2 = rObj.GetLogicRect();

    // Values in XML are so as if it is a LTR sheet. The object is shifted to negative page on loading
    // so that the snap rectangle appears mirrored. For transformed objects the shifted logic rectangle
    // is not the mirrored LTR rectangle. We calculate the mirrored LTR rectangle here.
    if (rDoc.IsNegativePage(nTab))
    {
        const tools::Rectangle aSnapRect(rObj.GetSnapRect());
        aObjRect2.Move(Size(-aSnapRect.Left() - aSnapRect.Right(), 0));
        MirrorRectRTL(aObjRect2);
    }

    ScDrawObjData aNoRotatedAnchor;
    GetCellAnchorFromPosition(
        aObjRect2,
        aNoRotatedAnchor,
        rDoc,
        nTab);

    aNoRotatedAnchor.mbResizeWithCell = bResizeWithCell;
    SetNonRotatedAnchor( rObj, aNoRotatedAnchor);
    // And update maShapeRect. It is used in adjustAnchoredPosition() in ScDrawView::Notify().
    if (ScDrawObjData* pAnchor = GetNonRotatedObjData(&rObj))
    {
        pAnchor->setShapeRect(&rDoc, rObj.GetLogicRect());
    }
}

void ScDrawLayer::GetCellAnchorFromPosition(
    const tools::Rectangle &rObjRect,
    ScDrawObjData &rAnchor,
    const ScDocument &rDoc,
    SCTAB nTab,
    bool bHiddenAsZero)
{
    ScRange aRange = rDoc.GetRange( nTab, rObjRect, bHiddenAsZero );

    tools::Rectangle aCellRect;

    rAnchor.maStart = aRange.aStart;
    aCellRect = rDoc.GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(),
      aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(), bHiddenAsZero );
    rAnchor.maStartOffset.setY( rObjRect.Top()-aCellRect.Top() );
    if (!rDoc.IsNegativePage(nTab))
        rAnchor.maStartOffset.setX( rObjRect.Left()-aCellRect.Left() );
    else
        rAnchor.maStartOffset.setX( aCellRect.Right()-rObjRect.Right() );

    rAnchor.maEnd = aRange.aEnd;
    aCellRect = rDoc.GetMMRect( aRange.aEnd.Col(), aRange.aEnd.Row(),
      aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(), bHiddenAsZero );
    if (!rObjRect.IsEmpty())
        rAnchor.maEndOffset.setY( rObjRect.Bottom()-aCellRect.Top() );
    if (!rDoc.IsNegativePage(nTab))
    {
        if (!rObjRect.IsEmpty())
            rAnchor.maEndOffset.setX( rObjRect.Right()-aCellRect.Left() );
    }
    else
        rAnchor.maEndOffset.setX( aCellRect.Right()-rObjRect.Left() );
}

void ScDrawLayer::UpdateCellAnchorFromPositionEnd( const SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect )
{
    tools::Rectangle aObjRect(bUseLogicRect ? rObj.GetLogicRect() : rObj.GetSnapRect());
    ScRange aRange = rDoc.GetRange( nTab, aObjRect );

    ScDrawObjData* pAnchor = &rAnchor;
    pAnchor->maEnd = aRange.aEnd;

    tools::Rectangle aCellRect = rDoc.GetMMRect( aRange.aEnd.Col(), aRange.aEnd.Row(),
      aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab() );
    pAnchor->maEndOffset.setY( aObjRect.Bottom()-aCellRect.Top() );
    if (!rDoc.IsNegativePage(nTab))
        pAnchor->maEndOffset.setX( aObjRect.Right()-aCellRect.Left() );
    else
        pAnchor->maEndOffset.setX( aCellRect.Right()-aObjRect.Left() );
}

bool ScDrawLayer::IsCellAnchored( const SdrObject& rObj )
{
    // Cell anchored object always has a user data, to store the anchor cell
    // info. If it doesn't then it's page-anchored.
    return GetFirstUserDataOfType(&rObj, SC_UD_OBJDATA) != nullptr;
}

bool ScDrawLayer::IsResizeWithCell( const SdrObject& rObj )
{
    // Cell anchored object always has a user data, to store the anchor cell
    // info. If it doesn't then it's page-anchored.
    ScDrawObjData* pDrawObjData = GetObjData(const_cast<SdrObject*>(&rObj));
    if (!pDrawObjData)
        return false;

    return pDrawObjData->mbResizeWithCell;
}

void ScDrawLayer::SetPageAnchored( SdrObject &rObj )
{
    DeleteFirstUserDataOfType(&rObj, SC_UD_OBJDATA);
    DeleteFirstUserDataOfType(&rObj, SC_UD_OBJDATA);
}

ScAnchorType ScDrawLayer::GetAnchorType( const SdrObject &rObj )
{
    //If this object has a cell anchor associated with it
    //then it's cell-anchored, otherwise it's page-anchored
    const ScDrawObjData* pObjData = ScDrawLayer::GetObjData(const_cast<SdrObject*>(&rObj));

    // When there is no cell anchor, it is page anchored.
    if (!pObjData)
        return SCA_PAGE;

    // It's cell-anchored, check if the object resizes with the cell
    if (pObjData->mbResizeWithCell)
        return SCA_CELL_RESIZE;

    return SCA_CELL;
}

std::vector<SdrObject*>
ScDrawLayer::GetObjectsAnchoredToRows(SCTAB nTab, SCROW nStartRow, SCROW nEndRow)
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    if (!pPage || pPage->GetObjCount() < 1)
        return std::vector<SdrObject*>();

    std::vector<SdrObject*> aObjects;
    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    ScRange aRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab);
    while (pObject)
    {
        if (!dynamic_cast<SdrCaptionObj*>(pObject)) // Caption objects are handled differently
        {
            ScDrawObjData* pObjData = GetObjData(pObject);
            if (pObjData && aRange.In(pObjData->maStart))
                aObjects.push_back(pObject);
        }
        pObject = aIter.Next();
    }
    return aObjects;
}

std::map<SCROW, std::vector<SdrObject*>>
ScDrawLayer::GetObjectsAnchoredToRange(SCTAB nTab, SCCOL nCol, SCROW nStartRow, SCROW nEndRow)
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    if (!pPage || pPage->GetObjCount() < 1)
        return std::map<SCROW, std::vector<SdrObject*>>();

    std::map<SCROW, std::vector<SdrObject*>> aRowObjects;
    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    ScRange aRange( nCol, nStartRow, nTab, nCol, nEndRow, nTab);
    while (pObject)
    {
        if (!dynamic_cast<SdrCaptionObj*>(pObject)) // Caption objects are handled differently
        {
            ScDrawObjData* pObjData = GetObjData(pObject);
            if (pObjData && aRange.In(pObjData->maStart))
                aRowObjects[pObjData->maStart.Row()].push_back(pObject);
        }
        pObject = aIter.Next();
    }
    return aRowObjects;
}

bool ScDrawLayer::HasObjectsAnchoredInRange(const ScRange& rRange)
{
    // This only works for one table at a time
    assert(rRange.aStart.Tab() == rRange.aEnd.Tab());

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(rRange.aStart.Tab()));
    if (!pPage || pPage->GetObjCount() < 1)
        return false;

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (!dynamic_cast<SdrCaptionObj*>(pObject)) // Caption objects are handled differently
        {
            ScDrawObjData* pObjData = GetObjData(pObject);
            if (pObjData && rRange.In(pObjData->maStart)) // Object is in given range
                return true;
        }
        pObject = aIter.Next();
    }
    return false;
}

std::vector<SdrObject*> ScDrawLayer::GetObjectsAnchoredToCols(SCTAB nTab, SCCOL nStartCol,
                                                              SCCOL nEndCol)
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    if (!pPage || pPage->GetObjCount() < 1)
        return std::vector<SdrObject*>();

    std::vector<SdrObject*> aObjects;
    SdrObjListIter aIter(pPage, SdrIterMode::Flat);
    SdrObject* pObject = aIter.Next();
    ScRange aRange(nStartCol, 0, nTab, nEndCol, MAXROW, nTab);
    while (pObject)
    {
        if (!dynamic_cast<SdrCaptionObj*>(pObject)) // Caption objects are handled differently
        {
            ScDrawObjData* pObjData = GetObjData(pObject);
            if (pObjData && aRange.In(pObjData->maStart))
                aObjects.push_back(pObject);
        }
        pObject = aIter.Next();
    }
    return aObjects;
}

void ScDrawLayer::MoveObject(SdrObject* pObject, const ScAddress& rNewPosition)
{
    // Get anchor data
    ScDrawObjData* pObjData = GetObjData(pObject, false);
    if (!pObjData)
        return;
    const ScAddress aOldStart = pObjData->maStart;
    const ScAddress aOldEnd = pObjData->maEnd;

    // Set start address
    pObjData->maStart = rNewPosition;

    // Set end address
    const SCCOL nObjectColSpan = aOldEnd.Col() - aOldStart.Col();
    const SCROW nObjectRowSpan = aOldEnd.Row() - aOldStart.Row();
    ScAddress aNewEnd = rNewPosition;
    aNewEnd.IncRow(nObjectRowSpan);
    aNewEnd.IncCol(nObjectColSpan);
    pObjData->maEnd = aNewEnd;

    // Update draw object according to new anchor
    RecalcPos(pObject, *pObjData, false, false);
}

ScDrawObjData* ScDrawLayer::GetNonRotatedObjData( SdrObject* pObj, bool bCreate )
{
    sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
    sal_uInt16 nFound = 0;
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SdrInventor::ScOrSwDraw && pData->GetId() == SC_UD_OBJDATA && ++nFound == 2 )
            return static_cast<ScDrawObjData*>(pData);
    }
    if( pObj && bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(pData));
        return pData;
    }
    return nullptr;
}

ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, bool bCreate )
{
    if (SdrObjUserData *pData = GetFirstUserDataOfType(pObj, SC_UD_OBJDATA))
        return static_cast<ScDrawObjData*>(pData);

    if( pObj && bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(pData));
        return pData;
    }
    return nullptr;
}

ScDrawObjData* ScDrawLayer::GetObjDataTab( SdrObject* pObj, SCTAB nTab )
{
    ScDrawObjData* pData = GetObjData( pObj );
    if ( pData )
    {
        if ( pData->maStart.IsValid() )
            pData->maStart.SetTab( nTab );
        if ( pData->maEnd.IsValid() )
            pData->maEnd.SetTab( nTab );
    }
    return pData;
}

bool ScDrawLayer::IsNoteCaption( SdrObject* pObj )
{
    ScDrawObjData* pData = pObj ? GetObjData( pObj ) : nullptr;
    return pData && pData->meType == ScDrawObjData::CellNote;
}

ScDrawObjData* ScDrawLayer::GetNoteCaptionData( SdrObject* pObj, SCTAB nTab )
{
    ScDrawObjData* pData = pObj ? GetObjDataTab( pObj, nTab ) : nullptr;
    return (pData && pData->meType == ScDrawObjData::CellNote) ? pData : nullptr;
}

ScMacroInfo* ScDrawLayer::GetMacroInfo( SdrObject* pObj, bool bCreate )
{
    if (SdrObjUserData *pData = GetFirstUserDataOfType(pObj, SC_UD_MACRODATA))
        return static_cast<ScMacroInfo*>(pData);

    if ( bCreate )
    {
        ScMacroInfo* pData = new ScMacroInfo;
        pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(pData));
        return pData;
    }
    return nullptr;
}

void ScDrawLayer::SetGlobalDrawPersist(SfxObjectShell* pPersist)
{
    OSL_ENSURE(!pGlobalDrawPersist,"Multiple SetGlobalDrawPersist");
    pGlobalDrawPersist = pPersist;
}

void ScDrawLayer::SetChanged( bool bFlg /* = true */ )
{
    if ( bFlg && pDoc )
        pDoc->SetChartListenerCollectionNeedsUpdate( true );
    FmFormModel::SetChanged( bFlg );
}

css::uno::Reference< css::uno::XInterface > ScDrawLayer::createUnoModel()
{
    css::uno::Reference< css::uno::XInterface > xRet;
    if( pDoc && pDoc->GetDocumentShell() )
        xRet = pDoc->GetDocumentShell()->GetModel();

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
