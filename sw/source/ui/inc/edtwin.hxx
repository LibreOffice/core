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

#include <FrameControlsManager.hxx>

#include <svx/svdobj.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <swevent.hxx>


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
class   SdrDropMarkerOverlay;

/*--------------------------------------------------------------------
    Description:    input window
 --------------------------------------------------------------------*/

/** Window class for the Writer edit area, this is the one handling mouse
    and keyboard events and doing the final painting of the document from
    the buffered layout.

    To translate the pixel positions from the buffer OutputDevice to the real
    pixel positions, use the PixelToLogic methods of this class.
  */
class SwEditWin: public Window,
                public DropTargetHelper, public DragSourceHelper
{
friend void     ScrollMDI(ViewShell* pVwSh, const SwRect&,
                          sal_uInt16 nRangeX, sal_uInt16 nRangeY);
friend sal_Bool     IsScrollMDI(ViewShell* pVwSh, const SwRect&);

friend void     SizeNotify(ViewShell* pVwSh, const Size &);

friend void     PageNumNotify(  ViewShell* pVwSh,
                                sal_uInt16 nPhyNum,
                                sal_uInt16 nVirtNum,
                                const String& rPg );

    static  QuickHelpData* pQuickHlpData;

    static  sal_Bool    bReplaceQuote;
    static  long    nDDStartPosX, nDDStartPosY;

    static  Color   aTextColor;     // text color; for the watering can
    static  sal_Bool    bTransparentBackColor; // transparent background
    static  Color   aTextBackColor; // text background; for the watering can

    /*
     * timer and handler for scrolling on when the mousepointer
     * stopps outside of EditWin during a drag-operation.
     * The selection is increased towards the mouse position
     * regularly.
     */
    AutoTimer       aTimer;
    // timer for overlapping KeyInputs (e.g. for tables)
    Timer           aKeyInputTimer;
    // timer for ANY-KeyInut question without a following KeyInputEvent
    Timer           aKeyInputFlushTimer;

    String          aInBuffer;
    LanguageType    eBufferLanguage;
    Point           aStartPos;
    Point           aMovePos;
    Point           aRszMvHdlPt;
    Timer           aTemplateTimer;

    // type/object where the mouse pointer is
    SwCallMouseEvent aSaveCallEvent;

    SwApplyTemplate     *pApplyTempl;
    SwAnchorMarker      *pAnchorMarker; // for relocating the anchor

    SdrDropMarkerOverlay    *pUserMarker;
    SdrObject               *pUserMarkerObj;
    SwShadowCursor          *pShadCrsr;
    Point                   *pRowColumnSelectionStart; // save position where table row/column selection has been started

    SwView         &rView;

    int             aActHitType;    // current mouse pointer

    sal_uLong           m_nDropFormat;   // format from the last QueryDrop
    sal_uInt16          m_nDropAction;   // action from the last QueryDrop
    sal_uInt16          m_nDropDestination;  // destination from the last QueryDrop

    sal_uInt16          eBezierMode;
    sal_uInt16          nInsFrmColCount; // column number for interactive frame
    SdrObjKind      eDrawMode;
    sal_Bool            bLinkRemoved    : 1,
                    bMBPressed      : 1,
                    bInsDraw        : 1,
                    bInsFrm         : 1,
                    bIsInMove       : 1,
                    bIsInDrag       : 1, // don't execute StartExecuteDrag twice
                    bOldIdle        : 1, // to stop to idle
                    bOldIdleSet     : 1, // during QeueryDrop
                    bTblInsDelMode  : 1, //
                    bTblIsInsMode   : 1, //
                    bTblIsColMode   : 1, //
                    bChainMode      : 1, // connect frames
                    bWasShdwCrsr    : 1, // ShadowCrsr was on in MouseButtonDown
                    bLockInput      : 1, // lock while calc panel is active
                    bIsRowDrag      : 1, //selection of rows is used, in combination with pRowColumnSelectionStart
                    /** #i42732# display status of font size/name depending on either the input language or the
                        selection position depending on what has changed lately
                     */
                    bUseInputLanguage: 1,
                    bObjectSelect   : 1;

    sal_uInt16          nKS_NUMDOWN_Count; // #i23725#
    sal_uInt16          nKS_NUMINDENTINC_Count;

    SwFrameControlsManager m_aFrameControlsManager;

    void            LeaveArea(const Point &);
    void            JustifyAreaTimer();
    inline void     EnterArea();

    void            RstMBDownFlags();

    void            ChangeFly( sal_uInt8 nDir, sal_Bool bWeb = sal_False );
    void            ChangeDrawing( sal_uInt8 nDir );

    sal_Bool            EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos);
    sal_Bool            RulerColumnDrag( const MouseEvent& rMEvt, sal_Bool bVerticalMode);

    // helper function for D&D
    void            DropCleanup();
    void            CleanupDropUserMarker();
    sal_uInt16          GetDropDestination( const Point& rPixPnt,
                                        SdrObject ** ppObj = 0 );
    //select the object/cursor at the mouse position of the context menu request
    sal_Bool            SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos );

    /*
     * handler for scrolling on when the mousepointer
     * stopps outside of EditWin during a drag-operation.
     * The selection is regularly increased towards the mouse
     * position.
     */
    DECL_LINK( TimerHandler, void * );
    void            StartDDTimer();
    void            StopDDTimer(SwWrtShell *, const Point &);
    DECL_LINK( DDHandler, void * );

    // timer for ANY-KeyInut question without a following KeyInputEvent
    DECL_LINK( KeyInputFlushHandler, void * );

    // timer for overlapping KeyInputs (e.g. for tables)
    DECL_LINK( KeyInputTimerHandler, void * );

    // timer for ApplyTemplates via mouse (in disguise Drag&Drop)
    DECL_LINK( TemplateTimerHdl, void * );

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

    virtual rtl::OUString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    void    ShowAutoTextCorrectQuickHelp( const String& rWord, SvxAutoCorrCfg* pACfg,
                                SvxAutoCorrect* pACorr, sal_Bool bFromIME = sal_False );

    /// Shows or hides the header & footer separators; only if there is a change.
    void    ShowHeaderFooterSeparator( bool bShowHeader, bool bShowFooter );

    /// Returns true if in header/footer area, or in the header/footer control.
    bool    IsInHeaderFooter( const Point &rDocPt, FrameControlType &rControl ) const;
public:

    void            UpdatePointer(const Point &, sal_uInt16 nButtons = 0);

    sal_Bool            IsDrawSelMode();
    sal_Bool            IsDrawAction()                  { return (bInsDraw); }
    void            SetDrawAction(sal_Bool bFlag)       { bInsDraw = bFlag; }

    void            SetObjectSelect( sal_Bool bVal )    { bObjectSelect = bVal; }
    sal_Bool            IsObjectSelect() const          { return bObjectSelect; }

    inline SdrObjKind   GetSdrDrawMode(/*sal_Bool bBuf = sal_False*/) const { return eDrawMode; }
    inline void         SetSdrDrawMode( SdrObjKind eSdrObjectKind ) { eDrawMode = eSdrObjectKind; SetObjectSelect( sal_False ); }
    void                StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect );

    sal_Bool            IsFrmAction()                   { return (bInsFrm); }
    inline sal_uInt16   GetBezierMode()                 { return eBezierMode; }
    void            SetBezierMode(sal_uInt16 eBezMode)  { eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const Point& aDocPos); // turn on DrawTextEditMode
    void            InsFrm(sal_uInt16 nCols);
    void            StopInsFrm();
    sal_uInt16          GetFrmColCount() const {return nInsFrmColCount;} // column number for interactive frame


    void            SetChainMode( sal_Bool bOn );
    sal_Bool            IsChainMode() const             { return bChainMode; }

    void            FlushInBuffer();
    sal_Bool            IsInputSequenceCheckingRequired( const String &rText, const SwPaM& rCrsr ) const;


    static  void    SetReplaceQuote(sal_Bool bOn = sal_True) { bReplaceQuote = bOn; }
    static  sal_Bool    IsReplaceQuote() { return bReplaceQuote; }

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return pApplyTempl; }

    void            StartExecuteDrag();
    void            DragFinished();
    sal_uInt16          GetDropAction() const { return m_nDropAction; }
    sal_uLong           GetDropFormat() const { return m_nDropFormat; }

    Color           GetTextColor() { return aTextColor; }

    void            SetTextColor(const Color& rCol ) { aTextColor = rCol; }

    Color           GetTextBackColor()
                                            { return aTextBackColor; }
    void            SetTextBackColor(const Color& rCol )
                                            { aTextBackColor = rCol; }
    void            SetTextBackColorTransparent(sal_Bool bSet)
                                    { bTransparentBackColor = bSet; }
    sal_Bool            IsTextBackColorTransparent()
                                    { return bTransparentBackColor; }
    void            LockKeyInput(sal_Bool bSet){bLockInput = bSet;}

    const SwView &GetView() const { return rView; }
          SwView &GetView()       { return rView; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    static inline long GetDDStartPosX() { return nDDStartPosX; }
    static inline long GetDDStartPosY() { return nDDStartPosY; }

    static void _InitStaticData();
    static void _FinitStaticData();

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    void StopQuickHelp();

    // #i42921# - add parameter <bVerticalMode>
    sal_Bool RulerMarginDrag( const MouseEvent& rMEvt,
                                     const bool bVerticalMode );

    /** #i42732# display status of font size/name depending on either the input
        language or the selection position depending on what has changed lately
     */
    void        SetUseInputLanguage( sal_Bool bNew );
    sal_Bool    IsUseInputLanguage() const { return bUseInputLanguage; }

    SwFrameControlsManager& GetFrameControlsManager() { return m_aFrameControlsManager; }

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
