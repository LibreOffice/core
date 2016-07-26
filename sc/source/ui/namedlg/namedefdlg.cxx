/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "namedefdlg.hxx"

#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

#include <sfx2/app.hxx>

#include "document.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "rangenam.hxx"
#include "reffact.hxx"
#include "undorangename.hxx"
#include "tabvwsh.hxx"
#include "tokenarray.hxx"
#include "sc.hrc"

ScNameDefDlg::ScNameDefDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
        ScViewData* pViewData, const std::map<OUString, ScRangeName*>& aRangeMap,
        const ScAddress& aCursorPos, const bool bUndo )
    : ScAnyRefDlg( pB, pCW, pParent, "DefineNameDialog", "modules/scalc/ui/definename.ui" )
    ,
    mbUndo( bUndo ),
    mpDoc( pViewData->GetDocument() ),
    mpDocShell ( pViewData->GetDocShell() ),
    maCursorPos( aCursorPos ),

    maGlobalNameStr  ( ScGlobal::GetRscString(STR_GLOBAL_SCOPE) ),
    maErrInvalidNameStr( ScGlobal::GetRscString(STR_ERR_NAME_INVALID)),
    maErrNameInUse   ( ScGlobal::GetRscString(STR_ERR_NAME_EXISTS)),
    maRangeMap( aRangeMap )
{
    get(m_pEdName, "edit");
    get(m_pEdRange, "range");
    m_pEdRange->SetReferences(this, m_pEdName);
    get(m_pRbRange, "refbutton");
    m_pRbRange->SetReferences(this, m_pEdRange);
    get(m_pLbScope, "scope");
    get(m_pBtnRowHeader, "rowheader");
    get(m_pBtnColHeader, "colheader");
    get(m_pBtnPrintArea, "printarea");
    get(m_pBtnCriteria, "filter");
    get(m_pBtnAdd, "add");
    get(m_pBtnCancel, "cancel");
    get(m_pFtInfo, "label");
    maStrInfoDefault = m_pFtInfo->GetText();

    // Initialize scope list.
    m_pLbScope->InsertEntry(maGlobalNameStr);
    m_pLbScope->SelectEntryPos(0);
    SCTAB n = mpDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        OUString aTabName;
        mpDoc->GetName(i, aTabName);
        m_pLbScope->InsertEntry(aTabName);
    }

    m_pBtnCancel->SetClickHdl( LINK( this, ScNameDefDlg, CancelBtnHdl));
    m_pBtnAdd->SetClickHdl( LINK( this, ScNameDefDlg, AddBtnHdl ));
    m_pEdName->SetModifyHdl( LINK( this, ScNameDefDlg, NameModifyHdl ));
    m_pEdRange->SetGetFocusHdl( LINK( this, ScNameDefDlg, AssignGetFocusHdl ) );

    m_pBtnAdd->Disable(); // empty name is invalid

    ScRange aRange;

    pViewData->GetSimpleArea( aRange );
    OUString aAreaStr(aRange.Format(ScRefFlags::RANGE_ABS_3D, mpDoc,
            ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0)));

    m_pEdRange->SetText( aAreaStr );

    Selection aCurSel = Selection( 0, SELECTION_MAX );
    m_pEdName->GrabFocus();
    m_pEdName->SetSelection( aCurSel );
}

ScNameDefDlg::~ScNameDefDlg()
{
    disposeOnce();
}

void ScNameDefDlg::dispose()
{
    m_pEdName.clear();
    m_pEdRange.clear();
    m_pRbRange.clear();
    m_pLbScope.clear();
    m_pBtnRowHeader.clear();
    m_pBtnColHeader.clear();
    m_pBtnPrintArea.clear();
    m_pBtnCriteria.clear();
    m_pBtnAdd.clear();
    m_pBtnCancel.clear();
    m_pFtInfo.clear();
    ScAnyRefDlg::dispose();
}

void ScNameDefDlg::CancelPushed()
{
    if (mbUndo)
        Close();
    else
    {
        ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
        pViewSh->SwitchBetweenRefDialogs(this);
    }
}

bool ScNameDefDlg::IsFormulaValid()
{
    ScCompiler aComp( mpDoc, maCursorPos);
    aComp.SetGrammar( mpDoc->GetGrammar() );
    ScTokenArray* pCode = aComp.CompileString(m_pEdRange->GetText());
    if (pCode->GetCodeError())
    {
        //TODO: info message
        delete pCode;
        return false;
    }
    else
    {
        delete pCode;
        return true;
    }
}

bool ScNameDefDlg::IsNameValid()
{
    OUString aScope = m_pLbScope->GetSelectEntry();
    OUString aName = m_pEdName->GetText();

    ScRangeName* pRangeName = nullptr;
    if(aScope == maGlobalNameStr)
    {
        pRangeName = maRangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second;
    }
    else
    {
        pRangeName = maRangeMap.find(aScope)->second;
    }

    m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    if ( aName.isEmpty() )
    {
        m_pBtnAdd->Disable();
        m_pFtInfo->SetText(maStrInfoDefault);
        return false;
    }
    else if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        m_pFtInfo->SetText(maErrInvalidNameStr);
        m_pBtnAdd->Disable();
        return false;
    }
    else if (pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        m_pFtInfo->SetText(maErrNameInUse);
        m_pBtnAdd->Disable();
        return false;
    }

    if (!IsFormulaValid())
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        m_pBtnAdd->Disable();
        return false;
    }

    m_pFtInfo->SetText(maStrInfoDefault);
    m_pBtnAdd->Enable();
    return true;
}

void ScNameDefDlg::AddPushed()
{
    OUString aScope = m_pLbScope->GetSelectEntry();
    OUString aName = m_pEdName->GetText();
    OUString aExpression = m_pEdRange->GetText();

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
        if ( mpDoc )
        {
            ScRangeData::Type nType = ScRangeData::Type::Name;

            ScRangeData* pNewEntry = new ScRangeData( mpDoc,
                    aName,
                    aExpression,
                    maCursorPos,
                    nType );

            if ( m_pBtnRowHeader->IsChecked() ) nType |= ScRangeData::Type::RowHeader;
            if ( m_pBtnColHeader->IsChecked() ) nType |= ScRangeData::Type::ColHeader;
            if ( m_pBtnPrintArea->IsChecked() ) nType |= ScRangeData::Type::PrintArea;
            if ( m_pBtnCriteria->IsChecked()  ) nType |= ScRangeData::Type::Criteria;

            pNewEntry->AddType(nType);

            // aExpression valid?
            if ( 0 == pNewEntry->GetErrCode() )
            {
                if ( !pRangeName->insert( pNewEntry, false /*bReuseFreeIndex*/ ) )
                    pNewEntry = nullptr;

                if (mbUndo)
                {
                    // this means we called directly through the menu

                    SCTAB nTab;
                    // if no table with that name is found, assume global range name
                    if (!mpDoc->GetTable(aScope, nTab))
                        nTab = -1;

                    assert( pNewEntry);     // undo of no insertion smells fishy
                    if (pNewEntry)
                        mpDocShell->GetUndoManager()->AddUndoAction(
                                new ScUndoAddRangeData( mpDocShell, pNewEntry, nTab) );

                    // set table stream invalid, otherwise RangeName won't be saved if no other
                    // call invalidates the stream
                    if (nTab != -1)
                        mpDoc->SetStreamValid(nTab, false);
                    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
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
                Selection aCurSel = Selection( 0, SELECTION_MAX );
                m_pEdRange->GrabFocus();
                m_pEdRange->SetSelection( aCurSel );
            }
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
    return m_pEdRange->IsEnabled();
}

void ScNameDefDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    IsNameValid();
}

void ScNameDefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdRange->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdRange);
        OUString aRefStr(rRef.Format(ScRefFlags::RANGE_ABS_3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0)));
        m_pEdRange->SetRefString( aRefStr );
    }
}

bool ScNameDefDlg::Close()
{
    return DoClose( ScNameDefDlgWrapper::GetChildWindowId() );
}

void ScNameDefDlg::SetActive()
{
    m_pEdRange->GrabFocus();
    RefInputDone();
}

IMPL_LINK_NOARG_TYPED(ScNameDefDlg, CancelBtnHdl, Button*, void)
{
    CancelPushed();
}

IMPL_LINK_NOARG_TYPED(ScNameDefDlg, AddBtnHdl, Button*, void)
{
    AddPushed();
};

IMPL_LINK_NOARG_TYPED(ScNameDefDlg, NameModifyHdl, Edit&, void)
{
    IsNameValid();
}

IMPL_LINK_NOARG_TYPED(ScNameDefDlg, AssignGetFocusHdl, Control&, void)
{
    IsNameValid();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
