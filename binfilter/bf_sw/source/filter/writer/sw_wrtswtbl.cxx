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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

namespace binfilter {



//-----------------------------------------------------------------------


//-----------------------------------------------------------------------




//-----------------------------------------------------------------------

/*N*/ SwWriteTableCol::SwWriteTableCol( USHORT nPosition )
/*N*/ 	: nPos(nPosition),
/*N*/ 	bLeftBorder(TRUE), bRightBorder(TRUE),
/*N*/ 	nWidthOpt( 0 ), bRelWidthOpt( FALSE ),
/*N*/ 	bOutWidth( TRUE )
/*N*/ {
/*N*/ }

//-----------------------------------------------------------------------

/*N*/ long SwWriteTable::GetBoxWidth( const SwTableBox *pBox )
/*N*/ {
/*N*/ 	const SwFrmFmt *pFmt = pBox->GetFrmFmt();
/*N*/ 	const SwFmtFrmSize& aFrmSize=
/*N*/ 		(const SwFmtFrmSize&)pFmt->GetAttr( RES_FRM_SIZE );
/*N*/ 
/*N*/ 	return aFrmSize.GetSize().Width();
/*N*/ }


























}
