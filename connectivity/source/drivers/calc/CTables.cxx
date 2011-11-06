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
#include "precompiled_connectivity.hxx"
#include "calc/CTables.hxx"
#include "calc/CTable.hxx"
#include "file/FCatalog.hxx"
#ifndef _CONNECTIVITY_FILE_BCONNECTION_HXX_
#include "file/FConnection.hxx"
#endif
#include "calc/CCatalog.hxx"
#include <comphelper/types.hxx>
#include <rtl/logfile.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
namespace starutil      = ::com::sun::star::util;

sdbcx::ObjectType OCalcTables::createObject(const ::rtl::OUString& _rName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcTables::createObject" );
    OCalcTable* pTable = new OCalcTable(this,(OCalcConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection(),
                                        _rName,::rtl::OUString::createFromAscii("TABLE"));
    sdbcx::ObjectType xRet = pTable;
    pTable->construct();
    return xRet;
}
// -------------------------------------------------------------------------

