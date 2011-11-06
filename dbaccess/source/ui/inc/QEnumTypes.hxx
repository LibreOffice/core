/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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


