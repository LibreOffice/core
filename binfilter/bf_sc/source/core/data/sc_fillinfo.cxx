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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <bf_svx/boxitem.hxx>
#include <bf_svx/shaditem.hxx>

#include "document.hxx"
#include "cell.hxx"
#include "attarray.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
namespace binfilter {


// -----------------------------------------------------------------------

enum FillInfoLinePos
    {
        FILP_TOP,
        FILP_BOTTOM,
        FILP_LEFT,
        FILP_RIGHT
    };


/*N*/ inline const SvxBorderLine* GetNullOrLine( const SvxBoxItem* pBox, FillInfoLinePos eWhich )
/*N*/ {
/*N*/  if (pBox)
/*N*/ 	{
/*N*/ 		if (eWhich==FILP_TOP)
/*N*/ 			return pBox->GetTop();
/*N*/ 		else if (eWhich==FILP_BOTTOM)
/*N*/ 			return pBox->GetBottom();
/*N*/ 		else if (eWhich==FILP_LEFT)
/*N*/ 			return pBox->GetLeft();
/*N*/ 		else
/*N*/ 			return pBox->GetRight();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return NULL;
/*N*/ }

/*N*/ inline BOOL ScDocument::RowHidden( USHORT nRow, USHORT nTab )
/*N*/ {
/*N*/ 	return ( pTab[nTab]->pRowFlags[nRow] & CR_HIDDEN ) != 0;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
