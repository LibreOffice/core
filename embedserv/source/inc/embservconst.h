/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

