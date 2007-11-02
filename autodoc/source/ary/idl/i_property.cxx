/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_property.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:46:16 $
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

#include <precomp.h>
#include <ary/idl/i_property.hxx>
#include <ary/idl/ik_property.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>


namespace ary
{
namespace idl
{


Property::Property( const String &      i_sName,
                    Ce_id               i_nService,
                    Ce_id               i_nModule,
                    Type_id             i_nType,
                    Stereotypes         i_stereotypes )
    :   sName(i_sName),
        nOwner(i_nService),
        nNameRoom(i_nModule),
        nType(i_nType),
        aStereotypes(i_stereotypes)
{
}

Property::~Property()
{
}


void
Property::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Property::get_AryClass() const
{
    return class_id;
}

const String &
Property::inq_LocalName() const
{
    return sName;
}

Ce_id
Property::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Property::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Property::inq_SightLevel() const
{
    return sl_Member;
}

namespace ifc_property
{

inline const Property &
property_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Property::class_id );
    return static_cast< const Property& >(i_ce);
}

bool
attr::HasAnyStereotype( const CodeEntity &  i_ce )
{
    return property_cast(i_ce).aStereotypes.HasAny();
}

bool
attr::IsReadOnly( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsReadOnly();
}

bool
attr::IsBound( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsBound();
}

bool
attr::IsConstrained( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsConstrained();
}

bool
attr::IsMayBeAmbiguous( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeAmbiguous();
}

bool
attr::IsMayBeDefault( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeDefault();
}

bool
attr::IsMayBeVoid( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsMayBeVoid();
}

bool
attr::IsRemovable( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsRemovable();
}

bool
attr::IsTransient( const CodeEntity & i_ce )
{
    return property_cast(i_ce).aStereotypes.IsTransient();
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return property_cast(i_ce).nType;
}

} // namespace ifc_property


}   //  namespace   idl
}   //  namespace   ary
