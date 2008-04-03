/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlineViewShell.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:54:51 $
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

#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#define SD_OUTLINE_VIEW_SHELL_HXX

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_HXX
#include "OutlineView.hxx"
#endif

class SdPage;
class TransferableDataHelper;
class TransferableClipboardListener;

namespace sd {

class SdUnoOutlineView;

/** Show a textual overview of the text contents of all slides.
*/
class OutlineViewShell
    : public ViewShell
{
public:
    TYPEINFO();

    SFX_DECL_VIEWFACTORY(OutlineViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDOUTLINEVIEWSHELL)

    // The previous macros change access mode.  To be sure switch back
    // to public access.
public:
    /** Create a new view shell for the outline mode.
        @param rViewShellBase
            The new object will be stacked on this view shell base.
        @param pFrameView
            The frame view that makes it possible to pass information from
            one view shell to the next.
    */
    OutlineViewShell (
        SfxViewFrame* pFrame,
        ViewShellBase& rViewShellBase,
        ::Window* pParentWindow,
        FrameView* pFrameView = NULL);

    OutlineViewShell (
        SfxViewFrame* pFrame,
        ::Window* pParentWindow,
        const OutlineViewShell& rShell);

    virtual ~OutlineViewShell (void);

    virtual void Shutdown (void);

    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin);

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements (void);

    virtual USHORT PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );

    virtual long VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long VirtVScrollHdl(ScrollBar* pVHScroll);

    virtual void AddWindow(::sd::Window* pWin);
    virtual void RemoveWindow(::sd::Window* pWin);

    virtual void Activate( BOOL IsMDIActivate );
    virtual void Deactivate( BOOL IsMDIActivate );

    virtual SdPage* GetActualPage();

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const;

    /** Return a string that describes the currently selected pages.
    */
    String GetPageRangeString (void);

    void ExecCtrl(SfxRequest &rReq);
    void GetCtrlState(SfxItemSet &rSet);
    void GetMenuState(SfxItemSet &rSet);
    void GetAttrState(SfxItemSet &rSet);
    void GetState (SfxItemSet& rSet);

    void ExecStatusBar(SfxRequest& rReq);
    void GetStatusBarState(SfxItemSet& rSet);

    void FuTemporary(SfxRequest &rReq);
    void FuTemporaryModify(SfxRequest &rReq);
    void FuPermanent(SfxRequest &rReq);
    void FuSupport(SfxRequest &rReq);

    virtual void SetZoom(long nZoom);
    virtual void SetZoomRect(const Rectangle& rZoomRect);
    virtual String  GetSelectionText( BOOL bCompleteWords = FALSE );
    virtual BOOL    HasSelection( BOOL bText = TRUE ) const;

    void Execute(SfxRequest& rReq);

    virtual void ReadFrameViewData(FrameView* pView);
    virtual void WriteFrameViewData();

    virtual void Command( const CommandEvent& rCEvt, ::sd::Window* pWin );
    virtual BOOL KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);

    ULONG   Read(SvStream& rInput, const String& rBaseURL, USHORT eFormat);

    virtual void WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const Rectangle& rRect);

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

    /** Update the preview to show the specified page.
    */
    virtual void UpdatePreview (SdPage* pPage, BOOL bInit = FALSE);

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void);

    /** Make the given page the new current page.  This method
        notifies the controller and adapts the selection of the
        model.
        @param pPage
            The new current page.  Pass NULL when there is no current page.
    */
    void SetCurrentPage (SdPage* pPage);

    bool UpdateTitleObject( SdPage* pPage, Paragraph* pPara );
    bool UpdateOutlineObject( SdPage* pPage, Paragraph* pPara );

protected:
    virtual Size GetOptimalSizePixel() const;

private:
    OutlineView* pOlView;
    SdPage*         pLastPage; // Zur performanten Aufbereitung der Preview
    TransferableClipboardListener* pClipEvtLstnr;
    BOOL            bPastePossible;
    bool mbInitialized;

    void Construct (DrawDocShell* pDocSh);
    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
};

} // end of namespace sd

#endif
