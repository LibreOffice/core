/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _LAYOUT_UNO_HXX
#define _LAYOUT_UNO_HXX

#include <cstdio>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <layout/core/root.hxx>
#include <layout/core/factory.hxx>

#if LAYOUT_WEAK
#include <cppuhelper/implbase1.hxx>
class UnoBootstrapLayout : public ::cppu::WeakImplHelper1< com::sun::star::lang::XMultiServiceFactory >
#else /* !LAYOUT_WEAK */
class UnoBootstrapLayout : public com::sun::star::lang::XMultiServiceFactory
#endif /* LAYOUT_WEAK */
{
public:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    UnoBootstrapLayout( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xMSF )
        : mxMSF( xMSF )
    {
        fprintf( stderr, "UnoBootstrap Layout\n" );
    }
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    createInstance( const rtl::OUString& aServiceSpecifier ) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
    {
        if ( aServiceSpecifier.equalsAscii( "com.sun.star.awt.Layout" ) )
        {
            fprintf( stderr, "UnoBootstrapLayout: create service '%s'\n",
                     rtl::OUStringToOString (aServiceSpecifier, RTL_TEXTENCODING_UTF8 ).getStr() );
            return com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory >( new ::LayoutFactory( this ) );
        }
        else
        {
            fprintf( stderr, "UnoBootstrapLayout: create service '%s'\n",
                     rtl::OUStringToOString (aServiceSpecifier, RTL_TEXTENCODING_UTF8 ).getStr() );
            try
            {
                return mxMSF->createInstance( aServiceSpecifier );
            }
            catch ( const com::sun::star::uno::Exception &rExc )
            {
                rtl::OString aStr( rtl::OUStringToOString( rExc.Message, RTL_TEXTENCODING_ASCII_US ) );
                fprintf( stderr, "service construction exception '%s'\n", aStr.getStr());
                throw rExc;
            }
        }
    }
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
    createInstanceWithArguments( const rtl::OUString& ServiceSpecifier, const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& Arguments ) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
    {
        return mxMSF->createInstanceWithArguments( ServiceSpecifier, Arguments );
    }
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getAvailableServiceNames() throw (com::sun::star::uno::RuntimeException)
    {
        return mxMSF->getAvailableServiceNames();
    }

#if !LAYOUT_WEAK
    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
    {
        return mxMSF->queryInterface( rType );
    }
    virtual void SAL_CALL acquire() throw()
    {
        mxMSF->acquire();
    }
    virtual void SAL_CALL release() throw()
    {
        mxMSF->release();
    }
#endif /* !LAYOUT_WEAK */
};

#endif /* _LAYOUT_UNO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
