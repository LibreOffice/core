/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

