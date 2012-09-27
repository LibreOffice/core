/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <commonembobj.hxx>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <rtl/logfile.hxx>

#include <tools/diagnose_ex.h>

#define USE_STORAGEBASED_DOCUMENT

using namespace ::com::sun::star;


//------------------------------------------------------
uno::Sequence< beans::PropertyValue > GetValuableArgs_Impl( const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                            sal_Bool bCanUseDocumentBaseURL )
{
    uno::Sequence< beans::PropertyValue > aResult;
    sal_Int32 nResLen = 0;

    for ( sal_Int32 nInd = 0; nInd < aMedDescr.getLength(); nInd++ )
    {
        if ( aMedDescr[nInd].Name == "ComponentData" || aMedDescr[nInd].Name == "DocumentTitle"
          || aMedDescr[nInd].Name == "InteractionHandler" || aMedDescr[nInd].Name == "JumpMark"
          // || aMedDescr[nInd].Name == "Password" // makes no sence for embedded objects
          || aMedDescr[nInd].Name == "Preview" || aMedDescr[nInd].Name == "ReadOnly"
          || aMedDescr[nInd].Name == "StartPresentation" || aMedDescr[nInd].Name == "RepairPackage"
          || aMedDescr[nInd].Name == "StatusIndicator" || aMedDescr[nInd].Name == "ViewData"
          || aMedDescr[nInd].Name == "ViewId" || aMedDescr[nInd].Name == "MacroExecutionMode"
          || aMedDescr[nInd].Name == "UpdateDocMode"
          || (aMedDescr[nInd].Name == "DocumentBaseURL" && bCanUseDocumentBaseURL) )
        {
            aResult.realloc( ++nResLen );
            aResult[nResLen-1] = aMedDescr[nInd];
        }
    }

    return aResult;
}

//------------------------------------------------------
uno::Sequence< beans::PropertyValue > addAsTemplate( const uno::Sequence< beans::PropertyValue >& aOrig )
{
    sal_Bool bAsTemplateSet = sal_False;
    sal_Int32 nLength = aOrig.getLength();
    uno::Sequence< beans::PropertyValue > aResult( nLength );

    for ( sal_Int32 nInd = 0; nInd < nLength; nInd++ )
    {
        aResult[nInd].Name = aOrig[nInd].Name;
        if ( aResult[nInd].Name == "AsTemplate" )
        {
            aResult[nInd].Value <<= sal_True;
            bAsTemplateSet = sal_True;
        }
        else
            aResult[nInd].Value = aOrig[nInd].Value;
    }

    if ( !bAsTemplateSet )
    {
        aResult.realloc( nLength + 1 );
        aResult[nLength].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "AsTemplate" ));
        aResult[nLength].Value <<= sal_True;
    }

    return aResult;
}

//------------------------------------------------------
uno::Reference< io::XInputStream > createTempInpStreamFromStor(
                                                            const uno::Reference< embed::XStorage >& xStorage,
                                                            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    OSL_ENSURE( xStorage.is(), "The storage can not be empty!" );

    uno::Reference< io::XInputStream > xResult;

    uno::Reference < io::XStream > xTempStream( io::TempFile::create(comphelper::getComponentContext(xFactory)),
                                                            uno::UNO_QUERY_THROW );

    uno::Reference < lang::XSingleServiceFactory > xStorageFactory(
                xFactory->createInstance ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.StorageFactory" )) ),
                uno::UNO_QUERY );

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xTempStream;
    aArgs[1] <<= embed::ElementModes::READWRITE;
    uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException(); // TODO:

    try
    {
        xStorage->copyToStorage( xTempStorage );
    } catch( const uno::Exception& e )
    {
        throw embed::StorageWrappedTargetException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't copy storage!" )),
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
    }

    try {
        uno::Reference< lang::XComponent > xComponent( xTempStorage, uno::UNO_QUERY );
        OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( const uno::Exception& )
    {
    }

    try {
        uno::Reference< io::XOutputStream > xTempOut = xTempStream->getOutputStream();
        if ( xTempOut.is() )
            xTempOut->closeOutput();
    }
    catch ( const uno::Exception& )
    {
    }

    xResult = xTempStream->getInputStream();

    return xResult;

}

//------------------------------------------------------
static void TransferMediaType( const uno::Reference< embed::XStorage >& i_rSource, const uno::Reference< embed::XStorage >& i_rTarget )
{
    try
    {
        const uno::Reference< beans::XPropertySet > xSourceProps( i_rSource, uno::UNO_QUERY_THROW );
        const uno::Reference< beans::XPropertySet > xTargetProps( i_rTarget, uno::UNO_QUERY_THROW );
        const ::rtl::OUString sMediaTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
        xTargetProps->setPropertyValue( sMediaTypePropName, xSourceProps->getPropertyValue( sMediaTypePropName ) );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------
static uno::Reference< util::XCloseable > CreateDocument( const uno::Reference< lang::XMultiServiceFactory >& _rxFactory,
    const ::rtl::OUString& _rDocumentServiceName, bool _bEmbeddedScriptSupport, const bool i_bDocumentRecoverySupport )
{
    ::comphelper::NamedValueCollection aArguments;
    aArguments.put( "EmbeddedObject", (sal_Bool)sal_True );
    aArguments.put( "EmbeddedScriptSupport", (sal_Bool)_bEmbeddedScriptSupport );
    aArguments.put( "DocumentRecoverySupport", (sal_Bool)i_bDocumentRecoverySupport );

    uno::Reference< uno::XInterface > xDocument;
    try
    {
        xDocument = _rxFactory->createInstanceWithArguments( _rDocumentServiceName, aArguments.getWrappedPropertyValues() );
    }
    catch( const uno::Exception& )
    {
        // if an embedded object implementation does not support XInitialization,
        // the default factory from cppuhelper will throw an
        // IllegalArgumentException when we try to create the instance with arguments.
        // Okay, so we fall back to creating the instance without any arguments.
        OSL_ASSERT("Consider implementing interface XInitialization to avoid duplicate construction");
        xDocument = _rxFactory->createInstance( _rDocumentServiceName );
    }

    return uno::Reference< util::XCloseable >( xDocument, uno::UNO_QUERY );
}

//------------------------------------------------------
static void SetDocToEmbedded( const uno::Reference< frame::XModel > xDocument, const ::rtl::OUString& aModuleName )
{
    if ( xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aSeq( 1 );
        aSeq[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "SetEmbedded" ));
        aSeq[0].Value <<= sal_True;
        xDocument->attachResource( ::rtl::OUString(), aSeq );

        if ( !aModuleName.isEmpty() )
        {
            try
            {
                uno::Reference< frame::XModule > xModule( xDocument, uno::UNO_QUERY_THROW );
                xModule->setIdentifier( aModuleName );
            }
            catch( const uno::Exception& )
            {}
        }
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                                  const uno::Reference< embed::XStorage >& xNewObjectStorage,
                                                  const ::rtl::OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
    {
        OSL_ENSURE( xNewObjectStorage == m_xObjectStorage, "The storage must be the same!\n" );
        return;
    }

    uno::Reference< lang::XComponent > xComponent( m_xObjectStorage, uno::UNO_QUERY );
    OSL_ENSURE( !m_xObjectStorage.is() || xComponent.is(), "Wrong storage implementation!" );

    m_xObjectStorage = xNewObjectStorage;
    m_xParentStorage = xNewParentStorage;
    m_aEntryName = aNewName;

#ifdef USE_STORAGEBASED_DOCUMENT
    // the linked document should not be switched
    if ( !m_bIsLink )
    {
        uno::Reference< document::XStorageBasedDocument > xDoc( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xDoc.is() )
            SwitchDocToStorage_Impl( xDoc, m_xObjectStorage );
    }
#endif

    try {
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( const uno::Exception& )
    {
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                                  const ::rtl::OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
        return;

    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    uno::Reference< embed::XStorage > xNewOwnStorage = xNewParentStorage->openStorageElement( aNewName, nStorageMode );
    OSL_ENSURE( xNewOwnStorage.is(), "The method can not return empty reference!" );

    SwitchOwnPersistence( xNewParentStorage, xNewOwnStorage, aNewName );
}

//------------------------------------------------------
void OCommonEmbeddedObject::EmbedAndReparentDoc_Impl( const uno::Reference< util::XCloseable >& i_rxDocument ) const
{
    SetDocToEmbedded( uno::Reference< frame::XModel >( i_rxDocument, uno::UNO_QUERY ), m_aModuleName );

    try
    {
        uno::Reference < container::XChild > xChild( i_rxDocument, uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( m_xParent );
    }
    catch( const lang::NoSupportException & )
    {
        OSL_FAIL( "OCommonEmbeddedObject::EmbedAndReparentDoc: cannot set parent at document!" );
    }
}

//------------------------------------------------------
uno::Reference< util::XCloseable > OCommonEmbeddedObject::InitNewDocument_Impl()
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xFactory, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XModel > xModel( xDocument, uno::UNO_QUERY );
    uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    try
    {
        // set the document mode to embedded as the first action on document!!!
        EmbedAndReparentDoc_Impl( xDocument );

        // if we have a storage to recover the document from, do not use initNew, but instead load from that storage
        bool bInitNew = true;
        if ( m_xRecoveryStorage.is() )
        {
            uno::Reference< document::XStorageBasedDocument > xDoc( xLoadable, uno::UNO_QUERY );
            OSL_ENSURE( xDoc.is(), "OCommonEmbeddedObject::InitNewDocument_Impl: cannot recover from a storage when the document is not storage based!" );
            if ( xDoc.is() )
            {
                ::comphelper::NamedValueCollection aLoadArgs;
                FillDefaultLoadArgs_Impl( m_xRecoveryStorage, aLoadArgs );

                xDoc->loadFromStorage( m_xRecoveryStorage, aLoadArgs.getPropertyValues() );
                SwitchDocToStorage_Impl( xDoc, m_xObjectStorage );
                bInitNew = false;
            }
        }

        if ( bInitNew )
        {
            // init document as a new
            xLoadable->initNew();
        }
        xModel->attachResource( xModel->getURL(), m_aDocMediaDescriptor );
    }
    catch( const uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< util::XCloseable > OCommonEmbeddedObject::LoadLink_Impl()
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xFactory, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    sal_Int32 nLen = 2;
    uno::Sequence< beans::PropertyValue > aArgs( nLen );
    aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "URL" ));
    aArgs[0].Value <<= m_aLinkURL;
    aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
    aArgs[1].Value <<= m_aLinkFilterName;
    if ( m_bLinkHasPassword )
    {
        aArgs.realloc( ++nLen );
        aArgs[nLen-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Password" ) );
        aArgs[nLen-1].Value <<= m_aLinkPassword;
    }

    aArgs.realloc( m_aDocMediaDescriptor.getLength() + nLen );
    for ( sal_Int32 nInd = 0; nInd < m_aDocMediaDescriptor.getLength(); nInd++ )
    {
        aArgs[nInd+nLen].Name = m_aDocMediaDescriptor[nInd].Name;
        aArgs[nInd+nLen].Value = m_aDocMediaDescriptor[nInd].Value;
    }

    try
    {
        // the document is not really an embedded one, it is a link
        EmbedAndReparentDoc_Impl( xDocument );

        // load the document
        xLoadable->load( aArgs );

        if ( !m_bLinkHasPassword )
        {
            // check if there is a password to cache
            uno::Reference< frame::XModel > xModel( xLoadable, uno::UNO_QUERY_THROW );
            uno::Sequence< beans::PropertyValue > aProps = xModel->getArgs();
            for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
                if ( aProps[nInd].Name == "Password" && ( aProps[nInd].Value >>= m_aLinkPassword ) )
                {
                    m_bLinkHasPassword = sal_True;
                    break;
                }
        }
    }
    catch( const uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;

}

//------------------------------------------------------
::rtl::OUString OCommonEmbeddedObject::GetFilterName( sal_Int32 nVersion ) const
{
    ::rtl::OUString aFilterName = GetPresetFilterName();
    if ( aFilterName.isEmpty() )
    {
        try {
            ::comphelper::MimeConfigurationHelper aHelper( m_xFactory );
            aFilterName = aHelper.GetDefaultFilterFromServiceName( GetDocumentServiceName(), nVersion );
        } catch( const uno::Exception& )
        {}
    }

    return aFilterName;
}

//------------------------------------------------------
void OCommonEmbeddedObject::FillDefaultLoadArgs_Impl( const uno::Reference< embed::XStorage >& i_rxStorage,
        ::comphelper::NamedValueCollection& o_rLoadArgs ) const
{
    o_rLoadArgs.put( "DocumentBaseURL", GetBaseURL_Impl() );
    o_rLoadArgs.put( "HierarchicalDocumentName", m_aEntryName );
    o_rLoadArgs.put( "ReadOnly", m_bReadOnly );

    ::rtl::OUString aFilterName = GetFilterName( ::comphelper::OStorageHelper::GetXStorageFormat( i_rxStorage ) );
    OSL_ENSURE( !aFilterName.isEmpty(), "OCommonEmbeddedObject::FillDefaultLoadArgs_Impl: Wrong document service name!" );
    if ( aFilterName.isEmpty() )
        throw io::IOException();    // TODO: error message/code

    o_rLoadArgs.put( "FilterName", aFilterName );
}

//------------------------------------------------------
uno::Reference< util::XCloseable > OCommonEmbeddedObject::LoadDocumentFromStorage_Impl()
{
    ENSURE_OR_THROW( m_xObjectStorage.is(), "no object storage" );

    const uno::Reference< embed::XStorage > xSourceStorage( m_xRecoveryStorage.is() ? m_xRecoveryStorage : m_xObjectStorage );

    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xFactory, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    //#i103460# ODF: take the size given from the parent frame as default
    uno::Reference< chart2::XChartDocument > xChart( xDocument, uno::UNO_QUERY );
    if( xChart.is() )
    {
        uno::Reference< embed::XVisualObject > xChartVisualObject( xChart, uno::UNO_QUERY );
        if( xChartVisualObject.is() )
            xChartVisualObject->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, m_aDefaultSizeForChart_In_100TH_MM );
    }

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    uno::Reference< document::XStorageBasedDocument > xDoc
#ifdef USE_STORAGEBASED_DOCUMENT
            ( xDocument, uno::UNO_QUERY )
#endif
            ;
    if ( !xDoc.is() && !xLoadable.is() ) ///BUG: This should be || instead of && ?
        throw uno::RuntimeException();

    ::comphelper::NamedValueCollection aLoadArgs;
    FillDefaultLoadArgs_Impl( xSourceStorage, aLoadArgs );

    uno::Reference< io::XInputStream > xTempInpStream;
    if ( !xDoc.is() )
    {
        xTempInpStream = createTempInpStreamFromStor( xSourceStorage, m_xFactory );
        if ( !xTempInpStream.is() )
            throw uno::RuntimeException();

        ::rtl::OUString aTempFileURL;
        try
        {
            // no need to let the file stay after the stream is removed since the embedded document
            // can not be stored directly
            uno::Reference< beans::XPropertySet > xTempStreamProps( xTempInpStream, uno::UNO_QUERY_THROW );
            xTempStreamProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Uri" )) ) >>= aTempFileURL;
        }
        catch( const uno::Exception& )
        {
        }

        OSL_ENSURE( !aTempFileURL.isEmpty(), "Coudn't retrieve temporary file URL!\n" );

        aLoadArgs.put( "URL", aTempFileURL );
        aLoadArgs.put( "InputStream", xTempInpStream );
    }


    aLoadArgs.merge( m_aDocMediaDescriptor, true );

    try
    {
        // set the document mode to embedded as the first step!!!
        EmbedAndReparentDoc_Impl( xDocument );

        if ( xDoc.is() )
        {
            xDoc->loadFromStorage( xSourceStorage, aLoadArgs.getPropertyValues() );
            if ( xSourceStorage != m_xObjectStorage )
                SwitchDocToStorage_Impl( xDoc, m_xObjectStorage );
        }
        else
            xLoadable->load( aLoadArgs.getPropertyValues() );
    }
    catch( const uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< io::XInputStream > OCommonEmbeddedObject::StoreDocumentToTempStream_Impl(
                                                                            sal_Int32 nStorageFormat,
                                                                            const ::rtl::OUString& aBaseURL,
                                                                            const ::rtl::OUString& aHierarchName )
{
    uno::Reference < io::XOutputStream > xTempOut(
                io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
                uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > aResult( xTempOut, uno::UNO_QUERY );

    if ( !aResult.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< frame::XStorable > xStorable;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pDocHolder )
            xStorable = uno::Reference< frame::XStorable > ( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    }

    if( !xStorable.is() )
        throw uno::RuntimeException(); // TODO:

    ::rtl::OUString aFilterName = GetFilterName( nStorageFormat );

    OSL_ENSURE( !aFilterName.isEmpty(), "Wrong document service name!" );
    if ( aFilterName.isEmpty() )
        throw io::IOException(); // TODO:

    uno::Sequence< beans::PropertyValue > aArgs( 4 );
    aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
    aArgs[0].Value <<= aFilterName;
    aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OutputStream" ));
    aArgs[1].Value <<= xTempOut;
    aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DocumentBaseURL" ));
    aArgs[2].Value <<= aBaseURL;
    aArgs[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HierarchicalDocumentName" ));
    aArgs[3].Value <<= aHierarchName;

    xStorable->storeToURL( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "private:stream" )), aArgs );
    try
    {
        xTempOut->closeOutput();
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Looks like stream was closed already" );
    }

    return aResult;
}

//------------------------------------------------------
void OCommonEmbeddedObject::SaveObject_Impl()
{
    if ( m_xClientSite.is() )
    {
        try
        {
            // check whether the component is modified,
            // if not there is no need for storing
            uno::Reference< util::XModifiable > xModifiable( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
            if ( xModifiable.is() && !xModifiable->isModified() )
                return;
        }
        catch( const uno::Exception& )
        {}

        try {
            m_xClientSite->saveObject();
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "The object was not stored!\n" );
        }
    }
}

//------------------------------------------------------
::rtl::OUString OCommonEmbeddedObject::GetBaseURL_Impl() const
{
    ::rtl::OUString aBaseURL;
    sal_Int32 nInd = 0;

    if ( m_xClientSite.is() )
    {
        try
        {
            uno::Reference< frame::XModel > xParentModel( m_xClientSite->getComponent(), uno::UNO_QUERY_THROW );
            uno::Sequence< beans::PropertyValue > aModelProps = xParentModel->getArgs();
            for ( nInd = 0; nInd < aModelProps.getLength(); nInd++ )
                if ( aModelProps[nInd].Name.equals(
                                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentBaseURL" ) ) ) )
                {
                    aModelProps[nInd].Value >>= aBaseURL;
                    break;
                }


        }
        catch( const uno::Exception& )
        {}
    }

    if ( aBaseURL.isEmpty() )
    {
        for ( nInd = 0; nInd < m_aDocMediaDescriptor.getLength(); nInd++ )
            if ( m_aDocMediaDescriptor[nInd].Name.equals(
                                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentBaseURL" ) ) ) )
            {
                m_aDocMediaDescriptor[nInd].Value >>= aBaseURL;
                break;
            }
    }

    if ( aBaseURL.isEmpty() )
        aBaseURL = m_aDefaultParentBaseURL;

    return aBaseURL;
}

//------------------------------------------------------
::rtl::OUString OCommonEmbeddedObject::GetBaseURLFrom_Impl(
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    ::rtl::OUString aBaseURL;
    sal_Int32 nInd = 0;

    for ( nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name == "DocumentBaseURL" )
        {
            lArguments[nInd].Value >>= aBaseURL;
            break;
        }

    if ( aBaseURL.isEmpty() )
    {
        for ( nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
            if ( lObjArgs[nInd].Name == "DefaultParentBaseURL" )
            {
                lObjArgs[nInd].Value >>= aBaseURL;
                break;
            }
    }

    return aBaseURL;
}


//------------------------------------------------------
void OCommonEmbeddedObject::SwitchDocToStorage_Impl( const uno::Reference< document::XStorageBasedDocument >& xDoc, const uno::Reference< embed::XStorage >& xStorage )
{
    xDoc->switchToStorage( xStorage );

    uno::Reference< util::XModifiable > xModif( xDoc, uno::UNO_QUERY );
    if ( xModif.is() )
        xModif->setModified( sal_False );

    if ( m_xRecoveryStorage.is() )
        m_xRecoveryStorage.clear();
}

//------------------------------------------------------
void OCommonEmbeddedObject::StoreDocToStorage_Impl( const uno::Reference< embed::XStorage >& xStorage,
                                                    sal_Int32 nStorageFormat,
                                                    const ::rtl::OUString& aBaseURL,
                                                    const ::rtl::OUString& aHierarchName,
                                                    sal_Bool bAttachToTheStorage )
{
    OSL_ENSURE( xStorage.is(), "No storage is provided for storing!" );

    if ( !xStorage.is() )
        throw uno::RuntimeException(); // TODO:

#ifdef USE_STORAGEBASED_DOCUMENT
    uno::Reference< document::XStorageBasedDocument > xDoc;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pDocHolder )
            xDoc = uno::Reference< document::XStorageBasedDocument >( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    }

    if ( xDoc.is() )
    {
        ::rtl::OUString aFilterName = GetFilterName( nStorageFormat );

        OSL_ENSURE( !aFilterName.isEmpty(), "Wrong document service name!" );
        if ( aFilterName.isEmpty() )
            throw io::IOException(); // TODO:

        uno::Sequence< beans::PropertyValue > aArgs( 3 );
        aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
        aArgs[0].Value <<= aFilterName;
        aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DocumentBaseURL" ));
        aArgs[2].Value <<= aBaseURL;
        aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HierarchicalDocumentName" ));
        aArgs[1].Value <<= aHierarchName;

        xDoc->storeToStorage( xStorage, aArgs );
        if ( bAttachToTheStorage )
            SwitchDocToStorage_Impl( xDoc, xStorage );
    }
    else
#endif
    {
        // store document to temporary stream based on temporary file
        uno::Reference < io::XInputStream > xTempIn = StoreDocumentToTempStream_Impl( nStorageFormat, aBaseURL, aHierarchName );

        OSL_ENSURE( xTempIn.is(), "The stream reference can not be empty!\n" );

        // open storage based on document temporary file for reading
        uno::Reference < lang::XSingleServiceFactory > xStorageFactory(
                    m_xFactory->createInstance ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.StorageFactory" )) ),
                    uno::UNO_QUERY );

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= xTempIn;
        uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArgs ),
                                                            uno::UNO_QUERY );
        if ( !xTempStorage.is() )
            throw uno::RuntimeException(); // TODO:

        // object storage must be commited automatically
        xTempStorage->copyToStorage( xStorage );
    }
}

//------------------------------------------------------
uno::Reference< util::XCloseable > OCommonEmbeddedObject::CreateDocFromMediaDescr_Impl(
                                        const uno::Sequence< beans::PropertyValue >& aMedDescr )
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xFactory, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    try
    {
        // set the document mode to embedded as the first action on the document!!!
        EmbedAndReparentDoc_Impl( xDocument );

        xLoadable->load( addAsTemplate( aMedDescr ) );
    }
    catch( const uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< util::XCloseable > OCommonEmbeddedObject::CreateTempDocFromLink_Impl()
{
    uno::Reference< util::XCloseable > xResult;

    OSL_ENSURE( m_bIsLink, "The object is not a linked one!\n" );

    uno::Sequence< beans::PropertyValue > aTempMediaDescr;

    sal_Int32 nStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
    try {
        nStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( m_xParentStorage );
    }
    catch ( const beans::IllegalTypeException& )
    {
        // the container just has an unknown type, use current file format
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Can not retrieve storage media type!\n" );
    }

    if ( m_pDocHolder->GetComponent().is() )
    {
        aTempMediaDescr.realloc( 4 );

        // TODO/LATER: may be private:stream should be used as target URL
        ::rtl::OUString aTempFileURL;
        uno::Reference< io::XInputStream > xTempStream = StoreDocumentToTempStream_Impl( SOFFICE_FILEFORMAT_CURRENT,
                                                                                         ::rtl::OUString(),
                                                                                         ::rtl::OUString() );
        try
        {
            // no need to let the file stay after the stream is removed since the embedded document
            // can not be stored directly
            uno::Reference< beans::XPropertySet > xTempStreamProps( xTempStream, uno::UNO_QUERY_THROW );
            xTempStreamProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Uri" )) ) >>= aTempFileURL;
        }
        catch( const uno::Exception& )
        {
        }

        OSL_ENSURE( !aTempFileURL.isEmpty(), "Couldn't retrieve temporary file URL!\n" );

        aTempMediaDescr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "URL" ));
        aTempMediaDescr[0].Value <<= aTempFileURL;
        aTempMediaDescr[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "InputStream" ));
        aTempMediaDescr[1].Value <<= xTempStream;
        aTempMediaDescr[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
        aTempMediaDescr[2].Value <<= GetFilterName( nStorageFormat );
        aTempMediaDescr[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "AsTemplate" ));
        aTempMediaDescr[3].Value <<= sal_True;
    }
    else
    {
        aTempMediaDescr.realloc( 2 );
        aTempMediaDescr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "URL" ));
        aTempMediaDescr[0].Value <<= m_aLinkURL;
        aTempMediaDescr[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
        aTempMediaDescr[1].Value <<= m_aLinkFilterName;
    }

    xResult = CreateDocFromMediaDescr_Impl( aTempMediaDescr );

    return xResult;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::setPersistentEntry" );

    // the type of the object must be already set
    // a kind of typedetection should be done in the factory

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // May be LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        // if the object is not loaded
        // it can not get persistant representation without initialization

        // if the object is loaded
        // it can switch persistant representation only without initialization

        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't change persistant representation of activated object!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // saveCompleted is expected, handle it accordingly
            if ( m_xNewParentStorage == xStorage && m_aNewEntryName.equals( sEntName ) )
            {
                saveCompleted( sal_True );
                return;
            }

            // if a completely different entry is provided, switch first back to the old persistence in saveCompleted
            // and then switch to the target persistence
            sal_Bool bSwitchFurther = ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) );
            saveCompleted( sal_False );
            if ( !bSwitchFurther )
                return;
        }
        else
            throw embed::WrongStateException(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!\n" );
    if ( m_bIsLink )
    {
        m_aEntryName = sEntName;
        return;
    }

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    sal_Bool bElExists = xNameAccess->hasByName( sEntName );

    m_aDocMediaDescriptor = GetValuableArgs_Impl( lArguments,
                                                  nEntryConnectionMode != embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT );

    m_bReadOnly = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name == "ReadOnly" )
            lArguments[nInd].Value >>= m_bReadOnly;

    // TODO: use lObjArgs for StoreVisualReplacement
    for ( sal_Int32 nObjInd = 0; nObjInd < lObjArgs.getLength(); nObjInd++ )
        if ( lObjArgs[nObjInd].Name == "OutplaceDispatchInterceptor" )
        {
            uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
            if ( lObjArgs[nObjInd].Value >>= xDispatchInterceptor )
                m_pDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );
        }
        else if ( lObjArgs[nObjInd].Name == "DefaultParentBaseURL" )
        {
            lObjArgs[nObjInd].Value >>= m_aDefaultParentBaseURL;
        }
        else if ( lObjArgs[nObjInd].Name == "Parent" )
        {
            lObjArgs[nObjInd].Value >>= m_xParent;
        }
        else if ( lObjArgs[nObjInd].Name == "IndividualMiscStatus" )
        {
            sal_Int64 nMiscStatus=0;
            lObjArgs[nObjInd].Value >>= nMiscStatus;
            m_nMiscStatus |= nMiscStatus;
        }
        else if ( lObjArgs[nObjInd].Name == "CloneFrom" )
        {
            uno::Reference < embed::XEmbeddedObject > xObj;
            lObjArgs[nObjInd].Value >>= xObj;
            if ( xObj.is() )
            {
                m_bHasClonedSize = sal_True;
                m_aClonedSize = xObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
                m_nClonedMapUnit = xObj->getMapUnit( embed::Aspects::MSOLE_CONTENT );
            }
        }
        else if ( lObjArgs[nObjInd].Name == "OutplaceFrameProperties" )
        {
            uno::Sequence< uno::Any > aOutFrameProps;
            uno::Sequence< beans::NamedValue > aOutFramePropsTyped;
            if ( lObjArgs[nObjInd].Value >>= aOutFrameProps )
            {
                m_pDocHolder->SetOutplaceFrameProperties( aOutFrameProps );
            }
            else if ( lObjArgs[nObjInd].Value >>= aOutFramePropsTyped )
            {
                aOutFrameProps.realloc( aOutFramePropsTyped.getLength() );
                uno::Any* pProp = aOutFrameProps.getArray();
                for (   const beans::NamedValue* pTypedProp = aOutFramePropsTyped.getConstArray();
                        pTypedProp != aOutFramePropsTyped.getConstArray() + aOutFramePropsTyped.getLength();
                        ++pTypedProp, ++pProp
                    )
                {
                    *pProp <<= *pTypedProp;
                }
                m_pDocHolder->SetOutplaceFrameProperties( aOutFrameProps );
            }
            else
                OSL_FAIL( "OCommonEmbeddedObject::setPersistentEntry: illegal type for argument 'OutplaceFrameProperties'!" );
        }
        else if ( lObjArgs[nObjInd].Name == "ModuleName" )
        {
            lObjArgs[nObjInd].Value >>= m_aModuleName;
        }
        else if ( lObjArgs[nObjInd].Name == "EmbeddedScriptSupport" )
        {
            OSL_VERIFY( lObjArgs[nObjInd].Value >>= m_bEmbeddedScriptSupport );
        }
        else if ( lObjArgs[nObjInd].Name == "DocumentRecoverySupport" )
        {
            OSL_VERIFY( lObjArgs[nObjInd].Value >>= m_bDocumentRecoverySupport );
        }
        else if ( lObjArgs[nObjInd].Name == "RecoveryStorage" )
        {
            OSL_VERIFY( lObjArgs[nObjInd].Value >>= m_xRecoveryStorage );
        }


    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    SwitchOwnPersistence( xStorage, sEntName );

    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT )
    {
        if ( bElExists )
        {
            // the initialization from existing storage allows to leave object in loaded state
            m_nObjectState = embed::EmbedStates::LOADED;
        }
        else
        {
            m_pDocHolder->SetComponent( InitNewDocument_Impl(), m_bReadOnly );
            if ( !m_pDocHolder->GetComponent().is() )
                throw io::IOException(); // TODO: can not create document

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
    }
    else
    {
        if ( ( nStorageMode & embed::ElementModes::READWRITE ) != embed::ElementModes::READWRITE )
            throw io::IOException();

        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // the document just already changed its storage to store to
            // the links to OOo documents for now ignore this call
            // TODO: OOo links will have persistence so it will be switched here
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::TRUNCATE_INIT )
        {
            if ( m_xRecoveryStorage.is() )
                TransferMediaType( m_xRecoveryStorage, m_xObjectStorage );

            // TODO:
            m_pDocHolder->SetComponent( InitNewDocument_Impl(), m_bReadOnly );

            if ( !m_pDocHolder->GetComponent().is() )
                throw io::IOException(); // TODO: can not create document

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
        {
            m_pDocHolder->SetComponent( CreateDocFromMediaDescr_Impl( lArguments ), m_bReadOnly );
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Wrong connection mode is provided!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );
    }
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::storeToEntry" );

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't store object without persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!\n" );
    if ( m_bIsLink )
        return;

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

    sal_Int32 nTargetStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
    sal_Int32 nOriginalStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
    try {
        nTargetStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( xStorage );
    }
    catch ( const beans::IllegalTypeException& )
    {
        // the container just has an unknown type, use current file format
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Can not retrieve target storage media type!\n" );
    }

    try
    {
        nOriginalStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( m_xParentStorage );
    }
    catch ( const beans::IllegalTypeException& )
    {
        // the container just has an unknown type, use current file format
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Can not retrieve own storage media type!\n" );
    }

    sal_Bool bTryOptimization = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        // StoreVisualReplacement and VisualReplacement args have no sence here
        if ( lObjArgs[nInd].Name == "CanTryOptimization" )
            lObjArgs[nInd].Value >>= bTryOptimization;
    }

    sal_Bool bSwitchBackToLoaded = sal_False;

    // Storing to different format can be done only in running state.
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // TODO/LATER: copying is not legal for documents with relative links.
        if ( nTargetStorageFormat == nOriginalStorageFormat )
        {
            sal_Bool bOptimizationWorks = sal_False;
            if ( bTryOptimization )
            {
                try
                {
                    // try to use optimized copying
                    uno::Reference< embed::XOptimizedStorage > xSource( m_xParentStorage, uno::UNO_QUERY_THROW );
                    uno::Reference< embed::XOptimizedStorage > xTarget( xStorage, uno::UNO_QUERY_THROW );
                    xSource->copyElementDirectlyTo( m_aEntryName, xTarget, sEntName );
                    bOptimizationWorks = sal_True;
                }
                catch( const uno::Exception& )
                {
                }
            }

            if ( !bOptimizationWorks )
                m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
        }
        else
        {
            changeState( embed::EmbedStates::RUNNING );
            bSwitchBackToLoaded = sal_True;
        }
    }

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        uno::Reference< embed::XStorage > xSubStorage =
                    xStorage->openStorageElement( sEntName, embed::ElementModes::READWRITE );

        if ( !xSubStorage.is() )
            throw uno::RuntimeException(); //TODO

        aGuard.clear();
        // TODO/LATER: support hierarchical name for embedded objects in embedded objects
        StoreDocToStorage_Impl( xSubStorage, nTargetStorageFormat, GetBaseURLFrom_Impl( lArguments, lObjArgs ), sEntName, sal_False );
        aGuard.reset();

        if ( bSwitchBackToLoaded )
            changeState( embed::EmbedStates::LOADED );
    }

    // TODO: should the listener notification be done?
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::storeAsEntry" );

    // TODO: use lObjArgs

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't store object without persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!\n" );
    if ( m_bIsLink )
    {
        m_aNewEntryName = sEntName;
        return;
    }

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

    sal_Int32 nTargetStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
    sal_Int32 nOriginalStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
    try {
        nTargetStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( xStorage );
    }
    catch ( const beans::IllegalTypeException& )
    {
        // the container just has an unknown type, use current file format
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Can not retrieve target storage media type!\n" );
    }

    try
    {
        nOriginalStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( m_xParentStorage );
    }
    catch ( const beans::IllegalTypeException& )
    {
        // the container just has an unknown type, use current file format
    }
    catch ( const uno::Exception& )
    {
        OSL_FAIL( "Can not retrieve own storage media type!\n" );
    }

    PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSaveAs" )) );

    sal_Bool bTryOptimization = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        // StoreVisualReplacement and VisualReplacement args have no sence here
        if ( lObjArgs[nInd].Name == "CanTryOptimization" )
            lObjArgs[nInd].Value >>= bTryOptimization;
    }

    sal_Bool bSwitchBackToLoaded = sal_False;

    // Storing to different format can be done only in running state.
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // TODO/LATER: copying is not legal for documents with relative links.
        if ( nTargetStorageFormat == nOriginalStorageFormat )
        {
            sal_Bool bOptimizationWorks = sal_False;
            if ( bTryOptimization )
            {
                try
                {
                    // try to use optimized copying
                    uno::Reference< embed::XOptimizedStorage > xSource( m_xParentStorage, uno::UNO_QUERY_THROW );
                    uno::Reference< embed::XOptimizedStorage > xTarget( xStorage, uno::UNO_QUERY_THROW );
                    xSource->copyElementDirectlyTo( m_aEntryName, xTarget, sEntName );
                    bOptimizationWorks = sal_True;
                }
                catch( const uno::Exception& )
                {
                }
            }

            if ( !bOptimizationWorks )
                m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
        }
        else
        {
            changeState( embed::EmbedStates::RUNNING );
            bSwitchBackToLoaded = sal_True;
        }
    }

    uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::READWRITE );

    if ( !xSubStorage.is() )
        throw uno::RuntimeException(); //TODO

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        aGuard.clear();
        // TODO/LATER: support hierarchical name for embedded objects in embedded objects
        StoreDocToStorage_Impl( xSubStorage, nTargetStorageFormat, GetBaseURLFrom_Impl( lArguments, lObjArgs ), sEntName, sal_False );
        aGuard.reset();

        if ( bSwitchBackToLoaded )
            changeState( embed::EmbedStates::LOADED );
    }

    m_bWaitSaveCompleted = sal_True;
    m_xNewObjectStorage = xSubStorage;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
    m_aNewDocMediaDescriptor = GetValuableArgs_Impl( lArguments, sal_True );

    // TODO: register listeners for storages above, in case thay are disposed
    //       an exception will be thrown on saveCompleted( true )

    // TODO: should the listener notification be done here or in saveCompleted?
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::saveCompleted( sal_Bool bUseNew )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::saveCompleted" );

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't store object without persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!\n" );
    if ( m_bIsLink )
    {
        if ( bUseNew )
            m_aEntryName = m_aNewEntryName;
        m_aNewEntryName = ::rtl::OUString();
        return;
    }

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    OSL_ENSURE( m_bWaitSaveCompleted, "Unexpected saveCompleted() call!\n" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewObjectStorage.is() && m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewObjectStorage.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        SwitchOwnPersistence( m_xNewParentStorage, m_xNewObjectStorage, m_aNewEntryName );
        m_aDocMediaDescriptor = m_aNewDocMediaDescriptor;

        uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xModif.is() )
            xModif->setModified( sal_False );

        PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSaveAsDone" ) ));
    }
    else
    {
        try {
            uno::Reference< lang::XComponent > xComponent( m_xNewObjectStorage, uno::UNO_QUERY );
            OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
    }

    m_xNewObjectStorage = uno::Reference< embed::XStorage >();
    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = ::rtl::OUString();
    m_aNewDocMediaDescriptor.realloc( 0 );
    m_bWaitSaveCompleted = sal_False;

    if ( bUseNew )
    {
        // TODO: notify listeners

        if ( m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE )
        {
            // TODO: update visual representation
        }
    }
}

//------------------------------------------------------
sal_Bool SAL_CALL OCommonEmbeddedObject::hasEntry()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_xObjectStorage.is() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OCommonEmbeddedObject::getEntryName()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object persistence is not initialized!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aEntryName;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeOwn()
        throw ( embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::storeOwn" );

    // during switching from Activated to Running and from Running to Loaded states the object will
    // ask container to store the object, the container has to make decision
    // to do so or not

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't store object without persistence!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: access denied

    // nothing to do, if the object is in loaded state
    if ( m_nObjectState == embed::EmbedStates::LOADED )
        return;

    PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSave" )) );

    OSL_ENSURE( m_pDocHolder->GetComponent().is(), "If an object is activated or in running state it must have a document!\n" );
    if ( !m_pDocHolder->GetComponent().is() )
        throw uno::RuntimeException();

    if ( m_bIsLink )
    {
        // TODO: just store the document to it's location
        uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( !xStorable.is() )
            throw uno::RuntimeException(); // TODO

        // free the main mutex for the storing time
        aGuard.clear();

        xStorable->store();

        aGuard.reset();
    }
    else
    {
        OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

        if ( !m_xObjectStorage.is() )
            throw io::IOException(); //TODO: access denied

        sal_Int32 nStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
        try {
            nStorageFormat = ::comphelper::OStorageHelper::GetXStorageFormat( m_xParentStorage );
        }
        catch ( const beans::IllegalTypeException& )
        {
            // the container just has an unknown type, use current file format
        }
        catch ( const uno::Exception& )
        {
            OSL_FAIL( "Can not retrieve storage media type!\n" );
        }

        aGuard.clear();
        StoreDocToStorage_Impl( m_xObjectStorage, nStorageFormat, GetBaseURL_Impl(), m_aEntryName, sal_True );
        aGuard.reset();
    }

    uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    if ( xModif.is() )
        xModif->setModified( sal_False );

    PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSaveDone" )) );
}

//------------------------------------------------------
sal_Bool SAL_CALL OCommonEmbeddedObject::isReadonly()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object persistence is not initialized!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_bReadOnly;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments,
                const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs
    // for now this method is used only to switch readonly state

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object persistence is not initialized!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        // the object is still not loaded
        throw embed::WrongStateException(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object must be in loaded state to be reloaded!\n" )),
                                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_bIsLink )
    {
        // reload of the link
        ::rtl::OUString aOldLinkFilter = m_aLinkFilterName;

        ::rtl::OUString aNewLinkFilter;
        for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        {
            if ( lArguments[nInd].Name == "URL" )
            {
                // the new URL
                lArguments[nInd].Value >>= m_aLinkURL;
                m_aLinkFilterName = ::rtl::OUString();
            }
            else if ( lArguments[nInd].Name == "FilterName" )
            {
                lArguments[nInd].Value >>= aNewLinkFilter;
                m_aLinkFilterName = ::rtl::OUString();
            }
        }

        ::comphelper::MimeConfigurationHelper aHelper( m_xFactory );
        if ( m_aLinkFilterName.isEmpty() )
        {
            if ( !aNewLinkFilter.isEmpty() )
                m_aLinkFilterName = aNewLinkFilter;
            else
            {
                uno::Sequence< beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                aArgs[0].Value <<= m_aLinkURL;
                m_aLinkFilterName = aHelper.UpdateMediaDescriptorWithFilterName( aArgs, sal_False );
            }
        }

        if ( !aOldLinkFilter.equals( m_aLinkFilterName ) )
        {
            uno::Sequence< beans::NamedValue > aObject = aHelper.GetObjectPropsByFilter( m_aLinkFilterName );

            // TODO/LATER: probably the document holder could be cleaned explicitly as in the destructor
            m_pDocHolder->release();
            m_pDocHolder = NULL;

            LinkInit_Impl( aObject, lArguments, lObjArgs );
        }
    }

    m_aDocMediaDescriptor = GetValuableArgs_Impl( lArguments, sal_True );

    // TODO: use lObjArgs for StoreVisualReplacement
    for ( sal_Int32 nObjInd = 0; nObjInd < lObjArgs.getLength(); nObjInd++ )
        if ( lObjArgs[nObjInd].Name == "OutplaceDispatchInterceptor" )
        {
            uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
            if ( lObjArgs[nObjInd].Value >>= xDispatchInterceptor )
                m_pDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );

            break;
        }

    // TODO:
    // when document allows reloading through API the object can be reloaded not only in loaded state

    sal_Bool bOldReadOnlyValue = m_bReadOnly;

    m_bReadOnly = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name == "ReadOnly" )
            lArguments[nInd].Value >>= m_bReadOnly;

    if ( bOldReadOnlyValue != m_bReadOnly && !m_bIsLink )
    {
        // close own storage
        try {
            uno::Reference< lang::XComponent > xComponent( m_xObjectStorage, uno::UNO_QUERY );
            OSL_ENSURE( !m_xObjectStorage.is() || xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }

        sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;
        m_xObjectStorage = m_xParentStorage->openStorageElement( m_aEntryName, nStorageMode );
    }
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::breakLink( const uno::Reference< embed::XStorage >& xStorage,
                                                const ::rtl::OUString& sEntName )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_bIsLink )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is not a valid linked object!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }
    else
    {
        // the current implementation of OOo links does not implement this method since it does not implement
        // all the set of interfaces required for OOo embedded object ( XEmbedPersist is not supported ).
    }

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    if ( !m_bIsLink || m_nObjectState == -1 )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is not a valid linked object!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    /*sal_Bool bElExists =*/ xNameAccess->hasByName( sEntName );

    m_bReadOnly = sal_False;

    if ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) )
        SwitchOwnPersistence( xStorage, sEntName );

    // for linked object it means that it becomes embedded object
    // the document must switch it's persistence also

    // TODO/LATER: handle the case when temp doc can not be created
    // the document is a new embedded object so it must be marked as modified
    uno::Reference< util::XCloseable > xDocument = CreateTempDocFromLink_Impl();
    uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    if ( !xModif.is() )
        throw uno::RuntimeException();
    try
    {
        xModif->setModified( sal_True );
    }
    catch( const uno::Exception& )
    {}

    m_pDocHolder->SetComponent( xDocument, m_bReadOnly );
    OSL_ENSURE( m_pDocHolder->GetComponent().is(), "If document cant be created, an exception must be thrown!\n" );

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // the state is changed and can not be switched to loaded state back without saving
        m_nObjectState = embed::EmbedStates::RUNNING;
        StateChangeNotification_Impl( sal_False, embed::EmbedStates::LOADED, m_nObjectState, aGuard );
    }
    else if ( m_nObjectState == embed::EmbedStates::ACTIVE )
        m_pDocHolder->Show();

    m_bIsLink = sal_False;
    m_aLinkFilterName = ::rtl::OUString();
    m_aLinkURL = ::rtl::OUString();
}

//------------------------------------------------------
sal_Bool SAL_CALL  OCommonEmbeddedObject::isLink()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_bIsLink;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OCommonEmbeddedObject::getLinkURL()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_bIsLink )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is not a link object!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aLinkURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
