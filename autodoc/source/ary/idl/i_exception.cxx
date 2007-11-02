/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_exception.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:44:41 $
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
#include <ary/idl/i_exception.hxx>
#include <ary/idl/ik_exception.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"



namespace ary
{
namespace idl
{

Exception::Exception( const String &      i_sName,
                      Ce_id               i_nOwner,
                      Type_id             i_nBase )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nBase(i_nBase),
        aElements()
{
}

Exception::~Exception()
{
}

void
Exception::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Exception::get_AryClass() const
{
    return class_id;
}

const String &
Exception::inq_LocalName() const
{
    return sName;
}

Ce_id
Exception::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Exception::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Exception::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_exception
{

inline const Exception &
exception_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Exception::class_id );
    return static_cast< const Exception& >(i_ce);
}

Type_id
attr::Base( const CodeEntity & i_ce )
{
    return exception_cast(i_ce).nBase;
}

void
attr::Get_Elements( Dyn_CeIterator &    o_result,
                    const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>( exception_cast(i_ce).aElements );
}


void
xref::Get_Derivations( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(exception_2s_Derivations));
}

void
xref::Get_RaisingFunctions( Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(exception_2s_RaisingFunctions));
}


} // namespace ifc_exception


}   //  namespace   idl
}   //  namespace   ary
