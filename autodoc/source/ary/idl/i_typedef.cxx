/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_typedef.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:47:50 $
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
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ik_typedef.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/idl/ihost_ce.hxx>
#include <sci_impl.hxx>
#include "ipi_2s.hxx"


namespace ary
{
namespace idl
{


Typedef::Typedef( const String &      i_sName,
                  Ce_id               i_nOwner,
                  Type_id             i_nDefiningType )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nDefiningType(i_nDefiningType)
{
}

Typedef::~Typedef()
{
}

void
Typedef::do_Visit_CeHost( CeHost & o_rHost ) const
{
    o_rHost.Do_Typedef(*this);
}

RCid
Typedef::inq_ClassId() const
{
    return class_id;
}

const String &
Typedef::inq_LocalName() const
{
    return sName;
}

Ce_id
Typedef::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Typedef::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Typedef::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_typedef
{

inline const Typedef &
typedef_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.ClassId() == Typedef::class_id );
    return static_cast< const Typedef& >(i_ce);
}

Type_id
attr::DefiningType( const CodeEntity & i_ce )
{
    return typedef_cast(i_ce).nDefiningType;
}


void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_SynonymTypedefs));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(typedef_2s_AsDataTypes));
}

} // namespace ifc_typedef



}   //  namespace   idl
}   //  namespace   ary
