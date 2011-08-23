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
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TABCOL_HXX
#include <tabcol.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
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
