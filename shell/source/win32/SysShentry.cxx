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
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>
#include "SysShExec.hxx"

//-----------------------------------------------------------------------
// namespace directives
//-----------------------------------------------------------------------

using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using com::sun::star::system::XSystemShellExecute;

#define SYSSHEXEC_SERVICE_NAME  "com.sun.star.system.SystemShellExecute"
#define SYSSHEXEC_IMPL_NAME     "com.sun.star.system.SystemShellExecute"

//-----------------------------------------------------------------------

namespace
{
    Reference< XInterface > SAL_CALL createInstance( const Reference< XComponentContext >& xContext )
    {
        return Reference< XInterface >( static_cast< XSystemShellExecute* >( new CSysShExec(xContext) ) );
    }
}

extern "C"
{
//----------------------------------------------------------------------
// component_getFactory
// returns a factory to create XFilePicker-Services
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL syssh_component_getFactory( const sal_Char* pImplName, uno_Interface*, uno_Interface* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( 0 == rtl_str_compare( pImplName, SYSSHEXEC_IMPL_NAME ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString(SYSSHEXEC_SERVICE_NAME );

        Reference< XSingleComponentFactory > xFactory ( createSingleComponentFactory(
            createInstance,
            OUString::createFromAscii( pImplName ),
            aSNS ) );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
