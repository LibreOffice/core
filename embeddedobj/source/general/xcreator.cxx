/*************************************************************************
 *
 *  $RCSfile: xcreator.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:44:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDOBJECTFACTORY_HPP_
#include <com/sun/star/embed/XEmbedObjectFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKFACTORY_HPP_
#include <com/sun/star/embed/XLinkFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif


#include "xcreator.hxx"
#include "convert.hxx"


using namespace ::com::sun::star;


::rtl::OUString GetDocServiceNameFromFilter( const ::rtl::OUString& aFilterName,
                                            const uno::Reference< lang::XMultiServiceFactory >& xFactory );


//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL UNOEmbeddedObjectCreator::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.EmbeddedObjectCreator");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.EmbeddedObjectCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL UNOEmbeddedObjectCreator::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.EmbeddedObjectCreator");
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
    uno::Reference< uno::XInterface > xResult;

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    ::rtl::OUString aDocServiceName = GetDocumentServiceNameFromClassID( aClassID );

    if ( aDocServiceName.getLength() )
    {
        uno::Reference< embed::XEmbedObjectFactory > xOOoEmbFact(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOOoEmbFact.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOOoEmbFact->createInstanceUserInit( GetClassIDFromServName( aDocServiceName ),
                                                        GetClassNameFromServName( aDocServiceName ),
                                                        xStorage,
                                                        sEntName,
                                                        embed::EntryInitModes::TRUNCATE_INIT,
                                                        uno::Sequence< beans::PropertyValue >(),
                                                        lObjArgs );
    }
    else
    {
        // can be an OLE object
        // TODO: but in future it can be a different type of an object
        //       the information about implementation and factory to create it
        //       will be retrieved from configuration

        uno::Reference< embed::XEmbedObjectCreator > xOleEmbCreator(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOleEmbCreator.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOleEmbCreator->createInstanceInitNew( aClassID, aClassName, xStorage, sEntName, lObjArgs );
    }

    return xResult;
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
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    if ( !xNameAccess->hasByName( sEntName ) )
        throw container::NoSuchElementException();

    uno::Reference< uno::XInterface > xResult;
    if ( xStorage->isStorageElement( sEntName ) )
    {
        // the object must be based on storage
        uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::READ );

        uno::Reference< beans::XPropertySet > xPropSet( xSubStorage, uno::UNO_QUERY );
        if ( !xPropSet.is() )
            throw uno::RuntimeException();

        ::rtl::OUString aMediaType;
        try {
            uno::Any aAny = xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
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

        // TODO: in future it is possible to have another objects based on storages
        // in this case first a factory service name will be detected from the configuration
        // and then it will be used with ClassID from the same entry

        ::rtl::OUString aDocServiceName = GetDocumentServiceFromMediaType( aMediaType );
        if ( !aDocServiceName.getLength() )
        {
            // only own document can be based on storage for now
            // in case it is not possible to find related
            // document service name the storage entry is invalid

            throw io::IOException(); // unexpected mimetype of the storage
        }

        uno::Reference< embed::XEmbedObjectFactory > xOOoEmbFact(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOOoEmbFact.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOOoEmbFact->createInstanceUserInit( GetClassIDFromServName( aDocServiceName ),
                                                        GetClassNameFromServName( aDocServiceName ),
                                                        xStorage,
                                                        sEntName,
                                                        embed::EntryInitModes::DEFAULT_INIT,
                                                        aMedDescr,
                                                        lObjArgs );
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

        ::rtl::OUString aMediaType;
        try {
            uno::Any aAny = xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ) );
            aAny >>= aMediaType;
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

        if ( aMediaType.equalsAscii( "application/vnd.sun.star.oleobject" ) )
        {
            uno::Reference< embed::XEmbedObjectCreator > xOleEmbCreator(
                                m_xFactory->createInstance(
                                        ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) ),
                                uno::UNO_QUERY );
            if ( !xOleEmbCreator.is() )
                throw uno::RuntimeException(); // TODO:

            xResult = xOleEmbCreator->createInstanceInitFromEntry( xStorage, sEntName, aMedDescr, lObjArgs );
        }
        else
        {
            // TODO: in future it is possible to have different object implementations based on stream
            // in this case the media-type of the stream will be used to detect factory and may be ClassID
            // for OLE objects ClassID will be detected during object loading.
            // if ClassID can not be detected the *Creator interface must be used, otherwise *Factory can be used.

            throw io::IOException(); // unexpected mimetype of the stream
        }
    }

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
    // TODO: use lObjArgs

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is FilterName
    ::rtl::OUString aFilterName;
    for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
        if ( aTempMedDescr[nInd].Name.equalsAscii( "FilterName" ) )
            aTempMedDescr[nInd].Value >>= aFilterName;

    if ( !aFilterName.getLength() )
    {
        // filter name is not specified, so type detection should be done

        uno::Reference< document::XTypeDetection > xTypeDetection(
                m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
                uno::UNO_QUERY );

        if ( !xTypeDetection.is() )
            throw uno::RuntimeException(); // TODO

        // get TypeName
        ::rtl::OUString aTypeName = xTypeDetection->queryTypeByDescriptor( aTempMedDescr, sal_True );

        // get FilterName
        for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
            if ( aTempMedDescr[nInd].Name.equalsAscii( "FilterName" ) )
                aTempMedDescr[nInd].Value >>= aFilterName;

        if ( !aFilterName.getLength() && aTypeName.getLength() )
        {
            uno::Reference<container::XNameAccess> xNameAccess( xTypeDetection, uno::UNO_QUERY );
            if ( xNameAccess.is() && xNameAccess->hasByName( aTypeName ) )
            {
                uno::Sequence<beans::PropertyValue> aTypes;
                xNameAccess->getByName(aTypeName) >>= aTypes;
                for( sal_Int32 nInd = 0; nInd < aTypes.getLength(); nInd++ )
                    if ( aTypes[nInd].Name.equalsAscii( "PreferredFilter" ) )
                    {
                        aTypes[nInd].Value >>= aFilterName;
                        sal_Int32 nLen = aTempMedDescr.getLength();
                        aTempMedDescr.realloc(nLen+1);
                        aTempMedDescr[nLen].Value = aTypes[nInd].Value;
                        aTempMedDescr[nLen].Name = ::rtl::OUString::createFromAscii( "FilterName" );
                        break;
                    }
            }
        }
    }

    uno::Reference< uno::XInterface > xResult;

    if ( aFilterName.getLength() )
    {
        // the object can be loaded by one of the office application
        ::rtl::OUString aDocServiceName = GetDocServiceNameFromFilter( aFilterName, m_xFactory );
        if ( !aDocServiceName.getLength() )
            throw io::IOException(); // TODO: filter detection failed

        uno::Reference< embed::XEmbedObjectFactory > xOOoEmbFact(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOOoEmbFact.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOOoEmbFact->createInstanceUserInit( GetClassIDFromServName( aDocServiceName ),
                                                         GetClassNameFromServName( aDocServiceName ),
                                                         xStorage,
                                                         sEntName,
                                                         embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
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
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) ),
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
    uno::Reference< uno::XInterface > xResult;

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            4 );

    ::rtl::OUString aDocServiceName = GetDocumentServiceNameFromClassID( aClassID );

    if ( aDocServiceName.getLength() )
    {
        uno::Reference< embed::XEmbedObjectFactory > xOOoEmbFact(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOOoEmbFact.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOOoEmbFact->createInstanceUserInit( aClassID,
                                                        sClassName,
                                                        xStorage,
                                                        sEntName,
                                                        nEntryConnectionMode,
                                                        aArgs,
                                                        aObjectArgs );
    }
    else
    {
        // can be an OLE object
        // TODO: but in future it can be a different type of an object
        //       the information about implementation and factory to create it
        //       will be retrieved from configuration

        uno::Reference< embed::XEmbedObjectFactory > xOleEmbFactory(
                            m_xFactory->createInstance(
                                    ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xOleEmbFactory.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOleEmbFactory->createInstanceUserInit( aClassID,
                                                        sClassName,
                                                        xStorage,
                                                        sEntName,
                                                        nEntryConnectionMode,
                                                        aArgs,
                                                        aObjectArgs );
    }

    return xResult;

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
    uno::Sequence< beans::PropertyValue > aTempMedDescr( aMediaDescr );

    // check if there is FilterName and URL, URL must exist
    ::rtl::OUString aFilterName;
    ::rtl::OUString aURL;
    for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
        if ( aTempMedDescr[nInd].Name.equalsAscii( "FilterName" ) )
            aTempMedDescr[nInd].Value >>= aFilterName;
        else if ( aTempMedDescr[nInd].Name.equalsAscii( "URL" ) )
            aTempMedDescr[nInd].Value >>= aURL;

    if ( !aURL.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No URL for the link is provided!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );

    if ( !aFilterName.getLength() )
    {
        uno::Reference< document::XTypeDetection > xTypeDetection(
                m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
                uno::UNO_QUERY );

        if ( !xTypeDetection.is() )
            throw uno::RuntimeException(); // TODO

        ::rtl::OUString aTypeName = xTypeDetection->queryTypeByDescriptor( aTempMedDescr, sal_True );

        // TODO: in case of unexpected format we can try to create an OLE link;
        //       when we have different objects for unknown non-OLE formats
        //       we will need kind of typedetection to detect which type of the link we need;
        //       another way could be to provide ClassID to the call, but how
        //       can caller detect object type if factory can not?

        for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
            if ( aTempMedDescr[nInd].Name.equalsAscii( "FilterName" ) )
                aTempMedDescr[nInd].Value >>= aFilterName;
    }

    uno::Reference< uno::XInterface > xResult;

    if ( aFilterName.getLength() )
    {
        // find document service name
        //TODO: will use storages in future

        ::rtl::OUString aDocServiceName = GetDocServiceNameFromFilter( aFilterName, m_xFactory );
        if ( !aDocServiceName.getLength() )
            throw io::IOException(); // TODO: unexpected format

        uno::Reference< embed::XLinkFactory > xOOoLinkFact(
                                m_xFactory->createInstance(
                                        ::rtl::OUString::createFromAscii( "com.sun.star.embed.OOoEmbeddedObjectFactory" ) ),
                                uno::UNO_QUERY );
        if ( !xOOoLinkFact.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xOOoLinkFact->createInstanceLinkUserInit( GetClassIDFromServName( aDocServiceName ),
                                                         GetClassNameFromServName( aDocServiceName ),
                                                         xStorage,
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
            throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                                uno::Reference< uno::XInterface >(
                                                    reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                                3 );

        if ( !sEntName.getLength() )
            throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                                uno::Reference< uno::XInterface >(
                                                    reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                                4 );

        uno::Reference< embed::XLinkCreator > xLinkCreator(
                            m_xFactory->createInstance(
                                ::rtl::OUString::createFromAscii( "com.sun.star.embed.OLEEmbeddedObjectFactory" ) ),
                            uno::UNO_QUERY );
        if ( !xLinkCreator.is() )
            throw uno::RuntimeException(); // TODO:

        xResult = xLinkCreator->createInstanceLink( xStorage, sEntName, aTempMedDescr, lObjArgs );
    }

    return xResult;
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

