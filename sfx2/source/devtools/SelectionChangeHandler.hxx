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

#include <memory>

#include <sfx2/devtools/DevelopmentToolDockingWindow.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

typedef cppu::WeakComponentImplHelper<css::view::XSelectionChangeListener>
    SelectionChangeHandlerInterfaceBase;

class SelectionChangeHandler final : private ::cppu::BaseMutex,
                                     public SelectionChangeHandlerInterfaceBase
{
private:
    css::uno::Reference<css::frame::XController> mxController;
    VclPtr<DevelopmentToolDockingWindow> mpDockingWindow;

public:
    SelectionChangeHandler(const css::uno::Reference<css::frame::XController>& rxController,
                           DevelopmentToolDockingWindow* pDockingWindow)
        : SelectionChangeHandlerInterfaceBase(m_aMutex)
        , mxController(rxController)
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
    virtual void SAL_CALL disposing() override {}

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
