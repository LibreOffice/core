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

#ifndef _CONTENT_HXX
#define _CONTENT_HXX

#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#include "urlparameter.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }

namespace chelp
{

//=========================================================================

// UNO service name for the content.
#define MYUCP_CONTENT_SERVICE_NAME \
                            "com.sun.star.ucb.CHelpContent"

//=========================================================================

    class Databases;

    struct ContentProperties
    {
        OUString aTitle;         // Title
        OUString aContentType;   // ContentType
        sal_Bool        bIsDocument;    // IsDocument
        sal_Bool        bIsFolder;      // IsFolder

        ContentProperties()
            : bIsDocument( sal_True ), bIsFolder( sal_False ) {}
    };

//=========================================================================

    class Content : public ::ucbhelper::ContentImplHelper
    {
    public:

        Content( const ::com::sun::star::uno::Reference<
                 ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                 ::ucbhelper::ContentProviderImplHelper* pProvider,
                 const ::com::sun::star::uno::Reference<
                 ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                 Databases* pDatabases );

        virtual ~Content();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException );

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException );

        // XContent
        virtual rtl::OUString SAL_CALL
        getContentType()
            throw( com::sun::star::uno::RuntimeException );

        // XCommandProcessor
        virtual com::sun::star::uno::Any SAL_CALL
        execute( const com::sun::star::ucb::Command& aCommand,
                 sal_Int32 CommandId,
                 const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::ucb::CommandAbortedException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL
        abort( sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );


    private:

        // private members;

        ContentProperties m_aProps;
        URLParameter      m_aURLParameter;
        Databases*        m_pDatabases;


        // private methods

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv );
        virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
        getCommands( const com::sun::star::uno::Reference<
                     com::sun::star::ucb::XCommandEnvironment > & xEnv );

        virtual OUString getParentURL() { return OUString(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
        getPropertyValues( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::beans::Property >& rProperties );
        void setPropertyValues(
            const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& rValues );


    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
