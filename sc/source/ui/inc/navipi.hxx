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

#ifndef INCLUDED_SC_SOURCE_UI_INC_NAVIPI_HXX
#define INCLUDED_SC_SOURCE_UI_INC_NAVIPI_HXX

#include <vector>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <sfx2/childwin.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include "content.hxx"
#include <svtools/svmedit.hxx>

class SfxPoolItem;
class ScTabViewShell;
class ScViewData;
class ScArea;
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

class ScScenarioListBox : public ListBox
{
public:
    explicit            ScScenarioListBox( ScScenarioWindow& rParent );
    virtual             ~ScScenarioListBox() override;

    void                UpdateEntries( const std::vector<OUString> &aNewEntryList );

protected:
    virtual void        Select() override;
    virtual void        DoubleClick() override;
    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;

private:
    struct ScenarioEntry
    {
        OUString            maName;
        OUString            maComment;
        bool                mbProtected;

        explicit     ScenarioEntry() : mbProtected( false ) {}
    };
    typedef ::std::vector< ScenarioEntry > ScenarioList;

private:
    const ScenarioEntry* GetSelectedScenarioEntry() const;

    void                ExecuteScenarioSlot( sal_uInt16 nSlotId );
    void                SelectScenario();
    void                EditScenario();
    void                DeleteScenario();

private:
    ScScenarioWindow&   mrParent;
    ScenarioList        maEntries;
};

class ScScenarioWindow : public vcl::Window
{
public:
    ScScenarioWindow(vcl::Window* pParent, const OUString& rQH_List, const OUString& rQH_Comment);
    virtual ~ScScenarioWindow() override;
    virtual void dispose() override;
    void NotifyState(const SfxPoolItem* pState);
    void SetComment(const OUString& rComment)
    {
        aEdComment->SetText(rComment);
    }

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;

private:
    VclPtr<ScScenarioListBox>   aLbScenario;
    VclPtr<MultiLineEdit>       aEdComment;
};

class ColumnEdit : public SpinField
{
public:
    ColumnEdit(Window* pParent, WinBits nWinBits);
    ~ColumnEdit() override;
    void SetNavigatorDlg(ScNavigatorDlg *pNaviDlg)
    {
        xDlg = pNaviDlg;
    }
    SCCOL   GetCol() const { return nCol; }
    void    SetCol( SCCOL nColNo );

protected:
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    LoseFocus() override;
    virtual void    Up() override;
    virtual void    Down() override;
    virtual void    First() override;
    virtual void    Last() override;
    virtual void    dispose() override;

private:
    VclPtr<ScNavigatorDlg> xDlg;
    SCCOL           nCol;

    void EvalText();
    void ExecuteCol();
    static SCCOL AlphaToNum    ( OUString& rStr );
    static SCCOL NumStrToAlpha ( OUString& rStr );
    static SCCOL NumToAlpha    ( SCCOL nColNo, OUString& rStr );
};

class RowEdit : public NumericField
{
public:
    RowEdit(Window* pParent, WinBits nWinBits);
    ~RowEdit() override;
    void SetNavigatorDlg(ScNavigatorDlg *pNaviDlg)
    {
        xDlg = pNaviDlg;
    }
    SCROW   GetRow() const { return static_cast<SCROW>(GetValue()); }
    void    SetRow(SCROW nRow) { SetValue(nRow); }

protected:
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual Size    GetOptimalSize() const override;
    virtual void    LoseFocus() override;
    virtual void    dispose() override;

private:
    VclPtr<ScNavigatorDlg> xDlg;

    void    ExecuteRow();
};

class ScNavigatorDlg : public PanelLayout, public SfxListener
{
friend class ScNavigatorControllerItem;
friend class ScNavigatorDialogWrapper;
friend class ColumnEdit;
friend class RowEdit;
friend class ScContentTree;

private:
    static constexpr int CTRL_ITEMS = 4;

    SfxBindings&        rBindings;      // must be first member

    VclPtr<ColumnEdit> aEdCol;
    VclPtr<RowEdit> aEdRow;
    VclPtr<ToolBox> aTbxCmd;
    VclPtr<VclContainer> aContentBox;
    VclPtr<ScContentTree> aLbEntries;
    VclPtr<VclContainer> aScenarioBox;
    VclPtr<ScScenarioWindow> aWndScenarios;
    VclPtr<ListBox> aLbDocuments;

    Size            aExpandedSize;
    Idle            aContentIdle;

    OUString const  aStrDragMode;
    OUString const  aStrDisplay;
    OUString        aStrActive;
    OUString        aStrNotActive;
    OUString        aStrHidden;
    OUString const  aStrActiveWin;

    sal_uInt16      nZoomId;
    sal_uInt16      nChangeRootId;
    sal_uInt16      nDragModeId;
    sal_uInt16      nScenarioId;
    sal_uInt16      nDownId;
    sal_uInt16      nUpId;
    sal_uInt16      nDataId;
    std::unique_ptr<ScArea> pMarkArea;
    ScViewData*     pViewData;

    NavListMode     eListMode;
    sal_uInt16      nDropMode;
    SCCOL           nCurCol;
    SCROW           nCurRow;
    SCTAB           nCurTab;

    std::array<std::unique_ptr<ScNavigatorControllerItem>,CTRL_ITEMS> mvBoundItems;

    DECL_LINK(TimeHdl, Timer*, void);
    DECL_LINK(DocumentSelectHdl, ListBox&, void);
    DECL_LINK(ToolBoxSelectHdl, ToolBox*, void);
    DECL_LINK(ToolBoxDropdownClickHdl, ToolBox*, void);

    void    UpdateButtons();
    void    SetCurrentCell( SCCOL nCol, SCROW Row );
    void    SetCurrentCellStr( const OUString& rName );
    void    SetCurrentTable( SCTAB nTab );
    void    SetCurrentTableStr( const OUString& rName );
    void    SetCurrentObject( const OUString& rName );
    void    SetCurrentDoc( const OUString& rDocName );
    void    UpdateSelection();

    static ScTabViewShell*  GetTabViewShell();
    static ScNavigatorSettings* GetNavigatorSettings();
    bool                    GetViewData();

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

    const OUString& GetStrDragMode() const    { return aStrDragMode; }
    const OUString& GetStrDisplay() const     { return aStrDisplay; }

    void    CheckDataArea   ();
    void    MarkDataArea    ();
    void    UnmarkDataArea  ();
    void    StartOfDataArea ();
    void    EndOfDataArea   ();

    static void ReleaseFocus();

public:
    ScNavigatorDlg(SfxBindings* pB, vcl::Window* pParent);
    virtual ~ScNavigatorDlg() override;
    virtual void dispose() override;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void StateChanged(StateChangedType nStateChange) override;
};

class ScNavigatorDialogWrapper: public SfxChildWindowContext
{
public:
            ScNavigatorDialogWrapper( vcl::Window*           pParent,
                                      sal_uInt16            nId,
                                      SfxBindings*      pBindings,
                                      SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOWCONTEXT(ScNavigatorDialogWrapper)

private:
    VclPtr<ScNavigatorDlg> pNavigator;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_NAVIPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
