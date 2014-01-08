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

#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>
#include "hatchwindow.hxx"
#include "unowizard.hxx"

#include "comphelper/servicedecl.hxx"

#include "cppuhelper/implementationentry.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;


namespace sdecl = comphelper::service_decl;

namespace unographic {
extern sdecl::ServiceDecl const serviceDecl;
}

// for CreateInstance functions implemented elsewhere
#define DECLARE_CREATEINSTANCE( ImplName ) \
    Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& );

// for CreateInstance functions implemented elsewhere, while the function is within a namespace
#define DECLARE_CREATEINSTANCE_NAMESPACE( nmspe, ImplName ) \
    namespace nmspe {   \
        Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& ); \
    }

namespace
{
    static const struct ::cppu::ImplementationEntry s_aServiceEntries[] =
    {
        {
            ::svt::uno::Wizard::Create,
            ::svt::uno::Wizard::getImplementationName_static,
            ::svt::uno::Wizard::getSupportedServiceNames_static,
            ::cppu::createSingleComponentFactory, NULL, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

DECLARE_CREATEINSTANCE_NAMESPACE( svt, OAddressBookSourceDialogUno )
DECLARE_CREATEINSTANCE( SvFilterOptionsDialog )

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL svt_component_getFactory (
    const sal_Char * pImplementationName, void * _pServiceManager, void * pRegistryKey)
{
    void * pResult = 0;
    if ( _pServiceManager )
    {
        Reference< XMultiServiceFactory > xSMgr(reinterpret_cast< XMultiServiceFactory * >( _pServiceManager ) );

        Reference< XSingleServiceFactory > xFactory;
        if (rtl_str_compare (
                pImplementationName, "com.sun.star.comp.svtools.OAddressBookSourceDialogUno") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString( "com.sun.star.ui.AddressBookSourceDialog" );

            xFactory = ::cppu::createSingleFactory (xSMgr,
                OUString::createFromAscii( pImplementationName ),
                svt::OAddressBookSourceDialogUno_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare (
                     pImplementationName, "com.sun.star.svtools.SvFilterOptionsDialog") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString( "com.sun.star.ui.dialogs.FilterOptionsDialog" );

            xFactory = ::cppu::createSingleFactory (xSMgr,
                OUString::createFromAscii( pImplementationName ),
                SvFilterOptionsDialog_CreateInstance,
                aServiceNames);
        }
        else
        {
            pResult = comphelper::service_decl::component_getFactoryHelper(
                pImplementationName,
                static_cast<css::lang::XMultiServiceFactory *>(_pServiceManager),
                static_cast<css::registry::XRegistryKey *>(pRegistryKey),
                unographic::serviceDecl );
            if ( !pResult )
                pResult = cppu::component_getFactoryHelper(pImplementationName,
                                                        _pServiceManager,
                                                        pRegistryKey,
                                                        s_aServiceEntries);
        }

        if ( xFactory.is() )
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

}   // "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
