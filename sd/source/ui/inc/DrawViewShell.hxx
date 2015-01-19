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
class SdrExternalToolEdit;
class DrawDocShell;
class TabBar;
class SdrObject;
class SdrPageView;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractSvxNameDialog;
class SdrLayer;
class SvxClipboardFmtItem;
struct ESelection;

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

    TYPEINFO_OVERRIDE();
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
        ::vcl::Window* pParentWindow,
        PageKind ePageKind = PK_STANDARD,
        FrameView* pFrameView = NULL);

    virtual ~DrawViewShell (void);

    virtual void Init (bool bIsMainViewShell) SAL_OVERRIDE;

    virtual void Shutdown (void) SAL_OVERRIDE;

    void PrePaint() SAL_OVERRIDE;
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin) SAL_OVERRIDE;

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
    virtual void ArrangeGUIElements (void) SAL_OVERRIDE;

    void            HidePage();

    virtual bool    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin) SAL_OVERRIDE;
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin) SAL_OVERRIDE;
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin) SAL_OVERRIDE;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin) SAL_OVERRIDE;
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin) SAL_OVERRIDE;

    virtual void Resize (void) SAL_OVERRIDE;

    void            ShowMousePosInfo(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void    AddWindow(::sd::Window* pWin);
    virtual void    RemoveWindow(::sd::Window* pWin);

    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive);

    virtual void    SetZoom( long nZoom ) SAL_OVERRIDE;
    virtual void    SetZoomRect( const Rectangle& rZoomRect ) SAL_OVERRIDE;

    void            InsertURLField(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                   const Point* pPos);
    void            InsertURLButton(const OUString& rURL, const OUString& rText, const OUString& rTarget,
                                    const Point* pPos);

    virtual void    SetUIUnit(FieldUnit eUnit) SAL_OVERRIDE;

    void            SelectionHasChanged();
    void            ModelHasChanged();
    virtual void    Activate(bool bIsMDIActivate) SAL_OVERRIDE;
    virtual void    Deactivate(bool IsMDIActivate) SAL_OVERRIDE;
    virtual void    UIActivating( SfxInPlaceClient* ) SAL_OVERRIDE;
    virtual void    UIDeactivated( SfxInPlaceClient* ) SAL_OVERRIDE;
    virtual OUString GetSelectionText( bool bCompleteWords = false );
    virtual bool    HasSelection( bool bText = true ) const;

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

    void        ExecChar(SfxRequest& rReq);

    void            ExecuteAnnotation (SfxRequest& rRequest);
    void            GetAnnotationState (SfxItemSet& rItemSet);

    void StartRulerDrag (
        const Ruler& rRuler,
        const MouseEvent& rMEvt);

    virtual bool    PrepareClose( bool bUI = true ) SAL_OVERRIDE;

    PageKind        GetPageKind() { return mePageKind; }
    void            SetPageKind( PageKind ePageKind ) { mePageKind = ePageKind; }
    Point           GetMousePos() { return maMousePos; }
    bool            IsMousePosFreezed() { return mbMousePosFreezed; }
    void            SetMousePosFreezed( bool bIn ) { mbMousePosFreezed = bIn; }

    EditMode        GetEditMode() const { return meEditMode; }
    virtual SdPage* GetActualPage() SAL_OVERRIDE { return mpActualPage; }

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const SAL_OVERRIDE;

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
    void            MakeVisible(const Rectangle& rRect, ::vcl::Window& rWin);

    virtual void    ReadFrameViewData(FrameView* pView) SAL_OVERRIDE;
    virtual void    WriteFrameViewData() SAL_OVERRIDE;

    virtual ErrCode DoVerb(long nVerb) SAL_OVERRIDE;
    virtual bool    ActivateObject(SdrOle2Obj* pObj, long nVerb) SAL_OVERRIDE;

    void            SetZoomOnPage( bool bZoom = true ) { mbZoomOnPage = bZoom; }
    bool            IsZoomOnPage() { return mbZoomOnPage; }
    void            CheckLineTo (SfxRequest& rReq);
    void            SetChildWindowState( SfxItemSet& rSet );

    void            UpdateIMapDlg( SdrObject* pObj );

    void            LockInput();
    void            UnlockInput();
    bool            IsInputLocked() const { return mnLockCount > 0UL; }

    sal_uInt16          GetCurPageId() { return( maTabControl.GetCurPageId() ); }

    /** Show controls of the UI or hide them, depending on the given flag.
        Do not call this method directly.  Call the method at ViewShellBase
        instead.
    */
    virtual void ShowUIControls (bool bVisible = true) SAL_OVERRIDE;

    void            ScannerEvent( const ::com::sun::star::lang::EventObject& rEventObject );

    bool IsLayerModeActive (void) const { return mbIsLayerModeActive;}

    sal_uInt16*         GetSlotArray() const { return mpSlotArray; }

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer ) SAL_OVERRIDE;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, sal_uInt16 nPage, sal_uInt16 nLayer ) SAL_OVERRIDE;

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, bool bBrowse = false ) SAL_OVERRIDE;
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, bool bBrowse = false ) SAL_OVERRIDE;

    virtual void    VisAreaChanged(const Rectangle& rRect) SAL_OVERRIDE;

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleDrawDocumentView</type> object.
   */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) SAL_OVERRIDE;

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
    TabControl& GetPageTabControl (void) { return maTabControl;}

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

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void) SAL_OVERRIDE;

    DrawView*   GetDrawView() const { return mpDrawView; }

    /** Relocation to a new parent window is not supported for DrawViewShell
        objects so this method always returns <FALSE/>.
    */
    virtual bool RelocateToParentWindow (::vcl::Window* pParentWindow) SAL_OVERRIDE;

    OUString GetSidebarContextName (void) const;

    //move this method to ViewShell.
    //void  NotifyAccUpdate();
protected:
    DrawView*       mpDrawView;
    SdPage*         mpActualPage;
    Rectangle       maMarkRect;
    Point           maMousePos;
    bool            mbMousePosFreezed;
    TabControl      maTabControl;
    EditMode        meEditMode;
    PageKind        mePageKind;
    bool            mbZoomOnPage;
    bool            mbIsRulerDrag;
    sal_uLong           mnLockCount;
    bool            mbReadOnly;
    sal_uInt16*         mpSlotArray;

    static bool     mbPipette;

                    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
                    DECL_LINK( TabSplitHdl, TabBar * );
                    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );
                    DECL_LINK( RenameSlideHdl, AbstractSvxNameDialog* );

    void            DeleteActualPage();
    void            DeleteActualLayer();

    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, bool bIsFirst) SAL_OVERRIDE;
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin) SAL_OVERRIDE;
    virtual void    UpdateHRuler() SAL_OVERRIDE;
    virtual void    UpdateVRuler() SAL_OVERRIDE;
    virtual void    SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY) SAL_OVERRIDE;

    void            SetupPage( Size &rSize, long nLeft, long nRight, long nUpper, long nLower,
                               bool bSize, bool bMargin, bool bScaleAll );

    sal_uInt16      GetIdBySubId( sal_uInt16 nSId );
    void            MapSlot( sal_uInt16 nSId );
    void            UpdateToolboxImages( SfxItemSet &rSet, bool bPermanent = true );
    sal_uInt16      GetMappedSlot( sal_uInt16 nSId );
    sal_uInt16      GetArrayId( sal_uInt16 nSId );

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
    ::std::unique_ptr<SvxClipboardFmtItem> mpCurrentClipboardFormats;

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
        const sal_Int32 nInsertPosition = -1) SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< ::com::sun::star::scanner::XScannerManager2 > mxScannerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >      mxScannerListener;
    TransferableClipboardListener*                                                  mpClipEvtLstnr;
    bool                                                                            mbPastePossible;

    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint) SAL_OVERRIDE;

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
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
