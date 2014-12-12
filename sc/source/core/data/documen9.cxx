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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sot/exchange.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/langitem.hxx>
#include <osl/thread.h>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xtable.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/pathoptions.hxx>

#include "document.hxx"
#include "docoptio.hxx"
#include "table.hxx"
#include "drwlayer.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "detfunc.hxx"
#include "editutil.hxx"
#include "postit.hxx"
#include "charthelper.hxx"
#include "interpre.hxx"
#include <documentlinkmgr.hxx>

using namespace ::com::sun::star;
#include <stdio.h>

SfxBroadcaster* ScDocument::GetDrawBroadcaster()
{
    return pDrawLayer;
}

void ScDocument::BeginDrawUndo()
{
    if (pDrawLayer)
        pDrawLayer->BeginCalcUndo(false);
}

rtl::Reference<XColorList> ScDocument::GetColorList()
{
    if (pDrawLayer)
        return pDrawLayer->GetColorList();
    else
    {
        if (!pColorList.is())
            pColorList = XColorList::CreateStdColorList();
        return pColorList;
    }
}

void ScDocument::TransferDrawPage(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos)
{
    if (pDrawLayer && pSrcDoc->pDrawLayer)
    {
        SdrPage* pOldPage = pSrcDoc->pDrawLayer->GetPage(static_cast<sal_uInt16>(nSrcPos));
        SdrPage* pNewPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nDestPos));

        if (pOldPage && pNewPage)
        {
            SdrObjListIter aIter( *pOldPage, IM_FLAT );
            SdrObject* pOldObject = aIter.Next();
            while (pOldObject)
            {
                // #116235#
                SdrObject* pNewObject = pOldObject->Clone();
                // SdrObject* pNewObject = pOldObject->Clone( pNewPage, pDrawLayer );
                pNewObject->SetModel(pDrawLayer);
                pNewObject->SetPage(pNewPage);

                pNewObject->NbcMove(Size(0,0));
                pNewPage->InsertObject( pNewObject );

                if (pDrawLayer->IsRecording())
                    pDrawLayer->AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

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
    if (pDocShell && !pShell)
        pShell = pDocShell;

    if (!pDrawLayer)
    {
        OUString aName;
        if ( pShell && !pShell->IsLoading() )       // don't call GetTitle while loading
            aName = pShell->GetTitle();
        pDrawLayer = new ScDrawLayer( this, aName );

        sfx2::LinkManager* pMgr = GetDocLinkManager().getLinkManager(bAutoCalc);
        if (pMgr)
            pDrawLayer->SetLinkManager(pMgr);

        //UUUU set DrawingLayer's SfxItemPool at Calc's SfxItemPool as
        // secondary pool to support DrawingLayer FillStyle ranges (and similar)
        // in SfxItemSets using the Calc SfxItemPool. This is e.g. needed when
        // the PageStyle using SvxBrushItem is visualized and will be potentially
        // used more intense in the future
        if(xPoolHelper.is())
        {
            ScDocumentPool* pLocalPool = xPoolHelper->GetDocPool();

            if(pLocalPool)
            {
                OSL_ENSURE(!pLocalPool->GetSecondaryPool(), "OOps, already a secondary pool set where the DrawingLayer ItemPool is to be placed (!)");
                pLocalPool->SetSecondaryPool(&pDrawLayer->GetItemPool());
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
            pDrawLayer->ScAddPage( nTab );      // always add page, with or without the table
            if (maTabs[nTab])
            {
                OUString aTabName;
                maTabs[nTab]->GetName(aTabName);
                pDrawLayer->ScRenamePage( nTab, aTabName );

                maTabs[nTab]->SetDrawPageSize(false,false);     // set the right size immediately
            }
        }

        pDrawLayer->SetDefaultTabulator( GetDocOptions().GetTabDistance() );

        UpdateDrawPrinter();

        // set draw defaults directly
        SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
        rDrawPool.SetPoolDefaultItem( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

        UpdateDrawLanguages();
        if (bImportingXML)
            pDrawLayer->EnableAdjust(false);

        pDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
        pDrawLayer->SetCharCompressType( GetAsianCompression() );
        pDrawLayer->SetKernAsianPunctuation( GetAsianKerning() );
    }
}

void ScDocument::UpdateDrawLanguages()
{
    if (pDrawLayer)
    {
        SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eLanguage, EE_CHAR_LANGUAGE ) );
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCjkLanguage, EE_CHAR_LANGUAGE_CJK ) );
        rDrawPool.SetPoolDefaultItem( SvxLanguageItem( eCtlLanguage, EE_CHAR_LANGUAGE_CTL ) );
    }
}

void ScDocument::UpdateDrawPrinter()
{
    if (pDrawLayer)
    {
        // use the printer even if IsValid is false
        // Application::GetDefaultDevice causes trouble with changing MapModes
        pDrawLayer->SetRefDevice(GetRefDevice());
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
    // #109985#
    // IsChart() implementation moved to svx drawinglayer
    if(pObject && OBJ_OLE2 == pObject->GetObjIdentifier())
    {
        return static_cast<const SdrOle2Obj*>(pObject)->IsChart();
    }

    return false;
}

IMPL_LINK_INLINE_START( ScDocument, GetUserDefinedColor, sal_uInt16 *, pColorIndex )
{
    return reinterpret_cast<sal_IntPtr>( &((GetColorList()->GetColor(*pColorIndex))->GetColor()) );
}
IMPL_LINK_INLINE_END( ScDocument, GetUserDefinedColor, sal_uInt16 *, pColorIndex )

void ScDocument::DeleteDrawLayer()
{
    //UUUU remove DrawingLayer's SfxItemPool from Calc's SfxItemPool where
    // it is registered as secondary pool
    if (xPoolHelper.is() && !IsClipOrUndo()) //Using IsClipOrUndo as a proxy for SharePooledResources called
    {
        ScDocumentPool* pLocalPool = xPoolHelper->GetDocPool();

        if(pLocalPool && pLocalPool->GetSecondaryPool())
        {
            pLocalPool->SetSecondaryPool(0);
        }
    }
    delete pDrawLayer;
    pDrawLayer = 0;
}

bool ScDocument::DrawGetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const
{
    return pDrawLayer->GetPrintArea( rRange, bSetHor, bSetVer );
}

void ScDocument::DrawMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos )
{
    pDrawLayer->ScMovePage(nOldPos,nNewPos);
}

void ScDocument::DrawCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos )
{
    // angelegt wird die Page schon im ScTable ctor
    pDrawLayer->ScCopyPage( nOldPos, nNewPos );
}

void ScDocument::DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    SCTAB nTabCount = GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
        if (maTabs[*itr])
            pDrawLayer->DeleteObjectsInArea( *itr, nCol1, nRow1, nCol2, nRow2 );
}

void ScDocument::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    pDrawLayer->DeleteObjectsInSelection( rMark );
}

bool ScDocument::HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark )
{
    //  pTabMark is used only for selected tables. If pTabMark is 0, all tables of rRange are used.

    if (!pDrawLayer)
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
            Rectangle aMMRect = GetMMRect( rRange.aStart.Col(), rRange.aStart.Row(),
                                            rRange.aEnd.Col(), rRange.aEnd.Row(), nTab );

            SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
            OSL_ENSURE(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_FLAT );
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

void ScDocument::StartAnimations( SCTAB nTab, vcl::Window* pWin )
{
    if (!pDrawLayer)
        return;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (pObject->ISA(SdrGrafObj))
        {
            SdrGrafObj* pGrafObj = static_cast<SdrGrafObj*>(pObject);
            if ( pGrafObj->IsAnimated() )
            {
                const Rectangle& rRect = pGrafObj->GetCurrentBoundRect();
                pGrafObj->StartAnimation( pWin, rRect.TopLeft(), rRect.GetSize() );
            }
        }
        pObject = aIter.Next();
    }
}

bool ScDocument::HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect ) const
{
    //  Gibt es Objekte auf dem Hintergrund-Layer, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (fuer Drawing-Optimierung, vor dem Hintergrund braucht dann nicht geloescht
    //   zu werden)

    if (!pDrawLayer)
        return false;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return false;

    bool bFound = false;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_BACK && pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = true;
        pObject = aIter.Next();
    }

    return bFound;
}

bool ScDocument::HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect ) const
{
    //  Gibt es ueberhaupt Objekte, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (um leere Seiten beim Drucken zu erkennen)

    if (!pDrawLayer)
        return false;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return false;

    bool bFound = false;

    SdrObjListIter aIter( *pPage, IM_FLAT );
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
    if (pDrawLayer)
        pDrawLayer->EnsureGraphicNames();
}

SdrObject* ScDocument::GetObjectAtPoint( SCTAB nTab, const Point& rPos )
{
    //  fuer Drag&Drop auf Zeichenobjekt

    SdrObject* pFound = NULL;
    if (pDrawLayer && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetCurrentBoundRect().IsInside(rPos) )
                {
                    //  Intern interessiert gar nicht
                    //  Objekt vom Back-Layer nur, wenn kein Objekt von anderem Layer getroffen

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
                //  weitersuchen -> letztes (oberstes) getroffenes Objekt nehmen

                pObject = aIter.Next();
            }
        }
    }
    return pFound;
}

bool ScDocument::IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, bool bLeftIsEmpty,
                                ScRange* pLastRange, Rectangle* pLastMM ) const
{
    if (!IsBlockEmpty( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
        return false;

    if (HasAttrib(ScRange(nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab), HASATTR_LINES))
        // We want to print sheets with borders even if there is no cell content.
        return false;

    Rectangle aMMRect;
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

        aMMRect.Left()  = (long)(nLeft  * HMM_PER_TWIPS);
        aMMRect.Right() = (long)(nRight * HMM_PER_TWIPS);
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
        //  aehnlich wie in ScPrintFunc::AdjustPrintArea
        //! ExtendPrintArea erst ab Start-Spalte des Druckbereichs

        SCCOL nExtendCol = nStartCol - 1;
        SCROW nTmpRow = nEndRow;

        // ExtendMerge() is non-const, but called without refresh. GetPrinter()
        // might create and assign a printer.
        ScDocument* pThis = const_cast<ScDocument*>(this);

        pThis->ExtendMerge( 0,nStartRow, nExtendCol,nTmpRow, nTab,
                            false );      // kein Refresh, incl. Attrs

        OutputDevice* pDev = pThis->GetPrinter();
        pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
        ExtendPrintArea( pDev, nTab, 0, nStartRow, nExtendCol, nEndRow );
        if ( nExtendCol >= nStartCol )
            return false;
    }

    return true;
}

void ScDocument::Clear( bool bFromDestructor )
{
    TableContainer::iterator it = maTabs.begin();
    for (;it != maTabs.end(); ++it)
        delete *it;
    maTabs.clear();
    delete pSelectionAttr;
    pSelectionAttr = NULL;

    if (pDrawLayer)
    {
        pDrawLayer->ClearModel( bFromDestructor );
    }
}

bool ScDocument::HasDetectiveObjects(SCTAB nTab) const
{
    //  looks for detective objects, annotations don't count
    //  (used to adjust scale so detective objects hit their cells better)

    bool bFound = false;

    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
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
    //  In alten Versionen (bis incl. 4.0 ohne SP) wurden beim Austausch zwischen
    //  Systemen die CharSets in den Font-Attributen nicht angepasst.
    //  Das muss fuer Dokumente bis incl SP2 nun nachgeholt werden:
    //  Alles, was nicht SYMBOL ist, wird auf den System-CharSet umgesetzt.
    //  Bei neuen Dokumenten (Version SC_FONTCHARSET) sollte der CharSet stimmen.

    bool bUpdateOld = ( nSrcVer < SC_FONTCHARSET );

    rtl_TextEncoding eSysSet = osl_getThreadTextEncoding();
    if ( eSrcSet != eSysSet || bUpdateOld )
    {
        sal_uInt32 nCount,i;
        SvxFontItem* pItem;

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        nCount = pPool->GetItemCount2(ATTR_FONT);
        for (i=0; i<nCount; i++)
        {
            pItem = const_cast<SvxFontItem*>(static_cast<const SvxFontItem*>(pPool->GetItem2(ATTR_FONT, i)));
            if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                            ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                pItem->SetCharSet(eSysSet);
        }

        if ( pDrawLayer )
        {
            SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
            nCount = rDrawPool.GetItemCount2(EE_CHAR_FONTINFO);
            for (i=0; i<nCount; i++)
            {
                pItem = const_cast<SvxFontItem*>(static_cast<const SvxFontItem*>(rDrawPool.GetItem2(EE_CHAR_FONTINFO, i)));
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
    TableContainer::iterator it = maTabs.begin();
    for (; it != maTabs.end(); ++it)
    {
        if (!*it)
            return;

        (*it)->SetLoadingMedium(bVal);
    }
}

void ScDocument::SetImportingXML( bool bVal )
{
    bImportingXML = bVal;
    if (pDrawLayer)
        pDrawLayer->EnableAdjust(!bImportingXML);

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

rtl::Reference<SvxForbiddenCharactersTable> ScDocument::GetForbiddenCharacters()
{
    return xForbiddenCharacters;
}

void ScDocument::SetForbiddenCharacters( const rtl::Reference<SvxForbiddenCharactersTable> xNew )
{
    xForbiddenCharacters = xNew;
    if ( pEditEngine )
        pEditEngine->SetForbiddenCharsTable( xForbiddenCharacters );
    if ( pDrawLayer )
        pDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
}

bool ScDocument::IsValidAsianCompression() const
{
    return ( nAsianCompression != SC_ASIANCOMPRESSION_INVALID );
}

sal_uInt8 ScDocument::GetAsianCompression() const
{
    if ( nAsianCompression == SC_ASIANCOMPRESSION_INVALID )
        return 0;
    else
        return nAsianCompression;
}

void ScDocument::SetAsianCompression(sal_uInt8 nNew)
{
    nAsianCompression = nNew;
    if ( pEditEngine )
        pEditEngine->SetAsianCompressionMode( nAsianCompression );
    if ( pDrawLayer )
        pDrawLayer->SetCharCompressType( nAsianCompression );
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
    nAsianKerning = (sal_uInt8)bNew;
    if ( pEditEngine )
        pEditEngine->SetKernAsianPunctuation( static_cast<bool>( nAsianKerning ) );
    if ( pDrawLayer )
        pDrawLayer->SetKernAsianPunctuation( static_cast<bool>( nAsianKerning ) );
}

void ScDocument::ApplyAsianEditSettings( ScEditEngineDefaulter& rEngine )
{
    rEngine.SetForbiddenCharsTable( xForbiddenCharacters );
    rEngine.SetAsianCompressionMode( GetAsianCompression() );
    rEngine.SetKernAsianPunctuation( GetAsianKerning() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
