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

#include <vcl/InterimItemWindow.hxx>

#include <recentdocsview.hxx>
#include <templatedefaultview.hxx>

#include <svtools/acceleratorexecute.hxx>

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <memory>

class BackingWindow : public InterimItemWindow
{
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::frame::XDispatchProvider> mxDesktopDispatchProvider;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    /** helper for drag&drop. */
    css::uno::Reference<css::datatransfer::dnd::XDropTargetListener> mxDropTargetListener;

    std::unique_ptr<weld::Button> mxOpenButton;
    std::unique_ptr<weld::MenuToggleButton> mxRecentButton;
    std::unique_ptr<weld::Button> mxRemoteButton;
    std::unique_ptr<weld::MenuToggleButton> mxTemplateButton;

    std::unique_ptr<weld::Label> mxCreateLabel;
    std::unique_ptr<weld::Label> mxAltHelpLabel;

    std::unique_ptr<weld::Button> mxWriterAllButton;
    std::unique_ptr<weld::Button> mxCalcAllButton;
    std::unique_ptr<weld::Button> mxImpressAllButton;
    std::unique_ptr<weld::Button> mxDrawAllButton;
    std::unique_ptr<weld::Button> mxDBAllButton;
    std::unique_ptr<weld::Button> mxMathAllButton;
    std::unique_ptr<weld::Image> mxBrandImage;

    std::unique_ptr<weld::Button> mxHelpButton;
    std::unique_ptr<weld::Button> mxExtensionsButton;

    std::unique_ptr<weld::Container> mxAllButtonsBox;
    std::unique_ptr<weld::Container> mxButtonsBox;
    std::unique_ptr<weld::Container> mxSmallButtonsBox;

    std::unique_ptr<sfx2::RecentDocsView> mxAllRecentThumbnails;
    std::unique_ptr<weld::CustomWeld> mxAllRecentThumbnailsWin;
    std::unique_ptr<TemplateDefaultView> mxLocalView;
    std::unique_ptr<weld::CustomWeld> mxLocalViewWin;
    bool mbLocalViewInitialized;

    css::uno::Reference<css::datatransfer::dnd::XDropTarget> mxDropTarget;

    bool mbInitControls;
    std::unique_ptr<svt::AcceleratorExecute> mpAccExec;

    void dispatchURL(const OUString& i_rURL, const OUString& i_rTarget = OUString("_default"),
                     const css::uno::Reference<css::frame::XDispatchProvider>& i_xProv
                     = css::uno::Reference<css::frame::XDispatchProvider>(),
                     const css::uno::Sequence<css::beans::PropertyValue>& = css::uno::Sequence<
                         css::beans::PropertyValue>());

    DECL_LINK(ClickHdl, weld::Button&, void);
    DECL_LINK(ClickHelpHdl, weld::Button&, void);
    DECL_LINK(MenuSelectHdl, const OString&, void);
    DECL_LINK(ExtLinkClickHdl, weld::Button&, void);
    DECL_LINK(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK(EditTemplateHdl, ThumbnailViewItem*, void);

    void initControls();

    void initializeLocalView();

    void checkInstalledModules();

    void DataChanged(const DataChangedEvent&) override;

    template <typename WidgetClass> void setLargerFont(WidgetClass&, const vcl::Font&);
    void ApplyStyleSettings();

public:
    explicit BackingWindow(vcl::Window* pParent);
    virtual ~BackingWindow() override;
    virtual void dispose() override;

    virtual bool PreNotify(NotifyEvent& rNEvt) override;
    virtual void GetFocus() override;

    void setOwningFrame(const css::uno::Reference<css::frame::XFrame>& xFrame);

    void clearRecentFileList();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
