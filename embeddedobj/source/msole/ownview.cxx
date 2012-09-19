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

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/mimeconfighelper.hxx>

#include "ownview.hxx"


using namespace ::com::sun::star;
using namespace ::comphelper;

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
        if ( !m_aNativeTempURL.isEmpty() )
            KillFile_Impl( m_aNativeTempURL, m_xFactory );
    } catch( uno::Exception& ) {}
}

//--------------------------------------------------------
sal_Bool OwnView_Impl::CreateModelFromURL( const ::rtl::OUString& aFileURL )
{
    sal_Bool bResult = sal_False;

    if ( !aFileURL.isEmpty() )
    {
        try {
            uno::Reference < frame::XComponentLoader > xDocumentLoader(
                            m_xFactory->createInstance (
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) )),
                            uno::UNO_QUERY );

            if ( xDocumentLoader.is() )
            {
                uno::Sequence< beans::PropertyValue > aArgs( m_aFilterName.isEmpty() ? 4 : 5 );

                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "URL" ));
                aArgs[0].Value <<= aFileURL;

                aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ReadOnly" ));
                aArgs[1].Value <<= sal_True;

                aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
                aArgs[2].Value <<= uno::Reference< task::XInteractionHandler >(
                                    static_cast< ::cppu::OWeakObject* >( new DummyHandler_Impl() ), uno::UNO_QUERY );

                aArgs[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DontEdit" ));
                aArgs[3].Value <<= sal_True;

                if ( !m_aFilterName.isEmpty() )
                {
                    aArgs[4].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
                    aArgs[4].Value <<= m_aFilterName;
                }

                uno::Reference< frame::XModel > xModel( xDocumentLoader->loadComponentFromURL(
                                                                aFileURL,
                                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "_blank" )),
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
::rtl::OUString OwnView_Impl::GetFilterNameFromExtentionAndInStream(
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
                                                    const ::rtl::OUString& aNameWithExtention,
                                                    const uno::Reference< io::XInputStream >& xInputStream )
{
    if ( !xInputStream.is() )
        throw uno::RuntimeException();

    uno::Reference< document::XTypeDetection > xTypeDetection(
            xFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ) )),
            uno::UNO_QUERY_THROW );

    ::rtl::OUString aTypeName;

    if ( !aNameWithExtention.isEmpty() )
    {
        ::rtl::OUString aURLToAnalyze =
                ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "file:///" ) ) + aNameWithExtention );
        aTypeName = xTypeDetection->queryTypeByURL( aURLToAnalyze );
    }

    uno::Sequence< beans::PropertyValue > aArgs( aTypeName.isEmpty() ? 2 : 3 );
    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
    aArgs[0].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" ) );
    aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) );
    aArgs[1].Value <<= xInputStream;
    if ( !aTypeName.isEmpty() )
    {
        aArgs[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TypeName" ) );
        aArgs[2].Value <<= aTypeName;
    }

    aTypeName = xTypeDetection->queryTypeByDescriptor( aArgs, sal_True );

    ::rtl::OUString aFilterName;
    for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
        if ( aArgs[nInd].Name == "FilterName" )
            aArgs[nInd].Value >>= aFilterName;

    if ( aFilterName.isEmpty() && !aTypeName.isEmpty() )
    {
        // get the default filter name for the type
        uno::Reference< container::XNameAccess > xNameAccess( xTypeDetection, uno::UNO_QUERY_THROW );
        uno::Sequence< beans::PropertyValue > aTypes;

        if ( xNameAccess.is() && ( xNameAccess->getByName( aTypeName ) >>= aTypes ) )
        {
            for ( sal_Int32 nInd = 0; nInd < aTypes.getLength(); nInd++ )
            {
                if ( aTypes[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "PreferredFilter" ) ) && ( aTypes[nInd].Value >>= aFilterName ) )
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
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY_THROW );
    uno::Reference < io::XStream > xNativeTempStream( xNativeTempFile, uno::UNO_QUERY_THROW );
    uno::Reference < io::XOutputStream > xNativeOutTemp = xNativeTempStream->getOutputStream();
    uno::Reference < io::XInputStream > xNativeInTemp = xNativeTempStream->getInputStream();
    if ( !xNativeOutTemp.is() || !xNativeInTemp.is() )
        throw uno::RuntimeException();

    try {
        xNativeTempFile->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RemoveFile" )), uno::makeAny( sal_False ) );
        uno::Any aUrl = xNativeTempFile->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Uri" ) ));
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
        // read the first header ( have no idea what does this header mean )
        if ( xInStream->readBytes( aReadSeq, 2 ) != 2 || aReadSeq[0] != 2 || aReadSeq[1] != 0 )
            return sal_False;

        // read file name
        // only extension is interesting so only subset of symbols is accepted
        do
        {
            if ( xInStream->readBytes( aReadSeq, 1 ) != 1 )
                return sal_False;

            if (
                (aReadSeq[0] >= '0' && aReadSeq[0] <= '9') ||
                (aReadSeq[0] >= 'a' && aReadSeq[0] <= 'z') ||
                (aReadSeq[0] >= 'A' && aReadSeq[0] <= 'Z') ||
                aReadSeq[0] == '.'
               )
            {
                aFileSuffix += ::rtl::OUString::valueOf( (sal_Unicode) aReadSeq[0] );
            }

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
        m_aFilterName = GetFilterNameFromExtentionAndInStream( m_xFactory, aFileSuffix, xNativeInTemp );
        m_aNativeTempURL = aNativeTempURL;
    }

    return !bFailed;
}

//--------------------------------------------------------
void OwnView_Impl::CreateNative()
{
    if ( !m_aNativeTempURL.isEmpty() )
        return;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess2 > xAccess(
                ucb::SimpleFileAccess::create( comphelper::getComponentContext(m_xFactory) ) );

        uno::Reference< io::XInputStream > xInStream = xAccess->openFileRead( m_aTempFileURL );
        if ( !xInStream.is() )
            throw uno::RuntimeException();

        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= xInStream;
        uno::Reference< container::XNameAccess > xNameAccess(
                m_xFactory->createInstanceWithArguments(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLESimpleStorage" )),
                        aArgs ),
                uno::UNO_QUERY_THROW );

        ::rtl::OUString aSubStreamName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "\1Ole10Native" ));
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

                if ( MimeConfigurationHelper::ClassIDsEqual( aPackageClassID, aStorClassID ) )
                {
                    // the storage represents Object Package

                    bOk = ReadContentsAndGenerateTempFile( xSubStream->getInputStream(), sal_True );

                    if ( !bOk && !m_aNativeTempURL.isEmpty() )
                    {
                        KillFile_Impl( m_aNativeTempURL, m_xFactory );
                        m_aNativeTempURL = ::rtl::OUString();
                    }
                }

                if ( !bOk )
                {
                    bOk = ReadContentsAndGenerateTempFile( xSubStream->getInputStream(), sal_False );

                    if ( !bOk && !m_aNativeTempURL.isEmpty() )
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
            if ( m_aNativeTempURL.isEmpty() )
            {
                // create a temporary file for the native representation if there is no
                CreateNative();
            }

            if ( !m_aNativeTempURL.isEmpty() )
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
        if ( aEvent.Source == m_xModel && aEvent.EventName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OnSaveAsDone" ) ) )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
