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

#ifndef ARY_CPP_C_REPOSYPART_HXX
#define ARY_CPP_C_REPOSYPART_HXX



// BASE CLASSES
#include <cpp_internalgate.hxx>

namespace ary
{
namespace cpp
{
    class CeAdmin;
    class DefAdmin;
    class TypeAdmin;
}
}




namespace ary
{
namespace cpp
{



/** The C++ partition of the repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
                        RepositoryPartition(
                            RepositoryCenter &  i_reposyImpl );
    virtual             ~RepositoryPartition();

    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation();
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const String &
                        RepositoryTitle() const;
    virtual const CodeEntity *
                        Search_RelatedCe(
                            Type_id		        i_type ) const;
    virtual const ::ary::cpp::CppEntity *
                        Search_Entity(
                            GlobalId            i_id ) const;
    virtual uintt       Get_AlphabeticalList(
                            List_GlobalIds &    o_result,
                            const char *        i_begin,
                            const char *        i_end ) const;
    virtual const CePilot &
                        Ces() const;
    virtual const DefPilot &
                        Defs() const;
    virtual const TypePilot &
                        Types() const;
    virtual const loc::LocationPilot &
                        Locations() const;
    virtual CePilot &   Ces();
    virtual DefPilot &  Defs();
    virtual TypePilot & Types();
    virtual loc::LocationPilot &
                        Locations();
  private:
    // Locals
    void                Connect_AllTypes_2_TheirRelated_CodeEntites();

    // DATA
    RepositoryCenter *  pRepositoryCenter;

    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<DefAdmin>       pDefs;
    Dyn<loc::LocationPilot>
                        pLocations;
};




}   //  namespace cpp
}   //  namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
