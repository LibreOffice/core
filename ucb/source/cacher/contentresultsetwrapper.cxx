/*************************************************************************
 *
 *  $RCSfile: contentresultsetwrapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:35 $
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

#include <contentresultsetwrapper.hxx>

#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_FETCHERROR_HPP_
#include <com/sun/star/ucb/FetchError.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace cppu;
using namespace rtl;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class ContentResultSetWrapper
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

ContentResultSetWrapper::ContentResultSetWrapper(
                                Reference< XResultSet > xOrigin )
                : m_xResultSetOrigin( xOrigin )
                , m_xRowOrigin( NULL )
                , m_xContentAccessOrigin( NULL )
                , m_xPropertySetOrigin( NULL )
                , m_xPropertySetInfo( NULL )
                , m_xMetaDataFromOrigin( NULL )

                , m_pDisposeEventListeners( NULL )
                , m_pPropertyChangeListeners( NULL )
                , m_pVetoableChangeListeners( NULL )

                , m_bDisposed( sal_False )
                , m_bInDispose( sal_False )

                , m_nForwardOnly( 2 )
{
    m_pMyListenerImpl = new ContentResultSetWrapperListener( this );
    m_xMyListenerImpl = Reference< XPropertyChangeListener >( m_pMyListenerImpl );

    DBG_ASSERT( m_xResultSetOrigin.is(), "XResultSet is required" );

    m_xRowOrigin = Reference< XRow >( m_xResultSetOrigin, UNO_QUERY );
    DBG_ASSERT( m_xRowOrigin.is(), "interface XRow is required" );

    m_xContentAccessOrigin = Reference< XContentAccess >( m_xResultSetOrigin, UNO_QUERY );
    DBG_ASSERT( m_xContentAccessOrigin.is(), "interface XContentAccess is required" );

    m_xPropertySetOrigin = Reference< XPropertySet >( m_xResultSetOrigin, UNO_QUERY );
    DBG_ASSERT( m_xPropertySetOrigin.is(), "interface XPropertySet is required" );

    //call impl_init() at the end of constructor of derived class
};

void SAL_CALL ContentResultSetWrapper::impl_init()
{
    //call this at the end of constructor of derived class
    //

    //listen to disposing from Origin:
    Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
    DBG_ASSERT( xComponentOrigin.is(), "interface XComponent is required" );
    xComponentOrigin->addEventListener( static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
}

ContentResultSetWrapper::~ContentResultSetWrapper()
{
    //call impl_deinit() at start of destructor of derived class

    delete m_pDisposeEventListeners;
    delete m_pPropertyChangeListeners;
    delete m_pVetoableChangeListeners;
};

void SAL_CALL ContentResultSetWrapper::impl_deinit()
{
    //call this at start of destructor of derived class
    //
    m_pMyListenerImpl->impl_OwnerDies();
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::impl_initPropertySetInfo()
{
    {
        vos::OGuard aGuard( m_aMutex );
        if( m_xPropertySetInfo.is() )
            return;

        if( !m_xPropertySetOrigin.is() )
            return;
    }

    Reference< XPropertySetInfo > xOrig =
            m_xPropertySetOrigin->getPropertySetInfo();

    {
        vos::OGuard aGuard( m_aMutex );
        m_xPropertySetInfo = xOrig;
    }
}

void SAL_CALL ContentResultSetWrapper
::impl_EnsureNotDisposed()
    throw( DisposedException, RuntimeException )
{
    vos::OGuard aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}

ContentResultSetWrapper::PropertyChangeListenerContainer_Impl* SAL_CALL
    ContentResultSetWrapper
    ::impl_getPropertyChangeListenerContainer()
{
    vos::OGuard aGuard( m_aMutex );
    if ( !m_pPropertyChangeListeners )
        m_pPropertyChangeListeners =
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex );
    return m_pPropertyChangeListeners;
}

ContentResultSetWrapper::PropertyChangeListenerContainer_Impl* SAL_CALL
    ContentResultSetWrapper
    ::impl_getVetoableChangeListenerContainer()
{
    vos::OGuard aGuard( m_aMutex );
    if ( !m_pVetoableChangeListeners )
        m_pVetoableChangeListeners =
            new PropertyChangeListenerContainer_Impl( m_aContainerMutex );
    return m_pVetoableChangeListeners;
}

void SAL_CALL ContentResultSetWrapper
    ::impl_notifyPropertyChangeListeners(
                    const PropertyChangeEvent& rEvt )
{
    {
        vos::OGuard aGuard( m_aMutex );
        if( !m_pPropertyChangeListeners )
            return;
    }

    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper* pContainer =
            m_pPropertyChangeListeners->getContainer( rEvt.PropertyName );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }

    // Notify listeners interested in all properties.
    pContainer = m_pPropertyChangeListeners->getContainer( OUString() );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XPropertyChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->propertyChange( rEvt );
        }
    }
}

void SAL_CALL ContentResultSetWrapper
    ::impl_notifyVetoableChangeListeners( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    {
        vos::OGuard aGuard( m_aMutex );
        if( !m_pVetoableChangeListeners )
            return;
    }

    // Notify listeners interested especially in the changed property.
    OInterfaceContainerHelper* pContainer =
            m_pVetoableChangeListeners->getContainer( rEvt.PropertyName );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XVetoableChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->vetoableChange( rEvt );
        }
    }

    // Notify listeners interested in all properties.
    pContainer = m_pVetoableChangeListeners->getContainer( OUString() );
    if( pContainer )
    {
        OInterfaceIteratorHelper aIter( *pContainer );
        while( aIter.hasMoreElements() )
        {
            Reference< XVetoableChangeListener > xListener(
                                                    aIter.next(), UNO_QUERY );
            if( xListener.is() )
                xListener->vetoableChange( rEvt );
        }
    }
}

sal_Bool SAL_CALL ContentResultSetWrapper
    ::impl_isForwardOnly()
{
    //m_nForwardOnly == 2 -> don't know
    //m_nForwardOnly == 1 -> YES
    //m_nForwardOnly == 0 -> NO

    //@todo replace this with lines in comment
    vos::OGuard aGuard( m_aMutex );
    m_nForwardOnly = 0;
    return m_nForwardOnly;


    /*
    ReacquireableGuard aGuard( m_aMutex );
    if( m_nForwardOnly == 2 )
    {
        aGuard.clear();
        if( !getPropertySetInfo().is() )
        {
            aGuard.reacquire();
            m_nForwardOnly = 0;
            return m_nForwardOnly;
        }
        aGuard.reacquire();

        rtl::OUString aName = OUString::createFromAscii( "ResultSetType" );
        //find out, if we are ForwardOnly and cache the value:

        if( !m_xPropertySetOrigin.is() )
        {
            DBG_ERROR( "broadcaster was disposed already" );
            m_nForwardOnly = 0;
            return m_nForwardOnly;
        }

        aGuard.clear();
        Any aAny = m_xPropertySetOrigin->getPropertyValue( aName );

        aGuard.reacquire();
        long nResultSetType;
        if( ( aAny >>= nResultSetType ) &&
            ( nResultSetType == ResultSetType::FORWARD_ONLY ) )
            m_nForwardOnly = 1;
        else
            m_nForwardOnly = 0;
    }
    return m_nForwardOnly;
    */
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
//list all interfaces inclusive baseclasses of interfaces
QUERYINTERFACE_IMPL_START( ContentResultSetWrapper )

    SAL_STATIC_CAST( XComponent*, this ),
    SAL_STATIC_CAST( XCloseable*, this ),
    SAL_STATIC_CAST( XResultSetMetaDataSupplier*, this ),
    SAL_STATIC_CAST( XPropertySet*, this ),

    SAL_STATIC_CAST( XContentAccess*, this ),
    SAL_STATIC_CAST( XResultSet*, this ),
    SAL_STATIC_CAST( XRow*, this )

QUERYINTERFACE_IMPL_END

//--------------------------------------------------------------------------
// XComponent methods.
//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::dispose() throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( m_bInDispose || m_bDisposed )
        return;
    m_bInDispose = sal_True;

    if( m_xPropertySetOrigin.is() )
    {
        aGuard.clear();
        try
        {
            m_xPropertySetOrigin->removePropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            DBG_ERROR( "could not remove PropertyChangeListener" );
        }
        try
        {
            m_xPropertySetOrigin->removeVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            DBG_ERROR( "could not remove VetoableChangeListener" );
        }

        Reference< XComponent > xComponentOrigin( m_xResultSetOrigin, UNO_QUERY );
        DBG_ASSERT( xComponentOrigin.is(), "interface XComponent is required" );
        xComponentOrigin->removeEventListener( static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
    }

    aGuard.reacquire();
    if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );

        aGuard.clear();
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    if( m_pPropertyChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        m_pPropertyChangeListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    if( m_pVetoableChangeListeners )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XPropertySet * >( this );

        aGuard.clear();
        m_pVetoableChangeListeners->disposeAndClear( aEvt );
    }

    aGuard.reacquire();
    m_bDisposed = sal_True;
    m_bInDispose = sal_False;
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::addEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    vos::OGuard aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_aContainerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::removeEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    vos::OGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}

//--------------------------------------------------------------------------
//XCloseable methods.
//--------------------------------------------------------------------------
//virtual
void SAL_CALL ContentResultSetWrapper
    ::close()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    dispose();
}

//--------------------------------------------------------------------------
//XResultSetMetaDataSupplier methods.
//--------------------------------------------------------------------------
//virtual
Reference< XResultSetMetaData > SAL_CALL ContentResultSetWrapper
    ::getMetaData()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    ReacquireableGuard aGuard( m_aMutex );
    if( !m_xMetaDataFromOrigin.is() && m_xResultSetOrigin.is() )
    {
        Reference< XResultSetMetaDataSupplier > xMetaDataSupplier
            = Reference< XResultSetMetaDataSupplier >(
                m_xResultSetOrigin, UNO_QUERY );

        if( xMetaDataSupplier.is() )
        {
            aGuard.clear();

            Reference< XResultSetMetaData > xMetaData
                = xMetaDataSupplier->getMetaData();

            aGuard.reacquire();
            m_xMetaDataFromOrigin = xMetaData;
        }
    }
    return m_xMetaDataFromOrigin;
}


//--------------------------------------------------------------------------
// XPropertySet methods.
//--------------------------------------------------------------------------
// virtual
Reference< XPropertySetInfo > SAL_CALL ContentResultSetWrapper
    ::getPropertySetInfo() throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    {
        vos::OGuard aGuard( m_aMutex );
        if( m_xPropertySetInfo.is() )
            return m_xPropertySetInfo;
    }
    impl_initPropertySetInfo();
    return m_xPropertySetInfo;
}
//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::setPropertyValue( const OUString& rPropertyName, const Any& rValue )
    throw( UnknownPropertyException,
           PropertyVetoException,
           IllegalArgumentException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xPropertySetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    m_xPropertySetOrigin->setPropertyValue( rPropertyName, rValue );
}

//--------------------------------------------------------------------------
// virtual
Any SAL_CALL ContentResultSetWrapper
    ::getPropertyValue( const OUString& rPropertyName )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xPropertySetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    return m_xPropertySetOrigin->getPropertyValue( rPropertyName );
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::addPropertyChangeListener(
            const OUString& aPropertyName,
            const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }

    if( aPropertyName.getLength() )
    {
        m_xPropertySetInfo->getPropertyByName( aPropertyName );
        //throws UnknownPropertyException, if so
    }

    impl_getPropertyChangeListenerContainer();
    BOOL bNeedRegister = !m_pPropertyChangeListeners->
                        getContainedTypes().getLength();
    m_pPropertyChangeListeners->addInterface( aPropertyName, xListener );
    if( bNeedRegister )
    {
        {
            vos::OGuard aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                DBG_ERROR( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->addPropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& rEx )
        {
            m_pPropertyChangeListeners->removeInterface( aPropertyName, xListener );
            throw rEx;
        }
    }
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::addVetoableChangeListener(
            const OUString& rPropertyName,
            const Reference< XVetoableChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !getPropertySetInfo().is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw UnknownPropertyException();
    }
    if( rPropertyName.getLength() )
    {
        m_xPropertySetInfo->getPropertyByName( rPropertyName );
        //throws UnknownPropertyException, if so
    }

    impl_getVetoableChangeListenerContainer();
    BOOL bNeedRegister = !m_pVetoableChangeListeners->
                        getContainedTypes().getLength();
    m_pVetoableChangeListeners->addInterface( rPropertyName, xListener );
    if( bNeedRegister )
    {
        {
            vos::OGuard aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                DBG_ERROR( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->addVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& rEx )
        {
            m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );
            throw rEx;
        }
    }
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    {
        //noop, if no listener registered
        vos::OGuard aGuard( m_aMutex );
        if( !m_pPropertyChangeListeners )
            return;
    }
    OInterfaceContainerHelper* pContainer =
        m_pPropertyChangeListeners->getContainer( rPropertyName );

    if( !pContainer )
    {
        if( rPropertyName.getLength() )
        {
            if( !getPropertySetInfo().is() )
                throw UnknownPropertyException();

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_pPropertyChangeListeners->removeInterface( rPropertyName, xListener );

    if( !m_pPropertyChangeListeners->getContainedTypes().getLength() )
    {
        {
            vos::OGuard aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                DBG_ERROR( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->removePropertyChangeListener(
                OUString(), static_cast< XPropertyChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            DBG_ERROR( "could not remove PropertyChangeListener" );
        }
    }
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL ContentResultSetWrapper
    ::removeVetoableChangeListener(
            const OUString& rPropertyName,
            const Reference< XVetoableChangeListener >& xListener )
    throw( UnknownPropertyException,
           WrappedTargetException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    {
        //noop, if no listener registered
        vos::OGuard aGuard( m_aMutex );
        if( !m_pVetoableChangeListeners )
            return;
    }
    OInterfaceContainerHelper* pContainer =
        m_pVetoableChangeListeners->getContainer( rPropertyName );

    if( !pContainer )
    {
        if( rPropertyName.getLength() )
        {
            if( !getPropertySetInfo().is() )
                throw UnknownPropertyException();

            m_xPropertySetInfo->getPropertyByName( rPropertyName );
            //throws UnknownPropertyException, if so
        }
        return; //the listener was not registered
    }

    m_pVetoableChangeListeners->removeInterface( rPropertyName, xListener );

    if( !m_pVetoableChangeListeners->getContainedTypes().getLength() )
    {
        {
            vos::OGuard aGuard( m_aMutex );
            if( !m_xPropertySetOrigin.is() )
            {
                DBG_ERROR( "broadcaster was disposed already" );
                return;
            }
        }
        try
        {
            m_xPropertySetOrigin->removeVetoableChangeListener(
                OUString(), static_cast< XVetoableChangeListener * >( m_pMyListenerImpl ) );
        }
        catch( Exception& )
        {
            DBG_ERROR( "could not remove VetoableChangeListener" );
        }
    }
}

//--------------------------------------------------------------------------
// own methods.
//--------------------------------------------------------------------------

//virtual
void SAL_CALL ContentResultSetWrapper
    ::impl_disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    vos::OGuard aGuard( m_aMutex );

    if( !m_xResultSetOrigin.is() )
        return;

    //release all references to the broadcaster:
    m_xResultSetOrigin.clear();
    m_xRowOrigin.clear();
    m_xContentAccessOrigin.clear();
    m_xPropertySetOrigin.clear();
    m_xMetaDataFromOrigin.clear();
    m_xPropertySetInfo.clear();
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::impl_propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = FALSE;
    impl_notifyPropertyChangeListeners( aEvt );
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::impl_vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    PropertyChangeEvent aEvt( rEvt );
    aEvt.Source = static_cast< XPropertySet * >( this );
    aEvt.Further = FALSE;

    impl_notifyVetoableChangeListeners( aEvt );
}

//--------------------------------------------------------------------------
// XContentAccess methods.  ( -- position dependent )
//--------------------------------------------------------------------------

// virtual
OUString SAL_CALL ContentResultSetWrapper
    ::queryContentIdentfierString()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xContentAccessOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContentIdentfierString();
}

//--------------------------------------------------------------------------
// virtual
Reference< XContentIdentifier > SAL_CALL ContentResultSetWrapper
    ::queryContentIdentifier()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xContentAccessOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContentIdentifier();
}

//--------------------------------------------------------------------------
// virtual
Reference< XContent > SAL_CALL ContentResultSetWrapper
    ::queryContent()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xContentAccessOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xContentAccessOrigin->queryContent();
}

//-----------------------------------------------------------------
// XResultSet methods.
//-----------------------------------------------------------------
//virtual

sal_Bool SAL_CALL ContentResultSetWrapper
    ::next()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->next();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::previous()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->previous();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::absolute( sal_Int32 row )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->absolute( row );
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::relative( sal_Int32 rows )
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->relative( rows );
}


//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::first()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->first();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::last()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->last();
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::beforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->beforeFirst();
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::afterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->afterLast();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::isAfterLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isAfterLast();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::isBeforeFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isBeforeFirst();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::isFirst()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isFirst();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::isLast()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->isLast();
}


//virtual
sal_Int32 SAL_CALL ContentResultSetWrapper
    ::getRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->getRow();
}

//virtual
void SAL_CALL ContentResultSetWrapper
    ::refreshRow()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    m_xResultSetOrigin->refreshRow();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowUpdated()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowUpdated();
}
//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowInserted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowInserted();
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::rowDeleted()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();

    if( !m_xResultSetOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xResultSetOrigin->rowDeleted();
}

//virtual
Reference< XInterface > SAL_CALL ContentResultSetWrapper
    ::getStatement()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    //@todo ?return anything
    return Reference< XInterface >();
}

//-----------------------------------------------------------------
// XRow methods.
//-----------------------------------------------------------------

#define XROW_GETXXX( getXXX )                       \
impl_EnsureNotDisposed();                               \
if( !m_xRowOrigin.is() )                            \
{                                                   \
    DBG_ERROR( "broadcaster was disposed already" );\
    throw RuntimeException();                       \
}                                                   \
return m_xRowOrigin->getXXX( columnIndex );

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::wasNull()
    throw( SQLException,
           RuntimeException )
{
    impl_EnsureNotDisposed();
    if( !m_xRowOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xRowOrigin->wasNull();
}

//virtual
rtl::OUString SAL_CALL ContentResultSetWrapper
    ::getString( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getString );
}

//virtual
sal_Bool SAL_CALL ContentResultSetWrapper
    ::getBoolean( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBoolean );
}

//virtual
sal_Int8 SAL_CALL ContentResultSetWrapper
    ::getByte( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getByte );
}

//virtual
sal_Int16 SAL_CALL ContentResultSetWrapper
    ::getShort( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getShort );
}

//virtual
sal_Int32 SAL_CALL ContentResultSetWrapper
    ::getInt( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getInt );
}

//virtual
sal_Int64 SAL_CALL ContentResultSetWrapper
    ::getLong( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getLong );
}

//virtual
float SAL_CALL ContentResultSetWrapper
    ::getFloat( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getFloat );
}

//virtual
double SAL_CALL ContentResultSetWrapper
    ::getDouble( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDouble );
}

//virtual
Sequence< sal_Int8 > SAL_CALL ContentResultSetWrapper
    ::getBytes( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBytes );
}

//virtual
Date SAL_CALL ContentResultSetWrapper
    ::getDate( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getDate );
}

//virtual
Time SAL_CALL ContentResultSetWrapper
    ::getTime( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTime );
}

//virtual
DateTime SAL_CALL ContentResultSetWrapper
    ::getTimestamp( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getTimestamp );
}

//virtual
Reference< com::sun::star::io::XInputStream >
    SAL_CALL ContentResultSetWrapper
    ::getBinaryStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBinaryStream );
}

//virtual
Reference< com::sun::star::io::XInputStream >
    SAL_CALL ContentResultSetWrapper
    ::getCharacterStream( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getCharacterStream );
}

//virtual
Any SAL_CALL ContentResultSetWrapper
    ::getObject( sal_Int32 columnIndex,
           const Reference<
            com::sun::star::container::XNameAccess >& typeMap )
    throw( SQLException,
           RuntimeException )
{
    //if you change this macro please pay attention to
    //define XROW_GETXXX, where this is similar implemented

    impl_EnsureNotDisposed();
    if( !m_xRowOrigin.is() )
    {
        DBG_ERROR( "broadcaster was disposed already" );
        throw RuntimeException();
    }
    return m_xRowOrigin->getObject( columnIndex, typeMap );
}

//virtual
Reference< XRef > SAL_CALL ContentResultSetWrapper
    ::getRef( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getRef );
}

//virtual
Reference< XBlob > SAL_CALL ContentResultSetWrapper
    ::getBlob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getBlob );
}

//virtual
Reference< XClob > SAL_CALL ContentResultSetWrapper
    ::getClob( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getClob );
}

//virtual
Reference< XArray > SAL_CALL ContentResultSetWrapper
    ::getArray( sal_Int32 columnIndex )
    throw( SQLException,
           RuntimeException )
{
    XROW_GETXXX( getArray );
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class ContentResultSetWrapperListener
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

ContentResultSetWrapperListener::ContentResultSetWrapperListener(
    ContentResultSetWrapper* pOwner )
    : m_pOwner( pOwner )
{
}

ContentResultSetWrapperListener::~ContentResultSetWrapperListener()
{
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
//list all interfaces inclusive baseclasses of interfaces
XINTERFACE_COMMON_IMPL( ContentResultSetWrapperListener )
QUERYINTERFACE_IMPL_START( ContentResultSetWrapperListener )

    static_cast< XEventListener * >(
                     static_cast< XPropertyChangeListener * >(this))
    , SAL_STATIC_CAST( XPropertyChangeListener*, this )
    , SAL_STATIC_CAST( XVetoableChangeListener*, this )

QUERYINTERFACE_IMPL_END


//--------------------------------------------------------------------------
//XEventListener methods.
//--------------------------------------------------------------------------

//virtual
void SAL_CALL ContentResultSetWrapperListener
    ::disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}

//--------------------------------------------------------------------------
//XPropertyChangeListener methods.
//--------------------------------------------------------------------------

//virtual
void SAL_CALL ContentResultSetWrapperListener
    ::propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_propertyChange( rEvt );
}

//--------------------------------------------------------------------------
//XVetoableChangeListener methods.
//--------------------------------------------------------------------------
//virtual
void SAL_CALL ContentResultSetWrapperListener
    ::vetoableChange( const PropertyChangeEvent& rEvt )
    throw( PropertyVetoException,
           RuntimeException )
{
    if( m_pOwner )
        m_pOwner->impl_vetoableChange( rEvt );
}

void SAL_CALL ContentResultSetWrapperListener
    ::impl_OwnerDies()
{
    m_pOwner = NULL;
}

