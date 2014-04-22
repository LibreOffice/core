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


#ifndef __MNSINIPARSER_HXX__
#define __MNSINIPARSER_HXX__

#include <rtl/ustring.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <osl/process.h>
using namespace rtl;

#include <map>
#include <list>

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

struct ini_NameValue
{
    rtl::OUString sName;
    rtl::OUString sValue;

    inline ini_NameValue() SAL_THROW( () )
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW( () )
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    ini_NameValue
> NameValueList;

struct ini_Section
{
    rtl::OUString sName;
    NameValueList lList;
};
typedef std::map<rtl::OUString,
                ini_Section
                >IniSectionMap;


class IniParser
{
    IniSectionMap mAllSection;
public:
    IniSectionMap * getAllSection(){return &mAllSection;};
    IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException );
#if OSL_DEBUG_LEVEL > 0
    void Dump();
#endif

};

#endif

