/*************************************************************************
 *
 *  $RCSfile: i_attribute.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:13:51 $
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
#include <ary/idl/i_attribute.hxx>
#include <ary/idl/ik_attribute.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
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
Attribute::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Attribute(*this);
}

RCid
Attribute::inq_ClassId() const
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
    csv_assert( i_ce.ClassId() == Attribute::class_id );
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
