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
#ifndef _SVX_OPTGRID_HXX
#define _SVX_OPTGRID_HXX

// include ---------------------------------------------------------------

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
namespace binfilter {

class SvxGridTabPage;

// class SvxOptionsGrid --------------------------------------------------

class SvxOptionsGrid
{
protected:
    UINT32	nFldDrawX;
    UINT32 	nFldDivisionX;
    UINT32	nFldDrawY;
    UINT32 	nFldDivisionY;
    UINT32	nFldSnapX;
    UINT32	nFldSnapY;
    BOOL	bUseGridsnap:1;
    BOOL	bSynchronize:1;
    BOOL	bGridVisible:1;
    BOOL	bEqualGrid:	 1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void 	SetFldDrawX(	UINT32 nSet){nFldDrawX 		= nSet;}
    void 	SetFldDivisionX(UINT32 nSet){nFldDivisionX  = nSet;}
    void 	SetFldDrawY   (	UINT32 nSet){nFldDrawY      = nSet;}
    void 	SetFldDivisionY(UINT32 nSet){nFldDivisionY  = nSet;}
    void 	SetFldSnapX(	UINT32 nSet){nFldSnapX 		= nSet;}
    void 	SetFldSnapY   (	UINT32 nSet){nFldSnapY      = nSet;}
    void 	SetUseGridSnap( BOOL bSet ) {bUseGridsnap	= bSet;}
    void 	SetSynchronize( BOOL bSet ) {bSynchronize	= bSet;}
    void 	SetGridVisible( BOOL bSet ) {bGridVisible	= bSet;}
    void 	SetEqualGrid( BOOL bSet )	{bEqualGrid		= bSet;}

    UINT32	GetFldDrawX(	) const {  return nFldDrawX;    }
    UINT32 	GetFldDivisionX() const {  return nFldDivisionX;}
    UINT32	GetFldDrawY   (	) const {  return nFldDrawY;    }
    UINT32 	GetFldDivisionY() const {  return nFldDivisionY;}
    UINT32	GetFldSnapX(	) const {  return nFldSnapX;    }
    UINT32	GetFldSnapY   (	) const {  return nFldSnapY;    }
    BOOL	GetUseGridSnap( ) const {  return bUseGridsnap; }
    BOOL	GetSynchronize( ) const {  return bSynchronize; }
    BOOL	GetGridVisible( ) const {  return bGridVisible; }
    BOOL	GetEqualGrid()	  const {  return bEqualGrid;   }
};

// class SvxGridItem -----------------------------------------------------


// class SvxGridTabPage --------------------------------------------------


}//end of namespace binfilter
#endif

