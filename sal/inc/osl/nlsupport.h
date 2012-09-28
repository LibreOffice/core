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


#ifndef _OSL_NLSUPPORT_H_
#define _OSL_NLSUPPORT_H_

#include "sal/config.h"

#include "rtl/locale.h"
#include "rtl/textenc.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
    Determines the text encoding used by the underlying platform for the
    specified locale.

    @param pLocale
    the locale to return the text encoding for. If this parameter is NULL,
    the default locale of the current process is used.

    @returns the rtl_TextEncoding that matches the platform specific encoding
    description or RTL_TEXTENCODING_DONTKNOW if no mapping is available.
*/

SAL_DLLPUBLIC rtl_TextEncoding SAL_CALL osl_getTextEncodingFromLocale(
        rtl_Locale * pLocale );


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_NLSUPPORT_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
