/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_type.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:52:14 $
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



// USED SERVICES
    // BASE CLASSES
#include <store/storage.hxx>
    // COMPONENTS
#include <ary/idl/i_type.hxx>
#include <store/st_root.hxx>
#include "is_type_indices.hxx"
    // PARAMETERS


namespace ary
{
namespace idl
{


class PersistenceAdmin;


class Type_Storage : public ::ary::store22::Storage
{
  public:
    typedef Type                                        element_base_type;
    typedef ary::store::StorageUnit<element_base_type>  unit;
    typedef ary::store::Root<unit>                      container;
    typedef TypedId<element_base_type>                  key;

    // LIFECYCLE
                        Type_Storage(
                            uintt               i_nBLOCK_SIZE_LOG_2 = 10,
                            uintt               i_nInitialNrOfBlocks = 2 );
                        ~Type_Storage();

    // OPERATORS
    const unit &        operator[](
                            key                 i_nId ) const
                                                { return aContainer[i_nId]; }
    unit &              operator[](
                            key                 i_nId )
                                                { return aContainer[i_nId]; }
    // OPERATIONS
    void                EraseAll();
    void                Save(
                            PersistenceAdmin &  io_rSaver ) const;
    void                Load(
                            PersistenceAdmin &  io_rLoader );
    // INQUIRY
    const container &   Container() const       { return aContainer; }
    // ACCESS
    container &         Container()             { return aContainer; }
    Type_StorageIndices &
                        Indices()               { return aIndices; }
  private:
    // DATA
    container           aContainer;
    Type_StorageIndices aIndices;
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

}   // namespace idl
}   // namespace ary

#endif


