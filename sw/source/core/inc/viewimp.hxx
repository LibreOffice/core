/*************************************************************************
 *
 *  $RCSfile: viewimp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX


#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif
#include "swtypes.hxx"
#include "swrect.hxx"

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

class SwViewImp
{
    friend class ViewShell;

    friend class SwLayAction;   //Lay- und IdleAction tragen sich ein und aus.
    friend class SwLayIdle;

    ViewShell *pSh;             //Falls jemand einen Imp durchreicht und doch
                                //mal eine ViewShell braucht hier die
                                //Rueckwaertsverkettung.

    SwDrawView  *pDrawView;     //Unsere DrawView
    SdrPageView *pSdrPageView;  //Genau eine Seite fuer unsere DrawView

    SwPageFrm     *pFirstVisPage;//Zeigt immer auf die erste sichtbare Seite.
    SwRegionRects *pRegion;      //Sammler fuer Paintrects aus der LayAction.
    SwScrollAreas *pScrollRects; //Sammler fuer Scrollrects aus der LayAction.
    SwScrollAreas *pScrolledArea;//Sammler der gescrollten Rechtecke.

    SwLayAction   *pLayAct;      //Ist gesetzt wenn ein Action-Objekt existiert
                                 //Wird vom SwLayAction-CTor ein- und vom DTor
                                 //ausgetragen.
    SwLayIdle     *pIdleAct;     //Analog zur SwLayAction fuer SwLayIdle.

    AutoTimer     aScrollTimer;  //Fuer das Aufraeumen nach dem Scrollen.

    BOOL bFirstPageInvalid  :1; //Pointer auf erste Seite ungueltig?
    BOOL bNextScroll        :1; //Scroll in der folgenden EndAction erlaubt?
    BOOL bScroll            :1; //Scroll in der aktuellen EndAction erlaubt?
    BOOL bScrolled          :1; //Wurde gescrolled? Dann im Idle aufraeumen.

    BOOL bResetXorVisibility:1; //StartAction/EndAction
    BOOL bShowHdlPaint      :1; //LockPaint/UnlockPaint
    BOOL bResetHdlHiddenPaint:1;//  -- "" --
    BOOL bPaintInScroll     :1; //Paint (Update() im ScrollHdl der ViewShell

    BOOL bSmoothUpdate      :1; //Meber fuer SmoothScroll
    BOOL bStopSmooth        :1;

    USHORT nRestoreActions  :1; //Die Anzahl der zu restaurierenden Actions (UNO)
    SwRect aSmoothRect;

    void SetFirstVisPage();     //Neue Ermittlung der ersten sichtbaren Seite

    void ResetNextScroll()    { bNextScroll = FALSE; }
    void SetNextScroll()      { bNextScroll = TRUE; }
    void SetScroll()          { bScroll = TRUE; }
    void ResetScrolled()      { bScrolled = FALSE; }
    void SetScrolled()        { bScrolled = TRUE; }

    SwScrollAreas *GetScrollRects() { return pScrollRects; }
    void FlushScrolledArea();
    BOOL _FlushScrolledArea( SwRect& rRect );
    BOOL FlushScrolledArea( SwRect& rRect )
    { if( !pScrolledArea ) return FALSE; return _FlushScrolledArea( rRect ); }
    void _ScrolledRect( const SwRect& rRect, long nOffs );
    void ScrolledRect( const SwRect& rRect, long nOffs )
    { if( pScrolledArea ) _ScrolledRect( rRect, nOffs ); }

    void StartAction();         //Henkel Anzeigen und verstecken.
    void EndAction();           //gerufen von ViewShell::ImplXXXAction
    void LockPaint();           //dito, gerufen von ViewShell::ImplLockPaint
    void UnlockPaint();

    void PaintFlyChilds( SwFlyFrm *pFly, ExtOutputDevice& rOut,
                         const SdrPaintInfoRec& rInfoRec );

public:
    SwViewImp( ViewShell * );
    ~SwViewImp();
    void Init( const SwViewOption * );          //nur fuer ViewShell::Init()

    const ViewShell *GetShell() const { return pSh; }
          ViewShell *GetShell()       { return pSh; }

    Color GetRetoucheColor() const;

    //Verwaltung zur ersten sichtbaren Seite
    inline const SwPageFrm *GetFirstVisPage() const;
    inline       SwPageFrm *GetFirstVisPage();
    void SetFirstVisPageInvalid() { bFirstPageInvalid = TRUE; }

    //SS'en fuer Paint- und Scrollrects.
    BOOL AddPaintRect( const SwRect &rRect );
    void AddScrollRect( const SwFrm *pFrm, const SwRect &rRect, long nOffs );
    void MoveScrollArea();
    SwRegionRects *GetRegion()      { return pRegion; }
    void DelRegions();                      //Loescht Scroll- und PaintRects

    //Handler fuer das Refresh von gescrollten Bereichen (Korrektur des
    //Alignments). Ruft das Refresh mit der ScrolledArea.
    //RefreshScrolledArea kann z.B. beim Setzen des Crsr genutzt werden, es
    //wird nur der Anteil des Rect refreshed, der mit der ScrolledArea
    //ueberlappt. Das 'reingereichte Rechteck wird veraendert!
    void RestartScrollTimer()            { aScrollTimer.Start(); }
    DECL_LINK( RefreshScrolledHdl, Timer * );
    void _RefreshScrolledArea( const SwRect &rRect );
    void RefreshScrolledArea( SwRect &rRect );

    //Wird vom Layout ggf. waehrend einer Action gerufen, wenn der
    //Verdacht besteht, dass es etwas drunter und drueber geht.
    void ResetScroll()        { bScroll = FALSE; }

    BOOL IsNextScroll() const { return bNextScroll; }
    BOOL IsScroll()     const { return bScroll; }
    BOOL IsScrolled()   const { return bScrolled; }

    BOOL IsPaintInScroll() const { return bPaintInScroll; }

    // neues Interface fuer StarView Drawing
    inline const BOOL HasDrawView() const { return 0 != pDrawView; }
          SwDrawView* GetDrawView()       { return pDrawView; }
    const SwDrawView* GetDrawView() const { return pDrawView; }
          SdrPageView*GetPageView()       { return pSdrPageView; }
    const SdrPageView*GetPageView() const { return pSdrPageView; }
    void MakeDrawView();

    void   PaintLayer  ( const BYTE nLayerID, const SwRect &rRect ) const;

    //wird als Link an die DrawEngine uebergeben, entscheidet was wie
    //gepaintet wird oder nicht.
    DECL_LINK( PaintDispatcher, SdrPaintProcRec * );

    // Interface Drawing
    BOOL IsDragPossible( const Point &rPoint );
    void NotifySizeChg( const Size &rNewSz );

    //SS Fuer die Lay- bzw. IdleAction und verwandtes
    BOOL  IsAction() const                   { return pLayAct  != 0; }
    BOOL  IsIdleAction() const               { return pIdleAct != 0; }
          SwLayAction &GetLayAction()        { return *pLayAct; }
    const SwLayAction &GetLayAction() const  { return *pLayAct; }
          SwLayIdle   &GetIdleAction()       { return *pIdleAct;}
    const SwLayIdle   &GetIdleAction() const { return *pIdleAct;}

    //Wenn eine Aktion laueft wird diese gebeten zu pruefen ob es
    //an der zeit ist den WaitCrsr einzuschalten.
    void CheckWaitCrsr();
    BOOL IsCalcLayoutProgress() const;  //Fragt die LayAction wenn vorhanden.
    //TRUE wenn eine LayAction laeuft, dort wird dann auch das Flag fuer
    //ExpressionFields gesetzt.
    BOOL IsUpdateExpFlds();

    void    SetRestoreActions(USHORT nSet){nRestoreActions = nSet;}
    USHORT  GetRestoreActions() const{return nRestoreActions;}
};

//Kann auf dem Stack angelegt werden, wenn etwas ausgegeben oder
//gescrolled wird. Handles und sontiges vom Drawing werden im CTor
//gehidet und im DTor wieder sichtbar gemacht.
//AW 06-Sep99: Hiding of handles is no longer necessary, removed
class SwSaveHdl
{
    SwViewImp *pImp;
    BOOL       bXorVis;
public:
    SwSaveHdl( SwViewImp *pImp );
    ~SwSaveHdl();
};


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



#endif //_VIEWIMP_HXX

