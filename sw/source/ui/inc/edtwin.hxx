/*************************************************************************
 *
 *  $RCSfile: edtwin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-23 15:39:59 $
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
#ifndef _EDTWIN_HXX
#define _EDTWIN_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif

#define _SVSTDARR_STRINGSISORTDTOR
#include <svtools/svstdarr.hxx>

class   SwWrtShell;
class   SwView;
class   SwRect;
class   SwDrawBase;
class   ViewShell;
class   SwAnchorMarker;
class   SdrViewUserMarker;
class   SdrObject;
class   SwShadowCursor;
class   DataChangedEvent;

struct  SwApplyTemplate;
struct  QuickHelpData;

/*--------------------------------------------------------------------
    Beschreibung:   Eingabe-Fenster
 --------------------------------------------------------------------*/

class SwEditWin: public Window,
                public DropTargetHelper, public DragSourceHelper
{
friend void     ScrollMDI(ViewShell* pVwSh, const SwRect&,
                          USHORT nRangeX, USHORT nRangeY);
friend BOOL     IsScrollMDI(ViewShell* pVwSh, const SwRect&);

friend void     SizeNotify(ViewShell* pVwSh, const Size &);

friend void     PageNumNotify(  ViewShell* pVwSh,
                                USHORT nPhyNum,
                                USHORT nVirtNum,
                                const String& rPg );

    static  QuickHelpData* pQuickHlpData;

    static  BOOL    bReplaceQuote;
    static  long    nDDStartPosX, nDDStartPosY;

    static  Color   aTextColor;     //Textfarbe, fuer die Giesskanne
    static  BOOL    bTransparentBackColor; // Hintergrund transparent
    static  Color   aTextBackColor; //Texthintergrundfarbe, fuer die Giesskanne

    /*
     * Timer und Handler fuer das Weiterscrollen, wenn der
     * Mauspointer innerhalb eines Drag-Vorgangs ausserhalb des
     * EditWin stehen bleibt.  In regelmaessigen Intervallen wird
     * die Selektion in Richtung der Mausposition vergroessert.
     */
    AutoTimer       aTimer;
    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    Timer           aKeyInputTimer;
    // timer for ANY-KeyInut question without a following KeyInputEvent
    Timer           aKeyInputFlushTimer;

    String          aInBuffer;
    Point           aStartPos;
    Point           aMovePos;
    Point           aRszMvHdlPt;
    Timer           aTemplateTimer;

    // Type/Objecte ueber dem der MousePointer steht
    SwCallMouseEvent aSaveCallEvent;

    SwApplyTemplate     *pApplyTempl;
    SwAnchorMarker      *pAnchorMarker; // zum Verschieben eines Ankers

    SdrViewUserMarker   *pUserMarker;
    SdrObject           *pUserMarkerObj;
    SwShadowCursor      *pShadCrsr;

    SwView         &rView;

    int             aActHitType;    // aktueller Mauspointer

    ULONG           nDropFormat;    //Format aus dem letzten QueryDrop
    USHORT          nDropAction;    //Action aus dem letzten QueryDrop
    USHORT          nDropDestination;   //Ziel aus dem letzten QueryDrop

    UINT16          eDrawMode;
    UINT16          eBezierMode;
    UINT16          nInsFrmColCount; //Spaltenzahl fuer interaktiven Rahmen
    BOOL            bLinkRemoved    : 1,
                    bMBPressed      : 1,
                    bInsDraw        : 1,
                    bInsFrm         : 1,
                    bIsInMove       : 1,
                    bIsInDrag       : 1, //StartExecuteDrag nich doppelt ausfuehren
                    bOldIdle        : 1, //Zum abschalten des Idle'ns
                    bOldIdleSet     : 1, //waehrend QeueryDrop
                    bTblInsDelMode  : 1, //
                    bTblIsInsMode   : 1, //
                    bTblIsColMode   : 1, //
                    bChainMode      : 1, //Rahmen verbinden
                    bWasShdwCrsr    : 1, //ShadowCrsr war im MouseButtonDown an
                    bLockInput      : 1; //Lock waehrend die Rechenleiste aktiv ist


    void            LeaveArea(const Point &);
    void            JustifyAreaTimer();
    inline void     EnterArea();

    void            RstMBDownFlags();

    void            ChangeFly( BYTE nDir, BOOL bWeb = FALSE );

    BOOL            EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos);
    BOOL            RulerClook( SwView& , const MouseEvent& rMEvt);

    //Hilfsfunktionen fuer D&D
    void            DropCleanup();
    void            CleanupDropUserMarker();


    /*
     * Handler fuer das Weiterscrollen, wenn der Mauspointer innerhalb eines
     * Drag-Vorgangs ausserhalb des EditWin stehen bleibt. In regelmaessigen
     * Intervallen wird die Selektion in Richtung der Mausposition
     * vergroessert.
     */
    DECL_LINK( TimerHandler, Timer * );
    void            StartDDTimer();
    void            StopDDTimer(SwWrtShell *, const Point &);
    DECL_LINK( DDHandler, Timer * );

    // timer for ANY-KeyInut question without a following KeyInputEvent
    DECL_LINK( KeyInputFlushHandler, Timer * );

    // Timer fuer verschachtelte KeyInputs (z.B. fuer Tabellen)
    DECL_LINK( KeyInputTimerHandler, Timer * );

    // Timer fuer das ApplyTemplates per Maus (verkapptes Drag&Drop)
    DECL_LINK( TemplateTimerHdl, Timer* );

protected:

    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    KeyInput(const KeyEvent &rKEvt);

    virtual void    GetFocus();
    virtual void    LoseFocus();


    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    RequestHelp(const HelpEvent& rEvt);

    virtual void    Command( const CommandEvent& rCEvt );

                                // Drag & Drop Interface
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:

    void            UpdatePointer(const Point &, USHORT nButtons = 0);

    BOOL            IsDrawSelMode();
    BOOL            IsDrawAction()                  { return (bInsDraw); }
    void            SetDrawAction(BOOL bFlag)       { bInsDraw = bFlag; }
    inline UINT16   GetDrawMode(BOOL bBuf = FALSE) const { return eDrawMode; }
    inline void     SetDrawMode(UINT16 eDrwMode)    { eDrawMode = eDrwMode; }
    void            StdDrawMode(USHORT nSlotId);
    BOOL            IsFrmAction()                   { return (bInsFrm); }
    inline UINT16   GetBezierMode()                 { return eBezierMode; }
    void            SetBezierMode(UINT16 eBezMode)  { eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const Point& aDocPos); // DrawTextEditMode einschalten
    void            InsFrm(USHORT nCols);
    void            StopInsFrm();
    UINT16          GetFrmColCount() const {return nInsFrmColCount;} //Spaltenzahl fuer interaktiven Rahmen


    void            SetChainMode( BOOL bOn );
    BOOL            IsChainMode() const             { return bChainMode; }

    void            FlushInBuffer( SwWrtShell *pSh );

    static  void    SetReplaceQuote(BOOL bOn = TRUE) { bReplaceQuote = bOn; }
    static  BOOL    IsReplaceQuote() { return bReplaceQuote; }

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return pApplyTempl; }

    void            StartExecuteDrag();
    void            DragFinished();
    USHORT          GetDropAction() const { return nDropAction; }
    ULONG           GetDropFormat() const { return nDropFormat; }

    Color           GetTextColor() { return aTextColor; }
    void            SetTextColor(const Color& rCol ) { aTextColor = rCol; }

    Color           GetTextBackColor()
                                            { return aTextBackColor; }
    void            SetTextBackColor(const Color& rCol )
                                            { aTextBackColor = rCol; }
    void            SetTextBackColorTransparent(BOOL bSet)
                                    { bTransparentBackColor = bSet; }
    BOOL            IsTextBackColorTransparent()
                                    { return bTransparentBackColor; }
    void            LockKeyInput(BOOL bSet){bLockInput = bSet;}

    const SwView &GetView() const { return rView; }
          SwView &GetView()       { return rView; }

    // Tipfenster loeschen
    static void ClearTip();

    static inline long GetDDStartPosX() { return nDDStartPosX; }
    static inline long GetDDStartPosY() { return nDDStartPosY; }

    static void _InitStaticData();
    static void _FinitStaticData();

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
};


#endif

