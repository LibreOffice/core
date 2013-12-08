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

#include "pre_include_mozilla.h"

#include "nscore.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"

#include "pratom.h"
#include "prmem.h"
#include "plstr.h"
#include "prenv.h"

#include "nsIFactory.h"
#include "nsIComponentManager.h"
#include "nsIEnumerator.h"
#include "nsXPIDLString.h"
#include "nsEscape.h"
#include "nsIURL.h"

#include "prprf.h"

#include "nsIProfile.h"
#include "nsIProfileInternal.h"
#include "nsIProfileStartupListener.h"
#include "nsIProfileChangeStatus.h"
#include "nsCOMPtr.h"
#include "nsISupports.h"
#include "nsIRegistry.h"
#include "nsString.h"
#if defined __SUNPRO_CC
#pragma disable_warn
    // somewhere in the files included directly or indirectly in nsString.h, warnings are enabled, again
#endif
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIDirectoryService.h"

#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsFileStream.h"
#include "nsIStreamListener.h"
#include "nsIServiceManager.h"
#include "nsIModule.h"
#include "nsIGenericFactory.h"
#include "nsICategoryManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsDirectoryServiceDefs.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsIChromeRegistry.h"
#include "nsIStringBundle.h"
#include "nsIObserverService.h"
#include "nsHashtable.h"
#include "nsIAtom.h"
#include "nsCRT.h"

#include "post_include_mozilla.h"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
