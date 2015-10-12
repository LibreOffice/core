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

#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENT_HXX

#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include "ftpurl.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
    struct PropertyValue;
} } } }

namespace com { namespace sun { namespace star { namespace sdbc {
    class XRow;
} } } }


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


    virtual ~FTPContent();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException,
               std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException,
                   std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XContent
    virtual OUString SAL_CALL getContentType()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL execute( const css::ucb::Command& aCommand,
                                            sal_Int32 CommandId,
                                            const css::uno::Reference<
                                            css::ucb::XCommandEnvironment >& Environment )
        throw( css::uno::Exception,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL abort(sal_Int32 CommandId)
        throw( css::uno::RuntimeException, std::exception) override;

    // XContentCreator
    virtual css::uno::Sequence<
        css::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(  )
            throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<
        css::ucb::XContent > SAL_CALL
        createNewContent( const css::ucb::ContentInfo& Info )
            throw (css::uno::RuntimeException, std::exception) override;

    // XChild

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent )
        throw (css::lang::NoSupportException,
               css::uno::RuntimeException, std::exception) override;

    static css::uno::Sequence< css::ucb::ContentInfo > queryCreatableContentsInfo_Static()
        throw (css::uno::RuntimeException);

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
                css::beans::Property>& seqProp,
                const css::uno::Reference<
                css::ucb::XCommandEnvironment >& Environment
            );

    css::uno::Sequence<css::uno::Any>
        setPropertyValues( const css::uno::Sequence<
                           css::beans::PropertyValue>& seqPropVal);

    void insert(const css::ucb::InsertCommandArgument&,
                const css::uno::Reference<
                css::ucb::XCommandEnvironment>&);
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
