/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_type.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:54:46 $
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

#ifndef ARY_IDL_IS_TYPE_HXX
#define ARY_IDL_IS_TYPE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <ary/idl/i_type.hxx>




namespace ary
{
namespace idl
{


/** The data base for all ->ary::idl::CodeEntity objects.
*/
class Type_Storage : public ::ary::stg::Storage< ::ary::idl::Type >
{
  public:
                        Type_Storage();
                        ~Type_Storage();


    void                Add_Sequence(
                            Type_id             i_nRelatedType,
                            Type_id             i_nSequence );

    Type_id             Search_SequenceOf(
                            Type_id             i_nRelatedType );

    static Type_Storage &
                        Instance_();
  private:
    /**  value_type.first   := id of the base type
         value_type.second  := id of the sequence<base type>
    */
    typedef std::map<Type_id,Type_id>           Map_Sequences;

    // DATA
    Map_Sequences       aSequenceIndex;

    static Type_Storage *
                        pInstance_;
};



namespace predefined
{

enum E_Type
{
    type_Root_ofXNameRooms = 1,
    type_GlobalXNameRoom,
    type_any,
    type_boolean,
    type_byte,
    type_char,
    type_double,
    type_float,
    type_hyper,
    type_long,
    type_short,
    type_string,
    type_type,
    type_void,
    type_u_hyper,
    type_u_long,
    type_u_short,
    type_ellipse,    // ...
    type_MAX
};

}   // namespace predefined




// IMPLEMENTATION
inline Type_Storage &
Type_Storage::Instance_()
{
    csv_assert(pInstance_ != 0);
    return *pInstance_;
}




}   // namespace idl
}   // namespace ary
#endif
