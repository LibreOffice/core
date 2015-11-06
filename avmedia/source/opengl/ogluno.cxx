/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "oglmanager.hxx"

#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace ::com::sun::star;

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
    return uno::Reference< uno::XInterface >( *new ::avmedia::ogl::OGLManager( rxFact ) );
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL avmediaogl_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* )
{
    uno::Reference< lang::XSingleServiceFactory > xFactory;
    void* pRet = 0;

    if( rtl_str_compare( pImplName, "com.sun.star.comp.avmedia.Manager_OpenGL" ) == 0 )
    {
        xFactory.set( ::cppu::createSingleFactory(
                        static_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                        "com.sun.star.comp.avmedia.Manager_OpenGL",
                        create_MediaPlayer, uno::Sequence< OUString >{ "com.sun.star.media.Manager_OpenGL" } ) );
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
