/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ca_type.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:29:50 $
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

#ifndef ARY_CPP_CA_TYPE_HXX
#define ARY_CPP_CA_TYPE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_type.hxx>
    // OTHER
#include "cs_type.hxx"



namespace ary
{
namespace cpp
{
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{



/** Administrates all C++ types as uses in user declarations
    as return-, parameter- or variable-types.
*/
class TypeAdmin : public TypePilot
{
  public:
    // LIFECYCLE
                        TypeAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
    virtual             ~TypeAdmin();

    // INQUIRY
    /// @return A list of all stored types that are not C++ or STL builtin types.
    const Type_Storage &
                        Storage() const;

    // ACCESS
    Type_Storage &      Storage();

    // INHERITED
    // Interface TypePilot:
    virtual const Type &
                        CheckIn_UsedType(
                            const InputContext &
                                                i_context,
                            DYN UsedType &      pass_type );
    virtual const Type &
                        Find_Type(
                            Type_id             i_type ) const;
    virtual bool        Get_TypeText(
                            StreamStr &         o_result,
                            Type_id             i_type ) const;
    virtual bool        Get_TypeText(
                            StreamStr &         o_preName,              ///  ::ary::cpp::
                            StreamStr &         o_name,                 ///  MyClass
                            StreamStr &         o_postName,             ///  <TplArgument> * const &
                            Type_id             i_type ) const;
    virtual Type_id     Tid_Ellipse() const;

  private:
    // DATA
    Type_Storage        aStorage;
    RepositoryPartition *
                        pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Type_Storage &
TypeAdmin::Storage() const
{
    return aStorage;
}

inline Type_Storage &
TypeAdmin::Storage()
{
    return aStorage;
}





}   // namespace cpp
}   // namespace ary
#endif
