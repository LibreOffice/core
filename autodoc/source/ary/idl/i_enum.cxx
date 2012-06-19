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
#include <ary/idl/i_enum.hxx>
#include <ary/idl/ik_enum.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{

Enum::Enum( const String &      i_sName,
            Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        aValues()
{
}

Enum::~Enum()
{
}

void
Enum::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Enum::get_AryClass() const
{
    return class_id;
}

const String &
Enum::inq_LocalName() const
{
    return sName;
}

Ce_id
Enum::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Enum::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Enum::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_enum
{

inline const Enum &
enum_cast( const CodeEntity & i_ce )
{
    csv_assert( i_ce.AryClass() == Enum::class_id );
    return static_cast< const Enum& >(i_ce);
}

void
attr::Get_Values( Dyn_CeIterator &    o_result,
                  const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(enum_cast(i_ce).aValues);
}


void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(enum_2s_SynonymTypedefs));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(enum_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(enum_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(enum_2s_AsDataTypes));
}

} // namespace ifc_enum


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
