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

#define ITEMID_BOXINFO SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <bf_svx/svxids.hrc>


#include <horiornt.hxx>

#include <fesh.hxx>
#include <doc.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <cellfrm.hxx>
#include <ndtxt.hxx>
#include <tabcol.hxx>
#include <viscrs.hxx>

#include <swerror.h>
namespace binfilter {

//siehe auch swtable.cxx
#define COLFUZZY 20L


/*N*/ SwTabCols *pLastCols   = 0;



/*N*/ inline const SwCursor& GetShellCursor( const SwCrsrShell& rShell )
/*N*/ {
/*N*/ 	const SwShellCrsr *pCrsr = rShell.GetTableCrsr();
/*N*/ 	if( !pCrsr )
/*N*/ 		pCrsr = (SwShellCrsr*)*rShell.GetSwCrsr( FALSE );
/*N*/ 	return *pCrsr;
/*N*/ }


/*N*/ void ClearFEShellTabCols()
/*N*/ {
/*N*/ 	DELETEZ( pLastCols );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
