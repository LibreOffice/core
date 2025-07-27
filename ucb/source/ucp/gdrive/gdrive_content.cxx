/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gdrive_content.hxx"
#include "gdrive_provider.hxx"
#include "gdrive_datasupplier.hxx"
#include "gdrive_resultset.hxx"
#include "GoogleDriveApiClient.hxx"

#include <sal/log.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/resultsetmetadata.hxx>
#include <ucbhelper/resultset.hxx>
#include <sal/log.hxx>

#include <vector>

using namespace com::sun::star;
using namespace ucp::gdrive;

//=========================================================================
// Content Implementation
//=========================================================================

Content::Content( const uno::Reference< uno::XComponentContext >& rxContext,
                  ContentProvider* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >& Identifier )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_pProvider( pProvider ),
  m_eResourceType( UNKNOWN ),
  m_bTransient( false )
{
    // Extract file ID from URL
    OUString sURL = Identifier->getContentIdentifier();
    m_aFileInfo.id = ContentProvider::getFileIdFromURL( sURL );

    // Get API client
    m_pApiClient = m_pProvider->getApiClient( nullptr );
}

Content::Content( const uno::Reference< uno::XComponentContext >& rxContext,
                  ContentProvider* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >& Identifier,
                  const GDriveFileInfo& rFileInfo )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_pProvider( pProvider ),
  m_aFileInfo( rFileInfo ),
  m_eResourceType( rFileInfo.isFolder ? FOLDER : FILE ),
  m_bTransient( false )
{
    SAL_WARN("ucb.ucp.gdrive", "Content constructor with FileInfo - name: '" + m_aFileInfo.name +
             "', id: '" + m_aFileInfo.id + "', isFolder: " + OUString::boolean(m_aFileInfo.isFolder));

    // Get API client
    m_pApiClient = m_pProvider->getApiClient( nullptr );
}

Content::~Content()
{
}

//=========================================================================
// XInterface methods
//=========================================================================

uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
{
    uno::Any aRet = cppu::queryInterface( rType,
        static_cast< ucb::XContentCreator* >( this ) );
    if ( aRet.hasValue() )
        return aRet;
    else
        return ContentImplHelper::queryInterface( rType );
}

void SAL_CALL Content::acquire() noexcept
{
    ContentImplHelper::acquire();
}

void SAL_CALL Content::release() noexcept
{
    ContentImplHelper::release();
}

//=========================================================================
// XTypeProvider methods
//=========================================================================

uno::Sequence< sal_Int8 > SAL_CALL Content::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
{
    if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
    {
        static cppu::OTypeCollection s_aFolderTypes(
                        CPPU_TYPE_REF( lang::XTypeProvider ),
                        CPPU_TYPE_REF( lang::XServiceInfo ),
                        CPPU_TYPE_REF( lang::XComponent ),
                        CPPU_TYPE_REF( ucb::XContent ),
                        CPPU_TYPE_REF( ucb::XCommandProcessor ),
                        CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                        CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                        CPPU_TYPE_REF( beans::XPropertyContainer ),
                        CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                        CPPU_TYPE_REF( container::XChild ),
                        CPPU_TYPE_REF( ucb::XContentCreator ) );

        return s_aFolderTypes.getTypes();
    }
    else
    {
        static cppu::OTypeCollection s_aFileTypes(
                        CPPU_TYPE_REF( lang::XTypeProvider ),
                        CPPU_TYPE_REF( lang::XServiceInfo ),
                        CPPU_TYPE_REF( lang::XComponent ),
                        CPPU_TYPE_REF( ucb::XContent ),
                        CPPU_TYPE_REF( ucb::XCommandProcessor ),
                        CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                        CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                        CPPU_TYPE_REF( beans::XPropertyContainer ),
                        CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                        CPPU_TYPE_REF( container::XChild ) );

        return s_aFileTypes.getTypes();
    }
}

//=========================================================================
// XServiceInfo methods
//=========================================================================

OUString SAL_CALL Content::getImplementationName()
{
    return u"com.sun.star.comp.ucb.GoogleDriveContent"_ustr;
}

uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
{
    return { GDRIVE_CONTENT_SERVICE_NAME };
}

//=========================================================================
// XContent methods
//=========================================================================

OUString SAL_CALL Content::getContentType()
{
    if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
        return GDRIVE_FOLDER_TYPE;
    else
        return GDRIVE_FILE_TYPE;
}

//=========================================================================
// XCommandProcessor methods
//=========================================================================

uno::Any SAL_CALL Content::execute(
    const ucb::Command& aCommand,
    sal_Int32 /*CommandId*/,
    const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    SAL_WARN("ucb.ucp.gdrive", "Content::execute called - Command: " + aCommand.Name +
             ", Environment: " + OUString::boolean(Environment.is()));

    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"Wrong argument type!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"Wrong argument type!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        if ( !aProperties.hasElements() )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"No properties!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name == "open" )
    {
        // DEBUG: Check if Environment is available
        SAL_WARN("ucb.ucp.gdrive", "Content::execute - open command, Environment available: " +
                 OUString::boolean(Environment.is()));

        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"Wrong argument type!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        if ( ( aOpenCommand.Mode == ucb::OpenMode::ALL ) ||
             ( aOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
             ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENTS ) )
        {
            // Open command for folder content - create result set with Google Drive files
            try
            {
                uno::Reference< ucb::XDynamicResultSet > xSet = new DynamicResultSet(
                    m_xContext, this, aOpenCommand, Environment );
                aRet <<= xSet;
            }
            catch ( const uno::Exception& e )
            {
                // If we can't create the result set, return an empty one for now
                // This allows the UI to work while we debug API issues
                SAL_WARN("ucb.ucp.gdrive", "Failed to create dynamic result set: " + e.Message);
                uno::Reference< ucb::XDynamicResultSet > xEmptySet;
                aRet <<= xEmptySet;
            }
        }
        else if ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENT )
        {
            // Open command for document content
            if ( isFolder( Environment ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::Any( ucb::UnsupportedOpenModeException(
                        OUString(),
                        static_cast< cppu::OWeakObject * >( this ),
                        sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
            }

            SAL_WARN("ucb.ucp.gdrive", "Open command - Sink provided: " + OUString::boolean(aOpenCommand.Sink.is()));
            if ( aOpenCommand.Sink.is() )
            {
                SAL_WARN("ucb.ucp.gdrive", "Opening document with file ID: " + m_aFileInfo.id);

                // Check what type of sink we have - following CMIS pattern
                uno::Reference< io::XOutputStream > xOut(aOpenCommand.Sink, uno::UNO_QUERY);
                uno::Reference< io::XActiveDataSink > xDataSink(aOpenCommand.Sink, uno::UNO_QUERY);
                uno::Reference< io::XActiveDataStreamer > xDataStreamer(aOpenCommand.Sink, uno::UNO_QUERY);
                uno::Reference< io::XInputStream > xPipeInputForSink;  // Will store pipe input stream if needed

                SAL_WARN("ucb.ucp.gdrive", "Sink types - XOutputStream: " + OUString::boolean(xOut.is()) +
                         ", XActiveDataSink: " + OUString::boolean(xDataSink.is()) +
                         ", XActiveDataStreamer: " + OUString::boolean(xDataStreamer.is()));

                // Additional debugging for XActiveDataStreamer
                if (xDataStreamer.is()) {
                    auto xStream = xDataStreamer->getStream();
                    SAL_WARN("ucb.ucp.gdrive", "XActiveDataStreamer.getStream() available: " + OUString::boolean(xStream.is()));
                    if (xStream.is()) {
                        auto xOutputStream = xStream->getOutputStream();
                        SAL_WARN("ucb.ucp.gdrive", "Stream.getOutputStream() available: " + OUString::boolean(xOutputStream.is()));
                    }
                }

                // Check if we have any valid sink interface
                if (!xOut.is() && !xDataSink.is() && !xDataStreamer.is())
                {
                    SAL_WARN("ucb.ucp.gdrive", "No valid data sink interface available");
                    ucbhelper::cancelCommandExecution(
                        ucb::IOErrorCode_CANT_WRITE,
                        uno::Sequence< uno::Any >( 0 ),
                        Environment,
                        "No data sink available" );
                    return uno::Any();
                }

                // Handle XActiveDataStreamer by creating our own pipe stream
                if (xDataStreamer.is() && !xOut.is())
                {
                    SAL_WARN("ucb.ucp.gdrive", "Creating pipe stream for XActiveDataStreamer");
                    try {
                        // Create a pipe (bidirectional stream) - returns the pipe object itself
                        uno::Reference< uno::XInterface > xPipe(io::Pipe::create(m_xContext));

                        // The pipe implements both XInputStream and XOutputStream
                        uno::Reference< io::XInputStream > xPipeIn(xPipe, uno::UNO_QUERY);
                        uno::Reference< io::XOutputStream > xPipeOut(xPipe, uno::UNO_QUERY);
                        uno::Reference< io::XStream > xStream(xPipe, uno::UNO_QUERY);

                        SAL_WARN("ucb.ucp.gdrive", "Pipe interfaces - XInputStream: " + OUString::boolean(xPipeIn.is()) +
                                 ", XOutputStream: " + OUString::boolean(xPipeOut.is()) +
                                 ", XStream: " + OUString::boolean(xStream.is()));

                        if (xPipeIn.is() && xPipeOut.is()) {
                            // For XActiveDataStreamer, we'll use the output stream directly
                            // LibreOffice will read from the input stream side
                            SAL_WARN("ucb.ucp.gdrive", "Using pipe output stream for XActiveDataStreamer");
                            xOut = xPipeOut;  // Use the pipe's output stream for writing

                            // Store the pipe input stream for later use with XActiveDataSink pattern
                            xPipeInputForSink = xPipeIn;

                            // Check if XActiveDataStreamer also implements XActiveDataSink
                            uno::Reference< io::XActiveDataSink > xSinkInterface(xDataStreamer, uno::UNO_QUERY);
                            if (xSinkInterface.is()) {
                                SAL_WARN("ucb.ucp.gdrive", "XActiveDataStreamer also implements XActiveDataSink, using that interface");
                                xDataSink = xSinkInterface;
                                xOut.clear(); // Will use XActiveDataSink path instead
                            }
                        } else {
                            SAL_WARN("ucb.ucp.gdrive", "Failed to get required pipe interfaces - XInputStream: " +
                                     OUString::boolean(xPipeIn.is()) + ", XOutputStream: " + OUString::boolean(xPipeOut.is()));
                        }
                    } catch (const uno::Exception& e) {
                        SAL_WARN("ucb.ucp.gdrive", "Exception creating pipe stream: " + e.Message);
                    }
                }

                if ( xDataSink.is() || xOut.is() )
                {
                    try {
                        if (!m_pApiClient) {
                            SAL_WARN("ucb.ucp.gdrive", "No API client available for download");
                            ucbhelper::cancelCommandExecution(
                                uno::Any( ucb::IOErrorCode_NOT_EXISTING ),
                                Environment );
                        }

                        SAL_WARN("ucb.ucp.gdrive", "Downloading file with ID: " + m_aFileInfo.id);
                        uno::Reference< io::XInputStream > xIn =
                            m_pApiClient->downloadFile( m_aFileInfo.id );

                        if ( xIn.is() ) {
                            SAL_WARN("ucb.ucp.gdrive", "Download successful, transferring data");

                            if ( xDataSink.is() )
                            {
                                SAL_WARN("ucb.ucp.gdrive", "Using XActiveDataSink interface");
                                if ( xPipeInputForSink.is() )
                                {
                                    SAL_WARN("ucb.ucp.gdrive", "Using pipe for XActiveDataSink - copying data through pipe");
                                    // We have a pipe: copy downloaded data to pipe output, set pipe input to sink
                                    if ( xOut.is() )
                                    {
                                        copyData( xIn, xOut );
                                        xDataSink->setInputStream( xPipeInputForSink );
                                    }
                                }
                                else
                                {
                                    SAL_WARN("ucb.ucp.gdrive", "Direct XActiveDataSink - setting input stream directly");
                                    xDataSink->setInputStream( xIn );
                                }
                            }
                            else if ( xOut.is() )
                            {
                                SAL_WARN("ucb.ucp.gdrive", "Using XOutputStream interface");
                                copyData( xIn, xOut );
                            }
                        } else {
                            SAL_WARN("ucb.ucp.gdrive", "Download failed - no input stream returned");
                            ucbhelper::cancelCommandExecution(
                                uno::Any( ucb::IOErrorCode_CANT_READ ),
                                Environment );
                        }
                    } catch (const uno::Exception& e) {
                        SAL_WARN("ucb.ucp.gdrive", "Exception during file download: " + e.Message);
                        ucbhelper::cancelCommandExecution(
                            uno::Any( ucb::IOErrorCode_GENERAL ),
                            Environment );
                    }
                }
                else
                {
                    SAL_WARN("ucb.ucp.gdrive", "No data sink available");
                    ucbhelper::cancelCommandExecution(
                        uno::Any( ucb::UnsupportedDataSinkException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            aOpenCommand.Sink ) ),
                        Environment );
                }
            }
            else
            {
                SAL_WARN("ucb.ucp.gdrive", "No sink provided in open command");
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( ucb::UnsupportedOpenModeException(
                    OUString(),
                    static_cast< cppu::OWeakObject * >( this ),
                    sal_Int16( aOpenCommand.Mode ) ) ),
                Environment );
        }
    }
    else if ( aCommand.Name == "insert" )
    {
        ucb::InsertCommandArgument aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"Wrong argument type!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        insert( aArg.Data, aArg.ReplaceExisting ? ucb::NameClash::OVERWRITE : ucb::NameClash::ERROR, Environment );
    }
    else if ( aCommand.Name == "delete" )
    {
        bool bDeletePhysical = false;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical, Environment );
    }
    else if ( aCommand.Name == "transfer" )
    {
        ucb::TransferInfo aTransferInfo;
        if ( !( aCommand.Argument >>= aTransferInfo ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                    u"Wrong argument type!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ),
                    -1 ) ),
                Environment );
        }

        transfer( aTransferInfo, Environment );
    }
    else
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                aCommand.Name,
                static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
    }

    return aRet;
}

void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
{
    // Not implemented - command execution cannot be aborted
}

//=========================================================================
// XContentCreator methods
//=========================================================================

uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
{
    return
    {
        {
            GDRIVE_FILE_TYPE,
            ucb::ContentInfoAttribute::KIND_DOCUMENT,
            {}
        },
        {
            GDRIVE_FOLDER_TYPE,
            ucb::ContentInfoAttribute::KIND_FOLDER,
            {}
        }
    };
}

uno::Reference< ucb::XContent > SAL_CALL Content::createNewContent( const ucb::ContentInfo& Info )
{
    if ( !Info.Type.equals( GDRIVE_FILE_TYPE ) && !Info.Type.equals( GDRIVE_FOLDER_TYPE ) )
        return uno::Reference< ucb::XContent >();

    OUString aURL = m_xIdentifier->getContentIdentifier();

    if ( !aURL.endsWith( u"/"_ustr ) )
        aURL += u"/"_ustr;

    // Create new temporary identifier
    aURL += u"new_content"_ustr;

    uno::Reference< ucb::XContentIdentifier > xId =
        new ::ucbhelper::ContentIdentifier( aURL );

    try
    {
        rtl::Reference< Content > xContent = new Content( m_xContext, m_pProvider, xId );
        xContent->setTransient( true );

        GDriveFileInfo aInfo;
        aInfo.isFolder = Info.Type.equals( GDRIVE_FOLDER_TYPE );
        xContent->setFileInfo( aInfo );

        return xContent.get();
    }
    catch ( ucb::ContentCreationException const & )
    {
        return uno::Reference< ucb::XContent >();
    }
}

//=========================================================================
// Helper methods
//=========================================================================

rtl::Reference< Content > Content::create(
    const uno::Reference< uno::XComponentContext >& rxContext,
    ContentProvider* pProvider,
    const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    return new Content( rxContext, pProvider, Identifier );
}

void Content::setFileInfo( const GDriveFileInfo& rInfo )
{
    m_aFileInfo = rInfo;
    m_eResourceType = rInfo.isFolder ? FOLDER : FILE;
}

//=========================================================================
// ContentImplHelper override methods
//=========================================================================

uno::Sequence< beans::Property > Content::getProperties(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    static const beans::Property aGenericProperties[] =
    {
        beans::Property( u"ContentType"_ustr,
                         -1,
                         cppu::UnoType<OUString>::get(),
                         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( u"IsDocument"_ustr,
                         -1,
                         cppu::UnoType<bool>::get(),
                         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( u"IsFolder"_ustr,
                         -1,
                         cppu::UnoType<bool>::get(),
                         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( u"Title"_ustr,
                         -1,
                         cppu::UnoType<OUString>::get(),
                         beans::PropertyAttribute::BOUND ),
        beans::Property( u"Size"_ustr,
                         -1,
                         cppu::UnoType<sal_Int64>::get(),
                         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        beans::Property( u"DateModified"_ustr,
                         -1,
                         cppu::UnoType<util::DateTime>::get(),
                         beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY )
    };

    return uno::Sequence< beans::Property >( aGenericProperties, SAL_N_ELEMENTS( aGenericProperties ) );
}

uno::Sequence< ucb::CommandInfo > Content::getCommands(
    const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
{
    static const ucb::CommandInfo aCommandInfoTable[] =
    {
        ucb::CommandInfo( u"getCommandInfo"_ustr, -1, cppu::UnoType<void>::get() ),
        ucb::CommandInfo( u"getPropertySetInfo"_ustr, -1, cppu::UnoType<void>::get() ),
        ucb::CommandInfo( u"getPropertyValues"_ustr, -1, cppu::UnoType<uno::Sequence< beans::Property >>::get() ),
        ucb::CommandInfo( u"setPropertyValues"_ustr, -1, cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get() ),
        ucb::CommandInfo( u"open"_ustr, -1, cppu::UnoType<ucb::OpenCommandArgument2>::get() ),
        ucb::CommandInfo( u"insert"_ustr, -1, cppu::UnoType<ucb::InsertCommandArgument>::get() ),
        ucb::CommandInfo( u"delete"_ustr, -1, cppu::UnoType<bool>::get() ),
        ucb::CommandInfo( u"transfer"_ustr, -1, cppu::UnoType<ucb::TransferInfo>::get() )
    };

    return uno::Sequence< ucb::CommandInfo >( aCommandInfoTable, SAL_N_ELEMENTS( aCommandInfoTable ) );
}

OUString Content::getParentURL()
{
    // Extract parent folder ID from current URL
    // OUString sURL = m_xIdentifier->getContentIdentifier(); // TODO: Use for content operations

    // For gdrive://file_id, the parent is gdrive://parent_id
    // For now, return root as parent for all items
    return u"gdrive://root"_ustr;
}

bool Content::isFolder( const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    updateFileInfo();
    return m_eResourceType == FOLDER;
}

uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Sequence< beans::Property >& rProperties,
    const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    updateFileInfo();

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( m_xContext );

    sal_Int32 nCount = rProperties.getLength();
    SAL_WARN("ucb.ucp.gdrive", "getPropertyValues - Processing " + OUString::number(nCount) + " properties");

    // Process properties in the exact order requested to maintain column indices
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property& rProp = rProperties[n];
        SAL_WARN("ucb.ucp.gdrive", "Property[" + OUString::number(n) + "] = " + rProp.Name);

        if ( rProp.Name == "Title" )
        {
            SAL_WARN("ucb.ucp.gdrive", "getPropertyValues - Title requested, returning: '" + m_aFileInfo.name + "'");
            if (m_aFileInfo.name.isEmpty()) {
                SAL_WARN("ucb.ucp.gdrive", "WARNING: Title is empty!");
                xRow->appendString( rProp, u"Untitled"_ustr );
            } else {
                xRow->appendString( rProp, m_aFileInfo.name );
            }
        }
        else if ( rProp.Name == "ContentType" )
        {
            xRow->appendString( rProp, getContentType() );
        }
        else if ( rProp.Name == "IsDocument" )
        {
            xRow->appendBoolean( rProp, m_eResourceType == FILE );
        }
        else if ( rProp.Name == "IsFolder" )
        {
            xRow->appendBoolean( rProp, m_eResourceType == FOLDER );
        }
        else if ( rProp.Name == "Size" )
        {
            sal_Int64 nSize = 0;
            if ( !m_aFileInfo.size.isEmpty() )
                nSize = m_aFileInfo.size.toInt64();
            xRow->appendLong( rProp, nSize );
        }
        else if ( rProp.Name == "DateModified" )
        {
            // Parse ISO 8601 date from m_aFileInfo.modifiedTime
            util::DateTime aDateTime = GDriveJsonHelper::parseDateTime(m_aFileInfo.modifiedTime);
            xRow->appendTimestamp( rProp, aDateTime );
        }
        else
        {
            SAL_WARN("ucb.ucp.gdrive", "Unknown property requested: " + rProp.Name);
            xRow->appendVoid( rProp );
        }
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

uno::Sequence< uno::Any > Content::setPropertyValues(
    const uno::Sequence< beans::PropertyValue >& rValues,
    const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further = false;
    aEvent.PropertyHandle = -1;

    sal_Int32 nCount = rValues.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = rValues[n];

        if ( rValue.Name == "Title" )
        {
            OUString aNewTitle;
            if ( rValue.Value >>= aNewTitle )
            {
                if ( aNewTitle != m_aFileInfo.name )
                {
                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue = uno::Any( m_aFileInfo.name );
                    aEvent.NewValue = rValue.Value;

                    m_aFileInfo.name = aNewTitle;

                    aChanges.getArray()[ nChanged ] = aEvent;
                    nChanged++;
                }
            }
            else
            {
                aRet.getArray()[ n ] = uno::Any(
                    beans::IllegalTypeException(
                        u"Property value has wrong type!"_ustr,
                        static_cast< cppu::OWeakObject * >( this ) ) );
            }
        }
        else
        {
            aRet.getArray()[ n ] = uno::Any(
                beans::UnknownPropertyException(
                    u"Property is read-only!"_ustr,
                    static_cast< cppu::OWeakObject * >( this ) ) );
        }
    }

    if ( nChanged > 0 )
    {
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

void Content::queryChildren( ContentRefList& rChildren )
{
    if ( m_eResourceType != FOLDER )
        return;

    if ( m_pApiClient )
    {
        std::vector< GDriveFileInfo > aFileInfos =
            m_pApiClient->listFolderComplete( m_aFileInfo.id, 500 ); // Limit to 500 files for UI performance

        for ( const auto& fileInfo : aFileInfos )
        {
            OUString sContentId = u"gdrive://"_ustr + fileInfo.id;
            uno::Reference< ucb::XContentIdentifier > xId(
                new ucbhelper::ContentIdentifier( sContentId ) );
            rtl::Reference< Content > xContent =
                new Content( m_xContext, m_pProvider, xId, fileInfo );
            rChildren.push_back( xContent );
        }
    }
}

void Content::insert( const uno::Reference< io::XInputStream > & xInputStream,
                      sal_Int32 /*nNameClashResolve*/,
                      const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    if ( !m_bTransient )
        return;

    if ( m_eResourceType == FOLDER )
    {
        // Create folder
        if ( m_pApiClient )
        {
            m_pApiClient->createFolder( m_sParentId, m_aFileInfo.name );
        }
    }
    else
    {
        // Upload file
        if ( m_pApiClient && xInputStream.is() )
        {
            m_pApiClient->uploadFile( m_sParentId, m_aFileInfo.name, xInputStream );
        }
    }

    m_bTransient = false;
}

void Content::destroy( bool bDeletePhysical,
                       const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    if (bDeletePhysical && m_pApiClient && !m_aFileInfo.id.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Deleting file/folder: " + m_aFileInfo.name + " (ID: " + m_aFileInfo.id + ")");
        m_pApiClient->deleteFile(m_aFileInfo.id);
    }
}

void Content::transfer( const ucb::TransferInfo & rArgs,
                        const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    if (!m_pApiClient) {
        SAL_WARN("ucb.ucp.gdrive", "No API client available for transfer operation");
        return;
    }

    SAL_WARN("ucb.ucp.gdrive", "transfer() called - Operation: " <<
             (rArgs.MoveData ? "MOVE" : "COPY") <<
             ", NewTitle: " << rArgs.NewTitle <<
             ", SourceURL: " << rArgs.SourceURL);

    // Extract source file ID from URL
    rtl::OUString sSourceFileId = ContentProvider::getFileIdFromURL(rArgs.SourceURL);
    if (sSourceFileId.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Cannot extract source file ID from URL: " + rArgs.SourceURL);
        return;
    }

    // Use current content's file ID as the target parent
    rtl::OUString sTargetParentId = m_aFileInfo.id;
    if (sTargetParentId.isEmpty()) {
        SAL_WARN("ucb.ucp.gdrive", "Target parent ID is empty");
        return;
    }

    try {
        if (rArgs.MoveData) {
            // Move operation
            SAL_WARN("ucb.ucp.gdrive", "Moving file " + sSourceFileId + " to parent " + sTargetParentId);
            m_pApiClient->moveFile(sSourceFileId, sTargetParentId, rArgs.NewTitle);
        } else {
            // Copy operation
            SAL_WARN("ucb.ucp.gdrive", "Copying file " + sSourceFileId + " to parent " + sTargetParentId);
            rtl::OUString sNewFileId = m_pApiClient->copyFile(sSourceFileId, sTargetParentId, rArgs.NewTitle);
            if (!sNewFileId.isEmpty()) {
                SAL_WARN("ucb.ucp.gdrive", "Copy successful, new file ID: " + sNewFileId);
            }
        }
    } catch (const uno::Exception& e) {
        SAL_WARN("ucb.ucp.gdrive", "Exception during transfer: " + e.Message);
    }
}

void Content::updateFileInfo()
{
    if ( m_eResourceType == UNKNOWN && m_pApiClient && !m_aFileInfo.id.isEmpty() )
    {
        SAL_WARN("ucb.ucp.gdrive", "Updating file info for ID: " + m_aFileInfo.id);

        GDriveFileInfo updatedInfo = m_pApiClient->getFileInfo(m_aFileInfo.id);

        if (!updatedInfo.id.isEmpty()) {
            m_aFileInfo = updatedInfo;
            m_eResourceType = updatedInfo.isFolder ? FOLDER : FILE;
            SAL_WARN("ucb.ucp.gdrive", "Updated file info: " + m_aFileInfo.name +
                     " (type: " + (m_eResourceType == FOLDER ? u"folder" : u"file") + ")");
        } else {
            SAL_WARN("ucb.ucp.gdrive", "Failed to retrieve file info, assuming file exists");
            m_eResourceType = FILE;
        }
    }
}


uno::Reference< sdbc::XResultSet > Content::getResultSet(
    const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    // Create a result set for folder listing
    // TODO: Implement proper ResultSetMetaData with properties sequence
    // rtl::Reference< ::ucbhelper::ResultSetMetaData > xMetaData = new ::ucbhelper::ResultSetMetaData( m_xContext );

    /*
    xMetaData->setColumnCount( 6 );
    xMetaData->setColumnName( 1, u"ContentType"_ustr );
    xMetaData->setColumnName( 2, u"Title"_ustr );
    xMetaData->setColumnName( 3, u"Size"_ustr );
    xMetaData->setColumnName( 4, u"DateModified"_ustr );
    xMetaData->setColumnName( 5, u"IsFolder"_ustr );
    xMetaData->setColumnName( 6, u"IsDocument"_ustr );
    */

    // Create property value sets for each child
    std::vector< std::vector< uno::Any > > aRows;

    if ( m_eResourceType == FOLDER && m_pApiClient )
    {
        std::vector< GDriveFileInfo > aFileInfos =
            m_pApiClient->listFolderComplete( m_aFileInfo.id, 500 ); // Limit to 500 files for UI performance

        for ( const auto& fileInfo : aFileInfos )
        {
            OUString sContentId = u"gdrive://"_ustr + fileInfo.id;
            uno::Reference< ucb::XContentIdentifier > xId(
                new ucbhelper::ContentIdentifier( sContentId ) );
            rtl::Reference< Content > pContent =
                new Content( m_xContext, m_pProvider, xId, fileInfo );

            if ( pContent.is() )
            {
                std::vector< uno::Any > aRow( 6 );
                const GDriveFileInfo& rInfo = pContent->getFileInfo();

                aRow[0] <<= (rInfo.isFolder ? GDRIVE_FOLDER_TYPE : GDRIVE_FILE_TYPE);
                aRow[1] <<= rInfo.name;
                aRow[2] <<= (rInfo.size.isEmpty() ? sal_Int64(0) : rInfo.size.toInt64());
                aRow[3] <<= GDriveJsonHelper::parseDateTime(rInfo.modifiedTime);
                aRow[4] <<= rInfo.isFolder;
                aRow[5] <<= !rInfo.isFolder;

                aRows.push_back( aRow );
            }
        }
    }

    // TODO: Create proper result set from aRows
    // For now return empty result set
    return uno::Reference< sdbc::XResultSet >();
}

void Content::copyData( const css::uno::Reference< css::io::XInputStream >& xIn,
                       const css::uno::Reference< css::io::XOutputStream >& xOut )
{
    const sal_Int32 TRANSFER_BUFFER_SIZE = 32768;
    css::uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

    if ( !xIn.is() || !xOut.is() )
        return;

    try {
        while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
            xOut->writeBytes( theData );

        xOut->closeOutput();
    }
    catch ( const css::uno::Exception& )
    {
        // Ignore errors during copying
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
