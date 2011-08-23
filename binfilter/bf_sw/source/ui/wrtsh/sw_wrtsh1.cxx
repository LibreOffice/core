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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#include <bf_svx/svdview.hxx>
#include <bf_so3/ipenv.hxx>

#include <wdocsh.hxx>

#include <horiornt.hxx>

#include <wrtsh.hxx>
#include <cmdid.h>
#include <pagedesc.hxx>
#include <viscrs.hxx>
#include <swdtflvr.hxx>
#include <swerror.h>
#include <wrtsh.hrc>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
