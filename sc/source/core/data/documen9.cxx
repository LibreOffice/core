/*************************************************************************
 *
 *  $RCSfile: documen9.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:15:55 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sot/exchange.hxx>
#include <svx/fontitem.hxx>
#include <svx/forbiddencharacterstable.hxx>
#include <svx/langitem.hxx>
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
#include <svtools/saveopt.hxx>
#include <svtools/pathoptions.hxx>
#include <so3/ipobj.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>

#include "document.hxx"
#include "docoptio.hxx"
#include "table.hxx"
#include "drwlayer.hxx"
#include "markdata.hxx"
#include "userdat.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "chartarr.hxx"
#include "detfunc.hxx"      // for UpdateAllComments
#include "editutil.hxx"


// -----------------------------------------------------------------------


void ScDocument::SetLinkManager( SvxLinkManager* pNew )
{
    pLinkManager = pNew;
    if (pDrawLayer)
        pDrawLayer->SetLinkManager( pNew );
}

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

BOOL lcl_AdjustRanges( ScRangeList& rRanges, SCTAB nSource, SCTAB nDest, SCTAB nTabCount )
{
    //! if multiple sheets are copied, update references into the other copied sheets?

    BOOL bChanged = FALSE;

    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange* pRange = rRanges.GetObject(i);
        if ( pRange->aStart.Tab() == nSource && pRange->aEnd.Tab() == nSource )
        {
            pRange->aStart.SetTab( nDest );
            pRange->aEnd.SetTab( nDest );
            bChanged = TRUE;
        }
        if ( pRange->aStart.Tab() >= nTabCount )
        {
            pRange->aStart.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
        if ( pRange->aEnd.Tab() >= nTabCount )
        {
            pRange->aEnd.SetTab( nTabCount > 0 ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
    }

    return bChanged;
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

                //  #71726# if it's a chart, make sure the data references are valid
                //  (this must be after InsertObject!)

                if ( pNewObject->GetObjIdentifier() == OBJ_OLE2 )
                {
                    //  test if it's a chart with HasID, because GetChartData always loads the DLL
                    SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pNewObject)->GetObjRef();
                    if ( aIPObj.Is() && SotExchange::IsChart( *aIPObj->GetSvFactory() ) )
                    {
                        SchMemChart* pChartData = SchDLL::GetChartData(aIPObj);
                        if ( pChartData )
                        {
                            ScChartArray aArray( this, *pChartData );   // parses range description
                            ScRangeListRef xRanges = aArray.GetRangeList();
                            if ( xRanges.Is() )
                            {
                                ScRangeListRef xNewRanges = new ScRangeList( *xRanges );
                                if ( lcl_AdjustRanges( *xNewRanges,
                                                        nSrcPos, nDestPos, GetTableCount() ) )
                                {
                                    aArray.SetRangeList( xNewRanges );
                                }

                                // update all charts, even if the ranges were not changed

                                SchMemChart* pMemChart = aArray.CreateMemChart();
                                ScChartArray::CopySettings( *pMemChart, *pChartData );
                                SchDLL::Update( aIPObj, pMemChart );
                                delete pMemChart;
                            }
                        }
                    }
                }

                pOldObject = aIter.Next();
            }
        }
    }
}

void ScDocument::ClearDrawPage(SCTAB nTab)
{
    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        if (pPage)
            pPage->Clear();
        else
            DBG_ERROR("ScDocument::DeleteDrawObjects: pPage ???");
    }
}

void ScDocument::InitDrawLayer( SfxObjectShell* pDocShell )
{
    if (pDocShell && !pShell)
        pShell = pDocShell;

//  DBG_ASSERT(pShell,"InitDrawLayer ohne Shell");

    if (!pDrawLayer)
    {
        String aName;
        if ( pShell && !pShell->IsLoading() )       // #88438# don't call GetTitle while loading
            aName = pShell->GetTitle();
        pDrawLayer = new ScDrawLayer( this, aName );
        if (pLinkManager)
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
                String aName;
                pTab[nTab]->GetName(aName);
                pDrawLayer->ScRenamePage( nTab, aName );

                pTab[nTab]->SetDrawPageSize();  // #54782# sofort die richtige Groesse
#if 0
                ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
                ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
                pDrawLayer->SetPageSize( nTab, Size( nx, ny ) );
#endif
            }
        }

        pDrawLayer->SetDefaultTabulator( GetDocOptions().GetTabDistance() );

        UpdateDrawPrinter();
        UpdateDrawLanguages();
        if (bImportingXML)
            pDrawLayer->EnableAdjust(FALSE);

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

        OutputDevice* pRefDev = GetPrinter();
        pRefDev->SetMapMode( MAP_100TH_MM );
        pDrawLayer->SetRefDevice(pRefDev);
    }
}

sal_Bool ScDocument::IsChart( const SdrObject* pObject )
{
    // #109985#
    // IsChart() implementation moved to svx drawinglayer
    if(pObject && OBJ_OLE2 == pObject->GetObjIdentifier())
    {
        return ((SdrOle2Obj*)pObject)->IsChart();
    }

    return sal_False;
}

IMPL_LINK_INLINE_START( ScDocument, GetUserDefinedColor, USHORT *, pColorIndex )
{
    return (long) &((GetColorTable()->Get(*pColorIndex))->GetColor());
}
IMPL_LINK_INLINE_END( ScDocument, GetUserDefinedColor, USHORT *, pColorIndex )

void ScDocument::DeleteDrawLayer()
{
    delete pDrawLayer;
}

void ScDocument::DeleteColorTable()
{
    delete pColorTable;
}

void ScDocument::LoadDrawLayer(SvStream& rStream)
{
    InitDrawLayer();                        // anlegen
    pDrawLayer->Load(rStream);

    //  nMaxTableNumber ist noch nicht initialisiert

    sal_uInt16 nTableCount = 0;
    while ( nTableCount <= MAXTAB && pTab[nTableCount] )
        ++nTableCount;

    sal_uInt16 nPageCount = pDrawLayer->GetPageCount();
    if ( nPageCount > nTableCount && nTableCount != 0 )
    {
        //  Manchmal sind beim Kopieren/Verschieben/Undo von Tabellen zuviele
        //  (leere) Pages in der Tabelle stehengeblieben. Weg damit!

        DBG_ERROR("zuviele Draw-Pages in der Datei");

        for (sal_uInt16 i=nTableCount; i<nPageCount; i++)
            pDrawLayer->DeletePage(nTableCount);
    }

    //  Controls auf richtigen Layer setzen
    //  (zumindest in Dateien aus der 502 koennen sie falsch sein,
    //   wegen des fehlenden Layers in alten Dateien)

    nPageCount = pDrawLayer->GetPageCount();
    for (sal_uInt16 i=0; i<nPageCount; i++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(i);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            if ( pObject->ISA(SdrUnoObj) && pObject->GetLayer() != SC_LAYER_CONTROLS )
            {
                pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                DBG_ERROR("Control war auf falschem Layer");
            }
            pObject = aIter.Next();
        }
    }
}

void ScDocument::StoreDrawLayer(SvStream& rStream) const
{
    if (pDrawLayer)
    {
        //  SetSavePortable wird mit VCL nicht mehr gebraucht
        //BOOL bIndep = SFX_APP()->GetOptions().IsIndepGrfFmt();
        //pDrawLayer->SetSavePortable( bIndep );

        SvtSaveOptions aSaveOpt;
        SvtSaveOptions::SaveGraphicsMode eMode = aSaveOpt.GetSaveGraphicsMode();

        BOOL bNative = ( eMode == SvtSaveOptions::SaveGraphicsOriginal );
        BOOL bCompr = bNative || ( eMode == SvtSaveOptions::SaveGraphicsCompressed );

        pDrawLayer->SetSaveCompressed( bCompr );
        pDrawLayer->SetSaveNative( bNative );

        pDrawLayer->GetItemPool().SetFileFormatVersion( (USHORT)rStream.GetVersion() );
        pDrawLayer->Store(rStream);
    }
}

BOOL ScDocument::DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const
{
    return pDrawLayer->GetPrintArea( rRange, bSetHor, bSetVer );
}

void ScDocument::DrawMovePage( USHORT nOldPos, USHORT nNewPos )
{
    pDrawLayer->ScMovePage(nOldPos,nNewPos);
}

void ScDocument::DrawCopyPage( USHORT nOldPos, USHORT nNewPos )
{
    // angelegt wird die Page schon im ScTable ctor
    pDrawLayer->ScCopyPage( nOldPos, nNewPos, FALSE );
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

void ScDocument::DeleteObjects( SCTAB nTab )
{
    if (!pDrawLayer)
        return;

    if ( ValidTab(nTab) && pTab[nTab] )
        pDrawLayer->DeleteObjects( nTab );
    else
        DBG_ERROR("DeleteObjects: falsche Tabelle");
}

void ScDocument::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    pDrawLayer->DeleteObjectsInSelection( rMark );
}

BOOL ScDocument::HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark )
{
    //  pTabMark is used only for selected tables. If pTabMark is 0, all tables of rRange are used.

    if (!pDrawLayer)
        return FALSE;

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
                        return TRUE;

                    pObject = aIter.Next();
                }
            }
        }
    }

    return FALSE;
}


void ScDocument::StopAnimations( SCTAB nTab, Window* pWin )
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
//!             pGrafObj->StopAnimation( pWin );
                pGrafObj->StopAnimation();
        }
        pObject = aIter.Next();
    }
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

BOOL ScDocument::HasNoteObject( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
        {
            ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
            if ( pData && nCol == pData->aStt.Col() && nRow == pData->aStt.Row() )
                bFound = TRUE;
        }
        pObject = aIter.Next();
    }

    return bFound;
}

void ScDocument::RefreshNoteFlags()
{
    if (!pDrawLayer)
        return;

    BOOL bAnyIntObj = FALSE;
    SCTAB nTab;
    ScPostIt aNote;
    for (nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN )
                {
                    bAnyIntObj = TRUE;  // for all internal objects, including detective

                    if ( pObject->ISA( SdrCaptionObj ) )
                    {
                        ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
                        if ( pData )
                        {
                            if ( GetNote( pData->aStt.Col(), pData->aStt.Row(), nTab, aNote))
                                if ( !aNote.IsShown() )
                                {
                                    aNote.SetShown(TRUE);
                                    SetNote( pData->aStt.Col(), pData->aStt.Row(), nTab, aNote);
                                }
                        }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    if (bAnyIntObj)
    {
        //  update attributes for all note objects and the colors of detective objects
        //  (we don't know with which settings the file was created)

        ScDetectiveFunc aFunc( this, 0 );
        aFunc.UpdateAllComments();
        aFunc.UpdateAllArrowColors();
    }
}

BOOL ScDocument::HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect )
{
    //  Gibt es Objekte auf dem Hintergrund-Layer, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (fuer Drawing-Optimierung, vor dem Hintergrund braucht dann nicht geloescht
    //   zu werden)

    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_BACK && pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = TRUE;
        pObject = aIter.Next();
    }

    return bFound;
}

BOOL ScDocument::HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect )
{
    //  Gibt es ueberhaupt Objekte, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (um leere Seiten beim Drucken zu erkennen)

    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetCurrentBoundRect().IsOver( rMMRect ) )
            bFound = TRUE;
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
                    if ( nLayer != SC_LAYER_INTERN )
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

BOOL ScDocument::IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, BOOL bLeftIsEmpty,
                                ScRange* pLastRange, Rectangle* pLastMM ) const
{
    if (!IsBlockEmpty( nTab, nStartCol, nStartRow, nEndCol, nEndRow ))
        return FALSE;

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
        return FALSE;

    if ( nStartCol > 0 && !bLeftIsEmpty )
    {
        //  aehnlich wie in ScPrintFunc::AdjustPrintArea
        //! ExtendPrintArea erst ab Start-Spalte des Druckbereichs

        SCCOL nExtendCol = nStartCol - 1;
        SCROW nTmpRow = nEndRow;

        pThis->ExtendMerge( 0,nStartRow, nExtendCol,nTmpRow, nTab,
                            FALSE, TRUE );      // kein Refresh, incl. Attrs

        OutputDevice* pDev = pThis->GetPrinter();
        pDev->SetMapMode( MAP_PIXEL );              // wichtig fuer GetNeededSize
        pThis->ExtendPrintArea( pDev, nTab, 0, nStartRow, nExtendCol, nEndRow );
        if ( nExtendCol >= nStartCol )
            return FALSE;
    }

    return TRUE;
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
        // #116168#
        //pDrawLayer->Clear();
        pDrawLayer->ClearModel( bFromDestructor );
    }
}

BOOL ScDocument::HasControl( SCTAB nTab, const Rectangle& rMMRect )
{
    BOOL bFound = FALSE;

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
                        bFound = TRUE;
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

                        //pWin->Invalidate( aObjRect.GetIntersection( rMMRect ) );
                        pWin->Invalidate( aObjRect );
                    }
                }

                pObject = aIter.Next();
            }
        }
    }
}

BOOL ScDocument::HasDetectiveObjects(SCTAB nTab) const
{
    //  looks for detective objects, annotations don't count
    //  (used to adjust scale so detective objects hit their cells better)

    BOOL bFound = FALSE;

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
                if ( pObject->GetLayer() == SC_LAYER_INTERN && !pObject->ISA( SdrCaptionObj ) )
                    bFound = TRUE;

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

    BOOL bUpdateOld = ( nSrcVer < SC_FONTCHARSET );

    CharSet eSysSet = gsl_getSystemTextEncoding();
    if ( eSrcSet != eSysSet || bUpdateOld )
    {
        USHORT nCount,i;
        SvxFontItem* pItem;

        ScDocumentPool* pPool = xPoolHelper->GetDocPool();
        nCount = pPool->GetItemCount(ATTR_FONT);
        for (i=0; i<nCount; i++)
        {
            pItem = (SvxFontItem*)pPool->GetItem(ATTR_FONT, i);
            if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                            ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                pItem->GetCharSet() = eSysSet;
        }

        if ( pDrawLayer )
        {
            SfxItemPool& rDrawPool = pDrawLayer->GetItemPool();
            nCount = rDrawPool.GetItemCount(EE_CHAR_FONTINFO);
            for (i=0; i<nCount; i++)
            {
                pItem = (SvxFontItem*)rDrawPool.GetItem(EE_CHAR_FONTINFO, i);
                if ( pItem && ( pItem->GetCharSet() == eSrcSet ||
                                ( bUpdateOld && pItem->GetCharSet() != RTL_TEXTENCODING_SYMBOL ) ) )
                    pItem->GetCharSet() = eSysSet;
            }
        }
    }
}

void ScDocument::SetImportingXML( BOOL bVal )
{
    bImportingXML = bVal;
    if (pDrawLayer)
        pDrawLayer->EnableAdjust(!bImportingXML);
}

vos::ORef<SvxForbiddenCharactersTable> ScDocument::GetForbiddenCharacters()
{
    return xForbiddenCharacters;
}

void ScDocument::SetForbiddenCharacters( const vos::ORef<SvxForbiddenCharactersTable> xNew )
{
    xForbiddenCharacters = xNew;
    if ( pEditEngine )
        pEditEngine->SetForbiddenCharsTable( xForbiddenCharacters );
    if ( pDrawLayer )
        pDrawLayer->SetForbiddenCharsTable( xForbiddenCharacters );
}

BOOL ScDocument::IsValidAsianCompression() const
{
    return ( nAsianCompression != SC_ASIANCOMPRESSION_INVALID );
}

BYTE ScDocument::GetAsianCompression() const
{
    if ( nAsianCompression == SC_ASIANCOMPRESSION_INVALID )
        return 0;
    else
        return nAsianCompression;
}

void ScDocument::SetAsianCompression(BYTE nNew)
{
    nAsianCompression = nNew;
    if ( pEditEngine )
        pEditEngine->SetAsianCompressionMode( nAsianCompression );
    if ( pDrawLayer )
        pDrawLayer->SetCharCompressType( nAsianCompression );
}

BOOL ScDocument::IsValidAsianKerning() const
{
    return ( nAsianKerning != SC_ASIANKERNING_INVALID );
}

BOOL ScDocument::GetAsianKerning() const
{
    if ( nAsianKerning == SC_ASIANKERNING_INVALID )
        return FALSE;
    else
        return (BOOL)nAsianKerning;
}

void ScDocument::SetAsianKerning(BOOL bNew)
{
    nAsianKerning = (BYTE)bNew;
    if ( pEditEngine )
        pEditEngine->SetKernAsianPunctuation( (BOOL)nAsianKerning );
    if ( pDrawLayer )
        pDrawLayer->SetKernAsianPunctuation( (BOOL)nAsianKerning );
}

