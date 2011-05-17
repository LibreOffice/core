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

#ifndef ARY_CPP_CP_TYPE_HXX
#define ARY_CPP_CP_TYPE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class InputContext;
    class Type;
    class UsedType;
}
}





namespace ary
{
namespace cpp
{




/** Acess to all found C++ types (as they are used in declarations)
    in the repository.
*/
class TypePilot
{
  public:

    // LIFECYCLE
    virtual             ~TypePilot() {}


    // OPERATIONS
    virtual const Type &
                        CheckIn_UsedType(
                            const InputContext &
                                                i_context,
                            DYN UsedType &      pass_type ) = 0;
    // INQUIRY
    virtual const Type &
                        Find_Type(
                            Type_id             i_type ) const = 0;
    virtual bool        Get_TypeText(
                            StreamStr &         o_result,
                            Type_id             i_type ) const = 0;
    virtual bool        Get_TypeText(
                            StreamStr &         o_preName,              ///  ::ary::cpp::
                            StreamStr &         o_name,                 ///  MyClass
                            StreamStr &         o_postName,             ///  <TplArgument> * const &
                            Type_id             i_type ) const = 0;
    virtual Type_id     Tid_Ellipse() const = 0;
};




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
