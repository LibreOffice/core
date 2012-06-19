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
#include <ary/idl/i_interface.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <sci_impl.hxx>
#include "i2s_calculator.hxx"


namespace ary
{
namespace idl
{


class Interface_2s
{
};


Interface::Interface( const String &      i_sName,
                      Ce_id               i_nOwner )
    :   sName(i_sName),
        nOwner(i_nOwner),
        aBases(),
        aFunctions(),
        aAttributes(),
        p2s()
{
}

Interface::~Interface()
{
    for ( RelationList::iterator it = aBases.begin();
          it != aBases.end();
          ++it )
    {
        delete (*it).Info();
    }
}

void
Interface::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Interface::get_AryClass() const
{
    return class_id;
}

const String &
Interface::inq_LocalName() const
{
    return sName;
}

Ce_id
Interface::inq_NameRoom() const
{
    return nOwner;
}

Ce_id
Interface::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Interface::inq_SightLevel() const
{
    return sl_File;
}


namespace ifc_interface
{

inline const Interface &
interface_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Interface::class_id );
    return static_cast< const Interface& >(i_ce);
}

void
attr::Get_Bases( Dyn_StdConstIterator<CommentedRelation> &     o_result,
                 const CodeEntity &                             i_ce )
{
    o_result = new SCI_Vector<CommentedRelation>(interface_cast(i_ce).aBases);
}

void
attr::Get_Functions( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(interface_cast(i_ce).aFunctions);
}

void
attr::Get_Attributes( Dyn_CeIterator &    o_result,
                      const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(interface_cast(i_ce).aAttributes);
}

void
xref::Get_Derivations( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_Derivations));
}

void
xref::Get_SynonymTypedefs( Dyn_CeIterator &    o_result,
                           const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_SynonymTypedefs));
}

void
xref::Get_ExportingServices( Dyn_CeIterator &    o_result,
                             const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_ExportingServices));
}

void
xref::Get_ExportingSingletons( Dyn_CeIterator &    o_result,
                               const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_ExportingSingletons));
}

void
xref::Get_AsReturns( Dyn_CeIterator &    o_result,
                     const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsReturns));
}

void
xref::Get_AsParameters( Dyn_CeIterator &    o_result,
                        const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsParameters));
}

void
xref::Get_AsDataTypes( Dyn_CeIterator &    o_result,
                       const CodeEntity &  i_ce )
{
    o_result = new SCI_Vector<Ce_id>(i_ce.Secondaries().List(interface_2s_AsDataTypes));
}




}   // namespace ifc_interface


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
