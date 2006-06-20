/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mozilla_nsprofile.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:49:30 $
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

#include "pre_include_mozilla.h"
#if defined __GNUC__
    #pragma GCC system_header
#endif

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
#include "nsIFactory.h"

#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsFileStream.h"
#include "nsIStreamListener.h"
#include "nsIServiceManager.h"
#include "nsCOMPtr.h"
#include "nsIModule.h"
#include "nsIGenericFactory.h"
#include "nsICategoryManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsIDirectoryService.h"
#include "nsDirectoryServiceDefs.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsIChromeRegistry.h" // chromeReg
#include "nsIStringBundle.h"
#include "nsIObserverService.h"
#include "nsHashtable.h"
#include "nsIAtom.h"
#include "nsCRT.h"

#include "post_include_mozilla.h"
