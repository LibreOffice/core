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




#ifndef _CONNECTIVITY_MAB_TYPECONVERTER_HXX_
#define _CONNECTIVITY_MAB_TYPECONVERTER_HXX_

#include <rtl/ustring.hxx>

#include<string>

namespace connectivity
{
    namespace mozab
    {
        class MTypeConverter
        {
        public:
            static void  ouStringToNsString(const ::rtl::OUString&, nsString&);
            static void  nsStringToOUString(const nsString&, ::rtl::OUString&);
            static void  prUnicharToOUString(const PRUnichar*, ::rtl::OUString&);
            // Use free() for the following 3 calls.
            static char *ouStringToCCharStringAscii(const ::rtl::OUString&);
            static char *nsStringToCCharStringAscii(const nsString&);
            static char *ouStringToCCharStringUtf8(const ::rtl::OUString&);
            // Convert to stl-string.
            static ::std::string ouStringToStlString(const ::rtl::OUString&);
            static ::std::string nsStringToStlString(const nsString&);

            static ::rtl::OUString nsACStringToOUString( const nsACString& _source );
            static ::rtl::OString nsACStringToOString( const nsACString& _source );
            static void asciiOUStringToNsACString( const ::rtl::OUString& _asciiString, nsACString& _dest );
            static void asciiToNsACString( const sal_Char* _asciiString, nsACString& _dest );

        private:
            MTypeConverter() {};
        };
    }
}

#endif // _CONNECTIVITY_MAB_TYPECONVERTER_HXX_

