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



#ifndef INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX
#define INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX

#include <rtl/ustring.hxx>

namespace
{
    static const ::rtl::OUString s_sData                     = ::rtl::OUString::createFromAscii("org.openoffice.UserProfile/Data");
    static const ::rtl::OUString s_so                        = ::rtl::OUString::createFromAscii("o"); // USER_OPT_COMPANY
    static const ::rtl::OUString s_sgivenname                = ::rtl::OUString::createFromAscii("givenname"); // USER_OPT_FIRSTNAME
    static const ::rtl::OUString s_ssn                       = ::rtl::OUString::createFromAscii("sn"); // USER_OPT_LASTNAME
    static const ::rtl::OUString s_sinitials                 = ::rtl::OUString::createFromAscii("initials"); // USER_OPT_ID
    static const ::rtl::OUString s_sstreet                   = ::rtl::OUString::createFromAscii("street"); // USER_OPT_STREET
    static const ::rtl::OUString s_sl                        = ::rtl::OUString::createFromAscii("l"); // USER_OPT_CITY
    static const ::rtl::OUString s_sst                       = ::rtl::OUString::createFromAscii("st"); // USER_OPT_STATE
    static const ::rtl::OUString s_spostalcode               = ::rtl::OUString::createFromAscii("postalcode"); // USER_OPT_ZIP
    static const ::rtl::OUString s_sc                        = ::rtl::OUString::createFromAscii("c"); // USER_OPT_COUNTRY
    static const ::rtl::OUString s_stitle                    = ::rtl::OUString::createFromAscii("title"); // USER_OPT_TITLE
    static const ::rtl::OUString s_sposition                 = ::rtl::OUString::createFromAscii("position"); // USER_OPT_POSITION
    static const ::rtl::OUString s_shomephone                = ::rtl::OUString::createFromAscii("homephone"); // USER_OPT_TELEPHONEHOME
    static const ::rtl::OUString s_stelephonenumber          = ::rtl::OUString::createFromAscii("telephonenumber"); // USER_OPT_TELEPHONEWORK
    static const ::rtl::OUString s_sfacsimiletelephonenumber = ::rtl::OUString::createFromAscii("facsimiletelephonenumber"); // USER_OPT_FAX
    static const ::rtl::OUString s_smail                     = ::rtl::OUString::createFromAscii("mail"); // USER_OPT_EMAIL
    static const ::rtl::OUString s_scustomernumber           = ::rtl::OUString::createFromAscii("customernumber"); // USER_OPT_CUSTOMERNUMBER
    static const ::rtl::OUString s_sfathersname              = ::rtl::OUString::createFromAscii("fathersname"); // USER_OPT_FATHERSNAME
    static const ::rtl::OUString s_sapartment                = ::rtl::OUString::createFromAscii("apartment"); // USER_OPT_APARTMENT
}

#endif //  INCLUDE_CONFIGITEMS_USEROPTIONS_CONST_HXX
