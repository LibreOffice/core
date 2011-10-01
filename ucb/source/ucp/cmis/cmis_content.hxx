/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef CMIS_CONTENT_HXX
#define CMIS_CONTENT_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <ucbhelper/contenthelper.hxx>
#include <libcmis/session-factory.hxx>

#include <list>

namespace com { namespace sun { namespace star {
    namespace beans {
        struct Property;
        struct PropertyValue;
    }
    namespace sdbc {
        class XRow;
    }
}}}
namespace ucbhelper
{
    class Content;
}


namespace cmis
{

#define CMIS_FILE_TYPE   "application/vnd.sun.staroffice.cmis-file"
#define CMIS_FOLDER_TYPE "application/vnd.sun.staroffice.cmis-folder"

class ContentProvider;
class ContentProperties;
class Content : public ::ucbhelper::ContentImplHelper, public com::sun::star::ucb::XContentCreator
{
private:
    ContentProvider*       m_pProvider;
    libcmis::Session*      m_pSession;
    libcmis::CmisObjectPtr m_pObject;

    bool isFolder(const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv);

    com::sun::star::uno::Any getBadArgExcept();

    com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
        getPropertyValues(
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& rProperties,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );
private:
    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;

    void queryChildren( ContentRefList& rChildren );

    com::sun::star::uno::Any open(const com::sun::star::ucb::OpenCommandArgument2 & rArg,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    void transfer( const com::sun::star::ucb::TransferInfo& rTransferInfo,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    void insert( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting, const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv )
            throw( com::sun::star::uno::Exception );

    void destroy( sal_Bool bDeletePhysical ) throw( com::sun::star::uno::Exception );

    void copyData( com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xIn,
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xOut );

    com::sun::star::uno::Sequence< com::sun::star::uno::Any >
        setPropertyValues( const com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyValue >& rValues,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv );

    sal_Bool feedSink( com::sun::star::uno::Reference< com::sun::star::uno::XInterface> aSink,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

    sal_Bool exchangeIdentity(const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >&  xNewId);

public:
    Content( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxSMgr, ContentProvider *pProvider,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier)
            throw ( com::sun::star::ucb::ContentCreationException );

    Content( const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >& rxSMgr, ContentProvider *pProvider,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier,
        sal_Bool bIsFolder)
            throw ( com::sun::star::ucb::ContentCreationException );

    virtual ~Content();

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
        getCommands( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual ::rtl::OUString getParentURL();

    XINTERFACE_DECL()

    XTYPEPROVIDER_DECL()

    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL
    getContentType()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Any SAL_CALL
        execute( const com::sun::star::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& Environment )
            throw( com::sun::star::uno::Exception, com::sun::star::ucb::CommandAbortedException, com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL abort( sal_Int32 CommandId )
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
        SAL_CALL queryCreatableContentsInfo()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
        SAL_CALL createNewContent( const com::sun::star::ucb::ContentInfo& Info )
            throw( com::sun::star::uno::RuntimeException );

    com::sun::star::uno::Sequence< com::sun::star::ucb::ContentInfo >
        queryCreatableContentsInfo( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv )
                throw( com::sun::star::uno::RuntimeException );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
