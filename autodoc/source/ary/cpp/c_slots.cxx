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
#include <c_slots.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary_disp.hxx>
#include <ary/cpp/c_namesp.hxx>



namespace ary
{
namespace cpp
{



//***********************       Slot_SubNamespaces     ********************//


Slot_SubNamespaces::Slot_SubNamespaces( const Map_NamespacePtr & i_rData )
    :	pData( &i_rData )
{
}

Slot_SubNamespaces::~Slot_SubNamespaces()
{
}

uintt
Slot_SubNamespaces::Size() const
{
     return pData->size();
}

void
Slot_SubNamespaces::StoreEntries( ary::Display &  o_rDestination ) const
{
    for ( Map_NamespacePtr::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        (*(*it).second).Accept(o_rDestination);
    }
}


//***********************       Slot_BaseClass     ********************//

Slot_BaseClass::Slot_BaseClass(	const List_Bases & i_rData )
    :	pData( &i_rData )
{
}

Slot_BaseClass::~Slot_BaseClass()
{
}

uintt
Slot_BaseClass::Size() const
{
     return pData->size();
}

void
Slot_BaseClass::StoreEntries( ary::Display &  o_rDestination ) const
{
    for ( List_Bases::const_iterator it = pData->begin();
          it != pData->end();
          ++it )
    {
        csv::CheckedCall(o_rDestination, *it);
    }
}



}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
