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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
