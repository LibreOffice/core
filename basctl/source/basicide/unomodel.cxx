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


#include "unomodel.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>

#include <iderdll.hxx>
#include <basdoc.hxx>

namespace basctl
{

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
    EnsureIde();
    SfxObjectShell* pShell = new DocShell();
    return uno::Reference< uno::XInterface >( pShell->GetModel() );
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
