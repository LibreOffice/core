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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DRAWVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DRAWVIEWSHELL_HXX

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

class Outliner;
class SdPage;
class SdStyleSheet;
class SdrExternalToolEdit;
class DrawDocShell;
class TabBar;
class SdrObject;
class SdrPageView;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractSvxNameDialog;
class SdrLayer;
class SvxClipboardFormatItem;
struct ESelection;
class AbstractSvxObjectNameDialog;

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
    SFX_DECL_INTERFACE(SD_IF_SDDRAWVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
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
        vcl::Window* pParentWindow,
        PageKind ePageKind = PK_STANDARD,
        FrameView* pFrameView = nullptr);

    virtual ~DrawViewShell();

    virtual void Init (bool bIsMainViewShell) override;

    virtual void Shutdown() override;

    void PrePaint() override;
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin) override;

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
    virtual void ArrangeGUIElements() override;

    void            HidePage();

    virtual bool    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin) override;
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin) override;
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin) override;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin) override;
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin) override;

    virtual void Resize() override;

    void            ShowMousePosInfo(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive);

    virtual void    SetZoom( long nZoom ) override;
    virtual void    SetZoomRect( const Rectangle& rZoomRect ) override;

    void            InsertURLField(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                   const Point* pPos);
    void            InsertURLButton(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                    const Point* pPos);

    virtual void    SetUIUnit(FieldUnit eUnit) override;

    void            SelectionHasChanged();
    void            ModelHasChanged();
    virtual void    Activate(bool bIsMDIActivate) override;
    virtual void    Deactivate(bool IsMDIActivate) override;
    virtual void    UIActivating( SfxInPlaceClient* ) override;
    virtual void    UIDeactivated( SfxInPlaceClient* ) override;
    OUString GetSelectionText( bool bCompleteWords = false );
    bool    HasSelection( bool bText = true ) const;

    //If we are editing an PRESOBJ_OUTLINE return the Outliner and fill rSel
    //with the current selection
    ::Outliner*     GetOutlinerForMasterPageOutlineTextObj(ESelection &rSel);

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

    void            ExecBmpMask( SfxRequest& rReq );
    void            GetBmpMaskState( SfxItemSet& rSet );

    void            ExecIMap( SfxRequest& rReq );
    void            GetIMapState( SfxItemSet& rSet );

    void            FuTemporary(SfxRequest& rReq);
    void            FuPermanent(SfxRequest& rReq);
    void            FuSupport(SfxRequest& rReq);
    void            FuDeleteSelectedObjects();
    void            FuSupportRotate(SfxRequest& rReq);
    void            FuTable(SfxRequest& rReq);

    void            AttrExec (SfxRequest& rReq);
    void            AttrState (SfxItemSet& rSet);

    void            ExecGoToNextPage (SfxRequest& rReq);
    void            GetStateGoToNextPage (SfxItemSet& rSet);

    void            ExecGoToPreviousPage (SfxRequest& rReq);
    void            GetStateGoToPreviousPage (SfxItemSet& rSet);

    void            ExecGoToFirstPage (SfxRequest& rReq);
    void            GetStateGoToFirstPage (SfxItemSet& rSet);

    void            ExecGoToLastPage (SfxRequest& rReq);
    void            GetStateGoToLastPage (SfxItemSet& rSet);

    SD_DLLPUBLIC void ExecChar(SfxRequest& rReq);

    void            ExecuteAnnotation (SfxRequest& rRequest);
    void            GetAnnotationState (SfxItemSet& rItemSet);

    void StartRulerDrag (
        const Ruler& rRuler,
        const MouseEvent& rMEvt);

    virtual bool    PrepareClose( bool bUI = true ) override;

    PageKind        GetPageKind() { return mePageKind; }
    void            SetPageKind( PageKind ePageKind ) { mePageKind = ePageKind; }
    Point           GetMousePos() { return maMousePos; }
    void            SetMousePosFreezed( bool bIn ) { mbMousePosFreezed = bIn; }

    EditMode        GetEditMode() const { return meEditMode; }
    virtual SdPage* GetActualPage() override { return mpActualPage; }

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const override;

    void            ResetActualPage();
    void            ResetActualLayer();
    bool            SwitchPage(sal_uInt16 nPage);
    bool            IsSwitchPageAllowed() const;

    bool            GotoBookmark(const OUString& rBookmark);
    //Realize multi-selection of objects, If object is marked, the
    //corresponding entry is set true, else the corresponding entry is set
    //false.
    void            FreshNavigatrEntry();
    void            FreshNavigatrTree();
    void            MakeVisible(const Rectangle& rRect, vcl::Window& rWin);

    virtual void    ReadFrameViewData(FrameView* pView) override;
    virtual void    WriteFrameViewData() override;

    virtual ErrCode DoVerb(long nVerb) override;
    virtual bool    ActivateObject(SdrOle2Obj* pObj, long nVerb) override;

    void            SetZoomOnPage( bool bZoom = true ) { mbZoomOnPage = bZoom; }
    bool            IsZoomOnPage() { return mbZoomOnPage; }
    static void     CheckLineTo (SfxRequest& rReq);
    void            SetChildWindowState( SfxItemSet& rSet );

    void            UpdateIMapDlg( SdrObject* pObj );

    void            LockInput();
    void            UnlockInput();
    bool            IsInputLocked() const { return mnLockCount > 0UL; }

    sal_uInt16          GetCurPageId() { return maTabControl->GetCurPageId(); }

    /** Show controls of the UI or hide them, depending on the given flag.
        Do not call this method directly.  Call the method at ViewShellBase
        instead.
    */
    virtual void ShowUIControls (bool bVisible = true) override;

    void            ScannerEvent( const css::lang::EventObject& rEventObject );

    bool IsLayerModeActive() const { return mbIsLayerModeActive;}

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer ) override;

    virtual void    WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse = false ) override;
    virtual void    ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse = false ) override;

    virtual void    VisAreaChanged(const Rectangle& rRect) override;

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleDrawDocumentView</type> object.
   */
    virtual css::uno::Reference<css::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) override;

    /** Return the number of layers managed by the layer tab control.  This
        will usually differ from the number of layers managed by the layer
        administrator.
        @return
            The number of layers managed by the layer tab control.  The
            returned value is independent of whether the layer modus is
            currently active and the tab control is visible.
    */
    int GetTabLayerCount() const;

    /** Return the numerical id of the currently active layer as seen by the
        layer tab control.
        @return
            The returned id is a number between zero (inclusive) and the
            number of layers as returned by the
            <member>GetTabLayerCount</member> method (exclusive).
    */
    int GetActiveTabLayerIndex() const;

    /** Set the active layer at the layer tab control and update the control
        accordingly to reflect the change on screen.
        @param nId
            The id is expected to be a number between zero (inclusive) and
            the number of layers as returned by the
            <member>GetTabLayerCount</member> method (exclusive).  Note that
            Invalid values are ignored.  No excpetion is thrown in that case.
    */
    void SetActiveTabLayerIndex (int nId);

    /** Return a pointer to the tab control for pages.
    */
    TabControl& GetPageTabControl() { return *maTabControl.get(); }

    /** Return a pointer to the tab control for layers.
    */
    LayerTabBar* GetLayerTabControl();

    /** Renames the given slide using an SvxNameDialog

        @param nPageId the index of the page in the SdTabControl.
        @param rName the new name of the slide.

        @return false, if the new name is invalid for some reason.

        <p>Implemented in <code>drviews8.cxx</code>.</p>
     */
    bool RenameSlide( sal_uInt16 nPageId, const OUString & rName );

    /** modifies the given layer with the given values */
    void ModifyLayer( SdrLayer* pLayer, const OUString& rLayerName, const OUString& rLayerTitle, const OUString& rLayerDesc, bool bIsVisible, bool bIsLocked, bool bIsPrintable );

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() override;

    DrawView*   GetDrawView() const { return mpDrawView; }

    /** Relocation to a new parent window is not supported for DrawViewShell
        objects so this method always returns <FALSE/>.
    */
    virtual bool RelocateToParentWindow (vcl::Window* pParentWindow) override;

    OUString GetSidebarContextName() const;

    const Color& GetAppBackgroundColor() const { return mnAppBackgroundColor; }
    void SetAppBackgroundColor( Color nNewColor )  { mnAppBackgroundColor = nNewColor; }
    bool IsInSwitchPage() { return mbIsInSwitchPage; }

    //move this method to ViewShell.
    //void  NotifyAccUpdate();
protected:
    DrawView*           mpDrawView;
    SdPage*             mpActualPage;
    Rectangle           maMarkRect;
    Point               maMousePos;
    bool                mbMousePosFreezed;
    VclPtr<TabControl>  maTabControl;
    EditMode            meEditMode;
    PageKind            mePageKind;
    bool                mbZoomOnPage;
    bool                mbIsRulerDrag;
    sal_uLong           mnLockCount;
    bool                mbReadOnly;
    static bool         mbPipette;

                    DECL_LINK_TYPED( ClipboardChanged, TransferableDataHelper*, void );
                    DECL_LINK_TYPED( TabSplitHdl, TabBar *, void );
                    DECL_LINK_TYPED( NameObjectHdl, AbstractSvxObjectNameDialog&, bool );
                    DECL_LINK_TYPED( RenameSlideHdl, AbstractSvxNameDialog&, bool );

    void            DeleteActualPage();
    void            DeleteActualLayer();

    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, bool bIsFirst) override;
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin) override;
    virtual void    UpdateHRuler() override;
    virtual void    UpdateVRuler() override;
    virtual void    SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY) override;

    void            SetupPage( Size &rSize, long nLeft, long nRight, long nUpper, long nLower,
                               bool bSize, bool bMargin, bool bScaleAll );

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
    ::std::unique_ptr<SvxClipboardFormatItem> mpCurrentClipboardFormats;

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
        const sal_Int32 nInsertPosition = -1) override;

    css::uno::Reference< css::scanner::XScannerManager2 > mxScannerManager;
    css::uno::Reference< css::lang::XEventListener >      mxScannerListener;
    TransferableClipboardListener*                        mpClipEvtLstnr;
    bool                                                  mbPastePossible;

    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint) override;

    /** Stop a running slide show.  The frame the show is running in is
        destroyed if
        a) it is running in its own frame, i.e. is a full screen show and
        b) the given flag bCloseFrame is true.
        @param bCloseFrame
            Be careful with this flag when stopping a full screen show.
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

    ::std::unique_ptr< AnnotationManager > mpAnnotationManager;
    ::std::unique_ptr< ViewOverlayManager > mpViewOverlayManager;

    std::vector<std::unique_ptr<SdrExternalToolEdit>> m_ExternalEdits;

    // The colour of the area behind the slide (used to be called "Wiese")
    Color mnAppBackgroundColor;
};

    /// Merge the background properties together and deposit the result in rMergeAttr
    void MergePageBackgroundFilling(SdPage *pPage, SdStyleSheet *pStyleSheet, bool bMasterPage, SfxItemSet& rMergedAttr);

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
