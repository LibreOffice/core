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

#ifndef ARY_CPP_C_CE_HXX
#define ARY_CPP_C_CE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_cppentity.hxx>
    // COMPONENTS
#include <ary/doc/d_docu.hxx>
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_traits.hxx>
#include <ary/loc/loc_types4loc.hxx>


namespace ary
{
namespace cpp
{

typedef loc::Le_id  Lid;



/** Represents a C++ code entity.
*/
class CodeEntity : public ary::cpp::CppEntity
{
  public:
    typedef Ce_Traits       traits_t;

    // LIFECYCLE
    virtual			   	~CodeEntity() {}

    // INQUIRY
    Ce_id               CeId() const            { return Ce_id(Id()); }
    const String  &     LocalName() const;
    Cid                 Owner() const;
    Lid			        Location() const;
    bool                IsVisible() const;

    // ACCESS
    void                Set_InVisible()         { bIsVisible = false; }

  protected:
                        CodeEntity() : bIsVisible(true) {}

  private:
    // Locals
    virtual const String  &
                        inq_LocalName() const = 0;
    virtual Cid         inq_Owner() const = 0;
    virtual Lid			inq_Location() const = 0;

    // DATA
    mutable bool        bIsVisible;
};


// IMPLEMENTATION
inline const String  &
CodeEntity::LocalName() const
    { return inq_LocalName(); }
inline Cid
CodeEntity::Owner() const
    { return inq_Owner(); }
inline Lid
CodeEntity::Location() const
    { return inq_Location(); }




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
