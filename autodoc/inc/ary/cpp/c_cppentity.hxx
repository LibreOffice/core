/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_cppentity.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:46:33 $
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

#ifndef ARY_CPP_C_CPPENTITY_HXX
#define ARY_CPP_C_CPPENTITY_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // OTHER
#include <ary/doc/d_docu.hxx>



namespace ary
{
namespace cpp
{


/** A C++ code entity as parsed by Autodoc.
*/
class CppEntity : public Entity
{
  public:
    // LIFECYCLE
    virtual             ~CppEntity() {}

    // OPERATIONS

    // INQUIRY
    const ary::doc::Documentation &
                        Docu() const;
    // ACCESS
    void                Set_Docu(
                            DYN ary::doc::Node &
                                                pass_docudata );
  private:
    // DATA
    ary::doc::Documentation
                        aDocu;
};




// IMPLEMENTATION
inline const doc::Documentation &
CppEntity::Docu() const
{
    return aDocu;
}

inline void
CppEntity::Set_Docu(ary::doc::Node & pass_docudata)
{
    aDocu.Set_Data(pass_docudata);
}




}   // namespace cpp
}   // namespace ary
#endif
