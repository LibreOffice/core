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
#ifndef _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
#define _CONNECTIVITY_MAB_NS_INCLUDE_HXX_


// Only include Mozilla include files once and using this file...


// moved this here and in first position due to "long l;" unxsoli4 warning
#include <rtl/ustrbuf.hxx>

#include "pre_include_mozilla.h"

#include <nsDebug.h>

#include <nsCOMPtr.h>
#include <nsISupportsArray.h>
#include <nsString.h>

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
