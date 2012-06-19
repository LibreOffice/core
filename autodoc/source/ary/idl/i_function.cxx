/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
