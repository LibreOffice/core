/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    virtual bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
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
    virtual void    SetZoomRange(const basegfx::B2DRange& rZoomRange);

    virtual bool    HasSelection( bool bText = true ) const;

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
//IAccessibility2 Implementation 2009-----
    //Notify the SELECTION_CHANGE, SELECTION_ADD, SELECTION_REMOVE events
    void            SelectionHasChanged(sal_uInt16 nPage, sal_Bool bSelect);
//-----IAccessibility2 Implementation 2009
    void            PageLayoutHasChanged();
    void            FocusHasChanged( sal_uInt16 nOldFocusPage, sal_uInt16 nNewFocusPage );
    void            PageVisibilityHasChanged( sal_uInt16 nPage, bool bVisible );

    /** On activation the preview is turned off.
    */
    virtual void Activate (sal_Bool IsMDIActivate);

//IAccessibility2 Implementation 2009-----
    void SwitchViewFireFocus(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc );
//-----IAccessibility2 Implementation 2009
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
    void            ImplDrawFocusRect( sal_uInt16 nPage, bool bVisible );
};

} // end of namespace sd

#endif
