/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cs_def.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:32:50 $
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
#include "cs_def.hxx"

// NOT FULLY DEFINED SERVICES


namespace
{
const uintt
    C_nReservedElements = ::ary::cpp::predefined::de_MAX;    // Skipping "0"
}



namespace ary
{
namespace cpp
{

Def_Storage *    Def_Storage::pInstance_ = 0;




Def_Storage::Def_Storage()
    :   stg::Storage<DefineEntity>(C_nReservedElements)
{
    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Def_Storage::~Def_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

De_id
Def_Storage::Store_Define(DYN DefineEntity &  pass_de)
{
    De_id
        ret = Store_Entity(pass_de);
    aDefines.Add(ret);
    return ret;
}

De_id
Def_Storage::Store_Macro(DYN DefineEntity &  pass_de)
{
    De_id
        ret = Store_Entity(pass_de);
    aMacros.Add(ret);
    return ret;
}



}   // namespace cpp
}   // namespace ary
