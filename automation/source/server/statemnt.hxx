/*************************************************************************
 *
 *  $RCSfile: statemnt.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:49:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/***************************************************************************
**
**       Von StatementList werden alle Statements abgeleitet.
**   Es gibt immer nur eine Statementliste, die verpointert ist.
**       jederzeit kann das der Anfang der Kette abgefragt werden.
**
**
**
**
**
**
**
***************************************************************************/
#ifndef _STATEMNT_HXX
#define _STATEMNT_HXX

#ifndef _SV_WINTYPES_HXX //autogen
#include <vcl/wintypes.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _SV_DRAG_HXX //autogen
//#include <vcl/drag.hxx>
#endif
#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SB_SBSTAR_HXX
#include <basic/sbstar.hxx>
#endif
#include <vcl/event.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _SMARTID_HXX_
#include "smartid.hxx"
#endif

class Window;
class Point;
class SfxPoolItem;

class ScrollBar;

class SCmdStream;
class RetStream;
class ImplRemoteControl;

class TTProfiler;
class TTProperties;

class Dir;

class CommunicationLink;

#if OSL_DEBUG_LEVEL > 1
class EditWindow;
#endif

void SAL_CALL osl_TestToolDebugPrint( const sal_Char *pString );

typedef USHORT SearchFlags;
#define SEARCH_NOOVERLAP            ((SearchFlags) 0x0001)
#define SEARCH_NO_TOPLEVEL_WIN      ((SearchFlags) 0x0002)
#define SEARCH_FOCUS_FIRST          ((SearchFlags) 0x0004)
#define SEARCH_FIND_DISABLED        ((SearchFlags) 0x0008)

#define IsVisible IsReallyVisible
#define GET_REAL_PARENT() GetWindow( WINDOW_REALPARENT )

class Search
{
    SearchFlags nSearchFlags;
public:
    Search( SearchFlags nPSearchFlags = 0): nSearchFlags(nPSearchFlags) {}
    virtual ~Search() {}

    virtual BOOL IsWinOK( Window *pWin ) = 0;
    SearchFlags GetSearchFlags() { return nSearchFlags; }
    void AddSearchFlags( SearchFlags aNewFlags ) { nSearchFlags |= aNewFlags; }
    void RemoveSearchFlags( SearchFlags aRemoveFlags ) { nSearchFlags &= ( ~aRemoveFlags ); }
    BOOL HasSearchFlag( SearchFlags aQueryFlag ) { return (nSearchFlags & aQueryFlag) == aQueryFlag; }
};

BOOL IsDialog(Window *pWin);        // Ist *pWin von SystemWindow abgeleitet (Kann es Active sein)
BOOL IsAccessable(Window *pWin);    // Ist *pWin Zugreifbar (über IsEnabled und Parents geprüft)


//class SafePointer : CriticalSection
class SafePointer
{
    SafePointer *pSelf;
public:
    SafePointer()   { pSelf = this; }
    virtual ~SafePointer()  { DBG_ASSERT(pSelf==this,"Destructor von Nicht existierendem Objekt aufgerufen");
                              pSelf = NULL; }
//  static BOOL IsValid( SafePointer *pThis ) { return pThis == pThis->pSelf; }
// virtual      operator -> (); { DBG_ASSERT(pMyself == this,"-> von Nicht existierendem Objekt aufgerufen"); }
};


#define MAX_RETRIES 9
class StatementList : public SafePointer
{
private:
    StatementList(const StatementList&);
    StatementList & operator=(const StatementList&);

protected:
    StatementList();
    USHORT nRetryCount;
    void QueStatement(StatementList *pAfterThis);
    BOOL bStatementInQue;
    static USHORT nUseBindings;

    static TTProfiler *pProfiler;
    void InitProfile();
    void SendProfile( String aText );
    static StatementList *pCurrentProfileStatement;

    static BOOL bIsInReschedule;
    static Window *pLastFocusWindow;        // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
    static BOOL bWasDragManager;            // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
    static BOOL bWasPopupMenu;              // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
       static BOOL bBasicWasRunning;

    static USHORT nControlType;             // Auf diesen Typ werden Controls gecastst

    static USHORT nMinTypeKeysDelay;                /// Verzögerung der einzelnen Anschläge für TypeKeys
    static USHORT nMaxTypeKeysDelay;
    static BOOL bDoTypeKeysDelay;

    static Window* pFirstDocFrame;

public:
    static BOOL IsInReschedule() { return bIsInReschedule; }
    void SafeReschedule( BOOL bYield = FALSE )  // Setzt Flag, so daß nicht schon der nächste Befehl ausgeführt wird
    {
        bIsInReschedule = TRUE;
        pLastFocusWindow = GetpApp()->GetFocusWindow();
        bWasDragManager = false /*!= DragManager::GetDragManager()*/;
        bWasPopupMenu = NULL != PopupMenu::GetActivePopupMenu();
        bBasicWasRunning = StarBASIC::IsRunning();
        bWasExecuting = bExecuting;
        if ( bYield )
            GetpApp()->Yield();
        else
            GetpApp()->Reschedule();
        bExecuting = bWasExecuting;
        bBasicWasRunning = FALSE;
        bWasPopupMenu = FALSE;
        bWasDragManager = FALSE;
        pLastFocusWindow = NULL;
        bIsInReschedule = FALSE;
    }
    static BOOL MaybeResetSafeReschedule()
    {       // Implementierung muß hier zwar nicht sein, ist aber übersichtlicher so
        if ( !bIsInReschedule )
            return FALSE;

        if ( pLastFocusWindow != GetpApp()->GetFocusWindow()
//          || ( DragManager::GetDragManager() && !bWasDragManager )
            || ( PopupMenu::GetActivePopupMenu() && !bWasPopupMenu )
            || ( StarBASIC::IsRunning() && !bBasicWasRunning ) )
        {
            bIsInReschedule = FALSE;
            pLastFocusWindow = NULL;
            return TRUE;
        }
        else
            return FALSE;
    }
    static void NormalReschedule()  // Setzt das flag nicht
    {
        GetpApp()->Reschedule();
    }
#define Reschedule RescheduleNichtBenutzen_StattdessenSafeRescheduleAnStatementList

    static Window* GetMouseWin();
    static BOOL WinPtrValid(Window *pTest);
    static Window* SearchAllWin( Window *pBase, Search &aSearch, BOOL MaybeBase = TRUE );
protected:
    static Window* SearchClientWin( Window *pBase, Search &aSearch, BOOL MaybeBase = TRUE );

    Window* SearchTree( SmartId aUId, BOOL bSearchButtonOnToolbox = FALSE );
    Window* GetActive( WindowType nRT, BOOL MaybeBase = TRUE );
    Window* GetFocus( WindowType nRT ,BOOL ParentWasRT = FALSE, BOOL MaybeBase = TRUE );
    Window* GetAnyActive( BOOL MaybeBase = TRUE );
    ScrollBar* GetScrollBar( Window *pBase, USHORT nDirection, BOOL MaybeBase = TRUE );
    Window* GetPopupFloatingWin( BOOL MaybeBase = TRUE );
    Menu* GetMatchingMenu( Window* pWin, Menu* pBaseMenu = NULL );
    Window* GetWinByRT( Window *pBase, WindowType nRT, BOOL MaybeBase = TRUE, USHORT nSkip = 0, BOOL bSearchAll = FALSE );
    USHORT CountWinByRT( Window *pBase, WindowType nRT, BOOL MaybeBase = TRUE );
    Window* GetDocWin( USHORT nNr );
    USHORT GetDocWinCount();
    Window* GetFadeSplitWin( Window *pBase, WindowAlign nAlign, BOOL MaybeBase = TRUE );
    BOOL ValueOK(SmartId nId, String aBezeichnung, ULONG nValue, ULONG nMax);

public:
//  void AddStatement( StatementList *pNewStatement );

    virtual ~StatementList();
    void Advance();
    virtual BOOL Execute() = 0;
/***************************************************************************
** Bestimmt erst den nächsten Befehl, setzt Current
** und führt dann aus.
** Returnwert gibt an, ob Befehl nochmal ausgeführt
** werden soll. Dann muß auch der UserEvent verlassen werden, um der Applikation
** normales Arbeiten zu ermöglichen (Dialog schliessen)
** TRUE bedeutet, dass alles klar gegangen ist
** FALSE bedeutet nochmal Bitte
***************************************************************************/

    void ReportError(String aMessage);
    void ReportError(SmartId aUId, String aMessage);
    void ReportError(String aMessage, ULONG nWhatever);

    static void DirectLog( ULONG nType, String aString );

    String Tree(Window *pBase, int Indent);
    String ClientTree(Window *pBase, int Indent);

    StatementList *pNext;
    static StatementList /**pCurrent,*/ *pFirst;
    static BOOL bReadingCommands;
    static ULONG nWindowWaitUId;
    static Window *pWindowWaitPointer;
    static ULONG nWindowWaitOldHelpId;
    static ULONG nWindowWaitOldUniqueId;
    static RetStream *pRet;
    static BOOL IsError;
    static BOOL bDying;
    static BOOL bExecuting;             // Gesetzt, wenn ein Befehl rescheduled ohne einen neuen Befehl zu erlauben
    BOOL bWasExecuting;                 // Wurde bei einem MaybeResetSafeReschedule resettet, so wird der Zustand danach wiederhergestellt
    static SmartId aSubMenuId1;         // Untermenüs bei PopupMenus
    static SmartId aSubMenuId2;         // erstmal 2-Stufig
    static SmartId aSubMenuId3;         // and now even 3 levels #i31512#
    static SystemWindow *pMenuWindow;   // when using MenuBar as base for MenuCommands
    static TTProperties *pTTProperties; // Hier stehen die SlotIDs aus dem SFX drin

    BOOL CheckWindowWait();         //True heisst, dass Window noch existiert
                                    //False -> Window weg;
    static void SetFirstDocFrame( Window* pWin );
    static Window* GetFirstDocFrame();
    static BOOL IsFirstDocFrame( Window* pWin );
    static BOOL IsDocWin( Window* pWin );
    static BOOL IsIMEWin( Window* pWin );    // Input Window for CJK under Solaris
    static BOOL IsDocFrame( Window* pWin );
    static MenuBar* GetDocFrameMenuBar( Window* pWin );
    static USHORT GetDocFrameCount();

    static BOOL bCatchGPF;
#if OSL_DEBUG_LEVEL > 1
    static EditWindow *m_pDbgWin;
#endif
};

class StatementSlot : public StatementList  //Slots aufrufen
{
protected:
    USHORT nAnzahl;
    SfxPoolItem **pItemArr;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aArgs;
    USHORT nFunctionId;     // can get removed when the old (numeric) slothandling is removed
    String aUnoUrl;

    StatementSlot();
    void AddReferer();
public:
    StatementSlot( SCmdStream *pIn );
    StatementSlot( ULONG nSlot, SfxPoolItem* pItem = NULL );
    virtual ~StatementSlot();
    virtual BOOL Execute();
};

class StatementUnoSlot : public StatementSlot   //Uno Slots aufrufen
{
public:
    StatementUnoSlot(SCmdStream *pIn);
};

class StatementCommand : public StatementList   // Befehl ausführen (wintree, resetaplication ...)
{
    friend class ImplRemoteControl;
protected:
    USHORT nMethodId;
    SmartId aSmartMethodId;
    USHORT nParams;
    USHORT nNr1,nNr2,nNr3,nNr4;
    ULONG nLNr1;
    String aString1,aString2;
    BOOL bBool1,bBool2;

    Window* GetNextOverlap( Window* pBase );
    Window* GetNextRecoverWin();

    static USHORT nDirPos;
    static Dir *pDir;
    static pfunc_osl_printDebugMessage pOriginal_osl_DebugMessageFunc;


    BOOL UnpackStorage( SotStorageRef xStorage, DirEntry &aBaseDir );

    void HandleSAXParser();

    void HandleMacroRecorder();

public:
    StatementCommand( SCmdStream *pIn );
    StatementCommand( StatementList *pAfterThis, USHORT MethodId, USHORT Params, USHORT Nr1 );
    virtual BOOL Execute();
    BOOL DisplayHID();
    void Translate();
    void WriteControlData( Window *pBase, ULONG nConf, BOOL bFirst = TRUE );

};


enum TTHotSpots  { MitteLinks, Mitte, MitteOben };

class StatementControl : public StatementList
{
protected:
    SmartId aUId;
    USHORT nMethodId;
    USHORT nParams;
    USHORT nNr1,nNr2,nNr3,nNr4;
    ULONG nLNr1;
    String aString1,aString2;
    BOOL bBool1,bBool2;
    BOOL ControlOK( Window *pControl, const sal_Char* aBezeichnung );
    void AnimateMouse( Window *pControl, TTHotSpots aWohin );
    void AnimateMouse( Window *pControl, Point aWohin );

    BOOL MaybeDoTypeKeysDelay( Window *pTestWindow );

    BOOL HandleVisibleControls( Window *pControl );
    BOOL HandleCommonMethods( Window *pControl );

public:
    StatementControl( SCmdStream *pIn, USHORT nControlType );
    virtual BOOL Execute();

};

class StatementFlow : public StatementList      // Kommunikation mit Sequence
{
    USHORT nArt;

    USHORT nParams;
    USHORT nSNr1;
    ULONG nLNr1;
    String aString1;
    BOOL bBool1;


public:
    StatementFlow (ULONG nServiceId, SCmdStream *pIn, ImplRemoteControl *pRC );
    StatementFlow( StatementList *pAfterThis, USHORT nArtP );
    virtual BOOL Execute();
    static CommunicationLink *pCommLink;
    static BOOL bSending;

    static BOOL bUseIPC;    // Soll zur rückmeldung IPC verwendet werden?
    static ImplRemoteControl *pRemoteControl;   // Static für 2. Constructor

private:
    void SendViaSocket();
};

class SearchUID : public Search
{
    Window *pMaybeResult;
    SmartId aUId;
    BOOL bSearchButtonOnToolbox;
public:
    SearchUID( SmartId aUIdP, BOOL bSearchButtonOnToolboxP ): Search( SEARCH_FOCUS_FIRST ), pMaybeResult(NULL), aUId(aUIdP), bSearchButtonOnToolbox(bSearchButtonOnToolboxP) {}
    virtual BOOL IsWinOK( Window *pWin );
    Window* GetMaybeWin() { return pMaybeResult; }
};
class SearchActive : public Search
{
    WindowType nRT;
public:
    SearchActive( WindowType nRTP ): nRT(nRTP) {}
    virtual BOOL IsWinOK( Window *pWin );
};
class SearchPopupFloatingWin : public Search
{
public:
    SearchPopupFloatingWin(): Search( SEARCH_FOCUS_FIRST ) {}
    virtual BOOL IsWinOK( Window *pWin );
};
class SearchRT : public Search
{
    WindowType mnRT;
    USHORT mnSkip;
    USHORT mnCount;
public:
    SearchRT( WindowType nRTP, SearchFlags nSearchFlags, USHORT nSkip = 0 ): Search(nSearchFlags), mnRT(nRTP), mnSkip( nSkip ), mnCount( 0 ) {}
    virtual BOOL IsWinOK( Window *pWin );
    USHORT GetCount(){ return mnCount; }
};
class SearchScroll : public SearchRT
{
    USHORT nDirection;
public:
    SearchScroll( USHORT nDir, SearchFlags nSearchFlags ): SearchRT(WINDOW_SCROLLBAR, nSearchFlags), nDirection(nDir) {}
    virtual BOOL IsWinOK( Window *pWin );
};
class SearchWinPtr : public Search
{
    Window *pTest;
public:
    SearchWinPtr( Window *pTestP ): pTest(pTestP) {}
    virtual BOOL IsWinOK( Window *pWin );
};
class SearchFadeSplitWin : public Search
{
    WindowAlign nAlign;
public:
    SearchFadeSplitWin( WindowAlign nAlignP ): nAlign(nAlignP) {}
    virtual BOOL IsWinOK( Window *pWin );
};


void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt );
void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt );
void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt );
void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt );
void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt );

#endif
