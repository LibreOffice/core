/*************************************************************************
 *
 *  $RCSfile: olemisc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-18 09:03:55 $
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

#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif


#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#include <cppuhelper/interfacecontainer.h>

#include <oleembobj.hxx>
#include <olecomponent.hxx>

using namespace ::com::sun::star;

//------------------------------------------------------
OleEmbeddedObject::OleEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                      const uno::Sequence< sal_Int8 >& aClassID,
                                      const ::rtl::OUString& aClassName )
: m_pOleComponent( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
, m_bStoreVisRepl( sal_True )
, m_xFactory( xFactory )
, m_aClassID( aClassID )
, m_aClassName( aClassName )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_False )
{
}

//------------------------------------------------------
// In case of loading from persistent entry the classID of the object
// will be retrieved from the entry, during construction it is unknown
OleEmbeddedObject::OleEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_pOleComponent( NULL )
, m_pInterfaceContainer( NULL )
, m_bReadOnly( sal_False )
, m_bDisposed( sal_False )
, m_nObjectState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE )
, m_bStoreVisRepl( sal_True )
, m_xFactory( xFactory )
, m_bWaitSaveCompleted( sal_False )
, m_bIsLink( sal_False )
{
}


//------------------------------------------------------
OleEmbeddedObject::~OleEmbeddedObject()
{
    OSL_ENSURE( !m_pInterfaceContainer && !m_pOleComponent, "The object is not closed! DISASTER is possible!" );

    if ( m_pOleComponent || m_pInterfaceContainer )
    {
        // the component must be cleaned during closing
        m_refCount++; // to avoid crash
        try {
            Dispose();
        } catch( uno::Exception& ) {}
    }
}

//------------------------------------------------------
void OleEmbeddedObject::Dispose()
{
    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
        m_pInterfaceContainer->disposeAndClear( aSource );
        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_pOleComponent )
    {
        if ( m_nObjectState != embed::EmbedStates::EMBED_LOADED )
        {
            SaveObject_Impl();
            m_pOleComponent->CloseObject();
        }

        m_pOleComponent->removeCloseListener( m_xClosePreventer );
        m_pOleComponent->disconnectEmbeddedObject();
        m_pOleComponent->release();
        m_pOleComponent = NULL;
    }

    m_bDisposed = true;
}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL OleEmbeddedObject::getClassID()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassID;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL OleEmbeddedObject::getClassName()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_aClassName;
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& aClassID, const ::rtl::OUString& aClassName )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}

//------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL OleEmbeddedObject::getComponent()
        throw ( uno::RuntimeException )
{
    // TODO: The return type will be reference to XInterface

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

    if ( !m_pOleComponent )
        throw uno::RuntimeException(); // TODO

    return uno::Reference< util::XCloseable >( static_cast< ::cppu::OWeakObject* >( m_pOleComponent ), uno::UNO_QUERY );
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }
    }

    Dispose();
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ), xListener );
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ),
                                                xListener );
}

//------------------------------------------------------
void SAL_CALL OleEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
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
void SAL_CALL OleEmbeddedObject::removeEventListener(
                const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ),
                                                xListener );
}

