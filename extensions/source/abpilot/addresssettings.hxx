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



#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#define EXTENSIONS_ABP_ADDRESSSETTINGS_HXX

#include <rtl/ustring.hxx>
#include "abptypes.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= AddressSourceType
    //=====================================================================
    enum AddressSourceType
    {
        AST_MORK,
        AST_THUNDERBIRD,
        AST_EVOLUTION,
        AST_EVOLUTION_GROUPWISE,
        AST_EVOLUTION_LDAP,
        AST_KAB,
        AST_MACAB,
        AST_LDAP,
        AST_OUTLOOK,
        AST_OE,

        AST_OTHER,

        AST_INVALID
    };

    //=====================================================================
    //= AddressSettings
    //=====================================================================
    struct AddressSettings
    {
        AddressSourceType   eType;
        ::rtl::OUString     sDataSourceName;
        ::rtl::OUString     sRegisteredDataSourceName;
        ::rtl::OUString     sSelectedTable;
        bool                bIgnoreNoTable;
        MapString2String    aFieldMapping;
        bool                bRegisterDataSource;
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADDRESSSETTINGS_HXX

