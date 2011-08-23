/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
                      const CodeEntity &						  i_ce )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
