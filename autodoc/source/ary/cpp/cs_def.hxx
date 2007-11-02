/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cs_def.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:33:02 $
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
