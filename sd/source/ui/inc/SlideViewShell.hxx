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

#ifndef SD_SLIDE_VIEW_SHELL_HXX
#define SD_SLIDE_VIEW_SHELL_HXX


#include "ViewShell.hxx"
#include "SlideView.hxx"
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>



class SdPage;



namespace sd {

class SdUnoSlideView;
class Window;

/** Show an overview over the slides in an Impress document and allow
    some high level editing i.e. editing of the order in a show, not
    the contents of the slides.
*/
class SlideViewShell
    : public ViewShell
{
public:

    TYPEINFO();

    SFX_DECL_VIEWFACTORY(SlideViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDSLIDEVIEWSHELL)

    /** Create a new view shell for the slide view.
        @param rViewShellBase
            The new object will be stacked on this view shell base.
        @param pFrameView
            The frame view that makes it possible to pass information from
            one view shell to the next.
    */
    SlideViewShell(SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL);

    SlideViewShell(SfxViewFrame *pFrame,
        ::Window* pParentWindow,
        const SlideViewShell& rShell);

    virtual ~SlideViewShell (void);

    virtual void    Paint(const Rectangle& rRect, ::sd::Window* pWin);

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements (void);
    virtual void    AddWindow(::sd::Window* pWin) { pSlideView->AddWindowToPaintView((OutputDevice*) pWin); }
    virtual void    RemoveWindow(::sd::Window* pWin) { pSlideView->DeleteWindowFromPaintView((OutputDevice*) pWin); }

    virtual sal_Bool    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual SdPage* GetActualPage();

    /** @returns
            current or selected page or 0.
    */
    virtual SdPage* getCurrentPage() const;

    void            ExecCtrl(SfxRequest &rReq);
    void            GetCtrlState(SfxItemSet &rSet);
    void            GetMenuState(SfxItemSet &rSet);
    void            GetAttrState(SfxItemSet &rSet);

    void            SetPagesPerRow( sal_uInt16 nPagesPerRow );

    void            ExecStatusBar(SfxRequest& rReq);
    void            GetStatusBarState(SfxItemSet& rSet);

    void            FuTemporary(SfxRequest &rReq);
    void            FuPermanent(SfxRequest &rReq);
    void            FuSupport(SfxRequest &rReq);

    virtual void    ReadFrameViewData(FrameView* pView);
    virtual void    WriteFrameViewData();

    virtual void    SetZoom(long nZoom);
    virtual void    SetZoomRect(const Rectangle& rZoomRect);

    virtual sal_Bool    HasSelection( sal_Bool bText = sal_True ) const;

    /** Draw the rectangle arround the specified slide that indicates whether
        the slide is selected or not.  When not selected the rectangle is
        painted in the background color (WindowColor from the style settings)
        and is therefore not visible.  A selected slide is painted with the
        WindowTextColor from the style settings.  Painting takes place in
        all associated windows.  The line- and fill color of the windows are
        restored to their original values after the rectangle is painted.
        @param nPage
            When the page number is invalid then the call is ignored.
    */
    void            DrawSelectionRect( sal_uInt16 nPage );
    void            DrawFocusRect( sal_uInt16 nPage );

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    virtual void    VisAreaChanged(const Rectangle& rRect);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> CreateAccessibleDocumentView( ::sd::Window* pWindow );

    void            SelectionHasChanged();
    void            PageLayoutHasChanged();
    void            FocusHasChanged( sal_uInt16 nOldFocusPage, sal_uInt16 nNewFocusPage );
    void            PageVisibilityHasChanged( sal_uInt16 nPage, sal_Bool bVisible );

    /** On activation the preview is turned off.
    */
    virtual void Activate (sal_Bool IsMDIActivate);

protected:
    virtual Size    GetOptimalSizePixel() const;
    virtual long    VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long    VirtVScrollHdl(ScrollBar* pVHScroll);


private:
    SlideView* pSlideView;
    Point           aDisplayPos;
    Size            aDisplaySize;
    sal_uInt16          nCurFocusPage;
    bool            bSetInitialZoomFactor;
    bool            bInitializeWinPos;

    void            Construct(SdDrawDocument* pDoc);
    void            ImplDrawFocusRect( sal_uInt16 nPage, sal_Bool bVisible );
};

} // end of namespace sd

#endif
