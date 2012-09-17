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


#include <osl/diagnose.h>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <comphelper/componentcontext.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#ifdef WNT
#include <windows.h>
#endif
#include "odma_content.hxx"
#include "odma_contentprops.hxx"
#include "odma_provider.hxx"
#include "odma_resultset.hxx"
#include "odma_inputstream.hxx"
#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <rtl/ref.hxx>
#include <osl/file.hxx>

using namespace com::sun::star;
using namespace odma;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                  ContentProvider* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >& Identifier,
                  const ::rtl::Reference<ContentProperties>& _rProps)
    : ContentImplHelper( rxSMgr, pProvider, Identifier )
    ,m_aProps(_rProps)
    ,m_pProvider(pProvider)
    ,m_pContent(NULL)
{
    OSL_ENSURE(m_aProps.is(),"No valid ContentPropeties!");
}

//=========================================================================
// virtual
Content::~Content()
{
    delete m_pContent;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire() throw()
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release() throw()
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;

    // @@@ Add support for additional interfaces.

     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    // @@@ Add own interfaces.

    static cppu::OTypeCollection* pCollection = 0;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
            static cppu::OTypeCollection aCollection(
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
              pCollection = &aCollection;
        }
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    // @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
    return rtl::OUString("com.sun.star.comp.odma.Content");
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name.
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = rtl::OUString( ODMA_CONTENT_SERVICE_NAME );
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name ( def in odma_provider.hxx ).
    return rtl::OUString( ODMA_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            OSL_FAIL( "No properties!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name == "open" )
    {
        ucb::OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == ucb::OpenMode::ALL ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENTS ) );

        if ( bOpenFolder)
        {
            // open as folder - return result set

            uno::Reference< ucb::XDynamicResultSet > xSet
                            = new DynamicResultSet( m_xSMgr,
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
          }

        if ( aOpenCommand.Sink.is() )
        {
            // Open document - supply document data stream.

            // Check open mode
            if ( ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
            {
                // Unsupported.
                ucbhelper::cancelCommandExecution(
                    uno::makeAny( ucb::UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
            }


            rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
            rtl::OUString sFileURL = openDoc();
            delete m_pContent;
            m_pContent = new ::ucbhelper::Content
                                (sFileURL,NULL,
                                 comphelper::ComponentContext(m_xSMgr).getUNOContext());
            if(!m_pContent->isDocument())
            {
                rtl::OUString sErrorMsg("File: ");
                sErrorMsg += sFileURL;
                sErrorMsg += rtl::OUString(" could not be found.");
                ucbhelper::cancelCommandExecution(
                        uno::makeAny( io::IOException(
                                        sErrorMsg,
                                        static_cast< cppu::OWeakObject * >( this )) ),
                        Environment );
            }

            uno::Reference< io::XOutputStream > xOut
                = uno::Reference< io::XOutputStream >(
                    aOpenCommand.Sink, uno::UNO_QUERY );
            if ( xOut.is() )
              {
                // @@@ PUSH: write data into xOut
                m_pContent->openStream(xOut);
              }
            else
              {
                uno::Reference< io::XActiveDataSink > xDataSink
                    = uno::Reference< io::XActiveDataSink >(
                        aOpenCommand.Sink, uno::UNO_QUERY );
                  if ( xDataSink.is() )
                {
                      // @@@ PULL: wait for client read
                    uno::Reference< io::XInputStream > xIn;
                    try
                    {
                        xIn = m_pContent->openStream();
                    }
                    catch(uno::Exception&)
                    {
                        OSL_FAIL("Exception occurred while creating the file content!");
                    }
                    xDataSink->setInputStream( xIn );
                }
                  else
                {
                    uno::Reference< io::XActiveDataStreamer > activeDataStreamer( aOpenCommand.Sink,uno::UNO_QUERY );
                    if(activeDataStreamer.is())
                    {
                        activeDataStreamer->setStream(new OOdmaStream(m_pContent,getContentProvider(),m_aProps));
                        m_pContent = NULL; // don't delete here because the stream is now the owner
                    }
                    else
                    {
                        // Note: aOpenCommand.Sink may contain an XStream
                        //       implementation. Support for this type of
                        //       sink is optional...
                        ucbhelper::cancelCommandExecution(
                            uno::makeAny( ucb::UnsupportedDataSinkException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    aOpenCommand.Sink ) ),
                            Environment );
                        // Unreachable
                    }
                }
              }
        }
    }
    else if ( aCommand.Name == "close" )
    {
        getContentProvider()->closeDocument(m_aProps->m_sDocumentId);
    }
    else if ( aCommand.Name == "delete" )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );
        // Remove own and all childrens(!) persistent data.
        if(!getContentProvider()->deleteDocument(m_aProps))
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
    }
    else if ( aCommand.Name == "insert" )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        ucb::InsertCommandArgument arg;
          if ( !( aCommand.Argument >>= arg ) )
        {
              OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

          insert( arg.Data, arg.ReplaceExisting, Environment );
    }
    else if( ! aCommand.Name.compareToAscii( "transfer" ) )
    {
        // So far I have determined that this command is called when
        // doing "Save As" to copy an already written backup copy of
        // the document in the file system into the DMS.

        // Maybe also in other situations.

        ucb::TransferInfo aTransferInfo;
        if( ! ( aCommand.Argument >>= aTransferInfo ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        ::rtl::Reference<ContentProperties> aProp = m_aProps;
        if(aProp->m_bIsFolder)
        {
            aProp = getContentProvider()->getContentPropertyWithDocumentId(aTransferInfo.NewTitle);
            if(!aProp.is())
                aProp = getContentProvider()->getContentPropertyWithSavedAsName(aTransferInfo.NewTitle);
            sal_Bool bError = !aProp.is();

            // There used to be code below that called ODMSaveAsEx,
            // but that was very broken. We have already called
            // ODMSaveAsEx in the ODMA file picker when selecting the
            // name for a new document, or the document already exists
            // in the DMS and we don't need any ODMSaveAsEx. The ODMA
            // file picker tells odma::ContentProvider about the new
            // document's DOCID, so the
            // getContentPropertyWithDocumentId() call above should
            // succeed.

            if(bError)
                ucbhelper::cancelCommandExecution(
                        uno::makeAny( lang::IllegalArgumentException(
                                            rtl::OUString(),
                                            static_cast< cppu::OWeakObject * >( this ),
                                            -1 ) ),
                        Environment );
        }
        rtl::OUString sFileURL = ContentProvider::openDoc(aProp);

        sal_Int32 nLastIndex = sFileURL.lastIndexOf( sal_Unicode('/') );
        // Create a new Content object for the "shadow" file
        // corresponding to the opened document from the DMS.
        ::ucbhelper::Content aContent(sFileURL.copy(0,nLastIndex),NULL,
                                      comphelper::ComponentContext(m_xSMgr).getUNOContext());
        //  aTransferInfo.NameClash = ucb::NameClash::OVERWRITE;
        aTransferInfo.NewTitle = sFileURL.copy( 1 + nLastIndex );
        // Copy our saved backup copy to the "shadow" file.
        aContent.executeCommand(::rtl::OUString("transfer"),uno::makeAny(aTransferInfo));
        // Tell the DMS that the "shadow" file is done and can be
        // imported.
        getContentProvider()->saveDocument(aProp->m_sDocumentId);
    }
    else if ( aCommand.Name == "getCasePreservingURL" )
    {
        rtl::OUString CasePreservingURL = openDoc();
        aRet <<= CasePreservingURL;
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        OSL_FAIL( "Content::execute - unsupported command!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
    // @@@ Implement logic to abort running commands, if this makes
    //     sense for your content.
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// virtual
::rtl::OUString Content::getParentURL()
{
    ::rtl::OUString sURL = m_xIdentifier->getContentIdentifier();

    // @@@ Extract URL of parent from aURL and return it...
    static ::rtl::OUString sScheme1(ODMA_URL_SCHEME ODMA_URL_SHORT "/");
    static ::rtl::OUString sScheme2(ODMA_URL_SCHEME ODMA_URL_SHORT);
    if(sURL == sScheme1 || sURL == sScheme2)
        sURL = ::rtl::OUString();
    else
        sURL = sScheme1;

    return sURL;
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
            const uno::Sequence< beans::Property >& rProperties,
            const rtl::Reference<ContentProperties>& rData,
            const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
            const rtl::OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
                                = new ::ucbhelper::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name == "ContentType" )
            {
                xRow->appendString ( rProp, rData->m_sContentType );
            }
            else if ( rProp.Name == "Title" )
            {
                xRow->appendString ( rProp, rData->m_sTitle );
            }
            else if ( rProp.Name == "IsDocument" )
            {
                xRow->appendBoolean( rProp, rData->m_bIsDocument );
            }
            else if ( rProp.Name == "IsFolder" )
            {
                xRow->appendBoolean( rProp, rData->m_bIsFolder );
            }
            else if ( rProp.Name == "DateCreated" )
            {
                xRow->appendTimestamp( rProp, rData->m_aDateCreated );
            }
            else if ( rProp.Name == "DateModified" )
            {
                xRow->appendTimestamp( rProp, rData->m_aDateModified );
            }
            else if ( rProp.Name == "IsReadOnly" )
            {
                xRow->appendBoolean( rProp, rData->m_bIsReadOnly );
            }
            else if ( rProp.Name == "Author" )
            {
                xRow->appendString ( rProp, rData->m_sAuthor );
            }
            else if ( rProp.Name == "Subject" )
            {
                xRow->appendString ( rProp, rData->m_sSubject );
            }
            else if ( rProp.Name == "Keywords" )
            {
                xRow->appendString ( rProp, rData->m_sKeywords );
            }
            else
            {
                // @@@ Note: If your data source supports adding/removing
                //     properties, you should implement the interface
                //     XPropertyContainer by yourself and supply your own
                //     logic here. The base class uses the service
                //     "com.sun.star.ucb.Store" to maintain Additional Core
                //     properties. But using server functionality is preferred!

                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        // Append empty entry.
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            beans::Property( rtl::OUString("ContentType"),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_sContentType );
        xRow->appendString (
            beans::Property( rtl::OUString("Title"),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData->m_sTitle );
        xRow->appendBoolean(
            beans::Property( rtl::OUString("IsDocument"),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_bIsDocument );
        xRow->appendBoolean(
            beans::Property( rtl::OUString("IsFolder"),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_bIsFolder );

        // @@@ Append other properties supported directly.
        xRow->appendTimestamp(
            beans::Property( rtl::OUString("DateCreated"),
                      -1,
                      getCppuType(static_cast< const util::DateTime * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_aDateCreated );
        xRow->appendTimestamp(
            beans::Property( rtl::OUString("DateModified"),
                      -1,
                      getCppuType(static_cast< const util::DateTime * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_aDateModified );
        xRow->appendBoolean(
            beans::Property( rtl::OUString("IsReadOnly"),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData->m_bIsReadOnly );
        xRow->appendString (
            beans::Property( rtl::OUString("Author"),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData->m_sAuthor );
        xRow->appendString (
            beans::Property( rtl::OUString("Subject"),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData->m_sSubject );
        xRow->appendString (
            beans::Property( rtl::OUString("Keywords"),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData->m_sKeywords );

        // @@@ Note: If your data source supports adding/removing
        //     properties, you should implement the interface
        //     XPropertyContainer by yourself and supply your own
        //     logic here. The base class uses the service
        //     "com.sun.star.ucb.Store" to maintain Additional Core
        //     properties. But using server functionality is preferred!

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              rtl::Reference<
                                ::ucbhelper::ContentProviderImplHelper >(
                                    m_xProvider.get() ),
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name == "Title" )
        {
            changePropertyValue(rValue,n,m_aProps->m_sTitle,nChanged,aRet,aChanges);
        }
        else if ( rValue.Name == "Author" )
        {
            changePropertyValue(rValue,n,m_aProps->m_sAuthor,nChanged,aRet,aChanges);
        }
        else if ( rValue.Name == "Keywords" )
        {
            changePropertyValue(rValue,n,m_aProps->m_sKeywords,nChanged,aRet,aChanges);
        }
        else if ( rValue.Name == "Subject" )
        {
            changePropertyValue(rValue,n,m_aProps->m_sSubject,nChanged,aRet,aChanges);
        }
        else if (   rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "ContentType" ) )  ||
                    rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) )   ||
                    rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) )     ||
                    rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) )  ||
                    rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) ||
                    rValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM( "IsReadOnly" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else
        {
            // @@@ Note: If your data source supports adding/removing
            //     properties, you should implement the interface
            //     XPropertyContainer by yourself and supply your own
            //     logic here. The base class uses the service
            //     "com.sun.star.ucb.Store" to maintain Additional Core
            //     properties. But using server functionality is preferred!

            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    uno::Any aOldValue
                        = xAdditionalPropSet->getPropertyValue( rValue.Name );
                    if ( aOldValue != rValue.Value )
                    {
                        xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                    else
                    {
                        // Old value equals new value. No error!
                    }
                }
                catch ( beans::UnknownPropertyException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRet[ n ] <<= e;
                }
            }
            else
            {
                aRet[ n ] <<= uno::Exception(
                                rtl::OUString( "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
    }

    if ( nChanged > 0 )
    {
        // @@@ Save changes.
//      storeData();

        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}
//=========================================================================
void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.
    if ( !m_aProps->m_sTitle.getLength())
    {
        OSL_FAIL( "Content::insert - property value missing!" );

        uno::Sequence< rtl::OUString > aProps( 1 );
        aProps[ 0 ] = rtl::OUString("zzzz");
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        // Unreachable
    }

    if ( !xInputStream.is() )
    {
        OSL_FAIL( "Content::insert - No data stream!" );

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingInputStreamException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    // Assemble new content identifier...

    //  uno::Reference< ucb::XContentIdentifier > xId = ...;

    // Fail, if a resource with given id already exists.
    if ( !bReplaceExisting ) // && hasData( m_xIdentifier ) )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
//        ucbhelper::cancelCommandExecution(
//                      ucb::IOErrorCode_ALREADY_EXISTING,
//                      Environment,
//                      uno::makeAny(static_cast< cppu::OWeakObject * >( this ))
//                         );
        // Unreachable
    }

    //  m_xIdentifier = xId;

//  @@@
//  storeData();

    aGuard.clear();
    inserted();
}

// -----------------------------------------------------------------------------
::rtl::OUString Content::openDoc()
{
    OSL_ENSURE(m_aProps.is(),"No valid content properties!");
    return ContentProvider::openDoc(m_aProps);
}
// -----------------------------------------------------------------------------
void Content::changePropertyValue(const beans::PropertyValue& _rValue,
                                  sal_Int32 _rnCurrentPos,
                                  ::rtl::OUString& _rsMemberValue,
                                  sal_Int32& _rnChanged,
                                  uno::Sequence< uno::Any >& _rRet,
                                  uno::Sequence< beans::PropertyChangeEvent >& _rChanges) throw (beans::IllegalTypeException)
{
    rtl::OUString sNewValue;
    sal_Bool bError = sal_False;
    if ( _rValue.Value >>= sNewValue )
    {
        if ( sNewValue != _rsMemberValue )
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            // first we have to check if we could change the property inside the DMS
            ::rtl::OString sDocInfoValue = ::rtl::OUStringToOString(sNewValue,RTL_TEXTENCODING_ASCII_US);
            WORD nDocInfo = 0;
            if(&_rsMemberValue == &m_aProps->m_sTitle)
                nDocInfo = ODM_TITLETEXT;
            else if(&_rsMemberValue == &m_aProps->m_sAuthor)
                nDocInfo = ODM_AUTHOR;
            else if(&_rsMemberValue == &m_aProps->m_sSubject)
                nDocInfo = ODM_SUBJECT;
            else if(&_rsMemberValue == &m_aProps->m_sKeywords)
                nDocInfo = ODM_KEYWORDS;
            else
                bError = sal_True;

            if(!bError)
            {
                ODMSTATUS odm = NODMSetDocInfo( ContentProvider::getHandle(),
                                                const_cast<sal_Char*>(m_aProps->m_sDocumentId.getStr()),
                                                nDocInfo,
                                                const_cast<sal_Char*>(sDocInfoValue.getStr())
                                                );
                if(odm == ODM_SUCCESS)
                {
                    beans::PropertyChangeEvent aEvent;
                    aEvent.Source           = static_cast< cppu::OWeakObject * >( this );
                    aEvent.Further          = sal_False;
                    aEvent.PropertyHandle   = -1;
                    aEvent.PropertyName     = _rValue.Name;
                    aEvent.OldValue         = uno::makeAny( _rsMemberValue );
                    aEvent.NewValue         = uno::makeAny( sNewValue );

                    _rChanges.getArray()[ _rnChanged ] = aEvent;

                    _rsMemberValue = sNewValue;
                    ++_rnChanged;
                }
            }
        }
        else
        {
            // Old value equals new value. No error!
        }
    }
    else
        bError = sal_True;

    if(bError)
    {
        _rRet[ _rnCurrentPos ] <<= beans::IllegalTypeException(
                        rtl::OUString( "Property value has wrong type!" ),
                        static_cast< cppu::OWeakObject * >( this ) );
    }
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
