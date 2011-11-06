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
