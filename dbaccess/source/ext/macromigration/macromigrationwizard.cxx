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

#include "macromigrationwizard.hxx"

namespace dbmm
{
    // MacroMigrationDialogService
    MacroMigrationDialogService::MacroMigrationDialogService( const Reference< XComponentContext >& _rxContext )
        :MacroMigrationDialogService_Base( _rxContext )
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

    Sequence< sal_Int8 > SAL_CALL MacroMigrationDialogService::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    OUString SAL_CALL MacroMigrationDialogService::getImplementationName_static()
    {
        return OUString( "com.sun.star.comp.dbaccess.macromigration.MacroMigrationDialogService" );
    }

    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames_static()
    {
        Sequence< OUString > aServices { "com.sun.star.sdb.application.MacroMigrationWizard" };
        return aServices;
    }

    OUString SAL_CALL MacroMigrationDialogService::getImplementationName()
    {
        return getImplementationName_static();
    }

    Sequence< OUString > SAL_CALL MacroMigrationDialogService::getSupportedServiceNames()
    {
        return getSupportedServiceNames_static();
    }

    void SAL_CALL MacroMigrationDialogService::initialize( const Sequence< Any >& _rArguments )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInitialized )
            throw AlreadyInitializedException( OUString(), *this );

        if ( _rArguments.getLength() != 1 )
            throw IllegalArgumentException(
                DBA_RES(STR_INVALID_NUMBER_ARGS),
                *this,
                1
            );

        m_xDocument.set( _rArguments[0], UNO_QUERY );
        if ( !m_xDocument.is() )
            throw IllegalArgumentException(
                DBA_RES(STR_NO_DATABASE),
                *this,
                1
            );

        Reference< XStorable > xDocStor( m_xDocument, UNO_QUERY_THROW );
        if ( xDocStor->isReadonly() )
            throw IllegalArgumentException(
                DBA_RES(STR_NOT_READONLY),
                *this,
                1
            );

        m_bInitialized = true;
    }

    Reference< XPropertySetInfo > SAL_CALL MacroMigrationDialogService::getPropertySetInfo()
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
} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
