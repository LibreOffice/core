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


#include "olinefun.hxx"

#include "docsh.hxx"
#include "globstr.hrc"
#include "bf_sc.hrc"
namespace binfilter {


//========================================================================

/*N*/ BOOL ScOutlineDocFunc::MakeOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 BOOL bSuccess = FALSE;
/*N*/ }

/*N*/ BOOL ScOutlineDocFunc::RemoveOutline( const ScRange& rRange, BOOL bColumns, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 BOOL bDone = FALSE;
/*N*/ }

/*N*/ BOOL ScOutlineDocFunc::RemoveAllOutlines( USHORT nTab, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 BOOL bSuccess = FALSE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScOutlineDocFunc::AutoOutline( const ScRange& rRange, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 USHORT nStartCol = rRange.aStart.Col();
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScOutlineDocFunc::SelectLevel( USHORT nTab, BOOL bColumns, USHORT nLevel,
/*N*/ 									BOOL bRecord, BOOL bPaint, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ScDocument* pDoc = rDocShell.GetDocument();
/*N*/ 	return TRUE;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ BOOL ScOutlineDocFunc::ShowMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 BOOL bDone = FALSE;
/*N*/ }

/*N*/ BOOL ScOutlineDocFunc::HideMarkedOutlines( const ScRange& rRange, BOOL bRecord, BOOL bApi )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 BOOL bDone = FALSE;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
