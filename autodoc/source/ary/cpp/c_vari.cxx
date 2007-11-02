/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_vari.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:28:25 $
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
#include <ary/cpp/c_vari.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{
namespace cpp
{


Variable::Variable()
    :   // aEssentials,
           nType(0),
        eProtection(PROTECT_global)
        // aFlags
{
}

Variable::Variable( const String  &     i_sLocalName,
                    Cid                 i_nOwner,
                    E_Protection        i_eProtection,
                    Lid                 i_nFile,
                    Tid                 i_nType,
                    VariableFlags       i_aFlags,
                    const String  &     i_sArraySize,
                    const String  &     i_sInitValue )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
           nType(i_nType),
        eProtection(i_eProtection),
        aFlags(i_aFlags),
        sArraySize(i_sArraySize),
        sInitialisation(i_sInitValue)
{
}

Variable::~Variable()
{
}

const String  &
Variable::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Variable::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Variable::inq_Location() const
{
    return aEssentials.Location();
}

void
Variable::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Variable::get_AryClass() const
{
    return class_id;
}


}   // namespace cpp
}   // namespace ary
