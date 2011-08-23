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

#include <paralist.hxx>
#include <outliner.hxx>		// nur wegen Paragraph, muss geaendert werden!
#include <numdef.hxx>
namespace binfilter {

/*N*/ DBG_NAME(Paragraph)

/*N*/ Paragraph::Paragraph( USHORT nDDepth )
/*N*/ 	: aBulSize( -1, -1)
/*N*/ {
/*N*/ 	DBG_CTOR( Paragraph, 0 );
/*N*/ 
/*N*/     DBG_ASSERT( ( nDDepth < SVX_MAX_NUM ) || ( nDDepth == 0xFFFF ), "Paragraph-CTOR: nDepth invalid!" );
/*N*/ 
/*N*/ 	nDepth = nDDepth;
/*N*/ 	nFlags = 0;
/*N*/ 	bVisible = TRUE;
/*N*/ }


/*N*/ Paragraph::~Paragraph()
/*N*/ {
/*N*/ 	DBG_DTOR( Paragraph, 0 );
/*N*/ }

/*N*/ void ParagraphList::Clear( BOOL bDestroyParagraphs )
/*N*/ {
/*N*/ 	if ( bDestroyParagraphs )
/*N*/ 	{
/*N*/ 		for ( ULONG n = GetParagraphCount(); n; )
/*N*/ 		{
/*N*/ 			Paragraph* pPara = GetParagraph( --n );
/*N*/ 			delete pPara;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	List::Clear();
/*N*/ }









/*NBFF*/ Paragraph* ParagraphList::GetParent( Paragraph* pParagraph, USHORT& rRelPos ) const
/*NBFF*/ {
/*NBFF*/ 	rRelPos = 0;
/*NBFF*/ 	ULONG n = GetAbsPos( pParagraph );
/*NBFF*/ 	Paragraph* pPrev = GetParagraph( --n );
/*NBFF*/ 	while ( pPrev && ( pPrev->GetDepth() >= pParagraph->GetDepth() ) )
/*NBFF*/ 	{
/*NBFF*/ 		if ( pPrev->GetDepth() == pParagraph->GetDepth() )
/*NBFF*/ 			rRelPos++;
/*NBFF*/ 		pPrev = GetParagraph( --n );
/*NBFF*/ 	}
/*NBFF*/ 
/*NBFF*/ 	return pPrev;
/*NBFF*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
