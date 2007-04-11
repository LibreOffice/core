/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbaobjectex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:44:00 $
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
#define SVX_DBAOBJECTEX_HXX

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= OComponentTransferable
    //====================================================================
    class SVX_DLLPUBLIC OComponentTransferable : public TransferableHelper
    {
    protected:
        ODataAccessDescriptor   m_aDescriptor;

    public:
        /** construct the transferable
        */
        OComponentTransferable(
            const ::rtl::OUString&  _rDatasourceOrLocation
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent
        );

        /** checks whether or not a component descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
        */
        static sal_Bool canExtractComponentDescriptor(const DataFlavorExVector& _rFlavors,sal_Bool _bForm );

        /** extracts a component descriptor from the transferable given
        */
        static sal_Bool extractComponentDescriptor(
            const TransferableDataHelper&   _rData
            ,sal_Bool _bExtractForm
            ,::rtl::OUString&               _rDatasourceOrLocation
            ,::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent
        );

        /** extracts a component descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractComponentDescriptor(const TransferableDataHelper& _rData);

    protected:
        // TransferableHelper overridables
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

        static sal_uInt32   getDescriptorFormatId(sal_Bool _bExtractForm);
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_DBAOBJECTEX_HXX

