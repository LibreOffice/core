/*************************************************************************
 *
 *  $RCSfile: i_property.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:15:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <ary/idl/i_property.hxx>
#include <ary/idl/ik_property.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>


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
Property::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Property(*this);
}

RCid
Property::inq_ClassId() const
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
    csv_assert( i_ce.ClassId() == Property::class_id );
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




#if ENABLE_UDM
IMPL_UDM_GET_TRAITS( ary::idl::Property );
#endif // ENABLE_UDM
