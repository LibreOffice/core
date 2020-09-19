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

#ifndef INCLUDED_SFX2_SOURCE_DIALOG_BACKINGWINDOW_HXX
#define INCLUDED_SFX2_SOURCE_DIALOG_BACKINGWINDOW_HXX

#include <rtl/ustring.hxx>

#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>

#include <vcl/menubtn.hxx>

#include <recentdocsview.hxx>
#include <templatedefaultview.hxx>

#include <svtools/acceleratorexecute.hxx>

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <memory>

class ToolBox;

class BackingWindow : public vcl::Window, public VclBuilderContainer
{
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::frame::XDispatchProvider> mxDesktopDispatchProvider;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    /** helper for drag&drop. */
    css::uno::Reference<css::datatransfer::dnd::XDropTargetListener> mxDropTargetListener;

    VclPtr<PushButton> mpOpenButton;
    VclPtr<MenuToggleButton> mpRecentButton;
    VclPtr<PushButton> mpRemoteButton;
    VclPtr<MenuToggleButton> mpTemplateButton;

    VclPtr<FixedText>  mpCreateLabel;

    VclPtr<PushButton> mpWriterAllButton;
    VclPtr<PushButton> mpCalcAllButton;
    VclPtr<PushButton> mpImpressAllButton;
    VclPtr<PushButton> mpDrawAllButton;
    VclPtr<PushButton> mpDBAllButton;
    VclPtr<PushButton> mpMathAllButton;

    VclPtr<PushButton> mpHelpButton;
    VclPtr<PushButton> mpExtensionsButton;

    VclPtr<VclBox> mpAllButtonsBox;
    VclPtr<VclBox> mpButtonsBox;
    VclPtr<VclBox> mpSmallButtonsBox;

    VclPtr<sfx2::RecentDocsView> mpAllRecentThumbnails;
    VclPtr<TemplateDefaultView> mpLocalView;
    bool mbLocalViewInitialized;

    std::vector< VclPtr<vcl::Window> > maDndWindows;

    tools::Rectangle maStartCentButtons;

    bool mbInitControls;
    std::unique_ptr<svt::AcceleratorExecute> mpAccExec;

    void setupButton(PushButton* pButton);
    void setupButton(MenuToggleButton* pButton);

    void dispatchURL(const OUString& i_rURL,
                     const OUString& i_rTarget = OUString("_default"),
                     const css::uno::Reference<css::frame::XDispatchProvider >& i_xProv = css::uno::Reference<css::frame::XDispatchProvider>(),
                     const css::uno::Sequence<css::beans::PropertyValue >& = css::uno::Sequence<css::beans::PropertyValue>());

    DECL_LINK(ClickHdl, Button*, void);
    DECL_LINK(MenuSelectHdl, MenuButton*, void);
    DECL_LINK(ExtLinkClickHdl, Button*, void);
    DECL_LINK(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(EditTemplateHdl, ThumbnailViewItem*, void);

    void initControls();

    void initializeLocalView();

    void checkInstalledModules();

public:
    explicit BackingWindow(vcl::Window* pParent);
    virtual ~BackingWindow() override;
    virtual void dispose() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool PreNotify(NotifyEvent& rNEvt) override;
    virtual void GetFocus() override;

    virtual Size GetOptimalSize() const override;

    void setOwningFrame(const css::uno::Reference<css::frame::XFrame>& xFrame );

    void clearRecentFileList();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
