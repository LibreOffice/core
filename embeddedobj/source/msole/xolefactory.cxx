/*************************************************************************
 *
 *  $RCSfile: xolefactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:54:36 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif


#include "xolefactory.hxx"
#include "oleembobj.hxx"


using namespace ::com::sun::star;

// TODO: do not create OLE objects that represent OOo documents

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OleEmbeddedObjectFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.OLEEmbeddedObjectFactory");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OLEEmbeddedObjectFactory");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObjectFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OLEEmbeddedObjectFactory");
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
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
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
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            3 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
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
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >(
                                                reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >(
                                                reinterpret_cast< ::cppu::OWeakObject* >(this) ),
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

