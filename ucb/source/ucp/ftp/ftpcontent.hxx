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

#ifndef _FTP_FTPCONTENT_HXX
#define _FTP_FTPCONTENT_HXX

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



    class FTPContent
        : public ::ucbhelper::ContentImplHelper,
          public com::sun::star::ucb::XContentCreator
    {
    public:

        FTPContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext >& rxContext,
                    FTPContentProvider* pProvider,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                    const FTPURL& FtpUrl);

        FTPContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext >& rxContext,
                    FTPContentProvider* pProvider,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                    const com::sun::star::ucb::ContentInfo& aInfo);


        virtual ~FTPContent();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
            throw( css::uno::RuntimeException, std::exception );
        virtual void SAL_CALL acquire()
            throw();
        virtual void SAL_CALL release()
            throw();

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        XSERVICEINFO_DECL()

        // XContent
        virtual OUString SAL_CALL
        getContentType()
            throw( com::sun::star::uno::RuntimeException, std::exception );

        // XCommandProcessor
        virtual com::sun::star::uno::Any SAL_CALL
        execute( const com::sun::star::ucb::Command& aCommand,
                 sal_Int32 CommandId,
                 const com::sun::star::uno::Reference<
                 com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::ucb::CommandAbortedException,
                   com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL
        abort(sal_Int32 CommandId)
            throw( com::sun::star::uno::RuntimeException, std::exception);

        // XContentCreator
        virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(  )
            throw (com::sun::star::uno::RuntimeException, std::exception);

        virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
        createNewContent( const com::sun::star::ucb::ContentInfo& Info )
            throw (com::sun::star::uno::RuntimeException, std::exception);

        // XChild

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        getParent(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception);

        virtual void SAL_CALL
        setParent( const ::com::sun::star::uno::Reference<
                   ::com::sun::star::uno::XInterface >& Parent )
            throw (::com::sun::star::lang::NoSupportException,
                   ::com::sun::star::uno::RuntimeException, std::exception);


        static com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentInfo >
        queryCreatableContentsInfo_Static(  )
            throw (com::sun::star::uno::RuntimeException);

    private:

        FTPContentProvider *m_pFCP;
        FTPURL              m_aFTPURL;
        bool                m_bInserted;
        bool                m_bTitleSet;
        com::sun::star::ucb::ContentInfo m_aInfo;

        virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
                       com::sun::star::ucb::XCommandEnvironment > & xEnv );


        virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::CommandInfo>
        getCommands(const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment > & xEnv);


        virtual OUString getParentURL();

        com::sun::star::uno::Reference<com::sun::star::sdbc::XRow>
        getPropertyValues(
            const com::sun::star::uno::Sequence<
            com::sun::star::beans::Property>& seqProp,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& Environment
        );

        com::sun::star::uno::Sequence<com::sun::star::uno::Any>
        setPropertyValues(
            const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue>& seqPropVal);

        void insert(const com::sun::star::ucb::InsertCommandArgument&,
                    const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XCommandEnvironment>&);
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
