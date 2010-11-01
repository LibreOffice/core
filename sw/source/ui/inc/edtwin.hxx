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
#ifndef _EDTWIN_HXX
#define _EDTWIN_HXX

#include <svx/svdobj.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <swevent.hxx>

#define _SVSTDARR_STRINGSISORTDTOR
#include <svl/svstdarr.hxx>

class   SwWrtShell;
class   SwView;
class   SwRect;
class   ViewShell;
class   SwAnchorMarker;
class   SdrObject;
class   SwShadowCursor;
class   DataChangedEvent;
class   SvxAutoCorrCfg;
class   SvxAutoCorrect;
class   SwPaM;
struct  SwApplyTemplate;
struct  QuickHelpData;
class SdrDropMarkerOverlay;

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
    LanguageType    eBufferLanguage;
    Point           aStartPos;
    Point           aMovePos;
    Point           aRszMvHdlPt;
    Timer           aTemplateTimer;

    // Type/Objecte ueber dem der MousePointer steht
    SwCallMouseEvent aSaveCallEvent;

    SwApplyTemplate     *pApplyTempl;
    SwAnchorMarker      *pAnchorMarker; // zum Verschieben eines Ankers

    SdrDropMarkerOverlay    *pUserMarker;
    SdrObject               *pUserMarkerObj;
    SwShadowCursor          *pShadCrsr;
    Point                   *pRowColumnSelectionStart; // save position where table row/column selection has been started

    SwView         &rView;

    int             aActHitType;    // aktueller Mauspointer

    ULONG           m_nDropFormat;   //Format aus dem letzten QueryDrop
    USHORT          m_nDropAction;   //Action aus dem letzten QueryDrop
    USHORT          m_nDropDestination;  //Ziel aus dem letzten QueryDrop

    UINT16          eBezierMode;
    UINT16          nInsFrmColCount; //Spaltenzahl fuer interaktiven Rahmen
    SdrObjKind      eDrawMode;
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
                    bLockInput      : 1, //Lock waehrend die Rechenleiste aktiv ist
                    bIsRowDrag      : 1, //selection of rows is used, in combination with pRowColumnSelectionStart
                    /** #i42732# display status of font size/name depending on either the input language or the
                        selection position depending on what has changed lately
                     */
                    bUseInputLanguage: 1,
                    bObjectSelect   : 1;


    USHORT          nKS_NUMDOWN_Count; // #i23725#
    USHORT          nKS_NUMINDENTINC_Count;

    void            LeaveArea(const Point &);
    void            JustifyAreaTimer();
    inline void     EnterArea();

    void            RstMBDownFlags();

    void            ChangeFly( BYTE nDir, BOOL bWeb = FALSE );
    void            ChangeDrawing( BYTE nDir );

    BOOL            EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos);
    BOOL            RulerColumnDrag( const MouseEvent& rMEvt, BOOL bVerticalMode);

    //Hilfsfunktionen fuer D&D
    void            DropCleanup();
    void            CleanupDropUserMarker();
    USHORT          GetDropDestination( const Point& rPixPnt,
                                        SdrObject ** ppObj = 0 );
    //select the object/cursor at the mouse position of the context menu request
    BOOL            SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos );

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

    using OutputDevice::GetTextColor;

protected:

    virtual void    DataChanged( const DataChangedEvent& );
    virtual void    PrePaint();
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

    virtual XubString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    void    ShowAutoTextCorrectQuickHelp( const String& rWord, SvxAutoCorrCfg* pACfg,
                                SvxAutoCorrect* pACorr, sal_Bool bFromIME = sal_False );
public:

    void            UpdatePointer(const Point &, USHORT nButtons = 0);

    BOOL            IsDrawSelMode();
    BOOL            IsDrawAction()                  { return (bInsDraw); }
    void            SetDrawAction(BOOL bFlag)       { bInsDraw = bFlag; }

    void            SetObjectSelect( BOOL bVal )    { bObjectSelect = bVal; }
    BOOL            IsObjectSelect() const          { return bObjectSelect; }

    inline SdrObjKind   GetSdrDrawMode(/*BOOL bBuf = FALSE*/) const { return eDrawMode; }
    inline void         SetSdrDrawMode( SdrObjKind eSdrObjectKind ) { eDrawMode = eSdrObjectKind; SetObjectSelect( FALSE ); }
    void                StdDrawMode( SdrObjKind eSdrObjectKind, BOOL bObjSelect );

    BOOL            IsFrmAction()                   { return (bInsFrm); }
    inline UINT16   GetBezierMode()                 { return eBezierMode; }
    void            SetBezierMode(UINT16 eBezMode)  { eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const Point& aDocPos); // DrawTextEditMode einschalten
    void            InsFrm(USHORT nCols);
    void            StopInsFrm();
    UINT16          GetFrmColCount() const {return nInsFrmColCount;} //Spaltenzahl fuer interaktiven Rahmen


    void            SetChainMode( BOOL bOn );
    BOOL            IsChainMode() const             { return bChainMode; }

    void            FlushInBuffer();
    BOOL            IsInputSequenceCheckingRequired( const String &rText, const SwPaM& rCrsr ) const;


    static  void    SetReplaceQuote(BOOL bOn = TRUE) { bReplaceQuote = bOn; }
    static  BOOL    IsReplaceQuote() { return bReplaceQuote; }

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return pApplyTempl; }

    void            StartExecuteDrag();
    void            DragFinished();
    USHORT          GetDropAction() const { return m_nDropAction; }
    ULONG           GetDropFormat() const { return m_nDropFormat; }

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

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    // Tipfenster loeschen
    static void ClearTip();

    static inline long GetDDStartPosX() { return nDDStartPosX; }
    static inline long GetDDStartPosY() { return nDDStartPosY; }

    static void _InitStaticData();
    static void _FinitStaticData();

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    void StopQuickHelp();

    // --> OD 2005-02-18 #i42921# - add parameter <bVerticalMode>
    BOOL RulerMarginDrag( const MouseEvent& rMEvt,
                                     const bool bVerticalMode );
    // <--

    /** #i42732# display status of font size/name depending on either the input
        language or the selection position depending on what has changed lately
     */
    void        SetUseInputLanguage( sal_Bool bNew );
    sal_Bool    IsUseInputLanguage() const { return bUseInputLanguage; }

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
