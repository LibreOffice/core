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

//=========================================================================

// UNO service name for the content.
#define FTP_CONTENT_SERVICE_NAME "com.sun.star.ucb.FTPContent"

//=========================================================================

    class FTPContentProvider;

//=========================================================================

    class FTPContent
        : public ::ucbhelper::ContentImplHelper,
          public com::sun::star::ucb::XContentCreator
    {
    public:

        FTPContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                    FTPContentProvider* pProvider,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                    const FTPURL& FtpUrl);

        FTPContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                    FTPContentProvider* pProvider,
                    const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier,
                    const com::sun::star::ucb::ContentInfo& aInfo);


        virtual ~FTPContent();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        XSERVICEINFO_DECL()

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
        abort(sal_Int32 CommandId)
            throw( com::sun::star::uno::RuntimeException);

        // XContentCreator
        virtual com::sun::star::uno::Sequence<
        com::sun::star::ucb::ContentInfo > SAL_CALL
        queryCreatableContentsInfo(  )
            throw (com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XContent > SAL_CALL
        createNewContent( const com::sun::star::ucb::ContentInfo& Info )
            throw (com::sun::star::uno::RuntimeException);

        // XChild

        virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > SAL_CALL
        getParent(  )
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        setParent( const ::com::sun::star::uno::Reference<
                   ::com::sun::star::uno::XInterface >& Parent )
            throw (::com::sun::star::lang::NoSupportException,
                   ::com::sun::star::uno::RuntimeException);


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


        virtual ::rtl::OUString getParentURL();

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
