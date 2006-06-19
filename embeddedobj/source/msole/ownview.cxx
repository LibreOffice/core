/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ownview.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:31:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBRODCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <comphelper/storagehelper.hxx>
#include "ownview.hxx"
#include <convert.hxx>


using namespace ::com::sun::star;


::rtl::OUString GetNewTempFileURL_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory ) throw( io::IOException );
::rtl::OUString GetNewFilledTempFile_Impl( const uno::Reference< io::XInputStream >& xInStream, const uno::Reference< lang::XMultiServiceFactory >& xFactory ) throw( io::IOException );
sal_Bool KillFile_Impl( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory );
uno::Reference< io::XStream > TryToGetAcceptableFormat_Impl( const uno::Reference< io::XStream >& xStream, const uno::Reference< lang::XMultiServiceFactory >& xFactory ) throw ( uno::Exception );

//========================================================
// Dummy interaction handler
//========================================================
//--------------------------------------------------------
class DummyHandler_Impl : public ::cppu::WeakImplHelper1< task::XInteractionHandler >
{
public:
    DummyHandler_Impl() {}
    ~DummyHandler_Impl();

    virtual void SAL_CALL handle( const uno::Reference< task::XInteractionRequest >& xRequest )
            throw( uno::RuntimeException );
};

//--------------------------------------------------------
DummyHandler_Impl::~DummyHandler_Impl()
{
}

//--------------------------------------------------------
void SAL_CALL DummyHandler_Impl::handle( const uno::Reference< task::XInteractionRequest >& )
        throw( uno::RuntimeException )
{
    return;
}

//========================================================
// Object viewer
//========================================================
//--------------------------------------------------------
OwnView_Impl::OwnView_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                            const uno::Reference< io::XInputStream >& xInputStream )
: m_xFactory( xFactory )
, m_bBusy( sal_False )
, m_bUseNative( sal_False )
{
    if ( !xFactory.is() || !xInputStream.is() )
        throw uno::RuntimeException();

    m_aTempFileURL = GetNewFilledTempFile_Impl( xInputStream, m_xFactory );
}

//--------------------------------------------------------
OwnView_Impl::~OwnView_Impl()
{
    try {
        KillFile_Impl( m_aTempFileURL, m_xFactory );
    } catch( uno::Exception& ) {}

    try {
        if ( m_aNativeTempURL.getLength() )
            KillFile_Impl( m_aNativeTempURL, m_xFactory );
    } catch( uno::Exception& ) {}
}

//--------------------------------------------------------
sal_Bool OwnView_Impl::CreateModelFromURL( const ::rtl::OUString& aFileURL )
{
    sal_Bool bResult = sal_False;

    if ( aFileURL.getLength() )
    {
        try {
            uno::Reference < frame::XComponentLoader > xDocumentLoader(
                            m_xFactory->createInstance (
                                        ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                            uno::UNO_QUERY );

            if ( xDocumentLoader.is() )
            {
                uno::Sequence< beans::PropertyValue > aArgs( m_aFilterName.getLength() ? 5 : 4 );

                aArgs[0].Name = ::rtl::OUString::createFromAscii( "URL" );
                aArgs[0].Value <<= aFileURL;

                aArgs[1].Name = ::rtl::OUString::createFromAscii( "ReadOnly" );
                aArgs[1].Value <<= sal_True;

                aArgs[2].Name = ::rtl::OUString::createFromAscii( "InteractionHandler" );
                aArgs[2].Value <<= uno::Reference< task::XInteractionHandler >(
                                    static_cast< ::cppu::OWeakObject* >( new DummyHandler_Impl() ), uno::UNO_QUERY );

                aArgs[3].Name = ::rtl::OUString::createFromAscii( "DontEdit" );
                aArgs[3].Value <<= sal_True;

                if ( m_aFilterName.getLength() )
                {
                    aArgs[4].Name = ::rtl::OUString::createFromAscii( "FilterName" );
                    aArgs[4].Value <<= m_aFilterName;
                }

                uno::Reference< frame::XModel > xModel( xDocumentLoader->loadComponentFromURL(
                                                                aFileURL,
                                                                ::rtl::OUString::createFromAscii( "_blank" ),
                                                                0,
                                                                aArgs ),
                                                            uno::UNO_QUERY );

                if ( xModel.is() )
                {
                    uno::Reference< document::XEventBroadcaster > xBroadCaster( xModel, uno::UNO_QUERY );
                    if ( xBroadCaster.is() )
                        xBroadCaster->addEventListener( uno::Reference< document::XEventListener >(
                                                                static_cast< ::cppu::OWeakObject* >( this ),
                                                                 uno::UNO_QUERY ) );

                    uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
                    if ( xCloseable.is() )
                    {
                        xCloseable->addCloseListener( uno::Reference< util::XCloseListener >(
                                                                        static_cast< ::cppu::OWeakObject* >( this ),
                                                                          uno::UNO_QUERY ) );

                        ::osl::MutexGuard aGuard( m_aMutex );
                        m_xModel = xModel;
                        bResult = sal_True;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    return bResult;
}

//--------------------------------------------------------
sal_Bool OwnView_Impl::CreateModel( sal_Bool bUseNative )
{
    sal_Bool bResult = sal_False;

    try {
        bResult = CreateModelFromURL( bUseNative ? m_aNativeTempURL : m_aTempFileURL );
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

//--------------------------------------------------------
::rtl::OUString OwnView_Impl::GetFilterNameFromExtentionAndInStream( const ::rtl::OUString& aNameWithExtention,
                                                                    const uno::Reference< io::XInputStream >& xInputStream )
{
    if ( !xInputStream.is() )
        throw uno::RuntimeException();

    uno::Reference< document::XTypeDetection > xTypeDetection(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
            uno::UNO_QUERY_THROW );

    ::rtl::OUString aTypeName;

    if ( aNameWithExtention.getLength() )
    {
        ::rtl::OUString aURLToAnalyze =
                ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "file:///" ) ) + aNameWithExtention );
        aTypeName = xTypeDetection->queryTypeByURL( aURLToAnalyze );
    }

    uno::Sequence< beans::PropertyValue > aArgs( aTypeName.getLength() ? 3 : 2 );
    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
    aArgs[0].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" ) );
    aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) );
    aArgs[1].Value <<= xInputStream;
    if ( aTypeName.getLength() )
    {
        aArgs[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TypeName" ) );
        aArgs[2].Value <<= aTypeName;
    }

    aTypeName = xTypeDetection->queryTypeByDescriptor( aArgs, sal_True );

    ::rtl::OUString aFilterName;
    for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
        if ( aArgs[nInd].Name.equalsAscii( "FilterName" ) )
            aArgs[nInd].Value >>= aFilterName;

    if ( !aFilterName.getLength() && aTypeName.getLength() )
    {
        // get the default filter name for the type
        uno::Reference< container::XNameAccess > xNameAccess( xTypeDetection, uno::UNO_QUERY_THROW );
        uno::Sequence< beans::PropertyValue > aTypes;

        if ( xNameAccess.is() && ( xNameAccess->getByName( aTypeName ) >>= aTypes ) )
        {
            for ( sal_Int32 nInd = 0; nInd < aTypes.getLength(); nInd++ )
            {
                if ( aTypes[nInd].Name.equalsAscii( "PreferredFilter" ) && ( aTypes[nInd].Value >>= aFilterName ) )
                {
                    aTypes[nInd].Value >>= aFilterName;
                    break;
                }
            }
        }
    }

    return aFilterName;
}

//--------------------------------------------------------
sal_Bool OwnView_Impl::ReadContentsAndGenerateTempFile( const uno::Reference< io::XInputStream >& xInStream,
                                                        sal_Bool bParseHeader )
{
    uno::Reference< io::XSeekable > xSeekable( xInStream, uno::UNO_QUERY_THROW );
    xSeekable->seek( 0 );

    // create m_aNativeTempURL
    ::rtl::OUString aNativeTempURL;
    uno::Reference < beans::XPropertySet > xNativeTempFile(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY_THROW );
    uno::Reference < io::XStream > xNativeTempStream( xNativeTempFile, uno::UNO_QUERY_THROW );
    uno::Reference < io::XOutputStream > xNativeOutTemp = xNativeTempStream->getOutputStream();
    uno::Reference < io::XInputStream > xNativeInTemp = xNativeTempStream->getInputStream();
    if ( !xNativeOutTemp.is() || !xNativeInTemp.is() )
        throw uno::RuntimeException();

    try {
        xNativeTempFile->setPropertyValue( ::rtl::OUString::createFromAscii( "RemoveFile" ), uno::makeAny( sal_False ) );
        uno::Any aUrl = xNativeTempFile->getPropertyValue( ::rtl::OUString::createFromAscii( "Uri" ) );
        aUrl >>= aNativeTempURL;
    }
    catch ( uno::Exception& )
    {
    }

    sal_Bool bFailed = sal_False;
    ::rtl::OUString aFileSuffix;

    if ( bParseHeader )
    {
        uno::Sequence< sal_Int8 > aReadSeq( 4 );
        // read the complete size of the Object Package
        if ( xInStream->readBytes( aReadSeq, 4 ) != 4 )
            return sal_False;
/*
        sal_uInt32 nLength = (sal_uInt8)aReadSeq[0]
                            + (sal_uInt8)aReadSeq[1] * 0x100
                            + (sal_uInt8)aReadSeq[2] * 0x10000
                            + (sal_uInt8)aReadSeq[3] * 0x1000000;
*/
        // read the first header ( have no idea what does this header mean )
        if ( xInStream->readBytes( aReadSeq, 2 ) != 2 || aReadSeq[0] != 2 || aReadSeq[1] != 0 )
            return sal_False;

        // read file name
        // only extension is interesting so only subset of symbols is accepted
        do
        {
            if ( xInStream->readBytes( aReadSeq, 1 ) != 1 )
                return sal_False;

            if ( aReadSeq[0] >= '0' && aReadSeq[0] <= '9'
              || aReadSeq[0] >= 'a' && aReadSeq[0] <= 'z'
              || aReadSeq[0] >= 'A' && aReadSeq[0] <= 'Z'
              || aReadSeq[0] == '.' )
                aFileSuffix += ::rtl::OUString::valueOf( (sal_Unicode) aReadSeq[0] );

        } while( aReadSeq[0] );

        // skip url
        do
        {
            if ( xInStream->readBytes( aReadSeq, 1 ) != 1 )
                return sal_False;
        } while( aReadSeq[0] );

        // check the next header
        if ( xInStream->readBytes( aReadSeq, 4 ) != 4
          || aReadSeq[0] || aReadSeq[1] || aReadSeq[2] != 3 || aReadSeq[3] )
            return sal_False;

        // get the size of the next entry
        if ( xInStream->readBytes( aReadSeq, 4 ) != 4 )
            return sal_False;

        sal_uInt32 nUrlSize = (sal_uInt8)aReadSeq[0]
                            + (sal_uInt8)aReadSeq[1] * 0x100
                            + (sal_uInt8)aReadSeq[2] * 0x10000
                            + (sal_uInt8)aReadSeq[3] * 0x1000000;
        sal_Int64 nTargetPos = xSeekable->getPosition() + nUrlSize;

        xSeekable->seek( nTargetPos );

        // get the size of stored data
        if ( xInStream->readBytes( aReadSeq, 4 ) != 4 )
            return sal_False;

        sal_uInt32 nDataSize = (sal_uInt8)aReadSeq[0]
                            + (sal_uInt8)aReadSeq[1] * 0x100
                            + (sal_uInt8)aReadSeq[2] * 0x10000
                            + (sal_uInt8)aReadSeq[3] * 0x1000000;

        aReadSeq.realloc( 32000 );
        sal_uInt32 nRead = 0;
        while ( nRead < nDataSize )
        {
            sal_uInt32 nToRead = ( nDataSize - nRead > 32000 ) ? 32000 : nDataSize - nRead;
            sal_uInt32 nLocalRead = xInStream->readBytes( aReadSeq, nToRead );


            if ( !nLocalRead )
            {
                bFailed = sal_True;
                break;
            }
            else if ( nLocalRead == 32000 )
                xNativeOutTemp->writeBytes( aReadSeq );
            else
            {
                uno::Sequence< sal_Int8 > aToWrite( aReadSeq );
                aToWrite.realloc( nLocalRead );
                xNativeOutTemp->writeBytes( aToWrite );
            }

            nRead += nLocalRead;
        }
    }
    else
    {
        uno::Sequence< sal_Int8 > aData( 8 );
        if ( xInStream->readBytes( aData, 8 ) == 8
          && aData[0] == -1 && aData[1] == -1 && aData[2] == -1 && aData[3] == -1
          && ( aData[4] == 2 || aData[4] == 3 ) && aData[5] == 0 && aData[6] == 0 && aData[7] == 0 )
        {
            // the header has to be removed
            xSeekable->seek( 40 );
        }
        else
        {
            // the usual Ole10Native format
            xSeekable->seek( 4 );
        }

        ::comphelper::OStorageHelper::CopyInputToOutput( xInStream, xNativeOutTemp );
    }

    xNativeOutTemp->closeOutput();

    // The temporary native file is created, now the filter must be detected
    if ( !bFailed )
    {
        m_aFilterName = GetFilterNameFromExtentionAndInStream( aFileSuffix, xNativeInTemp );
        m_aNativeTempURL = aNativeTempURL;
    }

    return !bFailed;
}

//--------------------------------------------------------
void OwnView_Impl::CreateNative()
{
    if ( m_aNativeTempURL.getLength() )
        return;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess > xAccess(
                m_xFactory->createInstance (
                        ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                uno::UNO_QUERY_THROW );

        uno::Reference< io::XInputStream > xInStream = xAccess->openFileRead( m_aTempFileURL );
        if ( !xInStream.is() )
            throw uno::RuntimeException();

        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= xInStream;
        uno::Reference< container::XNameAccess > xNameAccess(
                m_xFactory->createInstanceWithArguments(
                        ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLESimpleStorage" ),
                        aArgs ),
                uno::UNO_QUERY_THROW );

        ::rtl::OUString aSubStreamName = ::rtl::OUString::createFromAscii( "\1Ole10Native" );
        uno::Reference< embed::XClassifiedObject > xStor( xNameAccess, uno::UNO_QUERY_THROW );
        uno::Sequence< sal_Int8 > aStorClassID = xStor->getClassID();

        if ( xNameAccess->hasByName( aSubStreamName ) )
        {
            sal_uInt8 aClassID[] =
                { 0x00, 0x03, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 };
            uno::Sequence< sal_Int8 > aPackageClassID( (sal_Int8*)aClassID, 16 );

            uno::Reference< io::XStream > xSubStream;
            xNameAccess->getByName( aSubStreamName ) >>= xSubStream;
            if ( xSubStream.is() )
            {
                sal_Bool bOk = sal_False;

                if ( ClassIDsEqual( aPackageClassID, aStorClassID ) )
                {
                    // the storage represents Object Package

                    bOk = ReadContentsAndGenerateTempFile( xSubStream->getInputStream(), sal_True );

                    if ( !bOk && m_aNativeTempURL.getLength() )
                    {
                        KillFile_Impl( m_aNativeTempURL, m_xFactory );
                        m_aNativeTempURL = ::rtl::OUString();
                    }
                }

                if ( !bOk )
                {
                    bOk = ReadContentsAndGenerateTempFile( xSubStream->getInputStream(), sal_False );

                    if ( !bOk && m_aNativeTempURL.getLength() )
                    {
                        KillFile_Impl( m_aNativeTempURL, m_xFactory );
                        m_aNativeTempURL = ::rtl::OUString();
                    }
                }
            }
        }
        else
        {
            // TODO/LATER: No native stream, needs a new solution
        }
    }
    catch( uno::Exception& )
    {}
}

//--------------------------------------------------------
sal_Bool OwnView_Impl::Open()
{
    sal_Bool bResult = sal_False;

    uno::Reference< frame::XModel > xExistingModel;

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        xExistingModel = m_xModel;
        if ( m_bBusy )
            return sal_False;

        m_bBusy = sal_True;
    }

    if ( xExistingModel.is() )
    {
        try {
            uno::Reference< frame::XController > xController = xExistingModel->getCurrentController();
            if ( xController.is() )
            {
                uno::Reference< frame::XFrame > xFrame = xController->getFrame();
                if ( xFrame.is() )
                {
                    xFrame->activate();
                    uno::Reference<awt::XTopWindow> xTopWindow( xFrame->getContainerWindow(), uno::UNO_QUERY );
                    if(xTopWindow.is())
                        xTopWindow->toFront();

                    bResult = sal_True;
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
    else
    {
        bResult = CreateModel( m_bUseNative );

        if ( !bResult && !m_bUseNative )
        {
            // the original storage can not be recognized
            if ( !m_aNativeTempURL.getLength() )
            {
                // create a temporary file for the native representation if there is no
                CreateNative();
            }

            if ( m_aNativeTempURL.getLength() )
            {
                bResult = CreateModel( sal_True );
                if ( bResult )
                    m_bUseNative = sal_True;
            }
        }
    }

    m_bBusy = sal_False;

    return bResult;
}

//--------------------------------------------------------
void OwnView_Impl::Close()
{
    uno::Reference< frame::XModel > xModel;

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xModel.is() )
            return;
        xModel = m_xModel;
        m_xModel = uno::Reference< frame::XModel >();

        if ( m_bBusy )
            return;

        m_bBusy = sal_True;
    }

    try {
        uno::Reference< document::XEventBroadcaster > xBroadCaster( xModel, uno::UNO_QUERY );
        if ( xBroadCaster.is() )
            xBroadCaster->removeEventListener( uno::Reference< document::XEventListener >(
                                                                    static_cast< ::cppu::OWeakObject* >( this ),
                                                                     uno::UNO_QUERY ) );

        uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            xCloseable->removeCloseListener( uno::Reference< util::XCloseListener >(
                                                                    static_cast< ::cppu::OWeakObject* >( this ),
                                                                     uno::UNO_QUERY ) );
            xCloseable->close( sal_True );
        }
    }
    catch( uno::Exception& )
    {}

    m_bBusy = sal_False;
}

//--------------------------------------------------------
void SAL_CALL OwnView_Impl::notifyEvent( const document::EventObject& aEvent )
        throw ( uno::RuntimeException )
{

    uno::Reference< frame::XModel > xModel;

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( aEvent.Source == m_xModel && aEvent.EventName.equalsAscii( "OnSaveAsDone" ) )
        {
            // SaveAs operation took place, so just forget the model and deregister listeners
            xModel = m_xModel;
            m_xModel = uno::Reference< frame::XModel >();
        }
    }

    if ( xModel.is() )
    {
        try {
            uno::Reference< document::XEventBroadcaster > xBroadCaster( xModel, uno::UNO_QUERY );
            if ( xBroadCaster.is() )
                xBroadCaster->removeEventListener( uno::Reference< document::XEventListener >(
                                                                        static_cast< ::cppu::OWeakObject* >( this ),
                                                                         uno::UNO_QUERY ) );

            uno::Reference< util::XCloseable > xCloseable( xModel, uno::UNO_QUERY );
            if ( xCloseable.is() )
                xCloseable->removeCloseListener( uno::Reference< util::XCloseListener >(
                                                                        static_cast< ::cppu::OWeakObject* >( this ),
                                                                         uno::UNO_QUERY ) );
        }
        catch( uno::Exception& )
        {}
    }
}

//--------------------------------------------------------
void SAL_CALL OwnView_Impl::queryClosing( const lang::EventObject&, sal_Bool )
        throw ( util::CloseVetoException,
                uno::RuntimeException )
{
}

//--------------------------------------------------------
void SAL_CALL OwnView_Impl::notifyClosing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( Source.Source == m_xModel )
        m_xModel = uno::Reference< frame::XModel >();
}

//--------------------------------------------------------
void SAL_CALL OwnView_Impl::disposing( const lang::EventObject& Source )
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( Source.Source == m_xModel )
        m_xModel = uno::Reference< frame::XModel >();
};

