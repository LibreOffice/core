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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <limits.h>




#ifndef _CMDID_H
#include <cmdid.h>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
#endif
namespace binfilter {

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

using namespace ::com::sun::star::util;


static long nStartDragX = 0, nStartDragY = 0;
static BOOL  bStartDrag = FALSE;

/*------------------------------------------------------------------------
 Beschreibung:	Rahmengebundenes Macro ausfuehren
------------------------------------------------------------------------*/

/*N*/ IMPL_LINK( SwWrtShell, ExecFlyMac, void *, pFlyFmt )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	const SwFrmFmt *pFmt = pFlyFmt ? (SwFrmFmt*)pFlyFmt : GetFlyFrmFmt();
/*N*/ 	return 0;
/*N*/ }

}
