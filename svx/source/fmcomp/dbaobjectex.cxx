/*************************************************************************
 *
 *  $RCSfile: dbaobjectex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:42:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    sal_uInt32 OComponentTransferable::getDescriptorFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName(String::CreateFromAscii("application/x-openoffice;windows_formatname=\"dbaccess.ComponentDescriptorTransfer\""));
            OSL_ENSURE((sal_uInt32)-1 != s_nFormat, "OComponentTransferable::getDescriptorFormatId: bad exchange id!");
        }
        return s_nFormat;
    }

    //--------------------------------------------------------------------
    void OComponentTransferable::AddSupportedFormats()
    {
        AddFormat(getDescriptorFormatId());
    }

    //--------------------------------------------------------------------
    sal_Bool OComponentTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat(_rFlavor);
        if ( nFormatId == getDescriptorFormatId() )
            return SetAny( makeAny( m_aDescriptor.createPropertyValueSequence() ), _rFlavor );

        return sal_False;
    }

    //--------------------------------------------------------------------
    sal_Bool OComponentTransferable::canExtractComponentDescriptor(const DataFlavorExVector& _rFlavors )
    {
        DataFlavorExVector::const_iterator aEnd = _rFlavors.end();
        for (   DataFlavorExVector::const_iterator aCheck = _rFlavors.begin();
                aCheck != aEnd;
                ++aCheck
            )
        {
            if ( getDescriptorFormatId() == aCheck->mnSotId )
                return sal_True;
        }

        return sal_False;
    }

    //--------------------------------------------------------------------
    ODataAccessDescriptor OComponentTransferable::extractComponentDescriptor(const TransferableDataHelper& _rData)
    {
        if ( _rData.HasFormat(getDescriptorFormatId()) )
        {
            // the object has a real descriptor object (not just the old compatible format)

            // extract the any from the transferable
            DataFlavor aFlavor;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            SotExchange::GetFormatDataFlavor(getDescriptorFormatId(), aFlavor);
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
        , ::rtl::OUString&  _rDatasourceOrLocation
        , ::com::sun::star::uno::Reference< XContent>& _xContent)
    {
        if (_rData.HasFormat(getDescriptorFormatId()))
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


