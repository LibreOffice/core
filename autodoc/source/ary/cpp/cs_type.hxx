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

#ifndef ARY_CPP_CS_TYPE_HXX
#define ARY_CPP_CS_TYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/s_storage.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_type.hxx>
#include <ary/cpp/c_traits.hxx>
#include <ary/cpp/usedtype.hxx>
#include <sortedids.hxx>



namespace ary
{
namespace cpp
{



/** The data base for all ->ary::cpp::Type objects.
*/
class Type_Storage : public ::ary::stg::Storage<Type>
{
  public:
    typedef SortedIds<UsedType_Compare>     UT_Index;

                        Type_Storage();
    virtual             ~Type_Storage();

    const UT_Index &    UsedTypeIndex() const;

    UT_Index &          UsedTypeIndex();
    Type_id             Search_BuiltInType(
                            const String &      i_specializedName ) const;

    static Type_Storage &
                        Instance_()             { csv_assert(pInstance_ != 0);
                                                  return *pInstance_; }
  private:
    // Locals
    void                Setup_BuiltInTypes();
    void                Set_BuiltInType(
                            Rid                 i_nId,
                            const char *        i_sName,
                            ary::cpp::E_TypeSpecialisation
                                                i_eSpecialisation = TYSP_none );
    // DATA
    UT_Index            aUsedTypes;
    std::map<String,Type_id>
                        aBuiltInTypes;


    static Type_Storage *
                        pInstance_;
};




namespace predefined
{

enum E_Type
{
    // 0 is always unused with repository storages.
    t_void = 1,
    t_bool,
    t_char,
    t_signed_char,
    t_unsigned_char,
    t_short,
    t_unsigned_short,
    t_int,
    t_unsigned_int,
    t_long,
    t_unsigned_long,
    t_float,
    t_double,
    t_size_t,
    t_wchar_t,
    t_ptrdiff_t,
    t_ellipse,
    t_MAX
};

}   // namespace predefined



// IMPLEMENTATION
inline const Type_Storage::UT_Index &
Type_Storage::UsedTypeIndex() const
{
    return aUsedTypes;
}

inline Type_Storage::UT_Index &
Type_Storage::UsedTypeIndex()
{
    return aUsedTypes;
}


}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
