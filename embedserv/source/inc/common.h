/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: common.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-03-26 14:49:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdafx.h"
#include <initguid.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _COMPHELPER_CLASSIDS_HXX
#include <comphelper/classids.hxx>
#endif

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
