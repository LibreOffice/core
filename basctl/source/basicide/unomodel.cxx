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


#include "basdoc.hxx"
#include <iderdll.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include "unomodel.hxx"

namespace basctl
{

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

uno::Any SAL_CALL SIDEModel::queryInterface( const uno::Type& rType )
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

void SAL_CALL SIDEModel::acquire() noexcept
{
    SolarMutexGuard aGuard;
    OWeakObject::acquire();
}

void SAL_CALL SIDEModel::release() noexcept
{
    SolarMutexGuard aGuard;
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SIDEModel::getTypes(  )
{
    return comphelper::concatSequences(
            SfxBaseModel::getTypes(),
            uno::Sequence {  cppu::UnoType<XServiceInfo>::get() });
}

OUString SIDEModel::getImplementationName()
{
    return "com.sun.star.comp.basic.BasicIDE";
}

sal_Bool SIDEModel::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SIDEModel::getSupportedServiceNames()
{
    return { "com.sun.star.script.BasicIDE" };
}

//  XStorable
void SAL_CALL SIDEModel::store()
{
    notImplemented();
}

void SAL_CALL SIDEModel::storeAsURL( const OUString&, const uno::Sequence< beans::PropertyValue >& )
{
    notImplemented();
}

void SAL_CALL SIDEModel::storeToURL( const OUString&,
        const uno::Sequence< beans::PropertyValue >& )
{
    notImplemented();
}

void  SIDEModel::notImplemented()
{
    throw io::IOException("Can't store IDE model" );
}

} // namespace basctl

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_basic_BasicID_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    SolarMutexGuard aGuard;
    basctl::EnsureIde();
    rtl::Reference<SfxObjectShell> pShell = new basctl::DocShell();
    auto pModel = pShell->GetModel();
    pModel->acquire();
    return pModel.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
