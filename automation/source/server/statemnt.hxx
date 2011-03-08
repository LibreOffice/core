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
/***************************************************************************
**
**       Von StatementList werden alle Statements abgeleitet.
**   Es gibt immer nur eine Statementliste, die verpointert ist.
**       jederzeit kann das der Anfang der Kette abgefragt werden.
**
***************************************************************************/
#ifndef _STATEMNT_HXX
#define _STATEMNT_HXX

#include <vcl/wintypes.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <tools/fsys.hxx>
#include <sot/storage.hxx>
#include <basic/sbstar.hxx>
#include <vcl/event.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/smartid.hxx>
#include <automation/commtypes.hxx>

class Window;
class SystemWindow;
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

#ifdef __cplusplus
extern "C"
{
#endif
    void SAL_CALL osl_TestToolDebugPrint( const sal_Char *pString );
#ifdef __cplusplus
}
#endif


#define IsVisible IsReallyVisible
#define GET_REAL_PARENT() GetWindow( WINDOW_REALPARENT )

// switch behaviour of ImplMouse* and ImplKeyInput
#define FORCE_DIRECT_CALL   TRUE

typedef USHORT SearchFlags;
#define SEARCH_NOOVERLAP            ((SearchFlags) 0x0001)
#define SEARCH_NO_TOPLEVEL_WIN      ((SearchFlags) 0x0002)
#define SEARCH_FOCUS_FIRST          ((SearchFlags) 0x0004)
#define SEARCH_FIND_DISABLED        ((SearchFlags) 0x0008)

class Search
{
    SearchFlags nmSearchFlags;
public:
    Search( SearchFlags nSearchFlags = 0): nmSearchFlags(nSearchFlags) {}
    virtual ~Search() {}

    virtual BOOL IsWinOK( Window *pWin ) = 0;
    SearchFlags GetSearchFlags() { return nmSearchFlags; }
    void AddSearchFlags( SearchFlags aNewFlags ) { nmSearchFlags |= aNewFlags; }
    void RemoveSearchFlags( SearchFlags aRemoveFlags ) { nmSearchFlags &= ( ~aRemoveFlags ); }
    BOOL HasSearchFlag( SearchFlags aQueryFlag ) { return (nmSearchFlags & aQueryFlag) == aQueryFlag; }
};

BOOL IsDialog(Window *pWin);        // Ist *pWin von SystemWindow abgeleitet (Kann es Active sein)
BOOL IsAccessable(Window *pWin);    // Ist *pWin Zugreifbar (über IsEnabled und Parents geprüft)


class SafePointer
{
    SafePointer *pSelf;
public:
    SafePointer()   { pSelf = this; }
    virtual ~SafePointer()  { DBG_ASSERT(pSelf==this,"Destructor von Nicht existierendem Objekt aufgerufen");
                              pSelf = NULL; }
};


class DisplayHidWin;
class StatementCommand;
class TranslateWin;

struct TTSettings
{
    // DisplayHID
       StatementCommand *pDisplayInstance;
    DisplayHidWin *pDisplayHidWin;
    Window *Old;
    Window *Act;
    String aOriginalCaption;

    // Translate
    TranslateWin *pTranslateWin;
    BOOL bToTop;
};


TTSettings* GetTTSettings();


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
        static USHORT nModalCount;
    static Window *pLastFocusWindow;        // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
    static BOOL bWasDragManager;            // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
    static BOOL bWasPopupMenu;              // Wenn dieses sich ändert wird Safe Reschedule abgebrochen
       static BOOL bBasicWasRunning;

    static USHORT nMinTypeKeysDelay;                /// Verzögerung der einzelnen Anschläge für TypeKeys
    static USHORT nMaxTypeKeysDelay;
    static BOOL bDoTypeKeysDelay;

    static Window* pFirstDocFrame;

    static BOOL bIsSlotInExecute;

public:
    static BOOL IsInReschedule() { return bIsInReschedule; }
    void SafeReschedule( BOOL bYield = FALSE )  // Setzt Flag, so daß nicht schon der nächste Befehl ausgeführt wird
    {
        nModalCount = Application::GetModalModeCount();
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
        nModalCount = 0;
    }
    static BOOL MaybeResetSafeReschedule()
    {       // Implementierung muß hier zwar nicht sein, ist aber übersichtlicher so
        if ( !bIsInReschedule )
            return FALSE;

        if ( pLastFocusWindow != GetpApp()->GetFocusWindow()
            || ( Application::GetModalModeCount() > nModalCount )
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
    Window* GetFocus( WindowType nRT, BOOL MaybeBase = TRUE );
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

    USHORT GetCurrentMenues( PopupMenu *&pPopup, MenuBar *&pMenuBar, Menu *&pMenu );

public:
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
    static SmartId aWindowWaitUId;
    static Window *pWindowWaitPointer;
    static SmartId aWindowWaitOldHelpId;
    static SmartId aWindowWaitOldUniqueId;
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

    static BOOL bUsePostEvents;         // use Application::Post*Event or own impl to handle key and mouseevents

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
    BOOL bMenuClosed;

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
    comm_USHORT nNr1,nNr2,nNr3,nNr4;
    comm_ULONG nLNr1;
    String aString1,aString2;
    BOOL bBool1,bBool2;

    Window* GetNextOverlap( Window* pBase );
    Window* GetNextRecoverWin();

    static USHORT nDirPos;
    static Dir *pDir;
    static pfunc_osl_printDebugMessage pOriginal_osl_DebugMessageFunc;


    BOOL UnpackStorage( SotStorageRef xStorage, DirEntry &aBaseDir );

    void HandleSAXParser();

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
    comm_USHORT nNr1,nNr2,nNr3,nNr4;
    comm_ULONG nLNr1;
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
    comm_USHORT nSNr1;
    comm_ULONG nLNr1;
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
    Window *pAlternateResult;
    SmartId aUId;
    BOOL bSearchButtonOnToolbox;
public:
    SearchUID( SmartId aUIdP, BOOL bSearchButtonOnToolboxP ): Search( SEARCH_FOCUS_FIRST ), pMaybeResult(NULL), pAlternateResult(NULL), aUId(aUIdP), bSearchButtonOnToolbox(bSearchButtonOnToolboxP) {}
    virtual BOOL IsWinOK( Window *pWin );
    Window* GetMaybeWin() { return pMaybeResult; }
    Window* GetAlternateResultWin() { return pAlternateResult; }
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


void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt, BOOL bForceDirect=FALSE );
void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect=FALSE );
void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect=FALSE );
void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt, BOOL bForceDirect=FALSE );
void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt );
void ImplEventWait( ULONG nID );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
