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
#include <ary/cpp/c_vari.hxx>


// NOT FULLY DECLARED SERVICES



namespace ary
{
namespace cpp
{

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
