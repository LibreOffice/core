/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbexchange.cxx,v $
 * $Revision: 1.28 $
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

#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
//#ifndef _COMPHELPER_EXTRACT_HXX_
//#include <comphelper/extract.hxx>
//#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif


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
        template<class T > void lcl_addListener(const Reference<T>& _xComponent,ODataClipboard* _pListener)
        {
            if ( _xComponent.is() )
            {
                Reference< XComponent> xCom(_xComponent,UNO_QUERY);
                if ( xCom.is() )
                    xCom->addEventListener(Reference< XEventListener>((::cppu::OWeakObject*)_pListener,UNO_QUERY));
            }
        }
        template<class T > void lcl_removeListener(const Reference<T>& _xComponent,ODataClipboard* _pListener)
        {
            if ( _xComponent.is() )
            {
                Reference< XComponent> xCom(_xComponent,UNO_QUERY);
                if ( xCom.is() )
                    xCom->removeEventListener(Reference< XEventListener>((::cppu::OWeakObject*)_pListener,UNO_QUERY));
            }
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
        lcl_addListener(_rxConnection,this);

        m_pHtml = new OHTMLImportExport(getDescriptor(), _rxORB, _rxFormatter);
        m_aEventListeners.push_back(m_pHtml);

        m_pRtf = new ORTFImportExport(getDescriptor(), _rxORB, _rxFormatter);
        m_aEventListeners.push_back(m_pRtf);
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
        m_pHtml = new OHTMLImportExport(getDescriptor(),_rxORB, _rxFormatter);
        m_aEventListeners.push_back(m_pHtml);

        m_pRtf = new ORTFImportExport(getDescriptor(),_rxORB, _rxFormatter);
        m_aEventListeners.push_back(m_pRtf);
    }

    // -----------------------------------------------------------------------------
    ODataClipboard::ODataClipboard( const Reference< XPropertySet >& _rxLivingForm,
                                    const Sequence< Any >& _rSelectedRows,
                                    const Reference< XResultSet>& _rxResultSet)
        :ODataAccessObjectTransferable( _rxLivingForm )
        ,m_pHtml(NULL)
        ,m_pRtf(NULL)
    {
        osl_incrementInterlockedCount( &m_refCount );

        Reference<XConnection> xConnection;
        getDescriptor()[daConnection] >>= xConnection;
        lcl_addListener(xConnection,this);
        lcl_addListener(_rxResultSet,this);

        getDescriptor()[daSelection]        <<= _rSelectedRows;
        getDescriptor()[daBookmarkSelection]<<= sal_False;  // by definition, it's the indicies
        getDescriptor()[daCursor]           <<= _rxResultSet;
        addCompatibleSelectionDescription( _rSelectedRows );

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
        // RTF?
        if (m_pRtf)
            AddFormat(SOT_FORMAT_RTF);

        // HTML?
        if (m_pHtml)
            AddFormat(SOT_FORMATSTR_ID_HTML);

        ODataAccessObjectTransferable::AddSupportedFormats();
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::GetData( const DataFlavor& rFlavor )
    {
        const ULONG nFormat = SotExchange::GetFormat(rFlavor);
        switch (nFormat)
        {
            case SOT_FORMAT_RTF:
                if ( m_pRtf )
                    m_pRtf->initialize(getDescriptor());
                return m_pRtf && SetObject(m_pRtf, SOT_FORMAT_RTF, rFlavor);
            case SOT_FORMATSTR_ID_HTML:
                if ( m_pHtml )
                    m_pHtml->initialize(getDescriptor());
                return m_pHtml && SetObject(m_pHtml, SOT_FORMATSTR_ID_HTML, rFlavor);
        }

        return ODataAccessObjectTransferable::GetData( rFlavor );
    }

    // -----------------------------------------------------------------------------
    void ODataClipboard::ObjectReleased()
    {
        if ( m_pHtml )
        {
            m_pHtml->dispose();
            m_pHtml = NULL;
        } // if ( m_pHtml )
        if ( m_pRtf )
        {
            m_pRtf->dispose();
            m_pRtf = NULL;
        }
        m_aEventListeners.clear();
        Reference<XConnection> xConnection;
        Reference<XResultSet> xProp;
        if ( getDescriptor().has(daConnection) && (getDescriptor()[daConnection] >>= xConnection) )
            lcl_removeListener(xConnection,this);
        if ( getDescriptor().has(daCursor) && (getDescriptor()[daCursor] >>= xProp) )
            lcl_removeListener(xProp,this);

        ODataAccessObjectTransferable::ObjectReleased( );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL ODataClipboard::disposing( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
    {
        Reference<XConnection> xConnection;
        Reference<XResultSet> xProp;
        if ( getDescriptor().has(daConnection) && (getDescriptor()[daConnection] >>= xConnection) )
        {
            lcl_removeListener(xConnection,this);
            getDescriptor().erase(daConnection);
        } // if ( getDescriptor().has(daConnection) && (getDescriptor()[daConnection] >>= xConnection) )
        if ( getDescriptor().has(daCursor) && (getDescriptor()[daCursor] >>= xProp) )
        {
            lcl_removeListener(xProp,this);
            getDescriptor().erase(daCursor);
        } // if ( getDescriptor().has(daCursor) && (getDescriptor()[daCursor] >>= xProp) )

        if ( getDescriptor().has(daColumnObject) )
            getDescriptor().erase(daColumnObject);

        if ( getDescriptor().has(daComponent) )
            getDescriptor().erase(daComponent);


        ClearFormats();
        //getDescriptor().clear();
        AddSupportedFormats();

        /*m_pHtml = NULL;
        m_pRtf = NULL;
        m_aEventListeners.clear();*/
    }
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( ODataClipboard, ODataAccessObjectTransferable, TDataClipboard_BASE )
}


