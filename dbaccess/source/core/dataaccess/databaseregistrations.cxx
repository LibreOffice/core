/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>

#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>

#include "databaseregistrations.hxx"

namespace dbaccess
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::container::NoSuchElementException;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::lang::IllegalAccessException;
    using ::com::sun::star::container::ElementExistException;
    using ::com::sun::star::sdb::XDatabaseRegistrations;
    using ::com::sun::star::sdb::XDatabaseRegistrationsListener;
    using ::com::sun::star::sdb::DatabaseRegistrationEvent;

    constexpr OUString CONF_ROOT_PATH = u"org.openoffice.Office.DataAccess/RegisteredNames"_ustr;
    constexpr OUString LOCATION = u"Location"_ustr;
    constexpr OUString NAME = u"Name"_ustr;

    // DatabaseRegistrations - declaration
    typedef ::cppu::WeakImplHelper<   XDatabaseRegistrations
                                   >   DatabaseRegistrations_Base;

    namespace {

    class DatabaseRegistrations :public ::cppu::BaseMutex
                                ,public DatabaseRegistrations_Base
    {
    public:
        explicit DatabaseRegistrations( const Reference<XComponentContext>& _rxContext );

    protected:
        virtual ~DatabaseRegistrations() override;

    public:
        virtual sal_Bool SAL_CALL hasRegisteredDatabase( const OUString& Name ) override;
        virtual Sequence< OUString > SAL_CALL getRegistrationNames() override;
        virtual OUString SAL_CALL getDatabaseLocation( const OUString& Name ) override;
        virtual void SAL_CALL registerDatabaseLocation( const OUString& Name, const OUString& Location ) override;
        virtual void SAL_CALL revokeDatabaseLocation( const OUString& Name ) override;
        virtual void SAL_CALL changeDatabaseLocation( const OUString& Name, const OUString& NewLocation ) override;
        virtual sal_Bool SAL_CALL isDatabaseRegistrationReadOnly( const OUString& Name ) override;
        virtual void SAL_CALL addDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener ) override;
        virtual void SAL_CALL removeDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener ) override;

    private:
        void
                impl_checkValidName_common(std::u16string_view _rName);
        ::utl::OConfigurationNode
                impl_checkValidName_throw_must_exist(const OUString& _rName);
        ::utl::OConfigurationNode
                impl_checkValidName_throw_must_not_exist(const OUString& _rName);

        void    impl_checkValidLocation_throw( std::u16string_view _rLocation );

        /** retrieves the configuration node whose "Name" sub node has the given value

            Since we separated the name of the registration node from the "Name" value of the registration, we cannot
            simply do a "getByName" (equivalent) when we want to retrieve the node for a given registration name.
            Instead, we must search all nodes.

            If a node with the given display name does not exist, then a NoSuchElementException is thrown.

            If no exception is thrown, then a valid node is returned: If the node existed it is returned.
        */
        ::utl::OConfigurationNode
                impl_getNodeForName_throw_must_exist(const OUString& _rName);

        /** retrieves the configuration node whose "Name" sub node has the given value

            Since we separated the name of the registration node from the "Name" value of the registration, we cannot
            simply do a "getByName" (equivalent) when we want to retrieve the node for a given registration name.
            Instead, we must search all nodes.

            If a node with the given name already exists, then an ElementExistException is thrown.

            If no exception is thrown, then a valid node is returned: If the node did not yet exist a new node is created,
            in this case the root node is not yet committed.
        */
        ::utl::OConfigurationNode
                impl_getNodeForName_throw_must_not_exist(const OUString& _rName);


        ::utl::OConfigurationNode
                impl_getNodeForName_nothrow(std::u16string_view _rName);

    private:
        Reference<XComponentContext>        m_aContext;
        ::utl::OConfigurationTreeRoot       m_aConfigurationRoot;
        ::comphelper::OInterfaceContainerHelper3<XDatabaseRegistrationsListener>  m_aRegistrationListeners;
    };

    }

    // DatabaseRegistrations - implementation
    DatabaseRegistrations::DatabaseRegistrations( const Reference<XComponentContext> & _rxContext )
        :m_aContext( _rxContext )
        ,m_aRegistrationListeners( m_aMutex )
    {
        m_aConfigurationRoot = ::utl::OConfigurationTreeRoot::createWithComponentContext(
            m_aContext, CONF_ROOT_PATH );
    }

    DatabaseRegistrations::~DatabaseRegistrations()
    {
    }

    ::utl::OConfigurationNode DatabaseRegistrations::impl_getNodeForName_nothrow( std::u16string_view _rName )
    {
        const Sequence< OUString > aNames( m_aConfigurationRoot.getNodeNames() );
        for ( auto const & nodeName : aNames )
        {
            ::utl::OConfigurationNode aNodeForName = m_aConfigurationRoot.openNode( nodeName );

            OUString sTestName;
            OSL_VERIFY( aNodeForName.getNodeValue( NAME ) >>= sTestName );
            if ( sTestName == _rName )
                return aNodeForName;
        }
        return ::utl::OConfigurationNode();
    }

    ::utl::OConfigurationNode DatabaseRegistrations::impl_getNodeForName_throw_must_exist(const OUString& _rName)
    {
        ::utl::OConfigurationNode aNodeForName( impl_getNodeForName_nothrow( _rName ) );

        if (!aNodeForName.isValid())
        {
            throw NoSuchElementException( _rName, *this );
        }

        return aNodeForName;
    }

    ::utl::OConfigurationNode DatabaseRegistrations::impl_getNodeForName_throw_must_not_exist(const OUString& _rName)
    {
        ::utl::OConfigurationNode aNodeForName( impl_getNodeForName_nothrow( _rName ) );

        if (aNodeForName.isValid())
            throw ElementExistException( _rName, *this );

        // make unique
        OUString sNewNodeName = "org.openoffice." + _rName;
        while ( m_aConfigurationRoot.hasByName( sNewNodeName ) )
        {
            sNewNodeName = "org.openoffice." + _rName + " 2";
        }

        ::utl::OConfigurationNode aNewNode( m_aConfigurationRoot.createNode( sNewNodeName ) );
        aNewNode.setNodeValue( NAME, Any( _rName ) );
        return aNewNode;
    }

    void DatabaseRegistrations::impl_checkValidName_common(std::u16string_view _rName)
    {
        if ( !m_aConfigurationRoot.isValid() )
            throw RuntimeException( OUString(), *this );

        if ( _rName.empty() )
            throw IllegalArgumentException( OUString(), *this, 1 );
    }

    ::utl::OConfigurationNode DatabaseRegistrations::impl_checkValidName_throw_must_exist(const OUString& _rName)
    {
        impl_checkValidName_common(_rName);
        return impl_getNodeForName_throw_must_exist(_rName);
    }

    ::utl::OConfigurationNode DatabaseRegistrations::impl_checkValidName_throw_must_not_exist(const OUString& _rName)
    {
        impl_checkValidName_common(_rName);
        return impl_getNodeForName_throw_must_not_exist(_rName);
    }

    void DatabaseRegistrations::impl_checkValidLocation_throw( std::u16string_view _rLocation )
    {
        if ( _rLocation.empty() )
            throw IllegalArgumentException( OUString(), *this, 2 );

        INetURLObject aURL( _rLocation );
        if ( aURL.GetProtocol() == INetProtocol::NotValid )
            throw IllegalArgumentException( OUString(), *this, 2 );
    }

    sal_Bool SAL_CALL DatabaseRegistrations::hasRegisteredDatabase( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::utl::OConfigurationNode aNodeForName = impl_getNodeForName_nothrow( Name );
        return aNodeForName.isValid();
    }

    Sequence< OUString > SAL_CALL DatabaseRegistrations::getRegistrationNames()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_aConfigurationRoot.isValid() )
            throw RuntimeException( OUString(), *this );

        const Sequence< OUString > aProgrammaticNames( m_aConfigurationRoot.getNodeNames() );
        Sequence< OUString > aDisplayNames( aProgrammaticNames.getLength() );
        OUString* pDisplayName = aDisplayNames.getArray();

        for ( auto const & name : aProgrammaticNames )
        {
            ::utl::OConfigurationNode aRegistrationNode = m_aConfigurationRoot.openNode( name );
            OSL_VERIFY( aRegistrationNode.getNodeValue( NAME ) >>= *pDisplayName );
            ++pDisplayName;
        }

        return aDisplayNames;
    }

    OUString SAL_CALL DatabaseRegistrations::getDatabaseLocation( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ::utl::OConfigurationNode aNodeForName = impl_checkValidName_throw_must_exist(Name);

        OUString sLocation;
        OSL_VERIFY( aNodeForName.getNodeValue( LOCATION ) >>= sLocation );
        sLocation = SvtPathOptions().SubstituteVariable( sLocation );

        return sLocation;
    }

    void SAL_CALL DatabaseRegistrations::registerDatabaseLocation( const OUString& Name, const OUString& Location )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        impl_checkValidLocation_throw( Location );
        ::utl::OConfigurationNode aDataSourceRegistration = impl_checkValidName_throw_must_not_exist(Name);

        // register
        aDataSourceRegistration.setNodeValue( LOCATION, Any( Location ) );
        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, Name, OUString(), Location );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::registeredDatabaseLocation, aEvent );
    }

    void SAL_CALL DatabaseRegistrations::revokeDatabaseLocation( const OUString& Name )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        ::utl::OConfigurationNode aNodeForName = impl_checkValidName_throw_must_exist(Name);

        // obtain properties for notification
        OUString sLocation;
        OSL_VERIFY( aNodeForName.getNodeValue( LOCATION ) >>= sLocation );

        // revoke
        if  (   aNodeForName.isReadonly()
            ||  !m_aConfigurationRoot.removeNode( aNodeForName.getLocalName() )
            )
            throw IllegalAccessException( OUString(), *this );

        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, Name, sLocation, OUString() );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::revokedDatabaseLocation, aEvent );
    }

    void SAL_CALL DatabaseRegistrations::changeDatabaseLocation( const OUString& Name, const OUString& NewLocation )
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        // check
        impl_checkValidLocation_throw( NewLocation );
        ::utl::OConfigurationNode aDataSourceRegistration = impl_checkValidName_throw_must_exist(Name);

        if  ( aDataSourceRegistration.isReadonly() )
            throw IllegalAccessException( OUString(), *this );

        // obtain properties for notification
        OUString sOldLocation;
        OSL_VERIFY( aDataSourceRegistration.getNodeValue( LOCATION ) >>= sOldLocation );

        // change
        aDataSourceRegistration.setNodeValue( LOCATION, Any( NewLocation ) );
        m_aConfigurationRoot.commit();

        // notify
        DatabaseRegistrationEvent aEvent( *this, Name, sOldLocation, NewLocation );
        aGuard.clear();
        m_aRegistrationListeners.notifyEach( &XDatabaseRegistrationsListener::changedDatabaseLocation, aEvent );
    }

    sal_Bool SAL_CALL DatabaseRegistrations::isDatabaseRegistrationReadOnly( const OUString& Name )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        ::utl::OConfigurationNode aDataSourceRegistration = impl_checkValidName_throw_must_exist(Name);
        return aDataSourceRegistration.isReadonly();
    }

    void SAL_CALL DatabaseRegistrations::addDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener )
    {
        if ( Listener.is() )
            m_aRegistrationListeners.addInterface( Listener );
    }

    void SAL_CALL DatabaseRegistrations::removeDatabaseRegistrationsListener( const Reference< XDatabaseRegistrationsListener >& Listener )
    {
        if ( Listener.is() )
            m_aRegistrationListeners.removeInterface( Listener );
    }

    // DatabaseRegistrations - factory
    Reference< XDatabaseRegistrations > createDataSourceRegistrations( const Reference<XComponentContext> & _rxContext )
    {
        return new DatabaseRegistrations( _rxContext );
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
