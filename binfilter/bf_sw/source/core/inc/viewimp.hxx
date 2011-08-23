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
#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX


#include <vcl/timer.hxx>
#include <tools/color.hxx>
// OD 25.06.2003 #108784#
#include <bf_svx/svdtypes.hxx>

#include "swtypes.hxx"
#include "swrect.hxx"
#include <tools/debug.hxx> //for strip
#ifdef ACCESSIBLE_LAYOUT
class Fraction;
#endif
namespace binfilter {
class ViewShell;
class SwFlyFrm;
class SwViewOption;
class SwRegionRects;
class SwScrollAreas;
class SwScrollColumn;
class SwFrm;
class SwLayAction;
class SwLayIdle;
class SwDrawView;
class SdrPageView;
class SwPageFrm;
class SwRegionRects;
class ExtOutputDevice;
class SdrPaintInfoRec;
struct SdrPaintProcRec;
#ifdef ACCESSIBLE_LAYOUT
class SwAccessibleMap;
class SdrObject;
//STRIP008 class Fraction;
#endif
// OD 12.12.2002 #103492#
class SwPagePreviewLayout;
// OD 15.01.2003 #103492#
} //namespace binfilter

#include <prevwpage.hxx>
// OD 15.01.2003 #103492#
#include <vector>
namespace binfilter {//STRIP009
class SwViewImp
{
    friend class ViewShell;

    friend class SwLayAction;	//Lay- und IdleAction tragen sich ein und aus.
    friend class SwLayIdle;

    // OD 12.12.2002 #103492# - for paint of page preview
    friend class SwPagePreviewLayout;

    ViewShell *pSh;				//Falls jemand einen Imp durchreicht und doch
                                //mal eine ViewShell braucht hier die
                                //Rueckwaertsverkettung.

    SwDrawView  *pDrawView;		//Unsere DrawView
    SdrPageView *pSdrPageView;	//Genau eine Seite fuer unsere DrawView

    SwPageFrm	  *pFirstVisPage;//Zeigt immer auf die erste sichtbare Seite.
    SwRegionRects *pRegion;      //Sammler fuer Paintrects aus der LayAction.
    SwScrollAreas *pScrollRects; //Sammler fuer Scrollrects aus der LayAction.
    SwScrollAreas *pScrolledArea;//Sammler der gescrollten Rechtecke.

    SwLayAction	  *pLayAct;		 //Ist gesetzt wenn ein Action-Objekt existiert
                                 //Wird vom SwLayAction-CTor ein- und vom DTor
                                 //ausgetragen.
    SwLayIdle	  *pIdleAct;	 //Analog zur SwLayAction fuer SwLayIdle.

#ifdef ACCESSIBLE_LAYOUT
#endif


    AutoTimer	  aScrollTimer;	 //Fuer das Aufraeumen nach dem Scrollen.

    BOOL bFirstPageInvalid	:1;	//Pointer auf erste Seite ungueltig?
    BOOL bNextScroll		:1;	//Scroll in der folgenden EndAction erlaubt?
    BOOL bScroll			:1; //Scroll in der aktuellen EndAction erlaubt?
    BOOL bScrolled			:1; //Wurde gescrolled? Dann im Idle aufraeumen.

    BOOL bResetXorVisibility:1; //StartAction/EndAction
    BOOL bShowHdlPaint		:1; //LockPaint/UnlockPaint
    BOOL bResetHdlHiddenPaint:1;//  -- "" --
    BOOL bPaintInScroll		:1;	//Paint (Update() im ScrollHdl der ViewShell

    BOOL bSmoothUpdate		:1;	//Meber fuer SmoothScroll
    BOOL bStopSmooth		:1;
    BOOL bStopPrt           :1; // Stop Printing

    USHORT nRestoreActions  ; //Die Anzahl der zu restaurierenden Actions (UNO)
    SwRect aSmoothRect;

    // OD 12.12.2002 #103492#

    /**
        Signal whether to stop printing.

        @param _useless just to fit macro
    */
    DECL_LINK(SetStopPrt, void * _useless = NULL);

    /**
       Returns if printer shall be stopped.

       @retval TRUE The printer shall be stopped.
       @retval FALSE else
    */
    BOOL IsStopPrt() { return bStopPrt; }

    /**
       Resets signal for stopping printing.

    */
    void ResetStopPrt() { bStopPrt = FALSE; }

    void SetFirstVisPage();		//Neue Ermittlung der ersten sichtbaren Seite

    void ResetNextScroll()	  { bNextScroll = FALSE; }
    void SetNextScroll()	  { bNextScroll = TRUE; }
    void SetScroll()		  { bScroll = TRUE; }
    void ResetScrolled()	  { bScrolled = FALSE; }
    void SetScrolled()		  { bScrolled = TRUE; }

    SwScrollAreas *GetScrollRects() { return pScrollRects; }

    void StartAction();			//Henkel Anzeigen und verstecken.
    void EndAction();			//gerufen von ViewShell::ImplXXXAction

#ifdef ACCESSIBLE_LAYOUT
#endif

public:
    ~SwViewImp();
    void Init( const SwViewOption * );			//nur fuer ViewShell::Init()

    const ViewShell *GetShell() const { return pSh; }
          ViewShell *GetShell()		  { return pSh; }


    //Verwaltung zur ersten sichtbaren Seite
    inline const SwPageFrm *GetFirstVisPage() const;
    inline		 SwPageFrm *GetFirstVisPage();
    void SetFirstVisPageInvalid() { bFirstPageInvalid = TRUE; }

    //SS'en fuer Paint- und Scrollrects.
    BOOL AddPaintRect( const SwRect &rRect );
    void AddScrollRect( const SwFrm *pFrm, const SwRect &rRect, long nOffs );
    SwRegionRects *GetRegion()		{ return pRegion; }
    void DelRegions();	 					//Loescht Scroll- und PaintRects

    //Handler fuer das Refresh von gescrollten Bereichen (Korrektur des
    //Alignments). Ruft das Refresh mit der ScrolledArea.
    //RefreshScrolledArea kann z.B. beim Setzen des Crsr genutzt werden, es
    //wird nur der Anteil des Rect refreshed, der mit der ScrolledArea
    //ueberlappt. Das 'reingereichte Rechteck wird veraendert!
    void RestartScrollTimer()			 { aScrollTimer.Start(); }
    DECL_LINK( RefreshScrolledHdl, Timer * );

    //Wird vom Layout ggf. waehrend einer Action gerufen, wenn der
    //Verdacht besteht, dass es etwas drunter und drueber geht.
    void ResetScroll()		  { bScroll = FALSE; }

    BOOL IsNextScroll() const { return bNextScroll; }
    BOOL IsScroll()		const { return bScroll; }
    BOOL IsScrolled()	const { return bScrolled; }

    BOOL IsPaintInScroll() const { return bPaintInScroll; }

    // neues Interface fuer StarView Drawing
    inline BOOL HasDrawView() const { return 0 != pDrawView; }
          SwDrawView* GetDrawView() 	  { return pDrawView; }
    const SwDrawView* GetDrawView() const { return pDrawView; }
          SdrPageView*GetPageView() 	  { return pSdrPageView; }
    const SdrPageView*GetPageView() const { return pSdrPageView; }
    void MakeDrawView();

    // OD 29.08.2002 #102450#
    // add 3rd parameter <const Color* pPageBackgrdColor> for setting this
    // color as the background color at the outliner of the draw view
    // for painting layers <hell> and <heaven>
    // OD 09.12.2002 #103045# - add 4th parameter for the horizontal text
    // direction of the page in order to set the default horizontal text
    // direction at the outliner of the draw view for painting layers <hell>
    // and <heaven>.
    // OD 25.06.2003 #108784# - correct type of 1st parameter

    //wird als Link an die DrawEngine uebergeben, entscheidet was wie
    //gepaintet wird oder nicht.

    // Interface Drawing
    void NotifySizeChg( const Size &rNewSz );

    //SS Fuer die Lay- bzw. IdleAction und verwandtes
    BOOL  IsAction() const 					 { return pLayAct  != 0; }
    BOOL  IsIdleAction() const				 { return pIdleAct != 0; }
          SwLayAction &GetLayAction()		 { return *pLayAct; }
    const SwLayAction &GetLayAction() const  { return *pLayAct; }
          SwLayIdle	  &GetIdleAction()		 { return *pIdleAct;}
    const SwLayIdle   &GetIdleAction() const { return *pIdleAct;}

    //Wenn eine Aktion laueft wird diese gebeten zu pruefen ob es
    //an der zeit ist den WaitCrsr einzuschalten.
    void CheckWaitCrsr();
    BOOL IsCalcLayoutProgress() const;	//Fragt die LayAction wenn vorhanden.
    //TRUE wenn eine LayAction laeuft, dort wird dann auch das Flag fuer
    //ExpressionFields gesetzt.
    BOOL IsUpdateExpFlds();

    void	SetRestoreActions(USHORT nSet){nRestoreActions = nSet;}
    USHORT 	GetRestoreActions() const{return nRestoreActions;}

    // OD 12.12.2002 #103492#

    // OD 12.12.2002 #103492#

#ifdef ACCESSIBLE_LAYOUT
    // Is this view accessible?


    // Update (this) accessible view

    // Remove a frame from the accessible view
    void DisposeAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                            sal_Bool bRecursive );
    inline void DisposeAccessibleObj( const SdrObject *pObj );

    // Move a frame's position in the accessible view
    void MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                         const SwRect& rOldFrm );

    // Add a frame in the accessible view

     inline void AddAccessibleObj( const SdrObject *pObj );

    // Invalidate accessible frame's frame's content
    void InvalidateAccessibleFrmContent( const SwFrm *pFrm );

    // Invalidate accessible frame's cursor position

    // Invalidate editable state for all accessible frames

    // Invalidate opaque state for all accessible frames

    // Invalidate frame's relation set (for chained frames)

    // update data for accessible preview
    // OD 15.01.2003 #103492# - change method signature due to new page preview
    // functionality


    // Fire all accessible events that have been collected so far
#endif
};

//Kann auf dem Stack angelegt werden, wenn etwas ausgegeben oder
//gescrolled wird. Handles und sontiges vom Drawing werden im CTor
//gehidet und im DTor wieder sichtbar gemacht.
//AW 06-Sep99: Hiding of handles is no longer necessary, removed


inline SwPageFrm *SwViewImp::GetFirstVisPage()
{
    if ( bFirstPageInvalid )
        SetFirstVisPage();
    return pFirstVisPage;
}

inline const SwPageFrm *SwViewImp::GetFirstVisPage() const
{
    if ( bFirstPageInvalid )
        ((SwViewImp*)this)->SetFirstVisPage();
    return pFirstVisPage;
}

#ifdef ACCESSIBLE_LAYOUT


inline void SwViewImp::DisposeAccessibleObj( const SdrObject *pObj )
{
    DisposeAccessible( 0, pObj, sal_False );
}



inline void SwViewImp::AddAccessibleObj( const SdrObject *pObj )
{
    SwRect aEmptyRect;
    MoveAccessible( 0, pObj, aEmptyRect );
}

#endif

} //namespace binfilter
#endif //_VIEWIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
