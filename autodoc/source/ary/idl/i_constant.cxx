/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_constant.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:21:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"

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
        nNameRoom(i_nNameRoom),
        nOwner(i_nOwner),
        nType(i_nType),
        sInitValue(i_sInitValue)
{
}

Constant::~Constant()
{
}

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
