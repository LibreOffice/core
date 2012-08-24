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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase2.hxx>

#include "fastparser.hxx"

using namespace sax_fastparser;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using ::rtl::OUString;
using namespace ::com::sun::star::lang;

namespace sax_fastparser
{

Reference< XInterface > SAL_CALL FastSaxParser_CreateInstance(
    SAL_UNUSED_PARAMETER const Reference< XMultiServiceFactory > & )
    throw(Exception)
{
    FastSaxParser *p = new FastSaxParser;
    return Reference< XInterface > ( (OWeakObject * ) p );
}

}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL fastsax_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr( reinterpret_cast< XMultiServiceFactory * > ( pServiceManager ) );

        OUString aImplementationName( OUString::createFromAscii( pImplName ) );

        if ( aImplementationName == PARSER_IMPLEMENTATION_NAME  )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        FastSaxParser_CreateInstance,
                                        FastSaxParser::getSupportedServiceNames_Static() );
        }

        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
