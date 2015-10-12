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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_ACCESSIBLE_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_ACCESSIBLE_HXX

#include <boost/noncopyable.hpp>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::accessibility::XAccessible
        > AccessibleInterfaceBase;
}

namespace sfx2 { namespace sidebar {


/** Simple implementation of the XAccessible interface.
    Its getAccessibleContext() method returns a context object given
    to its constructor.
*/
class Accessible
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public AccessibleInterfaceBase
{
public:
    explicit Accessible (
        const css::uno::Reference<css::accessibility::XAccessibleContext>& rxContext);
    virtual ~Accessible();

    virtual void SAL_CALL disposing() override;
    // XAccessible
    virtual css::uno::Reference<css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<css::accessibility::XAccessibleContext> mxContext;
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
