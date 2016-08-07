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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindowData.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace dbaui;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

DBG_NAME(OQueryTableWindowData)
//==================================================================
// class OQueryTableWindowData
//==================================================================
//------------------------------------------------------------------------------
OQueryTableWindowData::OQueryTableWindowData(const ::rtl::OUString& _rComposedName, const ::rtl::OUString& rTableName, const ::rtl::OUString& rTableAlias )
    :OTableWindowData(NULL,_rComposedName, rTableName, rTableAlias)
{
    DBG_CTOR(OQueryTableWindowData,NULL);
}

//------------------------------------------------------------------------------
OQueryTableWindowData::~OQueryTableWindowData()
{
    DBG_DTOR(OQueryTableWindowData,NULL);
}
// -----------------------------------------------------------------------------




