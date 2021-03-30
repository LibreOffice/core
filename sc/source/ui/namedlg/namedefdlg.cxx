/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <namedefdlg.hxx>

#include <formula/errorcodes.hxx>
#include <sfx2/app.hxx>
#include <unotools/charclass.hxx>

#include <compiler.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <globalnames.hxx>
#include <rangenam.hxx>
#include <reffact.hxx>
#include <undorangename.hxx>
#include <tabvwsh.hxx>
#include <tokenarray.hxx>

ScNameDefDlg::ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
        const ScViewData& rViewData, const std::map<OUString, ScRangeName*>& aRangeMap,
        const ScAddress& aCursorPos, const bool bUndo )
    : ScAnyRefDlgController( pB, pCW, pParent, "modules/scalc/ui/definename.ui", "DefineNameDialog")
    , mbUndo( bUndo )
    , mrDoc(rViewData.GetDocument())
    , mpDocShell ( rViewData.GetDocShell() )
    , maCursorPos( aCursorPos )
    , maGlobalNameStr  ( ScResId(STR_GLOBAL_SCOPE) )
    , maErrInvalidNameStr( ScResId(STR_ERR_NAME_INVALID))
    , maErrInvalidNameCellRefStr( ScResId(STR_ERR_NAME_INVALID_CELL_REF))
    , maErrNameInUse   ( ScResId(STR_ERR_NAME_EXISTS))
    , maRangeMap( aRangeMap )
    , m_xEdName(m_xBuilder->weld_entry("edit"))
    , m_xEdRange(new formula::RefEdit(m_xBuilder->weld_entry("range")))
    , m_xRbRange(new formula::RefButton(m_xBuilder->weld_button("refbutton")))
    , m_xLbScope(m_xBuilder->weld_combo_box("scope"))
    , m_xBtnRowHeader(m_xBuilder->weld_check_button("rowheader"))
    , m_xBtnColHeader(m_xBuilder->weld_check_button("colheader"))
    , m_xBtnPrintArea(m_xBuilder->weld_check_button("printarea"))
    , m_xBtnCriteria(m_xBuilder->weld_check_button("filter"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xFtInfo(m_xBuilder->weld_label("label"))
    , m_xExpander(m_xBuilder->weld_expander("more"))
    , m_xFtRange(m_xBuilder->weld_label("label3"))
{
    m_xEdRange->SetReferences(this, m_xFtRange.get());
    m_xRbRange->SetReferences(this, m_xEdRange.get());
    maStrInfoDefault = m_xFtInfo->get_label();

    // Initialize scope list.
    m_xLbScope->append_text(maGlobalNameStr);
    m_xLbScope->set_active(0);
    SCTAB n = mrDoc.GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        OUString aTabName;
        mrDoc.GetName(i, aTabName);
        m_xLbScope->append_text(aTabName);
    }

    m_xBtnCancel->connect_clicked( LINK( this, ScNameDefDlg, CancelBtnHdl));
    m_xBtnAdd->connect_clicked( LINK( this, ScNameDefDlg, AddBtnHdl ));
    m_xEdName->connect_changed( LINK( this, ScNameDefDlg, NameModifyHdl ));
    m_xEdRange->SetGetFocusHdl( LINK( this, ScNameDefDlg, AssignGetFocusHdl ) );

    m_xBtnAdd->set_sensitive(false); // empty name is invalid

    ScRange aRange;

    rViewData.GetSimpleArea( aRange );
    OUString aAreaStr(aRange.Format(mrDoc, ScRefFlags::RANGE_ABS_3D,
            ScAddress::Details(mrDoc.GetAddressConvention(), 0, 0)));

    m_xEdRange->SetText( aAreaStr );

    m_xEdName->grab_focus();
    m_xEdName->select_region(0, -1);
}

ScNameDefDlg::~ScNameDefDlg()
{
}

void ScNameDefDlg::CancelPushed()
{
    if (mbUndo)
        response(RET_CANCEL);
    else
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        pViewSh->SwitchBetweenRefDialogs(this);
    }
}

bool ScNameDefDlg::IsFormulaValid()
{
    ScCompiler aComp(mrDoc, maCursorPos, mrDoc.GetGrammar());
    std::unique_ptr<ScTokenArray> pCode = aComp.CompileString(m_xEdRange->GetText());
    if (pCode->GetCodeError() != FormulaError::NONE)
    {
        //TODO: info message
        return false;
    }
    else
    {
        return true;
    }
}

bool ScNameDefDlg::IsNameValid()
{
    OUString aScope = m_xLbScope->get_active_text();
    OUString aName = m_xEdName->get_text();

    ScRangeName* pRangeName = nullptr;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }

    ScRangeData::IsNameValidType eType;
    m_xFtInfo->set_label_type(weld::LabelType::Normal);
    if ( aName.isEmpty() )
    {
        m_xBtnAdd->set_sensitive(false);
        m_xFtInfo->set_label(maStrInfoDefault);
        return false;
    }
    else if ((eType = ScRangeData::IsNameValid(aName, mrDoc))
             != ScRangeData::IsNameValidType::NAME_VALID)
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        if (eType == ScRangeData::IsNameValidType::NAME_INVALID_BAD_STRING)
        {
            m_xFtInfo->set_label(maErrInvalidNameStr);
        }
        else if (eType == ScRangeData::IsNameValidType::NAME_INVALID_CELL_REF)
        {
            m_xFtInfo->set_label(maErrInvalidNameCellRefStr);
        }
        m_xBtnAdd->set_sensitive(false);
        return false;
    }
    else if (pRangeName->findByUpperName(ScGlobal::getCharClassPtr()->uppercase(aName)))
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        m_xFtInfo->set_label(maErrNameInUse);
        m_xBtnAdd->set_sensitive(false);
        return false;
    }

    if (!IsFormulaValid())
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        m_xBtnAdd->set_sensitive(false);
        return false;
    }

    m_xFtInfo->set_label(maStrInfoDefault);
    m_xBtnAdd->set_sensitive(true);
    return true;
}

void ScNameDefDlg::AddPushed()
{
    OUString aScope = m_xLbScope->get_active_text();
    OUString aName = m_xEdName->get_text();
    OUString aExpression = m_xEdRange->GetText();

    if (aName.isEmpty())
    {
        return;
    }
    if (aScope.isEmpty())
    {
        return;
    }

    ScRangeName* pRangeName = nullptr;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }
    if (!pRangeName)
        return;

    if (!IsNameValid()) //should not happen, but make sure we don't break anything
        return;
    else
    {
        ScRangeData::Type nType = ScRangeData::Type::Name;

        ScRangeData* pNewEntry = new ScRangeData( mrDoc,
                aName,
                aExpression,
                maCursorPos,
                nType );

        if ( m_xBtnRowHeader->get_active() ) nType |= ScRangeData::Type::RowHeader;
        if ( m_xBtnColHeader->get_active() ) nType |= ScRangeData::Type::ColHeader;
        if ( m_xBtnPrintArea->get_active() ) nType |= ScRangeData::Type::PrintArea;
        if ( m_xBtnCriteria->get_active()  ) nType |= ScRangeData::Type::Criteria;

        pNewEntry->AddType(nType);

        // aExpression valid?
        if ( FormulaError::NONE == pNewEntry->GetErrCode() )
        {
            if ( !pRangeName->insert( pNewEntry, false /*bReuseFreeIndex*/ ) )
                pNewEntry = nullptr;

            if (mbUndo)
            {
                // this means we called directly through the menu

                SCTAB nTab;
                // if no table with that name is found, assume global range name
                if (!mrDoc.GetTable(aScope, nTab))
                    nTab = -1;

                assert( pNewEntry);     // undo of no insertion smells fishy
                if (pNewEntry)
                    mpDocShell->GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoAddRangeData>( mpDocShell, pNewEntry, nTab) );

                // set table stream invalid, otherwise RangeName won't be saved if no other
                // call invalidates the stream
                if (nTab != -1)
                    mrDoc.SetStreamValid(nTab, false);
                SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScAreasChanged ) );
                mpDocShell->SetDocumentModified();
                Close();
            }
            else
            {
                maName = aName;
                maScope = aScope;
                ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
                pViewSh->SwitchBetweenRefDialogs(this);
            }
        }
        else
        {
            delete pNewEntry;
            m_xEdRange->GrabFocus();
            m_xEdRange->SelectAll();
        }
    }
}

void ScNameDefDlg::GetNewData(OUString& rName, OUString& rScope)
{
    rName = maName;
    rScope = maScope;
}

bool ScNameDefDlg::IsRefInputMode() const
{
    return m_xEdRange->GetWidget()->get_sensitive();
}

void ScNameDefDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlgController::RefInputDone(bForced);
    IsNameValid();
}

void ScNameDefDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (m_xEdRange->GetWidget()->get_sensitive())
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_xEdRange.get());
        OUString aRefStr(rRef.Format(rDocP, ScRefFlags::RANGE_ABS_3D,
                ScAddress::Details(rDocP.GetAddressConvention(), 0, 0)));
        m_xEdRange->SetRefString( aRefStr );
    }
}

void ScNameDefDlg::Close()
{
    DoClose( ScNameDefDlgWrapper::GetChildWindowId() );
}

void ScNameDefDlg::SetActive()
{
    m_xEdRange->GrabFocus();
    RefInputDone();
}

IMPL_LINK_NOARG(ScNameDefDlg, CancelBtnHdl, weld::Button&, void)
{
    CancelPushed();
}

IMPL_LINK_NOARG(ScNameDefDlg, AddBtnHdl, weld::Button&, void)
{
    AddPushed();
};

IMPL_LINK_NOARG(ScNameDefDlg, NameModifyHdl, weld::Entry&, void)
{
    IsNameValid();
}

IMPL_LINK_NOARG(ScNameDefDlg, AssignGetFocusHdl, formula::RefEdit&, void)
{
    IsNameValid();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
