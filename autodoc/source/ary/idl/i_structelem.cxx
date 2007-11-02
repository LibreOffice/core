/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_structelem.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:48:43 $
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
#include <ary/idl/i_structelem.hxx>
#include <ary/idl/ik_structelem.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


StructElement::StructElement( const String &      i_sName,
                              Ce_id               i_nOwner,
                              Ce_id               i_nNameRoom,
                              Type_id             i_nType )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        nType(i_nType)
{
}

StructElement::~StructElement()
{
}

void
StructElement::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
StructElement::get_AryClass() const
{
    return class_id;
}

const String &
StructElement::inq_LocalName() const
{
    return sName;
}

Ce_id
StructElement::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
StructElement::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
StructElement::inq_SightLevel() const
{
    return sl_Member;
}


namespace ifc_structelement
{

inline const StructElement &
selem_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == StructElement::class_id );
    return static_cast< const StructElement& >(i_ce);
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return selem_cast(i_ce).nType;
}

} // namespace ifc_structelement



}   //  namespace   idl
}   //  namespace   ary
