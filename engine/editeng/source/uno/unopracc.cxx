/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/Any.hxx>
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

uno::Reference< text::XText > SvxAccessibleTextPropertySet::getText()
{
  // TODO (empty?)
  return uno::Reference< text::XText > ();
}

cpo::uno::Any SvxAccessibleTextPropertySet::queryInterface( const cpo::uno::Type & rType )
{
    return OWeakObject::queryInterface(rType);
}

void SvxAccessibleTextPropertySet::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SvxAccessibleTextPropertySet::release()
    noexcept
{
    OWeakObject::release();
}

// XTypeProvider
cpo::uno::Sequence< cpo::uno::Type > SvxAccessibleTextPropertySet::getTypes()
{
    static ::cppu::OTypeCollection ourTypeCollection(
                ::cppu::UnoType<beans::XPropertySet>::get(),
                ::cppu::UnoType<beans::XMultiPropertySet>::get(),
                ::cppu::UnoType<beans::XPropertyState>::get(),
                ::cppu::UnoType<lang::XServiceInfo>::get(),
                ::cppu::UnoType<lang::XTypeProvider>::get() );

    return ourTypeCollection.getTypes() ;
}

cpo::uno::Sequence< sal_Int8 > SvxAccessibleTextPropertySet::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

// XServiceInfo
OUString SvxAccessibleTextPropertySet::getImplementationName()
{
    return u"SvxAccessibleTextPropertySet"_ustr;
}

bool SvxAccessibleTextPropertySet::supportsService (const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
