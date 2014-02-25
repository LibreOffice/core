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

#ifndef SC_NAVIPI_HXX
#define SC_NAVIPI_HXX

#include <vector>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svl/poolitem.hxx>
#include <svl/lstner.hxx>
#include <sfx2/childwin.hxx>
#include "content.hxx"
#include <svtools/svmedit.hxx>


class ScTabViewShell;
class ScViewData;
class ScArea;
class ScScenarioWindow;
class ScNavigatorControllerItem;
class ScNavigatorDialogWrapper;
class ScNavigatorDlg;
class ScNavigatorSettings;



#define SC_DROPMODE_URL         0
#define SC_DROPMODE_LINK        1
#define SC_DROPMODE_COPY        2

enum NavListMode { NAV_LMODE_NONE       = 0x4000,
                   NAV_LMODE_AREAS      = 0x2000,
                   NAV_LMODE_DBAREAS    = 0x1000,
                   NAV_LMODE_DOCS       = 0x800,
                   NAV_LMODE_SCENARIOS  = 0x400 };


// class ScScenarioListBox -----------------------------------------------


class ScScenarioListBox : public ListBox
{
public:
    explicit            ScScenarioListBox( ScScenarioWindow& rParent );
    virtual             ~ScScenarioListBox();

    void                UpdateEntries( const std::vector<OUString> &aNewEntryList );

protected:
    virtual void        Select();
    virtual void        DoubleClick();
    virtual bool        Notify( NotifyEvent& rNEvt );

private:
    struct ScenarioEntry
    {
        OUString            maName;
        OUString            maComment;
        bool                mbProtected;

        inline explicit     ScenarioEntry() : mbProtected( false ) {}
    };
    typedef ::std::vector< ScenarioEntry > ScenarioList;

private:
    const ScenarioEntry* GetSelectedEntry() const;

    void                ExecuteScenarioSlot( sal_uInt16 nSlotId );
    void                SelectScenario();
    void                EditScenario();
    void                DeleteScenario( bool bQueryBox );

private:
    ScScenarioWindow&   mrParent;
    ScenarioList        maEntries;
};


// class ScScenarioWindow ------------------------------------------------

class ScScenarioWindow : public Window
{
public:
            ScScenarioWindow( Window* pParent, const OUString& aQH_List, const OUString& aQH_Comment);
            ~ScScenarioWindow();

    void    NotifyState( const SfxPoolItem* pState );
    void    SetComment( const OUString& rComment )
                { aEdComment.SetText( rComment ); }

    void    SetSizePixel( const Size& rNewSize );

protected:

    virtual void    Paint( const Rectangle& rRect );

private:
    ScScenarioListBox   aLbScenario;
    MultiLineEdit       aEdComment;
};


//  class ColumnEdit

class ColumnEdit : public SpinField
{
public:
            ColumnEdit( ScNavigatorDlg* pParent, const ResId& rResId );
            ~ColumnEdit();

    SCCOL   GetCol() { return nCol; }
    void    SetCol( SCCOL nColNo );

protected:
    virtual bool    Notify( NotifyEvent& rNEvt );
    virtual void    LoseFocus();
    virtual void    Up();
    virtual void    Down();
    virtual void    First();
    virtual void    Last();

private:
    ScNavigatorDlg& rDlg;
    SCCOL           nCol;
    sal_uInt16          nKeyGroup;

    void    EvalText        ();
    void    ExecuteCol      ();
    SCCOL   AlphaToNum      ( OUString& rStr );
    SCCOL   NumStrToAlpha   ( OUString& rStr );
    SCCOL   NumToAlpha      ( SCCOL nColNo, OUString& rStr );
};



//  class RowEdit

class RowEdit : public NumericField
{
public:
            RowEdit( ScNavigatorDlg* pParent, const ResId& rResId );
            ~RowEdit();

    SCROW   GetRow()                { return (SCROW)GetValue(); }
    void    SetRow( SCROW nRow ){ SetValue( nRow ); }

protected:
    virtual bool    Notify( NotifyEvent& rNEvt );
    virtual void    LoseFocus();

private:
    ScNavigatorDlg& rDlg;

    void    ExecuteRow();
};



//  class ScDocListBox

class ScDocListBox : public ListBox
{
public:
            ScDocListBox( ScNavigatorDlg* pParent, const ResId& rResId );
            ~ScDocListBox();

protected:
    virtual void    Select();

private:
    ScNavigatorDlg& rDlg;
};



//  class CommandToolBox

class CommandToolBox : public ToolBox
{
public:
            CommandToolBox( ScNavigatorDlg* pParent, const ResId& rResId );
            ~CommandToolBox();

    void Select( sal_uInt16 nId );
    void UpdateButtons();
    void InitImageList();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    DECL_LINK( ToolBoxDropdownClickHdl, void* );

protected:
    virtual void    Select();
    virtual void    Click();

private:
    ScNavigatorDlg& rDlg;
};


//  class ScNavigatorDlg


class ScNavigatorDlg : public Window, public SfxListener
{
friend class ScNavigatorControllerItem;
friend class ScNavigatorDialogWrapper;
friend class ColumnEdit;
friend class RowEdit;
friend class ScDocListBox;
friend class CommandToolBox;
friend class ScContentTree;

private:
    SfxBindings&        rBindings;      // must be first member

    ImageList           aCmdImageList;  // must be before aTbxCmd
    FixedInfo           aFtCol;
    ColumnEdit          aEdCol;
    FixedInfo           aFtRow;
    RowEdit             aEdRow;
    CommandToolBox      aTbxCmd;
    ScContentTree       aLbEntries;
    ScScenarioWindow    aWndScenarios;
    ScDocListBox        aLbDocuments;

    Timer           aContentTimer;

    OUString        aTitleBase;
    OUString        aStrDragMode;
    OUString        aStrDisplay;
    OUString        aStrActive;
    OUString        aStrNotActive;
    OUString        aStrHidden;
    OUString        aStrActiveWin;

    SfxChildWindowContext*  pContextWin;
    Size                    aInitSize;
    ScArea*                 pMarkArea;
    ScViewData*             pViewData;

    long            nBorderOffset;
    long            nListModeHeight;
    long            nInitListHeight;
    NavListMode     eListMode;
    sal_uInt16      nDropMode;
    SCCOL           nCurCol;
    SCROW           nCurRow;
    SCTAB           nCurTab;
    bool            bFirstBig;
    bool mbUseStyleSettingsBackground;

    ScNavigatorControllerItem** ppBoundItems;

    DECL_LINK( TimeHdl, Timer* );

    void    DoResize();

    SfxBindings&    GetBindings()
                    { return rBindings; }

    void    SetCurrentCell( SCCOL nCol, SCROW Row );
    void    SetCurrentCellStr( const OUString rName );
    void    SetCurrentTable( SCTAB nTab );
    void    SetCurrentTableStr( const OUString& rName );
    void    SetCurrentObject( const OUString rName );
    void    SetCurrentDoc( const OUString& rDocName );

    ScTabViewShell*         GetTabViewShell() const;
    ScNavigatorSettings*    GetNavigatorSettings();
    bool                    GetViewData();

    void    UpdateColumn    ( const SCCOL* pCol = NULL );
    void    UpdateRow       ( const SCROW* pRow = NULL );
    void    UpdateTable     ( const SCTAB* pTab = NULL );
    void    UpdateAll       ();

    void    GetDocNames(const OUString* pSelEntry = NULL);

    void    SetListMode     ( NavListMode eMode, bool bSetSize = true );
    void    ShowList        ( bool bShow, bool bSetSize );
    void    ShowScenarios   ( bool bShow, bool bSetSize );

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

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resizing( Size& rSize );

public:
                ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, Window* pParent,
                    const bool bUseStyleSettingsBackground);
                ~ScNavigatorDlg();

    using Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            CursorPosChanged();

    virtual SfxChildAlignment
                    CheckAlignment(SfxChildAlignment,SfxChildAlignment);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};



class ScNavigatorDialogWrapper: public SfxChildWindowContext
{
public:
            ScNavigatorDialogWrapper( Window*           pParent,
                                      sal_uInt16            nId,
                                      SfxBindings*      pBindings,
                                      SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOWCONTEXT(ScNavigatorDialogWrapper)

    virtual void    Resizing( Size& rSize );

private:
    ScNavigatorDlg* pNavigator;
};



#endif // SC_NAVIPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
