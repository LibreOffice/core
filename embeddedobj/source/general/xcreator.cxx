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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#include <com/sun/star/embed/OOoEmbeddedObjectFactory.hpp>
#include <com/sun/star/embed/OLEEmbeddedObjectFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/documentconstants.hxx>
#include <officecfg/Office/Common.hxx>

#include <xcreator.hxx>
#include <dummyobject.hxx>


using namespace ::com::sun::star;

uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitNew(
                                            const uno::Sequence< sal_Int8 >& aClassID,
                                            const OUString& aClassName,
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            3 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            4 );

    OUString aEmbedFactory = m_aConfigHelper.GetFactoryNameByClassID( aClassID );
    if ( aEmbedFactory.isEmpty() )
    {
        // use system fallback
        // TODO: in future users factories can be tested
        aEmbedFactory = "com.sun.star.embed.OLEEmbeddedObjectFactory";
    }

    uno::Reference < uno::XInterface > xFact( m_xContext->getServiceManager()->createInstanceWithContext(aEmbedFactory, m_xContext) );
    uno::Reference< embed::XEmbedObjectCreator > xEmbCreator( xFact, uno::UNO_QUERY );
    if ( xEmbCreator.is() )
        return xEmbCreator->createInstanceInitNew( aClassID, aClassName, xStorage, sEntName, lObjArgs );

    uno::Reference < embed::XEmbedObjectFactory > xEmbFact( xFact, uno::UNO_QUERY_THROW );
    return xEmbFact->createInstanceUserInit( aClassID, aClassName, xStorage, sEntName, embed::EntryInitModes::TRUNCATE_INIT, uno::Sequence < beans::PropertyValue >(), lObjArgs);
}


uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitFromEntry(
                                                                    const uno::Reference< embed::XStorage >& xStorage,
                                                                    const OUString& sEntName,
                                                                    const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY_THROW );

    // detect entry existence
    if ( !xNameAccess->hasByName( sEntName ) )
        throw container::NoSuchElementException();

    OUString aMediaType;
    OUString aEmbedFactory;
    if ( xStorage->isStorageElement( sEntName ) )
    {
        // the object must be based on storage
        uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::READ );

        uno::Reference< beans::XPropertySet > xPropSet( xSubStorage, uno::UNO_QUERY_THROW );

        try {
            uno::Any aAny = xPropSet->getPropertyValue("MediaType");
            aAny >>= aMediaType;
        }
        catch ( const uno::Exception& )
        {
        }

        try {
            if ( xSubStorage.is() )
                xSubStorage->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
    }
    else
    {
        // the object must be based on stream
        // it means for now that this is an OLE object

        // the object will be created as embedded object
        // after it is loaded it can detect that it is a link

        uno::Reference< io::XStream > xSubStream =
                xStorage->openStreamElement( sEntName, embed::ElementModes::READ );

        uno::Reference< beans::XPropertySet > xPropSet( xSubStream, uno::UNO_QUERY_THROW );

        try {
            uno::Any aAny = xPropSet->getPropertyValue("MediaType");
            aAny >>= aMediaType;
            if ( aMediaType == "application/vnd.sun.star.oleobject" )
                aEmbedFactory = "com.sun.star.embed.OLEEmbeddedObjectFactory";
        }
        catch ( const uno::Exception& )
        {
        }

        try {
            uno::Reference< lang::XComponent > xComp( xSubStream, uno::UNO_QUERY );
            if ( xComp.is() )
                xComp->dispose();
        }
        catch ( const uno::Exception& )
        {
        }
    }

    OSL_ENSURE( !aMediaType.isEmpty(), "No media type is specified for the object!" );
    if ( !aMediaType.isEmpty() && aEmbedFactory.isEmpty() )
    {
        aEmbedFactory = m_aConfigHelper.GetFactoryNameByMediaType( aMediaType );

        // If no factory is found, fall back to the FileFormatVersion=6200 filter, Base only has that.
        if (aEmbedFactory.isEmpty() && aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII)
            aEmbedFactory = m_aConfigHelper.GetFactoryNameByMediaType(MIMETYPE_VND_SUN_XML_BASE_ASCII);
    }

    if ( !aEmbedFactory.isEmpty() )
    {
        uno::Reference< uno::XInterface > xFact = m_xContext->getServiceManager()->createInstanceWithContext(aEmbedFactory, m_xContext);

        uno::Reference< embed::XEmbedObjectCreator > xEmbCreator( xFact, uno::UNO_QUERY );
        if ( xEmbCreator.is() )
            return xEmbCreator->createInstanceInitFromEntry( xStorage, sEntName, aMedDescr, lObjArgs );

        uno::Reference < embed::XEmbedObjectFactory > xEmbFact( xFact, uno::UNO_QUERY );
        if ( xEmbFact.is() )
            return xEmbFact->createInstanceUserInit( uno::Sequence< sal_Int8 >(), OUString(), xStorage, sEntName, embed::EntryInitModes::DEFAULT_INIT, aMedDescr, lObjArgs);
    }

    // the default object should be created, it will allow to store the contents on the next saving
    uno::Reference< uno::XInterface > xResult( static_cast< cppu::OWeakObject* >( new ODummyEmbeddedObject() ) );
    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage, sEntName, embed::EntryInitModes::DEFAULT_INIT, aMedDescr, lObjArgs );
    return xResult;
}

/**
 * Decides if rFilter should be used to load data into a doc model or real OLE embedding should
 * happen. Empty return value means the later.
 */
static OUString HandleFilter(const uno::Reference<uno::XComponentContext>& xComponentContext,
                             const OUString& rFilter)
{
    OUString aRet = rFilter;

    if (!officecfg::Office::Common::Filter::Microsoft::Import::WinWordToWriter::get(
            xComponentContext))
    {
        if (rFilter == "MS Word 97" || rFilter == "MS Word 2007 XML")
        {
            aRet.clear();
        }
    }

    if (!officecfg::Office::Common::Filter::Microsoft::Import::ExcelToCalc::get(xComponentContext))
    {
        if (rFilter == "MS Excel 97" || rFilter == "Calc MS Excel 2007 XML")
        {
            aRet.clear();
        }
    }
    if (!officecfg::Office::Common::Filter::Microsoft::Import::PowerPointToImpress::get(
            xComponentContext))
    {
        if (rFilter == "MS PowerPoint 97" || rFilter == "Impress MS PowerPoint 2007 XML")
        {
            aRet.clear();
        }
    }
    if (!officecfg::Office::Common::Filter::Microsoft::Import::VisioToDraw::get(xComponentContext))
    {
        if (rFilter == "Visio Document")
        {
            aRet.clear();
        }
    }
    if (!officecfg::Office::Common::Filter::Adobe::Import::PDFToDraw::get(xComponentContext))
    {
        if (rFilter == "draw_pdf_import")
        {
            aRet.clear();
        }
    }

    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    // TODO: use lObjArgs

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< uno::XInterface > xResult;
    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is FilterName
    OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, false );

    aFilterName = HandleFilter(m_xContext, aFilterName);

    if ( !aFilterName.isEmpty() )
    {
        // the object can be loaded by one of the office application
        uno::Reference< embed::XEmbeddedObjectCreator > xOOoEmbCreator =
                            embed::OOoEmbeddedObjectFactory::create( m_xContext );

        xResult = xOOoEmbCreator->createInstanceInitFromMediaDescriptor( xStorage,
                                                                         sEntName,
                                                                         aTempMedDescr,
                                                                         lObjArgs );
    }
    else
    {
        // must be an OLE object

        // TODO: in future, when more object types are possible this place seems
        // to be a weak one, probably configuration must provide a type detection service
        // for every factory, so any file could go through services until it is recognized
        // or there is no more services
        // Or for example the typename can be used to detect object type if typedetection
        // was also extended.

        uno::Reference< embed::XEmbeddedObjectCreator > xOleEmbCreator =
                            embed::OLEEmbeddedObjectFactory::create( m_xContext );

        xResult = xOleEmbCreator->createInstanceInitFromMediaDescriptor( xStorage, sEntName, aTempMedDescr, lObjArgs );
    }

    return xResult;
}


uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceUserInit(
        const uno::Sequence< sal_Int8 >& aClassID,
        const OUString& sClassName,
        const uno::Reference< embed::XStorage >& xStorage,
        const OUString& sEntName,
        sal_Int32 nEntryConnectionMode,
        const uno::Sequence< beans::PropertyValue >& aArgs,
        const uno::Sequence< beans::PropertyValue >& aObjectArgs )
{
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            3 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            4 );

    OUString aEmbedFactory = m_aConfigHelper.GetFactoryNameByClassID( aClassID );
    uno::Reference< embed::XEmbedObjectFactory > xEmbFactory(
                        m_xContext->getServiceManager()->createInstanceWithContext(aEmbedFactory, m_xContext),
                        uno::UNO_QUERY_THROW );

    return xEmbFactory->createInstanceUserInit( aClassID,
                                                sClassName,
                                                xStorage,
                                                sEntName,
                                                nEntryConnectionMode,
                                                aArgs,
                                                aObjectArgs );
}


uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceLink(
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    uno::Reference< uno::XInterface > xResult;

    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is URL, URL must exist
    OUString aURL;
    for ( beans::PropertyValue const & prop : std::as_const(aTempMedDescr) )
        if ( prop.Name == "URL" )
            prop.Value >>= aURL;

    if ( aURL.isEmpty() )
        throw lang::IllegalArgumentException( "No URL for the link is provided!",
                                        static_cast< ::cppu::OWeakObject* >(this),
                                        3 );

    OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, false );

    if ( !aFilterName.isEmpty() )
    {
        // the object can be loaded by one of the office application
        uno::Reference< embed::XEmbeddedObjectCreator > xOOoLinkCreator =
                            embed::OOoEmbeddedObjectFactory::create( m_xContext );

        xResult = xOOoLinkCreator->createInstanceLink( xStorage,
                                                        sEntName,
                                                        aTempMedDescr,
                                                        lObjArgs );
    }
    else
    {
        // must be an OLE link

        // TODO: in future, when more object types are possible this place seems
        // to be a weak one, probably configuration must provide a type detection service
        // for every factory, so any file could go through services until it is recognized
        // or there is no more services
        // Or for example the typename can be used to detect object type if typedetection
        // was also extended.

        if ( !xStorage.is() )
            throw lang::IllegalArgumentException( "No parent storage is provided!",
                                                static_cast< ::cppu::OWeakObject* >(this),
                                                3 );

        if ( sEntName.isEmpty() )
            throw lang::IllegalArgumentException( "Empty element name is provided!",
                                                static_cast< ::cppu::OWeakObject* >(this),
                                                4 );

        uno::Reference< embed::XEmbeddedObjectCreator > xLinkCreator =
                            embed::OLEEmbeddedObjectFactory::create( m_xContext);

        xResult = xLinkCreator->createInstanceLink( xStorage, sEntName, aTempMedDescr, lObjArgs );
    }

    return xResult;
}

OUString SAL_CALL UNOEmbeddedObjectCreator::getImplementationName()
{
    return "com.sun.star.comp.embed.EmbeddedObjectCreator";
}

sal_Bool SAL_CALL UNOEmbeddedObjectCreator::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL UNOEmbeddedObjectCreator::getSupportedServiceNames()
{
    return { "com.sun.star.embed.EmbeddedObjectCreator", "com.sun.star.comp.embed.EmbeddedObjectCreator" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
embeddedobj_UNOEmbeddedObjectCreator_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new UNOEmbeddedObjectCreator(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
