/*************************************************************************
 *
 *  $RCSfile: DrawViewShell.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:54:43 $
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

#ifndef SD_DRAW_VIEW_SHELL_HXX
#define SD_DRAW_VIEW_SHELL_HXX

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif

#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif

#ifndef SD_TAB_CONTROL_HXX
#include "TabControl.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif

class SdPage;
class DrawDocShell;
class SdAnimationWin;
class SdRuler;
class TabBar;
class SdrObject;
class XPolygon;
class SdrPageView;
class TransferableDataHelper;
class TransferableClipboardListener;
class AbstractSvxNameDialog; //CHINA001 class SvxNameDialog;
class SdrLayer;

namespace sd {

class DrawView;
class LayerTabBar;
class Ruler;
class SdUnoDrawView;

#define CHECK_RANGE(nMin, nValue, nMax) ((nValue >= nMin) && (nValue <= nMax))

/** Base class of the stacked shells that provide graphical views to
    Draw and Impress documents and editing functionality.  In contrast
    to this other stacked shells are responsible for showing an
    overview over several slides (SlideViewShell) or a textual
    overview over the text in an Impress document (OutlineViewShell).
*/
class DrawViewShell
    : public ViewShell,
      public SfxListener
{
public:
    static const int SLOTARRAY_COUNT = 24;

    TYPEINFO();

    SFX_DECL_INTERFACE(SD_IF_SDDRAWVIEWSHELL);

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

    DrawViewShell(
        SfxViewFrame* pFrame,
        ::Window* pParentWindow,
        const DrawViewShell& rShell);

    virtual ~DrawViewShell (void);

    virtual void Init (void);

    virtual void    Paint(const Rectangle& rRect, ::sd::Window* pWin);

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

    void            HidePage(SdrPageView* pPV);

    virtual BOOL    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual void Resize (const Point& rPos, const Size& rSize);
    //  virtual void    OuterResizePixel(const Point& rPos, const Size& rSize);
    //  virtual void    InnerResizePixel(const Point& rPos, const Size& rSize);

    void            ShowMousePosInfo(const Rectangle& rRect, ::sd::Window* pWin);

    virtual void    AddWindow(::sd::Window* pWin);
    virtual void    RemoveWindow(::sd::Window* pWin);

    virtual void ChangeEditMode (EditMode eMode, bool bIsLayerModeActive);

    virtual void    SetZoom( long nZoom );
    virtual void    SetZoomRect( const Rectangle& rZoomRect );

    void            InsertURLField(const String& rURL, const String& rText, const String& rTarget,
                                   const Point* pPos);
    void            InsertURLButton(const String& rURL, const String& rText, const String& rTarget,
                                    const Point* pPos);

    virtual void    SetUIUnit(FieldUnit eUnit);

    void            SelectionHasChanged();
    void            ModelHasChanged();
    virtual void    Activate(BOOL bIsMDIActivate);
    virtual void    Deactivate(BOOL IsMDIActivate);
    virtual void    UIActivate( SvInPlaceObject* );
    virtual void    UIDeactivate( SvInPlaceObject* );
    virtual String  GetSelectionText( BOOL bCompleteWords = FALSE );
    virtual BOOL    HasSelection( BOOL bText = TRUE ) const;
    void            SetHelpIdBySelection();

    void            ExecCtrl(SfxRequest& rReq);
    void            GetCtrlState(SfxItemSet& rSet);
    void            GetMenuState(SfxItemSet& rSet);
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

    void            ExecObjPalette(SfxRequest& rReq);
    void            GetObjPaletteState(SfxItemSet& rSet);

    void            ExecAnimationWin(SfxRequest& rReq);
    void            GetAnimationWinState(SfxItemSet& rSet);

    void            ExecNavigatorWin(SfxRequest& rReq);
    void            GetNavigatorWinState(SfxItemSet& rSet);

    void            ExecEffectWin(SfxRequest& rReq);
    void            GetEffectWinState(SfxItemSet& rSet);
    void            UpdateEffectWindow();
    void            AssignFromEffectWindow();

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

    void            AttrExec (SfxRequest& rReq);
    void            AttrState (SfxItemSet& rSet);

    void StartRulerDrag (
        const Ruler& rRuler,
        const MouseEvent& rMEvt);

    virtual USHORT  PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );

    PageKind        GetPageKind() { return ePageKind; }

    Point           GetMousePos() { return aMousePos; }
    BOOL            IsMousePosFreezed() { return bMousePosFreezed; }
    void            SetMousePosFreezed( BOOL bIn ) { bMousePosFreezed = bIn; }

    EditMode        GetEditMode() const { return eEditMode; }
    virtual SdPage* GetActualPage() { return pActualPage; }

    void            ResetActualPage();
    void            ResetActualLayer();
    BOOL            SwitchPage(USHORT nPage);
    BOOL            IsSwitchPageAllowed() const;

    BOOL            GotoBookmark(const String& rBookmark);
    void            MakeVisible(const Rectangle& rRect, ::Window& rWin);

    virtual void    ReadFrameViewData(FrameView* pView);
    virtual void    WriteFrameViewData();

    virtual ErrCode DoVerb(long nVerb);
    virtual BOOL    ActivateObject(SdrOle2Obj* pObj, long nVerb);

    void            SetZoomOnPage( BOOL bZoom = TRUE ) { bZoomOnPage = bZoom; }
    BOOL            IsZoomOnPage() { return bZoomOnPage; }
    void            CheckLineTo (SfxRequest& rReq);
    void            FuTemp01(SfxRequest& rReq);
    void            FuTemp02(SfxRequest& rReq);
    void            FuTemp03(SfxRequest& rReq);
    void            FuTemp04(SfxRequest& rReq);
    void            SetChildWindowState( SfxItemSet& rSet );

    void            UpdateIMapDlg( SdrObject* pObj );

    void            LockInput();
    void            UnlockInput();
    BOOL            IsInputLocked() const { return nLockCount > 0UL; }

    USHORT          GetCurPageId() { return( aTabControl.GetCurPageId() ); }

    /** Show controls of the UI or hide them, depending on the given flag.
        Do not call this method directly.  Call the method at ViewShellBase
        instead.
    */
    virtual void ShowUIControls (bool bVisible = true);

    void            ScannerEvent( const ::com::sun::star::lang::EventObject& rEventObject );

    //af    SdUnoDrawView*  GetController() const { return pController; }
    bool IsLayerModeActive (void) const;

    USHORT*         GetSlotArray() const { return pSlotArray; }

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                    ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer );

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
    bool RenameSlide( USHORT nPageId, const String & rName );

    /** modifies the given layer with the given values */
    void ModifyLayer( SdrLayer* pLayer, String& rLayerName, bool bIsVisible, bool bIsLocked, bool bIsPrintable );

    virtual DrawController* GetController (void);

protected:
    DrawView* pDrView;
    SdPage*         pActualPage;
    UINT16          nLastSlot;
    Rectangle       aMarkRect;
    Point           aMousePos;
    BOOL            bMousePosFreezed;
    TabControl aTabControl;
    EditMode        eEditMode;
    PageKind        ePageKind;
    BOOL            bZoomOnPage;
    BOOL            bIsRulerDrag;
    XPolygon*       pXPolygon;
    USHORT          nPolygonIndex;
    BOOL            bLineError;
    BOOL            bLastWasLineTo;
    BOOL            bLastWasMoveTo;
    BOOL            bLastWasBezierTo;
    Color           aGradStartColor;
    Color           aGradEndColor;
    Color           aHatchColor;
    ULONG           nLockCount;
    Timer           aCloseTimer;
    BOOL            bReadOnly;
    BOOL            bInEffectAssignment;
    USHORT*         pSlotArray;

    static BOOL     bPipette;


                    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
                    DECL_LINK( CloseHdl, Timer* pTimer );
                    DECL_LINK( TabSplitHdl, TabBar * );
                    DECL_LINK( NameObjectHdl, AbstractSvxNameDialog* );
                    DECL_LINK( RenameSlideHdl, AbstractSvxNameDialog* );

    void            DeleteActualPage();
    void            DeleteActualLayer();

    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, BOOL bIsFirst);
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin);
    virtual void    UpdateHRuler();
    virtual void    UpdateVRuler();
    virtual long    GetHCtrlWidth();
    virtual void    SetZoomFactor(const Fraction& rZoomX, const Fraction& rZoomY);
    virtual Size    GetOptimalSizePixel() const;

    void            DestroyPolygons();

    void            SetupPage( Size &rSize, long nLeft, long nRight, long nUpper, long nLower,
                               BOOL bSize, BOOL bMargin, BOOL bScaleAll );

    USHORT          GetIdBySubId( USHORT nSId );
    void            MapSlot( USHORT nSId );
    void            UpdateToolboxImages( SfxItemSet &rSet, BOOL bPermanent = TRUE );
    USHORT          GetMappedSlot( USHORT nSId );
    USHORT          GetArrayId( USHORT nSId );

    void            GetMenuStateSel(SfxItemSet& rSet);

private:
    /** This flag controls whether the layer mode is active, i.e. the layer
        dialog is visible.
    */
    bool mbIsLayerModeActive;

    void Construct (DrawDocShell* pDocSh, PageKind ePageKind);

    /** Depending on the given request create a new page or duplicate an
        existing one.  See ViewShell::CreateOrDuplicatePage() for more
        information.
    */
    virtual void CreateOrDuplicatePage (
        SfxRequest& rRequest,
        PageKind ePageKind,
        SdPage* pPage);

    ::com::sun::star::uno::Reference< ::com::sun::star::scanner::XScannerManager >  mxScannerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >      mxScannerListener;
    TransferableClipboardListener*                                                  pClipEvtLstnr;
    BOOL                                                                            bPastePossible;

    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint);
};


} // end of namespace sd

#endif
