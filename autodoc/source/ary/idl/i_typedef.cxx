/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_typedef.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:49:13 $
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
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


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
Typedef::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Typedef::get_AryClass() const
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
    csv_assert( i_ce.AryClass() == Typedef::class_id );
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
