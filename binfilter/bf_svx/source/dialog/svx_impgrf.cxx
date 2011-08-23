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

// include ------------------------------------------------------------------

#include <limits.h>				// USHRT_MAX

#ifndef _SFXDOCFILE_HXX
#include <bf_sfx2/docfile.hxx>
#endif

using namespace ::ucbhelper;
using namespace ::com::sun::star::uno;

#include "dialogs.hrc"
#include "impgrf.hrc"

#define _SVX_IMPGRF_CXX
#include "impgrf.hxx"

#include "dialmgr.hxx"
#include "helpid.hrc"
namespace binfilter {

// defines ---------------------------------------------------------------

#define IMPGRF_INIKEY_ASLINK		"ImportGraphicAsLink"
#define IMPGRF_INIKEY_PREVIEW		"ImportGraphicPreview"
#define IMPGRF_CONFIGNAME			String(DEFINE_CONST_UNICODE("ImportGraphicDialog"))

// -----------------------------------------------------------------------

/*N*/ GraphicFilter* DialogsResMgr::GetGrfFilter_Impl()
/*N*/ {
/*N*/ 	if( !pGrapicFilter )
/*N*/ 	{
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		pGrapicFilter = new GraphicFilter;
/*N*/ #else
/*N*/ 		pGrapicFilter = new GraphicFilter(sal_False);
/*N*/ #endif
/*N*/ 		::binfilter::FillFilter( *pGrapicFilter );
/*N*/ 	}
/*N*/ 	return pGrapicFilter;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ GraphicFilter* GetGrfFilter()
/*N*/ {
/*N*/ 	return (*(DialogsResMgr**)GetAppData(BF_SHL_SVX))->GetGrfFilter_Impl();
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT FillFilter( GraphicFilter& rFilter )
/*N*/ {
/*N*/     return rFilter.GetImportFormatCount();
/*N*/ }

// -----------------------------------------------------------------------

}
