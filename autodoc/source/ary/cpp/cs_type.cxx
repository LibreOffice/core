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
#include "cs_type.hxx"

// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_builtintype.hxx>


namespace
{
    const uintt
        C_nReservedElements = ary::cpp::predefined::t_MAX;    // Skipping "0" and the builtin types
}


namespace ary
{
namespace cpp
{



Type_Storage *    Type_Storage::pInstance_ = 0;




Type_Storage::Type_Storage()
    :   stg::Storage<Type>(C_nReservedElements),
        aBuiltInTypes()
{
    Setup_BuiltInTypes();

    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Type_Storage::~Type_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

Type_id
Type_Storage::Search_BuiltInType( const String & i_specializedName ) const
{
    return csv::value_from_map(aBuiltInTypes, i_specializedName, Tid(0));
}

void
Type_Storage::Setup_BuiltInTypes()
{
    Set_BuiltInType( predefined::t_void, "void" );
    Set_BuiltInType( predefined::t_bool, "bool" );
    Set_BuiltInType( predefined::t_char, "char" );
    Set_BuiltInType( predefined::t_signed_char, "char", TYSP_signed );
    Set_BuiltInType( predefined::t_unsigned_char, "char", TYSP_unsigned );
    Set_BuiltInType( predefined::t_short, "short" );
    Set_BuiltInType( predefined::t_unsigned_short, "short", TYSP_unsigned );
    Set_BuiltInType( predefined::t_int, "int" );
    Set_BuiltInType( predefined::t_unsigned_int, "int", TYSP_unsigned );
    Set_BuiltInType( predefined::t_long, "long" );
    Set_BuiltInType( predefined::t_unsigned_long, "long", TYSP_unsigned );
    Set_BuiltInType( predefined::t_float, "float" );
    Set_BuiltInType( predefined::t_double, "double" );
    Set_BuiltInType( predefined::t_size_t, "size_t" );
    Set_BuiltInType( predefined::t_wchar_t, "wchar_t" );
    Set_BuiltInType( predefined::t_ptrdiff_t, "ptrdiff_t" );
    Set_BuiltInType( predefined::t_ellipse, "..." );
}

void
Type_Storage::Set_BuiltInType( Rid							    i_id,
                               const char *		                i_sName,
                               ary::cpp::E_TypeSpecialisation   i_eSpecialisation )
{
    DYN BuiltInType &
        rNew = *new BuiltInType(i_sName, i_eSpecialisation);
    Set_Reserved( i_id, rNew);  // Here goes the ownership for rNew.
    aBuiltInTypes[rNew.SpecializedName()] = rNew.TypeId();
}



}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
