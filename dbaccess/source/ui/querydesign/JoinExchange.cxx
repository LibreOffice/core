/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JoinExchange.cxx,v $
 * $Revision: 1.12 $
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
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SVX_DBEXCH_HRC
#include <svx/dbexch.hrc>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::datatransfer;

    String OJoinExchObj::m_sJoinFormat;
    //==================================================================
    // class OJoinExchObj
    //==================================================================
    DBG_NAME(OJoinExchObj)
    //------------------------------------------------------------------------
    OJoinExchObj::OJoinExchObj(const OJoinExchangeData& jxdSource,sal_Bool _bFirstEntry)
        :m_bFirstEntry(_bFirstEntry)
        ,m_jxdSourceDescription(jxdSource)
        ,m_pDragListener(NULL)
    {
        DBG_CTOR(OJoinExchObj,NULL);
        // Verfuegbare Typen in Liste einfuegen
    }

    //------------------------------------------------------------------------
    OJoinExchObj::~OJoinExchObj()
    {
        DBG_DTOR(OJoinExchObj,NULL);
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::StartDrag( Window* _pWindow, sal_Int8 _nDragSourceActions, IDragTransferableListener* _pListener )
    {
        m_pDragListener = _pListener;
        TransferableHelper::StartDrag(_pWindow, _nDragSourceActions);
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::DragFinished( sal_Int8 /*nDropAction*/ )
    {
        if (m_pDragListener)
            m_pDragListener->dragFinished();
        m_pDragListener = NULL;
    }

    //------------------------------------------------------------------------
    sal_Bool OJoinExchObj::isFormatAvailable( const DataFlavorExVector& _rFormats ,SotFormatStringId _nSlotID)
    {
        DataFlavorExVector::const_iterator aCheckEnd = _rFormats.end();
        for (   DataFlavorExVector::const_iterator aCheck = _rFormats.begin();
                aCheck != aCheckEnd;
                ++aCheck
            )
        {
            if ( _nSlotID == aCheck->mnSotId )
                return sal_True;
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    OJoinExchangeData OJoinExchObj::GetSourceDescription(const Reference< XTransferable >& _rxObject)
    {
        OJoinExchangeData aReturn;
        Reference< XUnoTunnel > xTunnel(_rxObject, UNO_QUERY);
        if (xTunnel.is())
        {
            OJoinExchObj* pImplementation = reinterpret_cast<OJoinExchObj*>(xTunnel->getSomething(getUnoTunnelImplementationId()));
            if (pImplementation)
                aReturn = pImplementation->m_jxdSourceDescription;
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    Sequence< sal_Int8 > OJoinExchObj::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //------------------------------------------------------------------------
    sal_Int64 SAL_CALL OJoinExchObj::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
    {
        if (_rIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
            return reinterpret_cast<sal_Int64>(this);

        return 0;
    }

    //------------------------------------------------------------------------
    void OJoinExchObj::AddSupportedFormats()
    {
        AddFormat( SOT_FORMATSTR_ID_SBA_JOIN );
        if ( m_bFirstEntry )
            AddFormat( SOT_FORMATSTR_ID_SBA_TABID );
    }

    //------------------------------------------------------------------------
    sal_Bool OJoinExchObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        sal_uInt32 nFormat = SotExchange::GetFormat(rFlavor);
        if ( SOT_FORMATSTR_ID_SBA_JOIN == nFormat )
            // this is a HACK
            // we don't really copy our data, the instances using us have to call GetSourceDescription ....
            // if, one day, we have a _lot_ of time, this hack should be removed ....
            return sal_True;

        return sal_False;
    }

    //------------------------------------------------------------------------
    Any SAL_CALL OJoinExchObj::queryInterface( const Type& _rType ) throw(RuntimeException)
    {
        Any aReturn = TransferableHelper::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = OJoinExchObj_Base::queryInterface(_rType);
        return aReturn;
    }

    //------------------------------------------------------------------------
    void SAL_CALL OJoinExchObj::acquire(  ) throw()
    {
        TransferableHelper::acquire( );
    }

    //------------------------------------------------------------------------
    void SAL_CALL OJoinExchObj::release(  ) throw()
    {
        TransferableHelper::release( );
    }


}   // namespace dbaui
