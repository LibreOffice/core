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

#include "composerdialogs.hxx"

#include "dbu_reghelper.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include "dbustrings.hrc"
#include "queryfilter.hxx"
#include "queryorder.hxx"
#include <comphelper/processfactory.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

extern "C" void SAL_CALL createRegistryInfo_ComposerDialogs()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::RowsetOrderDialog > aOrderDialogRegistration;
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::RowsetFilterDialog > aFilterDialogRegistration;
}

namespace dbaui
{

#define PROPERTY_ID_QUERYCOMPOSER       100
#define PROPERTY_ID_ROWSET              101

#define  PROPERTY_QUERYCOMPOSER   "QueryComposer"
#define  PROPERTY_ROWSET          "RowSet"

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    // ComposerDialog
    DBG_NAME(ComposerDialog)
    ComposerDialog::ComposerDialog(const Reference< XComponentContext >& _rxORB)
        :OGenericUnoDialog( _rxORB )
    {
        DBG_CTOR(ComposerDialog,NULL);

        registerProperty( PROPERTY_QUERYCOMPOSER, PROPERTY_ID_QUERYCOMPOSER, PropertyAttribute::TRANSIENT,
            &m_xComposer, ::getCppuType( &m_xComposer ) );
        registerProperty( PROPERTY_ROWSET, PROPERTY_ID_ROWSET, PropertyAttribute::TRANSIENT,
            &m_xRowSet, ::getCppuType( &m_xRowSet ) );
    }

    ComposerDialog::~ComposerDialog()
    {

        DBG_DTOR(ComposerDialog,NULL);
    }

    IMPLEMENT_IMPLEMENTATION_ID( ComposerDialog )

    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( ComposerDialog )

    Dialog* ComposerDialog::createDialog(Window* _pParent)
    {
        // obtain all the objects needed for the dialog
        Reference< XConnection > xConnection;
        Reference< XNameAccess > xColumns;
        try
        {
            // the connection the row set is working with
            if ( !::dbtools::isEmbeddedInDatabase( m_xRowSet, xConnection ) )
            {
                Reference< XPropertySet > xRowsetProps( m_xRowSet, UNO_QUERY );
                if ( xRowsetProps.is() )
                    xRowsetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            }

            // fallback: if there is a connection and thus a row set, but no composer, create one
            if ( xConnection.is() && !m_xComposer.is() )
                m_xComposer = ::dbtools::getCurrentSettingsComposer( Reference< XPropertySet >( m_xRowSet, UNO_QUERY ), m_aContext );

            // the columns of the row set
            Reference< XColumnsSupplier > xSuppColumns( m_xRowSet, UNO_QUERY );
            if ( xSuppColumns.is() )
                xColumns = xSuppColumns->getColumns();

            if ( !xColumns.is() || !xColumns->hasElements() )
            {   // perhaps the composer can supply us with columns? This is necessary for cases
                // where the dialog is invoked for a rowset which is not yet loaded
                // #i22878#
                xSuppColumns = xSuppColumns.query( m_xComposer );
                if ( xSuppColumns.is() )
                    xColumns = xSuppColumns->getColumns();
            }

            OSL_ENSURE( xColumns.is() && xColumns->hasElements(), "ComposerDialog::createDialog: not much fun without any columns!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !xConnection.is() || !xColumns.is() || !m_xComposer.is() )
            // can't create the dialog if I have improper settings
            return NULL;

        return createComposerDialog( _pParent, xConnection, xColumns );
    }

    // RowsetFilterDialog
    RowsetFilterDialog::RowsetFilterDialog( const Reference< XComponentContext >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(RowsetFilterDialog, "com.sun.star.uno.comp.sdb.RowsetFilterDialog")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(RowsetFilterDialog)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(RowsetFilterDialog, "com.sun.star.sdb.FilterDialog")

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL RowsetFilterDialog::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new RowsetFilterDialog( comphelper::getComponentContext(_rxORB)));
    }

    Dialog* RowsetFilterDialog::createComposerDialog( Window* _pParent, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxColumns )
    {
        return new DlgFilterCrit( _pParent, m_aContext, _rxConnection, m_xComposer, _rxColumns );
    }

    void SAL_CALL RowsetFilterDialog::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        if( aArguments.getLength() == 3 )
        {
            // this is the FilterDialog::createWithQuery method
            Reference<com::sun::star::sdb::XSingleSelectQueryComposer> xQueryComposer;
            aArguments[0] >>= xQueryComposer;
            Reference<com::sun::star::sdbc::XRowSet> xRowSet;
            aArguments[1] >>= xRowSet;
            Reference<com::sun::star::awt::XWindow> xParentWindow;
            aArguments[2] >>= xParentWindow;
            setPropertyValue( "QueryComposer", makeAny( xQueryComposer ) );
            setPropertyValue( "RowSet",        makeAny( xRowSet ) );
            setPropertyValue( "ParentWindow",  makeAny( xParentWindow ) );
        }
        else
            ComposerDialog::initialize(aArguments);
    }

    void RowsetFilterDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        ComposerDialog::executedDialog( _nExecutionResult );

        if ( _nExecutionResult && m_pDialog )
            static_cast< DlgFilterCrit* >( m_pDialog )->BuildWherePart();
    }

    // RowsetOrderDialog
    RowsetOrderDialog::RowsetOrderDialog( const Reference< XComponentContext >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(RowsetOrderDialog, "com.sun.star.uno.comp.sdb.RowsetOrderDialog")
    IMPLEMENT_SERVICE_INFO_SUPPORTS(RowsetOrderDialog)
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(RowsetOrderDialog, "com.sun.star.sdb.OrderDialog")

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL RowsetOrderDialog::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new RowsetOrderDialog( comphelper::getComponentContext(_rxORB)));
    }

    Dialog* RowsetOrderDialog::createComposerDialog( Window* _pParent, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxColumns )
    {
        return new DlgOrderCrit( _pParent, _rxConnection, m_xComposer, _rxColumns );
    }

    void SAL_CALL RowsetOrderDialog::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        if( aArguments.getLength() == 2 )
        {
            Reference<com::sun::star::sdb::XSingleSelectQueryComposer> xQueryComposer;
            aArguments[0] >>= xQueryComposer;
            Reference<com::sun::star::beans::XPropertySet> xRowSet;
            aArguments[1] >>= xRowSet;
            setPropertyValue( "QueryComposer", makeAny( xQueryComposer ) );
            setPropertyValue( "RowSet",        makeAny( xRowSet ) );
        }
        else
            ComposerDialog::initialize(aArguments);
    }

    void RowsetOrderDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        ComposerDialog::executedDialog( _nExecutionResult );

        if ( !m_pDialog )
            return;

        if ( _nExecutionResult )
            static_cast< DlgOrderCrit* >( m_pDialog )->BuildOrderPart();
        else if ( m_xComposer.is() )
            m_xComposer->setOrder( static_cast< DlgOrderCrit* >( m_pDialog )->GetOrignalOrder() );
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
