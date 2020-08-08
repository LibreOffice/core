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

#include <JoinExchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::datatransfer;

    void OJoinExchObj::setDescriptors(const OJoinExchangeData& jxdSource,bool _bFirstEntry)
    {
        m_bFirstEntry = _bFirstEntry;
        m_jxdSourceDescription = jxdSource;
    }

    OJoinExchObj::OJoinExchObj()
        : m_bFirstEntry(false)
        , m_pDragListener(nullptr)
    {
    }

    OJoinExchObj::~OJoinExchObj()
    {
    }

    void OJoinExchObj::DragFinished( sal_Int8 /*nDropAction*/ )
    {
        if (m_pDragListener)
            m_pDragListener->dragFinished();
        m_pDragListener = nullptr;
    }

    bool OJoinExchObj::isFormatAvailable( const DataFlavorExVector& _rFormats ,SotClipboardFormatId _nSlotID)
    {
        for (auto const& format : _rFormats)
        {
            if ( _nSlotID == format.mnSotId )
                return true;
        }
        return false;
    }

    OJoinExchangeData OJoinExchObj::GetSourceDescription(const Reference< XTransferable >& _rxObject)
    {
        OJoinExchangeData aReturn;
        auto pImplementation = comphelper::getUnoTunnelImplementation<OJoinExchObj>(_rxObject);
        if (pImplementation)
            aReturn = pImplementation->m_jxdSourceDescription;
        return aReturn;
    }

    Sequence< sal_Int8 > OJoinExchObj::getUnoTunnelId()
    {
        static ::cppu::OImplementationId implId;

        return implId.getImplementationId();
    }

    sal_Int64 SAL_CALL OJoinExchObj::getSomething( const Sequence< sal_Int8 >& _rIdentifier )
    {
        if (isUnoTunnelId<OJoinExchObj>(_rIdentifier))
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
            // we don't really copy our data, the instances using us have to call GetSourceDescription...
            // if, one day, we have a _lot_ of time, this hack should be removed...
            return true;

        return false;
    }

    Any SAL_CALL OJoinExchObj::queryInterface( const Type& _rType )
    {
        Any aReturn = TransferDataContainer::queryInterface(_rType);
        if (!aReturn.hasValue())
            aReturn = OJoinExchObj_Base::queryInterface(_rType);
        return aReturn;
    }

    void SAL_CALL OJoinExchObj::acquire(  ) throw()
    {
        TransferDataContainer::acquire( );
    }

    void SAL_CALL OJoinExchObj::release(  ) throw()
    {
        TransferDataContainer::release( );
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
