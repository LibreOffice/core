/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include "mcnttfactory.hxx"






#define MIMECONTENTTYPEFACTORY_SERVICE_NAME  "com.sun.star.datatransfer.MimeContentTypeFactory"


#define MIMECONTENTTYPEFACTORY_IMPL_NAME  "com.sun.star.datatransfer.MimeCntTypeFactory"





using namespace ::rtl                       ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::registry  ;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;





namespace
{

    
    
    
    
    

    Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >&  )
    {
        return Reference< XInterface >( static_cast< XMimeContentTypeFactory* >( new CMimeContentTypeFactory() ) );
    }
}

extern "C"
{





SAL_DLLPUBLIC_EXPORT void* SAL_CALL mcnttype_component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, MIMECONTENTTYPEFACTORY_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( MIMECONTENTTYPEFACTORY_SERVICE_NAME );

        Reference< XSingleServiceFactory > xFactory ( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createInstance,
            aSNS ) );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
