/*************************************************************************
 *
 *  $RCSfile: i_constant.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:12:42 $
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
#include <ary/idl/i_constant.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <ary/idl/ik_constant.hxx>


namespace ary
{
namespace idl
{

Constant::Constant( const String &      i_sName,
                    Ce_id               i_nOwner,
                    Ce_id               i_nNameRoom,
                    Type_id             i_nType,
                    const String &      i_sInitValue )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        nType(i_nType),
        sInitValue(i_sInitValue)
{
}

Constant::~Constant()
{
}

#if ENABLE_UDM

namespace
{
enum E_Data_Constant
{
    mid_Base = 0,
    mid_Name,
    mid_Owner,
    mid_NameRoom,
    mid_Type,
    mid_InitValue,
    mid_MAX
};
}

void
Constant::SetupUdmTraits_( udm::struct_traits<Constant> & o_rTraits )
{
    o_rTraits.reserve( mid_MAX );
    udm::add_traits_base( o_rTraits, csv::Type2Type<CodeEntity>(), mid_Base );
    udm::add_traits_member( o_rTraits, &Constant::sName, mid_Name );
    udm::add_traits_member( o_rTraits, &Constant::nOwner, mid_Owner );
    udm::add_traits_member( o_rTraits, &Constant::nNameRoom, mid_NameRoom );
    udm::add_traits_member( o_rTraits, &Constant::nType, mid_Type );
    udm::add_traits_member( o_rTraits, &Constant::sInitValue, mid_InitValue );
}
#endif // ENABLE_UDM


void
Constant::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Constant(*this);
}


RCid
Constant::inq_ClassId() const
{
    return class_id;
}

const String &
Constant::inq_LocalName() const
{
    return sName;
}

Ce_id
Constant::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Constant::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Constant::inq_SightLevel() const
{
    return sl_Member;
}


namespace ifc_constant
{

inline const Constant &
constant_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.ClassId() == Constant::class_id );
    return static_cast< const Constant& >(i_ce);
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return constant_cast(i_ce).nType;
}

const String &
attr::Value( const CodeEntity & i_ce )
{
    return constant_cast(i_ce).sInitValue;
}

} // namespace ifc_constant


}   //  namespace   idl
}   //  namespace   ary
