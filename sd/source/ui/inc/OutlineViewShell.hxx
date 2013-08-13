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

#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#define SD_OUTLINE_VIEW_SHELL_HXX

#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "OutlineView.hxx"

class SdPage;
class TransferableDataHelper;
class TransferableClipboardListener;

namespace sd {

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

    virtual ~OutlineViewShell (void);

    virtual void Shutdown (void);

    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin);

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements (void);

    virtual sal_uInt16 PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );

    virtual long VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long VirtVScrollHdl(ScrollBar* pVHScroll);

    virtual void AddWindow(::sd::Window* pWin);
    virtual void RemoveWindow(::sd::Window* pWin);

    virtual void Activate( sal_Bool IsMDIActivate );
    virtual void Deactivate( sal_Bool IsMDIActivate );

    virtual SdPage* GetActualPage();

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const;

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
    virtual OUString GetSelectionText( bool bCompleteWords = false );
    virtual sal_Bool    HasSelection( sal_Bool bText = sal_True ) const;

    void Execute(SfxRequest& rReq);

    virtual void ReadFrameViewData(FrameView* pView);
    virtual void WriteFrameViewData();

    virtual void Command( const CommandEvent& rCEvt, ::sd::Window* pWin );
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);

    sal_uLong   Read(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat);

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
    virtual void UpdatePreview (SdPage* pPage, sal_Bool bInit = sal_False);

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

private:
    void ShowSlideShow(SfxRequest& rReq);
    OutlineView* pOlView;
    SdPage*         pLastPage; // Zur performanten Aufbereitung der Preview
    TransferableClipboardListener* pClipEvtLstnr;
    sal_Bool            bPastePossible;
    bool mbInitialized;

    void Construct (DrawDocShell* pDocSh);
    DECL_LINK( ClipboardChanged, TransferableDataHelper* );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
