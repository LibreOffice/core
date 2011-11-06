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
#include "calc/CPreparedStatement.hxx"
#include "calc/CResultSet.hxx"

using namespace connectivity::calc;
using namespace connectivity::file;
using namespace com::sun::star::uno;
// -------------------------------------------------------------------------
OResultSet* OCalcPreparedStatement::createResultSet()
{
    return new OCalcResultSet(this,m_aSQLIterator);
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OCalcPreparedStatement,"com.sun.star.sdbc.driver.calc.PreparedStatement","com.sun.star.sdbc.PreparedStatement");

