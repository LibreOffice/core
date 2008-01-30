/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QEnumTypes.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:47:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


