/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
