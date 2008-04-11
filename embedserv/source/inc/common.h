/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: common.h,v $
 * $Revision: 1.6 $
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

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdafx.h"
#include <initguid.h>
#include <rtl/ustring.hxx>
#include <osl/interlck.h>
#include <comphelper/classids.hxx>

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace frame {
        class XModel;
    }
    namespace beans {
        struct PropertyValue;
    }
    namespace io {
        class XInputStream;
        class XOutputStream;
    }
} } }

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
