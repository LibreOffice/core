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

#ifndef SD_DRAW_VIEW_SHELL_HXX
#define SD_DRAW_VIEW_SHELL_HXX

#include "ViewShell.hxx"
#include "tools/AsynchronousCall.hxx"
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include "TabControl.hxx"
#include "pres.hxx"
#include <svx/sidebar/SelectionChangeHandler.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <unotools/caserotate.hxx>

class SdPage;
class DrawDocShell;
class TabBar;
class SdrObject;
class SdrPageView;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractSvxNameDialog;
class SdrLayer;
class SvxClipboardFmtItem;

namespace sd {

class DrawView;
class LayerTabBar;
class Ruler;
class AnnotationManager;
class ViewOverlayManager;

#define CHECK_RANGE(nMin, nValue, nMax) ((nValue >= nMin) && (nValue <= nMax))

/** Base class of the stacked shells that provide graphical views to
    Draw and Impress documents and editing functionality.  In contrast
    to this other stacked shells are responsible for showing an
    overview over several slides or a textual
    overview over the text in an Impress document (OutlineViewShell).
*/
class DrawViewShell
    : public ViewShell,
      public SfxListener
{
public:
    static const int SLOTARRAY_COUNT = 24;

    TYPEINFO();

    SFX_DECL_INTERFACE(SD_IF_SDDRAWVIEWSHELL)

    /** Create a new stackable shell that may take some information
        (e.g. the frame view) from the given previous shell.
        @param ePageKind
            This parameter gives the initial page kind that the new shell
            will show.
        @param pFrameView
            The frame view that makes it possible to pass information from
            one view shell to the next.
    */
    DrawViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        PageKind ePageKind = PK_STANDARD,
        FrameView* pFrameView = NULL);

    virtual ~DrawViewShell (void);

    virtual void Init (bool bIsMainViewShell);

    virtual void Shutdown (void);

    void PrePaint();
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin);

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
    //  virtual void    AdjustPosSizePixel(const Point &rPos, const Size &rSize);

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements (void);

    void            HidePage();

    virtual sal_Bool    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual void Resize (void);

    void            ShowMousePosInfo(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void    AddWindow(::sd::Window* pWin);
    virtual void    RemoveWindow(::sd::Window* pWin);

    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive);

    virtual void    SetZoom( long nZoom );
    virtual void    SetZoomRect( const Rectangle& rZoomRect );

    void            InsertURLField(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                   const Point* pPos);
    void            InsertURLButton(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                    const Point* pPos);

    virtual void    SetUIUnit(FieldUnit eUnit);

    void            SelectionHasChanged();
    void            ModelHasChanged();
    virtual void    Activate(sal_Bool bIsMDIActivate);
    virtual void    Deactivate(sal_Bool IsMDIActivate);
    virtual void    UIActivating( SfxInPlaceClient* );
    virtual void    UIDeactivated( SfxInPlaceClient* );
    virtual OUString GetSelectionText( bool bCompleteWords = false );
    virtual sal_Bool    HasSelection( sal_Bool bText = sal_True ) const;

    void            ExecCtrl(SfxRequest& rReq);
    void            GetCtrlState(SfxItemSet& rSet);
    void            GetDrawAttrState(SfxItemSet& rSet);
    void            GetMenuState(SfxItemSet& rSet);
    void            GetTableMenuState(SfxItemSet& rSet);
    /** Set the items of the given item set that are related to
        switching the editing mode to the correct values.
        <p>This function also sets the states of the mode buttons
        (those at the upper right corner) accordingly.</p>
    */
    void GetModeSwitchingMenuState (SfxItemSet &rSet);
    void            GetAttrState(SfxItemSet& rSet);
    void            GetSnapItemState(SfxItemSet& rSet);

    void            GetState (SfxItemSet& rSet);
    void            Execute (SfxRequest& rReq);

    void            ExecStatusBar(SfxRequest& rReq);
    void            GetStatusBarState(SfxItemSet& rSet);

    void            ExecOptionsBar(SfxRequest& rReq);
    void            GetOptionsBarState(SfxItemSet& rSet);

    void            ExecRuler(SfxRequest& rReq);
    void            GetRulerState(SfxItemSet& rSet);

    void            ExecFormText(SfxRequest& rReq);
    void            GetFormTextState(SfxItemSet& rSet);

    void            ExecAnimationWin(SfxRequest& rReq);
    void            GetAnimationWinState(SfxItemSet& rSet);

    void            ExecNavigatorWin(SfxRequest& rReq);
    void            GetNavigatorWinState(SfxItemSet& rSet);

    void         ExecutePropPanelAttr (SfxRequest& rReq);
    void            GetStatePropPanelAttr(SfxItemSet& rSet);

    void            ExecEffectWin(SfxRequest& rReq);

    void            Update3DWindow();
    void            AssignFrom3DWindow();

    void            ExecGallery(SfxRequest& rReq);
    void            GetGalleryState(SfxItemSet& rSet);

    void            ExecBmpMask( SfxRequest& rReq );
    void            GetBmpMaskState( SfxItemSet& rSet );

    void            ExecIMap( SfxRequest& rReq );
    void            GetIMapState( SfxItemSet& rSet );

    void            FuTemporary(SfxRequest& rReq);
    void            FuPermanent(SfxRequest& rReq);
    void            FuSupport(SfxRequest& rReq);
    void            FuSupportRotate(SfxRequest& rReq);
    void            FuTable(SfxRequest& rReq);

    void            AttrExec (SfxRequest& rReq);
    void            AttrState (SfxItemSet& rSet);

    void        ExecChar(SfxRequest& rReq);

    void            ExecuteAnnotation (SfxRequest& rRequest);
    void            GetAnnotationState (SfxItemSet& rItemSet);

    void StartRulerDrag (
        const Ruler& rRuler,
        const MouseEvent& rMEvt);

    virtual sal_uInt16  PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );

    PageKind        GetPageKind() { return mePageKind; }

    Point           GetMousePos() { return maMousePos; }
    sal_Bool            IsMousePosFreezed() { return mbMousePosFreezed; }
    void            SetMousePosFreezed( sal_Bool bIn ) { mbMousePosFreezed = bIn; }

    EditMode        GetEditMode() const { return meEditMode; }
    virtual SdPage* GetActualPage() { return mpActualPage; }

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const;

    void            ResetActualPage();
    void            ResetActualLayer();
    sal_Bool            SwitchPage(sal_uInt16 nPage);
    sal_Bool            IsSwitchPageAllowed() const;

    sal_Bool            GotoBookmark(const OUString& rBookmark);
    void            MakeVisible(const Rectangle& rRect, ::Window& rWin);

    virtual void    ReadFrameViewData(FrameView* pView);
    virtual void    WriteFrameViewData();

    virtual ErrCode DoVerb(long nVerb);
    virtual sal_Bool    ActivateObject(SdrOle2Obj* pObj, long nVerb);

    void            SetZoomOnPage( sal_Bool bZoom = sal_True ) { mbZoomOnPage = bZoom; }
    sal_Bool            IsZoomOnPage() { return mbZoomOnPage; }
    void            CheckLineTo (SfxRequest& rReq);
    void            SetChildWindowState( SfxItemSet& rSet );

    void            UpdateIMapDlg( SdrObject* pObj );

    void            LockInput();
    void            UnlockInput();
    sal_Bool            IsInputLocked() const { return mnLockCount > 0UL; }

    sal_uInt16          GetCurPageId() { return( maTabControl.GetCurPageId() ); }

    /** Show controls of the UI or hide them, depending on the given flag.
        Do not call this method directly.  Call the method at ViewShellBase
        instead.
    */
    virtual void ShowUIControls (bool bVisible = true);

    void            ScannerEvent( const ::com::sun::star::lang::EventObject& rEventObject );

    bool IsLayerModeActive (void) const;

    sal_uInt16*         GetSlotArray() const { return mpSlotArray; }

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer );

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    virtual void    VisAreaChanged(const Rectangle& rRect);

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleDrawDocumentView</type> object.
   */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

    /** Return the number of layers managed by the layer tab control.  This
        will usually differ from the number of layers managed by the layer
        administrator.
        @return
            The number of layers managed by the layer tab control.  The
            returned value is independent of whether the layer modus is
            currently active and the tab control is visible.
    */
    virtual int GetTabLayerCount (void) const;

    /** Return the numerical id of the currently active layer as seen by the
        layer tab control.
        @return
            The returned id is a number between zero (inclusive) and the
            number of layers as returned by the
            <member>GetTabLayerCount</member> method (exclusive).
    */
    virtual int GetActiveTabLayerIndex (void) const;

    /** Set the active layer at the layer tab control and update the control
        accordingly to reflect the change on screen.
        @param nId
            The id is expected to be a number between zero (inclusive) and
            the number of layers as returned by the
            <member>GetTabLayerCount</member> method (exclusive).  Note that
            Invalid values are ignored.  No excpetion is thrown in that case.
    */
    virtual void SetActiveTabLayerIndex (int nId);

    /** Return a pointer to the tab control for pages.
    */
    TabControl* GetPageTabControl (void);

    /** Return a pointer to the tab control for layers.
    */
    LayerTabBar* GetLayerTabControl (void);

    /** Renames the given slide using an SvxNameDialog

        @param nPageId the index of the page in the SdTabControl.
        @param rName the new name of the slide.

        @return false, if the new name is invalid for some reason.

        <p>Implemented in <code>drviews8.cxx</code>.</p>
     */
    bool RenameSlide( sal_uInt16 nPageId, const OUString & rName );

    /** modifies the given layer with the given values */
    void ModifyLayer( SdrLayer* pLayer, const OUString& rLayerName, const OUString& rLayerTitle, const OUString& rLayerDesc, bool bIsVisible, bool bIsLocked, bool bIsPrintable );

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void);

    DrawView*   GetDrawView() const { return mpDrawView; }

    /** Relocation to a new parent window is not supported for DrawViewShell
        objects so this method always returns <FALSE/>.
    */
    virtual bool RelocateToParentWindow (::Window* pParentWindow);

    ::rtl::OUString GetSidebarContextName (void) const;

protected:
    DrawView*       mpDrawView;
    SdPage*         mpActualPage;
    Rectangle       maMarkRect;
    Point           maMousePos;
    sal_Bool            mbMousePosFreezed;
    TabControl      maTabControl;
    EditMode        meEditMode;
    PageKind        mePageKind;
    sal_Bool            mbZoomOnPage;
    sal_Bool            mbIsRulerDrag;
    sal_uLong           mnLockCount;
    Timer           maCloseTimer;
    sal_Bool            mbReadOnly;
    sal_uInt16*         mpSlotArray;

    static sal_Bool     mbPipette;

                    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
                    DECL_LINK( CloseHdl, Timer* pTimer );
                    DECL_LINK( TabSplitHdl, TabBar * );
                    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );
                    DECL_LINK( RenameSlideHdl, AbstractSvxNameDialog* );

    void            DeleteActualPage();
    void            DeleteActualLayer();

    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, sal_Bool bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);
    virtual void    UpdateHRuler();
    virtual void    UpdateVRuler();
    virtual void    SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY);

    void            SetupPage( Size &rSize, long nLeft, long nRight, long nUpper, long nLower,
                               sal_Bool bSize, sal_Bool bMargin, sal_Bool bScaleAll );

    sal_uInt16          GetIdBySubId( sal_uInt16 nSId );
    void            MapSlot( sal_uInt16 nSId );
    void            UpdateToolboxImages( SfxItemSet &rSet, sal_Bool bPermanent = sal_True );
    sal_uInt16          GetMappedSlot( sal_uInt16 nSId );
    sal_uInt16          GetArrayId( sal_uInt16 nSId );

    void            GetMenuStateSel(SfxItemSet& rSet);

private:
    void ShowSlideShow(SfxRequest& rReq);
    /** This flag controls whether the layer mode is active, i.e. the layer
        dialog is visible.
    */
    bool mbIsLayerModeActive;

    /** This item contains the clipboard formats of the current clipboard
        content that are supported both by that content and by the
        DrawViewShell.
    */
    ::std::auto_ptr<SvxClipboardFmtItem> mpCurrentClipboardFormats;

    /** On some occasions it is necessary to make SwitchPage calls
        asynchronously.
    */
    tools::AsynchronousCall maAsynchronousSwitchPageCall;

    /** This flag is used to prevent nested calls to SwitchPage().
    */
    bool mbIsInSwitchPage;

    RotateTransliteration m_aRotateCase;

    /** Listen for selection changes and broadcast context changes for the sidebar.
    */
    ::rtl::Reference<svx::sidebar::SelectionChangeHandler> mpSelectionChangeHandler;

    void Construct (DrawDocShell* pDocSh, PageKind ePageKind);

    /** Depending on the given request create a new page or duplicate an
        existing one.  See ViewShell::CreateOrDuplicatePage() for more
        information.
    */
    virtual SdPage* CreateOrDuplicatePage (
        SfxRequest& rRequest,
        PageKind ePageKind,
        SdPage* pPage,
        const sal_Int32 nInsertPosition = -1);

    ::com::sun::star::uno::Reference< ::com::sun::star::scanner::XScannerManager2 > mxScannerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >      mxScannerListener;
    TransferableClipboardListener*                                                  mpClipEvtLstnr;
    sal_Bool                                                                            mbPastePossible;

    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint);

    /** Stop a running slide show.  The frame the show is running in is
        destroyed if
        a) it is running in its own frame, i.e. is a full screen show and
        b) the given flag bCloseFrame is true.
        @param bCloseFrame
            Be carefull with this flag when stopping a full screen show.
            When called from the destructor the flag has to be <FALSE/> or
            otherwise we run into a loop of calls to destructors of the view
            and the frame.
            When called from other places the flag should be <TRUE/> so that
            not an empty frame remains. When called with <TRUE/> it is the
            responsibility of the caller to avoid an illegal reentrant
            call.
    */
    void StopSlideShow (bool bCloseFrame);

    /** Show the context menu for snap lines and points.  Because snap lines
        can not be selected the index of the snap line/point for which the
        popup menu is opened has to be passed to the processing slot
        handlers.  This can be done only by manually showing the popup menu.
        @param rPageView
            The page view is used to access the help lines.
        @param nSnapLineIndex
            Index of the snap line or snap point for which to show the
            context menu.
        @param rMouseLocation
            The mouse location defines the location at which to display the
            context menu.
    */
    void ShowSnapLineContextMenu (
        SdrPageView& rPageView,
        const sal_uInt16 nSnapLineIndex,
        const Point& rMouseLocation);

    using ViewShell::Notify;

    ::std::auto_ptr< AnnotationManager > mpAnnotationManager;
    ::std::auto_ptr< ViewOverlayManager > mpViewOverlayManager;
};


} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
