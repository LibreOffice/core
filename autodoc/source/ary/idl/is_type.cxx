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
#include "is_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>

namespace
{

const uintt
    C_nReservedElements = ary::idl::predefined::type_MAX;    // Skipping "0" and the built in types.
}


namespace ary
{
namespace idl
{

Type_Storage *          Type_Storage::pInstance_ = 0;



Type_Storage::Type_Storage()
    :   stg::Storage<Type>(C_nReservedElements),
        aSequenceIndex()
{
    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Type_Storage::~Type_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

void
Type_Storage::Add_Sequence( Type_id             i_nRelatedType,
                            Type_id             i_nSequence )
{
    aSequenceIndex[i_nRelatedType] = i_nSequence;
}

Type_id
Type_Storage::Search_SequenceOf( Type_id i_nRelatedType )
{
    return csv::value_from_map(aSequenceIndex, i_nRelatedType);
}




}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
