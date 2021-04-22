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
#include <unotools/charclass.hxx>
#include <uiobject.hxx>

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
static SCCOL SCNAV_MAXCOL(const ScSheetLimits& rLimits) { return rLimits.GetMaxColCount(); }
static sal_Int32 SCNAV_COLDIGITS(const ScSheetLimits& rLimits)
{
    return static_cast<sal_Int32>( floor( log10( static_cast<double>(SCNAV_MAXCOL(rLimits)))) ) + 1;   // 1...256...18278
}
static sal_Int32 SCNAV_COLLETTERS(const ScSheetLimits& rLimits)
{
    return ::ScColToAlpha(SCNAV_MAXCOL(rLimits)).getLength();    // A...IV...ZZZ
}

static SCROW SCNAV_MAXROW(const ScSheetLimits& rSheetLimits)
{
    return rSheetLimits.GetMaxRowCount();
}

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

namespace
{
    SCCOL NumToAlpha(const ScSheetLimits& rSheetLimits, SCCOL nColNo, OUString& rStr)
    {
        if ( nColNo > SCNAV_MAXCOL(rSheetLimits) )
            nColNo = SCNAV_MAXCOL(rSheetLimits);
        else if ( nColNo < 1 )
            nColNo = 1;

        ::ScColToAlpha( rStr, nColNo - 1);

        return nColNo;
    }

    SCCOL AlphaToNum(const ScDocument& rDoc, OUString& rStr)
    {
        SCCOL  nColumn = 0;

        if ( CharClass::isAsciiAlpha( rStr) )
        {
            rStr = rStr.toAsciiUpperCase();

            if (::AlphaToCol( rDoc, nColumn, rStr))
                ++nColumn;

            if ( (rStr.getLength() > SCNAV_COLLETTERS(rDoc.GetSheetLimits())) ||
                 (nColumn > SCNAV_MAXCOL(rDoc.GetSheetLimits())) )
            {
                nColumn = SCNAV_MAXCOL(rDoc.GetSheetLimits());
                NumToAlpha( rDoc.GetSheetLimits(), nColumn, rStr );
            }
        }
        else
            rStr.clear();

        return nColumn;
    }

    SCCOL NumStrToAlpha(const ScSheetLimits& rSheetLimits, OUString& rStr)
    {
        SCCOL  nColumn = 0;

        if ( CharClass::isAsciiNumeric(rStr) )
            nColumn = NumToAlpha( rSheetLimits, static_cast<SCCOL>(rStr.toInt32()), rStr );
        else
            rStr.clear();

        return nColumn;
    }
}

IMPL_LINK(ScNavigatorDlg, ParseRowInputHdl, int*, result, bool)
{
    SCCOL nCol;

    OUString aStrCol = m_xEdCol->get_text();

    if (!aStrCol.isEmpty())
    {
        //  nKeyGroup is no longer set at VCL, in cause of lack of keyinput

        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current() );
        auto& rDoc = pViewSh->GetViewData().GetDocument();
        if ( CharClass::isAsciiNumeric(aStrCol) )
            nCol = NumStrToAlpha( rDoc.GetSheetLimits(), aStrCol );
        else
            nCol = AlphaToNum( rDoc, aStrCol );
    }
    else
        nCol = 0;

    *result = nCol;
    return true;
}

IMPL_LINK_NOARG(ScNavigatorDlg, ExecuteColHdl, weld::Entry&, bool)
{
    ReleaseFocus();

    SCROW nRow = m_xEdRow->get_value();
    SCCOL nCol = m_xEdCol->get_value();

    if ( (nCol > 0) && (nRow > 0) )
        SetCurrentCell(nCol - 1, nRow - 1);

    return true;
}

IMPL_LINK_NOARG(ScNavigatorDlg, FormatRowOutputHdl, weld::SpinButton&, void)
{
    OUString aStr;
    ::ScColToAlpha(aStr, m_xEdCol->get_value() - 1);
    m_xEdCol->set_text(aStr);
}

IMPL_LINK_NOARG(ScNavigatorDlg, ExecuteRowHdl, weld::Entry&, bool)
{
    ReleaseFocus();

    SCCOL nCol = m_xEdCol->get_value();
    SCROW nRow = m_xEdRow->get_value();

    if ( (nCol > 0) && (nRow > 0) )
        SetCurrentCell(nCol - 1, nRow - 1);

    return true;
}

IMPL_LINK(ScNavigatorDlg, DocumentSelectHdl, weld::ComboBox&, rListBox, void)
{
    ScNavigatorDlg::ReleaseFocus();

    OUString aDocName = rListBox.get_active_text();
    m_xLbEntries->SelectDoc(aDocName);
}

IMPL_LINK(ScNavigatorDlg, ToolBoxSelectHdl, const OString&, rSelId, void)
{
    //  Switch the mode?
    if (rSelId == "contents" || rSelId == "scenarios")
    {
        NavListMode eOldMode = eListMode;
        NavListMode eNewMode;

        if (rSelId == "scenarios")
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
    else if (rSelId == "dragmode")
        m_xTbxCmd2->set_menu_item_active("dragmode", !m_xTbxCmd2->get_menu_item_active("dragmode"));
    else
    {
        if (rSelId == "datarange")
            MarkDataArea();
        else if (rSelId == "start")
            StartOfDataArea();
        else if (rSelId == "end")
            EndOfDataArea();
        else if (rSelId == "toggle")
        {
            m_xLbEntries->ToggleRoot();
            UpdateButtons();
        }
    }
}

IMPL_LINK(ScNavigatorDlg, ToolBoxDropdownClickHdl, const OString&, rCommand, void)
{
    if (!m_xTbxCmd2->get_menu_item_active(rCommand))
        return;

    // the popup menu of the drop mode has to be called in the
    // click (button down) and not in the select (button up)
    if (rCommand != "dragmode")
        return;

    switch (GetDropMode())
    {
        case 0:
            m_xDragModeMenu->set_active("hyperlink", true);
            break;
        case 1:
            m_xDragModeMenu->set_active("link", true);
            break;
        case 2:
            m_xDragModeMenu->set_active("copy", true);
            break;
    }
}

IMPL_LINK(ScNavigatorDlg, MenuSelectHdl, const OString&, rIdent, void)
{
    if (rIdent == "hyperlink")
        SetDropMode(0);
    else if (rIdent == "link")
        SetDropMode(1);
    else if (rIdent == "copy")
        SetDropMode(2);
}

void ScNavigatorDlg::UpdateButtons()
{
    NavListMode eMode = eListMode;
    m_xTbxCmd2->set_item_active("scenarios", eMode == NAV_LMODE_SCENARIOS);
    m_xTbxCmd1->set_item_active("contents", eMode != NAV_LMODE_NONE);

    // the toggle button:
    if (eMode == NAV_LMODE_SCENARIOS || eMode == NAV_LMODE_NONE)
    {
        m_xTbxCmd2->set_item_sensitive("toggle", false);
        m_xTbxCmd2->set_item_active("toggle", false);
    }
    else
    {
        m_xTbxCmd2->set_item_sensitive("toggle", true);
        bool bRootSet = m_xLbEntries->GetRootType() != ScContentId::ROOT;
        m_xTbxCmd2->set_item_active("toggle", bRootSet);
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
    m_xTbxCmd2->set_item_icon_name("dragmode", sImageId);
}

ScNavigatorSettings::ScNavigatorSettings()
    : mnRootSelected(ScContentId::ROOT)
    , mnChildSelected(SC_CONTENT_NOCHILD)
{
    maExpandedVec.fill(false);
}

class ScNavigatorWin : public SfxNavigator
{
private:
    std::unique_ptr<ScNavigatorDlg> m_xNavigator;
public:
    ScNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* pMgr,
                   vcl::Window* pParent, SfxChildWinInfo* pInfo);
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual void dispose() override
    {
        m_xNavigator.reset();
        SfxNavigator::dispose();
    }
    virtual ~ScNavigatorWin() override
    {
        disposeOnce();
    }
};

ScNavigatorWin::ScNavigatorWin(SfxBindings* _pBindings, SfxChildWindow* _pMgr,
                               vcl::Window* _pParent, SfxChildWinInfo* pInfo)
    : SfxNavigator(_pBindings, _pMgr, _pParent, pInfo)
{
    m_xNavigator = std::make_unique<ScNavigatorDlg>(_pBindings, m_xContainer.get(), this);
    SetMinOutputSizePixel(GetOptimalSize());
}

ScNavigatorDlg::ScNavigatorDlg(SfxBindings* pB, weld::Widget* pParent, SfxNavigator* pNavigatorDlg)
    : PanelLayout(pParent, "NavigatorPanel", "modules/scalc/ui/navigatorpanel.ui")
    , rBindings(*pB)
    , m_xEdCol(m_xBuilder->weld_spin_button("column"))
    , m_xEdRow(m_xBuilder->weld_spin_button("row"))
    , m_xTbxCmd1(m_xBuilder->weld_toolbar("toolbox1"))
    , m_xTbxCmd2(m_xBuilder->weld_toolbar("toolbox2"))
    , m_xLbEntries(new ScContentTree(m_xBuilder->weld_tree_view("contentbox"), this))
    , m_xScenarioBox(m_xBuilder->weld_widget("scenariobox"))
    , m_xWndScenarios(new ScScenarioWindow(*m_xBuilder,
        ScResId(SCSTR_QHLP_SCEN_LISTBOX), ScResId(SCSTR_QHLP_SCEN_COMMENT)))
    , m_xLbDocuments(m_xBuilder->weld_combo_box("documents"))
    , m_xDragModeMenu(m_xBuilder->weld_menu("dragmodemenu"))
    , m_xNavigatorDlg(pNavigatorDlg)
    , aStrActiveWin(ScResId(SCSTR_ACTIVEWIN))
    , pViewData(nullptr )
    , eListMode(NAV_LMODE_NONE)
    , nDropMode(SC_DROPMODE_URL)
    , nCurCol(0)
    , nCurRow(0)
    , nCurTab(0)
{
    UpdateInitShow();

    UpdateSheetLimits();
    m_xEdRow->set_width_chars(5);
    //max rows is 1,000,000, which is too long for typical use
    m_xEdRow->connect_activate(LINK(this, ScNavigatorDlg, ExecuteRowHdl));

    m_xEdCol->connect_activate(LINK(this, ScNavigatorDlg, ExecuteColHdl));
    m_xEdCol->connect_output(LINK(this, ScNavigatorDlg, FormatRowOutputHdl));
    m_xEdCol->connect_input(LINK(this, ScNavigatorDlg, ParseRowInputHdl));

    m_xTbxCmd1->connect_clicked(LINK(this, ScNavigatorDlg, ToolBoxSelectHdl));
    m_xTbxCmd2->connect_clicked(LINK(this, ScNavigatorDlg, ToolBoxSelectHdl));

    m_xTbxCmd2->set_item_menu("dragmode", m_xDragModeMenu.get());
    m_xDragModeMenu->connect_activate(LINK(this, ScNavigatorDlg, MenuSelectHdl));
    m_xTbxCmd2->connect_menu_toggled(LINK(this, ScNavigatorDlg, ToolBoxDropdownClickHdl));

    ScNavipiCfg& rCfg = SC_MOD()->GetNavipiCfg();
    nDropMode = rCfg.GetDragMode();

    m_xLbDocuments->set_size_request(42, -1); // set a nominal width so it takes width of surroundings
    m_xLbDocuments->connect_changed(LINK(this, ScNavigatorDlg, DocumentSelectHdl));
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

    //  was a category chosen as root?
    ScContentId nLastRoot = rCfg.GetRootType();
    if ( nLastRoot != ScContentId::ROOT )
        m_xLbEntries->SetRootType( nLastRoot );

    m_xLbEntries->Refresh();
    GetDocNames(nullptr);

    UpdateButtons();

    UpdateColumn();
    UpdateRow();
    UpdateTable(nullptr);
    m_xLbEntries->hide();
    m_xScenarioBox->hide();

    aContentIdle.SetInvokeHandler( LINK( this, ScNavigatorDlg, TimeHdl ) );
    aContentIdle.SetPriority( TaskPriority::LOWEST );

    m_xLbEntries->SetNavigatorDlgFlag(true);

    // if scenario was active, switch on
    NavListMode eNavMode = static_cast<NavListMode>(rCfg.GetListMode());
    if (eNavMode == NAV_LMODE_SCENARIOS)
        m_xTbxCmd2->set_item_active("scenarios", true);
    else
        eNavMode = NAV_LMODE_AREAS;
    SetListMode(eNavMode);
}

weld::Window* ScNavigatorDlg::GetFrameWeld() const
{
    if (m_xNavigatorDlg)
        return m_xNavigatorDlg->GetFrameWeld();
    return PanelLayout::GetFrameWeld();
}

void ScNavigatorDlg::UpdateSheetLimits()
{
    if (ScViewData* pData = GetViewData())
    {
        ScDocument& rDoc = pData->GetDocument();
        m_xEdRow->set_range(1, SCNAV_MAXROW(rDoc.GetSheetLimits()));
        m_xEdCol->set_range(1, SCNAV_MAXCOL(rDoc.GetSheetLimits()));
        m_xEdCol->set_width_chars(SCNAV_COLDIGITS(rDoc.GetSheetLimits()));   // 1...256...18278 or A...IV...ZZZ
    }
}

void ScNavigatorDlg::UpdateInitShow()
{
    // When the navigator is displayed in the sidebar, or is otherwise
    // docked, it has the whole deck to fill. Therefore hide the button that
    // hides all controls below the top two rows of buttons.
    m_xTbxCmd1->set_item_visible("contents", ParentIsFloatingWindow(m_xNavigatorDlg));
}

void ScNavigatorWin::StateChanged(StateChangedType nStateChange)
{
    SfxNavigator::StateChanged(nStateChange);
    if (nStateChange == StateChangedType::InitShow)
        m_xNavigator->UpdateInitShow();
}

ScNavigatorDlg::~ScNavigatorDlg()
{
    aContentIdle.Stop();

    for (auto & p : mvBoundItems)
        p.reset();
    pMarkArea.reset();

    EndListening( *(SfxGetpApp()) );
    EndListening( rBindings );

    m_xEdCol.reset();
    m_xEdRow.reset();
    m_xTbxCmd1.reset();
    m_xTbxCmd2.reset();
    m_xDragModeMenu.reset();
    m_xLbEntries.reset();
    m_xWndScenarios.reset();
    m_xScenarioBox.reset();
    m_xLbDocuments.reset();
}

void ScNavigatorDlg::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (const SfxEventHint* pHint = dynamic_cast<const SfxEventHint*>(&rHint))
    {
        if (pHint->GetEventId() == SfxEventHintId::ActivateDoc)
        {
            UpdateSheetLimits();
            m_xLbEntries->ActiveDocChanged();
            UpdateAll();
        }
    }
    else
    {
        const SfxHintId nHintId = rHint.GetId();

        if (nHintId == SfxHintId::ScDocNameChanged)
        {
            m_xLbEntries->ActiveDocChanged();
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
                    m_xLbEntries->Refresh( ScContentId::TABLE );
                    break;

                case SfxHintId::ScDbAreasChanged:
                    m_xLbEntries->Refresh( ScContentId::DBAREA );
                    break;

                case SfxHintId::ScAreasChanged:
                    m_xLbEntries->Refresh( ScContentId::RANGENAME );
                    break;

                case SfxHintId::ScDrawChanged:
                    m_xLbEntries->Refresh( ScContentId::GRAPHIC );
                    m_xLbEntries->Refresh( ScContentId::OLEOBJECT );
                    m_xLbEntries->Refresh( ScContentId::DRAWING );
                    break;

                case SfxHintId::ScAreaLinksChanged:
                    m_xLbEntries->Refresh( ScContentId::AREALINK );
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
                    m_xLbEntries->ObjectFresh( ScContentId::OLEOBJECT );
                    m_xLbEntries->ObjectFresh( ScContentId::DRAWING );
                    m_xLbEntries->ObjectFresh( ScContentId::GRAPHIC );
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

    m_xLbEntries->Refresh( ScContentId::NOTE );
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
    if ((nColNo+1 == nCurCol) && (nRowNo+1 == nCurRow))
        return;

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

void ScNavigatorDlg::SetCurrentTableStr( std::u16string_view rName )
{
    if (!GetViewData()) return;

    ScDocument& rDoc = pViewData->GetDocument();
    SCTAB nCount = rDoc.GetTableCount();
    OUString aTabName;
    SCTAB nLastSheet = 0;

    for (SCTAB i = 0; i<nCount; i++)
    {
        rDoc.GetName(i, aTabName);
        if (aTabName == rName)
        {
            // Check if this is a Scenario sheet and if so select the sheet
            // where it belongs to, which is the previous non-Scenario sheet.
            if (rDoc.IsScenario(i))
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
            if (!rDoc.IsScenario(i))
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
    if( !xShapes )
        return;

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
            m_xLbEntries->SelectEntryByName( ScContentId::DRAWING, sName );
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

ScViewData* ScNavigatorDlg::GetViewData()
{
    ScTabViewShell* pViewSh = GetTabViewShell();
    pViewData = pViewSh ? &pViewSh->GetViewData() : nullptr;
    return pViewData;
}

void ScNavigatorDlg::UpdateColumn( const SCCOL* pCol )
{
    if ( pCol )
        nCurCol = *pCol;
    else if ( GetViewData() )
        nCurCol = pViewData->GetCurX() + 1;

    m_xEdCol->set_value(nCurCol);
}

void ScNavigatorDlg::UpdateRow( const SCROW* pRow )
{
    if ( pRow )
        nCurRow = *pRow;
    else if ( GetViewData() )
        nCurRow = pViewData->GetCurY() + 1;

    m_xEdRow->set_value(nCurRow);
}

void ScNavigatorDlg::UpdateTable( const SCTAB* pTab )
{
    if ( pTab )
        nCurTab = *pTab;
    else if ( GetViewData() )
        nCurTab = pViewData->GetTabNo();
}

void ScNavigatorDlg::UpdateAll()
{
    switch (eListMode)
    {
        case NAV_LMODE_AREAS:
            m_xLbEntries->Refresh();
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
        bool bForceParentResize = ParentIsFloatingWindow(m_xNavigatorDlg) &&
                                  (eMode == NAV_LMODE_NONE || eListMode == NAV_LMODE_NONE);
        SfxNavigator* pNav = bForceParentResize ? m_xNavigatorDlg.get() : nullptr;
        if (pNav && eMode == NAV_LMODE_NONE) //save last normal size on minimizing
            aExpandedSize = pNav->GetSizePixel();

        eListMode = eMode;

        switch (eMode)
        {
            case NAV_LMODE_NONE:
                ShowList(false);
                break;
            case NAV_LMODE_AREAS:
                m_xLbEntries->Refresh();
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
            pNav->InvalidateChildSizeCache();
            Size aOptimalSize(pNav->GetOptimalSize());
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
        m_xLbEntries->show();
        m_xLbDocuments->show();
    }
    else
    {
        m_xLbEntries->hide();
        m_xLbDocuments->hide();
    }
    m_xScenarioBox->hide();
}

void ScNavigatorDlg::ShowScenarios()
{
    rBindings.Invalidate( SID_SELECT_SCENARIO );
    rBindings.Update( SID_SELECT_SCENARIO );

    m_xScenarioBox->show();
    m_xLbDocuments->show();
    m_xLbEntries->hide();
}

//      documents for Dropdown-Listbox
void ScNavigatorDlg::GetDocNames( const OUString* pManualSel )
{
    m_xLbDocuments->clear();
    m_xLbDocuments->freeze();

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
            m_xLbDocuments->append_text(aEntry);

            if ( pManualSel ? ( aName == *pManualSel )
                            : ( pSh == pCurrentSh ) )
                aSelEntry = aEntry;                     // complete entry for selection
        }

        pSh = SfxObjectShell::GetNext( *pSh );
    }

    m_xLbDocuments->append_text(aStrActiveWin);

    OUString aHidden =  m_xLbEntries->GetHiddenTitle();
    if (!aHidden.isEmpty())
    {
        OUString aEntry = aHidden + aStrHidden;
        m_xLbDocuments->append_text(aEntry);

        if ( pManualSel && aHidden == *pManualSel )
            aSelEntry = aEntry;
    }

    m_xLbDocuments->thaw();

    m_xLbDocuments->set_active_text(aSelEntry);
}

void ScNavigatorDlg::MarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( !pViewSh )
        return;

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

void ScNavigatorDlg::UnmarkDataArea()
{
    ScTabViewShell* pViewSh = GetTabViewShell();

    if ( pViewSh )
    {
        pViewSh->Unmark();
        pMarkArea.reset();
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

        if ( (nCol+1 != m_xEdCol->get_value()) || (nRow+1 != m_xEdRow->get_value()) )
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

        if ( (nCol+1 != m_xEdCol->get_value()) || (nRow+1 != m_xEdRow->get_value()) )
            SetCurrentCell( nCol, nRow );
    }
}

SFX_IMPL_DOCKINGWINDOW(ScNavigatorWrapper, SID_NAVIGATOR);

ScNavigatorWrapper::ScNavigatorWrapper(vcl::Window *_pParent, sal_uInt16 nId,
                                       SfxBindings* pBindings, SfxChildWinInfo* pInfo)
    : SfxNavigatorWrapper(_pParent, nId)
{
    SetWindow(VclPtr<ScNavigatorWin>::Create(pBindings, this, _pParent, pInfo));
    Initialize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
