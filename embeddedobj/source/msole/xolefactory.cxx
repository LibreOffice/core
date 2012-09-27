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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <rtl/logfile.hxx>


#include "xolefactory.hxx"
#include "oleembobj.hxx"


using namespace ::com::sun::star;

// TODO: do not create OLE objects that represent OOo documents

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OleEmbeddedObjectFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.OLEEmbeddedObjectFactory"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.OLEEmbeddedObjectFactory"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObjectFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.OLEEmbeddedObjectFactory"));
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OleEmbeddedObjectFactory( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitFromEntry(
                                                                    const uno::Reference< embed::XStorage >& xStorage,
                                                                    const ::rtl::OUString& sEntName,
                                                                    const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            container::NoSuchElementException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObjectFactory::createInstanceInitFromEntry" );

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    if ( !xNameAccess->hasByName( sEntName ) )
        throw container::NoSuchElementException();

    if ( !xStorage->isStreamElement( sEntName ) )
    {
        // if it is not an OLE object throw an exception
        throw io::IOException(); // TODO:
    }

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xFactory, sal_False ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported by own document objects

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::DEFAULT_INIT,
                                    aMedDescr,
                                    lObjArgs );

    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
    {
        if ( lObjArgs[nInd].Name == "CloneFrom" )
        {
            try
            {
                uno::Reference < embed::XEmbeddedObject > xObj;
                uno::Reference < embed::XEmbeddedObject > xNew( xResult, uno::UNO_QUERY );
                lObjArgs[nInd].Value >>= xObj;
                if ( xObj.is() )
                    xNew->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, xObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ) );
            }
            catch ( const uno::Exception& ) {}
            break;
        }
    }

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const ::rtl::OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObjectFactory::createInstanceInitFromMediaDescriptor" );

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xFactory, sal_False ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported ( what about applets? )

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitNew(
                                            const uno::Sequence< sal_Int8 >& aClassID,
                                            const ::rtl::OUString& aClassName,
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const ::rtl::OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObjectFactory::createInstanceInitNew" );

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xFactory, aClassID, aClassName ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported by own document objects

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::TRUNCATE_INIT,
                                    uno::Sequence< beans::PropertyValue >(),
                                    lObjArgs );

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceLink(
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const ::rtl::OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObjectFactory::createInstanceLink" );

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >(
                                                static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >(
                                                static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xFactory, sal_True ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported by own document objects

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceUserInit(
            const uno::Sequence< sal_Int8 >& aClassID,
            const ::rtl::OUString& aClassName,
            const uno::Reference< embed::XStorage >& xStorage,
            const ::rtl::OUString& sEntName,
            sal_Int32 /*nEntryConnectionMode*/,
            const uno::Sequence< beans::PropertyValue >& /*lArguments*/,
            const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObjectFactory::createInstanceUserInit" );

    // the initialization is completelly controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xFactory, aClassID, aClassName ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );
    if ( xPersist.is() )
    {
        xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::DEFAULT_INIT,
                                    uno::Sequence< beans::PropertyValue >(),
                                    lObjArgs );

    }
    else
        throw uno::RuntimeException(); // TODO:

    return xResult;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObjectFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OleEmbeddedObjectFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OleEmbeddedObjectFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
