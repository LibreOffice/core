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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEWSHELL_HXX

#include <rtl/ref.hxx>

#include <vcl/field.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/prntypes.hxx>
#include <svtools/transfer.hxx>
#include "glob.hxx"
#include "pres.hxx"
#include "cfgids.hxx"
#include "View.hxx"
#include "sddllapi.h"

#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <memory>

class SdPage;
class SvxRuler;
class SdrOle2Obj;       // for the ones, who have undefined parts of SVDRAW
class ScrollBarBox;
class SdDrawDocument;
class ScrollBar;

namespace com { namespace sun { namespace star {
namespace embed {
    class XEmbeddedObject;
}}}}

namespace sd {

class DrawDocShell;
class FrameView;
class LayerTabBar;
class View;
class ViewShellBase;
class Window;
class WindowUpdater;
class ZoomList;

#undef OUTPUT_DRAWMODE_COLOR
#undef OUTPUT_DRAWMODE_CONTRAST

static const DrawModeFlags OUTPUT_DRAWMODE_COLOR = DrawModeFlags::Default;
static const DrawModeFlags OUTPUT_DRAWMODE_GRAYSCALE
        = DrawModeFlags::GrayLine | DrawModeFlags::GrayFill
        | DrawModeFlags::BlackText | DrawModeFlags::GrayBitmap
        | DrawModeFlags::GrayGradient;
static const DrawModeFlags OUTPUT_DRAWMODE_BLACKWHITE
        = DrawModeFlags::BlackLine | DrawModeFlags::BlackText
        | DrawModeFlags::WhiteFill | DrawModeFlags::GrayBitmap
        | DrawModeFlags::WhiteGradient;
static const DrawModeFlags OUTPUT_DRAWMODE_CONTRAST
        = DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill
        | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient;

/** Base class of the stacked shell hierarchy.

    <p>Despite its name this class is not a descendant of SfxViewShell
    but of SfxShell.  Its name expresses the fact that it acts like a
    view shell.  Being a stacked shell rather then being an actual view shell
    there can be several instances of this class that
    <ul>
    <li>all are based on the same view shell and thus show the same
    document and share common view functionality and</li>
    <li>are all visible at the same time and live in the same
    frame.</li>
    <ul></p>

    <p>This class replaces the former ViewShell class.</p>
*/
class ViewShell
    : public SfxShell
{
public:
    enum ShellType {
        ST_NONE,
        ST_DRAW,         // The Draw application.
        ST_IMPRESS,      // Main view of the Impress application.
        ST_NOTES,
        ST_HANDOUT,
        ST_OUTLINE,
        ST_SLIDE_SORTER,
        ST_PRESENTATION,
        ST_SIDEBAR
    };
    static const int MAX_HSPLIT_CNT = 1;
    static const int MAX_VSPLIT_CNT = 1;
    static const int MIN_SCROLLBAR_SIZE = 50;


    ViewShell (
        SfxViewFrame *pFrame,
        vcl::Window* pParentWindow,
        ViewShellBase& rViewShellBase,
        bool bAllowCenter = true);
    virtual ~ViewShell();

    /** The Init method has to be called from the outside directly
        after a new object of this class has been created.  It can be
        used for that part of the initialisation that can be run only
        after the creation of the new object is finished.  This
        includes registration as listener at event broadcasters.

        Derived classes should call this method at the head of their
        Init() methods.
        @param bIsMainViewShell
            This flag tells the Init() method whether the new ViewShell will
            be the main view shell.
    */
    virtual void Init (bool bIsMainViewShell);

    /** The Exit() method has to be called before the destructor so that the
        view shell is still a valid object and can safely call methods that
        rely on that.
    */
    void Exit();

    void Cancel();

    /** Return the window that is the parent of all controls of this view
        shell.  This may or may not be the window of the frame.
    */
    inline vcl::Window* GetParentWindow() const { return mpParentWindow; }

    inline ::sd::View* GetView() const { return mpView; }
    inline SdrView* GetDrawView() const;
    SD_DLLPUBLIC DrawDocShell* GetDocSh() const;

    SdDrawDocument*  GetDoc() const;

    SD_DLLPUBLIC SfxViewFrame* GetViewFrame() const;

    /** The active window is usually the mpContentWindow.  When there is a
        show running then the active window is a ShowWindow.
    */
    ::sd::Window* GetActiveWindow() const { return mpActiveWindow;}

    /** Set the active window.  When the shell is displayed in the center
        pane then the window of the ViewShellBase is also set to the given
        window.
    */
    void SetActiveWindow (::sd::Window* pWindow);

    /** Return the rectangle that encloses all windows of the view.  That
        excludes the controls in the frame like rulers, scroll bars, tab
        bar, and buttons.
        @return
            The rectangle is returned in screen coordinates, i.e. pixel
            values relative to the upper left corner of the screen?.
    */
    const Rectangle& GetAllWindowRect();

    // Mouse- & Key-Events
    virtual void PrePaint();
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
    virtual bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void Command(const CommandEvent& rCEvt, ::sd::Window* pWin);
    bool RequestHelp( const HelpEvent& rEvt, ::sd::Window* pWin );
    bool Notify( NotifyEvent& rNEvt, ::sd::Window* pWin );

    bool HandleScrollCommand(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual void SetUIUnit(FieldUnit eUnit);
    void SetDefTabHRuler( sal_uInt16 nDefTab );

    const SfxPoolItem* GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt16& nNumItemId);

    bool HasRuler() { return mbHasRulers;}
    void SetRuler(bool bRuler);

    /** Set internal values of all scroll bars that determine thumb size and
        position.  The external values like size and position of the scroll
        bar controls are not modified.
    */
    virtual void UpdateScrollBars();
    void    Scroll(long nX, long nY);
    void    ScrollLines(long nX, long nY);
    virtual void    SetZoom(long nZoom);
    virtual void    SetZoomRect(const Rectangle& rZoomRect);
    void    InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                        const Point& rWinPos, bool bUpdate = false);
    void    InvalidateWindows();
    /** This method is still used by the OutlineViewShell to update the
        model according to the content of the outline view.  This in turn
        updates the previews in the slide sorter.
    */
    virtual void UpdatePreview (SdPage* pPage, bool bInit = false);

    void    DrawMarkRect(const Rectangle& rRect) const;

    void    ExecReq( SfxRequest &rReq );

    ZoomList* GetZoomList() { return mpZoomList;}

    FrameView* GetFrameView() { return mpFrameView; }
    /** Setting a frame view triggers ReadFrameViewData() for the new
        frame.
        @param pFrameView
            The new frame view that replaces the old one.
    */
    void SetFrameView (FrameView* pFrameView);
    virtual void  ReadFrameViewData(FrameView* pView);
    virtual void  WriteFrameViewData();
    void  WriteUserData(OUString& rString);
    void  ReadUserData(const OUString& rString);

    virtual bool  ActivateObject(SdrOle2Obj* pObj, long nVerb);

    /** @returns
            current or selected page or 0. This method
            will fail in master page mode.

        @deprecated, please use getCurrentPage();
    */
    virtual SdPage* GetActualPage() = 0;

    /** @returns
            current or selected page or 0.
    */
    virtual SdPage* getCurrentPage() const = 0;

    rtl::Reference<FuPoor> GetOldFunction() const { return mxOldFunction; }
    bool HasOldFunction() const { return mxOldFunction.is(); }
    rtl::Reference<FuPoor> GetCurrentFunction() const { return mxCurrentFunction; }
    bool HasCurrentFunction( sal_uInt16 nSID ) { return mxCurrentFunction.is() && (mxCurrentFunction->GetSlotID() == nSID ); }
    bool HasCurrentFunction() { return mxCurrentFunction.is(); }

    void SetCurrentFunction(const rtl::Reference<FuPoor>& xFunction);
    void SetOldFunction(const rtl::Reference<FuPoor>& xFunction);
    void DeactivateCurrentFunction( bool bPermanent = false );

    void    SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                            long nLeft, long nRight, long nUpper, long nLower,
                            bool bScaleAll, Orientation eOrient, sal_uInt16 nPaperBin,
                            bool bBackgroundFullSize );

    void    SetStartShowWithDialog( bool bIn = true ) { mbStartShowWithDialog = bIn; }
    bool    IsStartShowWithDialog() const { return mbStartShowWithDialog; }

    sal_uInt16 GetPrintedHandoutPageNum() const { return mnPrintedHandoutPageNum; }
    void SetPrintedHandoutPageNum (sal_uInt16 nPageNumber) {mnPrintedHandoutPageNum=nPageNumber; }

    sal_uInt16 GetPrintedHandoutPageCount() const { return mnPrintedHandoutPageCount; }
    void SetPrintedHandoutPageCount (sal_uInt16 nPageCount) {mnPrintedHandoutPageCount=nPageCount; }

    virtual bool PrepareClose( bool bUI = true );

    void GetMenuState(SfxItemSet& rSet);

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                 ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                  ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );

    virtual void WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse = false );
    virtual void ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse = false );

    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const Rectangle& rRect);

    /** Create an accessible object representing the specified window.
        Override this method to provide view mode specific objects.  The
        default implementation returns an empty reference.
        @param pWindow
            Make the document displayed in this window accessible.
        @return
            This default implementation returns an empty reference.
    */
    virtual css::uno::Reference<css::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

    virtual void SwitchViewFireFocus( css::uno::Reference< css::accessibility::XAccessible > xAcc );
    void SwitchActiveViewFireFocus( );
    // Move these two methods from DrawViewShell to enable slide show view
    void    NotifyAccUpdate();
    void    fireSwitchCurrentPage(sal_Int32 pageIndex);
    void SetWinViewPos(const Point& rWinPos, bool bUpdate);
    Point GetWinViewPos() const;
    Point GetViewOrigin() const;

    /** Return the window updater of this view shell.
        @return
            In rare circumstances the returned pointer may be <null/>,
            i.e. when no memory is available anymore.
    */
    ::sd::WindowUpdater* GetWindowUpdater() const;

    /** Return the border that is drawn around the actual document view.
        The border contains typically rulers and scroll bars.
        @param bOuterResize
            When this flag is <TRUE/> then the border is used for an
            OuterResizePixel(), i.e. there is a given window size and the
            border elements are placed inside so that the document view has
            the given window size minus the border.
            When the flag is <FALSE/> then the border is used for an
            InnerResizePixel(), i.e. the document view has a given size and
            the border is placed outside.  In this scenario the parent
            window has the size of the document view plus the border.
    */
    SvBorder GetBorder (bool bOuterResize);

    /** Notify the view shell that its parent window has been resized.
        The ViewShell places and resizes its UI elements accordingly.
        The new size can be obtained from the parent window.
    */
    virtual void Resize();

    /** Set the position and size of the area which contains the GUI
        elements like rulers, sliders, and buttons as well as the document
        view.  Both size and position are expected to be in pixel
        coordinates.  The positions and sizes of the mentioned GUI elements
        are updated as well.

        <p> This method is implemented by first setting copying the given
        values to internal variables and then calling the
        <type>ArrangeGUIElements</type> method which performs the actual
        work of sizeing and arranging the UI elements accordingly.</p>
        @param rPos
            The position of the enclosing window relative to the document
            window.  This is only interesting if a Draw/Impress document
            view is embedded as OLE object into another document view.  For
            normal documents this position is (0,0).
        @param rSize
            The new size in pixel.
    */
    // This is to be replaced by Resize.
    //  virtual void AdjustPosSizePixel(const Point &rPos, const Size &rSize);

    /** Set position and size of the GUI elements that are controlled by
        the view shell like rulers and scroll bars as well as the actual
        document view according to the position and size that were given
        with the last Resize() call.
    */
    virtual void ArrangeGUIElements();

    //  virtual void OuterResizePixel(const Point &rPos, const Size &rSize);
    //  virtual void InnerResizePixel(const Point &rPos, const Size &rSize);

    // Exported for unit test
    SD_DLLPUBLIC ViewShellBase& GetViewShellBase() const;

    /** Return <TRUE/> when the called view shell is the main sub shell of
        its ViewShellBase object, i.e. is display in the center pane.  This
        convenience function is equivalent to comparing the this pointer to
        the result of ViewShellBase::GetViewShell(PT_CENTER).
    */
    bool IsMainViewShell() const;

    /** Set or reset the flag that indicates whether the called shell is the
        one displayed in the center pane.  By default this flag is set to
        <FALSE/>.  For the main view shell it thus has to be set to <TRUE/>.
    */
    void SetIsMainViewShell (bool bIsMainViewShell);

    /** Return a sub controller that implements the view shell specific
        part of the DrawController.
    */
    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() = 0;

    /** Return the type of the shell.
    */
    ShellType GetShellType() const;

    /** This method is more or less an alias to Deactivate().  It is called
        before an object of this class is taken from the stack of view
        shells.

        <p>When this method is not called before a view shell is taken from
        a stack then the Deactivate() call from the SFX as a response to
        RemoveSubShell() comes to late when the view shell is not on the
        stack anymore.</p>
    */
    virtual void Shutdown();

    /** This function is called from the underlying ViewShellBase
        object to handle a verb execution request.
    */
    virtual ErrCode DoVerb (long nVerb);

    virtual void UIActivating( SfxInPlaceClient* );
    virtual void UIDeactivated( SfxInPlaceClient* );

    /** Show controls of the UI or hide them, depending on the given flag.
        As a result the border is adapted.
    */
    virtual void ShowUIControls (bool bVisible = true);
    bool IsPageFlipMode() const;

    /** Set the given window as new parent window.  This is not possible for
        all views, so the return value tells the caller if the relocation
        was successful.
    */
    virtual bool RelocateToParentWindow (vcl::Window* pParentWindow);

    /** Depending on the given request create a new page or duplicate an
        existing one.  A new page is created behind the given slide.
        @param rRequest
            The request as passed to an Execute() method.  Its arguments are
            evaluated.  Its slot id determines whether to create or
            duplicate a slide.
        @param pPage
            This page is either duplicated or becomes the predecessor of the
            new slide.  If NULL a duplication request is ignored.  A new
            slide is inserted as first slide.
        @param nInsertPosition
            When -1 (the default) then insert after pPage.  Otherwise insert
            before the given index (of a standard page).
        @return
            The new slide is returned.  If for some reason a new page can
            not be created then NULL is returned.
    */
    virtual SdPage* CreateOrDuplicatePage (
        SfxRequest& rRequest,
        PageKind ePageKind,
        SdPage* pPage,
        const sal_Int32 nInsertPosition = -1);

    /// Same as MouseButtonDown(), but coordinates are in logic unit.
    void LogicMouseButtonDown(const MouseEvent& rMouseEvent);
    /// Same as MouseButtonUp(), but coordinates are in logic unit.
    void LogicMouseButtonUp(const MouseEvent& rMouseEvent);
    /// Same as MouseMove(), but coordinates are in logic unit.
    void LogicMouseMove(const MouseEvent& rMouseEvent);
    /// Allows adjusting the point or mark of the selection to a document coordinate.
    void SetCursorMm100Position(const Point& rPosition, bool bPoint, bool bClearMark);
    /// Gets the currently selected text.
    OString GetTextSelection(const OString& aMimeType, OString& rUsedMimeType);
    /// Allows starting or ending a graphic move or resize action.
    void SetGraphicMm100Position(bool bStart, const Point& rPosition);

    class Implementation;

protected:
    /** must be called in the beginning of each subclass d'tor.
        disposes and clears both current and old function. */
    void DisposeFunctions();

    friend class ViewShellBase;

    /** Window inside the rulers and scroll bars that shows a view of the
        document.
    */

    VclPtr<sd::Window> mpContentWindow;

    /// Horizontal scroll bar for the current slide is displayed when needed.
    VclPtr<ScrollBar> mpHorizontalScrollBar;
    /// Vertical scroll bar for whole document is always visible.
    VclPtr<ScrollBar> mpVerticalScrollBar;
    /// Horizontal ruler is not shown by default.
    VclPtr<SvxRuler> mpHorizontalRuler;
    /// Vertical ruler is not shown by default.
    VclPtr<SvxRuler> mpVerticalRuler;
    /// Filler of the little square enclosed by the two scroll bars.
    VclPtr<ScrollBarBox> mpScrollBarBox;
    /// Layer tab bar.
    VclPtr<LayerTabBar> mpLayerTabBar;

    /// This flag controls whether the rulers are visible.
    bool mbHasRulers;

    /// The active window.
    VclPtr< ::sd::Window> mpActiveWindow;
    ::sd::View* mpView;
    FrameView*  mpFrameView;

    rtl::Reference<FuPoor>   mxCurrentFunction;
    rtl::Reference<FuPoor>   mxOldFunction;
    ZoomList*   mpZoomList;

    Point       maViewPos;
    Size        maViewSize;
    Size        maScrBarWH;

    bool        mbCenterAllowed;          // will be forwarded to window

    bool        mbStartShowWithDialog;    // presentation is started by dialog
    sal_uInt16      mnPrintedHandoutPageNum; // Page number of the handout page that is to be printed.
    sal_uInt16      mnPrintedHandoutPageCount; // Page count of the handout pages that are to be printed.

    //af    bool        bPrintDirectSelected;       // Print only selected objects in direct print
    //afString      sPageRange;                 // pagerange if selected objects in direct print

    /** Area covered by all windows, i.e. the area of the parent window
        without the controls at the borders like rulers, scroll bars, tab
        bar, buttons.
        This rectangle may be set in window coordinates (i.e. pixel values
        relative to the parent window).  It is transformed by every call to
        GetAllWindowRectangle() into screen coordinates (relative to the
        upper left corner of the screen.
    */
    Rectangle maAllWindowRectangle;

    /// The type of the shell.  Returned by GetShellType().
    ShellType meShellType;

    ::std::unique_ptr<Implementation> mpImpl;

    // Support methods for centralized UNDO/REDO
    virtual ::svl::IUndoManager* ImpGetUndoManager() const;
    void ImpGetUndoStrings(SfxItemSet &rSet) const;
    void ImpGetRedoStrings(SfxItemSet &rSet) const;
    void ImpSidUndo(bool bDrawViewShell, SfxRequest& rReq);
    void ImpSidRedo(bool bDrawViewShell, SfxRequest& rReq);

    DECL_LINK_TYPED( HScrollHdl, ScrollBar *, void );
    DECL_LINK_TYPED( VScrollHdl, ScrollBar *, void );

    // virtual scroll handler, here, derivative classes can add themselves here
    virtual long VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long VirtVScrollHdl(ScrollBar* pVScroll);

    // virtual functions ruler handling
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, bool bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);
    virtual void UpdateHRuler();
    virtual void UpdateVRuler();

    virtual void Activate(bool IsMDIActivate) override;
    virtual void Deactivate(bool IsMDIActivate) override;

    virtual void SetZoomFactor( const Fraction &rZoomX,
                                const Fraction &rZoomY );

    /**
        This must be called after the ctor, but before anything else.
        It's the part of construction that is dependent
        on showing the top-level window.

        Showing a window with a11y enabled causes various callbacks
        to be triggered.

        Due to the "virtual methods are not virtual during constructors"
        problem, this is a disaster to call from the ctor

        i.e. construct calls Show, and if a11y is enabled this
        reenters the not-fully constructed object and calls
        CreateAccessibleDocumentView, so if construct is called
        from the ctor then if a derived class is contructed the base-case
        CreateAccessibleDocumentView is used, not the derived
        CreateAccessibleDocumentView. i.e. run smoketest under a11y with
        debugging assertions enabled
    */
    void doShow();

private:
    VclPtr<vcl::Window> mpParentWindow;
    /** This window updater is used to keep all relevant windows up to date
        with reference to the digit language used to display digits in text
        shapes.
    */
    ::std::unique_ptr< ::sd::WindowUpdater> mpWindowUpdater;

    /** Code common to all constructors.  It generally is a bad idea
        to call this function from outside a constructor.
    */
    void construct();

    /** Create the rulers.
    */
    void SetupRulers();
};

SdrView* ViewShell::GetDrawView() const
{
    return static_cast<SdrView*>(mpView);
}

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
