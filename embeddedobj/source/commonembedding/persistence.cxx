/*************************************************************************
 *
 *  $RCSfile: persistence.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-10-27 12:57:29 $
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

#include <commonembobj.hxx>

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDVERBS_HPP_
#include <com/sun/star/embed/EmbedVerbs.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCEESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#include "convert.hxx"

using namespace ::com::sun::star;


//------------------------------------------------------
uno::Sequence< beans::PropertyValue > addAsTemplate( const uno::Sequence< beans::PropertyValue >& aOrig )
{
    sal_Bool bAsTemplateSet = sal_False;
    sal_Int32 nLength = aOrig.getLength();
    uno::Sequence< beans::PropertyValue > aResult( nLength );

    for ( sal_Int32 nInd = 0; nInd < nLength; nInd++ )
    {
        aResult[nInd].Name = aOrig[nInd].Name;
        if ( aResult[nInd].Name.equalsAscii( "AsTemplate" ) )
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
        aResult[nLength].Name = ::rtl::OUString::createFromAscii( "AsTemplate" );
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

    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
    uno::Reference < io::XStream > xTempStream = uno::Reference < io::XStream > (
                                                            xFactory->createInstance ( aServiceName ),
                                                            uno::UNO_QUERY );
    if ( xTempStream.is() )
    {
        uno::Reference < lang::XSingleServiceFactory > xStorageFactory(
                    xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.embed.StorageFactory" ) ),
                    uno::UNO_QUERY );

        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xTempStream;
        aArgs[1] <<= embed::ElementModes::ELEMENT_READWRITE;
        uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArgs ),
                                                        uno::UNO_QUERY );
        if ( !xTempStorage.is() )
            throw uno::RuntimeException(); // TODO:

        try
        {
            xStorage->copyToStorage( xTempStorage );
        } catch( uno::Exception& e )
        {
            throw embed::StorageWTException(
                        ::rtl::OUString::createFromAscii( "Can't copy storage!" ),
                        uno::Reference< uno::XInterface >(),
                        uno::makeAny( e ) );
        }

        try {
            uno::Reference< lang::XComponent > xComponent( xTempStorage, uno::UNO_QUERY );
            OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( uno::Exception& )
        {
        }

        try {
            uno::Reference< io::XOutputStream > xTempOut = xTempStream->getOutputStream();
            if ( xTempOut.is() )
                xTempOut->closeOutput();
        }
        catch ( uno::Exception& )
        {
        }

        xResult = xTempStream->getInputStream();
    }

    return xResult;

}

//------------------------------------------------------
uno::Reference< frame::XModel > OCommonEmbeddedObject::InitNewDocument_Impl()
{
    uno::Reference< frame::XModel > xDocument( m_xFactory->createInstance( GetDocumentServiceName() ),
                                                uno::UNO_QUERY );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    try
    {
        xLoadable->initNew();
    }
    catch( uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< frame::XModel > OCommonEmbeddedObject::LoadLink_Impl()
{
    uno::Reference< frame::XModel > xDocument( m_xFactory->createInstance( GetDocumentServiceName() ),
                                                uno::UNO_QUERY );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    uno::Sequence< beans::PropertyValue > aArgs( 2 );
    aArgs[0].Name = ::rtl::OUString::createFromAscii( "URL" );
    aArgs[0].Value <<= m_aLinkURL;
    aArgs[1].Name = ::rtl::OUString::createFromAscii( "FilterName" );
    aArgs[1].Value <<= m_aLinkFilterName;

    try
    {
        xLoadable->load( aArgs );
    }
    catch( uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;

}

//------------------------------------------------------
uno::Reference< frame::XModel > OCommonEmbeddedObject::LoadDocumentFromStorage_Impl(
                                                            const uno::Reference< embed::XStorage >& xStorage )
{
    OSL_ENSURE( xStorage.is(), "The storage can not be empty!" );

    uno::Reference< frame::XModel > xDocument( m_xFactory->createInstance( GetDocumentServiceName() ),
                                                uno::UNO_QUERY );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xTempInpStream = createTempInpStreamFromStor( xStorage, m_xFactory );
    if ( !xTempInpStream.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aFilterName = GetDefaultFilterFromServName( GetDocumentServiceName() );
    OSL_ENSURE( aFilterName.getLength(), "Wrong document service name!" );
    if ( !aFilterName.getLength() )
        throw uno::RuntimeException(); // this point should not be reachable

    uno::Sequence< beans::PropertyValue > aArgs( 5 );
    aArgs[0].Name = ::rtl::OUString::createFromAscii( "URL" );
    aArgs[0].Value <<= ::rtl::OUString::createFromAscii( "private:stream" );
    aArgs[1].Name = ::rtl::OUString::createFromAscii( "InputStream" );
    aArgs[1].Value <<= xTempInpStream;
    aArgs[2].Name = ::rtl::OUString::createFromAscii( "ReadOnly" );
    aArgs[2].Value <<= m_bReadOnly;
    aArgs[3].Name = ::rtl::OUString::createFromAscii( "FilterName" );
    aArgs[3].Value <<= aFilterName;
    aArgs[4].Name = ::rtl::OUString::createFromAscii( "AsTemplate" );
    aArgs[4].Value <<= sal_True;

    try
    {
        xLoadable->load( aArgs );
    }
    catch( uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< io::XInputStream > OCommonEmbeddedObject::StoreDocumentToTempStream_Impl()
{
    uno::Reference < io::XOutputStream > xTempOut(
                m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                uno::UNO_QUERY );
    uno::Reference< io::XInputStream > aResult( xTempOut, uno::UNO_QUERY );

    if ( !xTempOut.is() || !aResult.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if( !xStorable.is() )
        throw uno::RuntimeException(); // TODO:

    ::rtl::OUString aFilterName = GetDefaultFilterFromServName( GetDocumentServiceName() );
    OSL_ENSURE( aFilterName.getLength(), "Wrong document service name!" );

    uno::Sequence< beans::PropertyValue > aArgs( 2 );
    aArgs[0].Name = ::rtl::OUString::createFromAscii( "FilterName" );
    aArgs[0].Value <<= aFilterName;
    aArgs[1].Name = ::rtl::OUString::createFromAscii( "OutputStream" );
    aArgs[1].Value <<= xTempOut;

    xStorable->storeToURL( ::rtl::OUString::createFromAscii( "private:stream" ), aArgs );
    try
    {
        xTempOut->closeOutput();
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Looks like stream was closed already" );
    }

    return aResult;
}

//------------------------------------------------------
void OCommonEmbeddedObject::SaveObject_Impl()
{
    if ( m_xClientSite.is() )
    {
        try {
            m_xClientSite->saveObject();
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "The object was not stored!\n" );
        }
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::StoreDocToStorage_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    OSL_ENSURE( xStorage.is(), "No storage is provided for storing!" );

    if ( !xStorage.is() )
        throw uno::RuntimeException(); // TODO:

    // store document to temporary stream based on temporary file
    uno::Reference < io::XInputStream > xTempIn = StoreDocumentToTempStream_Impl();
    OSL_ENSURE( xTempIn.is(), "The stream reference can not be empty!\n" );

    // open storage based on document temporary file for reading
    uno::Reference < lang::XSingleServiceFactory > xStorageFactory(
                m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.embed.StorageFactory" ) ),
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

//------------------------------------------------------
uno::Reference< frame::XModel > OCommonEmbeddedObject::CreateDocFromMediaDescr_Impl(
                                        const uno::Sequence< beans::PropertyValue >& aMedDescr )
{
    uno::Reference< frame::XModel > xDocument( m_xFactory->createInstance( GetDocumentServiceName() ),
                                                uno::UNO_QUERY );

    uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY );
    if ( !xLoadable.is() )
        throw uno::RuntimeException();

    try
    {
        xLoadable->load( addAsTemplate( aMedDescr ) );
    }
    catch( uno::Exception& )
    {
        uno::Reference< util::XCloseable > xCloseable( xDocument, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {
            }
        }

        throw; // TODO
    }

    return xDocument;
}

//------------------------------------------------------
uno::Reference< frame::XModel > OCommonEmbeddedObject::CreateTempDocFromLink_Impl()
{
    uno::Reference< frame::XModel > xResult;

    OSL_ENSURE( m_bIsLink, "The object is not a linked one!\n" );

    uno::Sequence< beans::PropertyValue > aTempMediaDescr;

    if ( m_pDocHolder->GetDocument().is() )
    {
        aTempMediaDescr.realloc( 4 );
        aTempMediaDescr[0].Name = ::rtl::OUString::createFromAscii( "URL" );
        aTempMediaDescr[0].Value <<= ::rtl::OUString::createFromAscii( "private:stream" );
        aTempMediaDescr[1].Name = ::rtl::OUString::createFromAscii( "InputStream" );
        aTempMediaDescr[1].Value <<= StoreDocumentToTempStream_Impl();
        aTempMediaDescr[2].Name = ::rtl::OUString::createFromAscii( "FilterName" );
        aTempMediaDescr[2].Value <<= GetDefaultFilterFromServName( GetDocumentServiceName() );
        aTempMediaDescr[3].Name = ::rtl::OUString::createFromAscii( "AsTemplate" );
        aTempMediaDescr[3].Value <<= sal_True;
    }
    else
    {
        aTempMediaDescr.realloc( 3 );
        aTempMediaDescr[0].Name = ::rtl::OUString::createFromAscii( "URL" );
        aTempMediaDescr[0].Value <<= m_aLinkURL;
        aTempMediaDescr[1].Name = ::rtl::OUString::createFromAscii( "FilterName" );
        aTempMediaDescr[1].Value <<= m_aLinkFilterName;
        aTempMediaDescr[2].Name = ::rtl::OUString::createFromAscii( "AsTemplate" );
        aTempMediaDescr[2].Value <<= sal_True;
    }

    xResult = CreateDocFromMediaDescr_Impl( aTempMediaDescr );

    return xResult;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& lArguments )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // the type of the object must be already set
    // a kind of typedetection should be done in the factory

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "No parent storage is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( !sEntName.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Empty element name is provided!\n" ),
                                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    // May be EMBED_LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::ENTRY_NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::ENTRY_NO_INIT ) )
    {
        // if the object is not loaded
        // it can not get persistant representation without initialization

        // if the object is loaded
        // it can switch persistant representation only without initialization

        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "Can't change persistant representation of activated object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY );
    if ( !xNameAccess.is() )
        throw uno::RuntimeException(); //TODO

    // detect entry existence
    sal_Bool bElExists = xNameAccess->hasByName( sEntName );

    m_bReadOnly = sal_False;
    for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name.equalsAscii( "ReadOnly" ) )
            lArguments[nInd].Value >>= m_bReadOnly;

    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::ELEMENT_READ : embed::ElementModes::ELEMENT_READWRITE;

    m_xObjectStorage = xStorage->openStorageElement( sEntName, nStorageMode );
    m_xParentStorage = xStorage;
    m_aEntryName = sEntName;

    // the object should be based on the storage ??? TODO
    if ( bElExists && !xStorage->isStorageElement( sEntName ) )
        throw io::IOException(); // TODO access denied

    if ( nEntryConnectionMode == embed::EntryInitModes::ENTRY_DEFAULT_INIT )
    {
        if ( bElExists )
        {
            // the initialization from existing storage allows to leave object in loaded state
            m_nObjectState = embed::EmbedStates::EMBED_LOADED;
        }
        else
        {
            m_pDocHolder->SetDocument( InitNewDocument_Impl(), m_bReadOnly );
            if ( !m_pDocHolder->GetDocument().is() )
                throw io::IOException(); // TODO: can not create document

            m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
        }
    }
    else
    {
        if ( ( nStorageMode & embed::ElementModes::ELEMENT_READWRITE ) != embed::ElementModes::ELEMENT_READWRITE )
            throw io::IOException();

        if ( nEntryConnectionMode == embed::EntryInitModes::ENTRY_NO_INIT )
        {
            // the document just already changed its storage to store to
            if ( m_bIsLink )
            {
                // for linked object it means that it becomes embedded object
                // the document must switch it's persistence also

                // the document is a new embedded object so it must be marked as modified
                uno::Reference< frame::XModel > xDocument = CreateTempDocFromLink_Impl();
                uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
                if ( !xModif.is() )
                    throw uno::RuntimeException();
                try
                {
                    xModif->setModified( sal_True );
                }
                catch( uno::Exception& )
                {}

                m_pDocHolder->SetDocument( xDocument, m_bReadOnly );
                OSL_ENSURE( m_pDocHolder->GetDocument().is(), "If document cant be created, an exception must be thrown!\n" );

                if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
                    m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
                else if ( m_nObjectState == embed::EmbedStates::EMBED_ACTIVE )
                    m_pDocHolder->Show();

                m_bIsLink = sal_False;
                m_aLinkFilterName = ::rtl::OUString();
                m_aLinkURL = ::rtl::OUString();

            }
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::ENTRY_TRUNCATE_INIT )
        {
            // TODO:
            m_pDocHolder->SetDocument( InitNewDocument_Impl(), m_bReadOnly );

            if ( !m_pDocHolder->GetDocument().is() )
                throw io::IOException(); // TODO: can not create document

            m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::ENTRY_MEDIA_DESCRIPTOR_INIT )
        {
            m_pDocHolder->SetDocument( CreateDocFromMediaDescr_Impl( lArguments ), m_bReadOnly );
            m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::ENTRY_TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong connection mode is provided!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );
    }
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeOwn()
        throw ( embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // during switching from Activated to Running and from Running to Loaded states the object will
    // ask container to store the object, the container has to make decision
    // to do so or not

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_bReadOnly )
        throw io::IOException(); // TODO: access denied

    if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        return; // nothing to do, the object is in loaded state

    OSL_ENSURE( m_pDocHolder->GetDocument().is(), "If an object is activated or in running state it must have a document!\n" );
    if ( !m_pDocHolder->GetDocument().is() )
        throw uno::RuntimeException();

    if ( m_bIsLink )
    {
        // TODO: just store the document to it's location
        uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
        if ( !xStorable.is() )
            throw uno::RuntimeException(); // TODO

        xStorable->store();
    }
    else
    {
        OSL_ENSURE( m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

        if ( !m_xObjectStorage.is() )
            throw io::IOException(); //TODO: access denied

        StoreDocToStorage_Impl( m_xObjectStorage );
    }

    // TODO:
    // notify listeners
    if ( m_nUpdateMode == embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
    {
        // TODO: update visual representation
    }
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( m_bIsLink || m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

    uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::ELEMENT_READWRITE );

    if ( !xSubStorage.is() )
        throw uno::RuntimeException(); //TODO

    StoreDocToStorage_Impl( xSubStorage );

    // TODO: should the listener notification be done?
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( m_bIsLink || m_xParentStorage.is() && m_xObjectStorage.is(), "The object has no valid persistence!\n" );

    uno::Reference< embed::XStorage > xSubStorage =
                xStorage->openStorageElement( sEntName, embed::ElementModes::ELEMENT_READWRITE );

    if ( !xSubStorage.is() )
        throw uno::RuntimeException(); //TODO

    StoreDocToStorage_Impl( xSubStorage );

    m_bWaitSaveCompleted = sal_True;
    m_xNewObjectStorage = xSubStorage;
    m_xNewParentStorage = xStorage;
    m_aEntryName = sEntName;

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
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Can't store object without persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    OSL_ENSURE( m_bWaitSaveCompleted, "Unexpected saveCompleted() call!\n" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewObjectStorage.is() && m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewObjectStorage.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        // the link object is not linked any more
        m_bIsLink = sal_False;
        m_aLinkURL = ::rtl::OUString();

        try {
            uno::Reference< lang::XComponent > xComponent( m_xObjectStorage, uno::UNO_QUERY );
            OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( uno::Exception& )
        {
        }

        m_xObjectStorage = m_xNewObjectStorage;
        m_xParentStorage = m_xNewParentStorage;
        m_aEntryName = m_aNewEntryName;
    }

    m_xNewObjectStorage = uno::Reference< embed::XStorage >();
    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = ::rtl::OUString();
    m_bWaitSaveCompleted = sal_False;

    if ( bUseNew )
    {
        // TODO: notify listeners

        if ( m_nUpdateMode == embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
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
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

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
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aEntryName;
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
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_bReadOnly;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object persistence is not initialized!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO:
    // throw away current document
    // load new document from current storage
    // use meaningfull part of lArguments
}

//------------------------------------------------------
sal_Bool SAL_CALL  OCommonEmbeddedObject::isLink()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_bIsLink;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OCommonEmbeddedObject::getLinkURL()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_bIsLink )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object is not a link object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aLinkURL;
}

