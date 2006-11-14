/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideViewShell.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:33:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_SLIDE_VIEW_SHELL_HXX
#define SD_SLIDE_VIEW_SHELL_HXX


#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_HXX
#include "SlideView.hxx"
#endif

#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif



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
    SFX_DECL_INTERFACE(SD_IF_SDSLIDEVIEWSHELL);

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

    virtual BOOL    KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void    MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void    Command(const CommandEvent& rCEvt, ::sd::Window* pWin);

    virtual SdPage* GetActualPage();

    /** @returns
            current or selected page or 0.
    */
    virtual SdPage* getCurrentPage() const;

    /** Return a string that describes the currently selected pages.
    */
    String GetPageRangeString (void);

    void            ExecCtrl(SfxRequest &rReq);
    void            GetCtrlState(SfxItemSet &rSet);
    void            GetMenuState(SfxItemSet &rSet);
    void            GetAttrState(SfxItemSet &rSet);

    void            SetPagesPerRow( USHORT nPagesPerRow );

    void            ExecStatusBar(SfxRequest& rReq);
    void            GetStatusBarState(SfxItemSet& rSet);

    void            FuTemporary(SfxRequest &rReq);
    void            FuPermanent(SfxRequest &rReq);
    void            FuSupport(SfxRequest &rReq);

    virtual void    ReadFrameViewData(FrameView* pView);
    virtual void    WriteFrameViewData();

    virtual void    SetZoom(long nZoom);
    virtual void    SetZoomRect(const Rectangle& rZoomRect);

    virtual BOOL    HasSelection( BOOL bText = TRUE ) const;

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
    void            DrawSelectionRect( USHORT nPage );
    void            DrawFocusRect( USHORT nPage );

    virtual void    WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    virtual void    VisAreaChanged(const Rectangle& rRect);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> CreateAccessibleDocumentView( ::sd::Window* pWindow );

    void            SelectionHasChanged();
    void            PageLayoutHasChanged();
    void            FocusHasChanged( USHORT nOldFocusPage, USHORT nNewFocusPage );
    void            PageVisibilityHasChanged( USHORT nPage, BOOL bVisible );

    /** On activation the preview is turned off.
    */
    virtual void Activate (BOOL IsMDIActivate);

protected:
    virtual Size    GetOptimalSizePixel() const;
    virtual long    VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long    VirtVScrollHdl(ScrollBar* pVHScroll);


private:
    SlideView* pSlideView;
    Point           aDisplayPos;
    Size            aDisplaySize;
    USHORT          nCurFocusPage;
    bool            bSetInitialZoomFactor;
    bool            bInitializeWinPos;

    void            Construct(SdDrawDocument* pDoc);
    void            ImplDrawFocusRect( USHORT nPage, BOOL bVisible );
};

} // end of namespace sd

#endif
