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



#ifndef _CONNECTIVITY_MACAB_HEADER_HXX_
#define _CONNECTIVITY_MACAB_HEADER_HXX_

#include "MacabRecord.hxx"
#include <cppuhelper/compbase3.hxx>

namespace connectivity
{
    namespace macab
    {
        class MacabHeader: public MacabRecord{
            protected:
                macabfield **sortRecord(sal_Int32 _start, sal_Int32 _length);
            public:
                MacabHeader();
                MacabHeader(const sal_Int32 _size, macabfield **_fields);
                virtual ~MacabHeader();
                void operator+= (const MacabHeader *r);
                ::rtl::OUString getString(const sal_Int32 i) const;
                void sortRecord();
                sal_Int32 getColumnNumber(const ::rtl::OUString s) const;

                static sal_Int32 compareFields(const macabfield *_field1, const macabfield *_field2);

                MacabHeader *begin();
                sal_Int32 end() const;
                class iterator{
                    protected:
                        sal_Int32 id;
                        MacabHeader *record;
                    public:
                        void operator= (MacabHeader *_record);
                        iterator();
                        ~iterator();
                        void operator++ ();
                        sal_Bool operator!= (const sal_Int32 i) const;
                        sal_Bool operator== (const sal_Int32 i) const;
                        macabfield *operator* () const;
                };
        };
    }
}

#endif // _CONNECTIVITY_MACAB_HEADER_HXX_
