/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSInclude.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:27:53 $
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

#ifndef BOOL
# define MOZ_BOOL

# define BOOL mozBOOL
# define Bool mozBooL
#endif

// Turn off DEBUG Assertions
#ifdef _DEBUG
    #define _DEBUG_WAS_DEFINED _DEBUG
    #undef _DEBUG
#else
    #undef _DEBUG_WAS_DEFINED
#endif

// and turn off the additional virtual methods which are part of some interfaces when compiled
// with debug
#ifdef DEBUG
    #define DEBUG_WAS_DEFINED DEBUG
    #undef DEBUG
#else
    #undef DEBUG_WAS_DEFINED
#endif

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
#include <nsIAddrBookSession.h>
#include <nsIAbDirectory.h>
#include <nsAbDirectoryQuery.h>
#include <nsIAbDirectoryQuery.h>
#include <nsIAbDirectoryQueryProxy.h>
#include <nsIAbDirFactory.h>
#include <nsIRunnable.h>

#ifdef MOZ_BOOL
# undef BOOL
# undef Bool
#endif

#ifdef DEBUG_WAS_DEFINED
    #define DEBUG DEBUG_WAS_DEFINED
#endif

#ifdef _DEBUG_WAS_DEFINED
    #define _DEBUG _DEBUG_WAS_DEFINED
#endif

#endif // _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
