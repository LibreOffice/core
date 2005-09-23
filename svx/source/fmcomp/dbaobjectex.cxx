/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbaobjectex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:58:40 $
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

#ifndef SVX_DBAOBJECTEX_HXX
#include "dbaobjectex.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

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
    using namespace ::svxform;
    using namespace ::comphelper;

    //====================================================================
    //= OComponentTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OComponentTransferable::OComponentTransferable(const ::rtl::OUString& _rDatasourceOrLocation
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
            s_nFormFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"dbaccess.FormComponentDescriptorTransfer\"" ));
            OSL_ENSURE((sal_uInt32)-1 != s_nFormFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        else if ( !_bExtractForm && (sal_uInt32)-1 == s_nReportFormat)
        {
            s_nReportFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"dbaccess.ReportComponentDescriptorTransfer\""));
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
                xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsForm"))) >>= bForm;
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
        sal_Bool bForm;
        if ( (bForm = _rData.HasFormat(getDescriptorFormatId(sal_True))) || _rData.HasFormat(getDescriptorFormatId(sal_False)) )
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

    //--------------------------------------------------------------------
    sal_Bool OComponentTransferable::extractComponentDescriptor(const TransferableDataHelper& _rData
        ,sal_Bool _bExtractForm
        , ::rtl::OUString&  _rDatasourceOrLocation
        , ::com::sun::star::uno::Reference< XContent>& _xContent)
    {
        if ( _rData.HasFormat( getDescriptorFormatId(_bExtractForm)) )
        {
            ODataAccessDescriptor aDescriptor = extractComponentDescriptor(_rData);
            _rDatasourceOrLocation = aDescriptor.getDataSource();
            aDescriptor[daComponent]            >>= _xContent;
            return sal_True;
        }

        return sal_False;
    }
//........................................................................
}   // namespace svx
//........................................................................


