/*************************************************************************
 *
 *  $RCSfile: olepersist.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:54:13 $
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

#include <oleembobj.hxx>

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
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


#include <olecomponent.hxx>
#include <closepreventer.hxx>


using namespace ::com::sun::star;

//------------------------------------------------------
// TODO: probably later such a common function can be moved
//       to a separate helper library.

const sal_Int32 n_ConstBufferSize = 32000;
void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& aIn,
                             const uno::Reference< io::XOutputStream >& aOut )
{
    sal_Int32 nRead;
    uno::Sequence < sal_Int8 > aSequence ( n_ConstBufferSize );

    do
    {
        nRead = aIn->readBytes ( aSequence, n_ConstBufferSize );
        if ( nRead < n_ConstBufferSize )
        {
            uno::Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            aOut->writeBytes ( aTempBuf );
        }
        else
            aOut->writeBytes ( aSequence );
    }
    while ( nRead == n_ConstBufferSize );
}

//------------------------------------------------------
void SetStreamMediaType_Impl( const uno::Reference< io::XStream >& xStream, const ::rtl::OUString& aMediaType )
{
    uno::Reference< beans::XPropertySet > xPropSet( xStream, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        throw uno::RuntimeException(); // TODO: all the storage streams must support XPropertySet

    xPropSet->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), uno::makeAny( aMediaType ) );
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const uno::Reference< io::XStream >& xNewObjectStream,
                                              const ::rtl::OUString& aNewName )
{
    try {
        uno::Reference< lang::XComponent > xComponent( m_xObjectStream, uno::UNO_QUERY );
        OSL_ENSURE( !m_xObjectStream.is() || xComponent.is(), "Wrong stream implementation!" );
        if ( xComponent.is() )
            xComponent->dispose();
    }
    catch ( uno::Exception& )
    {
    }

    m_xObjectStream = xNewObjectStream;
    m_xParentStorage = xNewParentStorage;
    m_aEntryName = aNewName;
}

//------------------------------------------------------
void OleEmbeddedObject::SwitchOwnPersistence( const uno::Reference< embed::XStorage >& xNewParentStorage,
                                              const ::rtl::OUString& aNewName )
{
    sal_Int32 nStreamMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    uno::Reference< io::XStream > xNewOwnStream = xNewParentStorage->openStreamElement( aNewName, nStreamMode );
    OSL_ENSURE( xNewOwnStream.is(), "The method can not return empty reference!" );

    SwitchOwnPersistence( xNewParentStorage, xNewOwnStream, aNewName );
}


//----------------------------------------------
sal_Bool OleEmbeddedObject::SaveObject_Impl()
{
    sal_Bool bResult = sal_False;

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->saveObject();
            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
        }
    }

    return bResult;
}

//----------------------------------------------
sal_Bool OleEmbeddedObject::OnShowWindow_Impl( sal_Bool bShow )
{
    sal_Bool bResult = sal_False;

    OSL_ENSURE( m_nObjectState != -1, "The object has no persistence!\n" );
    OSL_ENSURE( m_nObjectState != embed::EmbedStates::LOADED, "The object get OnShowWindow in loaded state!\n" );
    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        return sal_False;

    // the object is either activated or deactivated
    if ( bShow && m_nObjectState == embed::EmbedStates::RUNNING )
        m_nObjectState = embed::EmbedStates::ACTIVE;
    else if ( !bShow && m_nObjectState == embed::EmbedStates::ACTIVE )
        m_nObjectState = embed::EmbedStates::RUNNING;

    if ( m_xClientSite.is() )
    {
        try
        {
            m_xClientSite->visibilityChanged( bShow );
            bResult = sal_True;
        }
        catch( uno::Exception& )
        {
        }
    }

    return bResult;
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponent_Impl( OleComponent* pOleComponent )
{
#ifdef WNT
    if ( !m_pOleComponent )
    {
        m_pOleComponent = pOleComponent ? pOleComponent : new OleComponent( m_xFactory, this );
        m_pOleComponent->acquire(); // TODO: needs holder?

        if ( !m_xClosePreventer.is() )
            m_xClosePreventer = uno::Reference< util::XCloseListener >(
                                    static_cast< ::cppu::OWeakObject* >( new OClosePreventer ),
                                    uno::UNO_QUERY );

        m_pOleComponent->addCloseListener( m_xClosePreventer );
    }
    else
        OSL_ENSURE( sal_False, "Trying to recreate OLE component!\n" );
#endif
}

//------------------------------------------------------
void OleEmbeddedObject::CreateOleComponentAndLoad_Impl( OleComponent* pOleComponent )
{
#ifdef WNT
    if ( !m_pOleComponent )
    {
        if ( !m_xObjectStream.is() )
            throw uno::RuntimeException();

        CreateOleComponent_Impl( NULL );
        // load object from the stream
        uno::Reference< io::XInputStream > xInStream = m_xObjectStream->getInputStream();
        if ( !xInStream.is() )
            throw io::IOException(); // TODO: access denied

        // after the loading the object can appear as a link
        // will be detected later by olecomponent
        m_pOleComponent->LoadEmbeddedObject( xInStream );
    }
#endif
}

//------------------------------------------------------
uno::Reference< io::XOutputStream > OleEmbeddedObject::GetStreamForSaving()
{
    if ( !m_xObjectStream.is() )
        throw uno::RuntimeException(); //TODO:

    uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
    if ( !xOutStream.is() )
        throw io::IOException(); //TODO: access denied

    uno::Reference< io::XTruncate > xTruncate( xOutStream, uno::UNO_QUERY );
    if ( !xTruncate.is() )
        throw uno::RuntimeException(); //TODO:

    xTruncate->truncate();

    return xOutStream;
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::setPersistentEntry(
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
    // TODO: use lObjArgs

    // the type of the object must be already set
    // a kind of typedetection should be done in the factory;
    // the only exception is object initialized from a stream,
    // the class ID will be detected from the stream

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

    // May be LOADED should be forbidden here ???
    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
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
    sal_Int32 nInd = 0;
    for ( nInd = 0; nInd < lArguments.getLength(); nInd++ )
        if ( lArguments[nInd].Name.equalsAscii( "ReadOnly" ) )
            lArguments[nInd].Value >>= m_bReadOnly;

    sal_Int32 nStorageMode = m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE;

    SwitchOwnPersistence( xStorage, sEntName );

    for ( nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
        if ( lObjArgs[nInd].Name.equalsAscii( "StoreVisualReplacement" ) )
            lObjArgs[nInd].Value >>= m_bStoreVisRepl;

#ifdef WNT
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT )
    {
        if ( bElExists )
        {
            // load object from the stream
            // after the loading the object can appear as a link
            // will be detected by olecomponent
            CreateOleComponentAndLoad_Impl( NULL );
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction

            m_nObjectState = embed::EmbedStates::LOADED;
        }
        else
        {
            // create a new object
            CreateOleComponent_Impl();
            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
    }
    else
    {
        if ( ( nStorageMode & embed::ElementModes::READWRITE ) != embed::ElementModes::READWRITE )
            throw io::IOException();

        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
        {
            // the document just already changed its stream to store to;
            // the links to OLE documents switch their persistence in the same way
            // as normal embedded objects
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::TRUNCATE_INIT )
        {
            // create a new object, that will be stored in specified stream
            CreateOleComponent_Impl();

            m_pOleComponent->CreateNewEmbeddedObject( m_aClassID );
            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        else if ( nEntryConnectionMode == embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT )
        {
            // use URL ( may be content or stream later ) from MediaDescriptor to initialize object
            ::rtl::OUString aURL;
            for ( sal_Int32 nInd = 0; nInd < lArguments.getLength(); nInd++ )
                if ( lArguments[nInd].Name.equalsAscii( "URL" ) )
                    lArguments[nInd].Value >>= aURL;

            if ( !aURL.getLength() )
                throw lang::IllegalArgumentException(
                                    ::rtl::OUString::createFromAscii( "Empty URL is provided in the media descriptor!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                    4 );

            CreateOleComponent_Impl();

            // TODO: the m_bIsLink value must be set already
            if ( !m_bIsLink )
                m_pOleComponent->CreateObjectFromFile( aURL );
            else
                m_pOleComponent->CreateLinkFromFile( aURL );

            m_pOleComponent->RunObject();
            m_aClassID = m_pOleComponent->GetCLSID(); // was not set during consruction

            m_nObjectState = embed::EmbedStates::RUNNING;
        }
        //else if ( nEntryConnectionMode == embed::EntryInitModes::TRANSFERABLE_INIT )
        //{
            //TODO:
        //}
        else
            throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong connection mode is provided!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                        3 );
    }
#else
    // On unix the ole object can not do anything except storing itself somewere
    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT && bElExists )
    {
        // TODO: detect classID of the object
        // can be a real problem for the links
    }
    else
        throw lang::IllegalArgumentException( ::rtl::OUString::createFromAscii( "Wrong connection mode is provided!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                                    3 );

#endif
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeOwn()
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

    if ( m_nObjectState == embed::EmbedStates::LOADED )
        return; // nothing to do, the object is in loaded state

#ifdef WNT
    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

    if ( !m_xObjectStream.is() )
        throw io::IOException(); //TODO: access denied

    SetStreamMediaType_Impl( m_xObjectStream, ::rtl::OUString::createFromAscii( "application/vnd.sun.star.oleobject" ) );
    uno::Reference< io::XOutputStream > xOutStream = m_xObjectStream->getOutputStream();
    if ( !xOutStream.is() )
        throw io::IOException(); //TODO: access denied

    if ( m_bIsLink )
    {
        // just let the link store itself
        // in case visual repersentation must be stored also
        // the procedure should be the same as for embedded objects

        uno::Reference< io::XOutputStream > xOutStream = GetStreamForSaving();

        // should the component detect that it is a link???
        m_pOleComponent->StoreObjectToStream( xOutStream, m_bStoreVisRepl );
    }
    else
    {
        uno::Reference< io::XOutputStream > xOutStream = GetStreamForSaving();
        m_pOleComponent->StoreObjectToStream( xOutStream, m_bStoreVisRepl );
    }

    // TODO:
    // notify listeners
    if ( m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE )
    {
        // TODO: update visual representation
    }
#endif
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs

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

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
    }
#ifdef WNT
    else
    {
        if ( !m_pOleComponent )
            throw uno::RuntimeException();

        uno::Reference< io::XStream > xTargetStream =
                xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
        if ( !xTargetStream.is() )
            throw io::IOException(); //TODO: access denied

        SetStreamMediaType_Impl( xTargetStream, ::rtl::OUString::createFromAscii( "application/vnd.sun.star.oleobject" ) );
        uno::Reference< io::XOutputStream > xOutStream = xTargetStream->getOutputStream();
        if ( !xOutStream.is() )
            throw io::IOException(); //TODO: access denied

        sal_Bool bStoreVis = m_bStoreVisRepl;
        for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
            if ( lObjArgs[nInd].Name.equalsAscii( "StoreVisualReplacement" ) )
                lObjArgs[nInd].Value >>= bStoreVis;

        m_pOleComponent->StoreObjectToStream( xOutStream, bStoreVis );

        uno::Reference< lang::XComponent > xComp( xTargetStream, uno::UNO_QUERY );
        if ( xComp.is() )
        {
            try {
                xComp->dispose();
            } catch( uno::Exception& )
            {
            }
        }
    }
#else
    else
        throw io::IOException(); // TODO
#endif

    // TODO: should the listener notification be done?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& lArguments,
                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs

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

    OSL_ENSURE( m_xParentStorage.is() && m_xObjectStream.is(), "The object has no valid persistence!\n" );

    uno::Reference< io::XStream > xTargetStream =
            xStorage->openStreamElement( sEntName, embed::ElementModes::READWRITE );
    if ( !xTargetStream.is() )
        throw io::IOException(); //TODO: access denied

    SetStreamMediaType_Impl( xTargetStream, ::rtl::OUString::createFromAscii( "application/vnd.sun.star.oleobject" ) );
    uno::Reference< io::XOutputStream > xOutStream = xTargetStream->getOutputStream();
    if ( !xOutStream.is() )
        throw io::IOException(); //TODO: access denied

    sal_Bool bStoreVis = m_bStoreVisRepl;
    for ( sal_Int32 nInd = 0; nInd < lObjArgs.getLength(); nInd++ )
        if ( lObjArgs[nInd].Name.equalsAscii( "StoreVisualReplacement" ) )
            lObjArgs[nInd].Value >>= bStoreVis;

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        if ( bStoreVis != m_bStoreVisRepl )
            throw lang::IllegalArgumentException(
                            ::rtl::OUString::createFromAscii( "Can't change store mode in loaded state!\n" ),
                            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
                            1 );

        uno::Reference< io::XInputStream > xInStream = m_xObjectStream->getInputStream();
        copyInputToOutput_Impl( xInStream, xOutStream );
    }
#ifdef WNT
    else
    {
        if ( !m_pOleComponent )
            throw uno::RuntimeException(); // TODO:

        m_pOleComponent->StoreObjectToStream( xOutStream, bStoreVis );
    }
#else
    else
        throw io::IOException(); // TODO
#endif

    m_bWaitSaveCompleted = sal_True;
    m_xNewObjectStream = xTargetStream;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
    m_bNewStoreVisRepl = bStoreVis;

    // TODO: register listeners for storages above, in case thay are disposed
    //       an exception will be thrown on saveCompleted( true )

    // TODO: should the listener notification be done here or in saveCompleted?
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::saveCompleted( sal_Bool bUseNew )
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

    OSL_ENSURE( m_xNewObjectStream.is() && m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewObjectStream.is() || !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        SwitchOwnPersistence( m_xNewParentStorage, m_xNewObjectStream, m_aNewEntryName );
        m_bStoreVisRepl = m_bNewStoreVisRepl;
    }
    else
    {
        // close remembered stream
        try {
            uno::Reference< lang::XComponent > xComponent( m_xNewObjectStream, uno::UNO_QUERY );
            OSL_ENSURE( xComponent.is(), "Wrong storage implementation!" );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( uno::Exception& )
        {
        }
    }

    m_xNewObjectStream = uno::Reference< io::XStream >();
    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = ::rtl::OUString();
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
sal_Bool SAL_CALL OleEmbeddedObject::hasEntry()
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

    if ( m_xObjectStream.is() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getEntryName()
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
sal_Bool SAL_CALL OleEmbeddedObject::isReadonly()
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
void SAL_CALL OleEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& lArguments,
                const uno::Sequence< beans::PropertyValue >& lObjArgs )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: use lObjArgs

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
void SAL_CALL OleEmbeddedObject::breakLink( const uno::Reference< embed::XStorage >& xStorage,
                                                const ::rtl::OUString& sEntName )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
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

    // TODO: The object must be at least in Running state;
    if ( !m_bIsLink || m_nObjectState == -1 || !m_pOleComponent )
    {
        // it must be a linked initialized object
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object is not a valid linked object!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }
#ifdef WNT
    if ( m_bReadOnly )
        throw io::IOException(); // TODO: Access denied

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString::createFromAscii( "The object waits for saveCompleted() call!\n" ),
                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );


    // TODO: if the parent storage and entry are the same as object uses the breaking of link must be done
    // just by through OLE functionality

    // TODO: create an object based on the link
    OleComponent* pNewOleComponent = new OleComponent( m_xFactory, this );
    try {
        pNewOleComponent->InitEmbeddedCopyOfLink( m_pOleComponent );
    }
    catch ( uno::Exception& )
    {
        delete pNewOleComponent;
        throw;
    }

    try {
        GetRidOfComponent();
    }
    catch( uno::Exception& )
    {
        delete pNewOleComponent;
        throw;
    }

    CreateOleComponent_Impl( pNewOleComponent );

    if ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) )
        SwitchOwnPersistence( xStorage, sEntName );

    if ( m_nObjectState != embed::EmbedStates::LOADED )
    {
        // TODO: should we activate the new object if the link was activated?

        sal_Int32 nTargetState = m_nObjectState;
        m_nObjectState = embed::EmbedStates::LOADED;

        if ( m_nObjectState == embed::EmbedStates::RUNNING )
            m_pOleComponent->RunObject();
        else // m_nObjectState == embed::EmbedStates::ACTIVE
        {
            m_pOleComponent->RunObject();
            m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
        }

        m_nObjectState = nTargetState;
    }

    m_bIsLink = sal_False;
    m_aLinkURL = ::rtl::OUString();
#else
    throw io::IOException(); //TODO:
#endif
}

//------------------------------------------------------
sal_Bool SAL_CALL  OleEmbeddedObject::isLink()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_bIsLink;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getLinkURL()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
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

    // TODO: probably the link URL can be retrieved from OLE

    return m_aLinkURL;
}

