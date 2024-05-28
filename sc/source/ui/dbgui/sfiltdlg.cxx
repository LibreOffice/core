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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

// DEFINE --------------------------------------------------------------------

namespace
{
    void ERRORBOX(weld::Window* pParent, TranslateId rid)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  ScResId(rid)));
        xBox->run();
    }
}


ScSpecialFilterDlg::ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                        const SfxItemSet&   rArgSet )

    : ScAnyRefDlgController(pB, pCW, pParent, u"modules/scalc/ui/advancedfilterdialog.ui"_ustr, u"AdvancedFilterDialog"_ustr)
    , aStrUndefined   ( ScResId(SCSTR_UNDEFINED) )
    , nWhichQuery     ( rArgSet.GetPool()->GetWhichIDFromSlotID( SID_QUERY ) )
    , theQueryData    ( static_cast<const ScQueryItem&>(
                           rArgSet.Get( nWhichQuery )).GetQueryData() )
    , pViewData(nullptr)
    , pDoc(nullptr)
    , bRefInputMode(false)
    , m_pRefInputEdit(nullptr)
    , m_xLbFilterArea(m_xBuilder->weld_combo_box(u"lbfilterarea"_ustr))
    , m_xEdFilterArea(new formula::RefEdit(m_xBuilder->weld_entry(u"edfilterarea"_ustr)))
    , m_xRbFilterArea(new formula::RefButton(m_xBuilder->weld_button(u"rbfilterarea"_ustr)))
    , m_xExpander(m_xBuilder->weld_expander(u"more"_ustr))
    , m_xBtnCase(m_xBuilder->weld_check_button(u"case"_ustr))
    , m_xBtnRegExp(m_xBuilder->weld_check_button(u"regexp"_ustr))
    , m_xBtnHeader(m_xBuilder->weld_check_button(u"header"_ustr))
    , m_xBtnUnique(m_xBuilder->weld_check_button(u"unique"_ustr))
    , m_xBtnCopyResult(m_xBuilder->weld_check_button(u"copyresult"_ustr))
    , m_xLbCopyArea(m_xBuilder->weld_combo_box(u"lbcopyarea"_ustr))
    , m_xEdCopyArea(new formula::RefEdit(m_xBuilder->weld_entry(u"edcopyarea"_ustr)))
    , m_xRbCopyArea(new formula::RefButton(m_xBuilder->weld_button(u"rbcopyarea"_ustr)))
    , m_xBtnDestPers(m_xBuilder->weld_check_button(u"destpers"_ustr))
    , m_xFtDbAreaLabel(m_xBuilder->weld_label(u"dbarealabel"_ustr))
    , m_xFtDbArea(m_xBuilder->weld_label(u"dbarea"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBtnCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xFilterFrame(m_xBuilder->weld_frame(u"filterframe"_ustr))
    , m_xFilterLabel(m_xFilterFrame->weld_label_widget())
{
    m_xEdFilterArea->SetReferences(this, m_xFilterLabel.get());
    m_xRbFilterArea->SetReferences(this, m_xEdFilterArea.get());
    m_xEdCopyArea->SetReferences(this, m_xFtDbAreaLabel.get());
    m_xRbCopyArea->SetReferences(this, m_xEdCopyArea.get());

    Init( rArgSet );

    Link<formula::RefEdit&, void> aLinkEdit = LINK(this, ScSpecialFilterDlg, RefInputEditHdl);
    Link<formula::RefButton&, void> aLinkButton = LINK(this, ScSpecialFilterDlg, RefInputButtonHdl);
    m_xEdCopyArea->SetGetFocusHdl(aLinkEdit);
    m_xRbCopyArea->SetGetFocusHdl(aLinkButton);
    m_xEdFilterArea->SetGetFocusHdl(aLinkEdit);
    m_xRbFilterArea->SetGetFocusHdl(aLinkButton);
    m_xEdCopyArea->SetLoseFocusHdl(aLinkEdit);
    m_xRbCopyArea->SetLoseFocusHdl(aLinkButton);
    m_xEdFilterArea->SetLoseFocusHdl(aLinkEdit);
    m_xRbFilterArea->SetLoseFocusHdl(aLinkButton);

    m_xEdFilterArea->GrabFocus();
}

ScSpecialFilterDlg::~ScSpecialFilterDlg()
{
    pOptionsMgr.reset();

    pOutItem.reset();
}

void ScSpecialFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    m_xBtnOk->connect_clicked( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    m_xLbFilterArea->connect_changed( LINK( this, ScSpecialFilterDlg, FilterAreaSelHdl ) );
    m_xEdFilterArea->SetModifyHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaModHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? &pViewData->GetDocument() : nullptr;

    m_xEdFilterArea->SetText( OUString() );      // may be overwritten below

    if ( pViewData && pDoc )
    {
        if(pDoc->GetChangeTrack()!=nullptr) m_xBtnCopyResult->set_sensitive(false);

        ScRangeName* pRangeNames = pDoc->GetRangeName();
        m_xLbFilterArea->clear();
        m_xLbFilterArea->append_text(aStrUndefined);

        for (const auto& rEntry : *pRangeNames)
        {
            if (!rEntry.second->HasType(ScRangeData::Type::Criteria))
                continue;

            OUString aSymbol = rEntry.second->GetSymbol();
            m_xLbFilterArea->append(aSymbol, rEntry.second->GetName());
        }

        //  is there a stored source range?

        ScRange aAdvSource;
        if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
        {
            OUString aRefStr(aAdvSource.Format(*pDoc, ScRefFlags::RANGE_ABS_3D, pDoc->GetAddressConvention()));
            m_xEdFilterArea->SetRefString( aRefStr );
        }
    }

    m_xLbFilterArea->set_active( 0 );

    // let options be initialized:

    pOptionsMgr.reset( new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            m_xBtnCase.get(),
                            m_xBtnRegExp.get(),
                            m_xBtnHeader.get(),
                            m_xBtnUnique.get(),
                            m_xBtnCopyResult.get(),
                            m_xBtnDestPers.get(),
                            m_xLbCopyArea.get(),
                            m_xEdCopyArea.get(),
                            m_xRbCopyArea.get(),
                            m_xFtDbAreaLabel.get(),
                            m_xFtDbArea.get(),
                            aStrUndefined ) );

    //  special filter always needs column headers
    m_xBtnHeader->set_active(true);
    m_xBtnHeader->set_sensitive(false);

    // turn on modal mode
    // SetDispatcherLock( true );
    //@BugID 54702 enable/disable in base class only
    //SFX_APPWINDOW->Disable(false);        //! general method in ScAnyRefDlg
}

void ScSpecialFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    DoClose( ScSpecialFilterDlgWrapper::GetChildWindowId() );
}

// Transfer of a table area selected with the mouse, which is then displayed
// as a new selection in the reference edit.

void ScSpecialFilterDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if ( !(bRefInputMode && m_pRefInputEdit) )       // only possible if in the reference edit mode
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( m_pRefInputEdit );

    OUString aRefStr;
    const formula::FormulaGrammar::AddressConvention eConv = rDocP.GetAddressConvention();

    if (m_pRefInputEdit == m_xEdCopyArea.get())
        aRefStr = rRef.aStart.Format(ScRefFlags::ADDR_ABS_3D, &rDocP, eConv);
    else if (m_pRefInputEdit == m_xEdFilterArea.get())
        aRefStr = rRef.Format(rDocP, ScRefFlags::RANGE_ABS_3D, eConv);

    m_pRefInputEdit->SetRefString( aRefStr );
}

void ScSpecialFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if (m_pRefInputEdit == m_xEdCopyArea.get())
        {
            m_xEdCopyArea->GrabFocus();
            m_xEdCopyArea->GetModifyHdl().Call( *m_xEdCopyArea );
        }
        else if (m_pRefInputEdit == m_xEdFilterArea.get())
        {
            m_xEdFilterArea->GrabFocus();
            FilterAreaModHdl( *m_xEdFilterArea );
        }
    }
    else
        m_xDialog->grab_focus();

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

IMPL_LINK(ScSpecialFilterDlg, EndDlgHdl, weld::Button&, rBtn, void)
{
    OSL_ENSURE( pDoc && pViewData, "Document or ViewData not found. :-/" );

    if (&rBtn == m_xBtnOk.get() && pDoc && pViewData)
    {
        OUString          theCopyStr( m_xEdCopyArea->GetText() );
        OUString          theAreaStr( m_xEdFilterArea->GetText() );
        ScQueryParam    theOutParam( theQueryData );
        ScAddress       theAdrCopy;
        bool            bEditInputOk    = true;
        bool            bQueryOk        = false;
        ScRange         theFilterArea;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( m_xBtnCopyResult->get_active() )
        {
            sal_Int32 nColonPos = theCopyStr.indexOf( ':' );

            if ( -1 != nColonPos )
                theCopyStr = theCopyStr.copy( 0, nColonPos );

            ScRefFlags nResult = theAdrCopy.Parse( theCopyStr, *pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::ZERO )
            {
                if (!m_xExpander->get_expanded())
                    m_xExpander->set_expanded(true);

                ERRORBOX(m_xDialog.get(), STR_INVALID_TABREF);
                m_xEdCopyArea->GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            ScRefFlags nResult = ScRange().Parse( theAreaStr, *pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::ZERO )
            {
                ERRORBOX(m_xDialog.get(), STR_INVALID_TABREF);
                m_xEdFilterArea->GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            /*
             * All edit fields contain valid areas. Now try to create
             * a ScQueryParam from the filter area:
             */

            ScRefFlags  nResult = theFilterArea.Parse( theAreaStr, *pDoc, eConv );

            if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
            {
                ScAddress& rStart = theFilterArea.aStart;
                ScAddress& rEnd   = theFilterArea.aEnd;

                if ( m_xBtnCopyResult->get_active() )
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

                theOutParam.bHasHeader = m_xBtnHeader->get_active();
                theOutParam.bByRow     = true;
                theOutParam.bCaseSens  = m_xBtnCase->get_active();
                theOutParam.eSearchType = m_xBtnRegExp->get_active() ? utl::SearchParam::SearchType::Regexp :
                    utl::SearchParam::SearchType::Normal;
                theOutParam.bDuplicate = !m_xBtnUnique->get_active();
                theOutParam.bDestPers  = m_xBtnDestPers->get_active();

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
            response(RET_OK);
        }
        else
        {
            ERRORBOX(m_xDialog.get(), STR_INVALID_QUERYAREA);
            m_xEdFilterArea->GrabFocus();
        }
    }
    else if (&rBtn == m_xBtnCancel.get())
    {
        response(RET_CANCEL);
    }
}

IMPL_LINK_NOARG(ScSpecialFilterDlg, RefInputEditHdl, formula::RefEdit&, void)
{
    RefInputHdl();
}

IMPL_LINK_NOARG(ScSpecialFilterDlg, RefInputButtonHdl, formula::RefButton&, void)
{
    RefInputHdl();
}

void ScSpecialFilterDlg::RefInputHdl()
{
    if (!m_xDialog->has_toplevel_focus())
        return;

    if( m_xEdCopyArea->GetWidget()->has_focus() || m_xRbCopyArea->GetWidget()->has_focus() )
    {
        m_pRefInputEdit = m_xEdCopyArea.get();
        bRefInputMode = true;
    }
    else if( m_xEdFilterArea->GetWidget()->has_focus() || m_xRbFilterArea->GetWidget()->has_focus() )
    {
        m_pRefInputEdit = m_xEdFilterArea.get();
        bRefInputMode = true;
    }
    else if( bRefInputMode )
    {
        m_pRefInputEdit = nullptr;
        bRefInputMode = false;
    }
}

IMPL_LINK(ScSpecialFilterDlg, FilterAreaSelHdl, weld::ComboBox&, rLb, void)
{
    if (&rLb == m_xLbFilterArea.get())
    {
        OUString  aString;
        const sal_Int32 nSelPos = m_xLbFilterArea->get_active();

        if ( nSelPos > 0 )
            aString = m_xLbFilterArea->get_id(nSelPos);

        m_xEdFilterArea->SetText( aString );
    }
}

IMPL_LINK( ScSpecialFilterDlg, FilterAreaModHdl, formula::RefEdit&, rEd, void )
{
    if (&rEd != m_xEdFilterArea.get())
        return;

    if ( pDoc && pViewData )
    {
        OUString  theCurAreaStr = rEd.GetText();
        ScRefFlags  nResult = ScRange().Parse( theCurAreaStr, *pDoc );

        if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
        {
            const sal_Int32 nCount  = m_xLbFilterArea->get_count();
            for (sal_Int32 i = 1; i < nCount; ++i)
            {
                OUString aStr = m_xLbFilterArea->get_id(i);
                if (theCurAreaStr == aStr)
                {
                    m_xLbFilterArea->set_active( i );
                    return;
                }
            }
            m_xLbFilterArea->set_active( 0 );
        }
    }
    else
        m_xLbFilterArea->set_active( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
