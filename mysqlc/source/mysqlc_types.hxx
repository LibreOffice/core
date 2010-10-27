/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: mysqlc_types.hxx,v $
*
* $Revision: 1.1.2.2 $
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
************************************************************************/

#ifndef MYSQLC_TYPES_HXX
#define MYSQLC_TYPES_HXX

struct TypeInfoDef {
    const char *typeName;
    sal_Int32 dataType;
    sal_Int32 precision;
    const char *literalPrefix;
    const char *literalSuffix;
    const char *createParams;
    sal_Int16 nullable;
    sal_Bool caseSensitive;
    sal_Int16 searchable;
    sal_Bool isUnsigned;
    sal_Bool fixedPrecScale;
    sal_Bool autoIncrement;
    const char *localTypeName;
    sal_Int32 minScale;
    sal_Int32 maxScale;
    sal_Int32 sqlDataType;
    sal_Int32 sqlDateTimeSub;
    sal_Int32 numPrecRadix;
};

extern TypeInfoDef mysqlc_types[];

#endif /* MYSQLC_TYPES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
