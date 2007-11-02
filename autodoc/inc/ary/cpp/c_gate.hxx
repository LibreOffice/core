/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_gate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:49:00 $
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
                            Type_id             i_type ) const = 0;
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
