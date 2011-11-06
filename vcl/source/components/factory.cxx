/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>
#ifndef _OSL_MUTEX_HXX
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX
#include <rtl/ustrbuf.hxx>
#endif
#include <uno/dispatcher.h> // declaration of generic uno interface
#include <uno/mapping.hxx> // mapping stuff
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/dllapi.h>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

// service implementation
extern Sequence< OUString > SAL_CALL vcl_session_getSupportedServiceNames();
extern OUString SAL_CALL vcl_session_getImplementationName();
extern Reference< XInterface > SAL_CALL vcl_session_createInstance( const Reference< XMultiServiceFactory > & );

namespace vcl
{
extern Sequence< OUString > SAL_CALL DisplayAccess_getSupportedServiceNames();
extern OUString SAL_CALL DisplayAccess_getImplementationName();
extern Reference< XInterface > SAL_CALL DisplayAccess_createInstance( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL FontIdentificator_getSupportedServiceNames();
extern OUString SAL_CALL FontIdentificator_getImplementationName();
extern Reference< XInterface > SAL_CALL FontIdentificator_createInstance( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL StringMirror_getSupportedServiceNames();
extern OUString SAL_CALL StringMirror_getImplementationName();
extern Reference< XInterface > SAL_CALL StringMirror_createInstance( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL Clipboard_getSupportedServiceNames();
extern OUString SAL_CALL Clipboard_getImplementationName();
extern Reference< XSingleServiceFactory > SAL_CALL Clipboard_createFactory( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL DragSource_getSupportedServiceNames();
extern OUString SAL_CALL DragSource_getImplementationName();
extern Reference< XInterface > SAL_CALL DragSource_createInstance( const Reference< XMultiServiceFactory > & );

extern Sequence< OUString > SAL_CALL DropTarget_getSupportedServiceNames();
extern OUString SAL_CALL DropTarget_getImplementationName();
extern Reference< XInterface > SAL_CALL DropTarget_createInstance( const Reference< XMultiServiceFactory > & );

namespace rsvg
{
    extern Sequence< OUString > SAL_CALL Rasterizer_getSupportedServiceNames();
    extern OUString SAL_CALL Rasterizer_getImplementationName();
    extern Reference< XInterface > SAL_CALL Rasterizer_createInstance( const Reference< XMultiServiceFactory > & );
}
}

extern "C" {

    VCL_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment(
        const sal_Char** ppEnvTypeName,
        uno_Environment** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    VCL_DLLPUBLIC void* SAL_CALL component_getFactory(
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
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl_session_getImplementationName(), vcl_session_createInstance,
                    vcl_session_getSupportedServiceNames() );
            }
            else if( vcl::DisplayAccess_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::DisplayAccess_getImplementationName(), vcl::DisplayAccess_createInstance,
                    vcl::DisplayAccess_getSupportedServiceNames() );
            }
            else if( vcl::FontIdentificator_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::FontIdentificator_getImplementationName(), vcl::FontIdentificator_createInstance,
                    vcl::FontIdentificator_getSupportedServiceNames() );
            }
            else if( vcl::StringMirror_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::StringMirror_getImplementationName(), vcl::StringMirror_createInstance,
                    vcl::StringMirror_getSupportedServiceNames() );
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
            else if( vcl::rsvg::Rasterizer_getImplementationName().equalsAscii( pImplementationName ) )
            {
                xFactory = ::cppu::createSingleFactory(
                    xMgr, vcl::rsvg::Rasterizer_getImplementationName(), vcl::rsvg::Rasterizer_createInstance,
                    vcl::rsvg::Rasterizer_getSupportedServiceNames() );
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
