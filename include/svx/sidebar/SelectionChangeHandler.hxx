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
#ifndef INCLUDED_SVX_SIDEBAR_SELECTIONCHANGEHANDLER_HXX
#define INCLUDED_SVX_SIDEBAR_SELECTIONCHANGEHANDLER_HXX

#include <svx/svxdllapi.h>
#include <vcl/EnumContext.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <functional>

class SdrMarkView;


namespace svx { namespace sidebar {

typedef cppu::WeakComponentImplHelper<
    css::view::XSelectionChangeListener
    > SelectionChangeHandlerInterfaceBase;

class SVX_DLLPUBLIC SelectionChangeHandler
    : private ::cppu::BaseMutex,
      public SelectionChangeHandlerInterfaceBase
{
public:
    SelectionChangeHandler (
        const std::function<rtl::OUString ()>& rSelectionChangeCallback,
        const css::uno::Reference<css::frame::XController>& rxController,
        const vcl::EnumContext::Context eDefaultContext);
    virtual ~SelectionChangeHandler() override;

    virtual void SAL_CALL selectionChanged (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL disposing() override;

    void Connect();
    void Disconnect();

private:
    SelectionChangeHandler(const SelectionChangeHandler&) = delete;
    SelectionChangeHandler& operator=(const SelectionChangeHandler&) = delete;

    const std::function<rtl::OUString ()> maSelectionChangeCallback;
    css::uno::Reference<css::frame::XController> mxController;
    const vcl::EnumContext::Context meDefaultContext;
    bool mbIsConnected;
};


} } // end of namespace svx::sidebar


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
