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

#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_CONTENT_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_PROVIDER_CONTENT_HXX

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

    class Databases;

    class Content : public ::ucbhelper::ContentImplHelper
    {
    public:

        Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                 ::ucbhelper::ContentProviderImplHelper* pProvider,
                 const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
                 Databases* pDatabases );

        virtual ~Content();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL acquire()
            throw() override;
        virtual void SAL_CALL release()
            throw() override;

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
            throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
            throw( css::uno::RuntimeException, std::exception ) override;

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

        // XContent
        virtual OUString SAL_CALL
        getContentType()
            throw( css::uno::RuntimeException, std::exception ) override;

        // XCommandProcessor
        virtual css::uno::Any SAL_CALL
        execute( const css::ucb::Command& aCommand,
                 sal_Int32 CommandId,
                 const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
            throw( css::uno::Exception,
                   css::ucb::CommandAbortedException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        abort( sal_Int32 CommandId )
            throw( css::uno::RuntimeException, std::exception ) override;

    private:

        // private members;

        URLParameter      m_aURLParameter;
        Databases*        m_pDatabases;

        // private methods

        virtual css::uno::Sequence< css::beans::Property >
        getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
        virtual css::uno::Sequence< css::ucb::CommandInfo >
        getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;

        virtual OUString getParentURL() override { return OUString(); }

        css::uno::Reference< css::sdbc::XRow >
        getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
