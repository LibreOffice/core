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

#ifndef INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX
#define INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX

#include <sal/config.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/component.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <strings.hxx>

// OSubComponent - a component which holds a hard ref to its parent
//                 and is been hold itself (by the parent) with a weak ref
class OSubComponent : public ::cppu::OComponentHelper
{
protected:
    // the parent must support the tunnel implementation
    css::uno::Reference< css::uno::XInterface > m_xParent;
    virtual ~OSubComponent() override;

public:
    OSubComponent(::osl::Mutex& _rMutex,
                  const css::uno::Reference< css::uno::XInterface >& _xParent);

// css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

// css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL release() noexcept override;

    operator css::uno::Reference< css::uno::XInterface > () const
        { return static_cast<css::uno::XWeak *>(const_cast<OSubComponent *>(this)); }

};

// helper for implementing the createArrayHelper
#define DECL_PROP_IMPL(varname, cpputype) \
    pDesc[nPos++] = css::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cpputype,

#endif // INCLUDED_DBACCESS_SOURCE_INC_APITOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
