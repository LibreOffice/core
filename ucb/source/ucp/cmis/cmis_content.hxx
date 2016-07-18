/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_CONTENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CMIS_CONTENT_HXX

#include "cmis_url.hxx"
#include "children_provider.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/CheckinArgument.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
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
                public css::ucb::XContentCreator,
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

    bool isFolder( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );
    void setCmisProperty(const std::string& rName, const std::string& rValue,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    css::uno::Any getBadArgExcept();

    css::uno::Reference< css::sdbc::XRow >
        getPropertyValues(
            const css::uno::Sequence< css::beans::Property >& rProperties,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    libcmis::Session* getSession( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );
    libcmis::ObjectTypePtr const & getObjectType( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

private:
    typedef rtl::Reference< Content > ContentRef;
    typedef std::list< ContentRef > ContentRefList;

    css::uno::Any open(const css::ucb::OpenCommandArgument2 & rArg,
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw( css::uno::Exception, libcmis::Exception );

    void transfer( const css::ucb::TransferInfo& rTransferInfo,
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw( css::uno::Exception );

    void insert( const css::uno::Reference< css::io::XInputStream > & xInputStream,
        bool bReplaceExisting, const OUString & rMimeType,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
            throw (css::uno::Exception, std::exception);

    OUString checkIn( const css::ucb::CheckinArgument& rArg,
        const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw( css::uno::Exception );

    OUString checkOut( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw( css::uno::Exception );

    OUString cancelCheckOut( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw( css::uno::Exception );

    static void copyData( const css::uno::Reference< css::io::XInputStream >& xIn,
        const css::uno::Reference< css::io::XOutputStream >& xOut );

    css::uno::Sequence< css::uno::Any >
        setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
            const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    css::uno::Sequence< css::document::CmisVersion >
        getAllVersions( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv )
            throw(css::uno::Exception,
                  std::exception);

    bool feedSink( const css::uno::Reference< css::uno::XInterface>& aSink,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

public:
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ContentProvider *pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
        libcmis::ObjectPtr pObject = libcmis::ObjectPtr( ) )
            throw ( css::ucb::ContentCreationException );

    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ContentProvider *pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
        bool bIsFolder)
            throw ( css::ucb::ContentCreationException );

    virtual ~Content();

    virtual css::uno::Sequence< css::beans::Property >
        getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;

    libcmis::ObjectPtr updateProperties(
            const css::uno::Any& iCmisProps,
            const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv);

    virtual css::uno::Sequence< css::ucb::CommandInfo >
        getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;

    virtual OUString getParentURL() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
    getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL
    getContentType()
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Any SAL_CALL
        execute( const css::ucb::Command& aCommand,
        sal_Int32 CommandId,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment )
            throw( css::uno::Exception, css::ucb::CommandAbortedException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL abort( sal_Int32 CommandId )
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< css::ucb::ContentInfo >
        SAL_CALL queryCreatableContentsInfo()
            throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::ucb::XContent >
        SAL_CALL createNewContent( const css::ucb::ContentInfo& Info )
            throw( css::uno::RuntimeException, std::exception ) override;

    css::uno::Sequence< css::ucb::ContentInfo >
        queryCreatableContentsInfo( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv )
                throw( css::uno::RuntimeException );

    virtual std::list< css::uno::Reference< css::ucb::XContent > > getChildren( ) override;

    libcmis::ObjectPtr const & getObject( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv ) throw (css::uno::RuntimeException, css::ucb::CommandFailedException, libcmis::Exception);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
