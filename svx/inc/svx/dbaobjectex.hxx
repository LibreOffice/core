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

#ifndef SVX_DBAOBJECTEX_HXX
#define SVX_DBAOBJECTEX_HXX

#include <svtools/transfer.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "svx/svxdllapi.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
