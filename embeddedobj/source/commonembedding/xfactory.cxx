/*************************************************************************
 *
 *  $RCSfile: xfactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:44:30 $
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


#include "xfactory.hxx"
#include "commonembobj.hxx"
#include "oleembobj.hxx"
#include "convert.hxx"


using namespace ::com::sun::star;

//-------------------------------------------------------------------------
::rtl::OUString GetDocServiceNameFromFilter( const ::rtl::OUString& aFilterName,
                                            const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    ::rtl::OUString aDocServiceName;

    uno::Reference< container::XNameAccess > xFilterFactory(
            xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
            uno::UNO_QUERY );

    if ( !xFilterFactory.is() )
        throw uno::RuntimeException(); // TODO

    uno::Any aFilterAnyData = xFilterFactory->getByName( aFilterName );
    uno::Sequence< beans::PropertyValue > aFilterData;
    if ( aFilterAnyData >>= aFilterData )
    {
        for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
            if ( aFilterData[nInd].Name.equalsAscii( "DocumentService" ) )
                aFilterData[nInd].Value >>= aDocServiceName;
    }

    return ConvertServiceToImplementationName( aDocServiceName );
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOoEmbeddedObjectFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.OOoEmbeddedObjectFactory");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OOoEmbeddedObjectFactory");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OOoEmbeddedObjectFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OOoEmbeddedObjectFactory");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OOoEmbeddedObjectFactory( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitFromEntry(
                                                                    const uno::Reference< embed::XStorage >& xStorage,
                                                                    const ::rtl::OUString& sEntName,
                                                                    const uno::Sequence< beans::PropertyValue >& aMediaDescr,
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
        xSubStorage = uno::Reference< embed::XStorage >();

        ::rtl::OUString aDocServiceName = GetDocumentServiceFromMediaType( aMediaType );
        if ( !aDocServiceName.getLength() )
        {
            // only own document can be based on storage
            // in case it is not possible to find related
            // document service name the storage entry is invalid

            throw io::IOException(); // unexpected mimetype of the storage
        }

        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                                m_xFactory,
                                                GetClassIDFromServName( aDocServiceName ),
                                                GetClassNameFromServName( aDocServiceName ),
                                                aDocServiceName ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported by own document objects

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::DEFAULT_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const ::rtl::OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
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
                    if ( aTypes[nInd].Name.equalsAscii("PreferredFilter") )
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

    // find document service name
    if ( aFilterName.getLength() )
    {
        ::rtl::OUString aDocServiceName = GetDocServiceNameFromFilter( aFilterName, m_xFactory );
        if ( !aDocServiceName.getLength() )
            throw io::IOException(); // TODO: filter detection failed

        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                            m_xFactory,
                                            GetClassIDFromServName( aDocServiceName ),
                                            GetClassNameFromServName( aDocServiceName ),
                                            aDocServiceName ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );

    if ( !xPersist.is() )
        throw uno::RuntimeException(); // TODO: the interface must be supported ( what about applets? )

    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aTempMedDescr,
                                    lObjArgs );

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceInitNew(
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
        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                                m_xFactory,
                                                aClassID,
                                                aClassName,
                                                aDocServiceName ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

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
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceUserInit(
            const uno::Sequence< sal_Int8 >& aClassID,
            const ::rtl::OUString& aClassName,
            const uno::Reference< embed::XStorage >& xStorage,
            const ::rtl::OUString& sEntName,
            sal_Int32 nEntryConnectionMode,
            const uno::Sequence< beans::PropertyValue >& lArguments,
            const uno::Sequence< beans::PropertyValue >& lObjArgs )
    throw ( lang::IllegalArgumentException,
            io::IOException,
            uno::Exception,
            uno::RuntimeException )
{
    // the initialization is completelly controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );


    ::rtl::OUString aDocServiceName = GetDocumentServiceNameFromClassID( aClassID );

    uno::Reference< uno::XInterface > xResult;
    if ( aDocServiceName.getLength() )
    {
        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                                m_xFactory,
                                                aClassID,
                                                aClassName,
                                                aDocServiceName ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );
    if ( xPersist.is() )
    {
        xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    nEntryConnectionMode,
                                    lArguments,
                                    lObjArgs );

    }
    else
        throw uno::RuntimeException(); // TODO:

    return xResult;
}


//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceLink(
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

        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                            m_xFactory,
                                            GetClassIDFromServName( aDocServiceName ),
                                            GetClassNameFromServName( aDocServiceName ),
                                            aDocServiceName,
                                            aFilterName,
                                            aURL ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    // TODO: set persistence and use lObjArgs

    return xResult;
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OOoEmbeddedObjectFactory::createInstanceLinkUserInit(
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
    // the initialization is completelly controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    uno::Sequence< beans::PropertyValue > aTempMedDescr( lArguments );

    ::rtl::OUString aURL;
    ::rtl::OUString aFilterName;
    for ( sal_Int32 nInd = 0; nInd < aTempMedDescr.getLength(); nInd++ )
        if ( aTempMedDescr[nInd].Name.equalsAscii( "FilterName" ) )
            aTempMedDescr[nInd].Value >>= aFilterName;
        else if ( aTempMedDescr[nInd].Name.equalsAscii( "URL" ) )
            aTempMedDescr[nInd].Value >>= aURL;

    if ( !aURL.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No URL for the link is provided!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );

    ::rtl::OUString aDocServiceName = GetDocumentServiceNameFromClassID( aClassID );
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
    }

    uno::Reference< uno::XInterface > xResult;
    if ( aDocServiceName.getLength() && aFilterName.getLength() )
    {
        xResult = uno::Reference< uno::XInterface >(
                    static_cast< ::cppu::OWeakObject* > ( new OCommonEmbeddedObject(
                                            m_xFactory,
                                            aClassID,
                                            aClassName,
                                            aDocServiceName,
                                            aFilterName,
                                            aURL ) ),
                    uno::UNO_QUERY );
    }
    else
    {
        // the object must be OOo embedded object, if it is not an exception must be thrown
        throw io::IOException(); // TODO:
    }

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY );
    if ( xPersist.is() )
    {
        xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    uno::Sequence< beans::PropertyValue >(),
                                    lObjArgs );

    }
    else
        throw uno::RuntimeException(); // TODO:

    return xResult;

}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OOoEmbeddedObjectFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OOoEmbeddedObjectFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OOoEmbeddedObjectFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

