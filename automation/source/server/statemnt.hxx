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

#include <tools/wintypes.hxx>
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
#define FORCE_DIRECT_CALL   sal_True

typedef sal_uInt16 SearchFlags;
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

    virtual sal_Bool IsWinOK( Window *pWin ) = 0;
    SearchFlags GetSearchFlags() { return nmSearchFlags; }
    void AddSearchFlags( SearchFlags aNewFlags ) { nmSearchFlags |= aNewFlags; }
    void RemoveSearchFlags( SearchFlags aRemoveFlags ) { nmSearchFlags &= ( ~aRemoveFlags ); }
    sal_Bool HasSearchFlag( SearchFlags aQueryFlag ) { return (nmSearchFlags & aQueryFlag) == aQueryFlag; }
};

sal_Bool IsDialog(Window *pWin);        // Ist *pWin von SystemWindow abgeleitet (Kann es Active sein)
sal_Bool IsAccessable(Window *pWin);    // Ist *pWin Zugreifbar (�ber IsEnabled und Parents gepr�ft)


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
    sal_Bool bToTop;
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
    sal_uInt16 nRetryCount;
    void QueStatement(StatementList *pAfterThis);
    sal_Bool bStatementInQue;
    static sal_uInt16 nUseBindings;

    static TTProfiler *pProfiler;
    void InitProfile();
    void SendProfile( String aText );
    static StatementList *pCurrentProfileStatement;

    static sal_Bool bIsInReschedule;
        static sal_uInt16 nModalCount;
    static Window *pLastFocusWindow;        // Wenn dieses sich �ndert wird Safe Reschedule abgebrochen
    static sal_Bool bWasDragManager;            // Wenn dieses sich �ndert wird Safe Reschedule abgebrochen
    static sal_Bool bWasPopupMenu;              // Wenn dieses sich �ndert wird Safe Reschedule abgebrochen
       static sal_Bool bBasicWasRunning;

    static sal_uInt16 nMinTypeKeysDelay;                /// Verz�gerung der einzelnen Anschl�ge f�r TypeKeys
    static sal_uInt16 nMaxTypeKeysDelay;
    static sal_Bool bDoTypeKeysDelay;

    static Window* pFirstDocFrame;

    static sal_Bool bIsSlotInExecute;

public:
    static sal_Bool IsInReschedule() { return bIsInReschedule; }
    void SafeReschedule( sal_Bool bYield = sal_False )  // Setzt Flag, so da� nicht schon der n�chste Befehl ausgef�hrt wird
    {
        nModalCount = Application::GetModalModeCount();
        bIsInReschedule = sal_True;
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
        bBasicWasRunning = sal_False;
        bWasPopupMenu = sal_False;
        bWasDragManager = sal_False;
        pLastFocusWindow = NULL;
        bIsInReschedule = sal_False;
        nModalCount = 0;
    }
    static sal_Bool MaybeResetSafeReschedule()
    {       // Implementierung mu� hier zwar nicht sein, ist aber �bersichtlicher so
        if ( !bIsInReschedule )
            return sal_False;

        if ( pLastFocusWindow != GetpApp()->GetFocusWindow()
            || ( Application::GetModalModeCount() > nModalCount )
            || ( PopupMenu::GetActivePopupMenu() && !bWasPopupMenu )
            || ( StarBASIC::IsRunning() && !bBasicWasRunning ) )
        {
            bIsInReschedule = sal_False;
            pLastFocusWindow = NULL;
            return sal_True;
        }
        else
            return sal_False;
    }
    static void NormalReschedule()  // Setzt das flag nicht
    {
        GetpApp()->Reschedule();
    }
#define Reschedule RescheduleNichtBenutzen_StattdessenSafeRescheduleAnStatementList

    static Window* GetMouseWin();
    static sal_Bool WinPtrValid(Window *pTest);
    static Window* SearchAllWin( Window *pBase, Search &aSearch, sal_Bool MaybeBase = sal_True );
protected:
    static Window* SearchClientWin( Window *pBase, Search &aSearch, sal_Bool MaybeBase = sal_True );

    Window* SearchTree( rtl::OString aUId, sal_Bool bSearchButtonOnToolbox = sal_False );
    Window* GetActive( WindowType nRT, sal_Bool MaybeBase = sal_True );
    Window* GetFocus( WindowType nRT, sal_Bool MaybeBase = sal_True );
    Window* GetAnyActive( sal_Bool MaybeBase = sal_True );
    ScrollBar* GetScrollBar( Window *pBase, sal_uInt16 nDirection, sal_Bool MaybeBase = sal_True );
    Window* GetPopupFloatingWin( sal_Bool MaybeBase = sal_True );
    Menu* GetMatchingMenu( Window* pWin, Menu* pBaseMenu = NULL );
    Window* GetWinByRT( Window *pBase, WindowType nRT, sal_Bool MaybeBase = sal_True, sal_uInt16 nSkip = 0, sal_Bool bSearchAll = sal_False );
    sal_uInt16 CountWinByRT( Window *pBase, WindowType nRT, sal_Bool MaybeBase = sal_True );
    Window* GetDocWin( sal_uInt16 nNr );
    sal_uInt16 GetDocWinCount();
    Window* GetFadeSplitWin( Window *pBase, WindowAlign nAlign, sal_Bool MaybeBase = sal_True );
    sal_Bool ValueOK(rtl::OString nId, String aBezeichnung, sal_uLong nValue, sal_uLong nMax);

    sal_uInt16 GetCurrentMenues( PopupMenu *&pPopup, MenuBar *&pMenuBar, Menu *&pMenu );

public:
    virtual ~StatementList();
    void Advance();
    virtual sal_Bool Execute() = 0;
/***************************************************************************
** Bestimmt erst den n�chsten Befehl, setzt Current
** und f�hrt dann aus.
** Returnwert gibt an, ob Befehl nochmal ausgef�hrt
** werden soll. Dann mu� auch der UserEvent verlassen werden, um der Applikation
** normales Arbeiten zu erm�glichen (Dialog schliessen)
** sal_True bedeutet, dass alles klar gegangen ist
** sal_False bedeutet nochmal Bitte
***************************************************************************/

    void ReportError(String aMessage);
    void ReportError(rtl::OString aUId, String aMessage);
    void ReportError(String aMessage, sal_uLong nWhatever);

    static void DirectLog( sal_uLong nType, String aString );

    String Tree(Window *pBase, int Indent);
    String ClientTree(Window *pBase, int Indent);

    StatementList *pNext;
    static StatementList /**pCurrent,*/ *pFirst;
    static sal_Bool bReadingCommands;
    static rtl::OString aWindowWaitUId;
    static Window *pWindowWaitPointer;
    static rtl::OString aWindowWaitOldHelpId;
    static rtl::OString aWindowWaitOldUniqueId;
    static RetStream *pRet;
    static sal_Bool IsError;
    static sal_Bool bDying;
    static sal_Bool bExecuting;             // Gesetzt, wenn ein Befehl rescheduled ohne einen neuen Befehl zu erlauben
    sal_Bool bWasExecuting;                 // Wurde bei einem MaybeResetSafeReschedule resettet, so wird der Zustand danach wiederhergestellt
    static sal_uInt16 aSubMenuId1;          // Untermen�s bei PopupMenus
    static sal_uInt16 aSubMenuId2;          // erstmal 2-Stufig
    static sal_uInt16 aSubMenuId3;          // and now even 3 levels #i31512#
    static SystemWindow *pMenuWindow;   // when using MenuBar as base for MenuCommands
    static TTProperties *pTTProperties; // Hier stehen die SlotIDs aus dem SFX drin

    sal_Bool CheckWindowWait();         //True heisst, dass Window noch existiert
                                    //False -> Window weg;
    static void SetFirstDocFrame( Window* pWin );
    static Window* GetFirstDocFrame();
    static sal_Bool IsFirstDocFrame( Window* pWin );
    static sal_Bool IsDocWin( Window* pWin );
    static sal_Bool IsIMEWin( Window* pWin );    // Input Window for CJK under Solaris
    static sal_Bool IsDocFrame( Window* pWin );
    static MenuBar* GetDocFrameMenuBar( Window* pWin );
    static sal_uInt16 GetDocFrameCount();

    static sal_Bool bCatchGPF;

    static sal_Bool bUsePostEvents;         // use Application::Post*Event or own impl to handle key and mouseevents

#if OSL_DEBUG_LEVEL > 1
    static EditWindow *m_pDbgWin;
#endif
};

class StatementSlot : public StatementList  //Slots aufrufen
{
protected:
    sal_uInt16 nAnzahl;
    SfxPoolItem **pItemArr;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aArgs;
    sal_uInt16 nFunctionId;     // can get removed when the old (numeric) slothandling is removed
    String aUnoUrl;
    sal_Bool bMenuClosed;

    StatementSlot();
    void AddReferer();
public:
    StatementSlot( SCmdStream *pIn );
    StatementSlot( sal_uLong nSlot, SfxPoolItem* pItem = NULL );
    virtual ~StatementSlot();
    virtual sal_Bool Execute();
};

class StatementUnoSlot : public StatementSlot   //Uno Slots aufrufen
{
public:
    StatementUnoSlot(SCmdStream *pIn);
};

class StatementCommand : public StatementList   // Befehl ausf�hren (wintree, resetaplication ...)
{
    friend class ImplRemoteControl;
protected:
    sal_uInt16 nMethodId;
    sal_uInt16 nParams;
    comm_USHORT nNr1,nNr2,nNr3,nNr4;
    comm_ULONG nLNr1;
    String aString1,aString2;
    sal_Bool bBool1,bBool2;

    Window* GetNextOverlap( Window* pBase );
    Window* GetNextRecoverWin();

    static sal_uInt16 nDirPos;
    static Dir *pDir;
    static pfunc_osl_printDebugMessage pOriginal_osl_DebugMessageFunc;


    sal_Bool UnpackStorage( SotStorageRef xStorage, DirEntry &aBaseDir );

    void HandleSAXParser();

public:
    StatementCommand( SCmdStream *pIn );
    StatementCommand( StatementList *pAfterThis, sal_uInt16 MethodId, sal_uInt16 Params, sal_uInt16 Nr1 );
    virtual sal_Bool Execute();
    sal_Bool DisplayHID();
    void Translate();
    void WriteControlData( Window *pBase, sal_uLong nConf, sal_Bool bFirst = sal_True );

};


enum TTHotSpots  { MitteLinks, Mitte, MitteOben };

class StatementControl : public StatementList
{
protected:
    rtl::OString aUId;
    sal_uInt16 nMethodId;
    sal_uInt16 nParams;
    comm_USHORT nNr1,nNr2,nNr3,nNr4;
    comm_ULONG nLNr1;
    String aString1,aString2;
    sal_Bool bBool1,bBool2;
    sal_Bool ControlOK( Window *pControl, const sal_Char* aBezeichnung );
    void AnimateMouse( Window *pControl, TTHotSpots aWohin );
    void AnimateMouse( Window *pControl, Point aWohin );

    sal_Bool MaybeDoTypeKeysDelay( Window *pTestWindow );

    sal_Bool HandleVisibleControls( Window *pControl );
    sal_Bool HandleCommonMethods( Window *pControl );

public:
    StatementControl( SCmdStream *pIn, sal_uInt16 nControlType );
    virtual sal_Bool Execute();

};

class StatementFlow : public StatementList      // Kommunikation mit Sequence
{
    sal_uInt16 nArt;

    sal_uInt16 nParams;
    comm_USHORT nSNr1;
    comm_ULONG nLNr1;
    String aString1;
    sal_Bool bBool1;


public:
    StatementFlow (sal_uLong nServiceId, SCmdStream *pIn, ImplRemoteControl *pRC );
    StatementFlow( StatementList *pAfterThis, sal_uInt16 nArtP );
    virtual sal_Bool Execute();
    static CommunicationLink *pCommLink;
    static sal_Bool bSending;

    static sal_Bool bUseIPC;    // Soll zur r�ckmeldung IPC verwendet werden?
    static ImplRemoteControl *pRemoteControl;   // Static f�r 2. Constructor

private:
    void SendViaSocket();
};

class SearchUID : public Search
{
    Window *pMaybeResult;
    Window *pAlternateResult;
    rtl::OString aUId;
    sal_Bool bSearchButtonOnToolbox;
public:
    SearchUID( rtl::OString aUIdP, sal_Bool bSearchButtonOnToolboxP ): Search( SEARCH_FOCUS_FIRST ), pMaybeResult(NULL), pAlternateResult(NULL), aUId(aUIdP), bSearchButtonOnToolbox(bSearchButtonOnToolboxP) {}
    virtual sal_Bool IsWinOK( Window *pWin );
    Window* GetMaybeWin() { return pMaybeResult; }
    Window* GetAlternateResultWin() { return pAlternateResult; }
};
class SearchActive : public Search
{
    WindowType nRT;
public:
    SearchActive( WindowType nRTP ): nRT(nRTP) {}
    virtual sal_Bool IsWinOK( Window *pWin );
};
class SearchPopupFloatingWin : public Search
{
public:
    SearchPopupFloatingWin(): Search( SEARCH_FOCUS_FIRST ) {}
    virtual sal_Bool IsWinOK( Window *pWin );
};
class SearchRT : public Search
{
    WindowType mnRT;
    sal_uInt16 mnSkip;
    sal_uInt16 mnCount;
public:
    SearchRT( WindowType nRTP, SearchFlags nSearchFlags, sal_uInt16 nSkip = 0 ): Search(nSearchFlags), mnRT(nRTP), mnSkip( nSkip ), mnCount( 0 ) {}
    virtual sal_Bool IsWinOK( Window *pWin );
    sal_uInt16 GetCount(){ return mnCount; }
};
class SearchScroll : public SearchRT
{
    sal_uInt16 nDirection;
public:
    SearchScroll( sal_uInt16 nDir, SearchFlags nSearchFlags ): SearchRT(WINDOW_SCROLLBAR, nSearchFlags), nDirection(nDir) {}
    virtual sal_Bool IsWinOK( Window *pWin );
};
class SearchWinPtr : public Search
{
    Window *pTest;
public:
    SearchWinPtr( Window *pTestP ): pTest(pTestP) {}
    virtual sal_Bool IsWinOK( Window *pWin );
};
class SearchFadeSplitWin : public Search
{
    WindowAlign nAlign;
public:
    SearchFadeSplitWin( WindowAlign nAlignP ): nAlign(nAlignP) {}
    virtual sal_Bool IsWinOK( Window *pWin );
};


void ImplKeyInput( Window* pWin, KeyEvent &aKEvnt, sal_Bool bForceDirect=sal_False );
void ImplMouseMove( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect=sal_False );
void ImplMouseButtonDown( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect=sal_False );
void ImplMouseButtonUp( Window* pWin, MouseEvent &aMEvnt, sal_Bool bForceDirect=sal_False );
void ImplCommand( Window* pWin, CommandEvent &aCmdEvnt );
void ImplEventWait( sal_uLong nID );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
