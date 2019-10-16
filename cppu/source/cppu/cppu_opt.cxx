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

#include <sal/config.h>

#include <com/sun/star/uno/Any.hxx>
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <rtl/ustring.hxx>


using namespace ::rtl;


extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iquery_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUString ret = "unsatisfied query for interface of type "
        + OUString::unacquired( &pType->pTypeName ) + "!";
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}


extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iset_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUString ret = "invalid attempt to assign an empty interface of type "
        + OUString::unacquired( &pType->pTypeName ) + "!";
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}


extern "C" rtl_uString * SAL_CALL cppu_Any_extraction_failure_msg(
    uno_Any const * pAny, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    OUString ret = "Cannot extract an Any(" +
        OUString::unacquired(&pAny->pType->pTypeName) +
        ") to " +
        OUString::unacquired(&pType->pTypeName) +
        "!";
    rtl_uString_acquire( ret.pData );
    return ret.pData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
