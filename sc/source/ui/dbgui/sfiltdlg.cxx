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

#include <sfx2/dispatch.hxx>
#include <vcl/idle.hxx>

#include <uiitems.hxx>
#include <rangenam.hxx>
#include <reffact.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <scresid.hxx>

#include <foptmgr.hxx>

#include <globstr.hrc>
#include <strings.hrc>

#include <filtdlg.hxx>
#include <vcl/weld.hxx>

// DEFINE --------------------------------------------------------------------

namespace
{
    void ERRORBOX(weld::Window* pParent, const char* rid)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  ScResId(rid)));
        xBox->run();
    }
}

//  class ScSpecialFilterDialog

ScSpecialFilterDlg::ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                        const SfxItemSet&   rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, "AdvancedFilterDialog", "modules/scalc/ui/advancedfilterdialog.ui" ),

        aStrUndefined   ( ScResId(SCSTR_UNDEFINED) ),
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( static_cast<const ScQueryItem&>(
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pViewData       ( nullptr ),
        pDoc            ( nullptr ),
        pRefInputEdit   ( nullptr ),
        bRefInputMode   ( false )
{
    get(pLbFilterArea,"lbfilterarea");
    get(pEdFilterArea,"edfilterarea");
    pEdFilterArea->SetReferences(this, get<VclFrame>("filterframe")->get_label_widget());
    get(pRbFilterArea,"rbfilterarea");
    pRbFilterArea->SetReferences(this, pEdFilterArea);
    get(pBtnCase,"case");
    get(pBtnRegExp,"regexp");
    get(pBtnHeader,"header");
    get(pBtnUnique,"unique");
    get(pBtnCopyResult,"copyresult");
    get(pLbCopyArea,"lbcopyarea");
    get(pEdCopyArea,"edcopyarea");
    pEdCopyArea->SetReferences(this, pBtnCopyResult);
    get(pRbCopyArea,"rbcopyarea");
    pRbCopyArea->SetReferences(this, pEdCopyArea);
    get(pBtnDestPers,"destpers");
    get(pFtDbAreaLabel,"dbarealabel");
    get(pFtDbArea,"dbarea");
    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");
    get(pExpander,"more");

    Init( rArgSet );
    pEdFilterArea->GrabFocus();

    // hack: control of RefInput
    pIdle.reset( new Idle("Special Filter Dialog") );
    // FIXME: this is an abomination
    pIdle->SetPriority( TaskPriority::LOWEST );
    pIdle->SetInvokeHandler( LINK( this, ScSpecialFilterDlg, TimeOutHdl ) );
    pIdle->Start();
}

ScSpecialFilterDlg::~ScSpecialFilterDlg()
{
    disposeOnce();
}

void ScSpecialFilterDlg::dispose()
{
    const sal_Int32 nEntries = pLbFilterArea->GetEntryCount();

    for ( sal_Int32 i=1; i<nEntries; ++i )
        delete static_cast<OUString*>(pLbFilterArea->GetEntryData( i ));

    pOptionsMgr.reset();

    pOutItem.reset();

    // hack: control of RefInput
    pIdle->Stop();
    pIdle.reset();

    pLbFilterArea.clear();
    pEdFilterArea.clear();
    pRbFilterArea.clear();
    pExpander.clear();
    pBtnCase.clear();
    pBtnRegExp.clear();
    pBtnHeader.clear();
    pBtnUnique.clear();
    pBtnCopyResult.clear();
    pLbCopyArea.clear();
    pEdCopyArea.clear();
    pRbCopyArea.clear();
    pBtnDestPers.clear();
    pFtDbAreaLabel.clear();
    pFtDbArea.clear();
    pBtnOk.clear();
    pBtnCancel.clear();
    pRefInputEdit.clear();
    ScAnyRefDlg::dispose();
}

void ScSpecialFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    pBtnOk->SetClickHdl          ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    pBtnCancel->SetClickHdl      ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    pLbFilterArea->SetSelectHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaSelHdl ) );
    pEdFilterArea->SetModifyHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaModHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument()  : nullptr;

    pEdFilterArea->SetText( EMPTY_OUSTRING );      // may be overwritten below

    if ( pViewData && pDoc )
    {
        if(pDoc->GetChangeTrack()!=nullptr) pBtnCopyResult->Disable();

        ScRangeName* pRangeNames = pDoc->GetRangeName();
        pLbFilterArea->Clear();
        pLbFilterArea->InsertEntry( aStrUndefined, 0 );

        for (const auto& rEntry : *pRangeNames)
        {
            if (!rEntry.second->HasType(ScRangeData::Type::Criteria))
                continue;

            const sal_Int32 nInsert = pLbFilterArea->InsertEntry(rEntry.second->GetName());
            OUString aSymbol;
            rEntry.second->GetSymbol(aSymbol);
            pLbFilterArea->SetEntryData(nInsert, new OUString(aSymbol));
        }

        //  is there a stored source range?

        ScRange aAdvSource;
        if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
        {
            OUString aRefStr(aAdvSource.Format(ScRefFlags::RANGE_ABS_3D, pDoc, pDoc->GetAddressConvention()));
            pEdFilterArea->SetRefString( aRefStr );
        }
    }

    pLbFilterArea->SelectEntryPos( 0 );

    // let options be initialized:

    pOptionsMgr.reset( new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            pBtnCase,
                            pBtnRegExp,
                            pBtnHeader,
                            pBtnUnique,
                            pBtnCopyResult,
                            pBtnDestPers,
                            pLbCopyArea,
                            pEdCopyArea,
                            pRbCopyArea,
                            pFtDbAreaLabel,
                            pFtDbArea,
                            aStrUndefined ) );

    //  special filter always needs column headers
    pBtnHeader->Check();
    pBtnHeader->Disable();

    // turn on modal mode
    // SetDispatcherLock( true );
    //@BugID 54702 enable/disable in base class only
    //SFX_APPWINDOW->Disable(false);        //! general method in ScAnyRefDlg
}

bool ScSpecialFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    return DoClose( ScSpecialFilterDlgWrapper::GetChildWindowId() );
}

// Transfer of a table area selected with the mouse, which is then displayed
// as a new selection in the reference edit.

void ScSpecialFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( bRefInputMode && pRefInputEdit )       // only possible if in the reference edit mode
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        OUString aRefStr;
        const formula::FormulaGrammar::AddressConvention eConv = pDocP->GetAddressConvention();

        if ( pRefInputEdit == pEdCopyArea)
            aRefStr = rRef.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDocP, eConv);
        else if ( pRefInputEdit == pEdFilterArea)
            aRefStr = rRef.Format(ScRefFlags::RANGE_ABS_3D, pDocP, eConv);

        pRefInputEdit->SetRefString( aRefStr );
    }
}

void ScSpecialFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if ( pRefInputEdit == pEdCopyArea )
        {
            pEdCopyArea->GrabFocus();
            pEdCopyArea->GetModifyHdl().Call( *pEdCopyArea );
        }
        else if ( pRefInputEdit == pEdFilterArea )
        {
            pEdFilterArea->GrabFocus();
            FilterAreaModHdl( *pEdFilterArea );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}

ScQueryItem* ScSpecialFilterDlg::GetOutputItem( const ScQueryParam& rParam,
                                                const ScRange& rSource )
{
    pOutItem.reset(new ScQueryItem( nWhichQuery, &rParam ));
    pOutItem->SetAdvancedQuerySource( &rSource );
    return pOutItem.get();
}

bool ScSpecialFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

// Handler:

IMPL_LINK( ScSpecialFilterDlg, EndDlgHdl, Button*, pBtn, void )
{
    OSL_ENSURE( pDoc && pViewData, "Document or ViewData not found. :-/" );

    if ( (pBtn == pBtnOk) && pDoc && pViewData )
    {
        OUString          theCopyStr( pEdCopyArea->GetText() );
        OUString          theAreaStr( pEdFilterArea->GetText() );
        ScQueryParam    theOutParam( theQueryData );
        ScAddress       theAdrCopy;
        bool            bEditInputOk    = true;
        bool            bQueryOk        = false;
        ScRange         theFilterArea;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( pBtnCopyResult->IsChecked() )
        {
            sal_Int32 nColonPos = theCopyStr.indexOf( ':' );

            if ( -1 != nColonPos )
                theCopyStr = theCopyStr.copy( 0, nColonPos );

            ScRefFlags nResult = theAdrCopy.Parse( theCopyStr, pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::ZERO )
            {
                if (!pExpander->get_expanded())
                    pExpander->set_expanded(true);

                ERRORBOX(GetFrameWeld(), STR_INVALID_TABREF);
                pEdCopyArea->GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            ScRefFlags nResult = ScRange().Parse( theAreaStr, pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::ZERO )
            {
                ERRORBOX(GetFrameWeld(), STR_INVALID_TABREF);
                pEdFilterArea->GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            /*
             * All edit fields contain valid areas. Now try to create
             * a ScQueryParam from the filter area:
             */

            ScRefFlags  nResult = theFilterArea.Parse( theAreaStr, pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
            {
                ScAddress& rStart = theFilterArea.aStart;
                ScAddress& rEnd   = theFilterArea.aEnd;

                if ( pBtnCopyResult->IsChecked() )
                {
                    theOutParam.bInplace    = false;
                    theOutParam.nDestTab    = theAdrCopy.Tab();
                    theOutParam.nDestCol    = theAdrCopy.Col();
                    theOutParam.nDestRow    = theAdrCopy.Row();
                }
                else
                {
                    theOutParam.bInplace    = true;
                    theOutParam.nDestTab    = 0;
                    theOutParam.nDestCol    = 0;
                    theOutParam.nDestRow    = 0;
                }

                theOutParam.bHasHeader = pBtnHeader->IsChecked();
                theOutParam.bByRow     = true;
                theOutParam.bCaseSens  = pBtnCase->IsChecked();
                theOutParam.eSearchType = pBtnRegExp->IsChecked() ? utl::SearchParam::SearchType::Regexp :
                    utl::SearchParam::SearchType::Normal;
                theOutParam.bDuplicate = !pBtnUnique->IsChecked();
                theOutParam.bDestPers  = pBtnDestPers->IsChecked();

                bQueryOk = pDoc->CreateQueryParam(ScRange(rStart,rEnd), theOutParam);
            }
        }

        if ( bQueryOk )
        {
            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->ExecuteList(FID_FILTER_OK,
                    SfxCallMode::SLOT | SfxCallMode::RECORD,
                    { GetOutputItem(theOutParam, theFilterArea) });
            Close();
        }
        else
        {
            ERRORBOX(GetFrameWeld(), STR_INVALID_QUERYAREA);
            pEdFilterArea->GrabFocus();
        }
    }
    else if ( pBtn == pBtnCancel )
    {
        Close();
    }
}

IMPL_LINK( ScSpecialFilterDlg, TimeOutHdl, Timer*, _pIdle, void )
{
    // every 50ms check whether RefInputMode is still true

    if( (_pIdle == pIdle.get()) && IsActive() )
    {
        if( pEdCopyArea->HasFocus() || pRbCopyArea->HasFocus() )
        {
            pRefInputEdit = pEdCopyArea;
            bRefInputMode = true;
        }
        else if( pEdFilterArea->HasFocus() || pRbFilterArea->HasFocus() )
        {
            pRefInputEdit = pEdFilterArea;
            bRefInputMode = true;
        }
        else if( bRefInputMode )
        {
            pRefInputEdit = nullptr;
            bRefInputMode = false;
        }
    }

    pIdle->Start();
}

IMPL_LINK( ScSpecialFilterDlg, FilterAreaSelHdl, ListBox&, rLb, void )
{
    if ( &rLb == pLbFilterArea )
    {
        OUString  aString;
        const sal_Int32 nSelPos = pLbFilterArea->GetSelectedEntryPos();

        if ( nSelPos > 0 )
            aString = *static_cast<OUString*>(pLbFilterArea->GetEntryData( nSelPos ));

        pEdFilterArea->SetText( aString );
    }
}

IMPL_LINK( ScSpecialFilterDlg, FilterAreaModHdl, Edit&, rEd, void )
{
    if ( &rEd == pEdFilterArea )
    {
        if ( pDoc && pViewData )
        {
            OUString  theCurAreaStr = rEd.GetText();
            ScRefFlags  nResult = ScRange().Parse( theCurAreaStr, pDoc );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
            {
                const sal_Int32 nCount  = pLbFilterArea->GetEntryCount();

                for ( sal_Int32 i=1; i<nCount; ++i )
                {
                    OUString* pStr = static_cast<OUString*>(pLbFilterArea->GetEntryData( i ));
                    if (theCurAreaStr == *pStr)
                    {
                        pLbFilterArea->SelectEntryPos( i );
                        return;
                    }
                }
                pLbFilterArea->SelectEntryPos( 0 );
            }
        }
        else
            pLbFilterArea->SelectEntryPos( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
