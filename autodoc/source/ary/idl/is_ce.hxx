/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_ce.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:51:00 $
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

#ifndef ARY_IDL_IS_CE_HXX
#define ARY_IDL_IS_CE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/storage.hxx>
    // COMPONENTS
#include <ary/idl/i_ce.hxx>
#include <store/st_root.hxx>
#include "is_ce_indices.hxx"
    // PARAMETERS



namespace ary
{
namespace idl
{

class PersistenceAdmin;


class Ce_Storage : public ::ary::store22::Storage
{
  public:
    typedef CodeEntity                                  element_base_type;
    typedef ary::store::StorageUnit<element_base_type>  unit;
    typedef ary::store::Root<unit>                      container;
    typedef TypedId<element_base_type>                  key;

    // LIFECYCLE
                        Ce_Storage(
                            uintt               i_nBLOCK_SIZE_LOG_2 = 10,
                            uintt               i_nInitialNrOfBlocks = 2 );
                        ~Ce_Storage();

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

  private:
    // DATA
    container           aContainer;
};




namespace predefined
{

enum E_CodeEntity
{
    ce_GlobalNamespace = 1,
    ce_MAX
};

}   // namespace predefined



}   // namespace idl
}   // namespace ary

#endif


