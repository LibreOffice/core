/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_ce.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:45:55 $
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
    virtual             ~CodeEntity() {}

    // INQUIRY
    Ce_id               CeId() const            { return Ce_id(Id()); }
    const String  &     LocalName() const;
    Cid                 Owner() const;
    Lid                 Location() const;
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
    virtual Lid         inq_Location() const = 0;

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
