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
        // wenn dieser Fkt.Typ gesetzt handelt es sich um EXISTS oder UNIQUE, der FieldName enthält das gesamte Statement
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

    //==================================================================
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
#endif // DBAUI_ENUMTYPES_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
