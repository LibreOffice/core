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
#include "precompiled_sc.hxx"

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "global.hxx"
#include "addincol.hxx"
#include "addincfg.hxx"
#include "scmod.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//==================================================================

#define CFGPATH_ADDINS  "Office.CalcAddIns/AddInInfo"

ScAddInCfg::ScAddInCfg() :
    ConfigItem( rtl::OUString::createFromAscii( CFGPATH_ADDINS ) )
{
    uno::Sequence<rtl::OUString> aNames(1);     // one entry: empty string
    EnableNotification( aNames );
}

void ScAddInCfg::Commit()
{
    DBG_ERROR("ScAddInCfg shouldn't be modified");
}

void ScAddInCfg::Notify( const uno::Sequence<rtl::OUString>& )
{
    // forget all add-in information, re-initialize when needed next time
    ScGlobal::GetAddInCollection()->Clear();

    // function list must also be rebuilt, but can't be modified while function
    // autopilot is open (function list for autopilot is then still old)
    if ( SC_MOD()->GetCurRefDlgId() != SID_OPENDLG_FUNCTION )
        ScGlobal::ResetFunctionList();
}


