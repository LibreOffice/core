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
#include <ary/cpp/c_tydef.hxx>


// NOT FULLY DECLARED SERVICES
#include <slots.hxx>
#include "c_slots.hxx"




namespace ary
{
namespace cpp
{

Typedef::Typedef( const String  &     i_sLocalName,
                  Cid                 i_nOwner,
                  E_Protection        i_eProtection,
                  Lid                 i_nFile,
                  Tid                 i_nDescribingType )
    :   aEssentials( i_sLocalName,
                     i_nOwner,
                     i_nFile ),
        nDescribingType(i_nDescribingType),
        eProtection(i_eProtection)
{
}

Typedef::~Typedef()
{

}

const String  &
Typedef::inq_LocalName() const
{
    return aEssentials.LocalName();
}

Cid
Typedef::inq_Owner() const
{
    return aEssentials.Owner();
}

Lid
Typedef::inq_Location() const
{
    return aEssentials.Location();
}

void
Typedef::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Typedef::get_AryClass() const
{
    return class_id;
}


}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
