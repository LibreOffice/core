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


#ifndef _DAVPROPERTIES_HXX_
#define _DAVPROPERTIES_HXX_

#include <rtl/ustring.hxx>
#include "SerfTypes.hxx"

namespace http_dav_ucp
{

struct DAVProperties
{
    static const ::rtl::OUString CREATIONDATE;
    static const ::rtl::OUString DISPLAYNAME;
    static const ::rtl::OUString GETCONTENTLANGUAGE;
    static const ::rtl::OUString GETCONTENTLENGTH;
    static const ::rtl::OUString GETCONTENTTYPE;
    static const ::rtl::OUString GETETAG;
    static const ::rtl::OUString GETLASTMODIFIED;
    static const ::rtl::OUString LOCKDISCOVERY;
    static const ::rtl::OUString RESOURCETYPE;
    static const ::rtl::OUString SUPPORTEDLOCK;
    static const ::rtl::OUString EXECUTABLE;

    static void createSerfPropName( const rtl::OUString & rFullName,
                                    SerfPropName & rName );
    static void createUCBPropName ( const char * nspace,
                                    const char * name,
                                    rtl::OUString & rFullName );

    static bool isUCBDeadProperty( const SerfPropName & rName );
    static bool isUCBSpecialProperty( const rtl::OUString & rFullName,
                                      rtl::OUString & rParsedName );
};

} // namespace http_dav_ucp

#endif // _DAVPROPERTIES_HXX_
