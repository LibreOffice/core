/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <svtools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::IllegalAccessException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::sdb::XDatabaseRegistrations;
    using ::com::sun::star::sdb::XDatabaseRegistrationsListener;
    using ::com::sun::star::sdb::DatabaseRegistrationEvent;
    using ::com::sun::star::uno::XAggregation;
    /** === end UNO using === **/

    //--------------------------------------------------------------------
    static const ::rtl::OUString& getConfigurationRootPath()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("org.openoffice.Office.DataAccess/RegisteredNames");
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    const ::rtl::OUString& getLocationNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii( "Location" );
        return s_sNodeName;
    }

    //--------------------------------------------------------------------
    const ::rtl::OUString& getNameNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii( "Name" );
        return s_sNodeName;
    }

    //====================================================================
    //= DatabaseRegistrations - declaration
    //====================================================================
    typedef ::cppu::WeakAggImplHelper1  <   XDatabaseRegistrations
                                        >   DatabaseRegistrations_Base;
    class DatabaseRegistrations :public ::cppu::BaseMutex
                                ,public DatabaseRegistrations_Base
    {
    public:
        DatabaseRegistrations( const ::comphelper::ComponentContext& _rxContext );

    protected:
        ~DatabaseRegistrations();

    public:
        virtual ::sal_Bool SAL_CALL hasRegisteredDatabase( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getRegistrationNames() throw (RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDatabaseLocation( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, RuntimeException);
        virtual void SAL_CALL registerDatabaseLocation( const ::rtl::OUString& _Name, const ::rtl::OUString& _Location ) throw (IllegalArgumentException, ElementExistException, RuntimeException);
        virtual void SAL_CALL revokeDatabaseLocation( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, IllegalAccessException, RuntimeException);
        virtual void SAL_CALL changeDatabaseLocation( const ::rtl::OUString& Name, const ::rtl::OUString& NewLocation ) throw (IllegalArgumentException, NoSuchElementException, IllegalAccessException, RuntimeException);
        virtual ::sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, RuntimeException);
        virtual void SAL_CALL addDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener ) throw (RuntimeException);
        virtual void SAL_CALL removeDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener ) throw (RuntimeException);

    private:
        void    impl_checkValidName_throw( const ::rtl::OUString& _rName, const bool _bMustExist );
        void    impl_checkValidLocation_throw( const ::rtl::OUString& _rLocation );

    private:
        ::comphelper::ComponentContext      m_aContext;
        ::utl::OConfigurationTreeRoot       m_aConfigurationRoot;
        ::cppu::OInterfaceContainerHelper   m_aRegistrationListeners;
    };

    //====================================================================
    //= DatabaseRegistrations - implementation
    //====================================================================
    //--------------------------------------------------------------------
    DatabaseRegistrations::DatabaseRegistrations( const ::comphelper::ComponentContext& _rxContext )
        :m_aContext( _rxContext )
        ,m_aConfigurationRoot()
        ,m_aRegistrationListeners( m_aMutex )
    {
        m_aConfigurationRoot = ::utl::OConfigurationTreeRoot::createWithServiceFactory(
            m_aContext.getLegacyServiceFactory(), getConfigurationRootPath(), -1, ::utl::OConfigurationTreeRoot::CM_UPDATABLE );
    }

    //--------------------------------------------------------------------
    DatabaseRegistrations::~DatabaseRegistrations()
    {
    }

    //--------------------------------------------------------------------
    void DatabaseRegistrations::impl_checkValidName_throw( const ::rtl::OUString& _rName, const bool _bMustExist )
    {
        if ( !m_aConfigurationRoot.isValid() )
            throw RuntimeException( ::rtl::OUString(), *this );

        if ( !_rName.getLength() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

        if ( _bMustExist && !m_aConfigurationRoot.hasByName( _rName ) )
            throw NoSuchElementException( _rName, *this );

        if ( !_bMustExist && m_aConfigurationRoot.hasByName( _rName ) )
            throw ElementExistException( _rName, *this );
    }

    //--------------------------------------------------------------------
    void DatabaseRegistrations::impl_checkValidLocation_throw( const ::rtl::OUString& _rLocation )
    {
        if ( !_rLocation.getLength() )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );

        INetURLObject aURL( _rLocation );
        if ( aURL.GetProtocol() == INET_PROT_NOT_VALID )
            throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DatabaseRegistrations::hasRegisteredDatabase( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_aConfigurationRoot.isValid() && m_aConfigurationRoot.hasByName( _Name );
    }

    //------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL DatabaseRegistrations::getRegistrationNames() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_aConfigurationRoot.isValid() )
            throw RuntimeException( ::rtl::OUString(), *this );

        Sequence< ::rtl::OUString > aNames( m_aConfigurationRoot.getNodeNames() );
        return aNames;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL DatabaseRegistrations::getDatabaseLocation( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkValidName_throw( _Name, true );

        ::rtl::OUString sLocation;

        ::utl::OConfigurationNode aRegisterObj( m_aConfigurationRoot.openNode( _Name ) );
        OSL_VERIFY( aRegisterObj.getNodeValue( getLocationNodeName() ) >>= sLocation );
        sLocation = SvtPathOptions().SubstituteVariable( sLocation );

        return sLocation;
    }

    //--------------------------------------------------------------------
    void SAL_CALL DatabaseRegistrations::registerDatabaseLocation( const ::rtl::OUString& _Name, const ::rtl::OUString& _Location ) throw (IllegalArgumentException, ElementExistException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        impl_checkValidName_throw( _Name, false );
        impl_checkValidLocation_throw( _Location );

        // register
        ::utl::OConfigurationNode aDataSourceRegistration = m_aConfigurationRoot.createNode( _Name );

        aDataSourceRegistration.setNodeValue( getNameNodeName(), makeAny( _Name ) );
        aDataSourceRegistration.setNodeValue( getLocationNodeName(), makeAny( _Location ) );

        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, _Name, ::rtl::OUString(), _Location );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::registeredDatabaseLocation, aEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DatabaseRegistrations::revokeDatabaseLocation( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, IllegalAccessException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        impl_checkValidName_throw( _Name, true );

        const ::rtl::OUString sLocation( getDatabaseLocation( _Name ) );

        // revoke
        if  (   isDatabaseRegistrationReadOnly( _Name )
            ||  !m_aConfigurationRoot.removeNode( _Name )
            )
            throw IllegalAccessException( ::rtl::OUString(), *this );

        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, _Name, sLocation, ::rtl::OUString() );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::revokedDatabaseLocation, aEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DatabaseRegistrations::changeDatabaseLocation( const ::rtl::OUString& _Name, const ::rtl::OUString& _NewLocation ) throw (IllegalArgumentException, NoSuchElementException, IllegalAccessException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        impl_checkValidName_throw( _Name, true );
        impl_checkValidLocation_throw( _NewLocation );

        if  ( isDatabaseRegistrationReadOnly( _Name ) )
            throw IllegalAccessException( ::rtl::OUString(), *this );

        const ::rtl::OUString sOldLocation( getDatabaseLocation( _Name ) );

        // change
        ::utl::OConfigurationNode aDataSourceRegistration = m_aConfigurationRoot.openNode( _Name );
        aDataSourceRegistration.setNodeValue( getLocationNodeName(), makeAny( _NewLocation ) );
        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, _Name, sOldLocation, _NewLocation );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::changedDatabaseLocation, aEvent );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL DatabaseRegistrations::isDatabaseRegistrationReadOnly( const ::rtl::OUString& _Name ) throw (IllegalArgumentException, NoSuchElementException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_checkValidName_throw( _Name, true );

        ::utl::OConfigurationNode aDataSourceRegistration = m_aConfigurationRoot.openNode( _Name );
        return aDataSourceRegistration.isReadonly();
    }

    //--------------------------------------------------------------------
    void SAL_CALL DatabaseRegistrations::addDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& _Listener ) throw (RuntimeException)
    {
        if ( _Listener.is() )
            m_aRegistrationListeners.addInterface( _Listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL DatabaseRegistrations::removeDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& _Listener ) throw (RuntimeException)
    {
        if ( _Listener.is() )
            m_aRegistrationListeners.removeInterface( _Listener );
    }

    //====================================================================
    //= DatabaseRegistrations - factory
    //====================================================================
    Reference< XAggregation > createDataSourceRegistrations( const ::comphelper::ComponentContext& _rxContext )
    {
        return new DatabaseRegistrations( _rxContext );
    }

//........................................................................
} // namespace dbaccess
//........................................................................
