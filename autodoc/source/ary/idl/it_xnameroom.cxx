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
#include "it_xnameroom.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/processor.hxx>
#include <cosv/tpl/tpltools.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_type.hxx>



namespace ary
{
namespace idl
{


ExplicitNameRoom::ExplicitNameRoom()
    :   aImpl()
{
}

ExplicitNameRoom::ExplicitNameRoom( const String &           i_sName,
                                    const ExplicitNameRoom & i_rParent )
    :   aImpl( i_sName, i_rParent.aImpl, i_rParent.TypeId() )
{
}

ExplicitNameRoom::~ExplicitNameRoom()
{
}

ClassId
ExplicitNameRoom::get_AryClass() const
{
    return class_id;
}

void
ExplicitNameRoom::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

void
ExplicitNameRoom::inq_Get_Text( StringVector &      o_module,
                                String &            ,           // o_name
                                Ce_id &             ,           // o_nRelatedCe
                                int &               ,           // o_nSequemceCount
                                const Gate &        ) const     // i_rGate
{
    StringVector::const_iterator it = NameChain_Begin();
    if ( it != NameChain_End()
            ? (*it).empty()
            : false )
    {	// Don't put out the root global namespace
        ++it;
    }

    for ( ;
          it != NameChain_End();
          ++it )
    {
        o_module.push_back(*it);
    }
}




}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
