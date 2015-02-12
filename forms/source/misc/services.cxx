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

#include <config_features.h>

#include "services.hxx"
#include "frm_module.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

static Sequence< OUString >              s_aClassImplementationNames;
static Sequence<Sequence< OUString > >   s_aClassServiceNames;

// need to use sal_Int64 instead of ComponentInstantiation, as ComponentInstantiation has no cppuType, so
// it can't be used with sequences
static Sequence<sal_Int64>               s_aFactories;

void registerClassInfo(
        const OUString& _rClassImplName,                 // the ImplName of the class
        const Sequence< OUString >& _rServiceNames,      // the services supported by this class
        ::cppu::ComponentInstantiation _pCreateFunction  // the method for instantiating such a class
        )
{
    sal_Int32 nCurrentLength = s_aClassImplementationNames.getLength();
    OSL_ENSURE((nCurrentLength == s_aClassServiceNames.getLength())
        && (nCurrentLength == s_aFactories.getLength()),
        "forms::registerClassInfo : invalid class infos !");

    s_aClassImplementationNames.realloc(nCurrentLength + 1);
    s_aClassServiceNames.realloc(nCurrentLength + 1);
    s_aFactories.realloc(nCurrentLength + 1);

    s_aClassImplementationNames.getArray()[nCurrentLength] = _rClassImplName;
    s_aClassServiceNames.getArray()[nCurrentLength] = _rServiceNames;
    s_aFactories.getArray()[nCurrentLength] = reinterpret_cast<sal_Int64>(_pCreateFunction);
}


void ensureClassInfos()
{
    if (s_aClassImplementationNames.getLength())
        // nothing to do
        return;
    Sequence< OUString > aServices;

    aServices.realloc(1);
    aServices[0] = "com.sun.star.xforms.XForms";

    registerClassInfo(
        "com.sun.star.form.XForms",
        aServices,
        frm::XForms_CreateInstance);
}


extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL frm_component_getFactory(const sal_Char* _pImplName, void* _pServiceManager, void* /*_pRegistryKey*/)
{
    if (!_pServiceManager || !_pImplName)
        return NULL;


    // a lot of stuff which is implemented "manually" here in this file
    void* pRet = NULL;

    // collect the class infos
    ensureClassInfos();

    // both our static sequences should have the same length ...
    sal_Int32 nClasses = s_aClassImplementationNames.getLength();
    OSL_ENSURE((s_aClassServiceNames.getLength() == nClasses) &&
        (s_aFactories.getLength() == nClasses),
        "forms::component_getFactory : invalid class infos !");

    // loop through the sequences and register the service providers
    const OUString* pClasses = s_aClassImplementationNames.getConstArray();
    const Sequence< OUString >* pServices = s_aClassServiceNames.getConstArray();
    const sal_Int64* pFunctionsAsInts = s_aFactories.getConstArray();

    for (sal_Int32 i=0; i<nClasses; ++i, ++pClasses, ++pServices, ++pFunctionsAsInts)
    {
        if (rtl_ustr_ascii_compare(pClasses->getStr(), _pImplName) == 0)
        {
            ::cppu::ComponentInstantiation aCurrentCreateFunction =
                reinterpret_cast< ::cppu::ComponentInstantiation>(*pFunctionsAsInts);

            Reference<XSingleServiceFactory> xFactory(
                ::cppu::createSingleFactory(
                    static_cast<css::lang::XMultiServiceFactory*>(
                        _pServiceManager),
                    *pClasses,
                    aCurrentCreateFunction,
                    *pServices
                )
            );
            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
                break;
            }
        }
    }


    // the real way - use the OModule
    if ( !pRet )
    {
        // let the module look for the component
        Reference< XInterface > xRet;
        xRet = ::frm::OFormsModule::getComponentFactory(
            OUString::createFromAscii( _pImplName ),
            static_cast< XMultiServiceFactory* >( _pServiceManager ) );

        if ( xRet.is() )
            xRet->acquire();
        pRet = xRet.get();
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
