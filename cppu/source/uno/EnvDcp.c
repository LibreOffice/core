/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvDcp.c,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:38:04 $
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

#include "uno/EnvDcp.h"


void uno_EnvDcp_getTypeName(rtl_uString const * pEnvDcp, rtl_uString ** ppEnvTypeName)
{
    sal_Int32 colIdx = rtl_ustr_indexOfChar_WithLength(pEnvDcp->buffer, pEnvDcp->length, ':');
    if (colIdx >= 0)
        rtl_uString_newFromStr_WithLength(ppEnvTypeName, pEnvDcp->buffer, colIdx);

    else
        rtl_uString_newFromStr(ppEnvTypeName, pEnvDcp->buffer);
}

void uno_EnvDcp_getPurpose(rtl_uString const * pEnvDcp, rtl_uString ** ppEnvPurpose)
{
    sal_Int32 colIdx = rtl_ustr_indexOfChar_WithLength(pEnvDcp->buffer, pEnvDcp->length, ':');
    if (colIdx >= 0)
        rtl_uString_newFromStr_WithLength(ppEnvPurpose, pEnvDcp->buffer + colIdx, pEnvDcp->length - colIdx);

    else
        rtl_uString_new(ppEnvPurpose);
}
