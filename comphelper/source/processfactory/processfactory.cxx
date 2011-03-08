/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "com/sun/star/beans/XPropertySet.hpp"


using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace osl;

namespace comphelper
{

/*
    This function preserves only that the xProcessFactory variable will not be create when
    the library is loaded.
*/
Reference< XMultiServiceFactory > localProcessFactory( const Reference< XMultiServiceFactory >& xSMgr, sal_Bool bSet )
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );

    static Reference< XMultiServiceFactory > xProcessFactory;
    if ( bSet )
    {
        xProcessFactory = xSMgr;
    }

    return xProcessFactory;
}


void setProcessServiceFactory(const Reference< XMultiServiceFactory >& xSMgr)
{
    localProcessFactory( xSMgr, sal_True );
}

Reference< XMultiServiceFactory > getProcessServiceFactory()
{
    Reference< XMultiServiceFactory> xReturn;
    xReturn = localProcessFactory( xReturn, sal_False );
    return xReturn;
}

Reference< XInterface > createProcessComponent( const ::rtl::OUString& _rServiceSpecifier ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstance( _rServiceSpecifier );

    return xComponent;
}

Reference< XInterface > createProcessComponentWithArguments( const ::rtl::OUString& _rServiceSpecifier,
        const Sequence< Any >& _rArgs ) SAL_THROW( ( RuntimeException ) )
{
    Reference< XInterface > xComponent;

    Reference< XMultiServiceFactory > xFactory( getProcessServiceFactory() );
    if ( xFactory.is() )
        xComponent = xFactory->createInstanceWithArguments( _rServiceSpecifier, _rArgs );

    return xComponent;
}

Reference< XComponentContext > getProcessComponentContext()
{
    Reference< XComponentContext > xRet;
    uno::Reference<beans::XPropertySet> const xProps(
        comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
    if (xProps.is()) {
        try {
            xRet.set( xProps->getPropertyValue( rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ),
                      uno::UNO_QUERY );
        }
        catch (beans::UnknownPropertyException const&) {
        }
    }
    return xRet;
}

} // namespace comphelper

extern "C" {
uno::XComponentContext * comphelper_getProcessComponentContext()
{
    uno::Reference<uno::XComponentContext> xRet;
    xRet = ::comphelper::getProcessComponentContext();
    if (xRet.is())
        xRet->acquire();
    return xRet.get();
}
} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
