/*************************************************************************
 *
 *  $RCSfile: miscobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-04 14:30:19 $
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
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.h>

#include "closepreventer.hxx"
#include "intercept.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------
OCommonEmbeddedObject::OCommonEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                                const uno::Sequence< sal_Int8 >& aClassID,
                                                const ::rtl::OUString& aClassName,
                                                const ::rtl::OUString& aDocServiceName )
: m_pDocHolder( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_aClassID( aClassID )
, m_aClassName( aClassName )
, m_aDocServiceName( aDocServiceName )
, m_aAcceptedStates( NUM_SUPPORTED_STATES )
, m_aVerbTable( NUM_SUPPORTED_VERBS )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_False )
{
    CommonInit_Impl();
}

//------------------------------------------------------
OCommonEmbeddedObject::OCommonEmbeddedObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
        const ::rtl::OUString& aClassName,
        const ::rtl::OUString& aDocServiceName,
        const ::rtl::OUString& aLinkFilterName,
        const ::rtl::OUString& aLinkURL )
: m_pDocHolder( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( embed::EmbedStates::EMBED_LOADED )
, m_nUpdateMode ( embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
, m_xFactory( xFactory )
, m_aClassID( aClassID )
, m_aClassName( aClassName )
, m_aDocServiceName( aDocServiceName )
, m_aAcceptedStates( NUM_SUPPORTED_STATES )
, m_aVerbTable( NUM_SUPPORTED_VERBS )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_True )
, m_aLinkFilterName( aLinkFilterName )
, m_aLinkURL( aLinkURL )
{
    OSL_ENSURE( m_aLinkURL.getLength(), "The link for linked object must not be empty!\n" );

    // linked object has no persistence so it is in loaded state starting from creation
    CommonInit_Impl();
}

//------------------------------------------------------
void OCommonEmbeddedObject::CommonInit_Impl()
{
    OSL_ENSURE( m_xFactory.is(), "No ServiceFactory is provided!\n" );
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();

    m_pDocHolder = new DocumentHolder( m_xFactory, this );
    m_pDocHolder->acquire();

    // accepted states
    m_aAcceptedStates[0] = embed::EmbedStates::EMBED_LOADED;
    m_aAcceptedStates[1] = embed::EmbedStates::EMBED_RUNNING;
    m_aAcceptedStates[2] = embed::EmbedStates::EMBED_ACTIVE;

    // intermediate states
    m_pIntermediateStatesSeqs[0][2].realloc( 1 );
    m_pIntermediateStatesSeqs[0][2][0] = embed::EmbedStates::EMBED_RUNNING;

    m_pIntermediateStatesSeqs[2][0].realloc( 1 );
    m_pIntermediateStatesSeqs[2][0][0] = embed::EmbedStates::EMBED_RUNNING;

    // verbs table
    m_aVerbTable[0].realloc( 2 );
    m_aVerbTable[0][0] = embed::EmbedVerbs::MS_OLEVERB_PRIMARY;
    m_aVerbTable[0][1] = embed::EmbedStates::EMBED_ACTIVE;

    m_aVerbTable[1].realloc( 2 );
    m_aVerbTable[1][0] = embed::EmbedVerbs::MS_OLEVERB_SHOW;
    m_aVerbTable[1][1] = embed::EmbedStates::EMBED_ACTIVE;

    m_aVerbTable[2].realloc( 2 );
    m_aVerbTable[2][0] = embed::EmbedVerbs::MS_OLEVERB_OPEN;
    m_aVerbTable[2][1] = embed::EmbedStates::EMBED_ACTIVE;

    m_xClosePreventer = uno::Reference< util::XCloseListener >(
                                            static_cast< ::cppu::OWeakObject* >( new OClosePreventer() ),
                                            uno::UNO_QUERY );
}

//------------------------------------------------------
OCommonEmbeddedObject::~OCommonEmbeddedObject()
{
    if ( m_pInterfaceContainer )
    {
        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_pDocHolder )
    {
        m_pDocHolder->CloseFrame();
        m_pDocHolder->CloseDocument();
        m_pDocHolder->FreeOffice();

        m_pDocHolder->release();
        m_pDocHolder = NULL;
    }
}

//------------------------------------------------------
void OCommonEmbeddedObject::PostEvent_Impl( const ::rtl::OUString& aEventName )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_pInterfaceContainer->getContainer(
                                            ::getCppuType((const uno::Reference< document::XEventListener >*)0) );
        if( pIC )
        {
            document::EventObject aEvent( (embed::XEmbeddedObject*)this, aEventName );
            ::cppu::OInterfaceIteratorHelper aIt( *pIC );
            while( aIt.hasMoreElements() )
            {
                try
                {
                    ((document::XEventListener *)aIt.next())->notifyEvent( aEvent );
                }
                catch( uno::RuntimeException& )
                {
                    aIt.remove();
                }
            }
        }
    }
}

//------------------------------------------------------
uno::Any SAL_CALL OCommonEmbeddedObject::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    aReturn <<= ::cppu::queryInterface( rType,
                                        static_cast< embed::XEmbeddedObject* >( this ),
                                        static_cast< embed::XVisualObject* >( this ),
                                        static_cast< embed::XEmbedPersist* >( this ),
                                        static_cast< embed::XLinkageSupport* >( this ),
                                        static_cast< embed::XClassifiedObject* >( this ),
                                        static_cast< embed::XComponentSupplier* >( this ),
                                        static_cast< document::XEventBroadcaster* >( this ) );

    if ( aReturn.hasValue() )
        return aReturn;
    else
        return ::cppu::OWeakObject::queryInterface( rType ) ;

}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::acquire()
        throw()
{
    ::cppu::OWeakObject::acquire() ;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::release()
        throw()
{
    ::cppu::OWeakObject::release() ;
}

//------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OCommonEmbeddedObject::getTypes()
        throw( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;

    if ( !pTypeCollection )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pTypeCollection )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                                        ::getCppuType( (const uno::Reference< lang::XTypeProvider >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XEmbeddedObject >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XVisualObject >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XEmbedPersist >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XLinkageSupport >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XClassifiedObject >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XComponentSupplier >*)NULL ),
                                        ::getCppuType( (const uno::Reference< document::XEventBroadcaster >*)NULL ) );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;

}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getImplementationId()
        throw( uno::RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( !pID )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;
        if ( !pID )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getClassID()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassID;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OCommonEmbeddedObject::getClassName()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassName;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& aClassID, const ::rtl::OUString& aClassName )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}

//------------------------------------------------------
uno::Reference< lang::XComponent > SAL_CALL OCommonEmbeddedObject::getComponent()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // add an exception
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

    return uno::Reference< lang::XComponent >( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ), xListener );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ),
                                                xListener );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::dispose()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aEvent( (embed::XEmbeddedObject*)this );
        m_pInterfaceContainer->disposeAndClear( aEvent );

        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_pDocHolder )
    {
        m_pDocHolder->CloseFrame();
        m_pDocHolder->CloseDocument();
        m_pDocHolder->FreeOffice();

        m_pDocHolder->release();
        m_pDocHolder = NULL;
    }

    m_bDisposed = true;
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< lang::XEventListener >*)0 ), xListener );
}

//------------------------------------------------------
void SAL_CALL OCommonEmbeddedObject::removeEventListener(
                const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< lang::XEventListener >*)0 ),
                                                xListener );
}

