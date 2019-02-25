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

#ifndef INCLUDED_SD_SOURCE_UI_INC_OUTLINEVIEWSHELL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_OUTLINEVIEWSHELL_HXX

#include "ViewShell.hxx"
#include <glob.hxx>

class SdPage;
class TransferableDataHelper;
class TransferableClipboardListener;

namespace sd { class OutlineView; }

namespace sd {

/** Show a textual overview of the text contents of all slides.
*/
class OutlineViewShell
    : public ViewShell
{
public:

    SFX_DECL_VIEWFACTORY(OutlineViewShell);
    SFX_DECL_INTERFACE(SD_IF_SDOUTLINEVIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

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
        vcl::Window* pParentWindow,
        FrameView* pFrameView);

    virtual ~OutlineViewShell() override;

    virtual void Shutdown() override;

    virtual void Paint(const ::tools::Rectangle& rRect, ::sd::Window* pWin) override;

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements() override;

    virtual bool PrepareClose( bool bUI = true ) override;

    virtual void VirtHScrollHdl(ScrollBar* pHScroll) override;
    virtual void VirtVScrollHdl(ScrollBar* pVHScroll) override;

    virtual void Activate( bool IsMDIActivate ) override;
    virtual void Deactivate( bool IsMDIActivate ) override;

    virtual SdPage* GetActualPage() override;

    /// inherited from sd::ViewShell
    virtual SdPage* getCurrentPage() const override;

    void ExecCtrl(SfxRequest &rReq);
    void GetCtrlState(SfxItemSet &rSet);
    // FIXME non-virtual override???
    void GetMenuState(SfxItemSet &rSet);
    void GetAttrState(SfxItemSet &rSet);
    void GetState (SfxItemSet& rSet);

    static void ExecStatusBar(SfxRequest& rReq);
    void GetStatusBarState(SfxItemSet& rSet);

    void FuTemporary(SfxRequest &rReq);
    void FuTemporaryModify(SfxRequest &rReq);
    void FuPermanent(SfxRequest &rReq);
    void FuSupport(SfxRequest &rReq);

    virtual void SetZoom(long nZoom) override;
    virtual void SetZoomRect(const ::tools::Rectangle& rZoomRect) override;

    void Execute(SfxRequest& rReq);

    virtual void ReadFrameViewData(FrameView* pView) override;
    virtual void WriteFrameViewData() override;

    virtual void Command( const CommandEvent& rCEvt, ::sd::Window* pWin ) override;
    virtual bool KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin) override;
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin) override;

    ErrCode ReadRtf(SvStream& rInput);

    virtual void WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& ) override;
    virtual void ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& ) override;

    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const ::tools::Rectangle& rRect) override;

    /** Create an accessible object representing the specified window.
        @param pWindow
            The returned object makes the document displayed in this window
            accessible.
        @return
            Returns an <type>AccessibleDrawDocumentView</type> object.
   */
    virtual css::uno::Reference<css::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow) override;

    /** Update the preview to show the specified page.
    */
    virtual void UpdatePreview (SdPage* pPage, bool bInit = false) override;

    virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController() override;

    /** Make the given page the new current page.  This method
        notifies the controller and adapts the selection of the
        model.
        @param pPage
            The new current page.  Pass NULL when there is no current page.
    */
    void SetCurrentPage (SdPage* pPage);

    void UpdateTitleObject( SdPage* pPage, Paragraph const * pPara );
    void UpdateOutlineObject( SdPage* pPage, Paragraph* pPara );

private:
    OUString m_StrOldPageName;
    std::unique_ptr<OutlineView> pOlView;
    SdPage*         pLastPage; // For efficient processing of the preview
    rtl::Reference<TransferableClipboardListener> mxClipEvtLstnr;
    bool            bPastePossible;
    bool mbInitialized;

    void Construct();
    DECL_LINK( ClipboardChanged, TransferableDataHelper*, void );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
