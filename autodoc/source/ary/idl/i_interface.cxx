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
