/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CMIS_CONTENT_HXX
#define CMIS_CONTENT_HXX

#include "cmis_url.hxx"
#include "children_provider.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/CheckinArgument.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/document/CmisVersion.hpp>
#include <ucbhelper/contenthelper.hxx>
#include <libcmis/libcmis.hxx>

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

#define CMIS_FILE_TYPE   "application/vnd.libreoffice.cmis-file"
#define CMIS_FOLDER_TYPE "application/vnd.libreoffice.cmis-folder"

class ContentProvider;
class Content : public ::ucbhelper::ContentImplHelper,
                public com::sun::star::ucb::XContentCreator,
                public ChildrenProvider
{
private:
    ContentProvider*       m_pProvider;
    libcmis::Session*      m_pSession;
    libcmis::ObjectPtr     m_pObject;
    OUString          m_sObjectPath;
    OUString          m_sObjectId;
    OUString          m_sURL;
    cmis::URL              m_aURL;

    // Members to be set for non-persistent content
    bool                   m_bTransient;
    bool                   m_bIsFolder;
    libcmis::ObjectTypePtr m_pObjectType;
    std::map< std::string, libcmis::PropertyPtr > m_pObjectProps;

    bool isFolder( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );
    void setCmisProperty( std::string sName, std::string sValue,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

    com::sun::star::uno::Any getBadArgExcept();

    com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
        getPropertyValues(
            const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& rProperties,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

    libcmis::Session* getSession( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );
    libcmis::ObjectTypePtr getObjectType( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

private:
    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;

    com::sun::star::uno::Any open(const com::sun::star::ucb::OpenCommandArgument2 & rArg,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    void transfer( const com::sun::star::ucb::TransferInfo& rTransferInfo,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    void insert( const com::sun::star::uno::Reference< com::sun::star::io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting, const OUString & rMimeType,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv )
            throw( com::sun::star::uno::Exception );

    OUString checkIn( const com::sun::star::ucb::CheckinArgument& rArg,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    OUString checkOut( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    OUString cancelCheckOut( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    void destroy( ) throw( com::sun::star::uno::Exception );

    void copyData( com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xIn,
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xOut );

    com::sun::star::uno::Sequence< com::sun::star::uno::Any >
        setPropertyValues( const com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyValue >& rValues,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment >& xEnv );

    com::sun::star::uno::Sequence< com::sun::star::document::CmisVersion >
        getAllVersions( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > & xEnv )
            throw( com::sun::star::uno::Exception );

    sal_Bool feedSink( com::sun::star::uno::Reference< com::sun::star::uno::XInterface> aSink,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv );

public:
    Content( const com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier,
        libcmis::ObjectPtr pObject = libcmis::ObjectPtr( ) )
            throw ( com::sun::star::ucb::ContentCreationException );

    Content( const com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >& rxContext, ContentProvider *pProvider,
        const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier,
        sal_Bool bIsFolder)
            throw ( com::sun::star::ucb::ContentCreationException );

    virtual ~Content();

    virtual com::sun::star::uno::Sequence< com::sun::star::beans::Property >
        getProperties( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual libcmis::ObjectPtr updateProperties(
            const com::sun::star::uno::Any& iCmisProps,
            const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv);

    virtual com::sun::star::uno::Sequence< com::sun::star::ucb::CommandInfo >
        getCommands( const com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    virtual OUString getParentURL();

    XINTERFACE_DECL()

    XTYPEPROVIDER_DECL()

    virtual OUString SAL_CALL
    getImplementationName()
            throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );

    virtual OUString SAL_CALL
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

    virtual std::list< com::sun::star::uno::Reference< com::sun::star::ucb::XContent > > getChildren( );

    libcmis::ObjectPtr getObject( const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment >& xEnv ) throw ( libcmis::Exception );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
