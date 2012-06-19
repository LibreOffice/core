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
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/ik_siservice.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>


namespace ary
{
namespace idl
{

SglIfcService::SglIfcService( const String &      i_sName,
                              Ce_id               i_nOwner,
                              Type_id             i_nBaseInterface )
    :   sName(i_sName),
        nOwner(i_nOwner),
        nBaseInterface(i_nBaseInterface),
        aConstructors()
{
}

SglIfcService::~SglIfcService()
{
}

void
SglIfcService::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
SglIfcService::get_AryClass() const
{
    return class_id;
}

const String &
SglIfcService::inq_LocalName() const
{
    return sName;
}

Ce_id
SglIfcService::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
SglIfcService::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
SglIfcService::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_sglifcservice
{

inline const SglIfcService &
sglifcservice_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == SglIfcService::class_id );
    return static_cast< const SglIfcService& >(i_ce);
}

Type_id
attr::BaseInterface( const CodeEntity & i_ce )
{
    return sglifcservice_cast(i_ce).nBaseInterface;
}

void
attr::Get_Constructors( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>( sglifcservice_cast(i_ce).aConstructors );
}

} // namespace ifc_sglifcservice


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
