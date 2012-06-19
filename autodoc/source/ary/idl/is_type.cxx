/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
