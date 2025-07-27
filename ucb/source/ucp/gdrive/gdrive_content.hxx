/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vector>
#include <rtl/ref.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <ucbhelper/contenthelper.hxx>
#include "gdrive_json.hxx"

namespace com::sun::star::beans {
    struct Property;
    struct PropertyValue;
}

namespace com::sun::star::sdbc {
    class XRow;
}

namespace com::sun::star::ucb {
    struct OpenCommandArgument3;
    struct PropertyCommandArgument;
    struct TransferInfo;
}

namespace ucp {
namespace gdrive {

class ContentProvider;
class GoogleDriveApiClient;

// UNO service name for the content.
inline constexpr OUString GDRIVE_CONTENT_SERVICE_NAME = u"com.sun.star.ucb.GoogleDriveContent"_ustr;

// Using GDriveFileInfo from gdrive_json.hxx

class Content : public ::ucbhelper::ContentImplHelper,
                public css::ucb::XContentCreator
{
    enum ResourceType
    {
        UNKNOWN,      // the type of the resource is unknown
        NOT_FOUND,    // the resource does not exist
        FILE,         // the resource is a regular file
        FOLDER        // the resource is a folder
    };

    std::shared_ptr<GoogleDriveApiClient> m_pApiClient;
    ContentProvider*  m_pProvider; // No need for a ref, base class holds object
    GDriveFileInfo    m_aFileInfo; // Cached file information
    ResourceType      m_eResourceType;
    bool              m_bTransient;
    rtl::OUString     m_sParentId;

private:
    virtual css::uno::Sequence< css::beans::Property >
    getProperties( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual css::uno::Sequence< css::ucb::CommandInfo >
    getCommands( const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv ) override;
    virtual OUString getParentURL() override;

    /// @throws css::uno::Exception
    bool isFolder( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Reference< css::sdbc::XRow >
    getPropertyValues( const css::uno::Sequence< css::beans::Property >& rProperties,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Sequence< css::uno::Any >
    setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& rValues,
                       const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    typedef rtl::Reference< Content > ContentRef;
    typedef std::vector< ContentRef > ContentRefList;
    void queryChildren( ContentRefList& rChildren);

    static void copyData( const css::uno::Reference< css::io::XInputStream >& xIn,
                         const css::uno::Reference< css::io::XOutputStream >& xOut );

    /// @throws css::uno::Exception
    void insert( const css::uno::Reference< css::io::XInputStream > & xInputStream,
                 sal_Int32 nNameClashResolve,
                 const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    void destroy( bool bDeletePhysical,
                  const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    void transfer( const css::ucb::TransferInfo & rArgs,
                   const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    /// @throws css::uno::Exception
    css::uno::Reference< css::io::XInputStream >
    createTempFile( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

    void updateFileInfo();


    /// @throws css::uno::Exception
    css::uno::Reference< css::sdbc::XResultSet >
    getResultSet( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv );

public:
    /// @throws css::ucb::ContentCreationException
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );

    /// @throws css::ucb::ContentCreationException
    Content( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
             ContentProvider* pProvider,
             const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier,
             const GDriveFileInfo& rFileInfo );

    virtual ~Content() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContent
    virtual OUString SAL_CALL getContentType() override;

    // XCommandProcessor
    virtual css::uno::Any SAL_CALL
    execute( const css::ucb::Command& aCommand,
             sal_Int32 CommandId,
             const css::uno::Reference< css::ucb::XCommandEnvironment >& Environment ) override;

    virtual void SAL_CALL
    abort( sal_Int32 CommandId ) override;

    // XContentCreator
    virtual css::uno::Sequence< css::ucb::ContentInfo > SAL_CALL
    queryCreatableContentsInfo() override;

    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    createNewContent( const css::ucb::ContentInfo& Info ) override;

    // Helper methods
    static rtl::Reference< Content > create(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        ContentProvider* pProvider,
        const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier );

    const GDriveFileInfo& getFileInfo() const { return m_aFileInfo; }
    void setFileInfo( const GDriveFileInfo& rInfo );

    bool isTransient() const { return m_bTransient; }
    void setTransient( bool bTransient ) { m_bTransient = bTransient; }

    ContentProvider* getProvider() const { return m_pProvider; }
};

} // namespace gdrive
} // namespace ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
