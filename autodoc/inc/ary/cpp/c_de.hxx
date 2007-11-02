/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_de.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:46:55 $
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
