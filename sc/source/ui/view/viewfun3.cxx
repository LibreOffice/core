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



//----------------------------------------------------------------------------

#define _SV_NOXSOUND

#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
//#define _PRNDLG_HXX
//#define _POLY_HXX
#define _PVRWIN_HXX
//#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _SETBRW_HXX
//#define _STACK_HXX
//#define _STATUS_HXX ***
#define _STDCTRL_HXX
#define _STDMENU_HXX
//#define _TAB_HXX
#define _TABBAR_HXX
#define _TREELIST_HXX
#define _VALUESET_HXX
#define _VCATTR_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

//#define _SELENG_HXX
//#define _SOUND_HXX
//#define _SYSDLG_HXX




#define _PASSWD_HXX

#define _SFX_DOCFILE_HXX
//#define _SFX_DOCFILT_HXX
#define _SFX_DOCINF_HXX
#define _SFX_DOCSH_HXX
//#define _SFXDOCFILT_HXX
//#define _SFXDOCINF_HXX
//#define _SFXDOCSH_HXX
#define _SFX_PRNMON_HXX
#define _SFX_RESMGR_HXX
#define _SFX_TEMPLDLG_HXX
//#define _SFXAPPWIN_HXX
#define _SFXBASIC_HXX
#define _SFXCTRLITEM
#define _SFXDLGCFG_HXX
//#define _SFXDISPATCH_HXX
#define _SFXFILEDLG_HXX
//#define _SFXIMGMGR_HXX
#define _SFXIPFRM_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMULTISEL_HXX
//#define _SFXMSG_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFX_MINFITEM_HXX
#define _SFXOBJFACE_HXX
#define _SFXOBJFAC_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXSTBITEM_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXCTRL_HXX
#define _SFXTBXMGR_HXX

#define _SI_HXX
//#define _SI_DLL_HXX
//#define _SIDLL_HXX
//#define _SI_NOITEMS
//#define _SI_NOOTHERFORMS
//#define _SI_NOSBXCONTROLS
//#define _SINOSBXCONTROLS
//#define _SI_NODRW
//#define _SI_NOCONTROL

#define _SVBOXITM_HXX
#define _SVCONTNR_HXX     //

#define _SDR_NOTRANSFORM

#define _SVDRAG_HXX
#define _SVINCVW_HXX
//#define _SV_MULTISEL_HXX
#define _SVRTV_HXX
#define _SVTABBX_HXX
#define _SVTREEBOX_HXX
#define _SVTREELIST_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_LAYCTRL_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SELCTRL_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_STDDLG_HXX
#define _SVX_THESDLG_HXX

// INCLUDE -------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/dbexch.hrc>
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
#include "dbcolect.hxx"
#include "impex.hxx"            // Sylk-ID fuer CB
#include "chgtrack.hxx"
#include "waitoff.hxx"
#include "scmod.hxx"
#include "sc.hrc"
#include "inputopt.hxx"
#include "warnbox.hxx"
#include "drwlayer.hxx"
#include "editable.hxx"
#include "transobj.hxx"
#include "drwtrans.hxx"
#include "docuno.hxx"
#include "clipparam.hxx"
#include "drawview.hxx"
#include "chartlis.hxx"
#include "charthelper.hxx"


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

        CopyToClip( pClipDoc, sal_True, sal_False, bIncludeObjects );           // Ab ins Clipboard

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
            pDoc->CopyToDocument( aCopyRange, (IDF_ALL & ~IDF_OBJECTS) | IDF_NOCAPTIONS, sal_False, pUndoDoc );
            pDoc->BeginDrawUndo();
        }

        sal_uInt16 nExtFlags = 0;
        pDocSh->UpdatePaintExt( nExtFlags, aRange );

        HideCursor();                           // Cursor aendert sich !

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
        ShowCursor();                           // Cursor aendert sich !
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
    sal_Bool bDone = sal_False;
    if ( bStopEdit )
        UpdateInputLine();

    ScRange aRange;
    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aRange );
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
        if ( !pDoc->HasSelectedBlockMatrixFragment(
                        aRange.aStart.Col(), aRange.aStart.Row(),
                        aRange.aEnd.Col(),   aRange.aEnd.Row(),
                        rMark ) )
        {
            sal_Bool bSysClip = sal_False;
            if ( !pClipDoc )                                    // no clip doc specified
            {
                pClipDoc = new ScDocument( SCDOCMODE_CLIP );    // create one (deleted by ScTransferObj)
                bSysClip = sal_True;                                // and copy into system
            }

            if ( !bCut )
            {
                ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
                if ( pChangeTrack )
                    pChangeTrack->ResetLastCut();   // kein CutMode mehr
            }

            if ( bSysClip && bIncludeObjects )
            {
                sal_Bool bAnyOle = pDoc->HasOLEObjectsInArea( aRange, &rMark );
                // update ScGlobal::pDrawClipDocShellRef
                ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
            }

            ScClipParam aClipParam(aRange, bCut);
            aClipParam.setSourceDocID( pDoc->GetDocumentID() );
            pDoc->CopyToClip(aClipParam, pClipDoc, &rMark, false, false, bIncludeObjects);

            if ( pDoc && pClipDoc )
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

            if (bSysClip)
            {
                ScDrawLayer::SetGlobalDrawPersist(NULL);

                ScGlobal::SetClipDocName( pDoc->GetDocumentShell()->GetTitle( SFX_TITLE_FULLNAME ) );
            }
            pClipDoc->ExtendMerge( aRange, sal_True );

            if (bSysClip)
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
                    pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
                }

                pTransferObj->CopyToClipboard( GetActiveWin() );    // system clipboard
                SC_MOD()->SetClipObject( pTransferObj, NULL );      // internal clipboard
            }

            bDone = sal_True;
        }
        else
        {
            if (!bApi)
                ErrorMessage(STR_MATRIXFRAGMENTERR);
        }
    }
    else if (eMarkType == SC_MARK_MULTI)
    {
        bool bSuccess = false;
        ScClipParam aClipParam;
        aClipParam.mbCutMode = false;
        rMark.MarkToSimple();
        rMark.FillRangeListWithMarks(&aClipParam.maRanges, false);

        do
        {
            if (bCut)
                // We con't support cutting of multi-selections.
                break;

            if (pClipDoc)
                // TODO: What's this for?
                break;

            ::std::auto_ptr<ScDocument> pDocClip(new ScDocument(SCDOCMODE_CLIP));

            // Check for geometrical feasibility of the ranges.
            bool bValidRanges = true;
            ScRangePtr p = aClipParam.maRanges.First();
            SCCOL nPrevColDelta = 0;
            SCROW nPrevRowDelta = 0;
            SCCOL nPrevCol = p->aStart.Col();
            SCROW nPrevRow = p->aStart.Row();
            SCCOL nPrevColSize = p->aEnd.Col() - p->aStart.Col() + 1;
            SCROW nPrevRowSize = p->aEnd.Row() - p->aStart.Row() + 1;
            for (p = aClipParam.maRanges.Next(); p; p = aClipParam.maRanges.Next())
            {
                if (pDoc->HasSelectedBlockMatrixFragment(
                    p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), rMark))
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

            pDoc->CopyToClip(aClipParam, pDocClip.get(), &rMark, false, false, bIncludeObjects);

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
    else
    {
        if (!bApi)
            ErrorMessage(STR_NOMULTISELECT);
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
        PasteDraw( aPos, pDrawClip->GetModel(), sal_False,
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
        // #129384# keep a reference in case the clipboard is changed during PasteFromClip
        uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
        PasteFromClip( IDF_ALL, pOwnClip->GetDocument(),
                        PASTE_NOFUNC, sal_False, sal_False, sal_False, INS_NONE, IDF_NONE,
                        sal_True );     // allow warning dialog
    }
    else if (pDrawClip)
        PasteDraw();
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );

//      if (pClipObj.Is())
        {
            sal_uLong nBiff8 = SotExchange::RegisterFormatName(
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff8")));
            sal_uLong nBiff5 = SotExchange::RegisterFormatName(
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff5")));

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

                sal_Bool bDoRtf = sal_False;
                TransferableObjectDescriptor aObjDesc;
                if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) )
                {
                    bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                                 aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                               && aDataHelper.HasFormat( SOT_FORMAT_RTF ) );
                }
                if ( bDoRtf )
                    PasteFromSystem( FORMAT_RTF );
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
            // #89579# xxx_OLE formats come last, like in SotExchange tables
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE );
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
                PasteFromSystem( SOT_FORMATSTR_ID_LINK_SOURCE_OLE );
//          else
//              ErrorMessage(STR_PASTE_ERROR);
        }
//      else
//          ErrorMessage(STR_PASTE_ERROR);
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
                        PASTE_NOFUNC, sal_False, sal_False, sal_False, INS_NONE, IDF_NONE,
                        sal_True );     // allow warning dialog
    }
    else if (pDrawClip)
    {
        ScViewData* pViewData = GetViewData();
        SCCOL nPosX = pViewData->GetCurX();
        SCROW nPosY = pViewData->GetCurY();
        Window* pWin = GetActiveWin();
        Point aPos = pWin->PixelToLogic( pViewData->GetScrPos( nPosX, nPosY, pViewData->GetActivePart() ) );
        PasteDraw( aPos, pDrawClip->GetModel(), sal_False, pDrawClip->GetSourceDocID() == pViewData->GetDocument()->GetDocumentID() );
    }
    else
    {
            TransferableDataHelper aDataHelper( rxTransferable );
        {
            sal_uLong nBiff8 = SotExchange::RegisterFormatName(
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff8")));
            sal_uLong nBiff5 = SotExchange::RegisterFormatName(
                    String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Biff5")));
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
                sal_Bool bDoRtf = sal_False;
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
            // #89579# xxx_OLE formats come last, like in SotExchange tables
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ))
                nFormatId = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
            else if (aDataHelper.HasFormat( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ))
                nFormatId = SOT_FORMATSTR_ID_LINK_SOURCE_OLE;
            else
                return;

            PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                NULL, sal_False, sal_False );
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
        // #129384# keep a reference in case the clipboard is changed during PasteFromClip
        uno::Reference<datatransfer::XTransferable> aOwnClipRef( pOwnClip );
        PasteFromClip( IDF_ALL, pOwnClip->GetDocument(),
                        PASTE_NOFUNC, sal_False, sal_False, sal_False, INS_NONE, IDF_NONE,
                        !bApi );        // allow warning dialog
    }
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
        if ( !aDataHelper.GetTransferable().is() )
            return sal_False;

        bRet = PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                                GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                                NULL, sal_False, !bApi );       // allow warning dialog

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
    sal_Bool bRet = sal_False;
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
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        if ( rTabSelection.GetTableSelect(nTab) && pDoc->HasAttrib( nCol1, nRow1, nTab, nCol2, nRow2, nTab, nMask ) )
            return sal_True;
    return sal_False;
}

//
//      Einfuegen auf Tabelle:
//

//  internes Paste

namespace {

class CursorSwitcher
{
public:
    CursorSwitcher(ScViewFunc* pViewFunc) :
        mpViewFunc(pViewFunc)
    {
        mpViewFunc->HideCursor();
    }

    ~CursorSwitcher()
    {
        mpViewFunc->ShowCursor();
    }
private:
    ScViewFunc* mpViewFunc;
};

bool lcl_checkDestRangeForOverwrite(const ScRange& rDestRange, const ScDocument* pDoc, const ScMarkData& rMark, Window* pParentWnd)
{
    bool bIsEmpty = true;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab < nTabCount && bIsEmpty; ++nTab)
    {
        if (!rMark.GetTableSelect(nTab))
            continue;

        bIsEmpty = pDoc->IsBlockEmpty(nTab, rDestRange.aStart.Col(), rDestRange.aStart.Row(),
                                      rDestRange.aEnd.Col(), rDestRange.aEnd.Row());
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

sal_Bool ScViewFunc::PasteFromClip( sal_uInt16 nFlags, ScDocument* pClipDoc,
                                    sal_uInt16 nFunction, sal_Bool bSkipEmpty,
                                    sal_Bool bTranspose, sal_Bool bAsLink,
                                    InsCellCmd eMoveMode, sal_uInt16 nUndoExtraFlags,
                                    sal_Bool bAllowDialogs )
{
    if (!pClipDoc)
    {
        DBG_ERROR("PasteFromClip: pClipDoc=0 not allowed");
        return sal_False;
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
        return PasteMultiRangesFromClip(
            nFlags, pClipDoc, nFunction, bSkipEmpty, bTranspose, bAsLink, bAllowDialogs,
            eMoveMode, nContFlags, nUndoFlags);

    sal_Bool bCutMode = pClipDoc->IsCutMode();      // if transposing, take from original clipdoc
    sal_Bool bIncludeFiltered = bCutMode;

    // paste drawing: also if IDF_NOTE is set (to create drawing layer for note captions)
    sal_Bool bPasteDraw = ( pClipDoc->GetDrawLayer() && ( nFlags & (IDF_OBJECTS|IDF_NOTE) ) );

    ScDocShellRef aTransShellRef;   // for objects in xTransClip - must remain valid as long as xTransClip
    ScDocument* pOrigClipDoc = NULL;
    ::std::auto_ptr< ScDocument > xTransClip;
    if ( bTranspose )
    {
        SCCOL nX;
        SCROW nY;
        // include filtered rows until TransposeClip can skip them
        bIncludeFiltered = sal_True;
        pClipDoc->GetClipArea( nX, nY, sal_True );
        if ( nY > static_cast<sal_Int32>(MAXCOL) )                      // zuviele Zeilen zum Transponieren
        {
            ErrorMessage(STR_PASTE_FULL);
            return sal_False;
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
    pClipDoc->GetClipArea( nClipSizeX, nClipSizeY, sal_True );      // size in clipboard doc

    //  size in target doc: include filtered rows only if CutMode is set
    SCCOL nDestSizeX;
    SCROW nDestSizeY;
    pClipDoc->GetClipArea( nDestSizeX, nDestSizeY, bIncludeFiltered );

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ::svl::IUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    const sal_Bool bRecord(pDoc->IsUndoEnabled());

    ScDocShellModificator aModificator( *pDocSh );

    ScRange aMarkRange;
    ScMarkData aFilteredMark( rMark);   // local copy for all modifications
    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aMarkRange, aFilteredMark);
    bool bMarkIsFiltered = (eMarkType == SC_MARK_SIMPLE_FILTERED);
    bool bNoPaste = ((eMarkType != SC_MARK_SIMPLE && !bMarkIsFiltered) ||
            (bMarkIsFiltered && (eMoveMode != INS_NONE || bAsLink)));
    if (!bNoPaste && !rMark.IsMarked())
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
    if (bNoPaste)
    {
        ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);
        return sal_False;
    }

    SCROW nUnfilteredRows = aMarkRange.aEnd.Row() - aMarkRange.aStart.Row() + 1;
    ScRangeList aRangeList;
    if (bMarkIsFiltered)
    {
        ScViewUtil::UnmarkFiltered( aFilteredMark, pDoc);
        aFilteredMark.FillRangeListWithMarks( &aRangeList, sal_False);
        nUnfilteredRows = 0;
        for (ScRange* p = aRangeList.First(); p; p = aRangeList.Next())
        {
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
            return sal_False;
        }
#endif
    }

    SCCOL nMarkAddX = 0;
    SCROW nMarkAddY = 0;

    // Also for a filtered selection the area is used, for undo et al.
    if ( aFilteredMark.IsMarked() || bMarkIsFiltered )
    {
        aMarkRange.GetVars( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
        SCCOL nBlockAddX = nEndCol-nStartCol;
        SCROW nBlockAddY = nEndRow-nStartRow;

        //  #58422# Nachfrage, wenn die Selektion groesser als 1 Zeile/Spalte, aber kleiner
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
                return sal_False;
            }
        }

        if (nBlockAddX > nDestSizeX)
            nMarkAddX = nBlockAddX - nDestSizeX;            // fuer Merge-Test
        else
        {
            nEndCol = nStartCol + nDestSizeX;
            if (nEndCol > aMarkRange.aEnd.Col())
            {
                // #i113553# larger range has to be included in aFilteredMark (for undo), but extending columns can't changed the filtered status
                aMarkRange = ScRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab );
                aFilteredMark.SetMarkArea( aMarkRange );
                if (bMarkIsFiltered)
                {
                    ScViewUtil::UnmarkFiltered( aFilteredMark, pDoc );
                    aFilteredMark.FillRangeListWithMarks( &aRangeList, sal_True );
                }
            }
        }

        if (nBlockAddY > nDestSizeY)
            nMarkAddY = nBlockAddY - nDestSizeY;            // fuer Merge-Test
        else
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
                        return sal_False;
                    }
                }
                aMarkRange.GetVars( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab);
                aFilteredMark.SetMarkArea( aMarkRange);
                if (bMarkIsFiltered)
                {
                    ScViewUtil::UnmarkFiltered( aFilteredMark, pDoc);
                    aFilteredMark.FillRangeListWithMarks( &aRangeList, sal_True);
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
    sal_Bool bInsertCells = ( eMoveMode != INS_NONE && !bOffLimits );
    if ( bInsertCells )
    {
        //  #94115# Instead of EnterListAction, the paste undo action is merged into the
        //  insert action, so Repeat can insert the right cells

        MarkRange( aUserRange );            // wird vor CopyFromClip sowieso gesetzt

        // #72930# CutMode is reset on insertion of cols/rows but needed again on cell move
        sal_Bool bCut = pClipDoc->IsCutMode();
        if (!InsertCells( eMoveMode, bRecord, sal_True ))   // is inserting possible?
        {
            return sal_False;
            //  #i21036# EnterListAction isn't used, and InsertCells doesn't insert
            //  its undo action on failure, so no undo handling is needed here
        }
        if ( bCut )
            pClipDoc->SetCutMode( bCut );
    }
    else if (!bOffLimits)
    {
        sal_Bool bAskIfNotEmpty = bAllowDialogs &&
                                ( nFlags & IDF_CONTENTS ) &&
                                nFunction == PASTE_NOFUNC &&
                                SC_MOD()->GetInputOptions().GetReplaceCellsWarn();
        if ( bAskIfNotEmpty )
        {
            if (!lcl_checkDestRangeForOverwrite(aUserRange, pDoc, aFilteredMark, GetViewData()->GetDialogParent()))
                return false;
        }
    }

    SCCOL nClipStartX;                      // Clipboard-Bereich erweitern
    SCROW nClipStartY;
    pClipDoc->GetClipStart( nClipStartX, nClipStartY );
    SCCOL nUndoEndCol = nClipStartX + nClipSizeX;
    SCROW nUndoEndRow = nClipStartY + nClipSizeY;   // end of source area in clipboard document
    sal_Bool bClipOver = sal_False;
    // #i68690# ExtendMerge for the clip doc must be called with the clipboard's sheet numbers.
    // The same end column/row can be used for all calls because the clip doc doesn't contain
    // content outside the clip area.
    for (SCTAB nClipTab=0; nClipTab<=MAXTAB; nClipTab++)
        if ( pClipDoc->HasTable(nClipTab) )
            if ( pClipDoc->ExtendMerge( nClipStartX,nClipStartY, nUndoEndCol,nUndoEndRow, nClipTab, sal_False ) )
                bClipOver = sal_True;
    nUndoEndCol -= nClipStartX + nClipSizeX;
    nUndoEndRow -= nClipStartY + nClipSizeY;        // now contains only the difference added by ExtendMerge
    nUndoEndCol = sal::static_int_cast<SCCOL>( nUndoEndCol + nEndCol );
    nUndoEndRow = sal::static_int_cast<SCROW>( nUndoEndRow + nEndRow ); // destination area, expanded for merged cells

//  if (nUndoEndCol < nEndCol) nUndoEndCol = nEndCol;
//  if (nUndoEndRow < nEndRow) nUndoEndRow = nEndRow;

//  nUndoEndCol += nMarkAddX;
//  nUndoEndRow += nMarkAddY;

    if (nUndoEndCol>MAXCOL || nUndoEndRow>MAXROW)
    {
        ErrorMessage(STR_PASTE_FULL);
        return sal_False;
    }

    pDoc->ExtendMergeSel( nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, aFilteredMark, sal_False );

        //  Test auf Zellschutz

    ScEditableTester aTester( pDoc, nStartTab, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return sal_False;
    }

        //! Test auf Ueberlappung
        //! nur wirkliche Schnittmenge testen !!!!!!!

    //  pDoc->HasCommonAttr( StartCol,nStartRow, nUndoEndCol,nUndoEndRow, nStartTab,
    //                          pClipDoc, nClipStartX, nClipStartY );

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
                    pDoc->ExtendMerge(aRange, sal_True, sal_True);
                    rDocFunc.UnmergeCells(aRange, bRecord, sal_True);
                }
            }
        }

    if ( !bCutMode )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();   // kein CutMode mehr
    }

    sal_Bool bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
    sal_Bool bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );

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
                                nUndoFlags, sal_False, pUndoDoc );

        if ( bCutMode )
        {
            pRefUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1, sal_False, sal_False );

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

    HideCursor();                           // Cursor aendert sich !

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
                                    IDF_CONTENTS, sal_False, pMixDoc );
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
                pRefUndoDoc, pClipDoc, sal_True, sal_False, bIncludeFiltered,
                bSkipEmpty, (bMarkIsFiltered ? &aRangeList : NULL) );

        // bei Transpose Referenzen per Hand anpassen
        if ( bTranspose && bCutMode && (nFlags & IDF_CONTENTS) )
            pDoc->UpdateTranspose( aUserRange.aStart, pOrigClipDoc, aFilteredMark, pRefUndoDoc );
    }
    else if (!bTranspose)
    {
        //  copy with bAsLink=TRUE
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nNoObjFlags, pRefUndoDoc, pClipDoc,
                                sal_True, sal_True, bIncludeFiltered, bSkipEmpty );
    }
    else
    {
        //  alle Inhalte kopieren (im TransClipDoc stehen nur Formeln)
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nContFlags, pRefUndoDoc, pClipDoc );
    }

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        rDocFunc.UnmergeCells( aUserRange, sal_False, sal_True );

    pDoc->ExtendMergeSel( nStartCol, nStartRow, nEndCol, nEndRow, aFilteredMark, sal_True );    // Refresh
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
                                sal_True, sal_False, bIncludeFiltered );
    }

    //
    //
    //

    pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab,
                                       nEndCol,   nEndRow,   nEndTab );     // content after the change


        //  ggf. Autofilter-Koepfe loeschen
    if (bCutMode)
        if (pDoc->RefreshAutoFilter( nClipStartX,nClipStartY, nClipStartX+nClipSizeX,
                                        nClipStartY+nClipSizeY, nStartTab ))
            pDocSh->PostPaint( nClipStartX,nClipStartY,nStartTab,
                                nClipStartX+nClipSizeX,nClipStartY,nStartTab,
                                PAINT_GRID );

    ShowCursor();                           // Cursor aendert sich !

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
                                            IDF_FORMULA, sal_False, pUndoDoc );
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

        SfxUndoAction* pUndo = new ScUndoPaste( pDocSh,
                                nStartCol, nStartRow, nStartTab,
                                nUndoEndCol, nUndoEndRow, nEndTab, aFilteredMark,
                                pUndoDoc, pRedoDoc, nFlags | nUndoFlags,
                                pUndoData, NULL, NULL, NULL,
                                sal_False, &aOptions );     // sal_False = Redo data not yet copied

        if ( bInsertCells )
        {
            //  Merge the paste undo action into the insert action.
            //  Use ScUndoWrapper so the ScUndoPaste pointer can be stored in the insert action.

            pUndoMgr->AddUndoAction( new ScUndoWrapper( pUndo ), sal_True );
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
    pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                        nUndoEndCol, nUndoEndRow, nEndTab, nPaint, nExtFlags );
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

    return sal_True;
}

bool ScViewFunc::PasteMultiRangesFromClip(
    sal_uInt16 nFlags, ScDocument* pClipDoc, sal_uInt16 nFunction,
    bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
    InsCellCmd eMoveMode, sal_uInt16 /*nContFlags*/, sal_uInt16 nUndoFlags)
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

        ::std::auto_ptr<ScDocument> pTransClip(new ScDocument(SCDOCMODE_CLIP));
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
    SCTAB nTab2 = nTab1;
    for (SCTAB i = nTab1+1; i <= MAXTAB; ++i)
        if (aMark.GetTableSelect(i))
            nTab2 = i;

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
        if (!lcl_checkDestRangeForOverwrite(aMarkedRange, pDoc, aMark, rViewData.GetDialogParent()))
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

    ::std::auto_ptr<ScDocument> pUndoDoc;
    if (pDoc->IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndoSelected(pDoc, aMark, false, false);
        pDoc->CopyToDocument(aMarkedRange, nUndoFlags, false, pUndoDoc.get(), &aMark, true);
    }

    ::std::auto_ptr<ScDocument> pMixDoc;
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

    CursorSwitcher aCursorSwitch(this);
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

    pDocSh->PostPaint(
        aMarkedRange.aStart.Col(), aMarkedRange.aStart.Row(), nTab1,
        aMarkedRange.aEnd.Col(), aMarkedRange.aEnd.Row(), nTab1, PAINT_GRID);

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
            aMarkedRange.aStart.Col(),
            aMarkedRange.aStart.Row(),
            aMarkedRange.aStart.Tab(),
            aMarkedRange.aEnd.Col(),
            aMarkedRange.aEnd.Row(),
            aMarkedRange.aEnd.Tab(),
            aMark, pUndoDoc.release(), NULL, nFlags|nUndoFlags, NULL, NULL, NULL, NULL, false, &aOptions);

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

void ScViewFunc::PostPasteFromClip(const ScRange& rPasteRange, const ScMarkData& rMark)
{
    ScViewData* pViewData = GetViewData();
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pViewData->GetDocument();
    pDocSh->UpdateOle(pViewData);

    SelectionChanged();

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ScRangeList aChangeRanges;
        SCTAB nTabCount = pDoc->GetTableCount();
        for ( SCTAB i = 0; i < nTabCount; ++i )
        {
            if ( rMark.GetTableSelect( i ) )
            {
                ScRange aChangeRange(rPasteRange);
                aChangeRange.aStart.SetTab( i );
                aChangeRange.aEnd.SetTab( i );
                aChangeRanges.Append( aChangeRange );
            }
        }
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
    }
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

        MarkRange( ScRange( rDestPos, aDestEnd ), sal_False );          //! sal_False ???

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
            return sal_False;
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

    MoveCursorAbs( rDestPos.Col(), rDestPos.Row(), SC_FOLLOW_NONE, sal_False, sal_False );

    //  Paste

    PasteFromClip( IDF_ALL, pClipDoc, PASTE_NOFUNC, sal_False, sal_False, sal_True );       // als Link

    delete pClipDoc;

    return sal_True;
}




