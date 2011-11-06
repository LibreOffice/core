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


#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#define DBAUI_QUERY_TABLEWINDOWDATA_HXX

#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef INCLUDED_VECTOR
#define INCLUDED_VECTOR
#include <vector>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif


namespace dbaui
{
    class OQueryTableWindowData : public OTableWindowData
    {
    public:
        explicit OQueryTableWindowData(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rTableName, const ::rtl::OUString& rTableAlias);
        virtual ~OQueryTableWindowData();

        ::rtl::OUString GetAliasName() { return GetWinName(); }
        void SetAliasName(const ::rtl::OUString& rNewAlias) { SetWinName(rNewAlias); }
    };
}
#endif // DBAUI_QUERY_TABLEWINDOWDATA_HXX


