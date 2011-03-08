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

#ifndef ARY_CPP_CS_DE_HXX
#define ARY_CPP_CS_DE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/s_storage.hxx>
    // OTHER
#include <ary/cpp/c_de.hxx>
#include <ary/cpp/c_traits.hxx>
#include <sortedids.hxx>



namespace ary
{
namespace cpp
{




/** The data base for all ->ary::cpp::Type objects.
*/
class Def_Storage : public ::ary::stg::Storage<DefineEntity>
{
  public:
    typedef SortedIds<Def_Compare>      Index;

    // LIFECYCLE
                        Def_Storage();
    virtual             ~Def_Storage();

    De_id               Store_Define(
                            DYN DefineEntity &  pass_de );
    De_id               Store_Macro(
                            DYN DefineEntity &  pass_de );

    const Index &       DefineIndex() const     { return aDefines; }
    const Index &       MacroIndex() const      { return aMacros; }

    Index &             DefineIndex()           { return aDefines; }
    Index &             MacroIndex()            { return aMacros; }

    static Def_Storage &
                        Instance_()             { csv_assert(pInstance_ != 0);
                                                  return *pInstance_; }
  private:
    // DATA
    Index               aDefines;
    Index               aMacros;


    static Def_Storage *
                        pInstance_;
};




namespace predefined
{

enum E_DefineEntity
{
    // 0 is always unused with repository storages.
    de_MAX = 1
};

}   // namespace predefined






}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
