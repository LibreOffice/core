/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "composerdialogs.hxx"

/** === begin UNO includes === **/
#include "dbu_reghelper.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
/** === end UNO includes === **/
#include "dbustrings.hrc"
#include "queryfilter.hxx"
#include "queryorder.hxx"
#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

extern "C" void SAL_CALL createRegistryInfo_ComposerDialogs()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::RowsetOrderDialog > aOrderDialogRegistration;
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::RowsetFilterDialog > aFilterDialogRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

#define PROPERTY_ID_QUERYCOMPOSER       100
#define PROPERTY_ID_ROWSET              101

    IMPLEMENT_CONSTASCII_USTRING( PROPERTY_QUERYCOMPOSER,   "QueryComposer" );
    IMPLEMENT_CONSTASCII_USTRING( PROPERTY_ROWSET,          "RowSet" );

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;

    //=====================================================================
    //= ComposerDialog
    //=====================================================================
    DBG_NAME(ComposerDialog)
    //---------------------------------------------------------------------
    ComposerDialog::ComposerDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :ComposerDialog_BASE( _rxORB )
    {
        DBG_CTOR(ComposerDialog,NULL);

        registerProperty( PROPERTY_QUERYCOMPOSER, PROPERTY_ID_QUERYCOMPOSER, PropertyAttribute::TRANSIENT,
            &m_xComposer, ::getCppuType( &m_xComposer ) );
        registerProperty( PROPERTY_ROWSET, PROPERTY_ID_ROWSET, PropertyAttribute::TRANSIENT,
            &m_xRowSet, ::getCppuType( &m_xRowSet ) );
    }

    //---------------------------------------------------------------------
    ComposerDialog::~ComposerDialog()
    {

        DBG_DTOR(ComposerDialog,NULL);
    }

    //---------------------------------------------------------------------
    IMPLEMENT_IMPLEMENTATION_ID( ComposerDialog )

    //---------------------------------------------------------------------
    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( ComposerDialog )

    //---------------------------------------------------------------------
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
                m_xComposer = ::dbtools::getCurrentSettingsComposer( Reference< XPropertySet >( m_xRowSet, UNO_QUERY ), m_aContext.getLegacyServiceFactory() );

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

    //=====================================================================
    //= RowsetFilterDialog
    //=====================================================================
    //---------------------------------------------------------------------
    RowsetFilterDialog::RowsetFilterDialog( const Reference< XMultiServiceFactory >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    //---------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC( RowsetFilterDialog, "com.sun.star.uno.comp.sdb.RowsetFilterDialog", "com.sun.star.sdb.FilterDialog" )

    //---------------------------------------------------------------------
    Dialog* RowsetFilterDialog::createComposerDialog( Window* _pParent, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxColumns )
    {
        return new DlgFilterCrit( _pParent, m_aContext.getLegacyServiceFactory(), _rxConnection, m_xComposer, _rxColumns );
    }

    //---------------------------------------------------------------------
    void RowsetFilterDialog::executedDialog( sal_Int16 _nExecutionResult )
    {
        ComposerDialog::executedDialog( _nExecutionResult );

        if ( _nExecutionResult && m_pDialog )
            static_cast< DlgFilterCrit* >( m_pDialog )->BuildWherePart();
    }

    //=====================================================================
    //= RowsetOrderDialog
    //=====================================================================
    //---------------------------------------------------------------------
    RowsetOrderDialog::RowsetOrderDialog( const Reference< XMultiServiceFactory >& _rxORB )
        :ComposerDialog( _rxORB )
    {
    }

    //---------------------------------------------------------------------
    IMPLEMENT_SERVICE_INFO1_STATIC( RowsetOrderDialog, "com.sun.star.uno.comp.sdb.RowsetOrderDialog", "com.sun.star.sdb.OrderDialog" )

    //---------------------------------------------------------------------
    Dialog* RowsetOrderDialog::createComposerDialog( Window* _pParent, const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxColumns )
    {
        return new DlgOrderCrit( _pParent, _rxConnection, m_xComposer, _rxColumns );
    }

    //---------------------------------------------------------------------
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

//.........................................................................
}   // namespace dbaui
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
