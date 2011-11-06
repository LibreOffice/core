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


#ifndef RPTUI_GROUP_EXCHANGE_HXX
#define RPTUI_GROUP_EXCHANGE_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase2.hxx>
#include <svtools/transfer.hxx>
#include "GroupsSorting.hxx"

namespace rptui
{
    /** clipboard class for group rows in the groups and sorting dialog
    */
    class OGroupExchange : public TransferableHelper
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> m_aGroupRow;
    public:
        OGroupExchange(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& _aGroupRow);

        static sal_uInt32   getReportGroupId();
    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
    };
}
#endif // RPTUI_GROUP_EXCHANGE_HXX

