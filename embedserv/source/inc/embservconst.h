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

#ifndef _EMBSERVCONST_H_
#define _EMBSERVCONST_H_

#include <initguid.h>

#ifndef _COMPHELPER_CLASSIDS_HXX
#include <comphelper/classids.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

const sal_Int32 nConstBufferSize = 32000;

#define SUPPORTED_FACTORIES_NUM 10

#ifdef __MINGW32__
#define DECLSPEC_SELECTANY
#endif
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
