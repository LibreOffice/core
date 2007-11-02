/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_ce.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:53:16 $
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

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{


/** The data base for all ->ary::idl::CodeEntity objects.
*/
class Ce_Storage : public ::ary::stg::Storage< ::ary::idl::CodeEntity >
{
  public:
                        Ce_Storage();
    virtual             ~Ce_Storage();

    static Ce_Storage & Instance_()               { csv_assert(pInstance_ != 0);
                                                    return *pInstance_; }
  private:
    // DATA
    static Ce_Storage * pInstance_;
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
