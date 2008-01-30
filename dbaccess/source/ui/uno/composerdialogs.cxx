/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: composerdialogs.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:56:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBACCESS_SOURCE_UI_UNO_COMPOSERDIALOGS_HXX
#include "composerdialogs.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
/** === end UNO includes === **/
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_QUERYFILTER_HXX
#include "queryfilter.hxx"
#endif
#ifndef DBAUI_QUERYORDER_HXX
#include "queryorder.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
                m_xComposer = ::dbtools::getCurrentSettingsComposer( Reference< XPropertySet >( m_xRowSet, UNO_QUERY ), m_xORB );

            // the columns of the row set
            Reference< XColumnsSupplier > xSuppColumns( m_xRowSet, UNO_QUERY );
            if ( xSuppColumns.is() )
                xColumns = xSuppColumns->getColumns();

            if ( !xColumns.is() || !xColumns->hasElements() )
            {   // perhaps the composer can supply us with columns? This is necessary for cases
                // where the dialog is invoked for a rowset which is not yet loaded
                // #i22878# - 2003-12-16 - fs@openoffice.org
                xSuppColumns = xSuppColumns.query( m_xComposer );
                if ( xSuppColumns.is() )
                    xColumns = xSuppColumns->getColumns();
            }

            DBG_ASSERT( xColumns.is() && xColumns->hasElements(), "ComposerDialog::createDialog: not much fun without any columns!" );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ComposerDialog::createDialog: caught an exception!" );
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
        return new DlgFilterCrit( _pParent, m_xORB, _rxConnection, m_xComposer, _rxColumns );
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


