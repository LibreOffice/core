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

#ifndef _RTL_STRING_UTILS_CONST_H_
#define _RTL_STRING_UTILS_CONST_H_

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

static const sal_Int32 kErrCompareAStringToUString     = -2;
static const sal_Int32 kErrCompareNAStringToUString    = -3;
static const sal_Int32 kErrCompareAStringToRTLUString  = -4;
static const sal_Int32 kErrCompareNAStringToRTLUString = -5;
static const sal_Int32 kErrAStringToByteStringCompare  = -6;
static const sal_Int32 kErrAStringToByteStringNCompare = -7;
static const sal_Int32 kErrCompareAStringToString      = -8;
static const sal_Int32 kErrCompareNAStringToString     = -9;

#ifdef __cplusplus
}
#endif

#endif /* _RTL_STRING_UTILS_CONST_H_ */



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
