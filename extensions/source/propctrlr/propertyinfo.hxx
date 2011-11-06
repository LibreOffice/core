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



#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

#include <sal/types.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <vector>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= IPropertyInfoService
    //========================================================================
    class SAL_NO_VTABLE IPropertyInfoService
    {
    public:
        virtual sal_Int32                           getPropertyId(const String& _rName) const = 0;
        virtual String                              getPropertyTranslation(sal_Int32 _nId) const = 0;
        virtual rtl::OString                        getPropertyHelpId(sal_Int32 _nId) const = 0;
        virtual sal_Int16                           getPropertyPos(sal_Int32 _nId) const = 0;
        virtual sal_uInt32                          getPropertyUIFlags(sal_Int32 _nId) const = 0;
        virtual ::std::vector< ::rtl::OUString >    getPropertyEnumRepresentations(sal_Int32 _nId) const = 0;

        // this is only temporary, until the UNOization of the property browser is completed
        virtual String                  getPropertyName( sal_Int32 _nPropId ) = 0;

        virtual ~IPropertyInfoService() { }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_

