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


#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unopracc.hxx>
#include <editeng/unoedsrc.hxx>

using namespace ::com::sun::star;


SvxAccessibleTextPropertySet::SvxAccessibleTextPropertySet( const SvxEditSource* pEditSrc, const SvxItemPropertySet* pPropSet )
    : SvxUnoTextRangeBase( pEditSrc, pPropSet )
{
}

SvxAccessibleTextPropertySet::~SvxAccessibleTextPropertySet() noexcept
{
}

uno::Reference< text::XText > SAL_CALL SvxAccessibleTextPropertySet::getText()
{
  // TODO (empty?)
  return uno::Reference< text::XText > ();
}

uno::Any SAL_CALL SvxAccessibleTextPropertySet::queryInterface( const uno::Type & rType )
{
    return OWeakObject::queryInterface(rType);
}

void SAL_CALL SvxAccessibleTextPropertySet::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL SvxAccessibleTextPropertySet::release()
    noexcept
{
    OWeakObject::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxAccessibleTextPropertySet::getTypes()
{
    static ::cppu::OTypeCollection ourTypeCollection(
                ::cppu::UnoType<beans::XPropertySet>::get(),
                ::cppu::UnoType<beans::XMultiPropertySet>::get(),
                ::cppu::UnoType<beans::XPropertyState>::get(),
                ::cppu::UnoType<lang::XServiceInfo>::get(),
                ::cppu::UnoType<lang::XTypeProvider>::get() );

    return ourTypeCollection.getTypes() ;
}

uno::Sequence< sal_Int8 > SAL_CALL SvxAccessibleTextPropertySet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
OUString SAL_CALL SAL_CALL SvxAccessibleTextPropertySet::getImplementationName()
{
    return u"SvxAccessibleTextPropertySet"_ustr;
}

sal_Bool SAL_CALL SvxAccessibleTextPropertySet::supportsService (const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
