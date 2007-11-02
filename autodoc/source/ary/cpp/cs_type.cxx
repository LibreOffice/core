/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cs_type.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:33:20 $
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
Type_Storage::Set_BuiltInType( Rid                              i_id,
                               const char *                     i_sName,
                               ary::cpp::E_TypeSpecialisation   i_eSpecialisation )
{
    DYN BuiltInType &
        rNew = *new BuiltInType(i_sName, i_eSpecialisation);
    Set_Reserved( i_id, rNew);  // Here goes the ownership for rNew.
    aBuiltInTypes[rNew.SpecializedName()] = rNew.TypeId();
}



}   // namespace cpp
}   // namespace ary
