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
#include "precompiled_dbmm.hxx"

#include "dbmm_types.hxx"

#include "dbmm_global.hrc"
#include "dbmm_module.hxx"

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    String getScriptTypeDisplayName( const ScriptType _eType )
    {
        sal_uInt16 nResId( 0 );

        switch ( _eType )
        {
        case eBasic:        nResId = STR_OOO_BASIC;     break;
        case eBeanShell:    nResId = STR_BEAN_SHELL;    break;
        case eJavaScript:   nResId = STR_JAVA_SCRIPT;   break;
        case ePython:       nResId = STR_PYTHON;        break;
        case eJava:         nResId = STR_JAVA;          break;
        case eDialog:       nResId = STR_DIALOG;        break;
        }
        OSL_ENSURE( nResId != 0, "getScriptTypeDisplayName: illegal script type!" );
        return nResId ? String( MacroMigrationResId( nResId ) ) : String();
    }

//........................................................................
} // namespace dbmm
//........................................................................
