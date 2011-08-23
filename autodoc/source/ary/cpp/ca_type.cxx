/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "ca_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_builtintype.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/cpp/inpcontx.hxx>
#include <ary/cpp/usedtype.hxx>
#include <ary/getncast.hxx>
#include "c_reposypart.hxx"
#include "cs_type.hxx"




namespace ary
{
namespace cpp
{


TypeAdmin::TypeAdmin(RepositoryPartition & io_myReposyPartition)
    :   aStorage(),
        pCppRepositoryPartition(&io_myReposyPartition)
{
}

TypeAdmin::~TypeAdmin()
{
}


// KORR_FUTURE
//  Remove unused parameter.

const Type &
TypeAdmin::CheckIn_UsedType( const InputContext &   ,
                             DYN UsedType &    	    pass_type )
{
    Dyn<UsedType>
        pNewType(&pass_type);  // Ensure clean up of heap object.

    Type_id
        tid(0);
    if (pass_type.IsBuiltInType())
    {
        tid = aStorage.Search_BuiltInType(
                        BuiltInType::SpecializedName_( pass_type.LocalName().c_str(),
                                                       pass_type.TypeSpecialisation() ));
        csv_assert(tid.IsValid());
        return aStorage[tid];
    }

    tid = aStorage.UsedTypeIndex().Search(pass_type);
    if (tid.IsValid())
    {
        return aStorage[tid];
    }

    // Type does not yet exist:
        // Transfer ownership from pNewTypeand assign id:
    aStorage.Store_Entity(*pNewType.Release());

    aStorage.UsedTypeIndex().Add(pass_type.TypeId());
    return pass_type;
}

const Type &
TypeAdmin::Find_Type(Type_id i_type) const
{
    return aStorage[i_type];
}

bool
TypeAdmin::Get_TypeText( StreamStr &         o_result,
                         Type_id             i_type ) const
{
    if (NOT i_type.IsValid())
        return false;
    aStorage[i_type].Get_Text(o_result, *pCppRepositoryPartition);
    return true;
}

bool
TypeAdmin::Get_TypeText( StreamStr &         o_preName,
                         StreamStr &         o_name,
                         StreamStr &         o_postName,
                         Type_id             i_type ) const
{
    if (NOT i_type.IsValid())
        return false;
    aStorage[i_type].Get_Text(o_preName, o_name, o_postName, *pCppRepositoryPartition);
    return true;
}

Type_id
TypeAdmin::Tid_Ellipse() const
{
    return Type_id(predefined::t_ellipse);
}




}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
