/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSInclude.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:51:01 $
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
#ifndef _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
#define _CONNECTIVITY_MAB_NS_INCLUDE_HXX_ 1

//
// Only include Mozilla include files once and using this file...
//

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

#include "post_include_mozilla.h"

#endif // _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
