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

#ifndef ARY_IDL_IS_TYPE_HXX
#define ARY_IDL_IS_TYPE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <ary/idl/i_type.hxx>




namespace ary
{
namespace idl
{


/** The data base for all ->ary::idl::CodeEntity objects.
*/
class Type_Storage : public ::ary::stg::Storage< ::ary::idl::Type >
{
  public:
                        Type_Storage();
                        ~Type_Storage();


    void                Add_Sequence(
                            Type_id             i_nRelatedType,
                            Type_id             i_nSequence );

    Type_id             Search_SequenceOf(
                            Type_id             i_nRelatedType );

    static Type_Storage &
                        Instance_();
  private:
    /**  value_type.first   := id of the base type
         value_type.second  := id of the sequence<base type>
    */
    typedef std::map<Type_id,Type_id>           Map_Sequences;

    // DATA
    Map_Sequences       aSequenceIndex;

    static Type_Storage *
                        pInstance_;
};



namespace predefined
{

enum E_Type
{
    type_Root_ofXNameRooms = 1,
    type_GlobalXNameRoom,
    type_any,
    type_boolean,
    type_byte,
    type_char,
    type_double,
    type_float,
    type_hyper,
    type_long,
    type_short,
    type_string,
    type_type,
    type_void,
    type_u_hyper,
    type_u_long,
    type_u_short,
    type_ellipse,    // ...
    type_MAX
};

}   // namespace predefined




// IMPLEMENTATION
inline Type_Storage &
Type_Storage::Instance_()
{
    csv_assert(pInstance_ != 0);
    return *pInstance_;
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
