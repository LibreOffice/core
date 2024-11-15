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
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
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
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/securityoptions.hxx>

#include <tools/diagnose_ex.h>
#include "persistence.hxx"

using namespace ::com::sun::star;


uno::Sequence< beans::PropertyValue > GetValuableArgs_Impl( const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                            bool bCanUseDocumentBaseURL )
{
    uno::Sequence< beans::PropertyValue > aResult;
    sal_Int32 nResLen = 0;

    for ( sal_Int32 nInd = 0; nInd < aMedDescr.getLength(); nInd++ )
    {
        if ( aMedDescr[nInd].Name == "ComponentData" || aMedDescr[nInd].Name == "DocumentTitle"
          || aMedDescr[nInd].Name == "InteractionHandler" || aMedDescr[nInd].Name == "JumpMark"
          // || aMedDescr[nInd].Name == "Password" // makes no sense for embedded objects
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


uno::Sequence< beans::PropertyValue > addAsTemplate( const uno::Sequence< beans::PropertyValue >& aOrig )
{
    bool bAsTemplateSet = false;
    sal_Int32 nLength = aOrig.getLength();
    uno::Sequence< beans::PropertyValue > aResult( nLength );

    for ( sal_Int32 nInd = 0; nInd < nLength; nInd++ )
    {
        aResult[nInd].Name = aOrig[nInd].Name;
        if ( aResult[nInd].Name == "AsTemplate" )
        {
            aResult[nInd].Value <<= true;
            bAsTemplateSet = true;
        }
        else
            aResult[nInd].Value = aOrig[nInd].Value;
    }

    if ( !bAsTemplateSet )
    {
        aResult.realloc( nLength + 1 );
        aResult[nLength].Name = "AsTemplate";
        aResult[nLength].Value <<= true;
    }

    return aResult;
}


uno::Reference< io::XInputStream > createTempInpStreamFromStor(
                                                            const uno::Reference< embed::XStorage >& xStorage,
                                                            const uno::Reference< uno::XComponentContext >& xContext )
{
    SAL_WARN_IF( !xStorage.is(), "embeddedobj.common", "The storage can not be empty!" );

    uno::Reference< io::XInputStream > xResult;

    uno::Reference < io::XStream > xTempStream( io::TempFile::create(xContext), uno::UNO_QUERY_THROW );

    uno::Reference < lang::XSingleServiceFactory > xStorageFactory( embed::StorageFactory::create(xContext) );

    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xTempStream;
    aArgs[1] <<= embed::ElementModes::READWRITE;
    uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );

    try
    {
        xStorage->copyToStorage( xTempStorage );
    } catch( const uno::Exception& e )
    {
        throw embed::StorageWrappedTargetException(
                    "Can't copy storage!",
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
    }

    try {
        uno::Reference< lang::XComponent > xComponent( xTempStorage, uno::UNO_QUERY );
        SAL_WARN_IF( !xComponent.is(), "embeddedobj.common", "Wrong storage implementation!" );
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


static void TransferMediaType( const uno::Reference< embed::XStorage >& i_rSource, const uno::Reference< embed::XStorage >& i_rTarget )
{
    try
    {
        const uno::Reference< beans::XPropertySet > xSourceProps( i_rSource, uno::UNO_QUERY_THROW );
        const uno::Reference< beans::XPropertySet > xTargetProps( i_rTarget, uno::UNO_QUERY_THROW );
        const OUString sMediaTypePropName( "MediaType" );
        xTargetProps->setPropertyValue( sMediaTypePropName, xSourceProps->getPropertyValue( sMediaTypePropName ) );
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}


static uno::Reference< util::XCloseable > CreateDocument( const uno::Reference< uno::XComponentContext >& _rxContext,
    const OUString& _rDocumentServiceName, bool _bEmbeddedScriptSupport, const bool i_bDocumentRecoverySupport )
{
    ::comphelper::NamedValueCollection aArguments;
    aArguments.put( "EmbeddedObject", true );
    aArguments.put( "EmbeddedScriptSupport", _bEmbeddedScriptSupport );
    aArguments.put( "DocumentRecoverySupport", i_bDocumentRecoverySupport );

    uno::Reference< uno::XInterface > xDocument;
    try
    {
        xDocument = _rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        _rDocumentServiceName, aArguments.getWrappedPropertyValues(), _rxContext );
    }
    catch( const uno::Exception& )
    {
        // if an embedded object implementation does not support XInitialization,
        // the default factory from cppuhelper will throw an
        // IllegalArgumentException when we try to create the instance with arguments.
        // Okay, so we fall back to creating the instance without any arguments.
        OSL_FAIL("Consider implementing interface XInitialization to avoid duplicate construction");
        xDocument = _rxContext->getServiceManager()->createInstanceWithContext( _rDocumentServiceName, _rxContext );
    }

    SAL_WARN_IF(!xDocument.is(), "embeddedobj.common", "Service " << _rDocumentServiceName << " is not available?");
    return uno::Reference< util::XCloseable >( xDocument, uno::UNO_QUERY );
}


static void SetDocToEmbedded( const uno::Reference< frame::XModel >& rDocument, const OUString& aModuleName )
{
    if (rDocument.is())
    {
        uno::Sequence< beans::PropertyValue > aSeq( 1 );
        aSeq[0].Name = "SetEmbedded";
        aSeq[0].Value <<= true;
        rDocument->attachResource( OUString(), aSeq );

        if ( !aModuleName.isEmpty() )
        {
            try
            {
                uno::Reference< frame::XModule > xModule( rDocument, uno::UNO_QUERY_THROW );
                xModule->setIdentifier( aModuleName );
            }
            catch( const uno::Exception& )
            {}
        }
    }
}


void OCommonEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                                  const uno::Reference< embed::XStorage >& xNewObjectStorage,
                                                  const OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
    {
        SAL_WARN_IF( xNewObjectStorage != m_xObjectStorage, "embeddedobj.common", "The storage must be the same!" );
        return;
    }

    uno::Reference< lang::XComponent > xComponent( m_xObjectStorage, uno::UNO_QUERY );
    OSL_ENSURE( !m_xObjectStorage.is() || xComponent.is(), "Wrong storage implementation!" );

    m_xObjectStorage = xNewObjectStorage;
    m_xParentStorage = xNewParentStorage;
    m_aEntryName = aNewName;

    // the linked document should not be switched
    if ( !m_bIsLink )
    {
        uno::Reference< document::XStorageBasedDocument > xDoc( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xDoc.is() )
            SwitchDocToStorage_Impl( xDoc, m_xObjectStorage );
    }

    try {
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( const uno::Exception& )
    {
    }
}


void OCommonEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                                  const OUString& aNewName )
{
    if ( xNewParentStorage == m_xParentStorage && aNewName.equals( m_aEntryName ) )
        return;

    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    uno::Reference< embed::XStorage > xNewOwnStorage = xNewParentStorage->openStorageElement( aNewName, nStorageMode );
    SAL_WARN_IF( !xNewOwnStorage.is(), "embeddedobj.common", "The method can not return empty reference!" );

    SwitchOwnPersistence( xNewParentStorage, xNewOwnStorage, aNewName );
}


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
        SAL_WARN( "embeddedobj.common", "OCommonEmbeddedObject::EmbedAndReparentDoc: cannot set parent at document!" );
    }
}


uno::Reference< util::XCloseable > OCommonEmbeddedObject::InitNewDocument_Impl()
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xContext, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XModel > xModel( xDocument, uno::UNO_QUERY );
    uno::Reference< frame::XLoadable > xLoadable( xModel, uno::UNO_QUERY_THROW );

    try
    {
        // set the document mode to embedded as the first action on document!!!
        EmbedAndReparentDoc_Impl( xDocument );

        // if we have a storage to recover the document from, do not use initNew, but instead load from that storage
        bool bInitNew = true;
        if ( m_xRecoveryStorage.is() )
        {
            uno::Reference< document::XStorageBasedDocument > xDoc( xLoadable, uno::UNO_QUERY );
            SAL_WARN_IF( !xDoc.is(), "embeddedobj.common", "OCommonEmbeddedObject::InitNewDocument_Impl: cannot recover from a storage when the document is not storage based!" );
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
                xCloseable->close( true );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}


uno::Reference< util::XCloseable > OCommonEmbeddedObject::LoadLink_Impl()
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xContext, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY_THROW );

    sal_Int32 nLen = 2;
    uno::Sequence< beans::PropertyValue > aArgs( nLen );

    if (INetURLObject(m_aLinkURL).IsExoticProtocol())
    {
        SAL_WARN("embeddedobj.common", "Ignore exotic protocol: " << m_aLinkURL);
        return nullptr;
    }

    aArgs[0].Name = "URL";
    aArgs[0].Value <<= m_aLinkURL;
    aArgs[1].Name = "FilterName";
    aArgs[1].Value <<= m_aLinkFilterName;
    if ( m_bLinkHasPassword )
    {
        aArgs.realloc( ++nLen );
        aArgs[nLen-1].Name = "Password";
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
                    m_bLinkHasPassword = true;
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
                xCloseable->close( true );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;

}


OUString OCommonEmbeddedObject::GetFilterName( sal_Int32 nVersion ) const
{
    OUString aFilterName = GetPresetFilterName();
    if ( aFilterName.isEmpty() )
    {
        try {
            ::comphelper::MimeConfigurationHelper aHelper( m_xContext );
            aFilterName = aHelper.GetDefaultFilterFromServiceName( GetDocumentServiceName(), nVersion );

            // If no filter is found, fall back to the FileFormatVersion=6200 filter, Base only has that.
            if (aFilterName.isEmpty() && nVersion == SOFFICE_FILEFORMAT_CURRENT)
                aFilterName = aHelper.GetDefaultFilterFromServiceName(GetDocumentServiceName(), SOFFICE_FILEFORMAT_60);
        } catch( const uno::Exception& )
        {}
    }

    return aFilterName;
}


void OCommonEmbeddedObject::FillDefaultLoadArgs_Impl( const uno::Reference< embed::XStorage >& i_rxStorage,
        ::comphelper::NamedValueCollection& o_rLoadArgs ) const
{
    o_rLoadArgs.put( "DocumentBaseURL", GetBaseURL_Impl() );
    o_rLoadArgs.put( "HierarchicalDocumentName", m_aEntryName );
    o_rLoadArgs.put( "ReadOnly", m_bReadOnly );

    OUString aFilterName = GetFilterName( ::comphelper::OStorageHelper::GetXStorageFormat( i_rxStorage ) );
    SAL_WARN_IF( aFilterName.isEmpty(), "embeddedobj.common", "OCommonEmbeddedObject::FillDefaultLoadArgs_Impl: Wrong document service name!" );
    if ( aFilterName.isEmpty() )
        throw io::IOException();    // TODO: error message/code

    o_rLoadArgs.put( "FilterName", aFilterName );
}


uno::Reference< util::XCloseable > OCommonEmbeddedObject::LoadDocumentFromStorage_Impl()
{
    ENSURE_OR_THROW( m_xObjectStorage.is(), "no object storage" );

    const uno::Reference< embed::XStorage > xSourceStorage( m_xRecoveryStorage.is() ? m_xRecoveryStorage : m_xObjectStorage );

    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xContext, GetDocumentServiceName(),
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
    uno::Reference< document::XStorageBasedDocument > xDoc( xDocument, uno::UNO_QUERY );
    if ( !xDoc.is() && !xLoadable.is() )
        throw uno::RuntimeException();

    ::comphelper::NamedValueCollection aLoadArgs;
    FillDefaultLoadArgs_Impl( xSourceStorage, aLoadArgs );

    uno::Reference< io::XInputStream > xTempInpStream;
    if ( !xDoc.is() )
    {
        xTempInpStream = createTempInpStreamFromStor( xSourceStorage, m_xContext );
        if ( !xTempInpStream.is() )
            throw uno::RuntimeException();

        OUString aTempFileURL;
        try
        {
            // no need to let the file stay after the stream is removed since the embedded document
            // can not be stored directly
            uno::Reference< beans::XPropertySet > xTempStreamProps( xTempInpStream, uno::UNO_QUERY_THROW );
            xTempStreamProps->getPropertyValue("Uri") >>= aTempFileURL;
        }
        catch( const uno::Exception& )
        {
        }

        SAL_WARN_IF( aTempFileURL.isEmpty(), "embeddedobj.common", "Couldn't retrieve temporary file URL!" );

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
                xCloseable->close( true );
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


uno::Reference< io::XInputStream > OCommonEmbeddedObject::StoreDocumentToTempStream_Impl(
                                                                            sal_Int32 nStorageFormat,
                                                                            const OUString& aBaseURL,
                                                                            const OUString& aHierarchName )
{
    uno::Reference < io::XOutputStream > xTempOut(
                io::TempFile::create(m_xContext),
                uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > aResult( xTempOut, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XStorable > xStorable;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_xDocHolder.is() )
            xStorable.set( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
    }

    if( !xStorable.is() )
        throw uno::RuntimeException(); // TODO:

    OUString aFilterName = GetFilterName( nStorageFormat );

    SAL_WARN_IF( aFilterName.isEmpty(), "embeddedobj.common", "Wrong document service name!" );
    if ( aFilterName.isEmpty() )
        throw io::IOException(); // TODO:

    uno::Sequence< beans::PropertyValue > aArgs( 4 );
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= aFilterName;
    aArgs[1].Name = "OutputStream";
    aArgs[1].Value <<= xTempOut;
    aArgs[2].Name = "DocumentBaseURL";
    aArgs[2].Value <<= aBaseURL;
    aArgs[3].Name = "HierarchicalDocumentName";
    aArgs[3].Value <<= aHierarchName;

    xStorable->storeToURL( "private:stream", aArgs );
    try
    {
        xTempOut->closeOutput();
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "embeddedobj.common", "Looks like stream was closed already" );
    }

    return aResult;
}


void OCommonEmbeddedObject::SaveObject_Impl()
{
    if ( m_xClientSite.is() )
    {
        try
        {
            // check whether the component is modified,
            // if not there is no need for storing
            uno::Reference< util::XModifiable > xModifiable( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
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
            SAL_WARN( "embeddedobj.common", "The object was not stored!" );
        }
    }
}


OUString OCommonEmbeddedObject::GetBaseURL_Impl() const
{
    OUString aBaseURL;
    sal_Int32 nInd = 0;

    if ( m_xClientSite.is() )
    {
        try
        {
            uno::Reference< frame::XModel > xParentModel( m_xClientSite->getComponent(), uno::UNO_QUERY_THROW );
            uno::Sequence< beans::PropertyValue > aModelProps = xParentModel->getArgs();
            for ( nInd = 0; nInd < aModelProps.getLength(); nInd++ )
                if ( aModelProps[nInd].Name == "DocumentBaseURL" )
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
            if ( m_aDocMediaDescriptor[nInd].Name == "DocumentBaseURL" )
            {
                m_aDocMediaDescriptor[nInd].Value >>= aBaseURL;
                break;
            }
    }

    if ( aBaseURL.isEmpty() )
        aBaseURL = m_aDefaultParentBaseURL;

    return aBaseURL;
}


OUString OCommonEmbeddedObject::GetBaseURLFrom_Impl(
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    OUString aBaseURL;
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


void OCommonEmbeddedObject::SwitchDocToStorage_Impl( const uno::Reference< document::XStorageBasedDocument >& xDoc, const uno::Reference< embed::XStorage >& xStorage )
{
    xDoc->switchToStorage( xStorage );

    uno::Reference< util::XModifiable > xModif( xDoc, uno::UNO_QUERY );
    if ( xModif.is() )
        xModif->setModified( false );

    if ( m_xRecoveryStorage.is() )
        m_xRecoveryStorage.clear();
}

namespace {

OUString getStringPropertyValue( const uno::Sequence<beans::PropertyValue>& rProps, const OUString& rName )
{
    OUString aStr;

    for (sal_Int32 i = 0; i < rProps.getLength(); ++i)
    {
        if (rProps[i].Name == rName)
        {
            rProps[i].Value >>= aStr;
            break;
        }
    }

    return aStr;
}

}

void OCommonEmbeddedObject::StoreDocToStorage_Impl(
    const uno::Reference<embed::XStorage>& xStorage,
    const uno::Sequence<beans::PropertyValue>& rMediaArgs,
    const uno::Sequence<beans::PropertyValue>& rObjArgs,
    sal_Int32 nStorageFormat,
    const OUString& aHierarchName,
    bool bAttachToTheStorage )
{
    SAL_WARN_IF( !xStorage.is(), "embeddedobj.common", "No storage is provided for storing!" );

    if ( !xStorage.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< document::XStorageBasedDocument > xDoc;
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_xDocHolder.is() )
            xDoc.set( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
    }

    OUString aBaseURL = GetBaseURLFrom_Impl(rMediaArgs, rObjArgs);

    if ( xDoc.is() )
    {
        OUString aFilterName = GetFilterName( nStorageFormat );

        // No filter found? Try the older format, e.g. Base has only that.
        if (aFilterName.isEmpty() && nStorageFormat == SOFFICE_FILEFORMAT_CURRENT)
            aFilterName = GetFilterName( SOFFICE_FILEFORMAT_60 );

        SAL_WARN_IF( aFilterName.isEmpty(), "embeddedobj.common", "Wrong document service name!" );
        if ( aFilterName.isEmpty() )
            throw io::IOException(); // TODO:

        uno::Sequence<beans::PropertyValue> aArgs(5);
        aArgs[0].Name = "FilterName";
        aArgs[0].Value <<= aFilterName;
        aArgs[1].Name = "HierarchicalDocumentName";
        aArgs[1].Value <<= aHierarchName;
        aArgs[2].Name = "DocumentBaseURL";
        aArgs[2].Value <<= aBaseURL;
        aArgs[3].Name = "SourceShellID";
        aArgs[3].Value <<= getStringPropertyValue(rObjArgs, "SourceShellID");
        aArgs[4].Name = "DestinationShellID";
        aArgs[4].Value <<= getStringPropertyValue(rObjArgs, "DestinationShellID");

        xDoc->storeToStorage( xStorage, aArgs );
        if ( bAttachToTheStorage )
            SwitchDocToStorage_Impl( xDoc, xStorage );
    }
    else
    {
        // store document to temporary stream based on temporary file
        uno::Reference < io::XInputStream > xTempIn = StoreDocumentToTempStream_Impl( nStorageFormat, aBaseURL, aHierarchName );

        SAL_WARN_IF( !xTempIn.is(), "embeddedobj.common", "The stream reference can not be empty!" );

        // open storage based on document temporary file for reading
        uno::Reference < lang::XSingleServiceFactory > xStorageFactory = embed::StorageFactory::create(m_xContext);

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= xTempIn;
        uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArgs ),
                                                            uno::UNO_QUERY_THROW );

        // object storage must be committed automatically
        xTempStorage->copyToStorage( xStorage );
    }
}


uno::Reference< util::XCloseable > OCommonEmbeddedObject::CreateDocFromMediaDescr_Impl(
                                        const uno::Sequence< beans::PropertyValue >& aMedDescr )
{
    uno::Reference< util::XCloseable > xDocument( CreateDocument( m_xContext, GetDocumentServiceName(),
                                                m_bEmbeddedScriptSupport, m_bDocumentRecoverySupport ) );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY_THROW );

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
                xCloseable->close( true );
            }
            catch( const uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}


uno::Reference< util::XCloseable > OCommonEmbeddedObject::CreateTempDocFromLink_Impl()
{
    uno::Reference< util::XCloseable > xResult;

    SAL_WARN_IF( !m_bIsLink, "embeddedobj.common", "The object is not a linked one!" );

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
        SAL_WARN( "embeddedobj.common", "Can not retrieve storage media type!" );
    }

    if ( m_xDocHolder->GetComponent().is() )
    {
        aTempMediaDescr.realloc( 4 );

        // TODO/LATER: may be private:stream should be used as target URL
        OUString aTempFileURL;
        uno::Reference< io::XInputStream > xTempStream = StoreDocumentToTempStream_Impl( SOFFICE_FILEFORMAT_CURRENT,
                                                                                         OUString(),
                                                                                         OUString() );
        try
        {
            // no need to let the file stay after the stream is removed since the embedded document
            // can not be stored directly
            uno::Reference< beans::XPropertySet > xTempStreamProps( xTempStream, uno::UNO_QUERY_THROW );
            xTempStreamProps->getPropertyValue("Uri") >>= aTempFileURL;
        }
        catch( const uno::Exception& )
        {
        }

        SAL_WARN_IF( aTempFileURL.isEmpty(), "embeddedobj.common", "Couldn't retrieve temporary file URL!" );

        aTempMediaDescr[0].Name = "URL";
        aTempMediaDescr[0].Value <<= aTempFileURL;
        aTempMediaDescr[1].Name = "InputStream";
        aTempMediaDescr[1].Value <<= xTempStream;
        aTempMediaDescr[2].Name = "FilterName";
        aTempMediaDescr[2].Value <<= GetFilterName( nStorageFormat );
        aTempMediaDescr[3].Name = "AsTemplate";
        aTempMediaDescr[3].Value <<= true;
    }
    else
    {
        aTempMediaDescr.realloc( 2 );
        aTempMediaDescr[0].Name = "URL";
        aTempMediaDescr[0].Value <<= m_aLinkURL;
        aTempMediaDescr[1].Name = "FilterName";
        aTempMediaDescr[1].Value <<= m_aLinkFilterName;
    }

    xResult = CreateDocFromMediaDescr_Impl( aTempMediaDescr );

    return xResult;
}


void SAL_CALL OCommonEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& lArguments,
                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    // the type of the object must be already set
    // a kind of typedetection should be done in the factory

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    // May be LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        // if the object is not loaded
        // it can not get persistent representation without initialization

        // if the object is loaded
        // it can switch persistent representation only without initialization

        throw embed::WrongStateException(
                    "Can't change persistent representation of activated object!",
                     static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // saveCompleted is expected, handle it accordingly
            if ( m_xNewParentStorage == xStorage && m_aNewEntryName.equals( sEntName ) )
            {
                saveCompleted( true );
                return;
            }

            // if a completely different entry is provided, switch first back to the old persistence in saveCompleted
            // and then switch to the target persistence
            bool bSwitchFurther = ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) );
            saveCompleted( false );
            if ( !bSwitchFurther )
                return;
        }
        else
            throw embed::WrongStateException(
                        "The object waits for saveCompleted() call!",
                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!" );
    if ( m_bIsLink )
    {
        m_aEntryName = sEntName;
        return;
    }

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY_THROW );

    // detect entry existence
    bool bElExists = xNameAccess->hasByName( sEntName );

    m_aDocMediaDescriptor = GetValuableArgs_Impl( lArguments,
                                                  nEntryConnectionMode != embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT );

    m_bReadOnly = false;
    for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name == "ReadOnly" )
            lArguments[nInd].Value >>= m_bReadOnly;

    // TODO: use lObjArgs for StoreVisualReplacement
    for ( sal_Int32 nObjInd = 0; nObjInd < lObjArgs.getLength(); nObjInd++ )
        if ( lObjArgs[nObjInd].Name == "OutplaceDispatchInterceptor" )
        {
            uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
            if ( lObjArgs[nObjInd].Value >>= xDispatchInterceptor )
                m_xDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );
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
                m_bHasClonedSize = true;
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
                m_xDocHolder->SetOutplaceFrameProperties( aOutFrameProps );
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
                m_xDocHolder->SetOutplaceFrameProperties( aOutFrameProps );
            }
            else
                SAL_WARN( "embeddedobj.common", "OCommonEmbeddedObject::setPersistentEntry: illegal type for argument 'OutplaceFrameProperties'!" );
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
            m_xDocHolder->SetComponent( InitNewDocument_Impl(), m_bReadOnly );
            if ( !m_xDocHolder->GetComponent().is() )
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
            m_xDocHolder->SetComponent( InitNewDocument_Impl(), m_bReadOnly );

            if ( !m_xDocHolder->GetComponent().is() )
                throw io::IOException(); // TODO: can not create document

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
        {
            m_xDocHolder->SetComponent( CreateDocFromMediaDescr_Impl( lArguments ), m_bReadOnly );
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( "Wrong connection mode is provided!",
                                        static_cast< ::cppu::OWeakObject* >(this),
                                        3 );
    }
}


void SAL_CALL OCommonEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "Can't store object without persistence!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!" );
    if ( m_bIsLink )
        return;

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!" );

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
        SAL_WARN( "embeddedobj.common", "Can not retrieve target storage media type!" );
    }
    if (nTargetStorageFormat == SOFFICE_FILEFORMAT_60)
    {
        SAL_INFO("embeddedobj.common", "fdo#78159: Storing OOoXML as ODF");
        nTargetStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
        // setting MediaType is done later anyway, no need to do it here
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
        SAL_WARN( "embeddedobj.common", "Can not retrieve own storage media type!" );
    }

    bool bTryOptimization = false;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        // StoreVisualReplacement and VisualReplacement args have no sense here
        if ( lObjArgs[nInd].Name == "CanTryOptimization" )
            lObjArgs[nInd].Value >>= bTryOptimization;
    }

    bool bSwitchBackToLoaded = false;

    // Storing to different format can be done only in running state.
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // TODO/LATER: copying is not legal for documents with relative links.
        if ( nTargetStorageFormat == nOriginalStorageFormat )
        {
            bool bOptimizationWorks = false;
            if ( bTryOptimization )
            {
                try
                {
                    // try to use optimized copying
                    uno::Reference< embed::XOptimizedStorage > xSource( m_xParentStorage, uno::UNO_QUERY_THROW );
                    uno::Reference< embed::XOptimizedStorage > xTarget( xStorage, uno::UNO_QUERY_THROW );
                    xSource->copyElementDirectlyTo( m_aEntryName, xTarget, sEntName );
                    bOptimizationWorks = true;
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
            bSwitchBackToLoaded = true;
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
        StoreDocToStorage_Impl(
            xSubStorage, lArguments, lObjArgs, nTargetStorageFormat, sEntName, false );
        aGuard.reset();

        if ( bSwitchBackToLoaded )
            changeState( embed::EmbedStates::LOADED );
    }

    // TODO: should the listener notification be done?
}


void SAL_CALL OCommonEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    // TODO: use lObjArgs

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "Can't store object without persistence!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!" );
    if ( m_bIsLink )
    {
        m_aNewEntryName = sEntName;
        return;
    }

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!" );

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
        SAL_WARN( "embeddedobj.common", "Can not retrieve target storage media type!" );
    }
    if (nTargetStorageFormat == SOFFICE_FILEFORMAT_60)
    {
        SAL_INFO("embeddedobj.common", "fdo#78159: Storing OOoXML as ODF");
        nTargetStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
        // setting MediaType is done later anyway, no need to do it here
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
        SAL_WARN( "embeddedobj.common", "Can not retrieve own storage media type!" );
    }

    PostEvent_Impl( "OnSaveAs" );

    bool bTryOptimization = false;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        // StoreVisualReplacement and VisualReplacement args have no sense here
        if ( lObjArgs[nInd].Name == "CanTryOptimization" )
            lObjArgs[nInd].Value >>= bTryOptimization;
    }

    bool bSwitchBackToLoaded = false;

    // Storing to different format can be done only in running state.
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // TODO/LATER: copying is not legal for documents with relative links.
        if ( nTargetStorageFormat == nOriginalStorageFormat )
        {
            bool bOptimizationWorks = false;
            if ( bTryOptimization )
            {
                try
                {
                    // try to use optimized copying
                    uno::Reference< embed::XOptimizedStorage > xSource( m_xParentStorage, uno::UNO_QUERY_THROW );
                    uno::Reference< embed::XOptimizedStorage > xTarget( xStorage, uno::UNO_QUERY_THROW );
                    xSource->copyElementDirectlyTo( m_aEntryName, xTarget, sEntName );
                    bOptimizationWorks = true;
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
            bSwitchBackToLoaded = true;
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
        StoreDocToStorage_Impl(
            xSubStorage, lArguments, lObjArgs, nTargetStorageFormat, sEntName, false );
        aGuard.reset();

        if ( bSwitchBackToLoaded )
            changeState( embed::EmbedStates::LOADED );
    }

    m_bWaitSaveCompleted = true;
    m_xNewObjectStorage = xSubStorage;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
    m_aNewDocMediaDescriptor = GetValuableArgs_Impl( lArguments, true );

    // TODO: register listeners for storages above, in case thay are disposed
    //       an exception will be thrown on saveCompleted( true )

    // TODO: should the listener notification be done here or in saveCompleted?
}


void SAL_CALL OCommonEmbeddedObject::saveCompleted( sal_Bool bUseNew )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "Can't store object without persistence!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    // for now support of this interface is required to allow breaking of links and converting them to normal embedded
    // objects, so the persist name must be handled correctly ( althowgh no real persist entry is used )
    // OSL_ENSURE( !m_bIsLink, "This method implementation must not be used for links!" );
    if ( m_bIsLink )
    {
        if ( bUseNew )
            m_aEntryName = m_aNewEntryName;
        m_aNewEntryName.clear();
        return;
    }

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    SAL_WARN_IF( !m_bWaitSaveCompleted, "embeddedobj.common", "Unexpected saveCompleted() call!" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewObjectStorage.is() && m_xNewParentStorage.is() , "Internal object information is broken!" );
    if ( !m_xNewObjectStorage.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        SwitchOwnPersistence( m_xNewParentStorage, m_xNewObjectStorage, m_aNewEntryName );
        m_aDocMediaDescriptor = m_aNewDocMediaDescriptor;

        uno::Reference< util::XModifiable > xModif( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xModif.is() )
            xModif->setModified( false );

        PostEvent_Impl( "OnSaveAsDone");
    }
    else
    {
        try {
            uno::Reference< lang::XComponent > xComponent( m_xNewObjectStorage, uno::UNO_QUERY );
            SAL_WARN_IF( !xComponent.is(), "embeddedobj.common", "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
    }

    m_xNewObjectStorage.clear();
    m_xNewParentStorage.clear();
    m_aNewEntryName.clear();
    m_aNewDocMediaDescriptor.realloc( 0 );
    m_bWaitSaveCompleted = false;

    if ( bUseNew )
    {
        // TODO: notify listeners

        if ( m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE )
        {
            // TODO: update visual representation
        }
    }
}


sal_Bool SAL_CALL OCommonEmbeddedObject::hasEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_xObjectStorage.is() )
        return true;

    return false;
}


OUString SAL_CALL OCommonEmbeddedObject::getEntryName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "The object persistence is not initialized!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    return m_aEntryName;
}


void SAL_CALL OCommonEmbeddedObject::storeOwn()
{
    // during switching from Activated to Running and from Running to Loaded states the object will
    // ask container to store the object, the container has to make decision
    // to do so or not

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "Can't store object without persistence!",
                                    static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: access denied

    // nothing to do, if the object is in loaded state
    if ( m_nObjectState == embed::EmbedStates::LOADED )
        return;

    PostEvent_Impl( "OnSave" );

    SAL_WARN_IF( !m_xDocHolder->GetComponent().is(), "embeddedobj.common", "If an object is activated or in running state it must have a document!" );
    if ( !m_xDocHolder->GetComponent().is() )
        throw uno::RuntimeException();

    if ( m_bIsLink )
    {
        // TODO: just store the document to its location
        uno::Reference< frame::XStorable > xStorable( m_xDocHolder->GetComponent(), uno::UNO_QUERY_THROW );

        // free the main mutex for the storing time
        aGuard.clear();

        xStorable->store();

        aGuard.reset();
    }
    else
    {
        OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!" );

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
            SAL_WARN( "embeddedobj.common", "Can not retrieve storage media type!" );
        }
        if (nStorageFormat == SOFFICE_FILEFORMAT_60)
        {
            SAL_INFO("embeddedobj.common", "fdo#78159: Storing OOoXML as ODF");
            nStorageFormat = SOFFICE_FILEFORMAT_CURRENT;
            // setting MediaType is done later anyway, no need to do it here
        }

        aGuard.clear();
        uno::Sequence<beans::PropertyValue> aEmpty;
        uno::Sequence<beans::PropertyValue> aMediaArgs(1);
        aMediaArgs[0].Name = "DocumentBaseURL";
        aMediaArgs[0].Value <<= GetBaseURL_Impl();
        StoreDocToStorage_Impl( m_xObjectStorage, aMediaArgs, aEmpty, nStorageFormat, m_aEntryName, true );
        aGuard.reset();
    }

    uno::Reference< util::XModifiable > xModif( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
    if ( xModif.is() )
        xModif->setModified( false );

    PostEvent_Impl( "OnSaveDone" );
}


sal_Bool SAL_CALL OCommonEmbeddedObject::isReadonly()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "The object persistence is not initialized!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    return m_bReadOnly;
}


void SAL_CALL OCommonEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments,
                const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    // TODO: use lObjArgs
    // for now this method is used only to switch readonly state

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( "The object persistence is not initialized!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        // the object is still not loaded
        throw embed::WrongStateException(
                                "The object must be in loaded state to be reloaded!",
                                static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_bIsLink )
    {
        // reload of the link
        OUString aOldLinkFilter = m_aLinkFilterName;

        OUString aNewLinkFilter;
        for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        {
            if ( lArguments[nInd].Name == "URL" )
            {
                // the new URL
                lArguments[nInd].Value >>= m_aLinkURL;
                m_aLinkFilterName.clear();
            }
            else if ( lArguments[nInd].Name == "FilterName" )
            {
                lArguments[nInd].Value >>= aNewLinkFilter;
                m_aLinkFilterName.clear();
            }
        }

        ::comphelper::MimeConfigurationHelper aHelper( m_xContext );
        if ( m_aLinkFilterName.isEmpty() )
        {
            if ( !aNewLinkFilter.isEmpty() )
                m_aLinkFilterName = aNewLinkFilter;
            else
            {
                uno::Sequence< beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name = "URL";
                aArgs[0].Value <<= m_aLinkURL;
                m_aLinkFilterName = aHelper.UpdateMediaDescriptorWithFilterName( aArgs, false );
            }
        }

        if ( !aOldLinkFilter.equals( m_aLinkFilterName ) )
        {
            uno::Sequence< beans::NamedValue > aObject = aHelper.GetObjectPropsByFilter( m_aLinkFilterName );

            // TODO/LATER: probably the document holder could be cleaned explicitly as in the destructor
            m_xDocHolder.clear();

            LinkInit_Impl( aObject, lArguments, lObjArgs );
        }
    }

    m_aDocMediaDescriptor = GetValuableArgs_Impl( lArguments, true );

    // TODO: use lObjArgs for StoreVisualReplacement
    for ( sal_Int32 nObjInd = 0; nObjInd < lObjArgs.getLength(); nObjInd++ )
        if ( lObjArgs[nObjInd].Name == "OutplaceDispatchInterceptor" )
        {
            uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
            if ( lObjArgs[nObjInd].Value >>= xDispatchInterceptor )
                m_xDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );

            break;
        }

    // TODO:
    // when document allows reloading through API the object can be reloaded not only in loaded state

    bool bOldReadOnlyValue = m_bReadOnly;

    m_bReadOnly = false;
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

sal_Bool SAL_CALL OCommonEmbeddedObject::isStored()
{
    uno::Reference<container::XNameAccess> xNA(m_xObjectStorage, uno::UNO_QUERY);
    if (!xNA.is())
        return false;

    return xNA->getElementNames().getLength() > 0;
}


void SAL_CALL OCommonEmbeddedObject::breakLink( const uno::Reference< embed::XStorage >& xStorage,
                                                const OUString& sEntName )
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_bIsLink )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    "The object is not a valid linked object!",
                    static_cast< ::cppu::OWeakObject* >(this) );
    }
    else
    {
        // the current implementation of OOo links does not implement this method since it does not implement
        // all the set of interfaces required for OOo embedded object ( XEmbedPersist is not supported ).
    }

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    if ( !m_bIsLink || m_nObjectState == -1 )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    "The object is not a valid linked object!",
                    static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY_THROW );

    m_bReadOnly = false;

    if ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) )
        SwitchOwnPersistence( xStorage, sEntName );

    // for linked object it means that it becomes embedded object
    // the document must switch it's persistence also

    // TODO/LATER: handle the case when temp doc can not be created
    // the document is a new embedded object so it must be marked as modified
    uno::Reference< util::XCloseable > xDocument = CreateTempDocFromLink_Impl();
    uno::Reference< util::XModifiable > xModif( m_xDocHolder->GetComponent(), uno::UNO_QUERY_THROW );
    try
    {
        xModif->setModified( true );
    }
    catch( const uno::Exception& )
    {}

    m_xDocHolder->SetComponent( xDocument, m_bReadOnly );
    SAL_WARN_IF( !m_xDocHolder->GetComponent().is(), "embeddedobj.common", "If document can't be created, an exception must be thrown!" );

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // the state is changed and can not be switched to loaded state back without saving
        m_nObjectState = embed::EmbedStates::RUNNING;
        StateChangeNotification_Impl( false, embed::EmbedStates::LOADED, m_nObjectState, aGuard );
    }
    else if ( m_nObjectState == embed::EmbedStates::ACTIVE )
        m_xDocHolder->Show();

    m_bIsLink = false;
    m_aLinkFilterName.clear();
    m_aLinkURL.clear();
}


sal_Bool SAL_CALL  OCommonEmbeddedObject::isLink()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_bIsLink;
}


OUString SAL_CALL OCommonEmbeddedObject::getLinkURL()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_bIsLink )
        throw embed::WrongStateException(
                    "The object is not a link object!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    return m_aLinkURL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
