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
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <svl/stritem.hxx>
#include <svl/ptitem.hxx>
#include <svl/urlbmk.hxx>
#include <comphelper/classids.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <tools/urlobj.hxx>
#include <sot/exchange.hxx>
#include <memory>

#include <sfx2/lokhelper.hxx>

#include <attrib.hxx>
#include <patattr.hxx>
#include <dociter.hxx>
#include <viewfunc.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <undoblk.hxx>
#include <refundo.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <transobj.hxx>
#include <drwtrans.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <impex.hxx>
#include <chgtrack.hxx>
#include <waitoff.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <inputopt.hxx>
#include <warnbox.hxx>
#include <drwlayer.hxx>
#include <editable.hxx>
#include <docuno.hxx>
#include <clipparam.hxx>
#include <undodat.hxx>
#include <drawview.hxx>
#include <cliputil.hxx>
#include <clipoptions.hxx>
#include <gridwin.hxx>
#include <com/sun/star/util/XCloneable.hpp>

using namespace com::sun::star;

//  GlobalName of writer-DocShell from comphelper/classids.hxx

//      C U T

void ScViewFunc::CutToClip()
{
    UpdateInputLine();

    ScEditableTester aTester( this );
    if (!aTester.IsEditable())                  // selection editable?
    {
        ErrorMessage( aTester.GetMessageId() );
        return;
    }

    ScRange aRange;                             // delete this range
    if ( GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScMarkData& rMark = GetViewData().GetMarkData();
        const bool bRecord(pDoc->IsUndoEnabled());                  // Undo/Redo

        ScDocShellModificator aModificator( *pDocSh );

        if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )          // mark the range if not marked yet
        {
            DoneBlockMode();
            InitOwnBlockMode();
            rMark.SetMarkArea( aRange );
            MarkDataChanged();
        }

        CopyToClip( nullptr, true, false, true/*bIncludeObjects*/ );           // copy to clipboard

        ScAddress aOldEnd( aRange.aEnd );       //  combined cells in this range?
        pDoc->ExtendMerge( aRange, true );

        ScDocumentUniquePtr pUndoDoc;
        if ( bRecord )
        {
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndoSelected( pDoc, rMark );
            // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
            ScRange aCopyRange = aRange;
            aCopyRange.aStart.SetTab(0);
            aCopyRange.aEnd.SetTab(pDoc->GetTableCount()-1);
            pDoc->CopyToDocument( aCopyRange, (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc );
            pDoc->BeginDrawUndo();
        }

        sal_uInt16 nExtFlags = 0;
        pDocSh->UpdatePaintExt( nExtFlags, aRange );

        rMark.MarkToMulti();
        pDoc->DeleteSelection( InsertDeleteFlags::ALL, rMark );
        pDoc->DeleteObjectsInSelection( rMark );
        rMark.MarkToSimple();

        if ( !AdjustRowHeight( aRange.aStart.Row(), aRange.aEnd.Row() ) )
            pDocSh->PostPaint( aRange, PaintPartFlags::Grid, nExtFlags );

        if ( bRecord )                          // Draw-Undo now available
            pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoCut>( pDocSh, aRange, aOldEnd, rMark, std::move(pUndoDoc) ) );

        aModificator.SetDocumentModified();
        pDocSh->UpdateOle(&GetViewData());

        CellContentChanged();
    }
    else
        ErrorMessage( STR_NOMULTISELECT );
}

//      C O P Y

bool ScViewFunc::CopyToClip( ScDocument* pClipDoc, bool bCut, bool bApi, bool bIncludeObjects, bool bStopEdit )
{
    ScRange aRange;
    ScMarkType eMarkType = GetViewData().GetSimpleArea( aRange );
    ScMarkData& rMark = GetViewData().GetMarkData();
    bool bDone = false;

    if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
       ScRangeList aRangeList( aRange );
       bDone = CopyToClip( pClipDoc, aRangeList, bCut, bApi, bIncludeObjects, bStopEdit );
    }
    else if (eMarkType == SC_MARK_MULTI)
    {
        ScRangeList aRangeList;
        rMark.MarkToSimple();
        rMark.FillRangeListWithMarks(&aRangeList, false);
        bDone = CopyToClip( pClipDoc, aRangeList, bCut, bApi, bIncludeObjects, bStopEdit );
    }
    else
    {
        if (!bApi)
            ErrorMessage(STR_NOMULTISELECT);
    }

    return bDone;
}

// Copy the content of the Range into clipboard.
bool ScViewFunc::CopyToClip( ScDocument* pClipDoc, const ScRangeList& rRanges, bool bCut, bool bApi, bool bIncludeObjects, bool bStopEdit )
{
    if ( rRanges.empty() )
        return false;
    if ( bStopEdit )
        UpdateInputLine();

    bool bDone;
    if (rRanges.size() > 1) // isMultiRange
        bDone = CopyToClipMultiRange(pClipDoc, rRanges, bCut, bApi, bIncludeObjects);
    else
        bDone = CopyToClipSingleRange(pClipDoc, rRanges, bCut, bIncludeObjects);
    return bDone;
}

bool ScViewFunc::CopyToClipSingleRange( ScDocument* pClipDoc, const ScRangeList& rRanges, bool bCut, bool bIncludeObjects )
{
    ScRange aRange = rRanges[0];
    ScClipParam aClipParam( aRange, bCut );
    aClipParam.maRanges = rRanges;
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();

    if ( !pDoc
        || pDoc->HasSelectedBlockMatrixFragment( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), rMark ) )
        return false;

    bool bSysClip = false;
    if ( !pClipDoc )                                    // no clip doc specified
    {
        // Create one (deleted by ScTransferObj).
        pClipDoc = new ScDocument( SCDOCMODE_CLIP );
        bSysClip = true;                                // and copy into system
    }
    if ( !bCut )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();
    }

    if ( bSysClip && bIncludeObjects )
    {
        bool bAnyOle = pDoc->HasOLEObjectsInArea( aRange );
        // Update ScGlobal::xDrawClipDocShellRef.
        ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );
    }

    // is this necessary?, will setting the doc id upset the
    // following paste operation with range? would be nicer to just set this always
    // and lose the 'if' above
    aClipParam.setSourceDocID( pDoc->GetDocumentID() );

    if (SfxObjectShell* pObjectShell = pDoc->GetDocumentShell())
    {
        // Copy document properties from pObjectShell to pClipDoc (to its clip options, as it has no object shell).
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(pObjectShell->GetModel(), uno::UNO_QUERY);
        uno::Reference<util::XCloneable> xCloneable(xDocumentPropertiesSupplier->getDocumentProperties(), uno::UNO_QUERY);
        std::unique_ptr<ScClipOptions> pOptions(new ScClipOptions);
        pOptions->m_xDocumentProperties.set(xCloneable->createClone(), uno::UNO_QUERY);
        pClipDoc->SetClipOptions(std::move(pOptions));
    }

    pDoc->CopyToClip( aClipParam, pClipDoc, &rMark, false, bIncludeObjects );
    if (ScDrawLayer* pDrawLayer = pClipDoc->GetDrawLayer())
    {
        ScClipParam& rClipDocClipParam = pClipDoc->GetClipParam();
        ScRangeListVector& rRangesVector = rClipDocClipParam.maProtectedChartRangesVector;
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

    if ( bSysClip )
    {
        ScDrawLayer::SetGlobalDrawPersist(nullptr);
        ScGlobal::SetClipDocName( pDoc->GetDocumentShell()->GetTitle( SFX_TITLE_FULLNAME ) );
    }
    pClipDoc->ExtendMerge( aRange, true );

    if ( bSysClip )
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        TransferableObjectDescriptor aObjDesc;
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScTransferObj ctor

        ScTransferObj* pTransferObj = new ScTransferObj( ScDocumentUniquePtr(pClipDoc), aObjDesc );
        uno::Reference<css::datatransfer::XTransferable2> xTransferObj = pTransferObj;
        if ( ScGlobal::xDrawClipDocShellRef.is() )
        {
            SfxObjectShellRef aPersistRef( ScGlobal::xDrawClipDocShellRef.get() );
            pTransferObj->SetDrawPersist( aPersistRef );// keep persist for ole objects alive
        }
        pTransferObj->CopyToClipboard( GetActiveWin() );
    }

    return true;
}

bool ScViewFunc::CopyToClipMultiRange( const ScDocument* pInputClipDoc, const ScRangeList& rRanges, bool bCut, bool bApi, bool bIncludeObjects )
{
    if (bCut)
    {
        // We don't support cutting of multi-selections.
        if (!bApi)
            ErrorMessage(STR_NOMULTISELECT);
        return false;
    }
    if (pInputClipDoc)
    {
        // TODO: What's this for?
        if (!bApi)
            ErrorMessage(STR_NOMULTISELECT);
        return false;
    }

    ScClipParam aClipParam( rRanges[0], bCut );
    aClipParam.maRanges = rRanges;
    ScDocument* pDoc = GetViewData().GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    bool bDone = false;
    bool bSuccess = false;
    aClipParam.mbCutMode = false;

    do
    {
        ScDocumentUniquePtr pDocClip(new ScDocument(SCDOCMODE_CLIP));

        // Check for geometrical feasibility of the ranges.
        bool bValidRanges = true;
        ScRange const * p = &aClipParam.maRanges.front();
        SCCOL nPrevColDelta = 0;
        SCROW nPrevRowDelta = 0;
        SCCOL nPrevCol = p->aStart.Col();
        SCROW nPrevRow = p->aStart.Row();
        SCCOL nPrevColSize = p->aEnd.Col() - p->aStart.Col() + 1;
        SCROW nPrevRowSize = p->aEnd.Row() - p->aStart.Row() + 1;
        for ( size_t i = 1; i < aClipParam.maRanges.size(); ++i )
        {
            p = &aClipParam.maRanges[i];
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
        pDoc->CopyToClip(aClipParam, pDocClip.get(), &rMark, false, bIncludeObjects );

        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();   // no more cut-mode

        ScDocShell* pDocSh = GetViewData().GetDocShell();
        TransferableObjectDescriptor aObjDesc;
        pDocSh->FillTransferableObjectDescriptor( aObjDesc );
        aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
        // maSize is set in ScTransferObj ctor

        ScTransferObj* pTransferObj = new ScTransferObj( std::move(pDocClip), aObjDesc );
        uno::Reference<css::datatransfer::XTransferable2> xTransferObj = pTransferObj;
        if ( ScGlobal::xDrawClipDocShellRef.is() )
        {
            SfxObjectShellRef aPersistRef( ScGlobal::xDrawClipDocShellRef.get() );
            pTransferObj->SetDrawPersist( aPersistRef );    // keep persist for ole objects alive
        }
        pTransferObj->CopyToClipboard( GetActiveWin() );    // system clipboard

        bSuccess = true;
    }
    while (false);

    if (!bSuccess && !bApi)
        ErrorMessage(STR_NOMULTISELECT);

    bDone = bSuccess;

    return bDone;
}

ScTransferObj* ScViewFunc::CopyToTransferable()
{
    ScRange aRange;
    if ( GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        ScMarkData& rMark = GetViewData().GetMarkData();
        if ( !pDoc->HasSelectedBlockMatrixFragment(
                        aRange.aStart.Col(), aRange.aStart.Row(),
                        aRange.aEnd.Col(),   aRange.aEnd.Row(),
                        rMark ) )
        {
            ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));    // create one (deleted by ScTransferObj)

            bool bAnyOle = pDoc->HasOLEObjectsInArea( aRange, &rMark );
            ScDrawLayer::SetGlobalDrawPersist( ScTransferObj::SetDrawClipDoc( bAnyOle ) );

            ScClipParam aClipParam(aRange, false);
            pDoc->CopyToClip(aClipParam, pClipDoc.get(), &rMark, false, true);

            ScDrawLayer::SetGlobalDrawPersist(nullptr);
            pClipDoc->ExtendMerge( aRange, true );

            ScDocShell* pDocSh = GetViewData().GetDocShell();
            TransferableObjectDescriptor aObjDesc;
            pDocSh->FillTransferableObjectDescriptor( aObjDesc );
            aObjDesc.maDisplayName = pDocSh->GetMedium()->GetURLObject().GetURLNoPass();
            ScTransferObj* pTransferObj = new ScTransferObj( std::move(pClipDoc), aObjDesc );
            return pTransferObj;
        }
    }

    return nullptr;
}

//      P A S T E

void ScViewFunc::PasteDraw()
{
    ScViewData& rViewData = GetViewData();
    SCCOL nPosX = rViewData.GetCurX();
    SCROW nPosY = rViewData.GetCurY();
    vcl::Window* pWin = GetActiveWin();
    Point aPos = pWin->PixelToLogic( rViewData.GetScrPos( nPosX, nPosY,
                                     rViewData.GetActivePart() ) );
    const ScDrawTransferObj* pDrawClip = ScDrawTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(rViewData.GetActiveWin()));
    if (pDrawClip)
    {
        const OUString& aSrcShellID = pDrawClip->GetShellID();
        OUString aDestShellID = SfxObjectShell::CreateShellID(rViewData.GetDocShell());
        PasteDraw(aPos, pDrawClip->GetModel(), false, aSrcShellID, aDestShellID);
    }
}

void ScViewFunc::PasteFromSystem()
{
    UpdateInputLine();

    vcl::Window* pWin = GetActiveWin();
    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable2(ScTabViewShell::GetClipData(pWin));
    const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(xTransferable2);
    // keep a reference in case the clipboard is changed during PasteFromClip
    const ScDrawTransferObj* pDrawClip = ScDrawTransferObj::GetOwnClipboard(xTransferable2);
    if (pOwnClip)
    {
        PasteFromClip( InsertDeleteFlags::ALL, pOwnClip->GetDocument(),
                        ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                        true );     // allow warning dialog
    }
    else if (pDrawClip)
        PasteDraw();
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );

        {
            SotClipboardFormatId nBiff8 = SotExchange::RegisterFormatName("Biff8");
            SotClipboardFormatId nBiff5 = SotExchange::RegisterFormatName("Biff5");

            SotClipboardFormatId nFormat; // output param for GetExchangeAction
            sal_uInt8 nEventAction;      // output param for GetExchangeAction

            uno::Reference<css::datatransfer::XTransferable> xTransferable( aDataHelper.GetXTransferable() );
            sal_uInt8 nAction = SotExchange::GetExchangeAction(
                                    aDataHelper.GetDataFlavorExVector(),
                                    SotExchangeDest::SCDOC_FREE_AREA,
                                    EXCHG_IN_ACTION_COPY,
                                    EXCHG_IN_ACTION_DEFAULT,
                                    nFormat, nEventAction, SotClipboardFormatId::NONE,
                                    &xTransferable );

            if ( nAction != EXCHG_INOUT_ACTION_NONE )
            {
                switch( nAction )
                {
                case EXCHG_OUT_ACTION_INSERT_SVXB:
                case EXCHG_OUT_ACTION_INSERT_GDIMETAFILE:
                case EXCHG_OUT_ACTION_INSERT_BITMAP:
                case EXCHG_OUT_ACTION_INSERT_GRAPH:
                    // SotClipboardFormatId::BITMAP
                    // SotClipboardFormatId::PNG
                    // SotClipboardFormatId::GDIMETAFILE
                    // SotClipboardFormatId::SVXB
                    PasteFromSystem(nFormat);
                    break;
                default:
                    nAction = EXCHG_INOUT_ACTION_NONE;
                }
            }

            if ( nAction == EXCHG_INOUT_ACTION_NONE )
            {
                //  first SvDraw-model, then drawing
                //  (only one drawing is allowed)

                if (aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ))
                {
                    // special case for tables from drawing
                    if( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) )
                    {
                        PasteFromSystem( SotClipboardFormatId::RTF );
                    }
                    else if( aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) )
                    {
                        PasteFromSystem( SotClipboardFormatId::RICHTEXT );
                    }
                    else
                    {
                        PasteFromSystem( SotClipboardFormatId::DRAWING );
                    }
                }
                else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE ))
                {
                    //  If it's a Writer object, insert RTF instead of OLE

                    //  Else, if the class id is all-zero, and SYLK is available,
                    //  it probably is spreadsheet cells that have been put
                    //  on the clipboard by OOo, so use the SYLK. (fdo#31077)

                    bool bDoRtf = false;
                    TransferableObjectDescriptor aObjDesc;
                    if( aDataHelper.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc ) )
                    {
                        bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                                     aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                                   && ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) || aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) ) );
                    }
                    if ( bDoRtf )
                        PasteFromSystem( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT );
                    else if ( aObjDesc.maClassName == SvGlobalName( 0,0,0,0,0,0,0,0,0,0,0 )
                              && aDataHelper.HasFormat( SotClipboardFormatId::SYLK ))
                        PasteFromSystem( SotClipboardFormatId::SYLK );
                    else
                        PasteFromSystem( SotClipboardFormatId::EMBED_SOURCE );
                }
                else if (aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE ))
                    PasteFromSystem( SotClipboardFormatId::LINK_SOURCE );
                    // the following format can not affect scenario from #89579#
                else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ_OLE ))
                    PasteFromSystem( SotClipboardFormatId::EMBEDDED_OBJ_OLE );
                    // SotClipboardFormatId::PRIVATE no longer here (can't work if pOwnClip is NULL)
                else if (aDataHelper.HasFormat(nBiff8))      // before xxx_OLE formats
                    PasteFromSystem(nBiff8);
                else if (aDataHelper.HasFormat(nBiff5))
                    PasteFromSystem(nBiff5);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::RTF))
                    PasteFromSystem(SotClipboardFormatId::RTF);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::RICHTEXT))
                    PasteFromSystem(SotClipboardFormatId::RICHTEXT);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::HTML))
                    PasteFromSystem(SotClipboardFormatId::HTML);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::HTML_SIMPLE))
                    PasteFromSystem(SotClipboardFormatId::HTML_SIMPLE);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::SYLK))
                    PasteFromSystem(SotClipboardFormatId::SYLK);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::STRING_TSVC))
                    PasteFromSystem(SotClipboardFormatId::STRING_TSVC);
                else if (aDataHelper.HasFormat(SotClipboardFormatId::STRING))
                    PasteFromSystem(SotClipboardFormatId::STRING);
                // xxx_OLE formats come last, like in SotExchange tables
                else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE_OLE ))
                    PasteFromSystem( SotClipboardFormatId::EMBED_SOURCE_OLE );
                else if (aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE_OLE ))
                    PasteFromSystem( SotClipboardFormatId::LINK_SOURCE_OLE );
            }
        }
    }
    //  no exception-> SID_PASTE has FastCall-flag from idl
    //  will be called in case of empty clipboard (#42531#)
}

void ScViewFunc::PasteFromTransferable( const uno::Reference<datatransfer::XTransferable>& rxTransferable )
{
    ScTransferObj *pOwnClip=nullptr;
    ScDrawTransferObj *pDrawClip=nullptr;
    uno::Reference<lang::XUnoTunnel> xTunnel( rxTransferable, uno::UNO_QUERY );
    if ( xTunnel.is() )
    {
        sal_Int64 nHandle = xTunnel->getSomething( ScTransferObj::getUnoTunnelId() );
        if ( nHandle )
            pOwnClip = reinterpret_cast<ScTransferObj*>( static_cast<sal_IntPtr>(nHandle));
        else
        {
            nHandle = xTunnel->getSomething( ScDrawTransferObj::getUnoTunnelId() );
            if ( nHandle )
                pDrawClip = reinterpret_cast<ScDrawTransferObj*>( static_cast<sal_IntPtr>(nHandle) );
        }
    }

    if (pOwnClip)
    {
        PasteFromClip( InsertDeleteFlags::ALL, pOwnClip->GetDocument(),
                        ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                        true );     // allow warning dialog
    }
    else if (pDrawClip)
    {
        ScViewData& rViewData = GetViewData();
        SCCOL nPosX = rViewData.GetCurX();
        SCROW nPosY = rViewData.GetCurY();
        vcl::Window* pWin = GetActiveWin();
        Point aPos = pWin->PixelToLogic( rViewData.GetScrPos( nPosX, nPosY, rViewData.GetActivePart() ) );
        PasteDraw(
            aPos, pDrawClip->GetModel(), false,
            pDrawClip->GetShellID(), SfxObjectShell::CreateShellID(rViewData.GetDocShell()));
    }
    else
    {
            TransferableDataHelper aDataHelper( rxTransferable );
            SotClipboardFormatId nBiff8 = SotExchange::RegisterFormatName("Biff8");
            SotClipboardFormatId nBiff5 = SotExchange::RegisterFormatName("Biff5");
            SotClipboardFormatId nFormatId = SotClipboardFormatId::NONE;
                //  first SvDraw-model, then drawing
                //  (only one drawing is allowed)

            if (aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ))
                nFormatId = SotClipboardFormatId::DRAWING;
            else if (aDataHelper.HasFormat( SotClipboardFormatId::SVXB ))
                nFormatId = SotClipboardFormatId::SVXB;
            else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE ))
            {
                //  If it's a Writer object, insert RTF instead of OLE
                bool bDoRtf = false;
                TransferableObjectDescriptor aObjDesc;
                if( aDataHelper.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc ) )
                {
                    bDoRtf = ( ( aObjDesc.maClassName == SvGlobalName( SO3_SW_CLASSID ) ||
                                 aObjDesc.maClassName == SvGlobalName( SO3_SWWEB_CLASSID ) )
                               && ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) || aDataHelper.HasFormat( SotClipboardFormatId::RICHTEXT ) ));
                }
                if ( bDoRtf )
                    nFormatId = aDataHelper.HasFormat( SotClipboardFormatId::RTF ) ? SotClipboardFormatId::RTF : SotClipboardFormatId::RICHTEXT;
                else
                    nFormatId = SotClipboardFormatId::EMBED_SOURCE;
            }
            else if (aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE ))
                nFormatId = SotClipboardFormatId::LINK_SOURCE;
            // the following format can not affect scenario from #89579#
            else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ_OLE ))
                nFormatId = SotClipboardFormatId::EMBEDDED_OBJ_OLE;
            // SotClipboardFormatId::PRIVATE no longer here (can't work if pOwnClip is NULL)
            else if (aDataHelper.HasFormat(nBiff8))      // before xxx_OLE formats
                nFormatId = nBiff8;
            else if (aDataHelper.HasFormat(nBiff5))
                nFormatId = nBiff5;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::RTF))
                nFormatId = SotClipboardFormatId::RTF;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::RICHTEXT))
                nFormatId = SotClipboardFormatId::RICHTEXT;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::HTML))
                nFormatId = SotClipboardFormatId::HTML;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::HTML_SIMPLE))
                nFormatId = SotClipboardFormatId::HTML_SIMPLE;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::SYLK))
                nFormatId = SotClipboardFormatId::SYLK;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::STRING_TSVC))
                nFormatId = SotClipboardFormatId::STRING_TSVC;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::STRING))
                nFormatId = SotClipboardFormatId::STRING;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::GDIMETAFILE))
                nFormatId = SotClipboardFormatId::GDIMETAFILE;
            else if (aDataHelper.HasFormat(SotClipboardFormatId::BITMAP))
                nFormatId = SotClipboardFormatId::BITMAP;
            // xxx_OLE formats come last, like in SotExchange tables
            else if (aDataHelper.HasFormat( SotClipboardFormatId::EMBED_SOURCE_OLE ))
                nFormatId = SotClipboardFormatId::EMBED_SOURCE_OLE;
            else if (aDataHelper.HasFormat( SotClipboardFormatId::LINK_SOURCE_OLE ))
                nFormatId = SotClipboardFormatId::LINK_SOURCE_OLE;
            else
                return;

            PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                GetViewData().GetCurX(), GetViewData().GetCurY(), nullptr );
    }
}

bool ScViewFunc::PasteFromSystem( SotClipboardFormatId nFormatId, bool bApi )
{
    UpdateInputLine();

    bool bRet = true;
    vcl::Window* pWin = GetActiveWin();
    // keep a reference in case the clipboard is changed during PasteFromClip
    const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(pWin));
    if ( nFormatId == SotClipboardFormatId::NONE && pOwnClip )
    {
        PasteFromClip( InsertDeleteFlags::ALL, pOwnClip->GetDocument(),
                        ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                        !bApi );        // allow warning dialog
    }
    else
    {
        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pWin ) );
        if ( !aDataHelper.GetTransferable().is() )
            return false;

        SCCOL nPosX = 0;
        SCROW nPosY = 0;

        ScViewData& rViewData = GetViewData();
        ScRange aRange;
        if ( rViewData.GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
        {
            nPosX = aRange.aStart.Col();
            nPosY = aRange.aStart.Row();
        }
        else
        {
            nPosX = rViewData.GetCurX();
            nPosY = rViewData.GetCurY();
        }

        bRet = PasteDataFormat( nFormatId, aDataHelper.GetTransferable(),
                                nPosX, nPosY,
                                nullptr, false, !bApi );       // allow warning dialog

        if ( !bRet && !bApi )
            ErrorMessage(STR_PASTE_ERROR);
    }
    return bRet;
}

//      P A S T E

bool ScViewFunc::PasteOnDrawObjectLinked(
    const uno::Reference<datatransfer::XTransferable>& rxTransferable,
    SdrObject& rHitObj)
{
    TransferableDataHelper aDataHelper( rxTransferable );

    if ( aDataHelper.HasFormat( SotClipboardFormatId::SVXB ) )
    {
        tools::SvRef<SotStorageStream> xStm;
        ScDrawView* pScDrawView = GetScDrawView();

        if( pScDrawView && aDataHelper.GetSotStorageStream( SotClipboardFormatId::SVXB, xStm ) )
        {
            Graphic aGraphic;

            ReadGraphic( *xStm, aGraphic );

            const OUString aBeginUndo(ScResId(STR_UNDO_DRAGDROP));

            if(pScDrawView->ApplyGraphicToObject( rHitObj, aGraphic, aBeginUndo, "", "" ))
            {
                return true;
            }
        }
    }
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::GDIMETAFILE ) )
    {
        GDIMetaFile aMtf;
        ScDrawView* pScDrawView = GetScDrawView();

        if( pScDrawView && aDataHelper.GetGDIMetaFile( SotClipboardFormatId::GDIMETAFILE, aMtf ) )
        {
            const OUString aBeginUndo(ScResId(STR_UNDO_DRAGDROP));

            if(pScDrawView->ApplyGraphicToObject( rHitObj, Graphic(aMtf), aBeginUndo, "", "" ))
            {
                return true;
            }
        }
    }
    else if ( aDataHelper.HasFormat( SotClipboardFormatId::BITMAP ) || aDataHelper.HasFormat( SotClipboardFormatId::PNG ) )
    {
        BitmapEx aBmpEx;
        ScDrawView* pScDrawView = GetScDrawView();

        if( pScDrawView && aDataHelper.GetBitmapEx( SotClipboardFormatId::BITMAP, aBmpEx ) )
        {
            const OUString aBeginUndo(ScResId(STR_UNDO_DRAGDROP));

            if(pScDrawView->ApplyGraphicToObject( rHitObj, Graphic(aBmpEx), aBeginUndo, "", "" ))
            {
                return true;
            }
        }
    }

    return false;
}

static bool lcl_SelHasAttrib( const ScDocument* pDoc, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        const ScMarkData& rTabSelection, HasAttrFlags nMask )
{
    return std::any_of(rTabSelection.begin(), rTabSelection.end(),
        [&](const SCTAB& rTab) { return pDoc->HasAttrib( nCol1, nRow1, rTab, nCol2, nRow2, rTab, nMask ); });
}

//  paste into sheet:

//  internal paste

namespace {

bool checkDestRangeForOverwrite(const ScRangeList& rDestRanges, const ScDocument* pDoc, const ScMarkData& rMark, weld::Window* pParentWnd)
{
    bool bIsEmpty = true;
    size_t nRangeSize = rDestRanges.size();
    for (const auto& rTab : rMark)
    {
        for (size_t i = 0; i < nRangeSize && bIsEmpty; ++i)
        {
            const ScRange& rRange = rDestRanges[i];
            bIsEmpty = pDoc->IsBlockEmpty(
                rTab, rRange.aStart.Col(), rRange.aStart.Row(),
                rRange.aEnd.Col(), rRange.aEnd.Row());
        }
        if (!bIsEmpty)
            break;
    }

    if (!bIsEmpty)
    {
        ScReplaceWarnBox aBox(pParentWnd);
        if (aBox.run() != RET_YES)
        {
            //  changing the configuration is within the ScReplaceWarnBox
            return false;
        }
    }
    return true;
}

}

bool ScViewFunc::PasteFromClip( InsertDeleteFlags nFlags, ScDocument* pClipDoc,
                                ScPasteFunc nFunction, bool bSkipEmpty,
                                bool bTranspose, bool bAsLink,
                                InsCellCmd eMoveMode, InsertDeleteFlags nUndoExtraFlags,
                                bool bAllowDialogs )
{
    if (!pClipDoc)
    {
        OSL_FAIL("PasteFromClip: pClipDoc=0 not allowed");
        return false;
    }

    if (GetViewData().SelectionForbidsCellFill())
        return false;

    //  undo: save all or no content
    InsertDeleteFlags nContFlags = InsertDeleteFlags::NONE;
    if (nFlags & InsertDeleteFlags::CONTENTS)
        nContFlags |= InsertDeleteFlags::CONTENTS;
    if (nFlags & InsertDeleteFlags::ATTRIB)
        nContFlags |= InsertDeleteFlags::ATTRIB;
    // move attributes to undo without copying them from clip to doc
    InsertDeleteFlags nUndoFlags = nContFlags;
    if (nUndoExtraFlags & InsertDeleteFlags::ATTRIB)
        nUndoFlags |= InsertDeleteFlags::ATTRIB;
    // do not copy note captions into undo document
    nUndoFlags |= InsertDeleteFlags::NOCAPTIONS;

    ScClipParam& rClipParam = pClipDoc->GetClipParam();
    if (rClipParam.isMultiRange())
    {
        // Source data is multi-range.
        return PasteMultiRangesFromClip(
            nFlags, pClipDoc, nFunction, bSkipEmpty, bTranspose, bAsLink, bAllowDialogs,
            eMoveMode, nUndoFlags);
    }

    ScMarkData& rMark = GetViewData().GetMarkData();
    if (rMark.IsMultiMarked())
    {
        // Source data is single-range but destination is multi-range.
        return PasteFromClipToMultiRanges(
            nFlags, pClipDoc, nFunction, bSkipEmpty, bTranspose, bAsLink, bAllowDialogs,
            eMoveMode, nUndoFlags);
    }

    bool bCutMode = pClipDoc->IsCutMode();      // if transposing, take from original clipdoc
    bool bIncludeFiltered = bCutMode;

    // paste drawing: also if InsertDeleteFlags::NOTE is set (to create drawing layer for note captions)
    bool bPasteDraw = ( pClipDoc->GetDrawLayer() && ( nFlags & (InsertDeleteFlags::OBJECTS|InsertDeleteFlags::NOTE) ) );

    ScDocShellRef aTransShellRef;   // for objects in xTransClip - must remain valid as long as xTransClip
    ScDocument* pOrigClipDoc = nullptr;
    ScDocumentUniquePtr xTransClip;
    if ( bTranspose )
    {
        SCCOL nX;
        SCROW nY;
        // include filtered rows until TransposeClip can skip them
        bIncludeFiltered = true;
        pClipDoc->GetClipArea( nX, nY, true );
        if ( nY > static_cast<sal_Int32>(MAXCOL) )                      // too many lines for transpose
        {
            ErrorMessage(STR_PASTE_FULL);
            return false;
        }
        pOrigClipDoc = pClipDoc;        // refs

        if ( bPasteDraw )
        {
            aTransShellRef = new ScDocShell;        // DocShell needs a Ref immediately
            aTransShellRef->DoInitNew();
        }
        ScDrawLayer::SetGlobalDrawPersist( aTransShellRef.get() );

        xTransClip.reset( new ScDocument( SCDOCMODE_CLIP ));
        pClipDoc->TransposeClip( xTransClip.get(), nFlags, bAsLink );
        pClipDoc = xTransClip.get();

        ScDrawLayer::SetGlobalDrawPersist(nullptr);
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

    ScDocument* pDoc = GetViewData().GetDocument();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    const bool bRecord(pDoc->IsUndoEnabled());

    ScDocShellModificator aModificator( *pDocSh );

    ScRange aMarkRange;
    ScMarkData aFilteredMark( rMark);   // local copy for all modifications
    ScMarkType eMarkType = GetViewData().GetSimpleArea( aMarkRange, aFilteredMark);
    bool bMarkIsFiltered = (eMarkType == SC_MARK_SIMPLE_FILTERED);
    bool bNoPaste = ((eMarkType != SC_MARK_SIMPLE && !bMarkIsFiltered) ||
            (bMarkIsFiltered && (eMoveMode != INS_NONE || bAsLink)));

    if (!bNoPaste)
    {
        if (!rMark.IsMarked())
        {
            // Create a selection with clipboard row count and check that for
            // filtered.
            nStartCol = GetViewData().GetCurX();
            nStartRow = GetViewData().GetCurY();
            nStartTab = GetViewData().GetTabNo();
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
            ScRange & r = aRangeList[i];
            nUnfilteredRows += r.aEnd.Row() - r.aStart.Row() + 1;
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

        // request, if the selection is greater than one row/column, but smaller
        // as the Clipboard (then inserting is done beyond the selection)

        //  ClipSize is not size, but difference
        if ( ( nBlockAddX != 0 && nBlockAddX < nDestSizeX ) ||
             ( nBlockAddY != 0 && nBlockAddY < nDestSizeY ) ||
             ( bMarkIsFiltered && nUnfilteredRows < nDestSizeY+1 ) )
        {
            ScWaitCursorOff aWaitOff( GetFrameWin() );
            OUString aMessage = ScResId( STR_PASTE_BIGGER );

            vcl::Window* pWin = GetViewData().GetDialogParent();
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           aMessage));
            xQueryBox->set_default_response(RET_NO);
            if (xQueryBox->run() != RET_YES)
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
        nStartCol = GetViewData().GetCurX();
        nStartRow = GetViewData().GetCurY();
        nStartTab = GetViewData().GetTabNo();
        nEndCol = nStartCol + nDestSizeX;
        nEndRow = nStartRow + nDestSizeY;
        nEndTab = nStartTab;
    }

    bool bOffLimits = !ValidCol(nEndCol) || !ValidRow(nEndRow);

    //  target-range, as displayed:
    ScRange aUserRange( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab );

    //  should lines be inserted?
    //  ( too large nEndCol/nEndRow are detected below)
    bool bInsertCells = ( eMoveMode != INS_NONE && !bOffLimits );
    if ( bInsertCells )
    {
        //  Instead of EnterListAction, the paste undo action is merged into the
        //  insert action, so Repeat can insert the right cells

        MarkRange( aUserRange );            // set through CopyFromClip

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
                                ( nFlags & InsertDeleteFlags::CONTENTS ) &&
                                nFunction == ScPasteFunc::NONE &&
                                SC_MOD()->GetInputOptions().GetReplaceCellsWarn();
        if ( bAskIfNotEmpty )
        {
            ScRangeList aTestRanges(aUserRange);
            vcl::Window* pWin = GetViewData().GetDialogParent();
            if (!checkDestRangeForOverwrite(aTestRanges, pDoc, aFilteredMark, pWin ? pWin->GetFrameWeld() : nullptr))
                return false;
        }
    }

    SCCOL nClipStartX;                      // enlarge clipboard-range
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
            if ( pClipDoc->ExtendMerge( nClipStartX,nClipStartY, nUndoEndCol,nUndoEndRow, nClipTab ) )
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

    pDoc->ExtendMergeSel( nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, aFilteredMark );

        //  check cell-protection

    ScEditableTester aTester( pDoc, nStartTab, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }

        //! check overlapping
        //! just check truly intersection !!!!!!!

    ScDocFunc& rDocFunc = pDocSh->GetDocFunc();
    if ( bRecord )
    {
        OUString aUndo = ScResId( pClipDoc->IsCutMode() ? STR_UNDO_MOVE : STR_UNDO_COPY );
        pUndoMgr->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );
    }

    if (bClipOver)
        if (lcl_SelHasAttrib( pDoc, nStartCol,nStartRow, nUndoEndCol,nUndoEndRow, aFilteredMark, HasAttrFlags::Overlapped ))
        {       // "Cell merge not possible if cells already merged"
            ScDocAttrIterator aIter( pDoc, nStartTab, nStartCol, nStartRow, nUndoEndCol, nUndoEndRow );
            const ScPatternAttr* pPattern = nullptr;
            SCCOL nCol = -1;
            SCROW nRow1 = -1;
            SCROW nRow2 = -1;
            while ( ( pPattern = aIter.GetNext( nCol, nRow1, nRow2 ) ) != nullptr )
            {
                const ScMergeAttr& rMergeFlag = pPattern->GetItem(ATTR_MERGE);
                const ScMergeFlagAttr& rMergeFlagAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
                if (rMergeFlag.IsMerged() || rMergeFlagAttr.IsOverlapped())
                {
                    ScRange aRange(nCol, nRow1, nStartTab);
                    pDoc->ExtendOverlapped(aRange);
                    pDoc->ExtendMerge(aRange, true);
                    rDocFunc.UnmergeCells(aRange, bRecord, nullptr /*TODO: should pass combined UndoDoc if bRecord*/);
                }
            }
        }

    if ( !bCutMode )
    {
        ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        if ( pChangeTrack )
            pChangeTrack->ResetLastCut();   // no more cut-mode
    }

    bool bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
    bool bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDocument> pRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;

    if ( bRecord )
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndoSelected( pDoc, aFilteredMark, bColInfo, bRowInfo );

        // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        SCTAB nTabCount = pDoc->GetTableCount();
        pDoc->CopyToDocument( nStartCol, nStartRow, 0, nUndoEndCol, nUndoEndRow, nTabCount-1,
                              nUndoFlags, false, *pUndoDoc );

        if ( bCutMode )
        {
            pRefUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pRefUndoDoc->InitUndo( pDoc, 0, nTabCount-1 );

            pUndoData.reset(new ScRefUndoData( pDoc ));
        }
    }

    sal_uInt16 nExtFlags = 0;
    pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab,
                                       nEndCol,   nEndRow,   nEndTab );     // content before the change

    if (GetViewData().IsActive())
    {
        DoneBlockMode();
        InitOwnBlockMode();
    }
    rMark.SetMarkArea( aUserRange );
    MarkDataChanged();

        //  copy from clipboard
        //  save original data in case of calculation

    ScDocumentUniquePtr pMixDoc;
    if (nFunction != ScPasteFunc::NONE)
    {
        bSkipEmpty = false;
        if ( nFlags & InsertDeleteFlags::CONTENTS )
        {
            pMixDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pMixDoc->InitUndo( pDoc, nStartTab, nEndTab );
            pDoc->CopyToDocument(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                 InsertDeleteFlags::CONTENTS, false, *pMixDoc);
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

    InsertDeleteFlags nNoObjFlags = nFlags & ~InsertDeleteFlags::OBJECTS;
    if (!bAsLink)
    {
        //  copy normally (original range)
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nNoObjFlags,
                pRefUndoDoc.get(), pClipDoc, true, false, bIncludeFiltered,
                bSkipEmpty, (bMarkIsFiltered ? &aRangeList : nullptr) );

        // adapt refs manually in case of transpose
        if ( bTranspose && bCutMode && (nFlags & InsertDeleteFlags::CONTENTS) )
            pDoc->UpdateTranspose( aUserRange.aStart, pOrigClipDoc, aFilteredMark, pRefUndoDoc.get() );
    }
    else if (!bTranspose)
    {
        //  copy with bAsLink=TRUE
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nNoObjFlags, pRefUndoDoc.get(), pClipDoc,
                                true, true, bIncludeFiltered, bSkipEmpty );
    }
    else
    {
        //  copy all content (TransClipDoc contains only formula)
        pDoc->CopyFromClip( aUserRange, aFilteredMark, nContFlags, pRefUndoDoc.get(), pClipDoc );
    }

    // skipped rows and merged cells don't mix
    if ( !bIncludeFiltered && pClipDoc->HasClipFilteredRows() )
        rDocFunc.UnmergeCells( aUserRange, false, nullptr );

    pDoc->ExtendMergeSel( nStartCol, nStartRow, nEndCol, nEndRow, aFilteredMark, true );    // refresh
                                                                                    // new range

    if ( pMixDoc )              // calculate with original data?
    {
        pDoc->MixDocument( aUserRange, nFunction, bSkipEmpty, pMixDoc.get() );
    }
    pMixDoc.reset();

    AdjustBlockHeight();            // update row heights before pasting objects

    ::std::vector< OUString > aExcludedChartNames;
    SdrPage* pPage = nullptr;

    if ( nFlags & InsertDeleteFlags::OBJECTS )
    {
        ScDrawView* pScDrawView = GetScDrawView();
        SdrModel* pModel = ( pScDrawView ? pScDrawView->GetModel() : nullptr );
        pPage = ( pModel ? pModel->GetPage( static_cast< sal_uInt16 >( nStartTab ) ) : nullptr );
        if ( pPage )
        {
            ScChartHelper::GetChartNames( aExcludedChartNames, pPage );
        }

        //  Paste the drawing objects after the row heights have been updated.

        pDoc->CopyFromClip( aUserRange, aFilteredMark, InsertDeleteFlags::OBJECTS, pRefUndoDoc.get(), pClipDoc,
                                true, false, bIncludeFiltered );
    }

    pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab,
                                       nEndCol,   nEndRow,   nEndTab );     // content after the change

        //  if necessary, delete autofilter-heads
    if (bCutMode)
        if (pDoc->RefreshAutoFilter( nClipStartX,nClipStartY, nClipStartX+nClipSizeX,
                                        nClipStartY+nClipSizeY, nStartTab ))
        {
            pDocSh->PostPaint(
                ScRange(nClipStartX, nClipStartY, nStartTab, nClipStartX+nClipSizeX, nClipStartY, nStartTab),
                PaintPartFlags::Grid );
        }

    //!     remove block-range on RefUndoDoc !!!

    if ( bRecord )
    {
        ScDocumentUniquePtr pRedoDoc;
        // copy redo data after appearance of the first undo
        // don't create Redo-Doc without RefUndoDoc

        if (pRefUndoDoc)
        {
            pRedoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pRedoDoc->InitUndo( pDoc, nStartTab, nEndTab, bColInfo, bRowInfo );

            //      move adapted refs to Redo-Doc

            SCTAB nTabCount = pDoc->GetTableCount();
            pRedoDoc->AddUndoTab( 0, nTabCount-1 );
            pDoc->CopyUpdated( pRefUndoDoc.get(), pRedoDoc.get() );

            //      move old refs to Undo-Doc

            //      not charts?
            pUndoDoc->AddUndoTab( 0, nTabCount-1 );
            pRefUndoDoc->DeleteArea( nStartCol, nStartRow, nEndCol, nEndRow, aFilteredMark, InsertDeleteFlags::ALL );
            pRefUndoDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                            InsertDeleteFlags::FORMULA, false, *pUndoDoc );
            pRefUndoDoc.reset();
        }

        //  DeleteUnchanged for pUndoData is in ScUndoPaste ctor,
        //  UndoData for redo is made during first undo

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;

        std::unique_ptr<SfxUndoAction> pUndo(new ScUndoPaste(
            pDocSh, ScRange(nStartCol, nStartRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
            aFilteredMark, std::move(pUndoDoc), std::move(pRedoDoc), nFlags | nUndoFlags, std::move(pUndoData),
            false, &aOptions ));     // false = Redo data not yet copied

        if ( bInsertCells )
        {
            //  Merge the paste undo action into the insert action.
            //  Use ScUndoWrapper so the ScUndoPaste pointer can be stored in the insert action.

            pUndoMgr->AddUndoAction( std::make_unique<ScUndoWrapper>( std::move(pUndo) ), true );
        }
        else
            pUndoMgr->AddUndoAction( std::move(pUndo) );
        pUndoMgr->LeaveListAction();
    }

    PaintPartFlags nPaint = PaintPartFlags::Grid;
    if (bColInfo)
    {
        nPaint |= PaintPartFlags::Top;
        nUndoEndCol = MAXCOL;               // just for drawing !
    }
    if (bRowInfo)
    {
        nPaint |= PaintPartFlags::Left;
        nUndoEndRow = MAXROW;               // just for drawing !
    }
    pDocSh->PostPaint(
        ScRange(nStartCol, nStartRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
        nPaint, nExtFlags);
    // AdjustBlockHeight has already been called above

    ResetAutoSpell();
    aModificator.SetDocumentModified();
    PostPasteFromClip(aUserRange, rMark);

    if ( nFlags & InsertDeleteFlags::OBJECTS )
    {
        ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
        if ( pPage && pModelObj )
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
    InsertDeleteFlags nFlags, ScDocument* pClipDoc, ScPasteFunc nFunction,
    bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
    InsCellCmd eMoveMode, InsertDeleteFlags nUndoFlags)
{
    ScViewData& rViewData = GetViewData();
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

        ScDocumentUniquePtr pTransClip(new ScDocument(SCDOCMODE_CLIP));
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
        bAllowDialogs && (nFlags & InsertDeleteFlags::CONTENTS) &&
        nFunction == ScPasteFunc::NONE && SC_MOD()->GetInputOptions().GetReplaceCellsWarn();

    if (bAskIfNotEmpty)
    {
        ScRangeList aTestRanges(aMarkedRange);
        vcl::Window* pWin = GetViewData().GetDialogParent();
        if (!checkDestRangeForOverwrite(aTestRanges, pDoc, aMark, pWin ? pWin->GetFrameWeld() : nullptr))
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

    bool bRowInfo = ( aMarkedRange.aStart.Col()==0 && aMarkedRange.aEnd.Col()==MAXCOL );
    ScDocumentUniquePtr pUndoDoc;
    if (pDoc->IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndoSelected(pDoc, aMark, false, bRowInfo);
        pDoc->CopyToDocument(aMarkedRange, nUndoFlags, false, *pUndoDoc, &aMark);
    }

    ScDocumentUniquePtr pMixDoc;
    if ( bSkipEmpty || nFunction != ScPasteFunc::NONE)
    {
        if ( nFlags & InsertDeleteFlags::CONTENTS )
        {
            pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pMixDoc->InitUndoSelected(pDoc, aMark);
            pDoc->CopyToDocument(aMarkedRange, InsertDeleteFlags::CONTENTS, false, *pMixDoc, &aMark);
        }
    }

    /*  Make draw layer and start drawing undo.
        - Needed before AdjustBlockHeight to track moved drawing objects.
        - Needed before pDoc->CopyFromClip to track inserted note caption objects.
     */
    if (nFlags & InsertDeleteFlags::OBJECTS)
        pDocSh->MakeDrawLayer();
    if (pDoc->IsUndoEnabled())
        pDoc->BeginDrawUndo();

    InsertDeleteFlags nNoObjFlags = nFlags & ~InsertDeleteFlags::OBJECTS;
    pDoc->CopyMultiRangeFromClip(rCurPos, aMark, nNoObjFlags, pClipDoc,
                                 true, bAsLink, false, bSkipEmpty);

    if (pMixDoc.get())
        pDoc->MixDocument(aMarkedRange, nFunction, bSkipEmpty, pMixDoc.get());

    AdjustBlockHeight();            // update row heights before pasting objects

    if (nFlags & InsertDeleteFlags::OBJECTS)
    {
        //  Paste the drawing objects after the row heights have been updated.
        pDoc->CopyMultiRangeFromClip(rCurPos, aMark, InsertDeleteFlags::OBJECTS, pClipDoc,
                                     true, false, false, true);
    }

    if (bRowInfo)
        pDocSh->PostPaint(aMarkedRange.aStart.Col(), aMarkedRange.aStart.Row(), nTab1, MAXCOL, MAXROW, nTab1, PaintPartFlags::Grid|PaintPartFlags::Left);
    else
    {
        ScRange aTmp = aMarkedRange;
        aTmp.aStart.SetTab(nTab1);
        aTmp.aEnd.SetTab(nTab1);
        pDocSh->PostPaint(aTmp, PaintPartFlags::Grid);
    }

    if (pDoc->IsUndoEnabled())
    {
        SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        OUString aUndo = ScResId(
            pClipDoc->IsCutMode() ? STR_UNDO_CUT : STR_UNDO_COPY);
        pUndoMgr->EnterListAction(aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId());

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;

        std::unique_ptr<ScUndoPaste> pUndo(new ScUndoPaste(pDocSh,
            aMarkedRange, aMark, std::move(pUndoDoc), nullptr, nFlags|nUndoFlags, nullptr, false, &aOptions));

        if (bInsertCells)
            pUndoMgr->AddUndoAction(std::make_unique<ScUndoWrapper>(std::move(pUndo)), true);
        else
            pUndoMgr->AddUndoAction(std::move(pUndo));

        pUndoMgr->LeaveListAction();
    }

    ResetAutoSpell();
    aModificator.SetDocumentModified();
    PostPasteFromClip(aMarkedRange, aMark);
    return true;
}

bool ScViewFunc::PasteFromClipToMultiRanges(
    InsertDeleteFlags nFlags, ScDocument* pClipDoc, ScPasteFunc nFunction,
    bool bSkipEmpty, bool bTranspose, bool bAsLink, bool bAllowDialogs,
    InsCellCmd eMoveMode, InsertDeleteFlags nUndoFlags )
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

    ScViewData& rViewData = GetViewData();
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
        bAllowDialogs && (nFlags & InsertDeleteFlags::CONTENTS) &&
        nFunction == ScPasteFunc::NONE && SC_MOD()->GetInputOptions().GetReplaceCellsWarn();

    if (bAskIfNotEmpty)
    {
        vcl::Window* pWin = GetViewData().GetDialogParent();
        if (!checkDestRangeForOverwrite(aRanges, pDoc, aMark, pWin ? pWin->GetFrameWeld() : nullptr))
            return false;
    }

    ScDocumentUniquePtr pUndoDoc;
    if (pDoc->IsUndoEnabled())
    {
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndoSelected(pDoc, aMark);
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
        {
            pDoc->CopyToDocument(
                aRanges[i], nUndoFlags, false, *pUndoDoc, &aMark);
        }
    }

    ScDocumentUniquePtr pMixDoc;
    if (bSkipEmpty || nFunction != ScPasteFunc::NONE)
    {
        if (nFlags & InsertDeleteFlags::CONTENTS)
        {
            pMixDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pMixDoc->InitUndoSelected(pDoc, aMark);
            for (size_t i = 0, n = aRanges.size(); i < n; ++i)
            {
                pDoc->CopyToDocument(
                    aRanges[i], InsertDeleteFlags::CONTENTS, false, *pMixDoc, &aMark);
            }
        }
    }

    if (nFlags & InsertDeleteFlags::OBJECTS)
        pDocSh->MakeDrawLayer();
    if (pDoc->IsUndoEnabled())
        pDoc->BeginDrawUndo();

    // First, paste everything but the drawing objects.
    for (size_t i = 0, n = aRanges.size(); i < n; ++i)
    {
        pDoc->CopyFromClip(
            aRanges[i], aMark, (nFlags & ~InsertDeleteFlags::OBJECTS), nullptr, pClipDoc,
            false, false, true, bSkipEmpty);
    }

    if (pMixDoc.get())
    {
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
            pDoc->MixDocument(aRanges[i], nFunction, bSkipEmpty, pMixDoc.get());
    }

    AdjustBlockHeight();            // update row heights before pasting objects

    // Then paste the objects.
    if (nFlags & InsertDeleteFlags::OBJECTS)
    {
        for (size_t i = 0, n = aRanges.size(); i < n; ++i)
        {
            pDoc->CopyFromClip(
                aRanges[i], aMark, InsertDeleteFlags::OBJECTS, nullptr, pClipDoc,
                false, false, true, bSkipEmpty);
        }
    }

    // Refresh the range that includes all pasted ranges.  We only need to
    // refresh the current sheet.
    PaintPartFlags nPaint = PaintPartFlags::Grid;
    bool bRowInfo = (aSrcRange.aStart.Col()==0 &&  aSrcRange.aEnd.Col()==MAXCOL);
    if (bRowInfo)
        nPaint |= PaintPartFlags::Left;
    pDocSh->PostPaint(aRanges, nPaint);

    if (pDoc->IsUndoEnabled())
    {
        SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
        OUString aUndo = ScResId(
            pClipDoc->IsCutMode() ? STR_UNDO_CUT : STR_UNDO_COPY);
        pUndoMgr->EnterListAction(aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId());

        ScUndoPasteOptions aOptions;            // store options for repeat
        aOptions.nFunction  = nFunction;
        aOptions.bSkipEmpty = bSkipEmpty;
        aOptions.bTranspose = bTranspose;
        aOptions.bAsLink    = bAsLink;
        aOptions.eMoveMode  = eMoveMode;


        pUndoMgr->AddUndoAction(
            std::make_unique<ScUndoPaste>(
                pDocSh, aRanges, aMark, std::move(pUndoDoc), nullptr, nFlags|nUndoFlags, nullptr, false, &aOptions));
        pUndoMgr->LeaveListAction();
    }

    ResetAutoSpell();
    aModificator.SetDocumentModified();
    PostPasteFromClip(aRanges, aMark);

    return false;
}

void ScViewFunc::PostPasteFromClip(const ScRangeList& rPasteRanges, const ScMarkData& rMark)
{
    ScViewData& rViewData = GetViewData();
    ScDocShell* pDocSh = rViewData.GetDocShell();
    pDocSh->UpdateOle(&rViewData);

    SelectionChanged();

    ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh);
    if (!pModelObj)
        return;

    ScRangeList aChangeRanges;
    for (size_t i = 0, n = rPasteRanges.size(); i < n; ++i)
    {
        const ScRange& r = rPasteRanges[i];
        for (const auto& rTab : rMark)
        {
            ScRange aChangeRange(r);
            aChangeRange.aStart.SetTab(rTab);
            aChangeRange.aEnd.SetTab(rTab);
            aChangeRanges.push_back(aChangeRange);
        }
    }
    HelperNotifyChanges::Notify(*pModelObj, aChangeRanges);
}

//      D R A G   A N D   D R O P

//  inside the doc

bool ScViewFunc::MoveBlockTo( const ScRange& rSource, const ScAddress& rDestPos,
                                bool bCut )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    HideAllCursors();

    bool bSuccess = true;
    SCTAB nDestTab = rDestPos.Tab();
    const ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rSource.aStart.Tab() == nDestTab && rSource.aEnd.Tab() == nDestTab && rMark.GetSelectCount() > 1 )
    {
        //  moving within one table and several tables selected -> apply to all selected tables

        OUString aUndo = ScResId( bCut ? STR_UNDO_MOVE : STR_UNDO_COPY );
        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, GetViewData().GetViewShell()->GetViewShellId() );

        //  collect ranges of consecutive selected tables

        ScRange aLocalSource = rSource;
        ScAddress aLocalDest = rDestPos;
        SCTAB nTabCount = pDocSh->GetDocument().GetTableCount();
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
                                aLocalSource, aLocalDest, bCut, true/*bRecord*/, true/*bPaint*/, true/*bApi*/ );

                nStartTab = nEndTab + 1;
            }
        }

        pDocSh->GetUndoManager()->LeaveListAction();
    }
    else
    {
        //  move the block as specified
        bSuccess = pDocSh->GetDocFunc().MoveBlock(
                                rSource, rDestPos, bCut, true/*bRecord*/, true/*bPaint*/, true/*bApi*/ );
    }

    ShowAllCursors();
    if (bSuccess)
    {
        //   mark destination range
        ScAddress aDestEnd(
                    rDestPos.Col() + rSource.aEnd.Col() - rSource.aStart.Col(),
                    rDestPos.Row() + rSource.aEnd.Row() - rSource.aStart.Row(),
                    nDestTab );

        bool bIncludeFiltered = bCut;
        if ( !bIncludeFiltered )
        {
            // find number of non-filtered rows
            SCROW nPastedCount = pDocSh->GetDocument().CountNonFilteredRows(
                rSource.aStart.Row(), rSource.aEnd.Row(), rSource.aStart.Tab());

            if ( nPastedCount == 0 )
                nPastedCount = 1;
            aDestEnd.SetRow( rDestPos.Row() + nPastedCount - 1 );
        }

        MarkRange( ScRange( rDestPos, aDestEnd ), false );          //! sal_False ???

        pDocSh->UpdateOle(&GetViewData());
        SelectionChanged();
        ResetAutoSpell();
    }
    return bSuccess;
}

//  link inside the doc

bool ScViewFunc::LinkBlock( const ScRange& rSource, const ScAddress& rDestPos )
{
    //  check overlapping

    if ( rSource.aStart.Tab() == rDestPos.Tab() )
    {
        SCCOL nDestEndCol = rDestPos.Col() + ( rSource.aEnd.Col() - rSource.aStart.Col() );
        SCROW nDestEndRow = rDestPos.Row() + ( rSource.aEnd.Row() - rSource.aStart.Row() );

        if ( rSource.aStart.Col() <= nDestEndCol && rDestPos.Col() <= rSource.aEnd.Col() &&
             rSource.aStart.Row() <= nDestEndRow && rDestPos.Row() <= rSource.aEnd.Row() )
        {
            return false;
        }
    }

    //  run with paste

    ScDocument* pDoc = GetViewData().GetDocument();
    ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));
    pDoc->CopyTabToClip( rSource.aStart.Col(), rSource.aStart.Row(),
                            rSource.aEnd.Col(), rSource.aEnd.Row(),
                         rSource.aStart.Tab(), pClipDoc.get() );

    //  mark destination area (set cursor, no marks)

    if ( GetViewData().GetTabNo() != rDestPos.Tab() )
        SetTabNo( rDestPos.Tab() );

    MoveCursorAbs( rDestPos.Col(), rDestPos.Row(), SC_FOLLOW_NONE, false, false );

    //  Paste

    PasteFromClip( InsertDeleteFlags::ALL, pClipDoc.get(), ScPasteFunc::NONE, false, false, true );       // as a link

    return true;
}

void ScViewFunc::DataFormPutData( SCROW nCurrentRow ,
                                  SCROW nStartRow , SCCOL nStartCol ,
                                  SCROW nEndRow , SCCOL nEndCol ,
                                  std::vector<VclPtr<Edit> >& aEdits,
                                  sal_uInt16 aColLength )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScMarkData& rMark = GetViewData().GetMarkData();
    ScDocShellModificator aModificator( *pDocSh );
    SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();
    if ( pDoc )
    {
        const bool bRecord( pDoc->IsUndoEnabled());
        ScDocumentUniquePtr pUndoDoc;
        ScDocumentUniquePtr pRedoDoc;
        std::unique_ptr<ScRefUndoData> pUndoData;
        SCTAB nTab = GetViewData().GetTabNo();
        SCTAB nStartTab = nTab;
        SCTAB nEndTab = nTab;

        {
                ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
                if ( pChangeTrack )
                        pChangeTrack->ResetLastCut();   // no more cut-mode
        }
        ScRange aUserRange( nStartCol, nCurrentRow, nStartTab, nEndCol, nCurrentRow, nEndTab );
        bool bColInfo = ( nStartRow==0 && nEndRow==MAXROW );
        bool bRowInfo = ( nStartCol==0 && nEndCol==MAXCOL );
        SCCOL nUndoEndCol = nStartCol+aColLength-1;
        SCROW nUndoEndRow = nCurrentRow;

        if ( bRecord )
        {
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pUndoDoc->InitUndoSelected( pDoc , rMark , bColInfo , bRowInfo );
            pDoc->CopyToDocument( aUserRange , InsertDeleteFlags::VALUE , false, *pUndoDoc );
        }
        sal_uInt16 nExtFlags = 0;
        pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nStartRow, nStartTab , nEndCol, nEndRow, nEndTab ); // content before the change
        pDoc->BeginDrawUndo();

        for(sal_uInt16 i = 0; i < aColLength; i++)
        {
            if (aEdits[i] != nullptr)
            {
                OUString  aFieldName=aEdits[i]->GetText();
                pDoc->SetString( nStartCol + i, nCurrentRow, nTab, aFieldName );
            }
        }
        pDocSh->UpdatePaintExt( nExtFlags, nStartCol, nCurrentRow, nStartTab, nEndCol, nCurrentRow, nEndTab );  // content after the change
        std::unique_ptr<SfxUndoAction> pUndo( new ScUndoDataForm( pDocSh,
                                                   nStartCol, nCurrentRow, nStartTab,
                                                   nUndoEndCol, nUndoEndRow, nEndTab, rMark,
                                                   std::move(pUndoDoc), std::move(pRedoDoc),
                                                   std::move(pUndoData) ) );
        pUndoMgr->AddUndoAction( std::make_unique<ScUndoWrapper>( std::move(pUndo) ), true );

        PaintPartFlags nPaint = PaintPartFlags::Grid;
        if (bColInfo)
        {
                nPaint |= PaintPartFlags::Top;
                nUndoEndCol = MAXCOL;                           // just for drawing !
        }
        if (bRowInfo)
        {
                nPaint |= PaintPartFlags::Left;
                nUndoEndRow = MAXROW;                           // just for drawing !
        }

        pDocSh->PostPaint(
            ScRange(nStartCol, nCurrentRow, nStartTab, nUndoEndCol, nUndoEndRow, nEndTab),
            nPaint, nExtFlags);
        pDocSh->UpdateOle(&GetViewData());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
