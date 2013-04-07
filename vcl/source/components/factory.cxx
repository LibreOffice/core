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

#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/dllapi.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

// service implementation
extern Sequence< OUString > SAL_CALL vcl_session_getSupportedServiceNames();
extern OUString SAL_CALL vcl_session_getImplementationName();
extern Reference< XInterface > SAL_CALL vcl_session_createInstance( const Reference< XMultiServiceFactory > & );

namespace vcl
{
extern Sequence< OUString > SAL_CALL FontIdentificator_getSupportedServiceNames();
extern OUString SAL_CALL FontIdentificator_getImplementationName();
extern Reference< XInterface > SAL_CALL FontIdentificator_createInstance( const Reference< XMultiServiceFactory > & );

extern OUString SAL_CALL Clipboard_getImplementationName();
extern Reference< XSingleServiceFactory > SAL_CALL Clipboard_createFactory( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL DragSource_getSupportedServiceNames();
extern OUString SAL_CALL DragSource_getImplementationName();
extern Reference< XInterface > SAL_CALL DragSource_createInstance( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL DropTarget_getSupportedServiceNames();
extern OUString SAL_CALL DropTarget_getImplementationName();
extern Reference< XInterface > SAL_CALL DropTarget_createInstance( const Reference< XMultiServiceFactory > & );

}

extern "C" {

    VCL_DLLPUBLIC void* SAL_CALL vcl_component_getFactory(
        const sal_Char* pImplementationName,
        void* pXUnoSMgr,
        void* /*pXUnoKey*/
        )
    {
        void* pRet = 0;

        if( pXUnoSMgr )
        {
            Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr(
                reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pXUnoSMgr )
                );
            Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory;
            if( vcl_session_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createOneInstanceFactory(
                    xMgr, vcl_session_getImplementationName(), vcl_session_createInstance,
                    vcl_session_getSupportedServiceNames() );
            }
            else if( vcl::FontIdentificator_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::FontIdentificator_getImplementationName(), vcl::FontIdentificator_createInstance,
                    vcl::FontIdentificator_getSupportedServiceNames() );
            }
            else if( vcl::Clipboard_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = vcl::Clipboard_createFactory( xMgr );
            }
            else if( vcl::DragSource_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::DragSource_getImplementationName(), vcl::DragSource_createInstance,
                    vcl::DragSource_getSupportedServiceNames() );
            }
            else if( vcl::DropTarget_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::DropTarget_getImplementationName(), vcl::DropTarget_createInstance,
                    vcl::DropTarget_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }

} /* extern "C" */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
