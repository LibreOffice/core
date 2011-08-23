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


//#if STLPORT_VERSION>=321
//#include <math.h>	// prevent conflict between exception and std::exception
//#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <bf_svx/svdview.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <bf_so3/ipenv.hxx>
#endif

#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _VISCRS_HXX //autogen wg. SwSelPaintRects
#include <viscrs.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif
namespace binfilter {

/*N*/ void SwWrtShell::CalcAndSetScale( SvEmbeddedObjectRef xObj,
/*N*/ 							 const SwRect *pFlyPrtRect,
/*N*/ 							 const SwRect *pFlyFrmRect )
/*N*/ {
            DBG_ERROR("Split!");
/*N*/ }

    void SwWrtShell::ConnectObj( SvInPlaceObjectRef, const SwRect &, const SwRect & )
    {
    }

    SwWrtShell::~SwWrtShell()
    {
        SET_CURR_SHELL( this );
        while(IsModePushed())
        {
            DBG_BF_ASSERT(0, "STRIP");
        };
        while(PopCrsr(FALSE));
    }
}
