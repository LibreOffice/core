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
#ifndef INCLUDED_SVTOOLS_SOERR_HXX
#define INCLUDED_SVTOOLS_SOERR_HXX

#include <vcl/errcode.hxx>
#ifndef __RSC
#include <svtools/ehdl.hxx>
#include <svtools/svtdllapi.h>
#endif

// error codes
#define SO_ERR(x) ErrCode(ERRCODE_AREA_SO | ERRCODE_CLASS_SO | x)
#define SO_WRN(x) ErrCode(ERRCODE_AREA_SO | ERRCODE_CLASS_SO | ERRCODE_WARNING_MASK | x)

#define ERRCODE_SO_GENERALERROR             SO_ERR(1)
#define ERRCODE_SO_FALSE                    SO_WRN(23)
#define ERRCODE_SO_PENDING                  SO_ERR(25)
#define ERRCODE_SO_CANNOT_DOVERB_NOW        SO_WRN(28)
#define ERRCODE_SO_NOVERBS                  SO_ERR(30)
#define ERRCODE_SO_NOTIMPL                  SO_ERR(33)

// error contexts
#define ERRCTX_SO_DOVERB        1

#ifndef __RSC
SVT_DLLPUBLIC const ErrMsgCode* getRID_SO_ERROR_HANDLER();
SVT_DLLPUBLIC const ErrMsgCode* getRID_SO_ERRCTX();
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
