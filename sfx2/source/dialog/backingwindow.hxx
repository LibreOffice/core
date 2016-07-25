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
#include <vcl/tabctrl.hxx>
#include <vcl/layout.hxx>

#include <vcl/menubtn.hxx>

#include <sfx2/recentdocsview.hxx>
#include <sfx2/templatedefaultview.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/templateviewitem.hxx>

#include <svtools/acceleratorexecute.hxx>
#include <unotools/moduleoptions.hxx>

#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <memory>
#include <set>

class ToolBox;
class BackingWindowTemplateSearchView;

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
    VclPtr<ListBox>    mpCBApp;
    VclPtr<Edit>       mpSearchFilter;

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

    VclPtr<RecentDocsView> mpAllRecentThumbnails;
    VclPtr<TemplateDefaultView> mpLocalView;
    VclPtr<BackingWindowTemplateSearchView> mpSearchView;
    bool mbLocalViewInitialized;

    std::vector< VclPtr<vcl::Window> > maDndWindows;

    Color maButtonsTextColor;
    Rectangle maStartCentButtons;

    bool mbInitControls;
    sal_Int32 mnHideExternalLinks;
    std::unique_ptr<svt::AcceleratorExecute> mpAccExec;

    void setupButton(PushButton* pButton);
    void setupButton(MenuToggleButton* pButton);

    void dispatchURL(const OUString& i_rURL,
                     const OUString& i_rTarget = OUString("_default"),
                     const css::uno::Reference<css::frame::XDispatchProvider >& i_xProv = css::uno::Reference<css::frame::XDispatchProvider>(),
                     const css::uno::Sequence<css::beans::PropertyValue >& = css::uno::Sequence<css::beans::PropertyValue>());

    DECL_LINK_TYPED(ClickHdl, Button*, void);
    DECL_LINK_TYPED(MenuSelectHdl, MenuButton*, void);
    DECL_LINK_TYPED(SearchUpdateHdl, Edit&, void);
    DECL_LINK_TYPED(ExtLinkClickHdl, Button*, void);
    DECL_LINK_TYPED(CreateContextMenuHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(OpenTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(EditTemplateHdl, ThumbnailViewItem*, void);
    DECL_LINK_TYPED(SelectApplicationHdl, ListBox&, void);
    DECL_LINK_TYPED(GetFocusHdl, Control&, void);

    void initControls();

    void initializeLocalView();

    void checkInstalledModules();

    /// Return filter according to the currently selected application filter.
    FILTER_APPLICATION getCurrentApplicationFilter();

public:
    explicit BackingWindow(vcl::Window* pParent);
    virtual ~BackingWindow();
    virtual void dispose() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool PreNotify(NotifyEvent& rNEvt) override;
    virtual void GetFocus() override;

    virtual Size GetOptimalSize() const override;

    void setOwningFrame(const css::uno::Reference<css::frame::XFrame>& xFrame );

    void clearRecentFileList();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
