/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gdrive_datasupplier.hxx"
#include "gdrive_content.hxx"
#include "gdrive_provider.hxx"
#include "GoogleDriveApiClient.hxx"

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <ucbhelper/authenticationfallback.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>
#include <utility>

using namespace com::sun::star;
using namespace ucp::gdrive;

DataSupplier::DataSupplier( const uno::Reference< uno::XComponentContext >& /*rxContext*/,
                           rtl::Reference< Content > xContent,
                           sal_Int32 nOpenMode )
    : ucbhelper::ResultSetDataSupplier(),
      m_xContent( std::move(xContent) ),
      m_nOpenMode( nOpenMode ),
      m_bCountFinal( false )
{
}

DataSupplier::~DataSupplier()
{
}

bool DataSupplier::getData(std::unique_lock<std::mutex>& /*rResultSetGuard*/)
{
    if (m_bCountFinal)
        return true;

    // Get the command environment from the result set to enable authentication
    uno::Reference< ucb::XCommandEnvironment > xEnv;
    if ( getResultSet() )
        xEnv = getResultSet()->getEnvironment();

    // DEBUG: Show a dialog to confirm DataSupplier::getData() is called
    // Force show dialog even without command environment for testing
    try {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        if (xContext.is()) {
            uno::Reference<task::XInteractionHandler> xIH(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                uno::UNO_QUERY);
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"DEBUG: DataSupplier::getData() method was called! Command environment available: "_ustr +
                        (xEnv.is() ? u"YES" : u"NO"),
                        u"test://debug-datasupplier"_ustr);
                xIH->handle(xRequest);
            }
        }
    } catch (...) {
        // Ignore errors in debug code
    }

    // Get the API client from content with proper command environment
    std::shared_ptr<GoogleDriveApiClient> pApiClient =
        m_xContent->getProvider()->getApiClient( xEnv );

    if ( !pApiClient )
    {
        SAL_WARN("ucb.ucp.gdrive", "Failed to get API client");
        return false;
    }

    // Show debug dialog before API call
    try {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        if (xContext.is()) {
            uno::Reference<task::XInteractionHandler> xIH(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                uno::UNO_QUERY);
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"DEBUG: About to call Google Drive API for folder: " +
                        m_xContent->getFileInfo().id,
                        u"test://debug-before-api"_ustr);
                xIH->handle(xRequest);
            }
        }
    } catch (...) {}

    // Get folder contents from Google Drive API
    std::vector< GDriveFileInfo > aFileInfos =
        pApiClient->listFolder( m_xContent->getFileInfo().id );

    // Convert file infos to Content objects
    std::vector< uno::Reference< ucb::XContent > > aContents;
    for (const auto& fileInfo : aFileInfos) {
        OUString sContentId = u"gdrive://"_ustr + fileInfo.id;
        uno::Reference< ucb::XContentIdentifier > xId(
            new ucbhelper::ContentIdentifier( sContentId ) );
        uno::Reference< ucb::XContent > xContent =
            new Content( ::comphelper::getProcessComponentContext(),
                        m_xContent->getProvider(), xId, fileInfo );
        aContents.push_back( xContent );
    }

    // Show debug dialog after API call
    try {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        if (xContext.is()) {
            uno::Reference<task::XInteractionHandler> xIH(
                xContext->getServiceManager()->createInstanceWithContext(
                    u"com.sun.star.task.InteractionHandler"_ustr, xContext),
                uno::UNO_QUERY);
            if (xIH.is()) {
                rtl::Reference<ucbhelper::AuthenticationFallbackRequest> xRequest
                    = new ucbhelper::AuthenticationFallbackRequest(
                        u"DEBUG: API call returned " + OUString::number(aContents.size()) + u" files",
                        u"test://debug-after-api"_ustr);
                xIH->handle(xRequest);
            }
        }
    } catch (...) {}

    // If no files returned and no token, fall back to test data
    if (aContents.empty())
    {
        SAL_WARN("ucb.ucp.gdrive", "No files from API, using test data");

    // Create test file entries
    for (int i = 1; i <= 3; i++)
    {
        GDriveFileInfo aTestFile;
        aTestFile.id = u"test_file_"_ustr + OUString::number(i);
        aTestFile.name = u"Test Document "_ustr + OUString::number(i) + u".docx";
        aTestFile.isFolder = false;
        aTestFile.size = OUString::number(1024 * i);
        aTestFile.modifiedTime = u"2024-01-01T00:00:00.000Z"_ustr;

        SAL_WARN("ucb.ucp.gdrive", "Creating test file - name: '" + aTestFile.name + "', id: '" + aTestFile.id + "'");

        // Create content identifier for this file
        OUString sContentId = u"gdrive://"_ustr + aTestFile.id;
        uno::Reference< ucb::XContentIdentifier > xId(
            new ucbhelper::ContentIdentifier( sContentId ) );

        // Create content object
        uno::Reference< ucb::XContent > xContent =
            new Content( ::comphelper::getProcessComponentContext(),
                        m_xContent->getProvider(), xId, aTestFile );
        aContents.push_back( xContent );
    }

    // Add a test folder
    GDriveFileInfo aTestFolder;
    aTestFolder.id = u"test_folder_1"_ustr;
    aTestFolder.name = u"Test Folder"_ustr;
    aTestFolder.isFolder = true;
    aTestFolder.modifiedTime = u"2024-01-01T00:00:00.000Z"_ustr;

    OUString sFolderId = u"gdrive://"_ustr + aTestFolder.id;
    uno::Reference< ucb::XContentIdentifier > xFolderId(
        new ucbhelper::ContentIdentifier( sFolderId ) );
    uno::Reference< ucb::XContent > xFolderContent =
        new Content( ::comphelper::getProcessComponentContext(),
                    m_xContent->getProvider(), xFolderId, aTestFolder );
    aContents.push_back( xFolderContent );
    }  // End of test data fallback

    for ( const auto& xContent : aContents )
    {
        if ( Content* pContent = static_cast< Content* >( xContent.get() ) )
        {
            const GDriveFileInfo& rInfo = pContent->getFileInfo();

            // Filter based on open mode
            switch ( m_nOpenMode )
            {
                case ucb::OpenMode::FOLDERS:
                    if ( !rInfo.isFolder )
                        continue;
                    break;
                case ucb::OpenMode::DOCUMENTS:
                    if ( rInfo.isFolder )
                        continue;
                    break;
                case ucb::OpenMode::ALL:
                default:
                    break;
            }

            // Store the content object directly in the result entry
            auto pEntry = new ResultListEntry( rInfo );
            pEntry->xContent = xContent;
            m_aResults.emplace_back( pEntry );
        }
    }

    m_bCountFinal = true;
    return true;
}

OUString DataSupplier::queryContentIdentifierString( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        OUString aId = m_aResults[ nIndex ]->aId;
        if ( aId.getLength() )
        {
            // Already cached.
            return aId;
        }
    }

    if ( getResult( rResultSetGuard, nIndex ) )
    {
        // Create gdrive:// URL for this file
        OUString aId = u"gdrive://"_ustr + m_aResults[ nIndex ]->aFileInfo.id;
        m_aResults[ nIndex ]->aId = aId;
        return aId;
    }

    return OUString();
}

uno::Reference< ucb::XContentIdentifier > DataSupplier::queryContentIdentifier( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContentIdentifier > xId = m_aResults[ nIndex ]->xId;
        if ( xId.is() )
        {
            // Already cached.
            return xId;
        }
    }

    OUString aId = queryContentIdentifierString( rResultSetGuard, nIndex );
    if ( aId.getLength() )
    {
        uno::Reference< ucb::XContentIdentifier > xId = new ucbhelper::ContentIdentifier( aId );
        m_aResults[ nIndex ]->xId = xId;
        return xId;
    }

    return uno::Reference< ucb::XContentIdentifier >();
}

uno::Reference< ucb::XContent > DataSupplier::queryContent( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< ucb::XContent > xContent = m_aResults[ nIndex ]->xContent;
        if ( xContent.is() )
        {
            // Already cached.
            return xContent;
        }
    }

    uno::Reference< ucb::XContentIdentifier > xId = queryContentIdentifier( rResultSetGuard, nIndex );
    if ( xId.is() )
    {
        try
        {
            uno::Reference< ucb::XContent > xContent = m_xContent->getProvider()->queryContent( xId );
            m_aResults[ nIndex ]->xContent = xContent;
            return xContent;
        }
        catch ( const ucb::IllegalIdentifierException& )
        {
        }
    }

    return uno::Reference< ucb::XContent >();
}

bool DataSupplier::getResult( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
        return true;

    if ( !getData( rResultSetGuard ) )
        return false;

    return nIndex < m_aResults.size();
}

sal_uInt32 DataSupplier::totalCount(std::unique_lock<std::mutex>& rResultSetGuard)
{
    getData( rResultSetGuard );
    return m_aResults.size();
}

sal_uInt32 DataSupplier::currentCount()
{
    return m_aResults.size();
}

bool DataSupplier::isCountFinal()
{
    return m_bCountFinal;
}

uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex )
{
    SAL_WARN("ucb.ucp.gdrive", "DataSupplier::queryPropertyValues called for index: " +
             OUString::number(nIndex));

    if ( nIndex < m_aResults.size() )
    {
        uno::Reference< sdbc::XRow > xRow = m_aResults[ nIndex ]->xRow;
        if ( xRow.is() )
        {
            // Already cached.
            return xRow;
        }
    }

    if ( getResult( rResultSetGuard, nIndex ) )
    {
        uno::Reference< ucb::XContent > xContent = queryContent( rResultSetGuard, nIndex );
        if ( xContent.is() )
        {
            try
            {
                uno::Reference< ucb::XCommandProcessor > xCmdProc(
                    xContent, uno::UNO_QUERY_THROW );

                sal_Int32 nCmdId = xCmdProc->createCommandIdentifier();

                ucb::Command aGetPropsCommand;
                aGetPropsCommand.Name = u"getPropertyValues"_ustr;
                aGetPropsCommand.Handle = -1;

                // Use the properties requested by the caller
                if ( getResultSet() )
                {
                    aGetPropsCommand.Argument <<= getResultSet()->getProperties();
                }
                else
                {
                    // Fallback to default properties
                    uno::Sequence< beans::Property > aProps( 6 );
                    auto pProps = aProps.getArray();
                    pProps[ 0 ].Name = u"ContentType"_ustr;
                    pProps[ 0 ].Handle = -1;
                    pProps[ 1 ].Name = u"Title"_ustr;
                    pProps[ 1 ].Handle = -1;
                    pProps[ 2 ].Name = u"Size"_ustr;
                    pProps[ 2 ].Handle = -1;
                    pProps[ 3 ].Name = u"DateModified"_ustr;
                    pProps[ 3 ].Handle = -1;
                    pProps[ 4 ].Name = u"IsFolder"_ustr;
                    pProps[ 4 ].Handle = -1;
                    pProps[ 5 ].Name = u"IsDocument"_ustr;
                    pProps[ 5 ].Handle = -1;

                    aGetPropsCommand.Argument <<= aProps;
                }

                uno::Reference< sdbc::XRow > xRow;
                xCmdProc->execute( aGetPropsCommand, nCmdId, uno::Reference< ucb::XCommandEnvironment >() ) >>= xRow;

                if ( xRow.is() )
                {
                    m_aResults[ nIndex ]->xRow = xRow;
                    return xRow;
                }
            }
            catch ( const uno::Exception& )
            {
            }
        }
    }

    return uno::Reference< sdbc::XRow >();
}

void DataSupplier::releasePropertyValues( sal_uInt32 nIndex )
{
    if ( nIndex < m_aResults.size() )
        m_aResults[ nIndex ]->xRow.clear();
}

void DataSupplier::close()
{
}

void DataSupplier::validate()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */