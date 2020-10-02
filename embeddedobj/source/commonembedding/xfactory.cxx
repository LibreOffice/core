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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/documentconstants.hxx>

#include "xfactory.hxx"
#include <commonembobj.hxx>
#include <specialobject.hxx>


using namespace ::com::sun::star;


uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitFromEntry(
                                                                    const uno::Reference< embed::XStorage >& xStorage,
                                                                    const OUString& sEntName,
                                                                    const uno::Sequence< beans::PropertyValue >& aMediaDescr,
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

    uno::Reference< uno::XInterface > xResult;
    if ( !xStorage->isStorageElement( sEntName ) )
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }
    // the object must be based on storage
    uno::Reference< embed::XStorage > xSubStorage =
            xStorage->openStorageElement( sEntName, embed::ElementModes::READ );

    uno::Reference< beans::XPropertySet > xPropSet( xSubStorage, uno::UNO_QUERY_THROW );

    OUString aMediaType;
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
    xSubStorage.clear();

    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByMediaType( aMediaType );

    // If the sequence is empty, fall back to the FileFormatVersion=6200 filter, Base only has that.
    if (!aObject.hasElements() && aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII)
        aObject = m_aConfigHelper.GetObjectPropsByMediaType(MIMETYPE_VND_SUN_XML_BASE_ASCII);

    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage

    xResult.set(static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                            m_xContext,
                                            aObject ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::DEFAULT_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
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

    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is FilterName
    OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, false );

    uno::Reference< uno::XInterface > xResult;

    // find document service name
    if ( aFilterName.isEmpty() )
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }
    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByFilter( aFilterName );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage


    xResult.set(static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                        m_xContext,
                                        aObject ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aTempMedDescr,
                                    lObjArgs );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitNew(
                                            const uno::Sequence< sal_Int8 >& aClassID,
                                            const OUString& /*aClassName*/,
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    uno::Reference< uno::XInterface > xResult;

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByClassID( aClassID );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage

    xResult.set( static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                             m_xContext,
                                             aObject ) ),
                 uno::UNO_QUERY );


    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::TRUNCATE_INIT,
                                    uno::Sequence< beans::PropertyValue >(),
                                    lObjArgs );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceUserInit(
            const uno::Sequence< sal_Int8 >& aClassID,
            const OUString& /*aClassName*/,
            const uno::Reference< embed::XStorage >& xStorage,
            const OUString& sEntName,
            sal_Int32 nEntryConnectionMode,
            const uno::Sequence< beans::PropertyValue >& lArguments,
            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    // the initialization is completely controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByClassID( aClassID );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage

    uno::Sequence< beans::PropertyValue > aTempMedDescr( lArguments );
    if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
    {
        OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, aObject );
        if ( aFilterName.isEmpty() )
        // the object must be OOo embedded object, if it is not an exception must be thrown
            throw io::IOException(); // TODO:
    }

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                                m_xContext,
                                                aObject ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                  sEntName,
                                  nEntryConnectionMode,
                                  aTempMedDescr,
                                  lObjArgs );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceLink(
                                            const uno::Reference< embed::XStorage >& /*xStorage*/,
                                            const OUString& /*sEntName*/,
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
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );

    OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, false );

    if ( aFilterName.isEmpty() )
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }
    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByFilter( aFilterName );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage


    xResult.set(static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                        m_xContext,
                                        aObject,
                                        aTempMedDescr,
                                        lObjArgs ) ),
                uno::UNO_QUERY );

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceLinkUserInit(
                                                const uno::Sequence< sal_Int8 >& aClassID,
                                                const OUString& /*aClassName*/,
                                                const uno::Reference< embed::XStorage >& xStorage,
                                                const OUString& sEntName,
                                                const uno::Sequence< beans::PropertyValue >& lArguments,
                                                const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    uno::Reference< uno::XInterface > xResult;

    // the initialization is completely controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Sequence< beans::PropertyValue > aTempMedDescr( lArguments );

    OUString aURL;
    for ( beans::PropertyValue const & prop : std::as_const(aTempMedDescr) )
        if ( prop.Name == "URL" )
            prop.Value >>= aURL;

    if ( aURL.isEmpty() )
        throw lang::IllegalArgumentException( "No URL for the link is provided!",
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );

    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByClassID( aClassID );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage

    OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, aObject );

    if ( aFilterName.isEmpty() )
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    xResult.set(static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                        m_xContext,
                                        aObject,
                                        aTempMedDescr,
                                        lObjArgs ) ),
                uno::UNO_QUERY );

    return xResult;
}

OUString SAL_CALL OOoEmbeddedObjectFactory::getImplementationName()
{
    return "com.sun.star.comp.embed.OOoEmbeddedObjectFactory";
}

sal_Bool SAL_CALL OOoEmbeddedObjectFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OOoEmbeddedObjectFactory::getSupportedServiceNames()
{
    return { "com.sun.star.embed.OOoEmbeddedObjectFactory", "com.sun.star.comp.embed.OOoEmbeddedObjectFactory" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
embeddedobj_OOoEmbeddedObjectFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new OOoEmbeddedObjectFactory(context)));
}


uno::Reference< uno::XInterface > SAL_CALL OOoSpecialEmbeddedObjectFactory::createInstanceUserInit(
            const uno::Sequence< sal_Int8 >& aClassID,
            const OUString& /*aClassName*/,
            const uno::Reference< embed::XStorage >& /*xStorage*/,
            const OUString& /*sEntName*/,
            sal_Int32 /*nEntryConnectionMode*/,
            const uno::Sequence< beans::PropertyValue >& /*lArguments*/,
            const uno::Sequence< beans::PropertyValue >& /*lObjArgs*/ )
{
    uno::Sequence< beans::NamedValue > aObject = m_aConfigHelper.GetObjectPropsByClassID( aClassID );
    if ( !aObject.hasElements() )
        throw io::IOException(); // unexpected mimetype of the storage

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OSpecialEmbeddedObject(
                                                m_xContext,
                                                aObject ) ),
                    uno::UNO_QUERY );
    return xResult;
}

OUString SAL_CALL OOoSpecialEmbeddedObjectFactory::getImplementationName()
{
    return "com.sun.star.comp.embed.OOoSpecialEmbeddedObjectFactory";
}

sal_Bool SAL_CALL OOoSpecialEmbeddedObjectFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OOoSpecialEmbeddedObjectFactory::getSupportedServiceNames()
{
    return { "com.sun.star.embed.OOoSpecialEmbeddedObjectFactory", "com.sun.star.comp.embed.OOoSpecialEmbeddedObjectFactory" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
embeddedobj_OOoSpecialEmbeddedObjectFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new OOoSpecialEmbeddedObjectFactory(context)));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
