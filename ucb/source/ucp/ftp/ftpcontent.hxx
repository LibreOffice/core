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

#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include "ftpurl.hxx"

namespace com::sun::star::beans {
    struct Property;
    struct PropertyValue;
}

namespace com::sun::star::sdbc {
    class XRow;
}


namespace ftp
{

class FTPContentProvider;

class FTPContent : public ::ucbhelper::ContentImplHelper,
                   public css::ucb::XContentCreator
{
public:

    FTPContent( const css::uno::Reference<
                css::uno::XComponentContext >& rxContext,
                FTPContentProvider* pProvider,
                const css::uno::Reference<
                css::ucb::XContentIdentifier >& Identifier,
                const FTPURL& FtpUrl);

    FTPContent( const css::uno::Reference<
                css::uno::XComponentContext >& rxContext,
                FTPContentProvider* pProvider,
                const css::uno::Reference<
                css::ucb::XContentIdentifier >& Identifier,
                const css::ucb::ContentInfo& aInfo);


    virtual ~FTPContent() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContent
    virtual OUString SAL_CALL getContentType() override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand,
                                            sal_Int32 CommandId,
                                            const css::uno::Reference<
                                            css::ucb::XCommandEnvironment >& Environment ) override;

    virtual void SAL_CALL abort(sal_Int32 CommandId) override;

    // XContentCreator
    virtual css::uno::Sequence<
        css::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(  ) override;

    virtual css::uno::Reference<
        css::ucb::XContent > SAL_CALL
        createNewContent( const css::ucb::ContentInfo& Info ) override;

    // XChild

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;

    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    /// @throws css::uno::RuntimeException
    static css::uno::Sequence< css::ucb::ContentInfo > queryCreatableContentsInfo_Static();

private:

    FTPContentProvider *m_pFCP;
    FTPURL              m_aFTPURL;
    bool                m_bInserted;
    bool                m_bTitleSet;
    css::ucb::ContentInfo m_aInfo;

    virtual css::uno::Sequence< css::beans::Property >
        getProperties( const css::uno::Reference<
                       css::ucb::XCommandEnvironment > & xEnv ) override;


    virtual css::uno::Sequence< css::ucb::CommandInfo>
        getCommands(const css::uno::Reference<
                    css::ucb::XCommandEnvironment > & xEnv) override;


    virtual OUString getParentURL() override;

    css::uno::Reference<css::sdbc::XRow>
        getPropertyValues(
                const css::uno::Sequence<
                css::beans::Property>& seqProp
            );

    css::uno::Sequence<css::uno::Any>
        setPropertyValues( const css::uno::Sequence<
                           css::beans::PropertyValue>& seqPropVal);

    void insert(const css::ucb::InsertCommandArgument&,
                const css::uno::Reference<
                css::ucb::XCommandEnvironment>&);
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
