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
#ifndef _SOERR_HXX
#define _SOERR_HXX

#include <tools/errcode.hxx>

// Fehler Codes
#define RID_SO_ERROR_HANDLER    32000

#define SO_ERR() (ERRCODE_AREA_SO | ERRCODE_CLASS_SO)
#define SO_WRN() (ERRCODE_AREA_SO | ERRCODE_CLASS_SO | ERRCODE_WARNING_MASK)

#define ERRCODE_SO_GENERALERROR             (SO_ERR() | 1)
#define ERRCODE_SO_CANT_BINDTOSOURCE        (SO_ERR() | 2)
#define ERRCODE_SO_NOCACHE_UPDATED          (SO_ERR() | 3)
#define ERRCODE_SO_SOMECACHES_NOTUPDATED    (SO_WRN() | 4)
#define ERRCODE_SO_MK_UNAVAILABLE           (SO_ERR() | 5)
#define ERRCODE_SO_E_CLASSDIFF              (SO_ERR() | 6)
#define ERRCODE_SO_MK_NO_OBJECT             (SO_ERR() | 7)
#define ERRCODE_SO_MK_EXCEEDED_DEADLINE     (SO_ERR() | 8)
#define ERRCODE_SO_MK_CONNECT_MANUALLY      (SO_ERR() | 9)
#define ERRCODE_SO_MK_INTERMEDIATE_INTERFACE_NOT_SUPPORTED  (SO_ERR() | 10)
#define ERRCODE_SO_NO_INTERFACE             (SO_ERR() | 11)
#define ERRCODE_SO_OUT_OF_MEMORY            (SO_ERR() | 12)
#define ERRCODE_SO_MK_SYNTAX                (SO_ERR() | 13)
#define ERRCODE_SO_MK_REDUCED_TO_SELF       (SO_WRN() | 14)
#define ERRCODE_SO_MK_NO_INVERSE            (SO_ERR() | 15)
#define ERRCODE_SO_MK_NO_PREFIX             (SO_ERR() | 16)
#define ERRCODE_SO_MK_HIM                   (SO_WRN() | 17)
#define ERRCODE_SO_MK_US                    (SO_WRN() | 18)
#define ERRCODE_SO_MK_ME                    (SO_WRN() | 19)
#define ERRCODE_SO_MK_NOT_BINDABLE          (SO_ERR() | 20)
#define ERRCODE_SO_NOT_IMPLEMENTED          (SO_ERR() | 21)
#define ERRCODE_SO_MK_NO_STORAGE            (SO_ERR() | 22)
#define ERRCODE_SO_FALSE                    (SO_WRN() | 23)
#define ERRCODE_SO_MK_NEED_GENERIC          (SO_ERR() | 24)
#define ERRCODE_SO_PENDING                  (SO_ERR() | 25)
#define ERRCODE_SO_NOT_INPLACEACTIVE        (SO_ERR() | 26)
#define ERRCODE_SO_LINDEX                   (SO_ERR() | 27)
#define ERRCODE_SO_CANNOT_DOVERB_NOW        (SO_WRN() | 28)
#define ERRCODE_SO_OLEOBJ_INVALIDHWND       (SO_WRN() | 29)
#define ERRCODE_SO_NOVERBS                  (SO_ERR() | 30)
#define ERRCODE_SO_INVALIDVERB              (SO_WRN() | 31)
#define ERRCODE_SO_MK_CONNECT               (SO_ERR() | 32)
#define ERRCODE_SO_NOTIMPL                  (SO_ERR() | 33)
#define ERRCODE_SO_MK_CANTOPENFILE          (SO_ERR() | 34)

// Fehler Contexte
#define RID_SO_ERRCTX           32001

#define ERRCTX_SO_DOVERB        1



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
