/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_le.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:17:59 $
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

#ifndef ARY_LOC_LE_HXX
#define ARY_LOC_LE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/loc/loc_types4loc.hxx>
#include <ary/loc/loc_traits.hxx>



namespace ary
{
namespace loc
{


/** Base class for all file locations in the Autodoc repository.
*/
class LocationEntity : public ::ary::Entity
{
  public:
    typedef Le_Traits       traits_t;

    virtual             ~LocationEntity() {}

    Le_id               LeId() const;
    const String &      LocalName() const;
    Le_id               ParentDirectory() const;

  private:
    virtual const String &
                        inq_LocalName() const = 0;
    virtual Le_id       inq_ParentDirectory() const = 0;
};




// IMPLEMENTATION
inline Le_id
LocationEntity::LeId() const
{
    return TypedId<LocationEntity>(Id());
}

inline const String &
LocationEntity::LocalName() const
{
    return inq_LocalName();
}

inline Le_id
LocationEntity::ParentDirectory() const
{
    return inq_ParentDirectory();
}





} // namespace loc
} // namespace ary
#endif
