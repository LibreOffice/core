/*************************************************************************
 *
 *  $RCSfile: navipi.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_NAVIPI_HXX
#define SC_NAVIPI_HXX

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef SC_CONTENT_HXX
#include "content.hxx"
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif


class ScViewData;
class ScArea;
class ScScenarioWindow;
class ScNavigatorControllerItem;
class ScNavigatorDialogWrapper;
class ScNavigatorDlg;
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
            ScScenarioListBox( Window* pParent );
            ~ScScenarioListBox();

    void UpdateEntries( List* pNewEntryList );

protected:
    virtual void    Select();
    virtual void    DoubleClick();
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual long    Notify( NotifyEvent& rNEvt );

private:
    ScScenarioWindow&   rParent;
    List                aEntryList;
    String              aCurText;
    Accelerator*        pAccel;
    void                ClearEntryList();
    void                CopyEntryList( List& rNewList );
    DECL_LINK( AccelSelectHdl, Accelerator * );
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

    USHORT  GetCol() { return nCol; }
    void    SetCol( USHORT nColNo );

protected:
#ifdef VCL
    virtual long    Notify( NotifyEvent& rNEvt );
#else
    virtual void    KeyInput( const KeyEvent& rKEvt );
#endif
    virtual void    LoseFocus();
    virtual void    Up();
    virtual void    Down();
    virtual void    First();
    virtual void    Last();

private:
    ScNavigatorDlg& rDlg;
    USHORT          nCol;
    USHORT          nKeyGroup;

    void    EvalText        ();
    void    ExecuteCol      ();
    USHORT  AlphaToNum      ( String& rStr );
    USHORT  NumStrToAlpha   ( String& rStr );
    USHORT  NumToAlpha      ( USHORT nColNo, String& rStr );
};


//==================================================================
//  class RowEdit
//==================================================================
class RowEdit : public NumericField
{
public:
            RowEdit( ScNavigatorDlg* pParent, const ResId& rResId );
            ~RowEdit();

    USHORT  GetRow()                { return (USHORT)GetValue(); }
    void    SetRow( USHORT nRow ){ SetValue( nRow ); }

protected:
#ifdef VCL
    virtual long    Notify( NotifyEvent& rNEvt );
#else
    virtual void    KeyInput( const KeyEvent& rKEvt );
#endif
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

    void Select( USHORT nId );
    void UpdateButtons();

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
    FixedInfo           aFtCol;
    ColumnEdit          aEdCol;
    FixedInfo           aFtRow;
    RowEdit             aEdRow;
    CommandToolBox      aTbxCmd;
    ScContentTree       aLbEntries;
    ScDocListBox        aLbDocuments;
    ScScenarioWindow    aWndScenarios;

    Timer           aContentTimer;

    String          aTitleBase;
    String          aStrDragMode;
    String          aStrDisplay;
    String          aStrActive;
    String          aStrNotActive;
    String          aStrHidden;
    String          aStrActiveWin;

    SfxChildWindowContext* pContextWin;
    Size            aInitSize;
    ScArea*         pMarkArea;
    ScViewData*     pViewData;
    long            nBorderOffset;
    long            nListModeHeight;
    long            nInitListHeight;
    NavListMode     eListMode;
    USHORT          nDropMode;
    USHORT          nCurCol;
    USHORT          nCurRow;
    USHORT          nCurTab;
    BOOL            bFirstBig;

    ScNavigatorControllerItem** ppBoundItems;
    SfxBindings&                rBindings;

    DECL_LINK( TimeHdl, Timer* );

    void    DoResize();

    BOOL    GetDBAtCursor( String& rStrName );
    BOOL    GetAreaAtCursor( String& rStrName );

    void    SetCurrentCell( USHORT nCol, USHORT Row );
    void    SetCurrentCellStr( const String rName );
    void    SetCurrentTable( USHORT nTab );
    void    SetCurrentTableStr( const String rName );
    void    SetCurrentObject( const String rName );
    void    SetCurrentDoc( const String& rDocName );

    BOOL    GetViewData     ();
    void    UpdateColumn    ( const USHORT* pCol = NULL );
    void    UpdateRow       ( const USHORT* pRow = NULL );
    void    UpdateTable     ( const USHORT* pTab = NULL );
    void    UpdateAll       ();

    void    GetDocNames(const String* pSelEntry = NULL);

    void    SetListMode     ( NavListMode eMode, BOOL bSetSize = TRUE );
    void    ShowList        ( BOOL bShow, BOOL bSetSize );
    void    ShowScenarios   ( BOOL bShow, BOOL bSetSize );

    void    SetDropMode(USHORT nNew);
    USHORT  GetDropMode() const         { return nDropMode; }

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

    virtual BOOL    Drop( const DropEvent& rEvt );
    virtual BOOL    QueryDrop( DropEvent& rEvt );

public:
                ScNavigatorDlg( SfxBindings* pB, SfxChildWindowContext* pCW, Window* pParent );
                ~ScNavigatorDlg();

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );

    void            CursorPosChanged();

    virtual SfxChildAlignment
                    CheckAlignment(SfxChildAlignment,SfxChildAlignment);
};

//==================================================================

class ScNavigatorDialogWrapper: public SfxChildWindowContext
{
public:
            ScNavigatorDialogWrapper( Window*           pParent,
                                      USHORT            nId,
                                      SfxBindings*      pBindings,
                                      SfxChildWinInfo*  pInfo );

    SFX_DECL_CHILDWINDOW_CONTEXT(ScNavigatorDialogWrapper)

    virtual void    Resizing( Size& rSize );

    virtual BOOL    Drop( const DropEvent& rEvt );
    virtual BOOL    QueryDrop( DropEvent& rEvt );

private:
    ScNavigatorDlg* pNavigator;
};



#endif // SC_NAVIPI_HXX

