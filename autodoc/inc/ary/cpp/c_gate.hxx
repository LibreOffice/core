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

#ifndef ARY_CPP_C_GATE_HXX
#define ARY_CPP_C_GATE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>



namespace autodoc
{
    class Options;
}
namespace ary
{
    class Entity;

namespace cpp
{
    class CodeEntity;
    class CppEntity;
    class CePilot;
    class DefPilot;
    class TypePilot;
}
namespace loc
{
    class LocationPilot;
}
}





namespace ary
{
namespace cpp
{



/** Acess to all stored objcts in the repository, which are
    relevant to C++.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}


    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation() = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;

    // INQUIRY
    virtual const String &
                        RepositoryTitle() const = 0;
    virtual const CodeEntity *
                        Search_RelatedCe(
                            Type_id		        i_type ) const = 0;
    virtual const ::ary::cpp::CppEntity *
                        Search_Entity(
                            GlobalId            i_id ) const = 0;
    virtual uintt       Get_AlphabeticalList(
                            List_GlobalIds &    o_result,
                            const char *        i_begin,
                            const char *        i_end ) const = 0;
    virtual const CePilot &
                        Ces() const = 0;
    virtual const DefPilot &
                        Defs() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    virtual const loc::LocationPilot &
                        Locations() const = 0;

    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual DefPilot &  Defs() = 0;
    virtual TypePilot & Types() = 0;
    virtual loc::LocationPilot &
                        Locations() = 0;
};



}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
