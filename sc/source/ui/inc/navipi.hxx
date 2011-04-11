/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
class ScRange;

//========================================================================

#define SC_DROPMODE_URL         0
#define SC_DROPMODE_LINK        1
#define SC_DROPMODE_COPY        2

enum NavListMode { NAV_LMODE_NONE       = 0x4000,
                   NAV_LMODE_AREAS      = 0x2000,
                   NAV_LMODE_DBAREAS    = 0x1000,
                   NAV_LMODE_DOCS       = 0x800,
                   NAV_LMODE_SCENARIOS  = 0x400 };

//========================================================================
// class ScScenarioListBox -----------------------------------------------
//========================================================================

class ScScenarioListBox : public ListBox
{
public:
    explicit            ScScenarioListBox( ScScenarioWindow& rParent );
    virtual             ~ScScenarioListBox();

    void                UpdateEntries( List* pNewEntryList );

protected:
    virtual void        Select();
    virtual void        DoubleClick();
    virtual long        Notify( NotifyEvent& rNEvt );

private:
    struct ScenarioEntry
    {
        String              maName;
        String              maComment;
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

//========================================================================
// class ScScenarioWindow ------------------------------------------------
//========================================================================
class ScScenarioWindow : public Window
{
public:
            ScScenarioWindow( Window* pParent,const String& aQH_List,const String& aQH_Comment);
            ~ScScenarioWindow();

    void    NotifyState( const SfxPoolItem* pState );
    void    SetComment( const String& rComment )
                { aEdComment.SetText( rComment ); }

    void    SetSizePixel( const Size& rNewSize );

protected:

    virtual void    Paint( const Rectangle& rRec );

private:
    ScScenarioListBox   aLbScenario;
    MultiLineEdit       aEdComment;
};

//==================================================================
//  class ColumnEdit
//==================================================================
class ColumnEdit : public SpinField
{
public:
            ColumnEdit( ScNavigatorDlg* pParent, const ResId& rResId );
            ~ColumnEdit();

    SCCOL   GetCol() { return nCol; }
    void    SetCol( SCCOL nColNo );

protected:
    virtual long    Notify( NotifyEvent& rNEvt );
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
    SCCOL   AlphaToNum      ( String& rStr );
    SCCOL   NumStrToAlpha   ( String& rStr );
    SCCOL   NumToAlpha      ( SCCOL nColNo, String& rStr );
};


//==================================================================
//  class RowEdit
//==================================================================
class RowEdit : public NumericField
{
public:
            RowEdit( ScNavigatorDlg* pParent, const ResId& rResId );
            ~RowEdit();

    SCROW   GetRow()                { return (SCROW)GetValue(); }
    void    SetRow( SCROW nRow ){ SetValue( nRow ); }

protected:
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    LoseFocus();

private:
    ScNavigatorDlg& rDlg;

    void    ExecuteRow();
};


//==================================================================
//  class ScDocListBox
//==================================================================
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


//==================================================================
//  class CommandToolBox
//==================================================================
class CommandToolBox : public ToolBox
{
public:
            CommandToolBox( ScNavigatorDlg* pParent, const ResId& rResId );
            ~CommandToolBox();

    void Select( sal_uInt16 nId );
    void UpdateButtons();
    void InitImageList();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    DECL_LINK( ToolBoxDropdownClickHdl, ToolBox* );

protected:
    virtual void    Select();
    virtual void    Click();

private:
    ScNavigatorDlg& rDlg;
};

//==================================================================
//  class ScNavigatorDlg
//==================================================================

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

    String          aTitleBase;
    String          aStrDragMode;
    String          aStrDisplay;
    String          aStrActive;
    String          aStrNotActive;
    String          aStrHidden;
    String          aStrActiveWin;

    SfxChildWindowContext*  pContextWin;
    Size                    aInitSize;
    ScArea*                 pMarkArea;
    ScViewData*             pViewData;

    long            nBorderOffset;
    long            nListModeHeight;
    long            nInitListHeight;
    NavListMode     eListMode;
    sal_uInt16          nDropMode;
    SCCOL           nCurCol;
    SCROW           nCurRow;
    SCTAB           nCurTab;
    sal_Bool            bFirstBig;

    ScNavigatorControllerItem** ppBoundItems;

    DECL_LINK( TimeHdl, Timer* );

    void    DoResize();

    SfxBindings&    GetBindings()
                    { return rBindings; }

    void    SetCurrentCell( SCCOL nCol, SCROW Row );
    void    SetCurrentCellStr( const String rName );
    void    SetCurrentTable( SCTAB nTab );
    void    SetCurrentTableStr( const String rName );
    void    SetCurrentObject( const String rName );
    void    SetCurrentDoc( const String& rDocName );

    ScTabViewShell*         GetTabViewShell() const;
    ScNavigatorSettings*    GetNavigatorSettings();
    sal_Bool                    GetViewData();

    void    UpdateColumn    ( const SCCOL* pCol = NULL );
    void    UpdateRow       ( const SCROW* pRow = NULL );
    void    UpdateTable     ( const SCTAB* pTab = NULL );
    void    UpdateAll       ();

    void    GetDocNames(const String* pSelEntry = NULL);

    void    SetListMode     ( NavListMode eMode, sal_Bool bSetSize = sal_True );
    void    ShowList        ( sal_Bool bShow, sal_Bool bSetSize );
    void    ShowScenarios   ( sal_Bool bShow, sal_Bool bSetSize );

    void    SetDropMode(sal_uInt16 nNew);
    sal_uInt16  GetDropMode() const         { return nDropMode; }

    const String& GetStrDragMode() const    { return aStrDragMode; }
    const String& GetStrDisplay() const     { return aStrDisplay; }

    void    CheckDataArea   ();
    void    MarkDataArea    ();
    void    UnmarkDataArea  ();
    void    StartOfDataArea ();
    void    EndOfDataArea   ();

    static void ReleaseFocus();

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRec );
    virtual void    Resizing( Size& rSize );

public:
                ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, Window* pParent );
                ~ScNavigatorDlg();

    using Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            CursorPosChanged();

    virtual SfxChildAlignment
                    CheckAlignment(SfxChildAlignment,SfxChildAlignment);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

//==================================================================

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
