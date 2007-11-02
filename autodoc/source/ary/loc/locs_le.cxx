/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: locs_le.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:17:44 $
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
#include "locs_le.hxx"

// NOT FULLY DEFINED SERVICES


namespace
{
    const uintt
    C_nReservedElements = ary::loc::predefined::le_MAX;    // Skipping "0"
}



namespace ary
{
namespace loc
{

Le_Storage *        Le_Storage::pInstance_ = 0;




Le_Storage::Le_Storage()
    :   stg::Storage<LocationEntity>(C_nReservedElements)
{
    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Le_Storage::~Le_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}


}   // namespace loc
}   // namespace ary
