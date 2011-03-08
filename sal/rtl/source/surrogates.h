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

#ifndef INCLUDED_SAL_RTL_SOURCE_SURROGATES_H
#define INCLUDED_SAL_RTL_SOURCE_SURROGATES_H

#include "sal/config.h"

#define SAL_RTL_FIRST_HIGH_SURROGATE 0xD800
#define SAL_RTL_LAST_HIGH_SURROGATE 0xDBFF
#define SAL_RTL_FIRST_LOW_SURROGATE 0xDC00
#define SAL_RTL_LAST_LOW_SURROGATE 0xDFFF

#define SAL_RTL_IS_HIGH_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_HIGH_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_HIGH_SURROGATE)

#define SAL_RTL_IS_LOW_SURROGATE(utf16) \
    ((utf16) >= SAL_RTL_FIRST_LOW_SURROGATE && \
     (utf16) <= SAL_RTL_LAST_LOW_SURROGATE)

#define SAL_RTL_COMBINE_SURROGATES(high, low) \
    ((((high) - SAL_RTL_FIRST_HIGH_SURROGATE) << 10) + \
     ((low) - SAL_RTL_FIRST_LOW_SURROGATE) + 0x10000)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
