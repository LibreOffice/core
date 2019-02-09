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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/autokernitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <osl/thread.h>
#include <svl/asiancfg.hxx>
#include <svx/svditer.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xtable.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>

#include <document.hxx>
#include <docoptio.hxx>
#include <table.hxx>
#include <drwlayer.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <rechead.hxx>
#include <poolhelp.hxx>
#include <docpool.hxx>
#include <editutil.hxx>
#include <charthelper.hxx>
#include <conditio.hxx>
#include <documentlinkmgr.hxx>

using namespace ::com::sun::star;

SfxBroadcaster* ScDocument::GetDrawBroadcaster()
{
    return mpDrawLayer.get();
}

void ScDocument::BeginDrawUndo()
{
    if (mpDrawLayer)
        mpDrawLayer->BeginCalcUndo(false);
}

void ScDocument::TransferDrawPage(const ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos)
{
    if (mpDrawLayer && pSrcDoc->mpDrawLayer)
    {
        SdrPage* pOldPage = pSrcDoc->mpDrawLayer->GetPage(static_cast<sal_uInt16>(nSrcPos));
        SdrPage* pNewPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nDestPos));

        if (pOldPage && pNewPage)
        {
            SdrObjListIter aIter( pOldPage, SdrIterMode::Flat );
            SdrObject* pOldObject = aIter.Next();
            while (pOldObject)
            {
                // Clone to target SdrModel
                SdrObject* pNewObject(pOldObject->CloneSdrObject(*mpDrawLayer));
                pNewObject->NbcMove(Size(0,0));
                pNewPage->InsertObject( pNewObject );

                if (mpDrawLayer->IsRecording())
                    mpDrawLayer->AddCalcUndo( std::make_unique<SdrUndoInsertObj>( *pNewObject ) );

                pOldObject = aIter.Next();
            }
        }
    }

    //  make sure the data references of charts are adapted
    //  (this must be after InsertObject!)
    ScChartHelper::AdjustRangesOfChartsOnDestinationPage( pSrcDoc, this, nSrcPos, nDestPos );
    ScChartHelper::UpdateChartsOnDestinationPage(this, nDestPos);
}

void ScDocument::InitDrawLayer( SfxObjectShell* pDocShell )
{
    if (pDocShell && !mpShell)
    {
        ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
        mpShell = pDocShell;
    }

    if (!mpDrawLayer)
    {
        ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
        OUString aName;
        if ( mpShell && !mpShell->IsLoading() )       // don't call GetTitle while loading
            aName = mpShell->GetTitle();
        mpDrawLayer.reset(new ScDrawLayer( this, aName ));

        sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(bAutoCalc);
        if (pMgr)
            mpDrawLayer->SetLinkManager(pMgr);

        // set DrawingLayer's SfxItemPool at Calc's SfxItemPool as
        // secondary pool to support DrawingLayer FillStyle ranges (and similar)
        // in SfxItemSets using the Calc SfxItemPool. This is e.g. needed when
        // the PageStyle using SvxBrushItem is visualized and will be potentially
        // used more intense in the future
        if (mxPoolHelper.is() && !IsClipOrUndo()) //Using IsClipOrUndo as a proxy for SharePooledResources called
        {
            ScDocumentPool* pLocalPool = mxPoolHelper->GetDocPool();

            if (pLocalPool)
            {
                OSL_ENSURE(!pLocalPool->GetSecondaryPool(), "OOps, already a secondary pool set where the DrawingLayer ItemPool is to be placed (!)");
                pLocalPool->SetSecondaryPool(&mpDrawLayer->GetItemPool());
            }
        }

        //  Drawing pages are accessed by table number, so they must also be present
        //  for preceding table numbers, even if the tables aren't allocated
        //  (important for clipboard documents).

        SCTAB nDrawPages = 0;
        SCTAB nTab;
        for (nTab=0; nTab < static_cast<SCTAB>(maTabs.size()); nTab++)
            if (maTabs[nTab])
                nDrawPages = nTab + 1;          // needed number of pages

        for (nTab=0; nTab<nDrawPages && nTab < static_cast<SCTAB>(maTabs.size()); nTab++)
        {
            mpDrawLayer->ScAddPage( nTab );     // always add page, with or without the table
            if (maTabs[nTab])
            {
                OUString aTabName = maTabs[nTab]->GetName();
                mpDrawLayer->ScRenamePage( nTab, aTabName );

                maTabs[nTab]->SetDrawPageSize(false,false);     // set the right size immediately
            }
        }

        mpDrawLayer->SetDefaultTabulator( GetDocOptions().GetTabDistance() );

        UpdateDrawPrinter();

        // set draw defaults directly
        SfxItemPool& rDrawPool = mpDrawLayer->GetItemPool();
        rDrawPool.SetPoolDefaultItem( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

        UpdateDrawLanguages();
        if (bImportingXML)
            mpDrawLayer->EnableAdjust(false);

        mpDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
        mpDrawLayer->SetCharCompressType( GetAsianCompression() );
        mpDrawLayer->SetKernAsianPunctuation( GetAsianKerning() );
    }
}

void ScDocument::UpdateDrawLanguages()
{
    if (mpDrawLayer)
    {
        SfxItemPool& rDrawPool = mpDrawLayer->GetItemPool();
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eLanguage, EE_CHAR_LANGUAGE ) );
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, EE_CHAR_LANGUAGE_CJK ) );
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, EE_CHAR_LANGUAGE_CTL ) );
    }
}

void ScDocument::UpdateDrawPrinter()
{
    if (mpDrawLayer)
    {
        // use the printer even if IsValid is false
        // Application::GetDefaultDevice causes trouble with changing MapModes
        mpDrawLayer->SetRefDevice(GetRefDevice());
    }
}

void ScDocument::SetDrawPageSize(SCTAB nTab)
{
    if (!ValidTab(nTab) || nTab >= static_cast<SCTAB>(maTabs.size()) || !maTabs[nTab])
        return;

    maTabs[nTab]->SetDrawPageSize();
}

bool ScDocument::IsChart( const SdrObject* pObject )
{
    // IsChart() implementation moved to svx drawinglayer
    if(pObject && OBJ_OLE2 == pObject->GetObjIdentifier())
    {
        return static_cast<const SdrOle2Obj*>(pObject)->IsChart();
    }

    return false;
}

IMPL_LINK( ScDocument, GetUserDefinedColor, sal_uInt16, nColorIndex, Color* )
{
    rtl::Reference<XColorList> xColorList;
    if (mpDrawLayer)
        xColorList = mpDrawLayer->GetColorList();
    else
    {
        ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);
        if (!pColorList.is())
            pColorList = XColorList::CreateStdColorList();
        xColorList = pColorList;
    }
    return const_cast<Color*>(&(xColorList->GetColor(nColorIndex)->GetColor()));
}

void ScDocument::DeleteDrawLayer()
{
    ScMutationGuard aGuard(this, ScMutationGuardFlags::CORE);

    // remove DrawingLayer's SfxItemPool from Calc's SfxItemPool where
    // it is registered as secondary pool
    if (mxPoolHelper.is() && !IsClipOrUndo()) //Using IsClipOrUndo as a proxy for SharePooledResources called
    {
        ScDocumentPool* pLocalPool = mxPoolHelper->GetDocPool();

        if(pLocalPool && pLocalPool->GetSecondaryPool())
        {
            pLocalPool->SetSecondaryPool(nullptr);
        }
    }
    mpDrawLayer.reset();
}

bool ScDocument::DrawGetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const
{
    return mpDrawLayer->GetPrintArea( rRange, bSetHor, bSetVer );
}

void ScDocument::DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        const ScMarkData& rMark )
{
    if (!mpDrawLayer)
        return;

    SCTAB nTabCount = GetTableCount();
    for (const auto& rTab : rMark)
    {
        if (rTab >= nTabCount)
            break;
        if (maTabs[rTab])
            mpDrawLayer->DeleteObjectsInArea( rTab, nCol1, nRow1, nCol2, nRow2 );
    }
}

void ScDocument::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    if (!mpDrawLayer)
        return;

    mpDrawLayer->DeleteObjectsInSelection( rMark );
}

bool ScDocument::HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark )
{
    //  pTabMark is used only for selected tables. If pTabMark is 0, all tables of rRange are used.

    if (!mpDrawLayer)
        return false;

    SCTAB nStartTab = 0;
    SCTAB nEndTab = static_cast<SCTAB>(maTabs.size());
    if ( !pTabMark )
    {
        nStartTab = rRange.aStart.Tab();
        nEndTab = rRange.aEnd.Tab();
    }

    for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++)
    {
        if ( !pTabMark || pTabMark->GetTableSelect(nTab) )
        {
            tools::Rectangle aMMRect = GetMMRect( rRange.aStart.Col(), rRange.aStart.Row(),
                                            rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );

            SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( pPage, SdrIterMode::Flat );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                            aMMRect.IsInside( pObject->GetCurrentBoundRect() ) )
                        return true;

                    pObject = aIter.Next();
                }
            }
        }
    }

    return false;
}

void ScDocument::StartAnimations( SCTAB nTab )
{
    if (!mpDrawLayer)
        return;
    SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return;

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (SdrGrafObj* pGrafObj = dynamic_cast<SdrGrafObj*>(pObject))
        {
            if ( pGrafObj->IsAnimated() )
            {
                pGrafObj->StartAnimation();
            }
        }
        pObject = aIter.Next();
    }
}

bool ScDocument::HasBackgroundDraw( SCTAB nTab, const tools::Rectangle& rMMRect ) const
{
    //  Are there objects in the background layer who are (partly) affected by rMMRect
    //  (for Drawing optimization, no deletion in front of the background
    if (!mpDrawLayer)
        return false;
    SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return false;

    bool bFound = false;

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_BACK && pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = true;
        pObject = aIter.Next();
    }

    return bFound;
}

bool ScDocument::HasAnyDraw( SCTAB nTab, const tools::Rectangle& rMMRect ) const
{
    //  Are there any objects at all who are (partly) affected by rMMRect?
    //  (To detect blank pages when printing)
    if (!mpDrawLayer)
        return false;
    SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return false;

    bool bFound = false;

    SdrObjListIter aIter( pPage, SdrIterMode::Flat );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = true;
        pObject = aIter.Next();
    }

    return bFound;
}

void ScDocument::EnsureGraphicNames()
{
    if (mpDrawLayer)
        mpDrawLayer->EnsureGraphicNames();
}

SdrObject* ScDocument::GetObjectAtPoint( SCTAB nTab, const Point& rPos )
{
    //  for Drag&Drop on draw object
    SdrObject* pFound = nullptr;
    if (mpDrawLayer && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::Flat );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetCurrentBoundRect().IsInside(rPos) )
                {
                    // Intern is of no interest
                    // Only object form background layer, when no object form another layer is found
                    SdrLayerID nLayer = pObject->GetLayer();
                    if ( (nLayer != SC_LAYER_INTERN) && (nLayer != SC_LAYER_HIDDEN) )
                    {
                        if ( nLayer != SC_LAYER_BACK ||
                                !pFound || pFound->GetLayer() == SC_LAYER_BACK )
                        {
                            pFound = pObject;
                        }
                    }
                }
                //  Continue search -> take last (on top) found object
                pObject = aIter.Next();
            }
        }
    }
    return pFound;
}

bool ScDocument::IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, bool bLeftIsEmpty,
                                ScRange* pLastRange, tools::Rectangle* pLastMM ) const
{
    if (!IsBlockEmpty( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
        return false;

    if (HasAttrib(ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab), HasAttrFlags::Lines))
        // We want to print sheets with borders even if there is no cell content.
        return false;

    tools::Rectangle aMMRect;
    if ( pLastRange && pLastMM && nTab == pLastRange->aStart.Tab() &&
            nStartRow == pLastRange->aStart.Row() && nEndRow == pLastRange->aEnd.Row() )
    {
        //  keep vertical part of aMMRect, only update horizontal position
        aMMRect = *pLastMM;

        long nLeft = 0;
        SCCOL i;
        for (i=0; i<nStartCol; i++)
            nLeft += GetColWidth(i,nTab);
        long nRight = nLeft;
        for (i=nStartCol; i<=nEndCol; i++)
            nRight += GetColWidth(i,nTab);

        aMMRect.SetLeft( static_cast<long>(nLeft  * HMM_PER_TWIPS) );
        aMMRect.SetRight( static_cast<long>(nRight * HMM_PER_TWIPS) );
    }
    else
        aMMRect = GetMMRect( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

    if ( pLastRange && pLastMM )
    {
        *pLastRange = ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        *pLastMM = aMMRect;
    }

    if ( HasAnyDraw( nTab, aMMRect ))
        return false;

    if ( nStartCol > 0 && !bLeftIsEmpty )
    {
        // similar to in ScPrintFunc::AdjustPrintArea
        // ExtendPrintArea starting only from the start column of the print area

        SCCOL nExtendCol = nStartCol - 1;
        SCROW nTmpRow = nEndRow;

        // ExtendMerge() is non-const, but called without refresh. GetPrinter()
        // might create and assign a printer.
        ScDocument* pThis = const_cast<ScDocument*>(this);

        pThis->ExtendMerge( 0,nStartRow, nExtendCol,nTmpRow, nTab );      // no Refresh, incl. Attrs

        OutputDevice* pDev = pThis->GetPrinter();
        pDev->SetMapMode(MapMode(MapUnit::MapPixel)); // Important for GetNeededSize
        ExtendPrintArea( pDev, nTab, 0, nStartRow, nExtendCol, nEndRow );
        if ( nExtendCol >= nStartCol )
            return false;
    }

    return true;
}

void ScDocument::Clear( bool bFromDestructor )
{
    for (auto& rxTab : maTabs)
        if (rxTab)
            rxTab->GetCondFormList()->clear();

    maTabs.clear();
    pSelectionAttr.reset();

    if (mpDrawLayer)
    {
        mpDrawLayer->ClearModel( bFromDestructor );
    }
}

bool ScDocument::HasDetectiveObjects(SCTAB nTab) const
{
    //  looks for detective objects, annotations don't count
    //  (used to adjust scale so detective objects hit their cells better)

    bool bFound = false;

    if (mpDrawLayer)
    {
        SdrPage* pPage = mpDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( pPage, SdrIterMode::DeepNoGroups );
            SdrObject* pObject = aIter.Next();
            while (pObject && !bFound)
            {
                // anything on the internal layer except captions (annotations)
                if ( (pObject->GetLayer() == SC_LAYER_INTERN) && !ScDrawLayer::IsNoteCaption( pObject ) )
                    bFound = true;

                pObject = aIter.Next();
            }
        }
    }

    return bFound;
}

void ScDocument::UpdateFontCharSet()
{
    // In old versions (until 4.0 without SP), when switching between systems,
    // the Font attribute was not adjusted.
    // This has to be redone for Documents until SP2:
    // Everything that is not SYMBOL is set to system CharSet.
    // CharSet should be correct for new documents (version SC_FONTCHARSET)

    bool bUpdateOld = ( nSrcVer < SC_FONTCHARSET );

    rtl_TextEncoding eSysSet = osl_getThreadTextEncoding();
    if ( eSrcSet != eSysSet || bUpdateOld )
    {
        ScDocumentPool* pPool = mxPoolHelper->GetDocPool();
        sal_uInt32 nCount = pPool->GetItemCount2(ATTR_FONT);
        for (sal_uInt32 i=0; i<nCount; i++)
        {
            SvxFontItem* pItem = const_cast<SvxFontItem*>(pPool->GetItem2(ATTR_FONT, i));
            if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                            ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                pItem->SetCharSet(eSysSet);
        }

        if ( mpDrawLayer )
        {
            SfxItemPool& rDrawPool = mpDrawLayer->GetItemPool();
            nCount = rDrawPool.GetItemCount2(EE_CHAR_FONTINFO);
            for (sal_uInt32 i=0; i<nCount; i++)
            {
                SvxFontItem* pItem = const_cast<SvxFontItem*>(rDrawPool.GetItem2(EE_CHAR_FONTINFO, i));
                if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                                ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                    pItem->SetCharSet( eSysSet );
            }
        }
    }
}

void ScDocument::SetLoadingMedium( bool bVal )
{
    bLoadingMedium = bVal;
    for (auto& rxTab : maTabs)
    {
        if (!rxTab)
            return;

        rxTab->SetLoadingMedium(bVal);
    }
}

void ScDocument::SetImportingXML( bool bVal )
{
    bImportingXML = bVal;
    if (mpDrawLayer)
        mpDrawLayer->EnableAdjust(!bImportingXML);

    if ( !bVal )
    {
        // #i57869# after loading, do the real RTL mirroring for the sheets that have the LoadingRTL flag set

        for ( SCTAB nTab=0; nTab< static_cast<SCTAB>(maTabs.size()) && maTabs[nTab]; nTab++ )
            if ( maTabs[nTab]->IsLoadingRTL() )
            {
                maTabs[nTab]->SetLoadingRTL( false );
                SetLayoutRTL( nTab, true );             // includes mirroring; bImportingXML must be cleared first
            }
    }

    SetLoadingMedium(bVal);
}

const std::shared_ptr<SvxForbiddenCharactersTable>& ScDocument::GetForbiddenCharacters()
{
    return xForbiddenCharacters;
}

void ScDocument::SetForbiddenCharacters(const std::shared_ptr<SvxForbiddenCharactersTable>& rNew)
{
    xForbiddenCharacters = rNew;
    if ( mpEditEngine )
        EditEngine::SetForbiddenCharsTable( xForbiddenCharacters );
    if ( mpDrawLayer )
        mpDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
}

bool ScDocument::IsValidAsianCompression() const
{
    return nAsianCompression != CharCompressType::Invalid;
}

CharCompressType ScDocument::GetAsianCompression() const
{
    if ( nAsianCompression == CharCompressType::Invalid )
        return CharCompressType::NONE;
    else
        return nAsianCompression;
}

void ScDocument::SetAsianCompression(CharCompressType nNew)
{
    nAsianCompression = nNew;
    if ( mpEditEngine )
        mpEditEngine->SetAsianCompressionMode( nAsianCompression );
    if ( mpDrawLayer )
        mpDrawLayer->SetCharCompressType( nAsianCompression );
}

bool ScDocument::IsValidAsianKerning() const
{
    return ( nAsianKerning != SC_ASIANKERNING_INVALID );
}

bool ScDocument::GetAsianKerning() const
{
    if ( nAsianKerning == SC_ASIANKERNING_INVALID )
        return false;
    else
        return static_cast<bool>(nAsianKerning);
}

void ScDocument::SetAsianKerning(bool bNew)
{
    nAsianKerning = static_cast<sal_uInt8>(bNew);
    if ( mpEditEngine )
        mpEditEngine->SetKernAsianPunctuation( static_cast<bool>( nAsianKerning ) );
    if ( mpDrawLayer )
        mpDrawLayer->SetKernAsianPunctuation( static_cast<bool>( nAsianKerning ) );
}

void ScDocument::ApplyAsianEditSettings( ScEditEngineDefaulter& rEngine )
{
    EditEngine::SetForbiddenCharsTable( xForbiddenCharacters );
    rEngine.SetAsianCompressionMode( GetAsianCompression() );
    rEngine.SetKernAsianPunctuation( GetAsianKerning() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
