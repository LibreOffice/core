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
#include "cs_ce.hxx"

// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_namesp.hxx>



namespace
{
const uintt
    C_nReservedElements = ::ary::cpp::predefined::ce_MAX;    // Skipping "0" and the GlobalNamespace
}




namespace ary
{
namespace cpp
{

Ce_Storage *    Ce_Storage::pInstance_ = 0;



Ce_Storage::Ce_Storage()
    :   stg::Storage<CodeEntity>(C_nReservedElements),
        aTypes(),
        aOperations(),
        aData()

{
    Set_Reserved( predefined::ce_GlobalNamespace,
                  *new Namespace );

    csv_assert(pInstance_ == 0);
    pInstance_ = this;
}

Ce_Storage::~Ce_Storage()
{
    csv_assert(pInstance_ != 0);
    pInstance_ = 0;
}

Ce_id
Ce_Storage::Store_Type(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aTypes.Add(ret);
    return ret;
}

Ce_id
Ce_Storage::Store_Operation(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aOperations.Add(ret);
    return ret;
}

Ce_id
Ce_Storage::Store_Datum(DYN CodeEntity & pass_ce)
{
    Ce_id
        ret = Store_Entity(pass_ce);
    aData.Add(ret);
    return ret;
}



}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
