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

#ifndef ARY_CPP_C_DE_HXX
#define ARY_CPP_C_DE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_cppentity.hxx>
    // OTHER
#include <ary/loc/loc_types4loc.hxx>
#include <ary/cpp/c_traits.hxx>




namespace ary
{
namespace cpp
{


/** Describes a C/C++ #define statement. May be a define or a macro, for which
    two cases the two different constructors are to be used.

    This class is used by cpp::PreProcessor.
*/
class DefineEntity : public ary::cpp::CppEntity
{
  public:
    typedef Def_Traits       traits_t;

    virtual             ~DefineEntity() {}

    // INQUIRY
    De_id               DefId() const           { return De_id(Id()); }
    const String  &     LocalName() const;
    loc::Le_id          Location() const;
    const StringVector &
                        DefinitionText() const;
    // ACCESS
  protected:
                        DefineEntity(
                            const String  &     i_name,
                            loc::Le_id          i_declaringFile );
  private:
    // Locals
    virtual const StringVector &
                        inq_DefinitionText() const = 0;

    // DATA
    String              sName;
    loc::Le_id          nLocation;
};




// IMPLEMENTATION
inline const String  &
DefineEntity::LocalName() const
    { return sName; }

inline loc::Le_id
DefineEntity::Location() const
    { return nLocation; }

inline const StringVector &
DefineEntity::DefinitionText() const
    { return inq_DefinitionText(); }





}   // end namespace cpp
}   // end namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
