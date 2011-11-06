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



#ifndef _EXTENSIONS_DBW_WIZARDSERVICES_HXX_
#define _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OGroupBoxSI
    //=====================================================================
    /// service info for the group box wizard
    struct OGroupBoxSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

    //=====================================================================
    //= OListComboSI
    //=====================================================================
    /// service info for the list/combo box wizard
    struct OListComboSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

    //=====================================================================
    //= OGridSI
    //=====================================================================
    /// service info for the grid wizard
    struct OGridSI
    {
    public:
        ::rtl::OUString         getImplementationName() const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                getServiceNames() const;
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBW_WIZARDSERVICES_HXX_

