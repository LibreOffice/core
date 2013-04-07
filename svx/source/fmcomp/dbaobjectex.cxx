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

#include <svx/dbaobjectex.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include "fmprop.hrc"
#include <comphelper/extract.hxx>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <comphelper/propertysetinfo.hxx>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::comphelper;

    //====================================================================
    //= OComponentTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OComponentTransferable::OComponentTransferable(const OUString& _rDatasourceOrLocation
            ,const Reference< XContent>& _xContent)
    {
        m_aDescriptor.setDataSource(_rDatasourceOrLocation);
        m_aDescriptor[daComponent] <<= _xContent;
    }


    //--------------------------------------------------------------------
    sal_uInt32 OComponentTransferable::getDescriptorFormatId(sal_Bool _bExtractForm)
    {
        static sal_uInt32 s_nReportFormat = (sal_uInt32)-1;
        static sal_uInt32 s_nFormFormat = (sal_uInt32)-1;
        if ( _bExtractForm && (sal_uInt32)-1 == s_nFormFormat )
        {
            s_nFormFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"dbaccess.FormComponentDescriptorTransfer\"" ));
            OSL_ENSURE((sal_uInt32)-1 != s_nFormFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        else if ( !_bExtractForm && (sal_uInt32)-1 == s_nReportFormat)
        {
            s_nReportFormat = SotExchange::RegisterFormatName(OUString("application/x-openoffice;windows_formatname=\"dbaccess.ReportComponentDescriptorTransfer\""));
            OSL_ENSURE((sal_uInt32)-1 != s_nReportFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return _bExtractForm ? s_nFormFormat : s_nReportFormat;
    }

    //--------------------------------------------------------------------
    void OComponentTransferable::AddSupportedFormats()
    {
        sal_Bool bForm = sal_True;
        try
        {
            Reference<XPropertySet> xProp;
            m_aDescriptor[daComponent] >>= xProp;
            if ( xProp.is() )
                xProp->getPropertyValue(OUString("IsForm")) >>= bForm;
        }
        catch(Exception)
        {}
        AddFormat(getDescriptorFormatId(bForm));
    }

    //--------------------------------------------------------------------
    sal_Bool OComponentTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
        if ( nFormatId == getDescriptorFormatId(sal_True) || nFormatId == getDescriptorFormatId(sal_False) )
            return SetAny( makeAny( m_aDescriptor.createPropertyValueSequence() ), _rFlavor );

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OComponentTransferable::canExtractComponentDescriptor(const DataFlavorExVector& _rFlavors,sal_Bool _bForm )
    {
        DataFlavorExVector::const_iterator aEnd = _rFlavors.end();
        for (   DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
                aCheck != aEnd;
                ++aCheck
            )
        {
            if ( getDescriptorFormatId(_bForm) == aCheck->mnSotId )
                return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor OComponentTransferable::extractComponentDescriptor(const TransferableDataHelper& _rData)
    {
        sal_Bool bForm = _rData.HasFormat(getDescriptorFormatId(sal_True));
        if ( bForm || _rData.HasFormat(getDescriptorFormatId(sal_False)) )
        {
            // the object has a real descriptor object (not just the old compatible format)

            // extract the any from the transferable
            DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            SotExchange::GetFormatDataFlavor(getDescriptorFormatId(bForm), aFlavor);
            OSL_ENSURE(bSuccess, "OComponentTransferable::extractColumnDescriptor: invalid data format (no flavor)!");

            Any aDescriptor = _rData.GetAny(aFlavor);

            // extract the property value sequence
            Sequence< PropertyValue > aDescriptorProps;
#if OSL_DEBUG_LEVEL > 0
            bSuccess =
#endif
            aDescriptor >>= aDescriptorProps;
            OSL_ENSURE(bSuccess, "OComponentTransferable::extractColumnDescriptor: invalid clipboard format!");

            // build the real descriptor
            return ODataAccessDescriptor(aDescriptorProps);
        }

        return ODataAccessDescriptor();
    }

//........................................................................
}   // namespace svx
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
