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
#ifndef _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
#define _CONNECTIVITY_MAB_NS_INCLUDE_HXX_ 1

//
// Only include Mozilla include files once and using this file...
//

// moved this here and in first position due to "long l;" unxsoli4 warning
#include <rtl/ustrbuf.hxx>

#include "pre_include_mozilla.h"
#if defined __GNUC__
    #pragma GCC system_header
#endif

#include <nsDebug.h>

#include <nsCOMPtr.h>
#include <nsISupportsArray.h>
#include <nsString.h>

#if defined __SUNPRO_CC
#pragma disable_warn
    // somewhere in the files included directly or indirectly in nsString.h, warnings are enabled, again
#endif
#include <nsMemory.h>
#include <prtypes.h>
#include <nsRDFCID.h>
#include <nsXPIDLString.h>
#include <nsIRDFService.h>
#include <nsIRDFResource.h>
#include <nsReadableUtils.h>
#include <msgCore.h>
#include <nsIServiceManager.h>
#include <nsIAbCard.h>
#include <nsAbBaseCID.h>
#include <nsAbAddressCollecter.h>
#include <nsIPref.h>
#include <nsIAddrBookSession.h>
#include <nsIMsgHeaderParser.h>
#include <nsIAddrBookSession.h>
#include <nsIAbDirectory.h>
#include <nsAbDirectoryQuery.h>
#include <nsIAbDirectoryQuery.h>
#include <nsIAbDirectoryQueryProxy.h>
#include <nsIAbDirFactory.h>
#include <nsIRunnable.h>
#include <nsIAbLDAPAttributeMap.h>
#include <nsILDAPMessage.h>

#include <prerr.h>

#include "post_include_mozilla.h"

#endif // _CONNECTIVITY_MAB_NS_INCLUDE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
