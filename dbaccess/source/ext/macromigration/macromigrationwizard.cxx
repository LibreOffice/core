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


#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "macromigrationdialog.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/componentcontext.hxx>
#include <svtools/genericunodialog.hxx>

//........................................................................
namespace dbmm
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
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::ucb::AlreadyInitializedException;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::frame::XStorable;
    /** === end UNO using === **/

    //====================================================================
    //= MacroMigrationDialogService
    //====================================================================
    class MacroMigrationDialogService;
    typedef ::svt::OGenericUnoDialog                                                MacroMigrationDialogService_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< MacroMigrationDialogService >  MacroMigrationDialogService_PBase;

    class MacroMigrationDialogService
                :public MacroMigrationDialogService_Base
                ,public MacroMigrationDialogService_PBase
                ,public MacroMigrationModuleClient
    {
    public:
        MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& aArguments ) throw(com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

        // XPropertySet
        virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // helper for factories
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& _rxContext );
        static OUString SAL_CALL getImplementationName_static() throw(RuntimeException);
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_static() throw(RuntimeException);

    protected:
        ~MacroMigrationDialogService();

    protected:
        virtual Dialog* createDialog( Window* _pParent );
        virtual void destroyDialog();

    private:
        ::comphelper::ComponentContext          m_aContext;
        Reference< XOfficeDatabaseDocument >    m_xDocument;
    };

    //====================================================================
    //= MacroMigrationDialogService
    //====================================================================
    //--------------------------------------------------------------------
    MacroMigrationDialogService::MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext )
        :MacroMigrationDialogService_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
        m_bNeedInitialization = true;
    }

    //--------------------------------------------------------------------
    MacroMigrationDialogService::~MacroMigrationDialogService()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
        // so this virtual-method-call the base class does does not work, we're already dead then ...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    //--------------------------------------------------------------------
    Reference< XInterface > SAL_CALL MacroMigrationDialogService::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new MacroMigrationDialogService( _rxContext ) );
    }

    //--------------------------------------------------------------------
    Dialog* MacroMigrationDialogService::createDialog( Window* _pParent )
    {
        return new MacroMigrationDialog( _pParent, m_aContext, m_xDocument );
    }

    //--------------------------------------------------------------------
    void MacroMigrationDialogService::destroyDialog()
    {
        MacroMigrationDialogService_Base::destroyDialog();
    }

    //--------------------------------------------------------------------
    Sequence< sal_Int8 > SAL_CALL MacroMigrationDialogService::getImplementationId() throw(RuntimeException)
    {
        static ::cppu::OImplementationId* pId = NULL;
        if ( !pId )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL MacroMigrationDialogService::getImplementationName_static() throw(RuntimeException)
    {
        return OUString( "com.sun.star.comp.dbaccess.macromigration.MacroMigrationDialogService" );
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< OUString > aServices(1);
        aServices[0] = OUString( "com.sun.star.sdb.application.MacroMigrationWizard" );
        return aServices;
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL MacroMigrationDialogService::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //--------------------------------------------------------------------
    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //--------------------------------------------------------------------
    void SAL_CALL MacroMigrationDialogService::initialize( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInitialized )
            throw AlreadyInitializedException( OUString(), *this );

        if ( _rArguments.getLength() != 1 )
            throw IllegalArgumentException(
                OUString(MacroMigrationResId(STR_INVALID_NUMBER_ARGS)),
                *this,
                1
            );

        m_xDocument.set( _rArguments[0], UNO_QUERY );
        if ( !m_xDocument.is() )
            throw IllegalArgumentException(
                OUString(MacroMigrationResId(STR_NO_DATABASE)),
                *this,
                1
            );

        Reference< XStorable > xDocStor( m_xDocument, UNO_QUERY_THROW );
        if ( xDocStor->isReadonly() )
            throw IllegalArgumentException(
                OUString(MacroMigrationResId(STR_NOT_READONLY)),
                *this,
                1
            );

        m_bInitialized = true;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL MacroMigrationDialogService::getPropertySetInfo() throw(RuntimeException)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL MacroMigrationDialogService::getInfoHelper()
    {
        return *const_cast< MacroMigrationDialogService* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* MacroMigrationDialogService::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //--------------------------------------------------------------------
    void createRegistryInfo_MacroMigrationDialogService()
    {
        static OAutoRegistration< MacroMigrationDialogService > aAutoRegistration;
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
