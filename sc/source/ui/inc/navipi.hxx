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

#pragma once

#include <vector>
#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/navigat.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <rangeutl.hxx>
#include "content.hxx"

class SfxPoolItem;
class ScTabViewShell;
class ScViewData;
class ScScenarioWindow;
class ScNavigatorControllerItem;
class ScNavigatorDlg;
class ScNavigatorSettings;

#define SC_DROPMODE_URL         0
#define SC_DROPMODE_LINK        1
#define SC_DROPMODE_COPY        2

enum NavListMode { NAV_LMODE_NONE       = 0x4000,
                   NAV_LMODE_AREAS      = 0x2000,
                   NAV_LMODE_SCENARIOS  = 0x400 };

class ScScenarioWindow
{
public:
    ScScenarioWindow(weld::Builder& rBuilder, const OUString& rQH_List, const OUString& rQH_Comment);
    ~ScScenarioWindow();
    void NotifyState(const SfxPoolItem* pState);
    void SetComment(const OUString& rComment)
    {
        m_xEdComment->set_text(rComment);
    }

private:
    std::unique_ptr<weld::TreeView> m_xLbScenario;
    std::unique_ptr<weld::TextView> m_xEdComment;

    struct ScenarioEntry
    {
        OUString            maName;
        OUString            maComment;
        bool                mbProtected;

        explicit     ScenarioEntry() : mbProtected( false ) {}
    };

    std::vector< ScenarioEntry > m_aEntries;

    void UpdateEntries(const std::vector<OUString> &rNewEntryList);
    void SelectScenario();
    void ExecuteScenarioSlot(sal_uInt16 nSlotId);
    void EditScenario();
    void DeleteScenario();
    const ScenarioEntry* GetSelectedScenarioEntry() const;

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ContextMenuHdl, const CommandEvent&, bool);
};

class ScNavigatorDlg : public PanelLayout, public SfxListener
{
friend class ScNavigatorWin;
friend class ScNavigatorControllerItem;
friend class ScContentTree;

private:
    static constexpr int CTRL_ITEMS = 4;

    SfxBindings&        rBindings;      // must be first member

    std::unique_ptr<weld::SpinButton> m_xEdCol;
    std::unique_ptr<weld::SpinButton> m_xEdRow;
    std::unique_ptr<weld::Toolbar> m_xTbxCmd1;
    std::unique_ptr<weld::Toolbar> m_xTbxCmd2;
    std::unique_ptr<ScContentTree> m_xLbEntries;
    std::unique_ptr<weld::Widget> m_xScenarioBox;
    std::unique_ptr<ScScenarioWindow> m_xWndScenarios;
    std::unique_ptr<weld::ComboBox> m_xLbDocuments;
    std::unique_ptr<weld::Menu> m_xDragModeMenu;

    VclPtr<SfxNavigator> m_xNavigatorDlg;

    Size            aExpandedSize;
    Idle            aContentIdle;

    OUString        aStrActive;
    OUString        aStrNotActive;
    OUString        aStrActiveWin;

    std::optional<ScArea> moMarkArea;
    ScViewData*     pViewData;

    NavListMode     eListMode;
    sal_uInt16      nDropMode;
    SCCOL           nCurCol;
    SCROW           nCurRow;
    SCTAB           nCurTab;

    std::array<std::unique_ptr<ScNavigatorControllerItem>,CTRL_ITEMS> mvBoundItems;

    DECL_LINK(TimeHdl, Timer*, void);
    DECL_LINK(DocumentSelectHdl, weld::ComboBox&, void);
    DECL_LINK(ExecuteRowHdl, weld::Entry&, bool);
    DECL_LINK(ExecuteColHdl, weld::Entry&, bool);
    DECL_LINK(ToolBoxSelectHdl, const OUString&, void);
    DECL_LINK(ToolBoxDropdownClickHdl, const OUString&, void);
    DECL_LINK(MenuSelectHdl, const OUString&, void);
    DECL_LINK(FormatRowOutputHdl, weld::SpinButton&, void);
    DECL_LINK(ParseRowInputHdl, int*, bool);

    void    UpdateButtons();
    void    SetCurrentCell( SCCOL nCol, SCROW Row );
    void    SetCurrentCellStr( const OUString& rName );
    void    SetCurrentTable( SCTAB nTab );
    void    SetCurrentTableStr( std::u16string_view rName );
    void    SetCurrentObject( const OUString& rName );
    void    SetCurrentDoc( const OUString& rDocName );
    void    UpdateSelection();
    void    ContentUpdated(); // stop aContentIdle because content is up to date

    SAL_RET_MAYBENULL static ScTabViewShell*  GetTabViewShell();
    static ScNavigatorSettings* GetNavigatorSettings();
    ScViewData*             GetViewData();

    void    UpdateSheetLimits();

    void    UpdateColumn    ( const SCCOL* pCol = nullptr );
    void    UpdateRow       ( const SCROW* pRow = nullptr );
    void    UpdateTable     ( const SCTAB* pTab );
    void    UpdateAll       ();

    void    GetDocNames(const OUString* pSelEntry);

    void    SetListMode(NavListMode eMode);
    void    ShowList(bool bShow);
    void    ShowScenarios();

    void    SetDropMode(sal_uInt16 nNew);
    sal_uInt16  GetDropMode() const         { return nDropMode; }

    void    MarkDataArea    ();
    void    UnmarkDataArea  ();
    void    StartOfDataArea ();
    void    EndOfDataArea   ();

    void    UpdateInitShow();

    static void ReleaseFocus();

public:
    ScNavigatorDlg(SfxBindings* pB, weld::Widget* pParent, SfxNavigator* pNavigatorDlg);
    virtual weld::Window* GetFrameWeld() const override;
    virtual ~ScNavigatorDlg() override;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

};

class ScNavigatorWrapper final : public SfxNavigatorWrapper
{
public:
    ScNavigatorWrapper(vcl::Window *pParent, sal_uInt16 nId,
                       SfxBindings* pBindings, SfxChildWinInfo* pInfo);
    SFX_DECL_CHILDWINDOW(ScNavigatorWrapper);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
