/*************************************************************************
 *
 *  $RCSfile: QEnumTypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:39:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        FKT_AGGREGATE=0,
        FKT_OTHER    ,
        FKT_NONE     ,
        FKT_CONDITION    // wenn dieser Fkt.Typ gesetzt handelt es sich um EXISTS oder UNIQUE, der FieldName enthält das gesamte Statement
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
        NATURAL_JOIN,
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
        tpType
    };

    enum EScrollDir
    {
        scrollUp,
        scrollDown,
        scrollHome,
        scrollEnd
    };

    enum EEditMode
    {
        ADD,
        DROP,
        KEY,
        ADD_DROP
    };


}
#endif // DBAUI_ENUMTYPES_HXX


