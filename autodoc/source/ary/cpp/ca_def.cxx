/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ca_def.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:29:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
