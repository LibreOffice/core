/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_INC_EDTWIN_HXX
#define INCLUDED_SW_SOURCE_UI_INC_EDTWIN_HXX

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
class   SwViewShell;
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
friend void     ScrollMDI(SwViewShell* pVwSh, const SwRect&,
                          sal_uInt16 nRangeX, sal_uInt16 nRangeY);
friend sal_Bool     IsScrollMDI(SwViewShell* pVwSh, const SwRect&);

friend void     SizeNotify(SwViewShell* pVwSh, const Size &);

friend void     PageNumNotify(  SwViewShell* pVwSh,
                                sal_uInt16 nPhyNum,
                                sal_uInt16 nVirtNum,
                                const OUString& rPg );

    static  QuickHelpData* m_pQuickHlpData;

    static  long    m_nDDStartPosX, m_nDDStartPosY;

    static  Color   m_aTextColor;     // text color; for the watering can
    static  bool    m_bTransparentBackColor; // transparent background
    static  Color   m_aTextBackColor; // text background; for the watering can

    /*
     * timer and handler for scrolling on when the mousepointer
     * stopps outside of EditWin during a drag-operation.
     * The selection is increased towards the mouse position
     * regularly.
     */
    AutoTimer       m_aTimer;
    // timer for overlapping KeyInputs (e.g. for tables)
    Timer           m_aKeyInputTimer;
    // timer for ANY-KeyInut question without a following KeyInputEvent
    Timer           m_aKeyInputFlushTimer;

    OUString        m_aInBuffer;
    LanguageType    m_eBufferLanguage;
    Point           m_aStartPos;
    Point           m_aMovePos;
    Point           m_aRszMvHdlPt;
    Timer           m_aTemplateTimer;

    // type/object where the mouse pointer is
    SwCallMouseEvent m_aSaveCallEvent;

    SwApplyTemplate     *m_pApplyTempl;
    SwAnchorMarker      *m_pAnchorMarker; // for relocating the anchor

    SdrDropMarkerOverlay    *m_pUserMarker;
    SdrObject               *m_pUserMarkerObj;
    SwShadowCursor          *m_pShadCrsr;
    Point                   *m_pRowColumnSelectionStart; // save position where table row/column selection has been started

    SwView         &m_rView;

    int             m_aActHitType;    // current mouse pointer

    sal_uLong       m_nDropFormat;   // format from the last QueryDrop
    sal_uInt16      m_nDropAction;   // action from the last QueryDrop
    sal_uInt16      m_nDropDestination;  // destination from the last QueryDrop

    sal_uInt16      m_eBezierMode;
    sal_uInt16      m_nInsFrmColCount; // column number for interactive frame
    SdrObjKind      m_eDrawMode;
    sal_Bool        m_bMBPressed      : 1,
                    m_bInsDraw        : 1,
                    m_bInsFrm         : 1,
                    m_bIsInMove       : 1,
                    m_bIsInDrag       : 1, // don't execute StartExecuteDrag twice
                    m_bOldIdle        : 1, // to stop to idle
                    m_bOldIdleSet     : 1, // during QeueryDrop
                    m_bTblInsDelMode  : 1,
                    m_bTblIsInsMode   : 1,
                    m_bChainMode      : 1, // connect frames
                    m_bWasShdwCrsr    : 1, // ShadowCrsr was on in MouseButtonDown
                    m_bLockInput      : 1, // lock while calc panel is active
                    m_bIsRowDrag      : 1, //selection of rows is used, in combination with m_pRowColumnSelectionStart
                    /** #i42732# display status of font size/name depending on either the input language or the
                        selection position depending on what has changed lately
                     */
                    m_bUseInputLanguage: 1,
                    m_bObjectSelect   : 1;

    sal_uInt16          m_nKS_NUMDOWN_Count; // #i23725#
    sal_uInt16          m_nKS_NUMINDENTINC_Count;

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

    virtual void    DataChanged( const DataChangedEvent& ) SAL_OVERRIDE;
    virtual void    PrePaint() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    KeyInput(const KeyEvent &rKEvt) SAL_OVERRIDE;

    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;

    virtual void    MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void    MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void    RequestHelp(const HelpEvent& rEvt) SAL_OVERRIDE;

    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

                                // Drag & Drop Interface
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    virtual OUString GetSurroundingText() const SAL_OVERRIDE;
    virtual Selection GetSurroundingTextSelection() const SAL_OVERRIDE;

    void    ShowAutoTextCorrectQuickHelp( const OUString& rWord, SvxAutoCorrCfg* pACfg,
                                SvxAutoCorrect* pACorr, bool bFromIME = false );

    /// Returns true if in header/footer area, or in the header/footer control.
    bool    IsInHeaderFooter( const Point &rDocPt, FrameControlType &rControl ) const;

    bool    IsOverHeaderFooterFly( const Point& rDocPos, FrameControlType& rControl, bool& bOverFly, bool& bPageAnchored ) const;
public:

    void            UpdatePointer(const Point &, sal_uInt16 nButtons = 0);

    sal_Bool            IsDrawSelMode();
    sal_Bool            IsDrawAction()                  { return (m_bInsDraw); }
    void            SetDrawAction(sal_Bool bFlag)       { m_bInsDraw = bFlag; }

    void            SetObjectSelect( sal_Bool bVal )    { m_bObjectSelect = bVal; }
    sal_Bool            IsObjectSelect() const          { return m_bObjectSelect; }

    inline SdrObjKind   GetSdrDrawMode(/*sal_Bool bBuf = sal_False*/) const { return m_eDrawMode; }
    inline void         SetSdrDrawMode( SdrObjKind eSdrObjectKind ) { m_eDrawMode = eSdrObjectKind; SetObjectSelect( sal_False ); }
    void                StdDrawMode( SdrObjKind eSdrObjectKind, sal_Bool bObjSelect );

    bool            IsFrmAction() const             { return (m_bInsFrm); }
    sal_uInt16      GetBezierMode() const           { return m_eBezierMode; }
    void            SetBezierMode(sal_uInt16 eBezMode)  { m_eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const Point& aDocPos); // turn on DrawTextEditMode
    void            InsFrm(sal_uInt16 nCols);
    void            StopInsFrm();
    sal_uInt16      GetFrmColCount() const {return m_nInsFrmColCount;} // column number for interactive frame

    void            SetChainMode( sal_Bool bOn );
    sal_Bool            IsChainMode() const             { return m_bChainMode; }

    void            FlushInBuffer();
    sal_Bool            IsInputSequenceCheckingRequired( const OUString &rText, const SwPaM& rCrsr ) const;

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return m_pApplyTempl; }

    void            StartExecuteDrag();
    void            DragFinished();
    sal_uInt16          GetDropAction() const { return m_nDropAction; }
    sal_uLong           GetDropFormat() const { return m_nDropFormat; }

    Color           GetTextColor() { return m_aTextColor; }

    void            SetTextColor(const Color& rCol ) { m_aTextColor = rCol; }

    Color           GetTextBackColor()
                                            { return m_aTextBackColor; }
    void            SetTextBackColor(const Color& rCol )
                                            { m_aTextBackColor = rCol; }
    void            SetTextBackColorTransparent(bool bSet)
                                    { m_bTransparentBackColor = bSet; }
    bool            IsTextBackColorTransparent()
                                    { return m_bTransparentBackColor; }
    void            LockKeyInput(sal_Bool bSet){m_bLockInput = bSet;}

    const SwView &GetView() const { return m_rView; }
          SwView &GetView()       { return m_rView; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    static inline long GetDDStartPosX() { return m_nDDStartPosX; }
    static inline long GetDDStartPosY() { return m_nDDStartPosY; }

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
    sal_Bool    IsUseInputLanguage() const { return m_bUseInputLanguage; }

    SwFrameControlsManager& GetFrameControlsManager() { return m_aFrameControlsManager; }

    SwEditWin(Window *pParent, SwView &);
    virtual ~SwEditWin();
    virtual void SwitchView();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
