/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_attribute.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:42:50 $
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
#include <ary/idl/i_attribute.hxx>
#include <ary/idl/ik_attribute.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/getncast.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{


Attribute::Attribute( const String &      i_sName,
                      Ce_id               i_nService,
                      Ce_id               i_nModule,
                      Type_id             i_nType,
                      bool                i_bReadonly,
                      bool                i_bBound )
    :   sName(i_sName),
        nOwner(i_nService),
        nNameRoom(i_nModule),
        nType(i_nType),
        aGetExceptions(),
        aSetExceptions(),
        bReadonly(i_bReadonly),
        bBound(i_bBound)
{
}

Attribute::~Attribute()
{
}


void
Attribute::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Attribute::get_AryClass() const
{
    return class_id;
}

const String &
Attribute::inq_LocalName() const
{
    return sName;
}

Ce_id
Attribute::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Attribute::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Attribute::inq_SightLevel() const
{
    return sl_Member;
}

namespace ifc_attribute
{

inline const Attribute &
attribute_cast( const CodeEntity &  i_ce )
{
    csv_assert( is_type<Attribute>(i_ce) );
    return static_cast< const Attribute& >(i_ce);
}

bool
attr::HasAnyStereotype( const CodeEntity & i_ce )
{
    const Attribute & rAttr = attribute_cast(i_ce);
    return rAttr.bReadonly OR rAttr.bBound;
}

bool
attr::IsReadOnly( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).bReadonly;
}

bool
attr::IsBound( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).bBound;
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return attribute_cast(i_ce).nType;
}

void
attr::Get_GetExceptions( Dyn_TypeIterator &  o_result,
                         const CodeEntity &  i_ce )
{
    o_result
        = new SCI_Vector<Type_id>( attribute_cast(i_ce).aGetExceptions );
}

void
attr::Get_SetExceptions( Dyn_TypeIterator &  o_result,
                         const CodeEntity &  i_ce )
{
    o_result
        = new SCI_Vector<Type_id>( attribute_cast(i_ce).aSetExceptions );
}


} // namespace ifc_attribute


}   //  namespace   idl
}   //  namespace   ary
