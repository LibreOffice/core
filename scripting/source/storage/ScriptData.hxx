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



#ifndef _SCRIPTING_STORAGE_SCRIPTDATA_HXX_
#define _SCRIPTING_STORAGE_SCRIPTDATA_HXX_

#include <vector>
#include <hash_map>

#include <cppu/macros.hxx>
#include <rtl/ustring.hxx>



namespace scripting_impl
{

typedef ::std::pair< ::rtl::OUString, ::rtl::OUString > str_pair;
typedef ::std::vector< str_pair > props_vec;
typedef ::std::hash_map< ::rtl::OUString, props_vec, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > strpairvec_map;
typedef ::std::hash_map< ::rtl::OUString, ::std::pair< ::rtl::OUString,
    ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > strpair_map;
typedef ::std::hash_map< ::rtl::OUString, ::std::pair< props_vec, strpairvec_map >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > filesets_map;

struct ScriptData
{

    inline ScriptData::ScriptData() SAL_THROW( () )
       : parcelURI()
       , language()
       , locales()
       , functionname()
       , logicalname()
       , languagedepprops()
       , filesets()
   {
   }

   inline ScriptData::ScriptData( const ::rtl::OUString __parcelURI,
       const ::rtl::OUString& __language,
       const strpair_map& __locales,
       const ::rtl::OUString& __functionname,
       const ::rtl::OUString& __logicalname,
       const props_vec& __languagedepprops,
       const filesets_map& __filesets ) SAL_THROW( () )
       : parcelURI( __parcelURI )
       , language( __language )
       , locales( __locales )
       , functionname( __functionname )
       , logicalname( __logicalname )
       , languagedepprops( __languagedepprops )
       , filesets( __filesets )
   {
   }

   ::rtl::OUString parcelURI;
   ::rtl::OUString language;
   strpair_map locales;
   ::rtl::OUString functionname;
   ::rtl::OUString logicalname;
   props_vec languagedepprops;
   filesets_map filesets;
};
} // namespace scripting_impl
#endif // _SCRIPTING_STORAGE_ScriptData_HXX_

