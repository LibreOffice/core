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

#ifndef INCLUDED_BASIC_SOURCE_RUNTIME_COMENUMWRAPPER_HXX
#define INCLUDED_BASIC_SOURCE_RUNTIME_COMENUMWRAPPER_HXX

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <cppuhelper/implbase1.hxx>

class ComEnumerationWrapper : public ::cppu::WeakImplHelper1< ::com::sun::star::container::XEnumeration >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation > m_xInvocation;
    sal_Int32 m_nCurInd;

public:
    ComEnumerationWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XInvocation >& xInvocation )
    : m_xInvocation( xInvocation )
    , m_nCurInd( 0 )
    {
    }

    // container::XEnumeration
    virtual ::sal_Bool SAL_CALL hasMoreElements() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // INCLUDED_BASIC_SOURCE_RUNTIME_COMENUMWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
