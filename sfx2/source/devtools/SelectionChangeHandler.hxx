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

#include <DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/compbase.hxx>
#include <utility>

typedef comphelper::WeakComponentImplHelper<css::view::XSelectionChangeListener>
    SelectionChangeHandlerInterfaceBase;

/** Selection change handler to listen to document selection changes.
 *
 * Listens to the changes and notifies the docking window with a new
 * selected object, when a change happens.
 */
class SelectionChangeHandler final : public SelectionChangeHandlerInterfaceBase
{
private:
    css::uno::Reference<css::frame::XController> mxController;
    VclPtr<DevelopmentToolDockingWindow> mpDockingWindow;

public:
    SelectionChangeHandler(css::uno::Reference<css::frame::XController> xController,
                           DevelopmentToolDockingWindow* pDockingWindow)
        : mxController(std::move(xController))
        , mpDockingWindow(pDockingWindow)
    {
        css::uno::Reference<css::view::XSelectionSupplier> xSupplier(mxController,
                                                                     css::uno::UNO_QUERY);
        xSupplier->addSelectionChangeListener(this);
    }

    ~SelectionChangeHandler() { mpDockingWindow.disposeAndClear(); }

    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& /*rEvent*/) override
    {
        css::uno::Reference<css::view::XSelectionSupplier> xSupplier(mxController,
                                                                     css::uno::UNO_QUERY);
        if (xSupplier.is())
        {
            css::uno::Any aAny = xSupplier->getSelection();
            auto xInterface = aAny.get<css::uno::Reference<css::uno::XInterface>>();
            mpDockingWindow->selectionChanged(xInterface);
        }
    }

    void stopListening()
    {
        css::uno::Reference<css::view::XSelectionSupplier> xSupplier(mxController,
                                                                     css::uno::UNO_QUERY);
        xSupplier->removeSelectionChangeListener(this);
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject& /*rEvent*/) override {}
    using comphelper::WeakComponentImplHelperBase::disposing;

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
