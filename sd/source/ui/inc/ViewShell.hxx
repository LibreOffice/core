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

#ifndef SD_VIEW_SHELL_HXX
#define SD_VIEW_SHELL_HXX

#include <rtl/ref.hxx>

#include <vcl/field.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/prntypes.hxx>
#include <svtools/transfer.hxx>
#include <comphelper/implementationreference.hxx>
#include "glob.hxx"
#include "pres.hxx"
#include "cfgids.hxx"
#include "View.hxx"
#include "sddllapi.h"

#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <memory>
#include <boost/shared_ptr.hpp>

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

extern const OUString aEmptyStr;

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

/** Base class of the stacked shell hierarchy.

    <p>Despite its name this class is not a descendant of SfxViewShell
    but of SfxShell.  Its name expresses the fact that it acts like a
    view shell.  Beeing a stacked shell rather then being an actual view shell
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
        ST_TASK_PANE,
        ST_SIDEBAR
    };
    static const int MAX_HSPLIT_CNT = 1;
    static const int MAX_VSPLIT_CNT = 1;
    static const int MIN_SCROLLBAR_SIZE = 50;

    static const sal_uLong OUTPUT_DRAWMODE_COLOR = DRAWMODE_DEFAULT;
    static const sal_uLong OUTPUT_DRAWMODE_GRAYSCALE
        = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL
        | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP
        | DRAWMODE_GRAYGRADIENT;
    static const int  OUTPUT_DRAWMODE_BLACKWHITE
        = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT
        | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP
        | DRAWMODE_WHITEGRADIENT;
    static const int OUTPUT_DRAWMODE_CONTRAST
        = DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL
        | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT;

    TYPEINFO();

    ViewShell (
        SfxViewFrame *pFrame,
        ::Window* pParentWindow,
        ViewShellBase& rViewShellBase,
        bool bAllowCenter = true);
    virtual ~ViewShell (void);

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
    virtual void Exit (void);

    void Cancel();

    /** Return the window that is the parent of all controls of this view
        shell.  This may or may not be the window of the frame.
    */
    inline ::Window* GetParentWindow (void) const;

    inline ::sd::View* GetView (void) const;
    inline SdrView* GetDrawView (void) const;
    SD_DLLPUBLIC DrawDocShell* GetDocSh (void) const;

    SdDrawDocument*  GetDoc (void) const;

    SD_DLLPUBLIC SfxViewFrame* GetViewFrame (void) const;

    /** The active window is usually the mpContentWindow.  When there is a
        show running then the active window is a ShowWindow.
    */
    ::sd::Window* GetActiveWindow (void) const;

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
    const Rectangle& GetAllWindowRect (void);

    // Mouse- & Key-Events
    virtual void PrePaint();
    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void Command(const CommandEvent& rCEvt, ::sd::Window* pWin);
    virtual sal_Bool RequestHelp( const HelpEvent& rEvt, ::sd::Window* pWin );
    virtual long Notify( NotifyEvent& rNEvt, ::sd::Window* pWin );

    virtual bool HandleScrollCommand(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual void Draw(OutputDevice &rDev, const Region &rReg);

    virtual void SetUIUnit(FieldUnit eUnit);
    virtual void SetDefTabHRuler( sal_uInt16 nDefTab );

    const SfxPoolItem* GetNumBulletItem(SfxItemSet& aNewAttr, sal_uInt16& nNumItemId);

    sal_Bool HasRuler (void);
    void SetRuler(sal_Bool bRuler);

    /** Set internal values of all scroll bars that determine thumb size and
        position.  The external values like size and position of the scroll
        bar controls are not modified.
    */
    virtual void UpdateScrollBars (void);
    void    Scroll(long nX, long nY);
    void    ScrollLines(long nX, long nY);
    virtual void    SetZoom(long nZoom);
    virtual void    SetZoomRect(const Rectangle& rZoomRect);
    void    InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                        const Point& rWinPos, sal_Bool bUpdate = sal_False);
    void    InvalidateWindows();
    /** This method is still used by the OutlineViewShell to update the
        model according to the content of the outline view.  This in turn
        updates the previews in the slide sorter.
    */
     virtual void UpdatePreview (SdPage* pPage, sal_Bool bInit = sal_False);

    void    DrawMarkRect(const Rectangle& rRect) const;

    void    ExecReq( SfxRequest &rReq );

    ZoomList* GetZoomList (void);

    FrameView* GetFrameView (void);
    /** Setting a frame view triggers ReadFrameViewData() for the new
        frame.
        @param pFrameView
            The new frame view that replaces the old one.
    */
    void SetFrameView (FrameView* pFrameView);
    virtual void  ReadFrameViewData(FrameView* pView);
    virtual void  WriteFrameViewData();
    virtual void  WriteUserData(OUString& rString);
    virtual void  ReadUserData(const OUString& rString);

    virtual sal_Bool  ActivateObject(SdrOle2Obj* pObj, long nVerb);

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

    FunctionReference GetOldFunction() const { return mxOldFunction; }
    bool HasOldFunction() const { return mxOldFunction.is(); }
    FunctionReference GetCurrentFunction() const { return mxCurrentFunction; }
    bool HasCurrentFunction( sal_uInt16 nSID ) { return mxCurrentFunction.is() && (mxCurrentFunction->GetSlotID() == nSID ); }
    bool HasCurrentFunction() { return mxCurrentFunction.is(); }

    void SetCurrentFunction(const FunctionReference& xFunction);
    void SetOldFunction(const FunctionReference& xFunction);
    void DeactivateCurrentFunction( bool bPermanent = false );

    void    SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                            long nLeft, long nRight, long nUpper, long nLower,
                            sal_Bool bScaleAll, Orientation eOrient, sal_uInt16 nPaperBin,
                            sal_Bool bBackgroundFullSize );

    void    SetStartShowWithDialog( sal_Bool bIn = sal_True ) { mbStartShowWithDialog = bIn; }
    sal_Bool    IsStartShowWithDialog() const { return mbStartShowWithDialog; }

    sal_uInt16 GetPrintedHandoutPageNum (void) const { return mnPrintedHandoutPageNum; }
    void SetPrintedHandoutPageNum (sal_uInt16 nPageNumber) {mnPrintedHandoutPageNum=nPageNumber; }

    sal_uInt16 GetPrintedHandoutPageCount(void) const { return mnPrintedHandoutPageCount; }
    void SetPrintedHandoutPageCount (sal_uInt16 nPageCount) {mnPrintedHandoutPageCount=nPageCount; }

    virtual sal_uInt16 PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );

    void GetMenuState(SfxItemSet& rSet);

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                 ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                  ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );

    virtual void WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const Rectangle& rRect);

    /** Create an accessible object representing the specified window.
        Overload this method to provide view mode specific objects.  The
        default implementation returns an empty reference.
        @param pWindow
            Make the document displayed in this window accessible.
        @return
            This default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

    void SetWinViewPos(const Point& rWinPos, bool bUpdate);
    Point GetWinViewPos() const;
    Point GetViewOrigin() const;

    /** Return the window updater of this view shell.
        @return
            In rare circumstances the returned pointer may be <null/>,
            i.e. when no memory is available anymore.
    */
    ::sd::WindowUpdater* GetWindowUpdater (void) const;

    /** Return the border that is drawn arround the actual document view.
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
    virtual void Resize (void);

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

    /** Set position and size of the GUI elements that are controllerd by
        the view shell like rulers and scroll bars as well as the actual
        document view according to the position and size that were given
        with the last Resize() call.
    */
    virtual void ArrangeGUIElements (void);

    //  virtual void OuterResizePixel(const Point &rPos, const Size &rSize);
    //  virtual void InnerResizePixel(const Point &rPos, const Size &rSize);

    ViewShellBase& GetViewShellBase (void) const;

    /** Return <TRUE/> when the called view shell is the main sub shell of
        its ViewShellBase object, i.e. is display in the center pane.  This
        convenience function is equivalent to comparing the this pointer to
        the result of ViewShellBase::GetViewShell(PT_CENTER).
    */
    bool IsMainViewShell (void) const;

    /** Set or reset the flag that indicates whether the called shell is the
        one displayed in the center pane.  By default this flag is set to
        <FALSE/>.  For the main view shell it thus has to be set to <TRUE/>.
    */
    void SetIsMainViewShell (bool bIsMainViewShell);

    /** Return a sub controller that implements the view shell specific
        part of the DrawController.
    */
    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void) = 0;

    /** Return the type of the shell.
    */
    virtual ShellType GetShellType (void) const;

    /** This method is more or less an alias to Deactivate().  It is called
        before an object of this class is taken from the stack of view
        shells.

        <p>When this method is not called before a view shell is taken from
        a stack then the Deactivate() call from the SFX as a response to
        RemoveSubShell() comes to late when the view shell is not on the
        stack anymore.</p>
    */
    virtual void Shutdown (void);

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
    sal_Bool IsPageFlipMode(void) const;

    /** Set the given window as new parent window.  This is not possible for
        all views, so the return value tells the caller if the relocation
        was successful.
    */
    virtual bool RelocateToParentWindow (::Window* pParentWindow);

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


    class Implementation;

protected:
    /** must be called in the beginning of each subclass d'tor.
        disposes and clears both current and old function. */
    void DisposeFunctions();

    friend class ViewShellBase;

    /** Window inside the rulers and scroll bars that shows a view of the
        document.
    */

    ::boost::shared_ptr<sd::Window> mpContentWindow;

    /// Horizontal scroll bar for the current slide is displayed when needed.
    ::boost::shared_ptr<ScrollBar> mpHorizontalScrollBar;
    /// Vertical scroll bar for whole document is always visible.
    ::boost::shared_ptr<ScrollBar> mpVerticalScrollBar;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    /// Horizontal ruler is not shown by default.
    ::std::auto_ptr<SvxRuler> mpHorizontalRuler;
    /// Vertical ruler is not shown by default.
    ::std::auto_ptr<SvxRuler> mpVerticalRuler;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    /// Filler of the little square enclosed by the two scroll bars.
    ::boost::shared_ptr<ScrollBarBox> mpScrollBarBox;
    /// Layer tab bar.
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<LayerTabBar> mpLayerTabBar;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    /// This flag controls whether the rulers are visible.
    bool mbHasRulers;

    /// The active window.
    ::sd::Window* mpActiveWindow;
    ::sd::View* mpView;
    FrameView*  mpFrameView;

    FunctionReference   mxCurrentFunction;
    FunctionReference   mxOldFunction;
    ZoomList*   mpZoomList;

    Point       maViewPos;
    Size        maViewSize;
    Size        maScrBarWH;

    sal_Bool        mbCenterAllowed;          // will be forwarded to window

    sal_Bool        mbStartShowWithDialog;    // presentation is started by dialog
    sal_uInt16      mnPrintedHandoutPageNum; // Page number of the handout page that is to be printed.
    sal_uInt16      mnPrintedHandoutPageCount; // Page count of the handout pages that are to be printed.

    //af    sal_Bool        bPrintDirectSelected;       // Print only selected objects in direct print
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

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<Implementation> mpImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // Support methods for centralized UNDO/REDO
    virtual ::svl::IUndoManager* ImpGetUndoManager (void) const;
    void ImpGetUndoStrings(SfxItemSet &rSet) const;
    void ImpGetRedoStrings(SfxItemSet &rSet) const;
    void ImpSidUndo(sal_Bool bDrawViewShell, SfxRequest& rReq);
    void ImpSidRedo(sal_Bool bDrawViewShell, SfxRequest& rReq);

    DECL_LINK( HScrollHdl, ScrollBar * );
    DECL_LINK( VScrollHdl, ScrollBar * );

    // virtual scroll handler, here, derivative classes can add themselves here
    virtual long VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long VirtVScrollHdl(ScrollBar* pVScroll);

    // virtual functions ruler handling
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, sal_Bool bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);
    virtual void UpdateHRuler();
    virtual void UpdateVRuler();

    virtual void Activate(sal_Bool IsMDIActivate);
    virtual void Deactivate(sal_Bool IsMDIActivate);

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
        from the ctor then if a derived class is contructed the base-cass
        CreateAccessibleDocumentView is used, not the derived
        CreateAccessibleDocumentView. i.e. run smoketest under a11y with
        debugging assertions enabled
    */
    void doShow();

private:
    ::Window* mpParentWindow;
    /** This window updater is used to keep all relevant windows up to date
        with reference to the digit langugage used to display digits in text
        shapes.
    */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::sd::WindowUpdater> mpWindowUpdater;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    /** Code common to all constructors.  It generally is a bad idea
        to call this function from outside a constructor.
    */
    void construct (void);

    DECL_LINK(FrameWindowEventListener, VclSimpleEvent*);

    /** Create the rulers.
    */
    void SetupRulers (void);
};




::Window* ViewShell::GetParentWindow (void) const
{
    return mpParentWindow;
}

::sd::View* ViewShell::GetView (void) const
{
    return mpView;
}

SdrView* ViewShell::GetDrawView (void) const
{
    return static_cast<SdrView*>(mpView);
}

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
