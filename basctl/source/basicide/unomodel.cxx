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


#include "unomodel.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>

#include <iderdll.hxx>
#include <basdoc.hxx>

using ::rtl::OUString;
using namespace ::cppu;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

SIDEModel::SIDEModel( SfxObjectShell *pObjSh )
: SfxBaseModel(pObjSh)
{
}

SIDEModel::~SIDEModel()
{
}

uno::Any SAL_CALL SIDEModel::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet =  ::cppu::queryInterface ( rType,
                                    // OWeakObject interfaces
                                    static_cast< XInterface* >( static_cast< OWeakObject* >( this ) ),
                                    static_cast< XWeak* > ( this ),
                                    static_cast< XServiceInfo*  > ( this ) );
    if (!aRet.hasValue())
        aRet = SfxBaseModel::queryInterface ( rType );
    return aRet;
}

void SAL_CALL SIDEModel::acquire() throw()
{
    SolarMutexGuard aGuard;
    OWeakObject::acquire();
}

void SAL_CALL SIDEModel::release() throw()
{
    SolarMutexGuard aGuard;
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SIDEModel::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes = SfxBaseModel::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    uno::Type* pTypes = aTypes.getArray();
    pTypes[nLen++] = ::getCppuType((Reference<XServiceInfo>*)0);

    return aTypes;
}

OUString SIDEModel::getImplementationName(void) throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

::rtl::OUString SIDEModel::getImplementationName_Static()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.basic.BasicIDE" ));
}

sal_Bool SIDEModel::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.BasicIDE" ));
}
uno::Sequence< OUString > SIDEModel::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SIDEModel::getSupportedServiceNames_Static(void)
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.BasicIDE" ));
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL SIDEModel_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & ) throw( uno::Exception )
{
    SolarMutexGuard aGuard;
    BasicIDEGlobals::ensure();
    SfxObjectShell* pShell = new BasicDocShell();
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

//  XStorable
void SAL_CALL SIDEModel::store() throw (io::IOException, uno::RuntimeException)
{
    notImplemented();
}

void SAL_CALL SIDEModel::storeAsURL( const ::rtl::OUString&, const uno::Sequence< PROPERTYVALUE >& )
        throw (io::IOException, uno::RuntimeException)
{
    notImplemented();
}

void SAL_CALL SIDEModel::storeToURL( const ::rtl::OUString&,
        const uno::Sequence< PROPERTYVALUE >& )
        throw (io::IOException, uno::RuntimeException)
{
    notImplemented();
}

void  SIDEModel::notImplemented() throw ( io::IOException )
{
    throw io::IOException("Can't store IDE model", uno::Reference< uno::XInterface >() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
