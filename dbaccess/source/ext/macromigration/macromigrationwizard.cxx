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
#include "macromigrationwizard.hxx"

#include <com/sun/star/ucb/AlreadyInitializedException.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <svtools/genericunodialog.hxx>

namespace dbmm
{

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

    // MacroMigrationDialogService
    class MacroMigrationDialogService;
    typedef ::svt::OGenericUnoDialog                                                MacroMigrationDialogService_Base;
    typedef ::comphelper::OPropertyArrayUsageHelper< MacroMigrationDialogService >  MacroMigrationDialogService_PBase;

    class MacroMigrationDialogService
                :public MacroMigrationDialogService_Base
                ,public MacroMigrationDialogService_PBase
                ,public MacroMigrationModuleClient
    {
    public:
        explicit MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext );

        // XTypeProvider
        virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XPropertySet
        virtual Reference< XPropertySetInfo >  SAL_CALL getPropertySetInfo() throw(RuntimeException, std::exception) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // helper for factories
        static Reference< XInterface > SAL_CALL Create( const Reference< XComponentContext >& _rxContext );
        static OUString SAL_CALL getImplementationName_static() throw(RuntimeException);
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_static() throw(RuntimeException);

    protected:
        virtual ~MacroMigrationDialogService();

    protected:
        virtual VclPtr<Dialog> createDialog( vcl::Window* _pParent ) override;
        virtual void destroyDialog() override;

    private:
        Reference<XComponentContext>          m_aContext;
        Reference< XOfficeDatabaseDocument >    m_xDocument;
    };

    // MacroMigrationDialogService
    MacroMigrationDialogService::MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext )
        :MacroMigrationDialogService_Base( _rxContext )
        ,m_aContext( _rxContext )
    {
        m_bNeedInitialization = true;
    }

    MacroMigrationDialogService::~MacroMigrationDialogService()
    {
        // we do this here cause the base class' call to destroyDialog won't reach us anymore: we're within an dtor,
        // so this virtual-method-call the base class does not work, we're already dead then...
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    Reference< XInterface > SAL_CALL MacroMigrationDialogService::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *(new MacroMigrationDialogService( _rxContext ) );
    }

    VclPtr<Dialog> MacroMigrationDialogService::createDialog( vcl::Window* _pParent )
    {
        return VclPtr<MacroMigrationDialog>::Create( _pParent, m_aContext, m_xDocument );
    }

    void MacroMigrationDialogService::destroyDialog()
    {
        MacroMigrationDialogService_Base::destroyDialog();
    }

    Sequence< sal_Int8 > SAL_CALL MacroMigrationDialogService::getImplementationId() throw(RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }

    OUString SAL_CALL MacroMigrationDialogService::getImplementationName_static() throw(RuntimeException)
    {
        return OUString( "com.sun.star.comp.dbaccess.macromigration.MacroMigrationDialogService" );
    }

    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames_static() throw(RuntimeException)
    {
        Sequence< OUString > aServices { "com.sun.star.sdb.application.MacroMigrationWizard" };
        return aServices;
    }

    OUString SAL_CALL MacroMigrationDialogService::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }

    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }

    void SAL_CALL MacroMigrationDialogService::initialize( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException, std::exception)
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

    Reference< XPropertySetInfo > SAL_CALL MacroMigrationDialogService::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        return createPropertySetInfo( getInfoHelper() );
    }

    ::cppu::IPropertyArrayHelper& SAL_CALL MacroMigrationDialogService::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* MacroMigrationDialogService::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    void createRegistryInfo_MacroMigrationDialogService()
    {
        static OAutoRegistration< MacroMigrationDialogService > aAutoRegistration;
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
