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


//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/servicehelper.hxx>


//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------

#include <editeng/unopracc.hxx>
#include <editeng/unoedsrc.hxx>

using namespace ::com::sun::star;

//------------------------------------------------------------------------
//
// SvxAccessibleTextPropertySet implementation
//
//------------------------------------------------------------------------

SvxAccessibleTextPropertySet::SvxAccessibleTextPropertySet( const SvxEditSource* pEditSrc, const SvxItemPropertySet* pPropSet )
    : SvxUnoTextRangeBase( pEditSrc, pPropSet )
{
}

SvxAccessibleTextPropertySet::~SvxAccessibleTextPropertySet() throw()
{
}

uno::Reference< text::XText > SAL_CALL SvxAccessibleTextPropertySet::getText() throw (uno::RuntimeException)
{
  // TODO (empty?)
  return uno::Reference< text::XText > ();
}

uno::Any SAL_CALL SvxAccessibleTextPropertySet::queryAggregation( const uno::Type & ) throw(uno::RuntimeException)
{
  // TODO (empty?)
  return uno::Any();
}

uno::Any SAL_CALL SvxAccessibleTextPropertySet::queryInterface( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    return OWeakObject::queryInterface(rType);
}

void SAL_CALL SvxAccessibleTextPropertySet::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SvxAccessibleTextPropertySet::release()
    throw()
{
    OWeakObject::release();
}

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SvxAccessibleTextPropertySet::getTypes() throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    // double-checked locking pattern.
    if ( pTypeCollection == NULL )
    {
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == NULL )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                ::getCppuType( static_cast< const uno::Reference< beans::XPropertySet >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< beans::XMultiPropertySet >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< beans::XPropertyState >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< lang::XServiceInfo >* > (0) ),
                ::getCppuType( static_cast< const uno::Reference< lang::XTypeProvider >* > (0) ) );

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

namespace
{
    class theSvxAccessibleTextPropertySetImplementationId : public rtl::Static< UnoTunnelIdInit, theSvxAccessibleTextPropertySetImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SvxAccessibleTextPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    return theSvxAccessibleTextPropertySetImplementationId::get().getSeq();
}

// XServiceInfo
OUString SAL_CALL SAL_CALL SvxAccessibleTextPropertySet::getImplementationName (void) throw (uno::RuntimeException)
{
    return OUString("SvxAccessibleTextPropertySet");
}

sal_Bool SAL_CALL SvxAccessibleTextPropertySet::supportsService (const OUString& sServiceName) throw (uno::RuntimeException)
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}

uno::Sequence< OUString> SAL_CALL SvxAccessibleTextPropertySet::getSupportedServiceNames (void) throw (uno::RuntimeException)
{
    // TODO
    return SvxUnoTextRangeBase::getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
