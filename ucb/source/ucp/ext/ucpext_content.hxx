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

#pragma once

#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <ucbhelper/contenthelper.hxx>

#include <optional>


namespace ucb::ucp::ext
{


    //= ExtensionContentType

    enum ExtensionContentType
    {
        E_ROOT,
        E_EXTENSION_ROOT,
        E_EXTENSION_CONTENT,

        E_UNKNOWN
    };


    //= ContentProvider

    typedef ::ucbhelper::ContentImplHelper  Content_Base;
    class Content : public Content_Base
    {
    public:
        Content(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            ::ucbhelper::ContentProviderImplHelper* pProvider,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier
        );

        static css::uno::Reference< css::sdbc::XRow >
            getArtificialNodePropertyValues(
                const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                const css::uno::Sequence< css::beans::Property >& rProperties,
                const OUString& rTitle
            );

        css::uno::Reference< css::sdbc::XRow >
            getPropertyValues(
                const css::uno::Sequence< css::beans::Property >& rProperties,
                const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv
            );

        static OUString
            encodeIdentifier( const OUString& i_rIdentifier );
        static OUString
            decodeIdentifier( const OUString& i_rIdentifier );

        virtual OUString getParentURL() override;

        ExtensionContentType getExtensionContentType() const { return m_eExtContentType; }

        /** retrieves the URL of the underlying physical content. Not to be called when getExtensionContentType()
            returns E_ROOT.
        */
        OUString getPhysicalURL() const;

    protected:
        virtual ~Content() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XContent
        virtual OUString SAL_CALL getContentType() override;

        // XCommandProcessor
        virtual css::uno::Any SAL_CALL
                execute(
                    const css::ucb::Command& aCommand,
                    sal_Int32 CommandId,
                    const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment
                ) override;

        virtual void SAL_CALL
                abort(
                    sal_Int32 CommandId
                ) override;

    private:
        virtual css::uno::Sequence< css::beans::Property > getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment >& i_rEnv ) override;
        virtual css::uno::Sequence< css::ucb::CommandInfo > getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment >& i_rEnv ) override;

        css::uno::Sequence< css::uno::Any >
            setPropertyValues(
                const css::uno::Sequence< css::beans::PropertyValue >& rValues
            );

        static bool denotesRootContent( const OUString& i_rContentIdentifier );

        bool impl_isFolder();
        void impl_determineContentType();

    private:
        ExtensionContentType                    m_eExtContentType;
        ::std::optional< bool >               m_aIsFolder;
        ::std::optional< OUString >    m_aContentType;
        OUString                         m_sExtensionId;
        OUString                         m_sPathIntoExtension;
    };


} // namespace ucb::ucp::ext


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
