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

#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _MACRODLG_HXX
#define _OUTLINER_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _STDCTRL_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX
#define _VALUESET_HXX

#define _PASSWD_HXX

#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX

#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX

#define _SVX_HYPHEN_HXX
#define _SVX_LAYCTRL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SELCTRL_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_STDDLG_HXX
#define _SVX_THESDLG_HXX

#include "scitems.hxx"
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svl/stritem.hxx>
#include <svl/ptitem.hxx>
#include <svl/urlbmk.hxx>
#include <sot/clsids.hxx>
#include <sot/formats.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <sot/exchange.hxx>
#include <memory>

#include "attrib.hxx"
#include "patattr.hxx"
#include "dociter.hxx"
#include "viewfunc.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "undoblk.hxx"
#include "refundo.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "impex.hxx"            // Sylk-ID fuer CB
#include "chgtrack.hxx"
#include "waitoff.hxx"
#include "scmod.hxx"
#include "sc.hrc"
#include "inputopt.hxx"
#include "warnbox.hxx"
#include "drwlayer.hxx"
#include "editable.hxx"
#include "docuno.hxx"
#include "clipparam.hxx"
#include "undodat.hxx"
#include "drawview.hxx"
#include "cliputil.hxx"

using namespace com::sun::star;

// STATIC DATA ---------------------------------------------------------------


//============================================================================

//  GlobalName der Writer-DocShell kommt jetzt aus comphelper/classids.hxx

//----------------------------------------------------------------------------
//      C U T

void ScViewFunc::CutToClip( ScDocument* pClipDoc, sal_Bool bIncludeObjects )
{
    UpdateInputLine();

    ScEditableTester aTester( this );
    if (!aTester.IsEditable())                  // selection editable?
    {
        ErrorMessage( aTester.GetMessageId() );
        return;
    }

    ScRange aRange;                             // zu loeschender Bereich
    if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        const sal_Bool bRecord(pDoc->IsUndoEnabled());                  // Undo/Redo

        ScDocShellModificator aModificator( *pDocSh );

        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )          // mark the range if not marked yet
        {
            DoneBlockMode();
            InitOwnBlockMode();
            rMark.SetMarkArea( aRange );
            MarkDataChanged();
        }

        CopyToClip( pClipDoc, sal_True, false, bIncludeObjects );           // Ab ins Clipboard

        ScAddress aOldEnd( aRange.aEnd );       // Zusammengefasste Zellen im Bereich?
        pDoc->ExtendMerge( aRange, sal_True );

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndoSelected( pDoc, rMark );
            // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
            ScRange aCopyRange = aRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(pDoc->GetTableCount()-1);
            pDoc->CopyToDocument( aCopyRange, (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS, false, pUndoDoc );
            pDoc->BeginDrawUndo();
        }

        sal_uInt16 nExtFlags = 0;
        pDocSh->UpdatePaintExt( nExtFlags, aRange );

        rMark.MarkToMulti();
        pDoc->DeleteSelection( IDF_ALL, rMark );
        if ( bIncludeObjects )
            pDoc->DeleteObjectsInSelection( rMark );
        rMark.MarkToSimple();

        if ( !AdjustRowHeight( aRange.aStart.Row(), aRange.aEnd.Row() ) )
            pDocSh->PostPaint( aRange, PAINT_GRID, nExtFlags );

        if ( bRecord )                          // erst jetzt ist Draw-Undo verfuegbar
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoCut( pDocSh, aRange, aOldEnd, rMark, pUndoDoc ) );

        aModificator.SetDocumentModified();
        pDocSh->UpdateOle(GetViewData());

        CellContentChanged();
    }
    else
        ErrorMessage( STR_NOMULTISELECT );
}


//----------------------------------------------------------------------------
//      C O P Y

sal_Bool ScViewFunc::CopyToClip( ScDocument* pClipDoc, sal_Bool bCut, sal_Bool bApi, sal_Bool bIncludeObjects, sal_Bool bStopEdit )
{
    ScRange aRange;
    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aRange );
    ScMarkData& rMark = GetViewData()->GetMarkData();
    sal_Bool bDone = sal_False;

    if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
       ScRangeList aRangeList;
       aRangeList.Append( aRange );
       bDone = CopyToClip( pClipDoc, aRangeList, bCut, bApi, bIncludeObjects, bStopEdit, sal_False );
    }
    else if (eMarkType == SC_MARK_MULTI)
    {
        ScRangeList aRangeList;
        rMark.MarkToSimple();
        rMark.FillRangeListWithMarks(&aRangeList, false);
        bDone = CopyToClip( pClipDoc, aRangeList, bCut, bApi, bIncludeObjects, bStopEdit, sal_False );
    }
    else
    {
        if (!bApi)
            ErrorMessage(STR_NOMULTISELECT);
    }

    return bDone;
}

// Copy the content of the Range into clipboard.
sal_Bool ScViewFunc::CopyToClip( ScDocument* pClipDoc, const ScRangeList& rRanges, sal_Bool bCut, sal_Bool bApi, sal_Bool bIncludeObjects, sal_Bool bStopEdit, sal_Bool bUseRangeForVBA )
{
    if ( rRanges.empty() )
        return false;
    sal_Bool bDone = false;
    if ( bStopEdit )
        UpdateInputLine();

    ScRange aRange = *rRanges[0];
    ScClipParam aClipParam( aRange, bCut );
    aClipParam.maRanges = rRanges;

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();

    if ( !aClipParam.isMultiRange() )
    {
        if ( pDoc && ( !pDoc->HasSelectedBlockMatrixFragment( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), rMark ) ) )
        {
            sal_Bool bSysClip = false;
            if ( !pClipDoc )                                    // no clip doc specified
            {
                // Create one (deleted by ScTransferObj).
                pClipDoc = new ScDocument( SCDOCMODE_CLIP );
                bSysClip = sal_True;                                // and copy into system
            }
            if ( !bCut )
            {
                ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
                if ( pChangeTrack )
                    pChangeTrack->ResetLastCut();
            }

            if ( bSysClip && bIncludeObjects )
            {
                sal_Bool bAnyOle = pDoc->HasOLEObjectsInArea( aRange );
                // Update ScGlobal::pDrawClipDocShellRef.
                ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
            }

            if ( !bUseRangeForVBA )
                // is this necessary?, will setting the doc id upset the
                // following paste operation with range? would be nicer to just set this always
                // and lose the 'if' above
                aClipParam.setSourceDocID( pDoc->GetDocumentID() );

            pDoc->CopyToClip( aClipParam, pClipDoc, &rMark, false, false, bIncludeObjects, true, bUseRangeForVBA );
            if ( !bUseRangeForVBA && pDoc && pClipDoc )
            {
                ScDrawLayer* pDrawLayer = pClipDoc->GetDrawLayer();
                if ( pDrawLayer )
                {
                    ScClipParam& rClipParam = pClipDoc->GetClipParam();
                    ScRangeListVector& rRangesVector = rClipParam.maProtectedChartRangesVector;
                    SCTAB nTabCount = pClipDoc->GetTableCount();
                    for ( SCTAB nTab = 0; nTab < nTabCount; ++nTab )
                    {
                        SdrPage* pPage = pDrawLayer->GetPage( static_cast< sal_uInt16 >( nTab ) );
                        if ( pPage )
                        {
                            ScChartHelper::FillProtectedChartRangesVector( rRangesVector, pDoc, pPage );
                        }
                    }
                }
            }

            if ( bSysClip )
            {
                ScDrawLayer::SetGlobalDrawPersist(NULL);
                ScGlobal::SetClipDocName( pDoc->GetDocumentShell()->GetTitle( SFX_TITLE_FULLNAME ) );
            }
            pClipDoc->ExtendMerge( aRange, true );

            if ( bSysClip )
            {
                ScDocShell* pDocSh = GetViewData()->GetDocShell();
                TransferableObjectDescriptor aObjDesc;
                pDocSh->FillTransferableObjectDescriptor( aObjDesc );
                aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
                // maSize is set in ScTransferObj ctor

                ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
                uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );
                if ( ScGlobal::pDrawClipDocShellRef )
                {
                    SfxObjectShellRef aPersistRef( &(*ScGlobal::pDrawClipDocShellRef) );
                    pTransferObj->SetDrawPersist( aPersistRef );// keep persist for ole objects alive

                }
                pTransferObj->CopyToClipboard( GetActiveWin() );
                SC_MOD()->SetClipObject( pTransferObj, NULL );
            }

            bDone = true;
        }
    }
    else
    {
        bool bSuccess = false;
        aClipParam.mbCutMode = false;

        do
        {
            if (bCut)
                // We con't support cutting of multi-selections.
                break;

            if (pClipDoc)
                // TODO: What's this for?
                break;

            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr<ScDocument> pDocClip(new ScDocument(SCDOCMODE_CLIP));
            SAL_WNODEPRECATED_DECLARATIONS_POP

            // Check for geometrical feasibility of the ranges.
            bool bValidRanges = true;
            ScRange* p = aClipParam.maRanges.front();
            SCCOL nPrevColDelta = 0;
            SCROW nPrevRowDelta = 0;
            SCCOL nPrevCol = p->aStart.Col();
            SCROW nPrevRow = p->aStart.Row();
            SCCOL nPrevColSize = p->aEnd.Col() - p->aStart.Col() + 1;
            SCROW nPrevRowSize = p->aEnd.Row() - p->aStart.Row() + 1;
            for ( size_t i = 1; i < aClipParam.maRanges.size(); ++i )
            {
                p = aClipParam.maRanges[i];
                if ( pDoc->HasSelectedBlockMatrixFragment(
                    p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), rMark) )
                {
                    if (!bApi)
                        ErrorMessage(STR_MATRIXFRAGMENTERR);
                    return false;
                }

                SCCOL nColDelta = p->aStart.Col() - nPrevCol;
                SCROW nRowDelta = p->aStart.Row() - nPrevRow;

                if ((nColDelta && nRowDelta) || (nPrevColDelta && nRowDelta) || (nPrevRowDelta && nColDelta))
                {
                    bValidRanges = false;
                    break;
                }

                if (aClipParam.meDirection == ScClipParam::Unspecified)
                {
                    if (nColDelta)
                        aClipParam.meDirection = ScClipParam::Column;
                    if (nRowDelta)
                        aClipParam.meDirection = ScClipParam::Row;
                }

                SCCOL nColSize = p->aEnd.Col() - p->aStart.Col() + 1;
                SCROW nRowSize = p->aEnd.Row() - p->aStart.Row() + 1;

                if (aClipParam.meDirection == ScClipParam::Column && nRowSize != nPrevRowSize)
                {
                    // column-oriented ranges must have identical row size.
                    bValidRanges = false;
                    break;
                }
                if (aClipParam.meDirection == ScClipParam::Row && nColSize != nPrevColSize)
                {
                    // likewise, row-oriented ranges must have identical
                    // column size.
                    bValidRanges = false;
                    break;
                }

                nPrevCol = p->aStart.Col();
                nPrevRow = p->aStart.Row();
                nPrevColDelta = nColDelta;
                nPrevRowDelta = nRowDelta;
                nPrevColSize  = nColSize;
                nPrevRowSize  = nRowSize;
            }
            if (!bValidRanges)
                break;
            pDoc->CopyToClip(aClipParam, pDocClip.get(), &rMark, false, false, bIncludeObjects, true, bUseRangeForVBA );

            ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
            if ( pChangeTrack )
                pChangeTrack->ResetLastCut();   // kein CutMode mehr

            {
                ScDocShell* pDocSh = GetViewData()->GetDocShell();
                TransferableObjectDescriptor aObjDesc;
                pDocSh->FillTransferableObjectDescriptor( aObjDesc );
                aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
                // maSize is set in ScTransferObj ctor

                ScTransferObj* pTransferObj = new ScTransferObj( pDocClip.release(), aObjDesc );
                uno::Reference<datatransfer::XTransferable> xTransferable( pTransferObj );

                if ( ScGlobal::pDrawClipDocShellRef )
                {
                    SfxObjectShellRef aPersistRef( &(*ScGlobal::pDrawClipDocShellRef) );
                    pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
                }

                pTransferObj->CopyToClipboard( GetActiveWin() );    // system clipboard
                SC_MOD()->SetClipObject( pTransferObj, NULL );      // internal clipboard
            }

            bSuccess = true;
        }
        while (false);

        if (!bSuccess && !bApi)
            ErrorMessage(STR_NOMULTISELECT);

        bDone = bSuccess;
    }

    return bDone;
}

ScTransferObj* ScViewFunc::CopyToTransferable()
{
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if ( !pDoc->HasSelectedBlockMatrixFragment(
                        aRange.aStart.Col(), aRange.aStart.Row(),
                        aRange.aEnd.Col(),   aRange.aEnd.Row(),
                        rMark ) )
        {
            ScDocument *pClipDoc = new ScDocument( SCDOCMODE_CLIP );    // create one (deleted by ScTransferObj)

            sal_Bool bAnyOle = pDoc->HasOLEObjectsInArea( aRange, &rMark );
            ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );

            ScClipParam aClipParam(aRange, false);
            pDoc->CopyToClip(aClipParam, pClipDoc, &rMark, false, false, true);

            ScDrawLayer::SetGlobalDrawPersist(NULL);
            pClipDoc->ExtendMerge( aRange, sal_True );

            ScDocShell* pDocSh = GetViewData()->GetDocShell();
            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            ScTransferObj* pTransferObj = new ScTransferObj( pClipDoc, aObjDesc );
            return pTransferObj;
        }
    }

    return NULL;
}

//----------------------------------------------------------------------------
//      P A S T E

void ScViewFunc::PasteDraw()
{
    ScViewData* pViewData = GetViewData();
    SCCOL nPosX = pViewData->GetCurX();
    SCROW nPosY = pViewData->GetCurY();
    Window* pWin = GetActiveWin();
    Point aPos = pWin->PixelToLogic( pViewData->GetScrPos( nPosX, nPosY,
                                     pViewData->GetActivePart() ) );
    ScDrawTransferObj* pDrawClip = ScDrawTransferObj::GetOwnClipboard( pWin );
    if (pDrawClip)
        PasteDraw( aPos, pDrawClip->GetModel(), false,
            pDrawClip->GetSourceDocID() == pViewData->GetDocument()->GetDocumentID() );
}

void ScViewFunc::PasteFromSystem()
{
    UpdateInputLine();

    Window* pWin = GetActiveWin();
    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
    ScDrawTransferObj* pDrawClip = ScDrawTransferObj::GetOwnClipboard( pWin );

    if (pOwnClip)
    {
        // keep a reference in case the clipboard is changed during PasteFromClip
        uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
        PasteFromClip( IDF_ALL, pOwnClip->GetDocument(),
                        PASTE_NOFUNC, false, false, false, INS_NONE, IDF_NONE,
                        sal_True );     // allow warning dialog
    }
    else if (pDrawClip)
        PasteDraw();
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );

        {
            sal_uLong nBiff8 = SotExchange::RegisterFormatName(rtl::OUString("Biff8"));
            sal_uLong nBiff5 = SotExchange::RegisterFormatName(rtl::OUString("Biff5"));

                //  als erstes SvDraw-Model, dann Grafik
                //  (Grafik darf nur bei einzelner Grafik drinstehen)

            if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ))
            {
                // special case for tables from drawing
                if( aDataHelper.HasFormat( SOT_FORMAT_RTF ) )
                {
                    PasteFromSystem( FORMAT_RTF );
                }
                else
                {
                    PasteFromSystem( SOT_FORMATSTR_ID_DRAWING );
                }
            }
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ))
                PasteFromSystem( SOT_FORMATSTR_ID_SVXB );
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ))
            {
                //  If it's a Writer object, insert RTF instead of OLE

                //  Else, if the class id is all-zero, and SYLK is available,
                //  it probably is spreadsheet cells that have been put
                //  on the clipboard by OOo, so use the SYLK. (fdo#31077)

                sal_Bool bDoRtf = false;
                TransferableObjectDescriptor aObjDesc;
                if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) )
                {
                    bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                                 aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                               && aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
                }
                if ( bDoRtf )
                    PasteFromSystem( FORMAT_RTF );
                else if ( aObjDesc.maClassName == SvGlobalName( 0,0,0,0,0,0,0,0,0,0,0 )
                          && aDataHelper.HasFormat( SOT_FORMATSTR_ID_SYLK ))
                    PasteFromSystem( SOT_FORMATSTR_ID_SYLK );
                else
                    PasteFromSystem( SOT_FORMATSTR_ID_EMBED_SOURCE );
            }
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ))
                PasteFromSystem( SOT_FORMATSTR_ID_LINK_SOURCE );
            // the following format can not affect scenario from #89579#
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE );
            // FORMAT_PRIVATE no longer here (can't work if pOwnClip is NULL)
            else if (aDataHelper.HasFormat(nBiff8))      // before xxx_OLE formats
                PasteFromSystem(nBiff8);
            else if (aDataHelper.HasFormat(nBiff5))
                PasteFromSystem(nBiff5);
            else if (aDataHelper.HasFormat(FORMAT_RTF))
                PasteFromSystem(FORMAT_RTF);
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_HTML))
                PasteFromSystem(SOT_FORMATSTR_ID_HTML);
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE))
                PasteFromSystem(SOT_FORMATSTR_ID_HTML_SIMPLE);
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_SYLK))
                PasteFromSystem(SOT_FORMATSTR_ID_SYLK);
            else if (aDataHelper.HasFormat(FORMAT_STRING))
                PasteFromSystem(FORMAT_STRING);
            else if (aDataHelper.HasFormat(FORMAT_GDIMETAFILE))
                PasteFromSystem(FORMAT_GDIMETAFILE);
            else if (aDataHelper.HasFormat(FORMAT_BITMAP))
                PasteFromSystem(FORMAT_BITMAP);
            // xxx_OLE formats come last, like in SotExchange tables
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE );
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_LINK_SOURCE_OLE );
        }
    }
    //  keine Fehlermeldung, weil SID_PASTE in der idl das FastCall-Flag hat,
    //  also auch gerufen wird, wenn nichts im Clipboard steht (#42531#)
}

void ScViewFunc::PasteFromTransferable( const uno::Reference<datatransfer::XTransferable>& rxTransferable )
{
    ScTransferObj *pOwnClip=0;
    ScDrawTransferObj *pDrawClip=0;
    uno::Reference<lang::XUnoTunnel> xTunnel( rxTransferable, uno::UNO_QUERY );
    if ( xTunnel.is() )
    {
        sal_Int64 nHandle = xTunnel->getSomething( ScTransferObj::getUnoTunnelId() );
        if ( nHandle )
            pOwnClip = (ScTransferObj*) (sal_IntPtr) nHandle;
        else
        {
            nHandle = xTunnel->getSomething( ScDrawTransferObj::getUnoTunnelId() );
            if ( nHandle )
                pDrawClip = (ScDrawTransferObj*) (sal_IntPtr) nHandle;
        }
    }

    if (pOwnClip)
    {
        PasteFromClip( IDF_ALL, pOwnClip->GetDocument(),
                        PASTE_NOFUNC, false, false, false, INS_NONE, IDF_NONE,
                        sal_True );     // allow warning dialog
    }
    else if (pDrawClip)
    {
        ScViewData* pViewData = GetViewData();
        SCCOL nPosX = pViewData->GetCurX();
        SCROW nPosY = pViewData->GetCurY();
        Window* pWin = GetActiveWin();
        Point aPos = pWin->PixelToLogic( pViewData->GetScrPos( nPosX, nPosY, pViewData->GetActivePart() ) );
        PasteDraw( aPos, pDrawClip->GetModel(), false, pDrawClip->GetSourceDocID() == pViewData->GetDocument()->GetDocumentID() );
    }
    else
    {
            TransferableDataHelper aDataHelper( rxTransferable );
        {
            sal_uLong nBiff8 = SotExchange::RegisterFormatName(rtl::OUString("Biff8"));
            sal_uLong nBiff5 = SotExchange::RegisterFormatName(rtl::OUString("Biff5"));
            sal_uLong nFormatId = 0;
                //  als erstes SvDraw-Model, dann Grafik
                //  (Grafik darf nur bei einzelner Grafik drinstehen)

            if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ))
                nFormatId = SOT_FORMATSTR_ID_DRAWING;
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ))
                nFormatId = SOT_FORMATSTR_ID_SVXB;
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ))
            {
                //  If it's a Writer object, insert RTF instead of OLE
                sal_Bool bDoRtf = false;
                TransferableObjectDescriptor aObjDesc;
                if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) )
                {
                    bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                                 aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                               && aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
                }
                if ( bDoRtf )
                    nFormatId = FORMAT_RTF;
                else
                    nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE;
            }
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE ))
                nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE;
            // the following format can not affect scenario from #89579#
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ))
                nFormatId = SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE;
            // FORMAT_PRIVATE no longer here (can't work if pOwnClip is NULL)
            else if (aDataHelper.HasFormat(nBiff8))      // before xxx_OLE formats
                nFormatId = nBiff8;
            else if (aDataHelper.HasFormat(nBiff5))
                nFormatId = nBiff5;
            else if (aDataHelper.HasFormat(FORMAT_RTF))
                nFormatId = FORMAT_RTF;
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_HTML))
                nFormatId = SOT_FORMATSTR_ID_HTML;
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE))
                nFormatId = SOT_FORMATSTR_ID_HTML_SIMPLE;
            else if (aDataHelper.HasFormat(SOT_FORMATSTR_ID_SYLK))
                nFormatId = SOT_FORMATSTR_ID_SYLK;
            else if (aDataHelper.HasFormat(FORMAT_STRING))
                nFormatId = FORMAT_STRING;
            else if (aDataHelper.HasFormat(FORMAT_GDIMETAFILE))
                nFormatId = FORMAT_GDIMETAFILE;
            else if (aDataHelper.HasFormat(FORMAT_BITMAP))
                nFormatId = FORMAT_BITMAP;
            // xxx_OLE formats come last, like in SotExchange tables
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ))
                nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
                nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE_OLE;
            else
                return;

            PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                NULL, false, false );
        }
    }
}

sal_Bool ScViewFunc::PasteFromSystem( sal_uLong nFormatId, sal_Bool bApi )
{
    UpdateInputLine();

    sal_Bool bRet = sal_True;
    Window* pWin = GetActiveWin();
    ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
    if ( nFormatId == 0 && pOwnClip )
    {
        // keep a reference in case the clipboard is changed during PasteFromClip
        uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
        PasteFromClip( IDF_ALL, pOwnClip->GetDocument(),
                        PASTE_NOFUNC, false, false, false, INS_NONE, IDF_NONE,
                        !bApi );        // allow warning dialog
    }
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
        if ( !aDataHelper.GetTransferable().is() )
            return false;

        bRet = PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                                GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                                NULL, false, !bApi );       // allow warning dialog

        if ( !bRet && !bApi )
            ErrorMessage(STR_PASTE_ERROR);
    }
    return bRet;
}


//----------------------------------------------------------------------------
//      P A S T E

sal_Bool ScViewFunc::PasteOnDrawObject( const uno::Reference<datatransfer::XTransferable>& rxTransferable,
                                    SdrObject* pHitObj, sal_Bool bLink )
{
    sal_Bool bRet = false;
    if ( bLink )
    {
        TransferableDataHelper aDataHelper( rxTransferable );
        if ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) )
        {
            SotStorageStreamRef xStm;
            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ) )
            {
                Graphic aGraphic;
                *xStm >> aGraphic;
                bRet = ApplyGraphicToObject( pHitObj, aGraphic );
            }
        }
        else if ( aDataHelper.HasFormat( SOT_FORMAT_GDIMETAFILE ) )
        {
            GDIMetaFile aMtf;
            if( aDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
                bRet = ApplyGraphicToObject( pHitObj, Graphic(aMtf) );
        }
        else if ( aDataHelper.HasFormat( SOT_FORMAT_BITMAP ) )
        {
            Bitmap aBmp;
            if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ) )
                bRet = ApplyGraphicToObject( pHitObj, Graphic(aBmp) );
        }
    }
    else
    {
        //  ham' wa noch nich
    }
    return bRet;
}

sal_Bool lcl_SelHasAttrib( ScDocument* pDoc, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        const ScMarkData& rTabSelection, sal_uInt16 nMask )
{
    ScMarkData::const_iterator itr = rTabSelection.begin(), itrEnd = rTabSelection.end();
    for (; itr != itrEnd; ++itr)
        if ( pDoc->HasAttrib( nCol1, nRow1, *itr, nCol2, nRow2, *itr, nMask ) )
            return sal_True;
    return false;
}

//
//      Einfuegen auf Tabelle:
//

//  internes Paste

namespace {

bool checkDestRangeForOverwrite(const ScRangeList& rDestRanges, const ScDocument* pDoc, const ScMarkData& rMark, Window* pParentWnd)
{
    bool bIsEmpty = true;
    ScMarkData::const_iterator itrTab = rMark.begin(), itrTabEnd = rMark.end();
    size_t nRangeSize = rDestRanges.size();
    for (; itrTab != itrTabEnd && bIsEmpty; ++itrTab)
    {
        for (size_t i = 0; i < nRangeSize && bIsEmpty; ++i)
        {
            const ScRange& rRange = *rDestRanges[i];
            bIsEmpty = pDoc->IsBlockEmpty(
                *itrTab, rRange.aStart.Col(), rRange.aStart.Row(),
                rRange.aEnd.Col(), rRange.aEnd.Row());
        }
    }

    if (!bIsEmpty)
    {
        ScReplaceWarnBox aBox(pParentWnd);
        if (aBox.Execute() != RET_YES)
        {
            //  changing the configuration is within the ScReplaceWarnBox
            return false;
        }
    }
    return true;
}

}

bool ScViewFunc::PasteFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc,
                                sal_uInt16 nFunction, bool bSkipEmpty,
                                bool bTranspose, bool bAsLink,
                                InsCellCmd eMoveMode, sal_uInt16 nUndoExtraFlags,
                                bool bAllowDialogs )
{
    if (!pClipDoc)
    {
        OSL_FAIL("PasteFromClip: pClipDoc=0 not allowed");
        return false;
    }

    //  fuer Undo etc. immer alle oder keine Inhalte sichern
    sal_uInt16 nContFlags = IDF_NONE;
    if (nFlags & IDF_CONTENTS)
        nContFlags |= IDF_CONTENTS;
    if (nFlags & IDF_ATTRIB)
        nContFlags |= IDF_ATTRIB;
    // evtl. Attribute ins Undo ohne sie vom Clip ins Doc zu kopieren
    sal_uInt16 nUndoFlags = nContFlags;
    if (nUndoExtraFlags & IDF_ATTRIB)
        nUndoFlags |= IDF_ATTRIB;
    // do not copy note captions into undo document
    nUndoFlags |= IDF_NOCAPTIONS;

    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    if (rClipParam.isMultiRange())
    {
        // Source data is multi-range.
        return PasteMultiRangesFromClip(
            nFlags, pClipDoc, nFunction, bSkipEmpty, bTranspose, bAsLink, bAllowDialogs,
            eMoveMode, nUndoFlags);
    }

    ScMarkData& rMark = GetViewData()->GetMarkData();
    if (rMark.IsMultiMarked())
    {
        // Source data is single-range but destination is multi-range.
        return PasteFromClipToMultiRanges(
            nFlags, pClipDoc, nFunction, bSkipEmpty, bTranspose, bAsLink, bAllowDialogs,
            eMoveMode, nUndoFlags);
    }

    bool bCutMode = pClipDoc->IsCutMode();      // if transposing, take from original clipdoc
    bool bIncludeFiltered = bCutMode;

    // paste drawing: also if IDF_NOTE is set (to create drawing layer for note captions)
    bool bPasteDraw = ( pClipDoc->GetDrawLayer() && ( nFlags & (IDF_OBJECTS|IDF_NOTE) ) );

    ScDocShellRef aTransShellRef;   // for objects in xTransClip - must remain valid as long as xTransClip
    ScDocument* pOrigClipDoc = NULL;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ScDocument > xTransClip;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( bTranspose )
    {
        SCCOL nX;
        SCROW nY;
        // include filtered rows until TransposeClip can skip them
        bIncludeFiltered = true;
        pClipDoc->GetClipArea( nX, nY, true );
        if ( nY > static_cast<sal_Int32>(MAXCOL) )                      // zuviele Zeilen zum Transponieren
        {
            ErrorMessage(STR_PASTE_FULL);
            return false;
        }
        pOrigClipDoc = pClipDoc;        // fuer Referenzen

        if ( bPasteDraw )
        {
            aTransShellRef = new ScDocShell;        // DocShell needs a Ref immediately
            aTransShellRef->DoInitNew(NULL);
        }
        ScDrawLayer::SetGlobalDrawPersist(aTransShellRef);

        xTransClip.reset( new ScDocument( SCDOCMODE_CLIP ));
        pClipDoc->TransposeClip( xTransClip.get(), nFlags, bAsLink );
        pClipDoc = xTransClip.get();

        ScDrawLayer::SetGlobalDrawPersist(NULL);
    }

    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;
    SCCOL nClipSizeX;
    SCROW nClipSizeY;
    pClipDoc->GetClipArea( nClipSizeX, nClipSizeY, true );      // size in clipboard doc

    //  size in target doc: include filtered rows only if CutMode is set
    SCCOL nDestSizeX;
    SCROW nDestSizeY;
    pClipDoc->GetClipArea( nDestSizeX, nDestSizeY, bIncludeFiltered );

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ::svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    const bool bRecord(pDoc->IsUndoEnabled());

    ScDocShellModificator aModificator( *pDocSh );

    ScRange aMarkRange;
    ScMarkData aFilteredMark( rMark);   // local copy for all modifications
    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aMarkRange, aFilteredMark);
    bool bMarkIsFiltered = (eMarkType == SC_MARK_SIMPLE_FILTERED);
    bool bNoPaste = ((eMarkType != SC_MARK_SIMPLE && !bMarkIsFiltered) ||
            (bMarkIsFiltered && (eMoveMode != INS_NONE || bAsLink)));

    if (!bNoPaste)
    {
        if (!rMark.IsMarked())
        {
            // Create a selection with clipboard row count and check that for
            // filtered.
            nStartCol = GetViewData()->GetCurX();
            nStartRow = GetViewData()->GetCurY();
            nStartTab = GetViewData()->GetTabNo();
            nEndCol = nStartCol + nDestSizeX;
            nEndRow = nStartRow + nDestSizeY;
            nEndTab = nStartTab;
            aMarkRange = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
            if (ScViewUtil::HasFiltered( aMarkRange, pDoc))
            {
                bMarkIsFiltered = true;
                // Fit to clipboard's row count unfiltered rows. If there is no
                // fit assume that pasting is not possible. Note that nDestSizeY is
                // size-1 (difference).
                if (!ScViewUtil::FitToUnfilteredRows( aMarkRange, pDoc, nDestSizeY+1))
                    bNoPaste = true;
            }
            aFilteredMark.SetMarkArea( aMarkRange);
        }
        else
        {
            // Expand the marked area when the destination area is larger than the
            // current selection, to get the undo do the right thing. (i#106711)
            ScRange aRange;
            aFilteredMark.GetMarkArea( aRange );
            if( (aRange.aEnd.Col() - aRange.aStart.Col()) < nDestSizeX )
            {
                aRange.aEnd.SetCol(aRange.aStart.Col() + nDestSizeX);
                aFilteredMark.SetMarkArea(aRange);
            }
        }
    }

    if (bNoPaste)
    {
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return false;
    }

    SCROW nUnfilteredRows = aMarkRange.aEnd.Row() - aMarkRange.aStart.Row() + 1;
    ScRangeList aRangeList;
    if (bMarkIsFiltered)
    {
        ScViewUtil::UnmarkFiltered( aFilteredMark, pDoc);
        aFilteredMark.FillRangeListWithMarks( &aRangeList, false);
        nUnfilteredRows = 0;
        size_t ListSize = aRangeList.size();
        for ( size_t i = 0; i < ListSize; ++i )
        {
            ScRange* p = aRangeList[i];
            nUnfilteredRows += p->aEnd.Row() - p->aStart.Row() + 1;
        }
#if 0
        /* This isn't needed but could be a desired restriction. */
        // For filtered, destination rows have to be an exact multiple of
        // source rows. Note that nDestSizeY is size-1 (difference), so
        // nDestSizeY==0 fits always.
        if ((nUnfilteredRows % (nDestSizeY+1)) != 0)
        {
            /* FIXME: this should be a more descriptive error message then. */
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
            return false;
        }
#endif
    }

    // Also for a filtered selection the area is used, for undo et al.
    if ( aFilteredMark.IsMarked() || bMarkIsFiltered )
    {
        aMarkRange.GetVars( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
        SCCOL nBlockAddX = nEndCol-nStartCol;
        SCROW nBlockAddY = nEndRow-nStartRow;

        //  Nachfrage, wenn die Selektion groesser als 1 Zeile/Spalte, aber kleiner
        //  als das Clipboard ist (dann wird ueber die Selektion hinaus eingefuegt)

        //  ClipSize is not size, but difference
        if ( ( nBlockAddX != 0 && nBlockAddX < nDestSizeX ) ||
             ( nBlockAddY != 0 && nBlockAddY < nDestSizeY ) ||
             ( bMarkIsFiltered && nUnfilteredRows < nDestSizeY+1 ) )
        {
            ScWaitCursorOff aWaitOff( GetFrameWin() );
            String aMessage = ScGlobal::GetRscString( STR_PASTE_BIGGER );
            QueryBox aBox( GetViewData()->GetDialogParent(),
                            WinBits(WB_YES_NO | WB_DEF_NO), aMessage );
            if ( aBox.Execute() != RET_YES )
            {
                return false;
            }
        }

        if (nBlockAddX <= nDestSizeX)
            nEndCol = nStartCol + nDestSizeX;

        if (nBlockAddY <= nDestSizeY)
        {
            nEndRow = nStartRow + nDestSizeY;
            if (bMarkIsFiltered || nEndRow > aMarkRange.aEnd.Row())
            {
                // Same as above if nothing was marked: re-fit selection to
                // unfiltered rows. Extending the selection actually may
                // introduce filtered rows where there weren't any before, so
                // we also need to test for that.
                aMarkRange = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
                if (bMarkIsFiltered || ScViewUtil::HasFiltered( aMarkRange, pDoc))
                {
                    bMarkIsFiltered = true;
                    // Worst case: all rows up to the end of the sheet are filtered.
                    if (!ScViewUtil::FitToUnfilteredRows( aMarkRange, pDoc, nDestSizeY+1))
                    {
                        ErrorMessage(STR_PASTE_FULL);
                        return false;
                    }
                }
                aMarkRange.GetVars( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
                aFilteredMark.SetMarkArea( aMarkRange);
                if (bMarkIsFiltered)
                {
                    ScViewUtil::UnmarkFiltered( aFilteredMark, pDoc);
                    aFilteredMark.FillRangeListWithMarks( &aRangeList, true);
                }
            }
        }
    }
    else
    {
        nStartCol = GetViewData()->GetCurX();
        nStartRow = GetViewData()->GetCurY();
        nStartTab = GetViewData()->GetTabNo();
        nEndCol = nStartCol + nDestSizeX;
        nEndRow = nStartRow + nDestSizeY;
        nEndTab = nStartTab;
    }

    bool bOffLimits = !ValidCol(nEndCol) || !ValidRow(nEndRow);

    //  Zielbereich, wie er angezeigt wird:
    ScRange aUserRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab );

    //  Sollen Zellen eingefuegt werden?
    //  (zu grosse nEndCol/nEndRow werden weiter unten erkannt)
    bool bInsertCells = ( eMoveMode != INS_NONE && !bOffLimits );
    if ( bInsertCells )
    {
        //  Instead of EnterListAction, the paste undo action is merged into the
        //  insert action, so Repeat can insert the right cells

        MarkRange( aUserRange );            // wird vor CopyFromClip sowieso gesetzt

        // CutMode is reset on insertion of cols/rows but needed again on cell move
        bool bCut = pClipDoc->IsCutMode();
        if (!InsertCells( eMoveMode, bRecord, true ))   // is inserting possible?
        {
            return false;
            //  #i21036# EnterListAction isn't used, and InsertCells doesn't insert
            //  its undo action on failure, so no undo handling is needed here
        }
        if ( bCut )
            pClipDoc->SetCutMode( bCut );
    }
    else if (!bOffLimits)
    {
        bool bAskIfNotEmpty = bAllowDialogs &&
                                ( nFlags & IDF_CONTENTS ) &&
                                nFunction == PASTE_NOFUNC &&
                                SC_MOD()->GetInputOptions().GetReplaceCellsWarn();
        if ( bAskIfNotEmpty )
        {
            ScRangeList aTestRanges;
            aTestRanges.Append(aUserRange);
            if (!checkDestRangeForOverwrite(aTestRanges, pDoc, aFilteredMark, GetViewData()->GetDialogParent()))
                return false;
        }
    }

    SCCOL nClipStartX;                      // Clipboard-Bereich erweitern
    SCROW nClipStartY;
    pClipDoc->GetClipStart( nClipStartX, nClipStartY );
    SCCOL nUndoEndCol = nClipStartX + nClipSizeX;
    SCROW nUndoEndRow = nClipStartY + nClipSizeY;   // end of source area in clipboard document
    bool bClipOver = false;
    // #i68690# ExtendMerge for the clip doc must be called with the clipboard's sheet numbers.
    // The same end column/row can be used for all calls because the clip doc doesn't contain
    // content outside the clip area.
    for (SCTAB nClipTab=0; nClipTab < pClipDoc->GetTableCount(); nClipTab++)
        if ( pClipDoc->HasTable(nClipTab) )
            if ( pClipDoc->ExtendMerge( nClipStartX,nClipStartY, nUndoEndCol,nUndoEndRow, nClipTab, false ) )
                bClipOver = true;
    nUndoEndCol -= nClipStartX + nClipSizeX;
    nUndoEndRow -= nClipStartY + nClipSizeY;        // now contains only the difference added by ExtendMerge
    nUndoEndCol = sal::static_int_cast<SCCOL>( nUndoEndCol + nEndCol );
    nUndoEndRow = sal::static_int_cast<SCROW>( nUndoEndRow + nEndRow ); // destination area, expanded for merged cells

    if (nUndoEndCol>MAXCOL || nUndoEndRow>MAXROW)
    {
        ErrorMessage(STR_PASTE_FULL);
        return false;
    }

    pDoc->ExtendMergeSel( nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, aFilteredMark, false );

        //  Test auf Zellschutz

    ScEditableTester aTester( pDoc, nStartTab, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }

        //! Test auf Ueberlappung
        //! nur wirkliche Schnittmenge testen !!!!!!!

    ScDocFunc& rDocFunc = pDocSh->GetDocFunc();
    if ( bRecord )
    {
        String aUndo = ScGlobal::GetRscString( pClipDoc->IsCutMode() ? STR_UNDO_MOVE : STR_UNDO_COPY );
        pUndoMgr->EnterListAction( aUndo, aUndo );
    }

    if (bClipOver)
        if (lcl_SelHasAttrib( pDoc, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, aFilteredMark, HASATTR_OVERLAPPED ))
        {       // "Cell merge not possible if cells already merged"
            ScDocAttrIterator aIter( pDoc, nStartTab, nStartCol, nStartRow, nUndoEndCol, nUndoEndRow );
            const ScPatternAttr* pPattern = NULL;
            const ScMergeAttr* pMergeFlag = NULL;
            const ScMergeFlagAttr* pMergeFlagAttr = NULL;
            SCCOL nCol = -1;
            SCROW nRow1 = -1;
            SCROW nRow2 = -1;
            while ( ( pPattern = aIter.GetNext( nCol, nRow1, nRow2 ) ) != NULL )
            {
                pMergeFlag = (const ScMergeAttr*) &pPattern->GetItem(ATTR_MERGE);
                pMergeFlagAttr = (const ScMergeFlagAttr*) &pPattern->GetItem(ATTR_MERGE_FLAG);
                if( ( pMergeFlag && pMergeFlag->IsMerged() ) || ( pMergeFlagAttr && pMergeFlagAttr->IsOverlapped() ) )
                {
                    ScRange aRange(nCol, nRow1, nStartTab);
                    pDoc->ExtendOverlapped(aRange);
                    pDoc->ExtendMerge(aRange, true);
                    rDocFunc.UnmergeCells(aRange, bRecord);
                }
            }
        }

    if ( !bCutMode )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();   // kein CutMode mehr
    }

    bool bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
    bool bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );

    ScDocument* pUndoDoc    = NULL;
    ScDocument* pRefUndoDoc = NULL;
    ScDocument* pRedoDoc    = NULL;
    ScRefUndoData* pUndoData = NULL;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndoSelected( pDoc, aFilteredMark, bColInfo, bRowInfo );

        // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        SCTAB nTabCount = pDoc->GetTableCount();
        pDoc->CopyToDocument( nStartCol, nStartRow, 0, nUndoEndCol, nUndoEndRow, nTabCount-1,
                                nUndoFlags, false, pUndoDoc );

        if ( bCutMode )
        {
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, false, false );

            pUndoData = new ScRefUndoData( pDoc );
        }
    }

    sal_uInt16 nExtFlags = 0;
    pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab,
                                       nEndCol,   nEndRow,   nEndTab );     // content before the change

    if (GetViewData()->IsActive())
    {
        DoneBlockMode();
        InitOwnBlockMode();
    }
    rMark.SetMarkArea( aUserRange );
    MarkDataChanged();

        //
        //  Aus Clipboard kopieren,
        //  wenn gerechnet werden soll, Originaldaten merken
        //

    ScDocument* pMixDoc = NULL;
    if ( bSkipEmpty || nFunction )
    {
        if ( nFlags & IDF_CONTENTS )
        {
            pMixDoc = new ScDocument( SCDOCMODE_UNDO );
            pMixDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    IDF_CONTENTS, false, pMixDoc );
        }
    }

    /*  Make draw layer and start drawing undo.
        - Needed before AdjustBlockHeight to track moved drawing objects.
        - Needed before pDoc->CopyFromClip to track inserted note caption objects.
     */
    if ( bPasteDraw )
        pDocSh->MakeDrawLayer();
    if ( bRecord )
        pDoc->BeginDrawUndo();

    sal_uInt16 nNoObjFlags = nFlags & ~IDF_OBJECTS;
    if (!bAsLink)
    {
        //  copy normally (original range)
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nNoObjFlags,
                pRefUndoDoc, pClipDoc, true, false, bIncludeFiltered,
                bSkipEmpty, (bMarkIsFiltered ? &aRangeList : NULL) );

        // bei Transpose Referenzen per Hand anpassen
        if ( bTranspose && bCutMode && (nFlags & IDF_CONTENTS) )
            pDoc->UpdateTranspose( aUserRange.aStart, pOrigClipDoc, aFilteredMark, pRefUndoDoc );
    }
    else if (!bTranspose)
    {
        //  copy with bAsLink=TRUE
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nNoObjFlags, pRefUndoDoc, pClipDoc,
                                true, true, bIncludeFiltered, bSkipEmpty );
    }
    else
    {
        //  alle Inhalte kopieren (im TransClipDoc stehen nur Formeln)
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nContFlags, pRefUndoDoc, pClipDoc );
    }

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        rDocFunc.UnmergeCells( aUserRange, false );

    pDoc->ExtendMergeSel( nStartCol, nStartRow, nEndCol, nEndRow, aFilteredMark, true );    // Refresh
                                                                                    // und Bereich neu

    if ( pMixDoc )              // Rechenfunktionen mit Original-Daten auszufuehren ?
    {
        pDoc->MixDocument( aUserRange, nFunction, bSkipEmpty, pMixDoc );
    }
    delete pMixDoc;

    AdjustBlockHeight();            // update row heights before pasting objects

    ::std::vector< ::rtl::OUString > aExcludedChartNames;
    SdrPage* pPage = NULL;

    if ( nFlags & IDF_OBJECTS )
    {
        ScDrawView* pScDrawView = GetScDrawView();
        SdrModel* pModel = ( pScDrawView ? pScDrawView->GetModel() : NULL );
        pPage = ( pModel ? pModel->GetPage( static_cast< sal_uInt16 >( nStartTab ) ) : NULL );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        //  Paste the drawing objects after the row heights have been updated.

        pDoc->CopyFromClip( aUserRange, aFilteredMark, IDF_OBJECTS, pRefUndoDoc, pClipDoc,
                                true, false, bIncludeFiltered );
    }


    pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab,
                                       nEndCol,   nEndRow,   nEndTab );     // content after the change


        //  ggf. Autofilter-Koepfe loeschen
    if (bCutMode)
        if (pDoc->RefreshAutoFilter( nClipStartX,nClipStartY, nClipStartX+nClipSizeX,
                                        nClipStartY+nClipSizeY, nStartTab ))
        {
            pDocSh->PostPaint(
                ScRange(nClipStartX, nClipStartY, nStartTab, nClipStartX+nClipSizeX, nClipStartY, nStartTab),
                PAINT_GRID );
        }

    //!     Block-Bereich bei RefUndoDoc weglassen !!!

    if ( bRecord )
    {
        // Redo-Daten werden erst beim ersten Undo kopiert
        // ohne RefUndoDoc muss das Redo-Doc noch nicht angelegt werden

        if (pRefUndoDoc)
        {
            pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRedoDoc->InitUndo( pDoc, nStartTab, nEndTab, bColInfo, bRowInfo );

            //      angepasste Referenzen ins Redo-Doc

            SCTAB nTabCount = pDoc->GetTableCount();
            pRedoDoc->AddUndoTab( 0, nTabCount-1 );
            pDoc->CopyUpdated( pRefUndoDoc, pRedoDoc );

            //      alte Referenzen ins Undo-Doc

            //! Tabellen selektieren ?
            pUndoDoc->AddUndoTab( 0, nTabCount-1 );
            pRefUndoDoc->DeleteArea( nStartCol, nStartRow, nEndCol, nEndRow, aFilteredMark, IDF_ALL );
            pRefUndoDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                            IDF_FORMULA, false, pUndoDoc );
            delete pRefUndoDoc;
        }

        //  DeleteUnchanged for pUndoData is in ScUndoPaste ctor,
        //  UndoData for redo is made during first undo

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;

        SfxUndoAction* pUndo = new ScUndoPaste(
            pDocSh, ScRange(nStartCol, nStartRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
            aFilteredMark, pUndoDoc, pRedoDoc, nFlags | nUndoFlags, pUndoData,
            false, &aOptions );     // false = Redo data not yet copied

        if ( bInsertCells )
        {
            //  Merge the paste undo action into the insert action.
            //  Use ScUndoWrapper so the ScUndoPaste pointer can be stored in the insert action.

            pUndoMgr->AddUndoAction( new ScUndoWrapper( pUndo ), true );
        }
        else
            pUndoMgr->AddUndoAction( pUndo );
        pUndoMgr->LeaveListAction();
    }

    sal_uInt16 nPaint = PAINT_GRID;
    if (bColInfo)
    {
        nPaint |= PAINT_TOP;
        nUndoEndCol = MAXCOL;               // nur zum Zeichnen !
    }
    if (bRowInfo)
    {
        nPaint |= PAINT_LEFT;
        nUndoEndRow = MAXROW;               // nur zum Zeichnen !
    }
    pDocSh->PostPaint(
        ScRange(nStartCol, nStartRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
        nPaint, nExtFlags);
    // AdjustBlockHeight has already been called above

    aModificator.SetDocumentModified();
    PostPasteFromClip(aUserRange, rMark);

    if ( nFlags & IDF_OBJECTS )
    {
        ScModelObj* pModelObj = ( pDocSh ? ScModelObj::getImplementation( pDocSh->GetModel() ) : NULL );
        if ( pDoc && pPage && pModelObj )
        {
            bool bSameDoc = ( rClipParam.getSourceDocID() == pDoc->GetDocumentID() );
            const ScRangeListVector& rProtectedChartRangesVector( rClipParam.maProtectedChartRangesVector );
            ScChartHelper::CreateProtectedChartListenersAndNotify( pDoc, pPage, pModelObj, nStartTab,
                rProtectedChartRangesVector, aExcludedChartNames, bSameDoc );
        }
    }

    return true;
}

bool ScViewFunc::PasteMultiRangesFromClip(
    sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
    bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
    InsCellCmd eMoveMode, sal_uInt16 nUndoFlags)
{
    ScViewData& rViewData = *GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScMarkData aMark(rViewData.GetMarkData());
    const ScAddress& rCurPos = rViewData.GetCurPos();
    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    SCCOL nColSize = rClipParam.getPasteColSize();
    SCROW nRowSize = rClipParam.getPasteRowSize();

    if (bTranspose)
    {
        if (static_cast<SCROW>(rCurPos.Col()) + nRowSize-1 > static_cast<SCROW>(MAXCOL))
        {
            ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDocument> pTransClip(new ScDocument(SCDOCMODE_CLIP));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        pClipDoc->TransposeClip(pTransClip.get(), nFlags, bAsLink);
        pClipDoc = pTransClip.release();
        SCCOL nTempColSize = nColSize;
        nColSize = static_cast<SCCOL>(nRowSize);
        nRowSize = static_cast<SCROW>(nTempColSize);
    }

    if (!ValidCol(rCurPos.Col()+nColSize-1) || !ValidRow(rCurPos.Row()+nRowSize-1))
    {
        ErrorMessage(STR_PASTE_FULL);
        return false;
    }

    // Determine the first and last selected sheet numbers.
    SCTAB nTab1 = aMark.GetFirstSelected();
    SCTAB nTab2 = aMark.GetLastSelected();

    ScDocShellModificator aModificator(*pDocSh);

    // For multi-selection paste, we don't support cell duplication for larger
    // destination range.  In case the destination is marked, we reset it to
    // the clip size.
    ScRange aMarkedRange(rCurPos.Col(), rCurPos.Row(), nTab1,
                         rCurPos.Col()+nColSize-1, rCurPos.Row()+nRowSize-1, nTab2);

    // Extend the marked range to account for filtered rows in the destination
    // area.
    if (ScViewUtil::HasFiltered(aMarkedRange, pDoc))
    {
        if (!ScViewUtil::FitToUnfilteredRows(aMarkedRange, pDoc, nRowSize))
            return false;
    }

    bool bAskIfNotEmpty =
        bAllowDialogs && (nFlags & IDF_CONTENTS) &&
        nFunction == PASTE_NOFUNC && SC_MOD()->GetInputOptions().GetReplaceCellsWarn();

    if (bAskIfNotEmpty)
    {
        ScRangeList aTestRanges;
        aTestRanges.Append(aMarkedRange);
        if (!checkDestRangeForOverwrite(aTestRanges, pDoc, aMark, rViewData.GetDialogParent()))
            return false;
    }

    aMark.SetMarkArea(aMarkedRange);
    MarkRange(aMarkedRange);

    bool bInsertCells = (eMoveMode != INS_NONE);
    if (bInsertCells)
    {
        if (!InsertCells(eMoveMode, pDoc->IsUndoEnabled(), true))
            return false;
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDocument> pUndoDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pDoc->IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndoSelected(pDoc, aMark, false, false);
        pDoc->CopyToDocument(aMarkedRange, nUndoFlags, false, pUndoDoc.get(), &aMark, true);
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<ScDocument> pMixDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( bSkipEmpty || nFunction )
    {
        if ( nFlags & IDF_CONTENTS )
        {
            pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pMixDoc->InitUndoSelected(pDoc, aMark, false, false);
            pDoc->CopyToDocument(aMarkedRange, IDF_CONTENTS, false, pMixDoc.get(), &aMark, true);
        }
    }

    /*  Make draw layer and start drawing undo.
        - Needed before AdjustBlockHeight to track moved drawing objects.
        - Needed before pDoc->CopyFromClip to track inserted note caption objects.
     */
    if (nFlags & IDF_OBJECTS)
        pDocSh->MakeDrawLayer();
    if (pDoc->IsUndoEnabled())
        pDoc->BeginDrawUndo();

    sal_uInt16 nNoObjFlags = nFlags & ~IDF_OBJECTS;
    pDoc->CopyMultiRangeFromClip(rCurPos, aMark, nNoObjFlags, pClipDoc,
                                 true, bAsLink, false, bSkipEmpty);

    if (pMixDoc.get())
        pDoc->MixDocument(aMarkedRange, nFunction, bSkipEmpty, pMixDoc.get());

    AdjustBlockHeight();            // update row heights before pasting objects

    if (nFlags & IDF_OBJECTS)
    {
        //  Paste the drawing objects after the row heights have been updated.
        pDoc->CopyMultiRangeFromClip(rCurPos, aMark, IDF_OBJECTS, pClipDoc,
                                     true, false, false, true);
    }

    ScRange aTmp = aMarkedRange;
    aTmp.aStart.SetTab(nTab1);
    aTmp.aEnd.SetTab(nTab1);
    pDocSh->PostPaint(aTmp, PAINT_GRID);

    if (pDoc->IsUndoEnabled())
    {
        ::svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        String aUndo = ScGlobal::GetRscString(
            pClipDoc->IsCutMode() ? STR_UNDO_CUT : STR_UNDO_COPY);
        pUndoMgr->EnterListAction(aUndo, aUndo);

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;

        ScUndoPaste* pUndo = new ScUndoPaste(pDocSh,
            aMarkedRange, aMark, pUndoDoc.release(), NULL, nFlags|nUndoFlags, NULL, false, &aOptions);

        if (bInsertCells)
            pUndoMgr->AddUndoAction(new ScUndoWrapper(pUndo), true);
        else
            pUndoMgr->AddUndoAction(pUndo, false);

        pUndoMgr->LeaveListAction();
    }
    aModificator.SetDocumentModified();
    PostPasteFromClip(aMarkedRange, aMark);
    return true;
}

bool ScViewFunc::PasteFromClipToMultiRanges(
    sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
    bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
    InsCellCmd eMoveMode, sal_uInt16 nUndoFlags )
{
    if (bTranspose)
    {
        // We don't allow transpose for this yet.
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return false;
    }

    if (eMoveMode != INS_NONE)
    {
        // We don't allow insertion mode either.  Too complicated.
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return false;
    }

    ScViewData& rViewData = *GetViewData();
    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    if (rClipParam.mbCutMode)
    {
        // No cut and paste with this, please.
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return false;
    }

    const ScAddress& rCurPos = rViewData.GetCurPos();
    ScDocument* pDoc = rViewData.GetDocument();

    ScRange aSrcRange = rClipParam.getWholeRange();
    SCROW nRowSize = aSrcRange.aEnd.Row() - aSrcRange.aStart.Row() + 1;
    SCCOL nColSize = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;

    if (!ValidCol(rCurPos.Col()+nColSize-1) || !ValidRow(rCurPos.Row()+nRowSize-1))
    {
        ErrorMessage(STR_PASTE_FULL);
        return false;
    }

    ScMarkData aMark(rViewData.GetMarkData());

    ScRangeList aRanges;
    aMark.MarkToSimple();
    aMark.FillRangeListWithMarks(&aRanges, false);
    if (!ScClipUtil::CheckDestRanges(pDoc, nColSize, nRowSize, aMark, aRanges))
    {
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return false;
    }

    ScDocShell* pDocSh = rViewData.GetDocShell();

    ScDocShellModificator aModificator(*pDocSh);

    bool bAskIfNotEmpty =
        bAllowDialogs && (nFlags & IDF_CONTENTS) &&
        nFunction == PASTE_NOFUNC && SC_MOD()->GetInputOptions().GetReplaceCellsWarn();

    if (bAskIfNotEmpty)
    {
        if (!checkDestRangeForOverwrite(aRanges, pDoc, aMark, rViewData.GetDialogParent()))
            return false;
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pUndoDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pDoc->IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndoSelected(pDoc, aMark, false, false);
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
        {
            pDoc->CopyToDocument(
                *aRanges[i], nUndoFlags, false, pUndoDoc.get(), &aMark, true);
        }
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr<ScDocument> pMixDoc;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (bSkipEmpty || nFunction)
    {
        if (nFlags & IDF_CONTENTS)
        {
            pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pMixDoc->InitUndoSelected(pDoc, aMark, false, false);
            for (size_t i = 0, n = aRanges.size(); i < n; ++i)
            {
                pDoc->CopyToDocument(
                    *aRanges[i], IDF_CONTENTS, false, pMixDoc.get(), &aMark, true);
            }
        }
    }

    if (nFlags & IDF_OBJECTS)
        pDocSh->MakeDrawLayer();
    if (pDoc->IsUndoEnabled())
        pDoc->BeginDrawUndo();

    // First, paste everything but the drawing objects.
    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        pDoc->CopyFromClip(
            *aRanges[i], aMark, (nFlags & ~IDF_OBJECTS), NULL, pClipDoc,
            false, false, true, bSkipEmpty, NULL);
    }

    if (pMixDoc.get())
    {
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
            pDoc->MixDocument(*aRanges[i], nFunction, bSkipEmpty, pMixDoc.get());
    }

    AdjustBlockHeight();            // update row heights before pasting objects

    // Then paste the objects.
    if (nFlags & IDF_OBJECTS)
    {
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
        {
            pDoc->CopyFromClip(
                *aRanges[i], aMark, IDF_OBJECTS, NULL, pClipDoc,
                false, false, true, bSkipEmpty, NULL);
        }
    }

    // Refresh the range that includes all pasted ranges.  We only need to
    // refresh the current sheet.
    pDocSh->PostPaint(aRanges, PAINT_GRID);

    if (pDoc->IsUndoEnabled())
    {
        svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        String aUndo = ScGlobal::GetRscString(
            pClipDoc->IsCutMode() ? STR_UNDO_CUT : STR_UNDO_COPY);
        pUndoMgr->EnterListAction(aUndo, aUndo);

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;

        ScUndoPaste* pUndo = new ScUndoPaste(
            pDocSh, aRanges, aMark, pUndoDoc.release(), NULL, nFlags|nUndoFlags, NULL, false, &aOptions);

        pUndoMgr->AddUndoAction(pUndo, false);
        pUndoMgr->LeaveListAction();
    }
    aModificator.SetDocumentModified();
    PostPasteFromClip(aRanges, aMark);

    return false;
}

void ScViewFunc::PostPasteFromClip(const ScRangeList& rPasteRanges, const ScMarkData& rMark)
{
    ScViewData* pViewData = GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    pDocSh->UpdateOle(pViewData);

    SelectionChanged();

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
    if (!pModelObj || !pModelObj->HasChangesListeners())
        return;

    ScRangeList aChangeRanges;
    for (size_t i = 0, n = rPasteRanges.size(); i < n; ++i)
    {
        const ScRange& r = *rPasteRanges[i];
        ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
        for (; itr != itrEnd; ++itr)
        {
            ScRange aChangeRange(r);
            aChangeRange.aStart.SetTab(*itr);
            aChangeRange.aEnd.SetTab(*itr);
            aChangeRanges.Append(aChangeRange);
        }
    }
    pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
}


//----------------------------------------------------------------------------
//      D R A G   A N D   D R O P
//
//  innerhalb des Dokuments

sal_Bool ScViewFunc::MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                sal_Bool bCut, sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    HideAllCursors();       // wegen zusammengefassten

    sal_Bool bSuccess = sal_True;
    SCTAB nDestTab = rDestPos.Tab();
    const ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rSource.aStart.Tab() == nDestTab && rSource.aEnd.Tab() == nDestTab && rMark.GetSelectCount() > 1 )
    {
        //  moving within one table and several tables selected -> apply to all selected tables

        if ( bRecord )
        {
            String aUndo = ScGlobal::GetRscString( bCut ? STR_UNDO_MOVE : STR_UNDO_COPY );
            pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
        }

        //  collect ranges of consecutive selected tables

        ScRange aLocalSource = rSource;
        ScAddress aLocalDest = rDestPos;
        SCTAB nTabCount = pDocSh->GetDocument()->GetTableCount();
        SCTAB nStartTab = 0;
        while ( nStartTab < nTabCount && bSuccess )
        {
            while ( nStartTab < nTabCount && !rMark.GetTableSelect(nStartTab) )
                ++nStartTab;
            if ( nStartTab < nTabCount )
            {
                SCTAB nEndTab = nStartTab;
                while ( nEndTab+1 < nTabCount && rMark.GetTableSelect(nEndTab+1) )
                    ++nEndTab;

                aLocalSource.aStart.SetTab( nStartTab );
                aLocalSource.aEnd.SetTab( nEndTab );
                aLocalDest.SetTab( nStartTab );

                bSuccess = pDocSh->GetDocFunc().MoveBlock(
                                aLocalSource, aLocalDest, bCut, bRecord, bPaint, bApi );

                nStartTab = nEndTab + 1;
            }
        }

        if ( bRecord )
            pDocSh->GetUndoManager()->LeaveListAction();
    }
    else
    {
        //  move the block as specified
        bSuccess = pDocSh->GetDocFunc().MoveBlock(
                                rSource, rDestPos, bCut, bRecord, bPaint, bApi );
    }

    ShowAllCursors();
    if (bSuccess)
    {
        //  Zielbereich markieren
        ScAddress aDestEnd(
                    rDestPos.Col() + rSource.aEnd.Col() - rSource.aStart.Col(),
                    rDestPos.Row() + rSource.aEnd.Row() - rSource.aStart.Row(),
                    nDestTab );

        sal_Bool bIncludeFiltered = bCut;
        if ( !bIncludeFiltered )
        {
            // find number of non-filtered rows
            SCROW nPastedCount = pDocSh->GetDocument()->CountNonFilteredRows(
                rSource.aStart.Row(), rSource.aEnd.Row(), rSource.aStart.Tab());

            if ( nPastedCount == 0 )
                nPastedCount = 1;
            aDestEnd.SetRow( rDestPos.Row() + nPastedCount - 1 );
        }

        MarkRange( ScRange( rDestPos, aDestEnd ), false );          //! sal_False ???

        pDocSh->UpdateOle(GetViewData());
        SelectionChanged();
    }
    return bSuccess;
}

//  Link innerhalb des Dokuments

sal_Bool ScViewFunc::LinkBlock( const ScRange& rSource, const ScAddress& rDestPos, sal_Bool bApi )
{
    //  Test auf Ueberlappung

    if ( rSource.aStart.Tab() == rDestPos.Tab() )
    {
        SCCOL nDestEndCol = rDestPos.Col() + ( rSource.aEnd.Col() - rSource.aStart.Col() );
        SCROW nDestEndRow = rDestPos.Row() + ( rSource.aEnd.Row() - rSource.aStart.Row() );

        if ( rSource.aStart.Col() <= nDestEndCol && rDestPos.Col() <= rSource.aEnd.Col() &&
             rSource.aStart.Row() <= nDestEndRow && rDestPos.Row() <= rSource.aEnd.Row() )
        {
            if (!bApi)
                ErrorMessage( STR_ERR_LINKOVERLAP );
            return false;
        }
    }

    //  Ausfuehren per Paste

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );
    pDoc->CopyTabToClip( rSource.aStart.Col(), rSource.aStart.Row(),
                            rSource.aEnd.Col(), rSource.aEnd.Row(),
                            rSource.aStart.Tab(), pClipDoc );

    //  Zielbereich markieren (Cursor setzen, keine Markierung)

    if ( GetViewData()->GetTabNo() != rDestPos.Tab() )
        SetTabNo( rDestPos.Tab() );

    MoveCursorAbs( rDestPos.Col(), rDestPos.Row(), SC_FOLLOW_NONE, false, false );

    //  Paste

    PasteFromClip( IDF_ALL, pClipDoc, PASTE_NOFUNC, false, false, sal_True );       // als Link

    delete pClipDoc;

    return sal_True;
}

void ScViewFunc::DataFormPutData( SCROW nCurrentRow ,
                                  SCROW nStartRow , SCCOL nStartCol ,
                                  SCROW nEndRow , SCCOL nEndCol ,
                                  boost::ptr_vector<boost::nullable<Edit> >& aEdits,
                                  sal_uInt16 aColLength )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScDocShellModificator aModificator( *pDocSh );
    ::svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    if ( pDoc )
    {
        const sal_Bool bRecord( pDoc->IsUndoEnabled());
        ScDocument* pUndoDoc = NULL;
        ScDocument* pRedoDoc = NULL;
        ScRefUndoData* pUndoData = NULL;
        SCTAB nTab = GetViewData()->GetTabNo();
        SCTAB nStartTab = nTab;
        SCTAB nEndTab = nTab;

        {
                ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
                if ( pChangeTrack )
                        pChangeTrack->ResetLastCut();   // kein CutMode mehr
        }
        ScRange aUserRange( nStartCol, nCurrentRow, nStartTab, nEndCol, nCurrentRow, nEndTab );
        sal_Bool bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
        sal_Bool bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );
        SCCOL nUndoEndCol = nStartCol+aColLength-1;
        SCROW nUndoEndRow = nCurrentRow;
        sal_uInt16 nUndoFlags = IDF_NONE;

        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndoSelected( pDoc , rMark , bColInfo , bRowInfo );
            pDoc->CopyToDocument( aUserRange , 1 , false , pUndoDoc );
        }
        sal_uInt16 nExtFlags = 0;
        pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab , nEndCol, nEndRow, nEndTab ); // content before the change
        pDoc->BeginDrawUndo();

        for(sal_uInt16 i = 0; i < aColLength; i++)
        {
            if (!aEdits.is_null(i))
            {
                String  aFieldName=aEdits[i].GetText();
                pDoc->SetString( nStartCol + i, nCurrentRow, nTab, aFieldName );
            }
        }
        pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nCurrentRow, nStartTab, nEndCol, nCurrentRow, nEndTab );  // content after the change
        SfxUndoAction* pUndo = new ScUndoDataForm( pDocSh,
                                                                nStartCol, nCurrentRow, nStartTab,
                                                                nUndoEndCol, nUndoEndRow, nEndTab, rMark,
                                                                pUndoDoc, pRedoDoc, nUndoFlags,
                                                                pUndoData, NULL, NULL, NULL,
                                                                false );           // FALSE = Redo data not yet copied
        pUndoMgr->AddUndoAction( new ScUndoWrapper( pUndo ), true );

        sal_uInt16 nPaint = PAINT_GRID;
        if (bColInfo)
        {
                nPaint |= PAINT_TOP;
                nUndoEndCol = MAXCOL;                           // nur zum Zeichnen !
        }
        if (bRowInfo)
        {
                nPaint |= PAINT_LEFT;
                nUndoEndRow = MAXROW;                           // nur zum Zeichnen !
        }

        pDocSh->PostPaint(
            ScRange(nStartCol, nCurrentRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
            nPaint, nExtFlags);
        pDocSh->UpdateOle(GetViewData());
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
