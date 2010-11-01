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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef _FTP_FTPCONTENTIDENTIFIER_HXX_
#define _FTP_FTPCONTENTIDENTIFIER_HXX_

#include <vector>
#include "curl.hxx"
#include <curl/easy.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include "ftpdirp.hxx"
#include "ftpurl.hxx"


namespace ftp {


    class FTPContentProvider;


    class FTPContentIdentifier
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::ucb::XContentIdentifier
    {
    public:

        FTPContentIdentifier(const rtl::OUString& ident);

        ~FTPContentIdentifier();

        // XInterface

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface( const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL acquire( void ) throw();

        virtual void SAL_CALL release( void ) throw();

        // XTypeProvider

        virtual
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL
        getTypes(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );

        virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(
        )
            throw(
                com::sun::star::uno::RuntimeException
            );


        // XContentIdentifier

        virtual ::rtl::OUString SAL_CALL
        getContentIdentifier(
        )
            throw (
                com::sun::star::uno::RuntimeException
            );

        virtual ::rtl::OUString SAL_CALL
        getContentProviderScheme(
        )
            throw (
                ::com::sun::star::uno::RuntimeException
            );


    private:

        rtl::OUString m_ident;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
