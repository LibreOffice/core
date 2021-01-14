/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <svx/svxdllapi.h>
#include <sfx2/dockwin.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <unordered_map>

class SAL_WARN_UNUSED SVX_DLLPUBLIC DevelopmentToolDockingWindow final : public SfxDockingWindow
{
private:
    std::unique_ptr<weld::Label> mpClassNameLabel;
    std::unique_ptr<weld::TreeView> mpClassListBox;
    std::unique_ptr<weld::TreeView> mpLeftSideTreeView;

    css::uno::Reference<css::uno::XInterface> mxRoot;
    OUString msDocumentType;

    std::unordered_map<OUString, css::uno::Reference<css::uno::XInterface>> maUnoObjectMap;

    DECL_LINK(ModelTreeViewExpanding, const weld::TreeIter&, bool);

    DECL_LINK(LeftSideSelected, weld::TreeView&, void);

    void inspectDocument();

    void clearChildren(weld::TreeIter const& rParent);

    void fillSheets(weld::TreeIter const& rParent);
    void fillPages(weld::TreeIter const& rParent);
    void fillSlides(weld::TreeIter const& rParent);
    void fillParagraphs(weld::TreeIter const& rParent);

public:
    DevelopmentToolDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 vcl::Window* pParent);

    virtual ~DevelopmentToolDockingWindow() override;

    virtual void dispose() override;

    virtual void ToggleFloatingMode() override;

    void introspect(css::uno::Reference<css::uno::XInterface> const& xInterface);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
