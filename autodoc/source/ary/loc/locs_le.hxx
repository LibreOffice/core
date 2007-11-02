/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: locs_le.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:18:00 $
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

#ifndef ARY_LOC_LOCS_LE_HXX
#define ARY_LOC_LOCS_LE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/loc/loc_le.hxx>
#include <ary/loc/loc_root.hxx>
#include <sortedids.hxx>




namespace ary
{
namespace loc
{


/** The data base for all ->ary::cpp::CodeEntity objects.
*/
class Le_Storage : public ::ary::stg::Storage<LocationEntity>
{
  public:
    typedef SortedIds<Le_Compare>           Index;

                        Le_Storage();
    virtual             ~Le_Storage();

    const Index &       RootIndex() const         { return aRoots; }
    Index &             RootIndex()               { return aRoots; }

    static Le_Storage & Instance_()               { csv_assert(pInstance_ != 0);
                                                    return *pInstance_; }
  private:
    // DATA
    Index               aRoots;

    static Le_Storage * pInstance_;
};




namespace predefined
{

enum E_LocationEntity
{
    le_MAX = 1
};

}   // namespace predefined




}   // namespace cpp
}   // namespace ary
#endif
