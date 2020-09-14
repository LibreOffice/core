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

#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <dbdata.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <scresid.hxx>

#include <globstr.hrc>
#include <strings.hrc>

#include <consdlg.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace
{
    void INFOBOX(weld::Window* pWindow, const char* id)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWindow,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(id)));
        xInfoBox->run();
    }
}

class ScAreaData
{
public:
    ScAreaData()
    {
    }

    void Set( const OUString& rName, const OUString& rArea )
    {
        aStrName  = rName;
        aStrArea  = rArea;
    }

    OUString  aStrName;
    OUString  aStrArea;
};

ScConsolidateDlg::ScConsolidateDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                   const SfxItemSet& rArgSet)

    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/consolidatedialog.ui", "ConsolidateDialog")
    , aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) )
    , theConsData     ( static_cast<const ScConsolidateItem&>(
                         rArgSet.Get( rArgSet.GetPool()->
                                          GetWhich( SID_CONSOLIDATE ) )
                                    ).GetData() )
    , rViewData       ( static_cast<ScTabViewShell*>(SfxViewShell::Current())->
                              GetViewData() )
    , rDoc            ( static_cast<ScTabViewShell*>(SfxViewShell::Current())->
                              GetViewData().GetDocument() )
    , nAreaDataCount  ( 0 )
    , nWhichCons      ( rArgSet.GetPool()->GetWhich( SID_CONSOLIDATE ) )
    , bDlgLostFocus   ( false )
    , m_xLbFunc(m_xBuilder->weld_combo_box("func"))
    , m_xLbConsAreas(m_xBuilder->weld_tree_view("consareas"))
    , m_xLbDataArea(m_xBuilder->weld_combo_box("lbdataarea"))
    , m_xEdDataArea(new formula::RefEdit(m_xBuilder->weld_entry("eddataarea")))
    , m_xRbDataArea(new formula::RefButton(m_xBuilder->weld_button("rbdataarea")))
    , m_xLbDestArea(m_xBuilder->weld_combo_box("lbdestarea"))
    , m_xEdDestArea(new formula::RefEdit(m_xBuilder->weld_entry("eddestarea")))
    , m_xRbDestArea(new formula::RefButton(m_xBuilder->weld_button("rbdestarea")))
    , m_xExpander(m_xBuilder->weld_expander("more"))
    , m_xBtnByRow(m_xBuilder->weld_check_button("byrow"))
    , m_xBtnByCol(m_xBuilder->weld_check_button("bycol"))
    , m_xBtnRefs(m_xBuilder->weld_check_button("refs"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnRemove(m_xBuilder->weld_button("delete"))
    , m_xDataFT(m_xBuilder->weld_label("ftdataarea"))
    , m_xDestFT(m_xBuilder->weld_label("ftdestarea"))
{
    m_pRefInputEdit = m_xEdDataArea.get();
    Init();
}

ScConsolidateDlg::~ScConsolidateDlg()
{
}

void ScConsolidateDlg::Init()
{
    OUString aStr;
    sal_uInt16 i=0;

    m_xRbDataArea->SetReferences(this, m_xEdDataArea.get());
    m_xEdDataArea->SetReferences(this, m_xDataFT.get());
    m_xRbDestArea->SetReferences(this, m_xEdDestArea.get());
    m_xEdDestArea->SetReferences(this, m_xDestFT.get());

    m_xEdDataArea->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetEditFocusHdl ) );
    m_xEdDestArea->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetEditFocusHdl ) );
    m_xLbDataArea->connect_focus_in( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    m_xLbDestArea->connect_focus_in( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    m_xEdDataArea->SetModifyHdl( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    m_xEdDestArea->SetModifyHdl( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    m_xLbConsAreas->connect_changed( LINK( this, ScConsolidateDlg, SelectTVHdl ) );
    m_xLbDataArea->connect_changed( LINK( this, ScConsolidateDlg, SelectCBHdl ) );
    m_xLbDestArea->connect_changed( LINK( this, ScConsolidateDlg, SelectCBHdl ) );
    m_xBtnOk->connect_clicked( LINK( this, ScConsolidateDlg, OkHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScConsolidateDlg, ClickHdl ) );
    m_xBtnAdd->connect_clicked( LINK( this, ScConsolidateDlg, ClickHdl ) );
    m_xBtnRemove->connect_clicked( LINK( this, ScConsolidateDlg, ClickHdl ) );

    m_xBtnAdd->set_sensitive(false);
    m_xBtnRemove->set_sensitive(false);

    m_xBtnByRow->set_active( theConsData.bByRow );
    m_xBtnByCol->set_active( theConsData.bByCol );
    m_xBtnRefs->set_active( theConsData.bReferenceData );

    m_xLbFunc->set_active( FuncToLbPos( theConsData.eFunction ) );

    m_xLbConsAreas->set_selection_mode(SelectionMode::Multiple);
    m_xLbConsAreas->set_size_request(m_xLbConsAreas->get_approximate_digit_width() * 16,
                                     m_xLbConsAreas->get_height_rows(5));

    // read consolidation areas
    m_xLbConsAreas->clear();
    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();
    for ( i=0; i<theConsData.nDataAreaCount; i++ )
    {
        const ScArea& rArea = theConsData.pDataAreas[i];
        if ( rArea.nTab < rDoc.GetTableCount() )
        {
            aStr = ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                    rArea.nColEnd, rArea.nRowEnd, rArea.nTab ).Format( rDoc,
                        ScRefFlags::RANGE_ABS_3D, eConv );
            m_xLbConsAreas->append_text(aStr);
        }
    }

    if ( theConsData.nTab < rDoc.GetTableCount() )
    {
        aStr = ScAddress( theConsData.nCol, theConsData.nRow, theConsData.nTab
                ).Format( ScRefFlags::ADDR_ABS_3D, &rDoc, eConv );
        m_xEdDestArea->SetText( aStr );
    }
    else
        m_xEdDestArea->SetText(OUString());

    // Use the ScAreaData helper class to save those range names from the
    // RangeNames and database ranges that appear in the ListBoxes.

    ScRangeName*    pRangeNames  = rDoc.GetRangeName();
    ScDBCollection* pDbNames     = rDoc.GetDBCollection();
    size_t nRangeCount = pRangeNames ? pRangeNames->size() : 0;
    size_t nDbCount = pDbNames ? pDbNames->getNamedDBs().size() : 0;

    nAreaDataCount = nRangeCount+nDbCount;
    pAreaData      = nullptr;

    if ( nAreaDataCount > 0 )
    {
        pAreaData.reset( new ScAreaData[nAreaDataCount] );

        OUString aStrName;
        sal_uInt16 nAt = 0;
        ScRange aRange;
        ScAreaNameIterator aIter( rDoc );
        while ( aIter.Next( aStrName, aRange ) )
        {
            OUString aStrArea(aRange.Format(rDoc, ScRefFlags::ADDR_ABS_3D, eConv));
            pAreaData[nAt++].Set( aStrName, aStrArea );
        }
    }

    FillAreaLists();
    ModifyHdl( *m_xEdDestArea );
    m_xLbDataArea->set_active( 0 );
    m_xEdDataArea->SetText(OUString());
    m_xEdDataArea->GrabFocus();

    //aFlSep.SetStyle( aFlSep.GetStyle() | WB_VERT );

    //@BugID 54702 enable/disable only in base class
    //SFX_APPWINDOW->set_sensitive(true);
}

void ScConsolidateDlg::FillAreaLists()
{
    m_xLbDataArea->clear();
    m_xLbDestArea->clear();
    m_xLbDataArea->append_text( aStrUndefined );
    m_xLbDestArea->append_text( aStrUndefined );

    if ( pAreaData && (nAreaDataCount > 0) )
    {
        for ( size_t i=0;
              (i<nAreaDataCount) && (!pAreaData[i].aStrName.isEmpty());
              i++ )
        {
            m_xLbDataArea->append_text(pAreaData[i].aStrName);
            m_xLbDestArea->append_text(pAreaData[i].aStrName);
        }
    }
}

// Handover of a range within a table that has been selected by the mouse.
// This range is then shown in the reference window as new selection.

void ScConsolidateDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if ( !m_pRefInputEdit )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( m_pRefInputEdit );

    OUString      aStr;
    ScRefFlags      nFmt = ScRefFlags::RANGE_ABS_3D;       //!!! nCurTab is still missing
    const formula::FormulaGrammar::AddressConvention eConv = rDocP.GetAddressConvention();

    if ( rRef.aStart.Tab() != rRef.aEnd.Tab() )
        nFmt |= ScRefFlags::TAB2_3D;

    if ( m_pRefInputEdit == m_xEdDataArea.get())
        aStr = rRef.Format(rDocP, nFmt, eConv);
    else if ( m_pRefInputEdit == m_xEdDestArea.get() )
        aStr = rRef.aStart.Format(nFmt, &rDocP, eConv);

    m_pRefInputEdit->SetRefString( aStr );
    ModifyHdl( *m_pRefInputEdit );
}

void ScConsolidateDlg::Close()
{
    DoClose( ScConsolidateDlgWrapper::GetChildWindowId() );
}

void ScConsolidateDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;

        if ( m_pRefInputEdit )
        {
            m_pRefInputEdit->GrabFocus();
            ModifyHdl( *m_pRefInputEdit );
        }
    }
    else
        m_xDialog->grab_focus();

    RefInputDone();
}

void ScConsolidateDlg::Deactivate()
{
    bDlgLostFocus = true;
}

bool ScConsolidateDlg::VerifyEdit( formula::RefEdit* pEd )
{
    if (pEd != m_xEdDataArea.get() && pEd != m_xEdDestArea.get())
        return false;

    SCTAB    nTab    = rViewData.GetTabNo();
    bool bEditOk = false;
    OUString theCompleteStr;
    const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();

    if ( pEd == m_xEdDataArea.get() )
    {
        bEditOk = ScRangeUtil::IsAbsArea( pEd->GetText(), rDoc,
                                         nTab, &theCompleteStr, nullptr, nullptr, eConv );
    }
    else if ( pEd == m_xEdDestArea.get() )
    {
        OUString aPosStr;

        ScRangeUtil::CutPosString( pEd->GetText(), aPosStr );
        bEditOk = ScRangeUtil::IsAbsPos( aPosStr, rDoc,
                                        nTab, &theCompleteStr, nullptr, eConv );
    }

    if ( bEditOk )
        pEd->SetText( theCompleteStr );

    return bEditOk;
}

// Handler:

IMPL_LINK( ScConsolidateDlg, GetEditFocusHdl, formula::RefEdit&, rControl, void )
{
    m_pRefInputEdit = &rControl;
}

IMPL_LINK( ScConsolidateDlg, GetFocusHdl, weld::Widget&, rControl, void )
{
    if (&rControl == m_xLbDataArea.get())
        m_pRefInputEdit = m_xEdDataArea.get();
    else if (&rControl == m_xLbDestArea.get())
        m_pRefInputEdit = m_xEdDestArea.get();
}

IMPL_LINK_NOARG(ScConsolidateDlg, OkHdl, weld::Button&, void)
{
    const sal_Int32 nDataAreaCount = m_xLbConsAreas->n_children();

    if ( nDataAreaCount > 0 )
    {
        ScRefAddress aDestAddress;
        SCTAB       nTab = rViewData.GetTabNo();
        OUString    aDestPosStr( m_xEdDestArea->GetText() );
        const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();

        if ( ScRangeUtil::IsAbsPos( aDestPosStr, rDoc, nTab, nullptr, &aDestAddress, eConv ) )
        {
            ScConsolidateParam  theOutParam( theConsData );
            std::unique_ptr<ScArea[]> pDataAreas(new ScArea[nDataAreaCount]);

            for ( sal_Int32 i=0; i<nDataAreaCount; ++i )
            {
                ScRangeUtil::MakeArea(m_xLbConsAreas->get_text(i),
                                      pDataAreas[i], rDoc, nTab, eConv);
            }

            theOutParam.nCol            = aDestAddress.Col();
            theOutParam.nRow            = aDestAddress.Row();
            theOutParam.nTab            = aDestAddress.Tab();
            theOutParam.eFunction       = LbPosToFunc( m_xLbFunc->get_active() );
            theOutParam.bByCol          = m_xBtnByCol->get_active();
            theOutParam.bByRow          = m_xBtnByRow->get_active();
            theOutParam.bReferenceData  = m_xBtnRefs->get_active();
            theOutParam.SetAreas( std::move(pDataAreas), nDataAreaCount );

            ScConsolidateItem aOutItem( nWhichCons, &theOutParam );

            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->ExecuteList(SID_CONSOLIDATE,
                                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                                      { &aOutItem });
            response(RET_OK);
        }
        else
        {
            INFOBOX(m_xDialog.get(), STR_INVALID_TABREF);
            m_xEdDestArea->GrabFocus();
        }
    }
    else
        response(RET_CANCEL); // no area defined -> Cancel
}

IMPL_LINK( ScConsolidateDlg, ClickHdl, weld::Button&, rBtn, void )
{
    if ( &rBtn == m_xBtnCancel.get() )
        response(RET_CANCEL);
    else if ( &rBtn == m_xBtnAdd.get() )
    {
        if ( !m_xEdDataArea->GetText().isEmpty() )
        {
            OUString    aNewEntry( m_xEdDataArea->GetText() );
            std::unique_ptr<ScArea[]> ppAreas;
            sal_uInt16      nAreaCount = 0;
            const formula::FormulaGrammar::AddressConvention eConv = rDoc.GetAddressConvention();

            if ( ScRangeUtil::IsAbsTabArea( aNewEntry, &rDoc, &ppAreas, &nAreaCount, true, eConv ) )
            {
                // IsAbsTabArea() creates an array of ScArea pointers,
                // which have been created dynamically as well.
                // These objects need to be deleted here.

                for ( sal_uInt16 i=0; i<nAreaCount; i++ )
                {
                    const ScArea& rArea = ppAreas[i];
                    OUString aNewArea = ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                            rArea.nColEnd, rArea.nRowEnd, rArea.nTab
                            ).Format(rDoc, ScRefFlags::RANGE_ABS_3D, eConv);

                    if (m_xLbConsAreas->find_text(aNewArea) == -1)
                    {
                        m_xLbConsAreas->append_text( aNewArea );
                    }
                }
            }
            else if ( VerifyEdit( m_xEdDataArea.get() ) )
            {
                OUString aNewArea( m_xEdDataArea->GetText() );

                if (m_xLbConsAreas->find_text(aNewArea) == -1)
                    m_xLbConsAreas->append_text(aNewArea);
                else
                    INFOBOX(m_xDialog.get(), STR_AREA_ALREADY_INSERTED);
            }
            else
            {
                INFOBOX(m_xDialog.get(), STR_INVALID_TABREF);
                m_xEdDataArea->GrabFocus();
            }
        }
    }
    else if ( &rBtn == m_xBtnRemove.get() )
    {
        std::vector<int> aSelectedRows(m_xLbConsAreas->get_selected_rows());
        std::sort(aSelectedRows.begin(), aSelectedRows.end());
        for (auto it = aSelectedRows.rbegin(); it != aSelectedRows.rend(); ++it)
            m_xLbConsAreas->remove(*it);
        m_xBtnRemove->set_sensitive(false);
    }
}

IMPL_LINK( ScConsolidateDlg, SelectTVHdl, weld::TreeView&, rLb, void )
{
    if (rLb.get_selected_index() != -1)
        m_xBtnRemove->set_sensitive(true);
    else
        m_xBtnRemove->set_sensitive(false);
}

IMPL_LINK( ScConsolidateDlg, SelectCBHdl, weld::ComboBox&, rLb, void )
{
    formula::RefEdit* pEd = (&rLb == m_xLbDataArea.get()) ? m_xEdDataArea.get() : m_xEdDestArea.get();
    const sal_Int32 nSelPos = rLb.get_active();

    if (   (nSelPos > 0)
        && (nAreaDataCount > 0)
        && (pAreaData != nullptr) )
    {
        if ( o3tl::make_unsigned(nSelPos) <= nAreaDataCount )
        {
            OUString aString( pAreaData[nSelPos-1].aStrArea );

            if ( &rLb == m_xLbDestArea.get() )
                ScRangeUtil::CutPosString( aString, aString );

            pEd->SetText( aString );

            if ( pEd == m_xEdDataArea.get() )
                m_xBtnAdd->set_sensitive(true);
        }
    }
    else
    {
        pEd->SetText( EMPTY_OUSTRING );
        if ( pEd == m_xEdDataArea.get() )
            m_xBtnAdd->set_sensitive(true);
    }
}

IMPL_LINK( ScConsolidateDlg, ModifyHdl, formula::RefEdit&, rEd, void )
{
    if ( &rEd == m_xEdDataArea.get() )
    {
        OUString aAreaStr( rEd.GetText() );
        if ( !aAreaStr.isEmpty() )
            m_xBtnAdd->set_sensitive(true);
        else
            m_xBtnAdd->set_sensitive(false);
    }
    else if ( &rEd == m_xEdDestArea.get() )
    {
        m_xLbDestArea->set_active(0);
    }
}

// TODO: generalize!
// Resource of the ListBox and these two conversion methods are also in
// tpsubt and everywhere, where StarCalc functions are selectable.

ScSubTotalFunc ScConsolidateDlg::LbPosToFunc( sal_Int32 nPos )
{
    switch ( nPos )
    {
        case  2:    return SUBTOTAL_FUNC_AVE;
        case  6:    return SUBTOTAL_FUNC_CNT;
        case  1:    return SUBTOTAL_FUNC_CNT2;
        case  3:    return SUBTOTAL_FUNC_MAX;
        case  4:    return SUBTOTAL_FUNC_MIN;
        case  5:    return SUBTOTAL_FUNC_PROD;
        case  7:    return SUBTOTAL_FUNC_STD;
        case  8:    return SUBTOTAL_FUNC_STDP;
        case  9:    return SUBTOTAL_FUNC_VAR;
        case 10:    return SUBTOTAL_FUNC_VARP;
        case  0:
        default:
            return SUBTOTAL_FUNC_SUM;
    }
}

sal_Int32 ScConsolidateDlg::FuncToLbPos( ScSubTotalFunc eFunc )
{
    switch ( eFunc )
    {
        case SUBTOTAL_FUNC_AVE:     return 2;
        case SUBTOTAL_FUNC_CNT:     return 6;
        case SUBTOTAL_FUNC_CNT2:    return 1;
        case SUBTOTAL_FUNC_MAX:     return 3;
        case SUBTOTAL_FUNC_MIN:     return 4;
        case SUBTOTAL_FUNC_PROD:    return 5;
        case SUBTOTAL_FUNC_STD:     return 7;
        case SUBTOTAL_FUNC_STDP:    return 8;
        case SUBTOTAL_FUNC_VAR:     return 9;
        case SUBTOTAL_FUNC_VARP:    return 10;
        case SUBTOTAL_FUNC_NONE:
        case SUBTOTAL_FUNC_SUM:
        default:
            return 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
