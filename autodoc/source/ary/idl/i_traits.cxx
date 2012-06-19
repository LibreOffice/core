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
#include <ary/idl/i_traits.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/namesort.hxx>
#include "is_ce.hxx"



namespace ary
{
namespace idl
{



//********************      Ce_Traits      ********************//
Ce_Traits::entity_base_type &
Ce_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Ce_Storage::Instance_()[i_id];
}

//********************      Ce_Compare      ********************//
const Ce_Compare::key_type &
Ce_Compare::KeyOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
}

bool
Ce_Compare::Lesser_( const key_type &    i_1,
                     const key_type &    i_2 )
{
    static ::ary::LesserName    less_;
    return less_(i_1,i_2);
}




}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
