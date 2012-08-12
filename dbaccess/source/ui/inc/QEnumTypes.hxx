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
#ifndef DBAUI_ENUMTYPES_HXX
#define DBAUI_ENUMTYPES_HXX

namespace dbaui
{
    enum EOrderDir
    {
        ORDER_NONE=0,
        ORDER_ASC,
        ORDER_DESC
    };

    enum EFunctionType
    {
        FKT_NONE        =0x00000000,
        FKT_OTHER       =0x00000001,
        FKT_AGGREGATE   =0x00000002,
        FKT_CONDITION   =0x00000004,
        FKT_NUMERIC     =0x00000008
        // if this function type is set, it is either EXISTS oder UNIQUE,
        // the FieldName contains the complete statement
    };

    enum EConnectionSide
    {
        JTCS_FROM=0,
        JTCS_TO
    };

    enum ETableFieldType
    {
        TAB_NORMAL_FIELD=0,
        TAB_PRIMARY_FIELD
    };

    enum EJoinType
    {
        FULL_JOIN=0,
        LEFT_JOIN,
        RIGHT_JOIN,
        UNION_JOIN,
        CROSS_JOIN,
        INNER_JOIN
    };

    enum EControlType
    {
        tpDefault = 0,
        tpRequired,
        tpTextLen,
        tpNumType,
        tpLength,
        tpScale,
        tpFormat,
        tpAutoIncrement,
        tpBoolDefault,
        tpColumnName,
        tpType,
        tpAutoIncrementValue
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
