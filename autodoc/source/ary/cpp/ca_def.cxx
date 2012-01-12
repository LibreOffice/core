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

#include <precomp.h>
#include "ca_def.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/cpp/inpcontx.hxx>




namespace ary
{
namespace cpp
{


DefAdmin::DefAdmin(RepositoryPartition & io_myReposyPartition)
    :   aStorage(),
        pCppRepositoryPartition(&io_myReposyPartition)
{
}

DefAdmin::~DefAdmin()
{
}

Define &
DefAdmin::Store_Define( const InputContext&     i_rContext,
                        const String  &         i_sName,
                        const StringVector &    i_rDefinition )
{
    Define &
        ret = *new Define( i_sName,
                           i_rDefinition,
                           i_rContext.CurFile().LeId() );
    aStorage.Store_Define(ret);
    return ret;

}

Macro &
DefAdmin::Store_Macro(  const InputContext&     i_rContext,
                        const String  &         i_sName,
                        const StringVector &    i_rParams,
                        const StringVector &    i_rDefinition )
{
    Macro &
        ret = *new Macro( i_sName,
                          i_rParams,
                          i_rDefinition,
                          i_rContext.CurFile().LeId() );
    aStorage.Store_Macro(ret);
    return ret;
}

const DefineEntity &
DefAdmin::Find_Def(De_id i_id) const
{
    return aStorage[i_id];
}

DefsResultList
DefAdmin::AllDefines() const
{
    return csv::make_range( aStorage.DefineIndex().Begin(),
                            aStorage.DefineIndex().End() );
}

DefsResultList
DefAdmin::AllMacros() const
{
    return csv::make_range( aStorage.MacroIndex().Begin(),
                            aStorage.MacroIndex().End() );
}





}   // namespace cpp
}   // namespace ary
