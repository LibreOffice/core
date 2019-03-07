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

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/navigat.hxx>
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <unotools/charclass.hxx>

#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <rangeutl.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <scresid.hxx>
#include <scmod.hxx>
#include <navicfg.hxx>
#include <navcitem.hxx>
#include <navipi.hxx>
#include <navsett.hxx>
#include <markdata.hxx>

#include <com/sun/star/uno/Reference.hxx>

using namespace com::sun::star;

//  maximum values for UI
#define SCNAV_MAXCOL        (MAXCOLCOUNT)
// macro is sufficient since only used in ctor
#define SCNAV_COLDIGITS     (static_cast<sal_Int32>( floor( log10( static_cast<double>(SCNAV_MAXCOL)))) + 1)   // 1...256...18278
// precomputed constant because it is used in every change of spin button field
static const sal_Int32 SCNAV_COLLETTERS = ::ScColToAlpha(SCNAV_MAXCOL).getLength();    // A...IV...ZZZ

#define SCNAV_MAXROW        (MAXROWCOUNT)

void ScNavigatorDlg::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();
        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

ColumnEdit::ColumnEdit(Window* pParent, WinBits nWinBits)
    : SpinField(pParent, nWinBits)
    , nCol(0)
{
    SetMaxTextLen(SCNAV_COLDIGITS);   // 1...256...18278 or A...IV...ZZZ
}

ColumnEdit::~ColumnEdit()
{
    disposeOnce();
}

VCL_BUILDER_FACTORY_ARGS(ColumnEdit, WB_BORDER | WB_SPIN | WB_REPEAT | WB_RIGHT)

bool ColumnEdit::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = SpinField::EventNotify(rNEvt);

    MouseNotifyEvent nType = rNEvt.GetType();
    if ( nType == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = pKEvt->GetKeyCode();

        if ( !aCode.IsMod1() && !aCode.IsMod2() )
        {
            //! Input Validation (only alphanumerics, max 2-3 digits)
            //! was before VCL not forwarded keyinput
            //! rethink this!!!

            if ( aCode.GetCode() == KEY_RETURN )
            {
                ScNavigatorDlg::ReleaseFocus();
                ExecuteCol();
                bHandled = true;
            }
        }
    }
    else if ( nType == MouseNotifyEvent::LOSEFOCUS )    // LoseFocus not called at VCL
        EvalText();                         // nCol set

    return bHandled;
}

void ColumnEdit::LoseFocus()
{
    EvalText();
}

void ColumnEdit::dispose()
{
    xDlg.clear();
    SpinField::dispose();
}

void ColumnEdit::Up()
{
    nCol++;

    if ( nCol <= SCNAV_MAXCOL )
        SetCol( nCol );
    else
        nCol--;
}

void ColumnEdit::Down()
{
    if ( nCol>1 )
        SetCol( nCol-1 );
}

void ColumnEdit::First()
{
    nCol = 1;
    SetText(OUString('A'));
}

void ColumnEdit::Last()
{
    OUString aStr;
    nCol = NumToAlpha( SCNAV_MAXCOL, aStr );
    SetText( aStr );
}

void ColumnEdit::EvalText()
{
    OUString aStrCol = GetText();

    if (!aStrCol.isEmpty())
    {
        //  nKeyGroup is no longer set at VCL, in cause of lack of keyinput

        if ( CharClass::isAsciiNumeric(aStrCol) )
            nCol = NumStrToAlpha( aStrCol );
        else
            nCol = AlphaToNum( aStrCol );
    }
    else
        nCol = 0;

    SetText( aStrCol );
}

void ColumnEdit::ExecuteCol()
{
    SCROW nRow = xDlg->aEdRow->GetRow();

    EvalText(); // sets nCol

    if ( (nCol > 0) && (nRow > 0) )
        xDlg->SetCurrentCell(nCol - 1, nRow - 1);
}

void ColumnEdit::SetCol( SCCOL nColNo )
{
    OUString aStr;

    if ( nColNo == 0 )
    {
        nCol = 0;
        SetText( aStr );
    }
    else
    {
        nColNo = NumToAlpha( nColNo, aStr );
        nCol = nColNo;
        SetText( aStr );
    }
}

SCCOL ColumnEdit::AlphaToNum( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiAlpha( rStr) )
    {
        rStr = rStr.toAsciiUpperCase();

        if (::AlphaToCol( nColumn, rStr))
            ++nColumn;

        if ( (rStr.getLength() > SCNAV_COLLETTERS) || (nColumn > SCNAV_MAXCOL) )
        {
            nColumn = SCNAV_MAXCOL;
            NumToAlpha( nColumn, rStr );
        }
    }
    else
        rStr.clear();

    return nColumn;
}

SCCOL ColumnEdit::NumStrToAlpha( OUString& rStr )
{
    SCCOL  nColumn = 0;

    if ( CharClass::isAsciiNumeric(rStr) )
        nColumn = NumToAlpha( static_cast<SCCOL>(rStr.toInt32()), rStr );
    else
        rStr.clear();

    return nColumn;
}

SCCOL ColumnEdit::NumToAlpha( SCCOL nColNo, OUString& rStr )
{
    if ( nColNo > SCNAV_MAXCOL )
        nColNo = SCNAV_MAXCOL;
    else if ( nColNo < 1 )
        nColNo = 1;

    ::ScColToAlpha( rStr, nColNo - 1);

    return nColNo;
}

RowEdit::RowEdit(Window* pParent, WinBits nWinBits)
    : NumericField(pParent, nWinBits)
{
    SetMax(SCNAV_MAXROW);
    SetLast(SCNAV_MAXROW);
}

RowEdit::~RowEdit()
{
    disposeOnce();
}

VCL_BUILDER_FACTORY_ARGS(RowEdit, WB_BORDER | WB_SPIN | WB_REPEAT | WB_RIGHT)

bool RowEdit::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = NumericField::EventNotify(rNEvt);

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        vcl::KeyCode aCode = pKEvt->GetKeyCode();
        if ( aCode.GetCode() == KEY_RETURN && !aCode.IsMod1() && !aCode.IsMod2() )
        {
            ScNavigatorDlg::ReleaseFocus();
            ExecuteRow();
            bHandled = true;
        }
    }

    return bHandled;
}

void RowEdit::LoseFocus()
{
}

void RowEdit::dispose()
{
    xDlg.clear();
    NumericField::dispose();
}

Size RowEdit::GetOptimalSize() const
{
    //max rows is 1,000,000, which is too long for typical use
    return CalcMinimumSizeForText("1,000");
}

void RowEdit::ExecuteRow()
{
    SCCOL nCol = xDlg->aEdCol->GetCol();
    SCROW nRow = static_cast<SCROW>(GetValue());

    if ( (nCol > 0) && (nRow > 0) )
        xDlg->SetCurrentCell(nCol - 1, nRow - 1);
}

IMPL_LINK(ScNavigatorDlg, DocumentSelectHdl, ListBox&, rListBox, void)
{
    ScNavigatorDlg::ReleaseFocus();

    OUString aDocName = rListBox.GetSelectedEntry();
    aLbEntries->SelectDoc(aDocName);
}

IMPL_LINK(ScNavigatorDlg, ToolBoxSelectHdl, ToolBox*, pToolBox, void)
{
    sal_uInt16 nSelId = pToolBox->GetCurItemId();
    //  Switch the mode?
    if (nSelId == nZoomId || nSelId == nScenarioId)
    {
        NavListMode eOldMode = eListMode;
        NavListMode eNewMode;

        if (nSelId == nScenarioId)
        {
            if (eOldMode == NAV_LMODE_SCENARIOS)
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_SCENARIOS;
        }
        else                                            // on/off
        {
            if (eOldMode == NAV_LMODE_NONE)
                eNewMode = NAV_LMODE_AREAS;
            else
                eNewMode = NAV_LMODE_NONE;
        }
        SetListMode(eNewMode);
        UpdateButtons();
    }
    else
    {
        if (nSelId == nDataId)
            MarkDataArea();
        else if (nSelId == nUpId)
            StartOfDataArea();
        else if (nSelId == nDownId)
            EndOfDataArea();
        else if (nSelId == nChangeRootId)
        {
            aLbEntries->ToggleRoot();
            UpdateButtons();
        }
    }
}

IMPL_LINK(ScNavigatorDlg, ToolBoxDropdownClickHdl, ToolBox *, pToolBox, void)
{
    // the popup menu of the drop mode has to be called in the
    // click (button down) and not in the select (button up)
    if (pToolBox->GetCurItemId() == nDragModeId)
    {
        VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/scalc/ui/dropmenu.ui", "");
        VclPtr<PopupMenu> aPop(aBuilder.get_menu("menu"));

        switch (GetDropMode())
        {
            case 0:
                aPop->CheckItem("hyperlink");
                break;
            case 1:
                aPop->CheckItem("link");
                break;
            case 2:
                aPop->CheckItem("copy");
                break;
        }

        sal_uInt16 nId = aPop->Execute(pToolBox, pToolBox->GetItemRect(nDragModeId), PopupMenuFlags::ExecuteDown);
        OString sIdent = aPop->GetItemIdent(nId);

        if (sIdent == "hyperlink")
            SetDropMode(0);
        else if (sIdent == "link")
            SetDropMode(1);
        else if (sIdent == "copy")
            SetDropMode(2);

        pToolBox->EndSelection();     // before SetDropMode (SetDropMode calls SetItemImage)
    }
}

void ScNavigatorDlg::UpdateButtons()
{
    NavListMode eMode = eListMode;
    aTbxCmd->CheckItem(nScenarioId, eMode == NAV_LMODE_SCENARIOS);
    aTbxCmd->CheckItem(nZoomId, eMode != NAV_LMODE_NONE);

    // the toggle button:
    if (eMode == NAV_LMODE_SCENARIOS || eMode == NAV_LMODE_NONE)
    {
        aTbxCmd->EnableItem(nChangeRootId, false);
        aTbxCmd->CheckItem(nChangeRootId, false);
    }
    else
    {
        aTbxCmd->EnableItem(nChangeRootId);
        bool bRootSet = aLbEntries->GetRootType() != ScContentId::ROOT;
        aTbxCmd->CheckItem(nChangeRootId, bRootSet);
    }

    OUString sImageId;
    switch (nDropMode)
    {
        case SC_DROPMODE_URL:
            sImageId = RID_BMP_DROP_URL;
            break;
        case SC_DROPMODE_LINK:
            sImageId = RID_BMP_DROP_LINK;
            break;
        case SC_DROPMODE_COPY:
            sImageId = RID_BMP_DROP_COPY;
            break;
    }
    aTbxCmd->SetItemImage(nDragModeId, Image(StockImage::Yes, sImageId));
}

ScNavigatorSettings::ScNavigatorSettings()
    : mnRootSelected(ScContentId::ROOT)
    , mnChildSelected(SC_CONTENT_NOCHILD)
{
    maExpandedVec.fill(false);
}

SFX_IMPL_CHILDWINDOWCONTEXT( ScNavigatorDialogWrapper, SID_NAVIGATOR )

ScNavigatorDialogWrapper::ScNavigatorDialogWrapper(vcl::Window* pParent,
                                                   sal_uInt16 nId,
                                                   SfxBindings* pBind,
                                                   SAL_UNUSED_PARAMETER SfxChildWinInfo* /* pInfo */)
    : SfxChildWindowContext(nId)
{
    pNavigator = VclPtr<ScNavigatorDlg>::Create(pBind, pParent);
    if (SfxNavigator* pNav = dynamic_cast<SfxNavigator*>(pParent))
        pNav->SetMinOutputSizePixel(pNavigator->GetOptimalSize());
    SetWindow(pNavigator);
}

ScNavigatorDlg::ScNavigatorDlg(SfxBindings* pB, vcl::Window* pParent)
    : PanelLayout(pParent, "NavigatorPanel", "modules/scalc/ui/navigatorpanel.ui", nullptr)
    , rBindings(*pB)
    , aStrDragMode(ScResId(SCSTR_DRAGMODE))
    , aStrDisplay(ScResId(SCSTR_DISPLAY))
    , aStrActiveWin(ScResId(SCSTR_ACTIVEWIN))
    , pViewData(nullptr )
    , eListMode(NAV_LMODE_NONE)
    , nDropMode(SC_DROPMODE_URL)
    , nCurCol(0)
    , nCurRow(0)
    , nCurTab(0)
{
    get(aLbDocuments, "documents");
    get(aEdCol, "column");
    aEdCol->SetNavigatorDlg(this);
    get(aEdRow, "row");
    aEdRow->SetNavigatorDlg(this);
    get(aTbxCmd, "toolbox");
    aTbxCmd->SetSelectHdl(LINK(this, ScNavigatorDlg, ToolBoxSelectHdl));
    aTbxCmd->SetDropdownClickHdl(LINK(this, ScNavigatorDlg, ToolBoxDropdownClickHdl));
    nZoomId = aTbxCmd->GetItemId("contents");
    nChangeRootId = aTbxCmd->GetItemId("toggle");
    nDragModeId = aTbxCmd->GetItemId("dragmode");
    aTbxCmd->SetItemBits(nDragModeId, aTbxCmd->GetItemBits(nDragModeId) | ToolBoxItemBits::DROPDOWNONLY);
    nScenarioId = aTbxCmd->GetItemId("scenarios");
    nDownId = aTbxCmd->GetItemId("end");
    nUpId = aTbxCmd->GetItemId("start");
    nDataId = aTbxCmd->GetItemId("datarange");
    get(aContentBox, "contentbox");
    aLbEntries = VclPtr<ScContentTree>::Create(aContentBox, this);
    aLbEntries->set_hexpand(true);
    aLbEntries->set_vexpand(true);
    aLbEntries->Show();
    get(aScenarioBox, "scenariobox");
    aWndScenarios = VclPtr<ScScenarioWindow>::Create(aScenarioBox,
        ScResId(SCSTR_QHLP_SCEN_LISTBOX), ScResId(SCSTR_QHLP_SCEN_COMMENT));
    aWndScenarios->set_hexpand(true);
    aWndScenarios->set_vexpand(true);
    aWndScenarios->Show();

    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    nDropMode = rCfg.GetDragMode();

    aTbxCmd->InsertBreak(3);
    aTbxCmd->SetLineCount(2);
    aLbDocuments->SetDropDownLineCount(9);
    aLbDocuments->SetSelectHdl(LINK(this, ScNavigatorDlg, DocumentSelectHdl));
    aStrActive    = " (" + ScResId(SCSTR_ACTIVE) + ")";     // " (active)"
    aStrNotActive = " (" + ScResId(SCSTR_NOTACTIVE) + ")";  // " (not active)"
    aStrHidden    = " (" + ScResId(SCSTR_HIDDEN) + ")";     // " (hidden)"

    rBindings.ENTERREGISTRATIONS();

    mvBoundItems[0].reset(new ScNavigatorControllerItem(SID_CURRENTCELL,*this,rBindings));
    mvBoundItems[1].reset(new ScNavigatorControllerItem(SID_CURRENTTAB,*this,rBindings));
    mvBoundItems[2].reset(new ScNavigatorControllerItem(SID_CURRENTDOC,*this,rBindings));
    mvBoundItems[3].reset(new ScNavigatorControllerItem(SID_SELECT_SCENARIO,*this,rBindings));

    rBindings.LEAVEREGISTRATIONS();

    StartListening( *(SfxGetpApp()) );
    StartListening( rBindings );

    aLbEntries->InitWindowBits(true);

    aLbEntries->SetSpaceBetweenEntries(0);
    aLbEntries->SetSelectionMode( SelectionMode::Single );
    aLbEntries->SetDragDropMode( DragDropMode::CTRL_MOVE |
                                 DragDropMode::CTRL_COPY |
                                 DragDropMode::ENABLE_TOP );

    //  was a category chosen as root?
    ScContentId nLastRoot = rCfg.GetRootType();
    if ( nLastRoot != ScContentId::ROOT )
        aLbEntries->SetRootType( nLastRoot );

    aLbEntries->Refresh();
    GetDocNames(nullptr);

    UpdateButtons();

    UpdateColumn();
    UpdateRow();
    UpdateTable(nullptr);
    aContentBox->Hide();
    aScenarioBox->Hide();

    aContentIdle.SetInvokeHandler( LINK( this, ScNavigatorDlg, TimeHdl ) );
    aContentIdle.SetPriority( TaskPriority::LOWEST );

    aLbEntries->SetNavigatorDlgFlag(true);

    // if scenario was active, switch on
    NavListMode eNavMode = static_cast<NavListMode>(rCfg.GetListMode());
    if (eNavMode == NAV_LMODE_SCENARIOS)
        aTbxCmd->CheckItem(nScenarioId);
    else
        eNavMode = NAV_LMODE_AREAS;
    SetListMode(eNavMode);

    aExpandedSize = GetOptimalSize();
}

void ScNavigatorDlg::StateChanged(StateChangedType nStateChange)
{
    PanelLayout::StateChanged(nStateChange);
    if (nStateChange == StateChangedType::InitShow)
    {
        // When the navigator is displayed in the sidebar, or is otherwise
        // docked, it has the whole deck to fill. Therefore hide the button that
        // hides all controls below the top two rows of buttons.
        aTbxCmd->ShowItem(nZoomId, SfxChildWindowContext::GetFloatingWindow(GetParent()) != nullptr);
    }
}

ScNavigatorDlg::~ScNavigatorDlg()
{
    disposeOnce();
}

void ScNavigatorDlg::dispose()
{
    aContentIdle.Stop();

    for (auto & p : mvBoundItems)
        p.reset();
    pMarkArea.reset();

    EndListening( *(SfxGetpApp()) );
    EndListening( rBindings );

    aEdCol.clear();
    aEdRow.clear();
    aTbxCmd.clear();
    aLbEntries.disposeAndClear();
    aContentBox.clear();
    aWndScenarios.disposeAndClear();
    aScenarioBox.clear();
    aLbDocuments.clear();
    PanelLayout::dispose();
}

void ScNavigatorDlg::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (const SfxEventHint* pHint = dynamic_cast<const SfxEventHint*>(&rHint))
    {
        if (pHint->GetEventId() == SfxEventHintId::ActivateDoc)
        {
            aLbEntries->ActiveDocChanged();
            UpdateAll();
        }
    }
    else
    {
        const SfxHintId nHintId = rHint.GetId();

        if (nHintId == SfxHintId::ScDocNameChanged)
        {
            aLbEntries->ActiveDocChanged();
        }
        else if (NAV_LMODE_NONE == eListMode)
        {
            //  Table not any more
        }
        else
        {
            switch ( nHintId )
            {
                case SfxHintId::ScTablesChanged:
                    aLbEntries->Refresh( ScContentId::TABLE );
                    break;

                case SfxHintId::ScDbAreasChanged:
                    aLbEntries->Refresh( ScContentId::DBAREA );
                    break;

                case SfxHintId::ScAreasChanged:
                    aLbEntries->Refresh( ScContentId::RANGENAME );
                    break;

                case SfxHintId::ScDrawChanged:
                    aLbEntries->Refresh( ScContentId::GRAPHIC );
                    aLbEntries->Refresh( ScContentId::OLEOBJECT );
                    aLbEntries->Refresh( ScContentId::DRAWING );
                    break;

                case SfxHintId::ScAreaLinksChanged:
                    aLbEntries->Refresh( ScContentId::AREALINK );
                    break;

                //  SfxHintId::DocChanged not only at document change

                case SfxHintId::ScNavigatorUpdateAll:
                    UpdateAll();
                    break;

                case SfxHintId::ScDataChanged:
                case SfxHintId::ScAnyDataChanged:
                    aContentIdle.Start();      // Do not search notes immediately
                    break;
                case SfxHintId::ScKillEditView:
                    aLbEntries->ObjectFresh( ScContentId::OLEOBJECT );
                    aLbEntries->ObjectFresh( ScContentId::DRAWING );
                    aLbEntries->ObjectFresh( ScContentId::GRAPHIC );
                    break;
                case SfxHintId::ScSelectionChanged:
                    UpdateSelection();
                    break;
                default:
                    break;
            }
        }
    }
}

IMPL_LINK( ScNavigatorDlg, TimeHdl, Timer*, pIdle, void )
{
    if ( pIdle != &aContentIdle )
        return;

    aLbEntries->Refresh( ScContentId::NOTE );
}

void ScNavigatorDlg::SetDropMode(sal_uInt16 nNew)
{
    nDropMode = nNew;
    UpdateButtons();
    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    rCfg.SetDragMode(nDropMode);
}

void ScNavigatorDlg::SetCurrentCell( SCCOL nColNo, SCROW nRowNo )
{
    if ( (nColNo+1 != nCurCol) || (nRowNo+1 != nCurRow) )
    {
        // SID_CURRENTCELL == Item #0 clear cache, so it's possible
        // setting the current cell even in combined areas
        mvBoundItems[0]->ClearCache();

        ScAddress aScAddress( nColNo, nRowNo, 0 );
        OUString aAddr(aScAddress.Format(ScRefFlags::ADDR_ABS));

        bool bUnmark = false;
        if ( GetViewData() )
            bUnmark = !pViewData->GetMarkData().IsCellMarked( nColNo, nRowNo );

        SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );
        SfxBoolItem     aUnmarkItem( FN_PARAM_1, bUnmark );     // cancel selection

        rBindings.GetDispatcher()->ExecuteList(SID_CURRENTCELL,
                                  SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                  { &aPosItem, &aUnmarkItem });
    }
}

void ScNavigatorDlg::SetCurrentCellStr( const OUString& rName )
{
    mvBoundItems[0]->ClearCache();
    SfxStringItem   aNameItem( SID_CURRENTCELL, rName );

    rBindings.GetDispatcher()->ExecuteList(SID_CURRENTCELL,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aNameItem });
}

void ScNavigatorDlg::SetCurrentTable( SCTAB nTabNo )
{
    if ( nTabNo != nCurTab )
    {
        // Table for basic is base-1
        SfxUInt16Item aTabItem( SID_CURRENTTAB, static_cast<sal_uInt16>(nTabNo) + 1 );
        rBindings.GetDispatcher()->ExecuteList(SID_CURRENTTAB,
                                  SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                                  { &aTabItem });
    }
}

void ScNavigatorDlg::SetCurrentTableStr( const OUString& rName )
{
    if (!GetViewData()) return;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nCount = pDoc->GetTableCount();
    OUString aTabName;
    SCTAB nLastSheet = 0;

    for (SCTAB i = 0; i<nCount; i++)
    {
        pDoc->GetName(i, aTabName);
        if (aTabName == rName)
        {
            // Check if this is a Scenario sheet and if so select the sheet
            // where it belongs to, which is the previous non-Scenario sheet.
            if (pDoc->IsScenario(i))
            {
                SetCurrentTable(nLastSheet);
                return;
            }
            else
            {
                SetCurrentTable(i);
                return;
            }
        }
        else
        {
            if (!pDoc->IsScenario(i))
                nLastSheet = i;
        }
    }
}

void ScNavigatorDlg::SetCurrentObject( const OUString& rName )
{
    SfxStringItem aNameItem( SID_CURRENTOBJECT, rName );
    rBindings.GetDispatcher()->ExecuteList( SID_CURRENTOBJECT,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aNameItem });
}

void ScNavigatorDlg::SetCurrentDoc( const OUString& rDocName )        // activate
{
    SfxStringItem aDocItem( SID_CURRENTDOC, rDocName );
    rBindings.GetDispatcher()->ExecuteList( SID_CURRENTDOC,
                              SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                              { &aDocItem });
}

void ScNavigatorDlg::UpdateSelection()
{
    ScTabViewShell* pViewSh = GetTabViewShell();
    if( !pViewSh )
        return;

    uno::Reference< drawing::XShapes > xShapes = pViewSh->getSelectedXShapes();
    if( xShapes )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess(
                xShapes, uno::UNO_QUERY_THROW );
        if( xIndexAccess->getCount() > 1 )
            return;
        uno::Reference< drawing::XShape > xShape;
        if( xIndexAccess->getByIndex(0) >>= xShape )
        {
            uno::Reference< container::XNamed > xNamed( xShape, uno::UNO_QUERY_THROW );
            OUString sName = xNamed->getName();
            if (!sName.isEmpty())
            {
                aLbEntries->SelectEntryByName( ScContentId::DRAWING, sName );
            }
        }
    }
}

ScTabViewShell* ScNavigatorDlg::GetTabViewShell()
{
    return dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
}

ScNavigatorSettings* ScNavigatorDlg::GetNavigatorSettings()
{
    //  Don't store the settings pointer here, because the settings belong to
    //  the view, and the view may be closed while the navigator is open (reload).
    //  If the pointer is cached here again later for performance reasons, it has to
    //  be forgotten when the view is closed.

    ScTabViewShell* pViewSh = GetTabViewShell();
    return pViewSh ? pViewSh->GetNavigatorSettings() : nullptr;
}

bool ScNavigatorDlg::GetViewData()
{
    ScTabViewShell* pViewSh = GetTabViewShell();
    pViewData = pViewSh ? &pViewSh->GetViewData() : nullptr;

    return ( pViewData != nullptr );
}

void ScNavigatorDlg::UpdateColumn( const SCCOL* pCol )
{
    if ( pCol )
        nCurCol = *pCol;
    else if ( GetViewData() )
        nCurCol = pViewData->GetCurX() + 1;

    aEdCol->SetCol( nCurCol );
    CheckDataArea();
}

void ScNavigatorDlg::UpdateRow( const SCROW* pRow )
{
    if ( pRow )
        nCurRow = *pRow;
    else if ( GetViewData() )
        nCurRow = pViewData->GetCurY() + 1;

    aEdRow->SetRow( nCurRow );
    CheckDataArea();
}

void ScNavigatorDlg::UpdateTable( const SCTAB* pTab )
{
    if ( pTab )
        nCurTab = *pTab;
    else if ( GetViewData() )
        nCurTab = pViewData->GetTabNo();

    CheckDataArea();
}

void ScNavigatorDlg::UpdateAll()
{
    switch (eListMode)
    {
        case NAV_LMODE_AREAS:
            aLbEntries->Refresh();
            break;
        case NAV_LMODE_NONE:
            //! ???
            break;
        default:
            break;
    }

    aContentIdle.Stop();       // not again
}

void ScNavigatorDlg::SetListMode(NavListMode eMode)
{
    if (eMode != eListMode)
    {
        bool bForceParentResize = SfxChildWindowContext::GetFloatingWindow(GetParent()) &&
                                  (eMode == NAV_LMODE_NONE || eListMode == NAV_LMODE_NONE);
        SfxNavigator* pNav = bForceParentResize ? dynamic_cast<SfxNavigator*>(GetParent()) : nullptr;
        if (pNav && eMode == NAV_LMODE_NONE) //save last normal size on minimizing
            aExpandedSize = GetSizePixel();

        eListMode = eMode;

        switch (eMode)
        {
            case NAV_LMODE_NONE:
                ShowList(false);
                break;
            case NAV_LMODE_AREAS:
                aLbEntries->Refresh();
                ShowList(true);
                break;
            case NAV_LMODE_SCENARIOS:
                ShowScenarios();
                break;
        }

        UpdateButtons();

        if (eMode != NAV_LMODE_NONE)
        {
            ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
            rCfg.SetListMode( static_cast<sal_uInt16>(eMode) );
        }

        if (pNav)
        {
            Size aOptimalSize(GetOptimalSize());
            Size aNewSize(pNav->GetOutputSizePixel());
            aNewSize.setHeight( eMode == NAV_LMODE_NONE ? aOptimalSize.Height() : aExpandedSize.Height() );
            pNav->SetMinOutputSizePixel(aOptimalSize);
            pNav->SetOutputSizePixel(aNewSize);
        }
    }

    if (pMarkArea)
        UnmarkDataArea();
}

void ScNavigatorDlg::ShowList(bool bShow)
{
    if (bShow)
    {
        aContentBox->Show();
        aLbDocuments->Show();
    }
    else
    {
        aContentBox->Hide();
        aLbDocuments->Hide();
    }
    aScenarioBox->Hide();
}

void ScNavigatorDlg::ShowScenarios()
{
    rBindings.Invalidate( SID_SELECT_SCENARIO );
    rBindings.Update( SID_SELECT_SCENARIO );

    aScenarioBox->Show();
    aLbDocuments->Show();
    aContentBox->Hide();
}

//      documents for Dropdown-Listbox

void ScNavigatorDlg::GetDocNames( const OUString* pManualSel )
{
    aLbDocuments->Clear();
    aLbDocuments->SetUpdateMode( false );

    ScDocShell* pCurrentSh = dynamic_cast<ScDocShell*>( SfxObjectShell::Current()  );

    OUString aSelEntry;
    SfxObjectShell* pSh = SfxObjectShell::GetFirst();
    while ( pSh )
    {
        if ( dynamic_cast<const ScDocShell*>( pSh) !=  nullptr )
        {
            OUString aName = pSh->GetTitle();
            OUString aEntry = aName;
            if (pSh == pCurrentSh)
                aEntry += aStrActive;
            else
                aEntry += aStrNotActive;
            aLbDocuments->InsertEntry( aEntry );

            if ( pManualSel ? ( aName == *pManualSel )
                            : ( pSh == pCurrentSh ) )
                aSelEntry = aEntry;                     // complete entry for selection
        }

        pSh = SfxObjectShell::GetNext( *pSh );
    }

    aLbDocuments->InsertEntry( aStrActiveWin );

    OUString aHidden =  aLbEntries->GetHiddenTitle();
    if (!aHidden.isEmpty())
    {
        OUString aEntry = aHidden;
        aEntry += aStrHidden;
        aLbDocuments->InsertEntry( aEntry );

        if ( pManualSel && aHidden == *pManualSel )
            aSelEntry = aEntry;
    }

    aLbDocuments->SetUpdateMode( true );

    aLbDocuments->SelectEntry( aSelEntry );
}

void ScNavigatorDlg::MarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
        if ( !pMarkArea )
            pMarkArea.reset( new ScArea );

        pViewSh->MarkDataArea();
        ScRange aMarkRange;
        pViewSh->GetViewData().GetMarkData().GetMarkArea(aMarkRange);
        pMarkArea->nColStart = aMarkRange.aStart.Col();
        pMarkArea->nRowStart = aMarkRange.aStart.Row();
        pMarkArea->nColEnd = aMarkRange.aEnd.Col();
        pMarkArea->nRowEnd = aMarkRange.aEnd.Row();
        pMarkArea->nTab = aMarkRange.aStart.Tab();
    }
}

void ScNavigatorDlg::UnmarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
        pViewSh->Unmark();
        pMarkArea.reset();
    }
}

void ScNavigatorDlg::CheckDataArea()
{
    if (aTbxCmd->IsItemChecked(nDataId) && pMarkArea)
    {
        if (   nCurTab   != pMarkArea->nTab
            || nCurCol <  pMarkArea->nColStart+1
            || nCurCol >  pMarkArea->nColEnd+1
            || nCurRow <  pMarkArea->nRowStart+1
            || nCurRow >  pMarkArea->nRowEnd+1 )
        {
            aTbxCmd->SetItemState(nDataId, TRISTATE_TRUE);
            aTbxCmd->TriggerItem(nDataId);
        }
    }
}

void ScNavigatorDlg::StartOfDataArea()
{
    //  pMarkArea evaluate ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        SCCOL nCol = aMarkRange.aStart.Col();
        SCROW nRow = aMarkRange.aStart.Row();

        if ( (nCol+1 != aEdCol->GetCol()) || (nRow+1 != aEdRow->GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

void ScNavigatorDlg::EndOfDataArea()
{
    //  pMarkArea evaluate ???

    if ( GetViewData() )
    {
        ScMarkData& rMark = pViewData->GetMarkData();
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );

        SCCOL nCol = aMarkRange.aEnd.Col();
        SCROW nRow = aMarkRange.aEnd.Row();

        if ( (nCol+1 != aEdCol->GetCol()) || (nRow+1 != aEdRow->GetRow()) )
            SetCurrentCell( nCol, nRow );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
