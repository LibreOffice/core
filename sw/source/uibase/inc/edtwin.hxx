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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_EDTWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_EDTWIN_HXX

#include <sot/exchange.hxx>
#include <svx/svdobj.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <swevent.hxx>
#include <swtypes.hxx>

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
class   SwFrameControlsManager;
enum class SdrHitKind;

// input window

/** Window class for the Writer edit area, this is the one handling mouse
    and keyboard events and doing the final painting of the document from
    the buffered layout.

    To translate the pixel positions from the buffer OutputDevice to the real
    pixel positions, use the PixelToLogic methods of this class.
  */
class SwEditWin final : public vcl::Window,
                public DropTargetHelper, public DragSourceHelper
{
    static  QuickHelpData* m_pQuickHlpData;

    static  long    m_nDDStartPosX, m_nDDStartPosY;

    Color m_aWaterCanTextColor;     // text color; for the watering can
    Color m_aWaterCanTextBackColor; // text background; for the watering can

    /*
     * timer and handler for scrolling on when the mousepointer
     * stops outside of EditWin during a drag-operation.
     * The selection is increased towards the mouse position
     * regularly.
     */
    AutoTimer       m_aTimer;
    // timer for ANY-KeyInput question without a following KeyInputEvent
    Timer           m_aKeyInputFlushTimer;

    OUString        m_aInBuffer;
    LanguageType    m_eBufferLanguage;
    Point           m_aStartPos;
    Point           m_aMovePos;
    Point           m_aRszMvHdlPt;
    Idle            m_aTemplateIdle;

    // type/object where the mouse pointer is
    SwCallMouseEvent m_aSaveCallEvent;

    std::unique_ptr<SwApplyTemplate>      m_pApplyTempl;
    std::unique_ptr<SwAnchorMarker>       m_pAnchorMarker; // for relocating the anchor

    std::unique_ptr<SdrDropMarkerOverlay> m_pUserMarker;
    SdrObject               *m_pUserMarkerObj;
    std::unique_ptr<SwShadowCursor>       m_pShadCursor;
    boost::optional<Point>                m_xRowColumnSelectionStart; // save position where table row/column selection has been started

    SwView         &m_rView;

    SdrHitKind      m_aActHitType;    // current mouse pointer

    SotClipboardFormatId m_nDropFormat;  // format from the last QueryDrop
    sal_uInt8       m_nDropAction;       // action from the last QueryDrop
    SotExchangeDest m_nDropDestination;  // destination from the last QueryDrop

    sal_uInt16      m_eBezierMode;
    sal_uInt16      m_nInsFrameColCount; // column number for interactive frame
    SdrObjKind      m_eDrawMode;
    bool        m_bMBPressed      : 1,
                    m_bInsDraw        : 1,
                    m_bInsFrame         : 1,
                    m_bIsInMove       : 1,
                    m_bIsInDrag       : 1, // don't execute StartExecuteDrag twice
                    m_bOldIdle        : 1, // to stop to idle
                    m_bOldIdleSet     : 1, // during QeueryDrop
                    m_bChainMode      : 1, // connect frames
                    m_bWasShdwCursor    : 1, // ShadowCursor was on in MouseButtonDown
                    m_bLockInput      : 1, // lock while calc panel is active
                    m_bIsRowDrag      : 1, //selection of rows is used, in combination with m_pRowColumnSelectionStart
                    /** #i42732# display status of font size/name depending on either the input language or the
                        selection position depending on what has changed lately
                     */
                    m_bUseInputLanguage: 1,
                    m_bObjectSelect   : 1;

    sal_uInt16          m_nKS_NUMDOWN_Count; // #i23725#
    sal_uInt16          m_nKS_NUMINDENTINC_Count;

    std::unique_ptr<SwFrameControlsManager> m_pFrameControlsManager;

    void            LeaveArea(const Point &);
    void            JustifyAreaTimer();
    inline void     EnterArea();

    void            RstMBDownFlags();

    void            ChangeFly( sal_uInt8 nDir, bool bWeb );
    void            ChangeDrawing( sal_uInt8 nDir );

    bool            EnterDrawMode(const MouseEvent& rMEvt, const Point& aDocPos);
    bool            RulerColumnDrag( const MouseEvent& rMEvt, bool bVerticalMode);

    // helper function for D&D
    void            DropCleanup();
    void            CleanupDropUserMarker();
    SotExchangeDest GetDropDestination( const Point& rPixPnt,
                                        SdrObject ** ppObj = nullptr );
    //select the object/cursor at the mouse position of the context menu request
    void            SelectMenuPosition(SwWrtShell& rSh, const Point& rMousePos );

    /*
     * handler for scrolling on when the mousepointer
     * stops outside of EditWin during a drag-operation.
     * The selection is regularly increased towards the mouse
     * position.
     */
    DECL_LINK( TimerHandler, Timer *, void );
    void            StartDDTimer();
    void            StopDDTimer(SwWrtShell *, const Point &);
    DECL_LINK( DDHandler, Timer *, void );

    // timer for ANY-KeyInut question without a following KeyInputEvent
    DECL_LINK( KeyInputFlushHandler, Timer *, void );

    // timer for ApplyTemplates via mouse (in disguise Drag&Drop)
    DECL_LINK( TemplateTimerHdl, Timer *, void );

    void            MoveCursor( SwWrtShell &rSh, const Point& rDocPos,
                                const bool bOnlyText, bool bLockView );

    virtual void    DataChanged( const DataChangedEvent& ) override;
    virtual void    PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;

    virtual void    MouseMove(const MouseEvent& rMEvt) override;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void    MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void    RequestHelp(const HelpEvent& rEvt) override;

                                // Drag & Drop Interface
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    virtual OUString GetSurroundingText() const override;
    virtual Selection GetSurroundingTextSelection() const override;

    void    ShowAutoTextCorrectQuickHelp( const OUString& rWord, SvxAutoCorrCfg const * pACfg,
                                SvxAutoCorrect* pACorr, bool bFromIME = false );

    /// Returns true if in header/footer area, or in the header/footer control.
    bool    IsInHeaderFooter( const Point &rDocPt, FrameControlType &rControl ) const;

    bool    IsOverHeaderFooterFly( const Point& rDocPos, FrameControlType& rControl, bool& bOverFly, bool& bPageAnchored ) const;

public:
    virtual void    KeyInput(const KeyEvent &rKEvt) override;
    void            UpdatePointer(const Point &, sal_uInt16 nButtons = 0);

    bool            IsDrawSelMode();
    bool            IsDrawAction()                  { return m_bInsDraw; }
    void            SetDrawAction(bool bFlag)       { m_bInsDraw = bFlag; }

    void            SetObjectSelect( bool bVal )    { m_bObjectSelect = bVal; }
    bool            IsObjectSelect() const          { return m_bObjectSelect; }

    SdrObjKind   GetSdrDrawMode() const { return m_eDrawMode; }
    void         SetSdrDrawMode( SdrObjKind eSdrObjectKind ) { m_eDrawMode = eSdrObjectKind; SetObjectSelect( false ); }
    void                StdDrawMode( SdrObjKind eSdrObjectKind, bool bObjSelect );

    bool            IsFrameAction() const             { return m_bInsFrame; }
    sal_uInt16      GetBezierMode() const           { return m_eBezierMode; }
    void            SetBezierMode(sal_uInt16 eBezMode)  { m_eBezierMode = eBezMode; }
    void            EnterDrawTextMode(const Point& aDocPos); // turn on DrawTextEditMode
    void            InsFrame(sal_uInt16 nCols);
    void            StopInsFrame();
    sal_uInt16      GetFrameColCount() const {return m_nInsFrameColCount;} // column number for interactive frame

    void            SetChainMode( bool bOn );
    bool            IsChainMode() const             { return m_bChainMode; }

    void            FlushInBuffer();
    static bool     IsInputSequenceCheckingRequired( const OUString &rText, const SwPaM& rCursor );

    void             SetApplyTemplate(const SwApplyTemplate &);
    SwApplyTemplate* GetApplyTemplate() const { return m_pApplyTempl.get(); }

    void            StartExecuteDrag();
    void            DragFinished();

    void         SetWaterCanTextColor(const Color& rCol ) { m_aWaterCanTextColor = rCol; }

    void         SetWaterCanTextBackColor(const Color& rCol ) { m_aWaterCanTextBackColor = rCol; }

    void            LockKeyInput(bool bSet){m_bLockInput = bSet;}

    const SwView &GetView() const { return m_rView; }
          SwView &GetView()       { return m_rView; }

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    static long GetDDStartPosX() { return m_nDDStartPosX; }
    static long GetDDStartPosY() { return m_nDDStartPosY; }

    static void InitStaticData();
    static void FinitStaticData();

    //#i3370# remove quick help to prevent saving of autocorrection suggestions
    void StopQuickHelp();

    // #i42921# - add parameter <bVerticalMode>
    bool RulerMarginDrag( const MouseEvent& rMEvt,
                                     const bool bVerticalMode );

    /** #i42732# display status of font size/name depending on either the input
        language or the selection position depending on what has changed lately
     */
    void    SetUseInputLanguage( bool bNew );
    bool    IsUseInputLanguage() const { return m_bUseInputLanguage; }

    /** fdo#55546 cut very long tooltips to 2/3 of the width of the screen
        via center ellipsis
     */
    OUString ClipLongToolTip(const OUString& rText);

    SwFrameControlsManager& GetFrameControlsManager();

    SwEditWin(vcl::Window *pParent, SwView &);
    virtual ~SwEditWin() override;
    virtual void dispose() override;

    virtual void    Command( const CommandEvent& rCEvt ) override;

    /// @see Window::LogicInvalidate().
    void LogicInvalidate(const tools::Rectangle* pRectangle) override;
    /// Same as MouseButtonDown(), but coordinates are in logic unit.
    virtual void LogicMouseButtonDown(const MouseEvent& rMouseEvent) override;
    /// Same as MouseButtonUp(), but coordinates are in logic unit.
    virtual void LogicMouseButtonUp(const MouseEvent& rMouseEvent) override;
    /// Same as MouseMove(), but coordinates are in logic unit.
    virtual void LogicMouseMove(const MouseEvent& rMouseEvent) override;
    /// Allows adjusting the point or mark of the selection to a document coordinate.
    void SetCursorTwipPosition(const Point& rPosition, bool bPoint, bool bClearMark);
    /// Allows starting or ending a graphic move or resize action.
    void SetGraphicTwipPosition(bool bStart, const Point& rPosition);

    virtual FactoryFunction GetUITestFactory() const override;
};

extern bool g_bFrameDrag;
extern bool g_bDDTimerStarted;
extern bool g_bFlushCharBuffer;
extern bool g_bDDINetAttr;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
