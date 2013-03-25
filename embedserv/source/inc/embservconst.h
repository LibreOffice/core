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

#ifndef _EMBSERVCONST_H_
#define _EMBSERVCONST_H_

#include <guiddef.h>

#include <comphelper/classids.hxx>

#if defined __MINGW32__
#pragma GCC diagnostic warning "-Wmissing-braces"
#endif

#define SUPPORTED_FACTORIES_NUM 10

#if defined(__MINGW32__) && !defined(INITGUID)
EXTERN_C const GUID DECLSPEC_SELECTANY OID_WriterTextServer;
EXTERN_C const GUID DECLSPEC_SELECTANY OID_WriterOASISTextServer;

EXTERN_C const GUID DECLSPEC_SELECTANY OID_CalcServer;
EXTERN_C const GUID DECLSPEC_SELECTANY OID_CalcOASISServer;

EXTERN_C const GUID DECLSPEC_SELECTANY OID_DrawingServer;
EXTERN_C const GUID DECLSPEC_SELECTANY OID_DrawingOASISServer;

EXTERN_C const GUID DECLSPEC_SELECTANY OID_PresentationServer;
EXTERN_C const GUID DECLSPEC_SELECTANY OID_PresentationOASISServer;

EXTERN_C const GUID DECLSPEC_SELECTANY OID_MathServer;
EXTERN_C const GUID DECLSPEC_SELECTANY OID_MathOASISServer;
#else
EXTERN_C const GUID DECLSPEC_SELECTANY OID_WriterTextServer = { SO3_SW_OLE_EMBED_CLASSID_60 };
EXTERN_C const GUID DECLSPEC_SELECTANY OID_WriterOASISTextServer = { SO3_SW_OLE_EMBED_CLASSID_8 };

EXTERN_C const GUID DECLSPEC_SELECTANY OID_CalcServer = { SO3_SC_OLE_EMBED_CLASSID_60 };
EXTERN_C const GUID DECLSPEC_SELECTANY OID_CalcOASISServer = { SO3_SC_OLE_EMBED_CLASSID_8 };

EXTERN_C const GUID DECLSPEC_SELECTANY OID_DrawingServer = { SO3_SDRAW_OLE_EMBED_CLASSID_60 };
EXTERN_C const GUID DECLSPEC_SELECTANY OID_DrawingOASISServer = { SO3_SDRAW_OLE_EMBED_CLASSID_8 };

EXTERN_C const GUID DECLSPEC_SELECTANY OID_PresentationServer = { SO3_SIMPRESS_OLE_EMBED_CLASSID_60 };
EXTERN_C const GUID DECLSPEC_SELECTANY OID_PresentationOASISServer = { SO3_SIMPRESS_OLE_EMBED_CLASSID_8 };

EXTERN_C const GUID DECLSPEC_SELECTANY OID_MathServer = { SO3_SM_OLE_EMBED_CLASSID_60 };
EXTERN_C const GUID DECLSPEC_SELECTANY OID_MathOASISServer = { SO3_SM_OLE_EMBED_CLASSID_8 };
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
