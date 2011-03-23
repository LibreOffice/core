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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sot/exchange.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/langitem.hxx>
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
#include "detfunc.hxx"      // for UpdateAllComments
#include "editutil.hxx"
#include "postit.hxx"
#include "charthelper.hxx"

using namespace ::com::sun::star;
#include <stdio.h>
// -----------------------------------------------------------------------


SfxBroadcaster* ScDocument::GetDrawBroadcaster()
{
    return pDrawLayer;
}

void ScDocument::BeginDrawUndo()
{
    if (pDrawLayer)
        pDrawLayer->BeginCalcUndo();
}

XColorTable* ScDocument::GetColorTable()
{
    if (pDrawLayer)
        return pDrawLayer->GetColorTable();
    else
    {
        if (!pColorTable)
        {
            SvtPathOptions aPathOpt;
            pColorTable = new XColorTable( aPathOpt.GetPalettePath() );
        }

        return pColorTable;
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
}

void ScDocument::InitDrawLayer( SfxObjectShell* pDocShell )
{
    if (pDocShell && !pShell)
        pShell = pDocShell;

    if (!pDrawLayer)
    {
        String aName;
        if ( pShell && !pShell->IsLoading() )       // don't call GetTitle while loading
            aName = pShell->GetTitle();
        pDrawLayer = new ScDrawLayer( this, aName );
        if (GetLinkManager())
            pDrawLayer->SetLinkManager( pLinkManager );

        //  Drawing pages are accessed by table number, so they must also be present
        //  for preceding table numbers, even if the tables aren't allocated
        //  (important for clipboard documents).

        SCTAB nDrawPages = 0;
        SCTAB nTab;
        for (nTab=0; nTab<=MAXTAB; nTab++)
            if (pTab[nTab])
                nDrawPages = nTab + 1;          // needed number of pages

        for (nTab=0; nTab<nDrawPages; nTab++)
        {
            pDrawLayer->ScAddPage( nTab );      // always add page, with or without the table
            if (pTab[nTab])
            {
                String aTabName;
                pTab[nTab]->GetName(aTabName);
                pDrawLayer->ScRenamePage( nTab, aTabName );

                pTab[nTab]->SetDrawPageSize(false,false);     // set the right size immediately
            }
        }

        pDrawLayer->SetDefaultTabulator( GetDocOptions().GetTabDistance() );

        UpdateDrawPrinter();
        UpdateDrawDefaults();
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

void ScDocument::UpdateDrawDefaults()
{
    // drawing layer defaults that are set for new documents (if InitNew was called)

    if ( pDrawLayer && bSetDrawDefaults )
    {
        SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
        rDrawPool.SetPoolDefaultItem( SvxAutoKernItem( sal_True, EE_CHAR_PAIRKERNING ) );
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
    if (!ValidTab(nTab) || !pTab[nTab])
        return;

    pTab[nTab]->SetDrawPageSize();
}

sal_Bool ScDocument::IsChart( const SdrObject* pObject )
{
    // #109985#
    // IsChart() implementation moved to svx drawinglayer
    if(pObject && OBJ_OLE2 == pObject->GetObjIdentifier())
    {
        return ((SdrOle2Obj*)pObject)->IsChart();
    }

    return false;
}

IMPL_LINK_INLINE_START( ScDocument, GetUserDefinedColor, sal_uInt16 *, pColorIndex )
{
    return (long) &((GetColorTable()->GetColor(*pColorIndex))->GetColor());
}
IMPL_LINK_INLINE_END( ScDocument, GetUserDefinedColor, sal_uInt16 *, pColorIndex )

void ScDocument::DeleteDrawLayer()
{
    delete pDrawLayer;
}

void ScDocument::DeleteColorTable()
{
    delete pColorTable;
}

sal_Bool ScDocument::DrawGetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const
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
    pDrawLayer->ScCopyPage( nOldPos, nNewPos, false );
}

void ScDocument::DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    SCTAB nTabCount = GetTableCount();
    for (SCTAB nTab=0; nTab<=nTabCount; nTab++)
        if (pTab[nTab] && rMark.GetTableSelect(nTab))
            pDrawLayer->DeleteObjectsInArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDocument::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    pDrawLayer->DeleteObjectsInSelection( rMark );
}

sal_Bool ScDocument::HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark )
{
    //  pTabMark is used only for selected tables. If pTabMark is 0, all tables of rRange are used.

    if (!pDrawLayer)
        return false;

    SCTAB nStartTab = 0;
    SCTAB nEndTab = MAXTAB;
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
            DBG_ASSERT(pPage,"Page ?");
            if (pPage)
            {
                SdrObjListIter aIter( *pPage, IM_FLAT );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( pObject->GetObjIdentifier() == OBJ_OLE2 &&
                            aMMRect.IsInside( pObject->GetCurrentBoundRect() ) )
                        return sal_True;

                    pObject = aIter.Next();
                }
            }
        }
    }

    return false;
}


void ScDocument::StartAnimations( SCTAB nTab, Window* pWin )
{
    if (!pDrawLayer)
        return;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if (pObject->ISA(SdrGrafObj))
        {
            SdrGrafObj* pGrafObj = (SdrGrafObj*)pObject;
            if ( pGrafObj->IsAnimated() )
            {
                const Rectangle& rRect = pGrafObj->GetCurrentBoundRect();
                pGrafObj->StartAnimation( pWin, rRect.TopLeft(), rRect.GetSize() );
            }
        }
        pObject = aIter.Next();
    }
}


sal_Bool ScDocument::HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect )
{
    //  Gibt es Objekte auf dem Hintergrund-Layer, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (fuer Drawing-Optimierung, vor dem Hintergrund braucht dann nicht geloescht
    //   zu werden)

    if (!pDrawLayer)
        return false;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return false;

    sal_Bool bFound = false;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_BACK && pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = sal_True;
        pObject = aIter.Next();
    }

    return bFound;
}

sal_Bool ScDocument::HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect )
{
    //  Gibt es ueberhaupt Objekte, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (um leere Seiten beim Drucken zu erkennen)

    if (!pDrawLayer)
        return false;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return false;

    sal_Bool bFound = false;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = sal_True;
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
    if (pDrawLayer && pTab[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");
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

sal_Bool ScDocument::IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_Bool bLeftIsEmpty,
                                ScRange* pLastRange, Rectangle* pLastMM ) const
{
    if (!IsBlockEmpty( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
        return false;

    ScDocument* pThis = (ScDocument*)this;  //! GetMMRect / HasAnyDraw etc. const !!!

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
        aMMRect = pThis->GetMMRect( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

    if ( pLastRange && pLastMM )
    {
        *pLastRange = ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab );
        *pLastMM = aMMRect;
    }

    if ( pThis->HasAnyDraw( nTab, aMMRect ))
        return false;

    if ( nStartCol > 0 && !bLeftIsEmpty )
    {
        //  aehnlich wie in ScPrintFunc::AdjustPrintArea
        //! ExtendPrintArea erst ab Start-Spalte des Druckbereichs

        SCCOL nExtendCol = nStartCol - 1;
        SCROW nTmpRow = nEndRow;

        pThis->ExtendMerge( 0,nStartRow, nExtendCol,nTmpRow, nTab,
                            false, sal_True );      // kein Refresh, incl. Attrs

        OutputDevice* pDev = pThis->GetPrinter();
        pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
        pThis->ExtendPrintArea( pDev, nTab, 0, nStartRow, nExtendCol, nEndRow );
        if ( nExtendCol >= nStartCol )
            return false;
    }

    return sal_True;
}

void ScDocument::Clear( sal_Bool bFromDestructor )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pTab[i])
        {
            delete pTab[i];
            pTab[i]=NULL;
        }
    delete pSelectionAttr;
    pSelectionAttr = NULL;

    if (pDrawLayer)
    {
        pDrawLayer->ClearModel( bFromDestructor );
    }
}

sal_Bool ScDocument::HasControl( SCTAB nTab, const Rectangle& rMMRect )
{
    sal_Bool bFound = false;

    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject && !bFound)
            {
                if (pObject->ISA(SdrUnoObj))
                {
                    Rectangle aObjRect = pObject->GetLogicRect();
                    if ( aObjRect.IsOver( rMMRect ) )
                        bFound = sal_True;
                }

                pObject = aIter.Next();
            }
        }
    }

    return bFound;
}

void ScDocument::InvalidateControls( Window* pWin, SCTAB nTab, const Rectangle& rMMRect )
{
    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if (pObject->ISA(SdrUnoObj))
                {
                    Rectangle aObjRect = pObject->GetLogicRect();
                    if ( aObjRect.IsOver( rMMRect ) )
                    {
                        //  Uno-Controls zeichnen sich immer komplett, ohne Ruecksicht
                        //  auf ClippingRegions. Darum muss das ganze Objekt neu gepainted
                        //  werden, damit die Selektion auf der Tabelle nicht uebermalt wird.

                        pWin->Invalidate( aObjRect );
                    }
                }

                pObject = aIter.Next();
            }
        }
    }
}

sal_Bool ScDocument::HasDetectiveObjects(SCTAB nTab) const
{
    //  looks for detective objects, annotations don't count
    //  (used to adjust scale so detective objects hit their cells better)

    sal_Bool bFound = false;

    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject && !bFound)
            {
                // anything on the internal layer except captions (annotations)
                if ( (pObject->GetLayer() == SC_LAYER_INTERN) && !ScDrawLayer::IsNoteCaption( pObject ) )
                    bFound = sal_True;

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

    sal_Bool bUpdateOld = ( nSrcVer < SC_FONTCHARSET );

    CharSet eSysSet = gsl_getSystemTextEncoding();
    if ( eSrcSet != eSysSet || bUpdateOld )
    {
        sal_uInt32 nCount,i;
        SvxFontItem* pItem;

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        nCount = pPool->GetItemCount2(ATTR_FONT);
        for (i=0; i<nCount; i++)
        {
            pItem = (SvxFontItem*)pPool->GetItem2(ATTR_FONT, i);
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
                pItem = (SvxFontItem*)rDrawPool.GetItem2(EE_CHAR_FONTINFO, i);
                if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                                ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                    pItem->SetCharSet( eSysSet );
            }
        }
    }
}

bool ScDocument::IsLoadingMedium() const
{
    return bLoadingMedium;
}

void ScDocument::SetLoadingMedium( bool bVal )
{
    bLoadingMedium = bVal;
    for (SCTAB nTab = 0; nTab <= MAXTAB; ++nTab)
    {
        if (!pTab[nTab])
            return;

        pTab[nTab]->SetLoadingMedium(bVal);
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

        for ( SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++ )
            if ( pTab[nTab]->IsLoadingRTL() )
            {
                pTab[nTab]->SetLoadingRTL( false );
                SetLayoutRTL( nTab, sal_True );             // includes mirroring; bImportingXML must be cleared first
            }
    }

    SetLoadingMedium(bVal);
}

void ScDocument::SetXMLFromWrapper( sal_Bool bVal )
{
    bXMLFromWrapper = bVal;
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

sal_Bool ScDocument::IsValidAsianCompression() const
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

sal_Bool ScDocument::IsValidAsianKerning() const
{
    return ( nAsianKerning != SC_ASIANKERNING_INVALID );
}

sal_Bool ScDocument::GetAsianKerning() const
{
    if ( nAsianKerning == SC_ASIANKERNING_INVALID )
        return false;
    else
        return (sal_Bool)nAsianKerning;
}

void ScDocument::SetAsianKerning(sal_Bool bNew)
{
    nAsianKerning = (sal_uInt8)bNew;
    if ( pEditEngine )
        pEditEngine->SetKernAsianPunctuation( (sal_Bool)nAsianKerning );
    if ( pDrawLayer )
        pDrawLayer->SetKernAsianPunctuation( (sal_Bool)nAsianKerning );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
