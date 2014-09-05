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
#include <sfx2/sidebar/EnumContext.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>


class SdrMarkView;


namespace svx { namespace sidebar {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::view::XSelectionChangeListener
        > SelectionChangeHandlerInterfaceBase;
}


class SVX_DLLPUBLIC SelectionChangeHandler
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SelectionChangeHandlerInterfaceBase
{
public:
    SelectionChangeHandler (
        const boost::function<rtl::OUString(void)>& rSelectionChangeCallback,
        const css::uno::Reference<css::frame::XController>& rxController,
        const sfx2::sidebar::EnumContext::Context eDefaultContext);
    virtual ~SelectionChangeHandler (void);

    virtual void SAL_CALL selectionChanged (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disposing (void)
        throw (css::uno::RuntimeException) SAL_OVERRIDE;

    void Connect (void);
    void Disconnect (void);

private:
    const boost::function<rtl::OUString(void)> maSelectionChangeCallback;
    css::uno::Reference<css::frame::XController> mxController;
    const sfx2::sidebar::EnumContext::Context meDefaultContext;
    bool mbIsConnected;
};


} } // end of namespace svx::sidebar


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
