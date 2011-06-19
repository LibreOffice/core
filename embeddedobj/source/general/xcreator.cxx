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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_embeddedobj.hxx"
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#include <com/sun/star/embed/XLinkFactory.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/logfile.hxx>


#include <xcreator.hxx>
#include <dummyobject.hxx>


using namespace ::com::sun::star;


//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL UNOEmbeddedObjectCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.EmbeddedObjectCreator"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.EmbeddedObjectCreator"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOEmbeddedObjectCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.EmbeddedObjectCreator"));
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new UNOEmbeddedObjectCreator( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitNew(
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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceInitNew" );

    uno::Reference< uno::XInterface > xResult;

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    ::rtl::OUString aEmbedFactory = m_aConfigHelper.GetFactoryNameByClassID( aClassID );
    if ( !aEmbedFactory.getLength() )
    {
        // use system fallback
        // TODO: in future users factories can be tested
        aEmbedFactory = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLEEmbeddedObjectFactory" ));
    }

    uno::Reference < uno::XInterface > xFact( m_xFactory->createInstance( aEmbedFactory ) );
    uno::Reference< embed::XEmbedObjectCreator > xEmbCreator( xFact, uno::UNO_QUERY );
    if ( xEmbCreator.is() )
        return xEmbCreator->createInstanceInitNew( aClassID, aClassName, xStorage, sEntName, lObjArgs );

    uno::Reference < embed::XEmbedObjectFactory > xEmbFact( xFact, uno::UNO_QUERY );
    if ( !xEmbFact.is() )
        throw uno::RuntimeException();
    return xEmbFact->createInstanceUserInit( aClassID, aClassName, xStorage, sEntName, embed::EntryInitModes::TRUNCATE_INIT, uno::Sequence < beans::PropertyValue >(), lObjArgs);
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitFromEntry(
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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceInitFromEntry" );

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    if ( !xNameAccess->hasByName( sEntName ) )
        throw container::NoSuchElementException();

    ::rtl::OUString aMediaType;
    ::rtl::OUString aEmbedFactory;
    if ( xStorage->isStorageElement( sEntName ) )
    {
        // the object must be based on storage
        uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::READ );

        uno::Reference< beans::XPropertySet > xPropSet( xSubStorage, uno::UNO_QUERY );
        if ( !xPropSet.is() )
            throw uno::RuntimeException();

        try {
            uno::Any aAny = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ));
            aAny >>= aMediaType;
        }
        catch ( uno::Exception& )
        {
        }

        try {
            uno::Reference< lang::XComponent > xComp( xSubStorage, uno::UNO_QUERY );
            if ( xComp.is() )
                xComp->dispose();
        }
        catch ( uno::Exception& )
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

        uno::Reference< beans::XPropertySet > xPropSet( xSubStream, uno::UNO_QUERY );
        if ( !xPropSet.is() )
            throw uno::RuntimeException();

        try {
            uno::Any aAny = xPropSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ));
            aAny >>= aMediaType;
            if ( aMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.oleobject" ) ) ) )
                aEmbedFactory = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) );
        }
        catch ( uno::Exception& )
        {
        }

        try {
            uno::Reference< lang::XComponent > xComp( xSubStream, uno::UNO_QUERY );
            if ( xComp.is() )
                xComp->dispose();
        }
        catch ( uno::Exception& )
        {
        }
    }

    OSL_ENSURE( aMediaType.getLength(), "No media type is specified for the object!" );
    if ( aMediaType.getLength() && !aEmbedFactory.getLength() )
        aEmbedFactory = m_aConfigHelper.GetFactoryNameByMediaType( aMediaType );

    if ( aEmbedFactory.getLength() )
    {
        uno::Reference< uno::XInterface > xFact = m_xFactory->createInstance( aEmbedFactory );

        uno::Reference< embed::XEmbedObjectCreator > xEmbCreator( xFact, uno::UNO_QUERY );
        if ( xEmbCreator.is() )
            return xEmbCreator->createInstanceInitFromEntry( xStorage, sEntName, aMedDescr, lObjArgs );

        uno::Reference < embed::XEmbedObjectFactory > xEmbFact( xFact, uno::UNO_QUERY );
        if ( xEmbFact.is() )
            return xEmbFact->createInstanceUserInit( uno::Sequence< sal_Int8 >(), ::rtl::OUString(), xStorage, sEntName, embed::EntryInitModes::DEFAULT_INIT, aMedDescr, lObjArgs);
    }

    // the default object should be created, it will allow to store the contents on the next saving
    uno::Reference< uno::XInterface > xResult( static_cast< cppu::OWeakObject* >( new ODummyEmbeddedObject() ) );
    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage, sEntName, embed::EntryInitModes::DEFAULT_INIT, aMedDescr, lObjArgs );
    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const ::rtl::OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceInitFromMediaDescriptor" );

    // TODO: use lObjArgs

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< uno::XInterface > xResult;
    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is FilterName
    ::rtl::OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, sal_False );

    if ( aFilterName.getLength() )
    {
        // the object can be loaded by one of the office application
        uno::Reference< embed::XEmbedObjectCreator > xOOoEmbCreator(
                            m_xFactory->createInstance(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) )),
                            uno::UNO_QUERY );
        if ( !xOOoEmbCreator.is() )
            throw uno::RuntimeException(); // TODO:

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

        uno::Reference< embed::XEmbedObjectCreator > xOleEmbCreator(
                            m_xFactory->createInstance(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) )),
                            uno::UNO_QUERY );
        if ( !xOleEmbCreator.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOleEmbCreator->createInstanceInitFromMediaDescriptor( xStorage, sEntName, aTempMedDescr, lObjArgs );
    }

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceUserInit(
        const uno::Sequence< sal_Int8 >& aClassID,
        const ::rtl::OUString& sClassName,
        const uno::Reference< embed::XStorage >& xStorage,
        const ::rtl::OUString& sEntName,
        sal_Int32 nEntryConnectionMode,
        const uno::Sequence< beans::PropertyValue >& aArgs,
        const uno::Sequence< beans::PropertyValue >& aObjectArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceUserInit" );

    uno::Reference< uno::XInterface > xResult;

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    ::rtl::OUString aEmbedFactory = m_aConfigHelper.GetFactoryNameByClassID( aClassID );
    uno::Reference< embed::XEmbedObjectFactory > xEmbFactory(
                        m_xFactory->createInstance( aEmbedFactory ),
                        uno::UNO_QUERY );
    if ( !xEmbFactory.is() )
        throw uno::RuntimeException(); // TODO:

    return xEmbFactory->createInstanceUserInit( aClassID,
                                                sClassName,
                                                xStorage,
                                                sEntName,
                                                nEntryConnectionMode,
                                                aArgs,
                                                aObjectArgs );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceLink(
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const ::rtl::OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceLink" );

    uno::Reference< uno::XInterface > xResult;

    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is URL, URL must exist
    ::rtl::OUString aURL;
    for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
        if ( aTempMedDescr[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "URL" ) ) )
            aTempMedDescr[nInd].Value >>= aURL;

    if ( !aURL.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No URL for the link is provided!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );

    ::rtl::OUString aFilterName = m_aConfigHelper.UpdateMediaDescriptorWithFilterName( aTempMedDescr, sal_False );

    if ( aFilterName.getLength() )
    {
        // the object can be loaded by one of the office application
        uno::Reference< embed::XLinkCreator > xOOoLinkCreator(
                            m_xFactory->createInstance(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) )),
                            uno::UNO_QUERY );
        if ( !xOOoLinkCreator.is() )
            throw uno::RuntimeException(); // TODO:

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
            throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                                uno::Reference< uno::XInterface >(
                                                    static_cast< ::cppu::OWeakObject* >(this) ),
                                                3 );

        if ( !sEntName.getLength() )
            throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                                uno::Reference< uno::XInterface >(
                                                    static_cast< ::cppu::OWeakObject* >(this) ),
                                                4 );

        uno::Reference< embed::XLinkCreator > xLinkCreator(
                            m_xFactory->createInstance(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) )),
                            uno::UNO_QUERY );
        if ( !xLinkCreator.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xLinkCreator->createInstanceLink( xStorage, sEntName, aTempMedDescr, lObjArgs );
    }

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL UNOEmbeddedObjectCreator::createInstanceLinkUserInit(
                                                const uno::Sequence< sal_Int8 >& aClassID,
                                                const ::rtl::OUString& aClassName,
                                                const uno::Reference< embed::XStorage >& xStorage,
                                                const ::rtl::OUString& sEntName,
                                                const uno::Sequence< beans::PropertyValue >& lArguments,
                                                const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) UNOEmbeddedObjectCreator::createInstanceLinkUserInit" );

    uno::Reference< uno::XInterface > xResult;

    ::rtl::OUString aEmbedFactory = m_aConfigHelper.GetFactoryNameByClassID( aClassID );
    uno::Reference< embed::XLinkFactory > xLinkFactory(
                        m_xFactory->createInstance( aEmbedFactory ),
                        uno::UNO_QUERY );
    if ( !xLinkFactory.is() )
        throw uno::RuntimeException(); // TODO:

    return xLinkFactory->createInstanceLinkUserInit( aClassID,
                                                    aClassName,
                                                    xStorage,
                                                    sEntName,
                                                    lArguments,
                                                    lObjArgs );

}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOEmbeddedObjectCreator::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL UNOEmbeddedObjectCreator::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL UNOEmbeddedObjectCreator::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
