/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_ce.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:43:07 $
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
#include <ary/idl/i_ce.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_oldidldocu.hxx>
#include <ary/getncast.hxx>


namespace ary
{
namespace idl
{

namespace
{
    const Ce_2s aConstCe2sDummy;
}



CodeEntity::CodeEntity()
    :   aDocu(),
        p2s(0)
{
}

CodeEntity::~CodeEntity()
{
}

const Ce_2s &
CodeEntity::Secondaries() const
{
    if (p2s)
        return *p2s;
    return aConstCe2sDummy;
}

Ce_2s &
CodeEntity::Secondaries()
{
    if (p2s)
        return *p2s;
    p2s = Ce_2s::Create_(AryClass());
    return *p2s;
}




}   // namespace idl
}   // namespace ary
