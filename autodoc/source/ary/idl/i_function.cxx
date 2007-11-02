/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_function.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:44:55 $
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
#include <ary/idl/i_function.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>



namespace ary
{
namespace idl
{

Function::Function( const String &      i_sName,
                    Ce_id               i_nOwner,
                    Ce_id               i_nNameRoom,
                    Type_id             i_nReturnType,
                    bool                i_bOneWay )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        nReturnType(i_nReturnType),
        aParameters(),
        aExceptions(),
        bOneWay(i_bOneWay),
        bEllipse(false)
{
}

Function::Function( const String &      i_sName,
                    Ce_id               i_nOwner,
                    Ce_id               i_nNameRoom )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nNameRoom(i_nNameRoom),
        nReturnType(0),
        aParameters(),
        aExceptions(),
        bOneWay(false),
        bEllipse(false)
{
}

Function::~Function()
{
}

void
Function::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Function::get_AryClass() const
{
    return class_id;
}

const String &
Function::inq_LocalName() const
{
    return sName;
}

Ce_id
Function::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Function::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Function::inq_SightLevel() const
{
    return sl_Member;
}


namespace ifc_function
{

inline const Function &
function_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Function::class_id );
    return static_cast< const Function& >(i_ce);
}

Type_id
attr::ReturnType( const CodeEntity & i_ce )
{
    return function_cast(i_ce).nReturnType;
}

bool
attr::IsOneway( const CodeEntity & i_ce )
{
    return function_cast(i_ce).bOneWay;
}

bool
attr::HasEllipse( const CodeEntity & i_ce )
{
    return function_cast(i_ce).bEllipse;
}

void
attr::Get_Parameters( Dyn_StdConstIterator<ary::idl::Parameter> & o_result,
                      const CodeEntity &                          i_ce )
{
    o_result
        = new SCI_Vector<Parameter>( function_cast(i_ce).aParameters );
}

void
attr::Get_Exceptions( Dyn_TypeIterator &  o_result,
                      const CodeEntity &  i_ce )
{
    o_result
        = new SCI_Vector<Type_id>( function_cast(i_ce).aExceptions );
}





}   // namespace ifc_function

}   //  namespace   idl
}   //  namespace   ary
