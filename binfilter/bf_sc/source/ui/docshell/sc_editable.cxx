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

#include "editable.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "markdata.hxx"

namespace binfilter {

//------------------------------------------------------------------

/*N*/ ScEditableTester::ScEditableTester() :
/*N*/ 	bIsEditable( TRUE ),
/*N*/ 	bOnlyMatrix( TRUE )
/*N*/ {
/*N*/ }


/*N*/ ScEditableTester::ScEditableTester( ScDocument* pDoc, USHORT nTab,
/*N*/ 						USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow ) :
/*N*/ 	bIsEditable( TRUE ),
/*N*/ 	bOnlyMatrix( TRUE )
/*N*/ {
/*N*/ 	TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
/*N*/ }

/*N*/ ScEditableTester::ScEditableTester( ScDocument* pDoc, 
/*N*/ 						USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 						const ScMarkData& rMark ) :
/*N*/ 	bIsEditable( TRUE ),
/*N*/ 	bOnlyMatrix( TRUE )
/*N*/ {
/*N*/ 	TestSelectedBlock( pDoc, nStartCol, nStartRow, nEndCol, nEndRow, rMark );
/*N*/ }

/*N*/ ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScRange& rRange ) :
/*N*/ 	bIsEditable( TRUE ),
/*N*/ 	bOnlyMatrix( TRUE )
/*N*/ {
/*N*/ }

/*N*/ ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScMarkData& rMark ) :
/*N*/ 	bIsEditable( TRUE ),
/*N*/ 	bOnlyMatrix( TRUE )
/*N*/ {
/*N*/ }


//------------------------------------------------------------------

/*N*/ void ScEditableTester::TestBlock( ScDocument* pDoc, USHORT nTab,
/*N*/ 						USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
/*N*/ {
/*N*/ 	if ( bIsEditable || bOnlyMatrix )
/*N*/ 	{
/*N*/ 		BOOL bThisMatrix;
/*N*/ 		if ( !pDoc->IsBlockEditable( nTab, nStartCol, nStartRow, nEndCol, nEndRow, &bThisMatrix ) )
/*N*/ 		{
/*N*/ 			bIsEditable = FALSE;
/*N*/ 			if ( !bThisMatrix )
/*N*/ 				bOnlyMatrix = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void ScEditableTester::TestSelectedBlock( ScDocument* pDoc, 
/*N*/ 						USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
/*N*/ 						const ScMarkData& rMark )
/*N*/ {
/*N*/ 	USHORT nTabCount = pDoc->GetTableCount();
/*N*/ 	for (USHORT nTab=0; nTab<nTabCount; nTab++)
/*N*/ 		if (rMark.GetTableSelect(nTab))
/*N*/ 			TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
/*N*/ }




//------------------------------------------------------------------

/*N*/ USHORT ScEditableTester::GetMessageId() const
/*N*/ {
/*N*/ 	if (bIsEditable)
/*N*/ 		return 0;
/*N*/ 	else if (bOnlyMatrix)
/*N*/ 		return STR_MATRIXFRAGMENTERR;
/*N*/ 	else
/*N*/ 		return STR_PROTECTIONERR;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
