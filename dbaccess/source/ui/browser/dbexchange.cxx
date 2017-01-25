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
#include <comphelper/processfactory.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
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

    ODataClipboard::ODataClipboard(
                    const OUString&  _rDatasource,
                    const sal_Int32         _nCommandType,
                    const OUString&  _rCommand,
                    const Reference< XConnection >& _rxConnection,
                    const Reference< XNumberFormatter >& _rxFormatter,
                    const Reference< XComponentContext >& _rxORB)
                    :ODataAccessObjectTransferable( _rDatasource,OUString(), _nCommandType, _rCommand, _rxConnection )
        ,m_pHtml(nullptr)
        ,m_pRtf(nullptr)
    {
        osl_atomic_increment( &m_refCount );
        lcl_setListener( _rxConnection, this, true );

        m_pHtml.set( new OHTMLImportExport( getDescriptor(), _rxORB, _rxFormatter ) );
        m_pRtf.set( new ORTFImportExport( getDescriptor(), _rxORB, _rxFormatter ) );

        osl_atomic_decrement( &m_refCount );
    }

    ODataClipboard::ODataClipboard(
                    const OUString&  _rDatasource,
                    const sal_Int32         _nCommandType,
                    const OUString&  _rCommand,
                    const Reference< XNumberFormatter >& _rxFormatter,
                    const Reference< XComponentContext >& _rxORB)
        :ODataAccessObjectTransferable( _rDatasource, OUString(),_nCommandType, _rCommand)
        ,m_pHtml(nullptr)
        ,m_pRtf(nullptr)
    {
        m_pHtml.set( new OHTMLImportExport( getDescriptor(),_rxORB, _rxFormatter ) );
        m_pRtf.set( new ORTFImportExport( getDescriptor(),_rxORB, _rxFormatter ) );
    }

    ODataClipboard::ODataClipboard( const Reference< XPropertySet >& i_rAliveForm,
                                    const Sequence< Any >& i_rSelectedRows,
                                    const bool i_bBookmarkSelection,
                                    const Reference< XComponentContext >& i_rORB )
        :ODataAccessObjectTransferable( i_rAliveForm )
        ,m_pHtml(nullptr)
        ,m_pRtf(nullptr)
    {
        OSL_PRECOND( i_rORB.is(), "ODataClipboard::ODataClipboard: having no factory is not good ..." );

        osl_atomic_increment( &m_refCount );

        Reference<XConnection> xConnection;
        getDescriptor()[ DataAccessDescriptorProperty::Connection ] >>= xConnection;
        lcl_setListener( xConnection, this, true );

        // do not pass the form itself as source result set, since the client might operate on the form, which
        // might lead to undesired effects. Instead, use a clone.
        Reference< XResultSet > xResultSetClone;
        Reference< XResultSetAccess > xResultSetAccess( i_rAliveForm, UNO_QUERY );
        if ( xResultSetAccess.is() )
            xResultSetClone = xResultSetAccess->createResultSet();
        OSL_ENSURE( xResultSetClone.is(), "ODataClipboard::ODataClipboard: could not clone the form's result set" );
        lcl_setListener( xResultSetClone, this, true );

        getDescriptor()[DataAccessDescriptorProperty::Cursor]           <<= xResultSetClone;
        getDescriptor()[DataAccessDescriptorProperty::Selection]        <<= i_rSelectedRows;
        getDescriptor()[DataAccessDescriptorProperty::BookmarkSelection]<<= i_bBookmarkSelection;
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

        osl_atomic_decrement( &m_refCount );
    }

    bool ODataClipboard::WriteObject( ::tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId, const css::datatransfer::DataFlavor& /*rFlavor*/ )
    {
        if (nUserObjectId == SotClipboardFormatId::RTF || nUserObjectId == SotClipboardFormatId::HTML )
        {
            ODatabaseImportExport* pExport = static_cast<ODatabaseImportExport*>(pUserObject);
            if ( pExport && rxOStm.is() )
            {
                pExport->setStream(rxOStm.get());
                return pExport->Write();
            }
        }
        return false;
    }

    void ODataClipboard::AddSupportedFormats()
    {
        if ( m_pRtf.is() )
            AddFormat( SotClipboardFormatId::RTF );

        if ( m_pHtml.is() )
            AddFormat( SotClipboardFormatId::HTML );

        ODataAccessObjectTransferable::AddSupportedFormats();
    }

    bool ODataClipboard::GetData( const DataFlavor& rFlavor, const OUString& rDestDoc )
    {
        const SotClipboardFormatId nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SotClipboardFormatId::RTF:
                if ( m_pRtf.is() )
                    m_pRtf->initialize(getDescriptor());
                return m_pRtf.is() && SetObject( m_pRtf.get(), SotClipboardFormatId::RTF, rFlavor );

            case SotClipboardFormatId::HTML:
                if ( m_pHtml.is() )
                    m_pHtml->initialize(getDescriptor());
                return m_pHtml.is() && SetObject( m_pHtml.get(), SotClipboardFormatId::HTML, rFlavor );

            default: break;
        }

        return ODataAccessObjectTransferable::GetData(rFlavor, rDestDoc);
    }

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

        if ( getDescriptor().has( DataAccessDescriptorProperty::Connection ) )
        {
            Reference<XConnection> xConnection( getDescriptor()[DataAccessDescriptorProperty::Connection], UNO_QUERY );
            lcl_setListener( xConnection, this, false );
        }

        if ( getDescriptor().has( DataAccessDescriptorProperty::Cursor ) )
        {
            Reference< XResultSet > xResultSet( getDescriptor()[ DataAccessDescriptorProperty::Cursor ], UNO_QUERY );
            lcl_setListener( xResultSet, this, false );
        }

        ODataAccessObjectTransferable::ObjectReleased( );
    }

    void SAL_CALL ODataClipboard::disposing( const css::lang::EventObject& i_rSource )
    {
        ODataAccessDescriptor& rDescriptor( getDescriptor() );

        if ( rDescriptor.has( DataAccessDescriptorProperty::Connection ) )
        {
            Reference< XConnection > xConnection( rDescriptor[DataAccessDescriptorProperty::Connection], UNO_QUERY );
            if ( xConnection == i_rSource.Source )
            {
                rDescriptor.erase( DataAccessDescriptorProperty::Connection );
            }
        }

        if ( rDescriptor.has( DataAccessDescriptorProperty::Cursor ) )
        {
            Reference< XResultSet > xResultSet( rDescriptor[ DataAccessDescriptorProperty::Cursor ], UNO_QUERY );
            if ( xResultSet == i_rSource.Source )
            {
                rDescriptor.erase( DataAccessDescriptorProperty::Cursor );
                // Selection and BookmarkSelection are meaningless without a result set
                if ( rDescriptor.has( DataAccessDescriptorProperty::Selection ) )
                    rDescriptor.erase( DataAccessDescriptorProperty::Selection );
                if ( rDescriptor.has( DataAccessDescriptorProperty::BookmarkSelection ) )
                    rDescriptor.erase( DataAccessDescriptorProperty::BookmarkSelection );
            }
        }

        // no matter whether it was the source connection or the source result set which died,
        // we cannot provide the data anymore.
        ClearFormats();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
