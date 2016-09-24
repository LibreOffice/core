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

#include "JoinExchange.hxx"
#include <sot/formats.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::datatransfer;

    OUString OJoinExchObj::m_sJoinFormat;

    // class OJoinExchObj
    OJoinExchObj::OJoinExchObj(const OJoinExchangeData& jxdSource,bool _bFirstEntry)
        :m_bFirstEntry(_bFirstEntry)
        ,m_jxdSourceDescription(jxdSource)
        ,m_pDragListener(nullptr)
    {
        // add available types to list
    }

    OJoinExchObj::~OJoinExchObj()
    {
    }

    void OJoinExchObj::StartDrag( vcl::Window* _pWindow, sal_Int8 _nDragSourceActions, IDragTransferableListener* _pListener )
    {
        m_pDragListener = _pListener;
        TransferableHelper::StartDrag(_pWindow, _nDragSourceActions);
    }

    void OJoinExchObj::DragFinished( sal_Int8 /*nDropAction*/ )
    {
        if (m_pDragListener)
            m_pDragListener->dragFinished();
        m_pDragListener = nullptr;
    }

    bool OJoinExchObj::isFormatAvailable( const DataFlavorExVector& _rFormats ,SotClipboardFormatId _nSlotID)
    {
        DataFlavorExVector::const_iterator aCheckEnd = _rFormats.end();
        for (   DataFlavorExVector::const_iterator aCheck = _rFormats.begin();
                aCheck != aCheckEnd;
                ++aCheck
            )
        {
            if ( _nSlotID == aCheck->mnSotId )
                return true;
        }
        return false;
    }

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

    Sequence< sal_Int8 > OJoinExchObj::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId * pId = nullptr;
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

    sal_Int64 SAL_CALL OJoinExchObj::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException, std::exception)
    {
        if (_rIdentifier.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
            return reinterpret_cast<sal_Int64>(this);

        return 0;
    }

    void OJoinExchObj::AddSupportedFormats()
    {
        AddFormat( SotClipboardFormatId::SBA_JOIN );
        if ( m_bFirstEntry )
            AddFormat( SotClipboardFormatId::SBA_TABID );
    }

    bool OJoinExchObj::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
    {
        SotClipboardFormatId nFormat = SotExchange::GetFormat(rFlavor);
        if ( SotClipboardFormatId::SBA_JOIN == nFormat )
            // this is a HACK
            // we don't really copy our data, the instances using us have to call GetSourceDescription ....
            // if, one day, we have a _lot_ of time, this hack should be removed ....
            return true;

        return false;
    }

    Any SAL_CALL OJoinExchObj::queryInterface( const Type& _rType ) throw(RuntimeException, std::exception)
    {
        Any aReturn = TransferableHelper::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = OJoinExchObj_Base::queryInterface(_rType);
        return aReturn;
    }

    void SAL_CALL OJoinExchObj::acquire(  ) throw()
    {
        TransferableHelper::acquire( );
    }

    void SAL_CALL OJoinExchObj::release(  ) throw()
    {
        TransferableHelper::release( );
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
