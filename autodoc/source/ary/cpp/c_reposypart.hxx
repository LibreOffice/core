/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_reposypart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:26:56 $
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
                            Type_id             i_type ) const;
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
