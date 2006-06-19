/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: is_ce.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:51:46 $
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
#include "is_ce.hxx"

// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace idl
{


namespace
{
const uintt C_nReservedElements = predefined::ce_MAX;    // Skipping "0" and the GlobalNamespace
}


Ce_Storage::Ce_Storage( uintt  i_nBLOCK_SIZE_LOG_2,
                        uintt  i_nInitialNrOfBlocks )
    :   aContainer(i_nBLOCK_SIZE_LOG_2, C_nReservedElements, i_nInitialNrOfBlocks)
{
}

Ce_Storage::~Ce_Storage()
{
}

void
Ce_Storage::EraseAll()
{
    aContainer.EraseAll();
}

void
Ce_Storage::Save( PersistenceAdmin &  ) const
{
    // KORR_FUTURE
}

void
Ce_Storage::Load( PersistenceAdmin & )
{
    // KORR_FUTURE
}


}   // namespace idl
}   // namespace ary

