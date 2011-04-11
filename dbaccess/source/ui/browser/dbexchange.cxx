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

#include "dbexchange.hxx"
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include "TokenWriter.hxx"
#include "dbustrings.hrc"
#include <comphelper/uno3.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include "UITools.hxx"


namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::svx;

    namespace
    {
        template<class T > void lcl_setListener(const Reference<T>& _xComponent, const Reference< XEventListener >& i_rListener, const bool i_bAdd )
        {
            if ( !_xComponent.is() )
                return;

            Reference< XComponent> xCom( _xComponent, UNO_QUERY );
            OSL_ENSURE( xCom.is(), "lcl_setListener: no component!" );
            if ( !xCom.is() )
                return;

            i_bAdd ? xCom->addEventListener( i_rListener ) : xCom->removeEventListener( i_rListener );
        }
    }

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard(
                    const ::rtl::OUString&  _rDatasource,
                    const sal_Int32         _nCommandType,
                    const ::rtl::OUString&  _rCommand,
                    const Reference< XConnection >& _rxConnection,
                    const Reference< XNumberFormatter >& _rxFormatter,
                    const Reference< XMultiServiceFactory >& _rxORB)
                    :ODataAccessObjectTransferable( _rDatasource,::rtl::OUString(), _nCommandType, _rCommand, _rxConnection )
        ,m_pHtml(NULL)
        ,m_pRtf(NULL)
    {
        osl_incrementInterlockedCount( &m_refCount );
        lcl_setListener( _rxConnection, this, true );

        m_pHtml.set( new OHTMLImportExport( getDescriptor(), _rxORB, _rxFormatter ) );
        m_pRtf.set( new ORTFImportExport( getDescriptor(), _rxORB, _rxFormatter ) );

        osl_decrementInterlockedCount( &m_refCount );
    }

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard(
                    const ::rtl::OUString&  _rDatasource,
                    const sal_Int32         _nCommandType,
                    const ::rtl::OUString&  _rCommand,
                    const Reference< XNumberFormatter >& _rxFormatter,
                    const Reference< XMultiServiceFactory >& _rxORB)
        :ODataAccessObjectTransferable( _rDatasource, ::rtl::OUString(),_nCommandType, _rCommand)
        ,m_pHtml(NULL)
        ,m_pRtf(NULL)
    {
        m_pHtml.set( new OHTMLImportExport( getDescriptor(),_rxORB, _rxFormatter ) );
        m_pRtf.set( new ORTFImportExport( getDescriptor(),_rxORB, _rxFormatter ) );
    }

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard( const Reference< XPropertySet >& i_rAliveForm,
                                    const Sequence< Any >& i_rSelectedRows,
                                    const sal_Bool i_bBookmarkSelection,
                                    const Reference< XMultiServiceFactory >& i_rORB )
        :ODataAccessObjectTransferable( i_rAliveForm )
        ,m_pHtml(NULL)
        ,m_pRtf(NULL)
    {
        OSL_PRECOND( i_rORB.is(), "ODataClipboard::ODataClipboard: having no factory is not good ..." );

        osl_incrementInterlockedCount( &m_refCount );

        Reference<XConnection> xConnection;
        getDescriptor()[ daConnection ] >>= xConnection;
        lcl_setListener( xConnection, this, true );

        // do not pass the form itself as source result set, since the client might operate on the form, which
        // might lead to undesired effects. Instead, use a clone.
        Reference< XResultSet > xResultSetClone;
        Reference< XResultSetAccess > xResultSetAccess( i_rAliveForm, UNO_QUERY );
        if ( xResultSetAccess.is() )
            xResultSetClone = xResultSetAccess->createResultSet();
        OSL_ENSURE( xResultSetClone.is(), "ODataClipboard::ODataClipboard: could not clone the form's result set" );
        lcl_setListener( xResultSetClone, this, true );

        getDescriptor()[daCursor]           <<= xResultSetClone;
        getDescriptor()[daSelection]        <<= i_rSelectedRows;
        getDescriptor()[daBookmarkSelection]<<= i_bBookmarkSelection;
        addCompatibleSelectionDescription( i_rSelectedRows );

        if ( xConnection.is() && i_rORB.is() )
        {
            Reference< XNumberFormatter > xFormatter( getNumberFormatter( xConnection, i_rORB ) );
            if ( xFormatter.is() )
            {
                m_pHtml.set( new OHTMLImportExport( getDescriptor(), i_rORB, xFormatter ) );
                m_pRtf.set( new ORTFImportExport( getDescriptor(), i_rORB, xFormatter ) );
            }
        }

        osl_decrementInterlockedCount( &m_refCount );
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& /*rFlavor*/ )
    {
        if (nUserObjectId == SOT_FORMAT_RTF || nUserObjectId == SOT_FORMATSTR_ID_HTML )
        {
            ODatabaseImportExport* pExport = reinterpret_cast<ODatabaseImportExport*>(pUserObject);
            if ( pExport && rxOStm.Is() )
            {
                pExport->setStream(&rxOStm);
                return pExport->Write();
            }
        }
        return sal_False;
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::AddSupportedFormats()
    {
        if ( m_pRtf.is() )
            AddFormat( SOT_FORMAT_RTF );

        if ( m_pHtml.is() )
            AddFormat( SOT_FORMATSTR_ID_HTML );

        ODataAccessObjectTransferable::AddSupportedFormats();
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::GetData( const DataFlavor& rFlavor )
    {
        const sal_uLong nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SOT_FORMAT_RTF:
                if ( m_pRtf.is() )
                    m_pRtf->initialize(getDescriptor());
                return m_pRtf.is() && SetObject( m_pRtf.get(), SOT_FORMAT_RTF, rFlavor );

            case SOT_FORMATSTR_ID_HTML:
                if ( m_pHtml.is() )
                    m_pHtml->initialize(getDescriptor());
                return m_pHtml.is() && SetObject( m_pHtml.get(), SOT_FORMATSTR_ID_HTML, rFlavor );
        }

        return ODataAccessObjectTransferable::GetData( rFlavor );
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::ObjectReleased()
    {
        if ( m_pHtml.is() )
        {
            m_pHtml->dispose();
            m_pHtml.clear();
        }

        if ( m_pRtf.is() )
        {
            m_pRtf->dispose();
            m_pRtf.clear();
        }

        if ( getDescriptor().has( daConnection ) )
        {
            Reference<XConnection> xConnection( getDescriptor()[daConnection], UNO_QUERY );
            lcl_setListener( xConnection, this, false );
        }

        if ( getDescriptor().has( daCursor ) )
        {
            Reference< XResultSet > xResultSet( getDescriptor()[ daCursor ], UNO_QUERY );
            lcl_setListener( xResultSet, this, false );
        }

        ODataAccessObjectTransferable::ObjectReleased( );
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL ODataClipboard::disposing( const ::com::sun::star::lang::EventObject& i_rSource ) throw (::com::sun::star::uno::RuntimeException)
    {
        ODataAccessDescriptor& rDescriptor( getDescriptor() );

        if ( rDescriptor.has( daConnection ) )
        {
            Reference< XConnection > xConnection( rDescriptor[daConnection], UNO_QUERY );
            if ( xConnection == i_rSource.Source )
            {
                rDescriptor.erase( daConnection );
            }
        }

        if ( rDescriptor.has( daCursor ) )
        {
            Reference< XResultSet > xResultSet( rDescriptor[ daCursor ], UNO_QUERY );
            if ( xResultSet == i_rSource.Source )
            {
                rDescriptor.erase( daCursor );
                // Selection and BookmarkSelection are meaningless without a result set
                if ( rDescriptor.has( daSelection ) )
                    rDescriptor.erase( daSelection );
                if ( rDescriptor.has( daBookmarkSelection ) )
                    rDescriptor.erase( daBookmarkSelection );
            }
        }

        // no matter whether it was the source connection or the source result set which died,
        // we cannot provide the data anymore.
        ClearFormats();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
