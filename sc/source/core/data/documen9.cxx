/*************************************************************************
 *
 *  $RCSfile: documen9.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-25 11:46:10 $
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
#include <svx/fontitem.hxx>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdundo.hxx>
#include <svx/xtable.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/saveopt.hxx>
#include <so3/ipobj.hxx>
#include <sch/schdll.hxx>
#include <sch/schdll0.hxx>
#include <sch/memchrt.hxx>

#include "document.hxx"
#include "docoptio.hxx"
#include "table.hxx"
#include "drwlayer.hxx"
#include "markdata.hxx"
#include "userdat.hxx"
#include "patattr.hxx"
#include "rechead.hxx"
#include "docpool.hxx"
#include "chartarr.hxx"
#include "detfunc.hxx"      // for UpdateAllComments


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
            SfxIniManager* pIni = SFX_APP()->GetIniManager();
            DBG_ASSERT( pIni, "SfxIniManager ???" );
            pColorTable = new XColorTable( pIni->Get( SFX_KEY_PALETTE_PATH ) );
        }

        return pColorTable;
    }
}

BOOL lcl_AdjustRanges( ScRangeList& rRanges, USHORT nSource, USHORT nDest, USHORT nTabCount )
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
            pRange->aStart.SetTab( nTabCount ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
        if ( pRange->aEnd.Tab() >= nTabCount )
        {
            pRange->aEnd.SetTab( nTabCount ? ( nTabCount - 1 ) : 0 );
            bChanged = TRUE;
        }
    }

    return bChanged;
}

void ScDocument::TransferDrawPage(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos)
{
    if (pDrawLayer && pSrcDoc->pDrawLayer)
    {
        SdrPage* pOldPage = pSrcDoc->pDrawLayer->GetPage(nSrcPos);
        SdrPage* pNewPage = pDrawLayer->GetPage(nDestPos);

        if (pOldPage && pNewPage)
        {
            SdrObjListIter aIter( *pOldPage, IM_FLAT );
            SdrObject* pOldObject = aIter.Next();
            while (pOldObject)
            {
                SdrObject* pNewObject = pOldObject->Clone( pNewPage, pDrawLayer );
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
                    if ( aIPObj.Is() && SchModuleDummy::HasID( *aIPObj->GetSvFactory() ) )
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

void ScDocument::ClearDrawPage(USHORT nTab)
{
    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
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
        if ( !pDocPool )        // der Pool wird fuer den Drucker gebraucht
        {
            DBG_ERROR("InitDrawLayer ohne DocPool");
            return;
        }

        SdrEngineDefaults::SetFontHeight(423);      // 12pt
        String aName;
        if (pShell)
            aName = pShell->GetTitle();
        pDrawLayer = new ScDrawLayer( this, aName );
        if (pLinkManager)
            pDrawLayer->SetLinkManager( pLinkManager );

        //  Draw-Pages initialisieren

        for (USHORT nTab=0; nTab<=MAXTAB; nTab++)
            if (pTab[nTab])
            {
                pDrawLayer->ScAddPage( nTab );
                String aName;
                pTab[nTab]->GetName(aName);
                pDrawLayer->ScRenamePage( nTab, aName );

                pTab[nTab]->SetDrawPageSize();  // #54782# sofort die richtige Groesse
#if 0
                ULONG nx = (ULONG) ((double) (MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
                ULONG ny = (ULONG) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
                pDrawLayer->SetPageSize( nTab, Size( nx, ny ) );
#endif
                pDrawLayer->SetDefaultTabulator(
                                    GetDocOptions().GetTabDistance());
            }

        UpdateDrawPrinter();
    }
}

void ScDocument::UpdateDrawPrinter()
{
    if (pDrawLayer)
    {
        OutputDevice* pRefDev = GetPrinter();
        if ( pPrinter->IsValid() )
            pRefDev->SetMapMode( MAP_100TH_MM );
        else
            pRefDev = Application::GetDefaultDevice();      // should not happen
        pDrawLayer->SetRefDevice(pRefDev);
    }
}

BOOL ScDocument::IsChart( SdrObject* pObject )
{
    if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
    {
        SvInPlaceObjectRef aIPObj = ((SdrOle2Obj*)pObject)->GetObjRef();
        if (aIPObj.Is())
        {
            SvGlobalName aObjClsId = *aIPObj->GetSvFactory();
            if (SchModuleDummy::HasID( aObjClsId ))
                return TRUE;
        }
    }
    return FALSE;
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

    USHORT nTableCount = 0;
    while ( nTableCount <= MAXTAB && pTab[nTableCount] )
        ++nTableCount;

    USHORT nPageCount = pDrawLayer->GetPageCount();
    if ( nPageCount > nTableCount && nTableCount != 0 )
    {
        //  Manchmal sind beim Kopieren/Verschieben/Undo von Tabellen zuviele
        //  (leere) Pages in der Tabelle stehengeblieben. Weg damit!

        DBG_ERROR("zuviele Draw-Pages in der Datei");

        for (USHORT i=nTableCount; i<nPageCount; i++)
            pDrawLayer->DeletePage(nTableCount);
    }

    //  Controls auf richtigen Layer setzen
    //  (zumindest in Dateien aus der 502 koennen sie falsch sein,
    //   wegen des fehlenden Layers in alten Dateien)

    nPageCount = pDrawLayer->GetPageCount();
    for (USHORT i=0; i<nPageCount; i++)
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

        SfxApplication* pSfxApp = SFX_APP();

        BOOL bCompr = pSfxApp->GetOptions().IsSaveGraphicsCompressed();
        pDrawLayer->SetSaveCompressed( bCompr );

        BOOL bNative = pSfxApp->GetOptions().IsSaveOriginalGraphics();
        pDrawLayer->SetSaveNative( bNative );

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

void ScDocument::DeleteObjectsInArea( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                        const ScMarkData& rMark )
{
    if (!pDrawLayer)
        return;

    USHORT nTabCount = GetTableCount();
    for (USHORT nTab=0; nTab<=nTabCount; nTab++)
        if (pTab[nTab] && rMark.GetTableSelect(nTab))
            pDrawLayer->DeleteObjectsInArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDocument::DeleteObjects( USHORT nTab )
{
    if (!pDrawLayer)
        return;

    if ( nTab<=MAXTAB && pTab[nTab] )
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

void ScDocument::StopAnimations( USHORT nTab, Window* pWin )
{
    if (!pDrawLayer)
        return;
    SdrPage* pPage = pDrawLayer->GetPage(nTab);
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

void ScDocument::StartAnimations( USHORT nTab, Window* pWin )
{
    if (!pDrawLayer)
        return;
    SdrPage* pPage = pDrawLayer->GetPage(nTab);
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
                const Rectangle& rRect = pGrafObj->GetBoundRect();
                pGrafObj->StartAnimation( pWin, rRect.TopLeft(), rRect.GetSize() );
            }
        }
        pObject = aIter.Next();
    }
}

BOOL ScDocument::HasNoteObject( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(nTab);
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
            if ( pData && nCol == pData->aStt.nCol && nRow == pData->aStt.nRow )
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

    BOOL bAnyNote = FALSE;
    USHORT nTab;
    ScPostIt aNote;
    for (nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetLayer() == SC_LAYER_INTERN && pObject->ISA( SdrCaptionObj ) )
                {
                    bAnyNote = TRUE;
                    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObject );
                    if ( pData )
                    {
                        if ( GetNote( pData->aStt.nCol, pData->aStt.nRow, nTab, aNote ) )
                            if ( !aNote.IsShown() )
                            {
                                aNote.SetShown(TRUE);
                                SetNote( pData->aStt.nCol, pData->aStt.nRow, nTab, aNote );
                            }
                    }
                }
                pObject = aIter.Next();
            }
        }
    }

    if (bAnyNote)
    {
        //  update attributes for all note objects

        ScDetectiveFunc aFunc( this, 0 );
        aFunc.UpdateAllComments();
    }
}

BOOL ScDocument::HasBackgroundDraw( USHORT nTab, const Rectangle& rMMRect )
{
    //  Gibt es Objekte auf dem Hintergrund-Layer, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (fuer Drawing-Optimierung, vor dem Hintergrund braucht dann nicht geloescht
    //   zu werden)

    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetLayer() == SC_LAYER_BACK && pObject->GetBoundRect().IsOver( rMMRect ) )
            bFound = TRUE;
        pObject = aIter.Next();
    }

    return bFound;
}

BOOL ScDocument::HasAnyDraw( USHORT nTab, const Rectangle& rMMRect )
{
    //  Gibt es ueberhaupt Objekte, die (teilweise) von rMMRect
    //  betroffen sind?
    //  (um leere Seiten beim Drucken zu erkennen)

    if (!pDrawLayer)
        return FALSE;
    SdrPage* pPage = pDrawLayer->GetPage(nTab);
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if ( pObject->GetBoundRect().IsOver( rMMRect ) )
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

SdrObject* ScDocument::GetObjectAtPoint( USHORT nTab, const Point& rPos )
{
    //  fuer Drag&Drop auf Zeichenobjekt

    SdrObject* pFound = NULL;
    if (pDrawLayer && pTab[nTab])
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_FLAT );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( pObject->GetBoundRect().IsInside(rPos) )
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

BOOL ScDocument::IsPrintEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, BOOL bLeftIsEmpty,
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
        USHORT i;
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

        USHORT nExtendCol = nStartCol - 1;
        USHORT nTmpRow = nEndRow;

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

void ScDocument::Clear()
{
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pTab[i])
        {
            delete pTab[i];
            pTab[i]=NULL;
        }
    delete pSelectionAttr;
    pSelectionAttr = NULL;

    if (pDrawLayer)
        pDrawLayer->Clear();
}

BOOL ScDocument::HasControl( USHORT nTab, const Rectangle& rMMRect )
{
    BOOL bFound = FALSE;

    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
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

void ScDocument::InvalidateControls( Window* pWin, USHORT nTab, const Rectangle& rMMRect )
{
    if (pDrawLayer)
    {
        SdrPage* pPage = pDrawLayer->GetPage(nTab);
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

        nCount = pDocPool->GetItemCount(ATTR_FONT);
        for (i=0; i<nCount; i++)
        {
            pItem = (SvxFontItem*)pDocPool->GetItem(ATTR_FONT, i);
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




