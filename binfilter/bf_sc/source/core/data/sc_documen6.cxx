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

#ifdef PCH
#include "core_pch.hxx"
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "scitems.hxx"
#include <bf_svx/scripttypeitem.hxx>

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "document.hxx"
#include "cell.hxx"
#include "cellform.hxx"
#include "patattr.hxx"
#include "scrdata.hxx"
#include "poolhelp.hxx"
namespace binfilter {

using namespace ::com::sun::star;

#define SC_BREAKITER_SERVICE	"com.sun.star.i18n.BreakIterator"

//
//	this file is compiled with exceptions enabled
//	put functions here that need exceptions!
//

// -----------------------------------------------------------------------

/*N*/ const uno::Reference< i18n::XBreakIterator >& ScDocument::GetBreakIterator()
/*N*/ {
/*N*/ 	if ( !pScriptTypeData )
/*N*/ 		pScriptTypeData = new ScScriptTypeData;
/*N*/     if ( !pScriptTypeData->xBreakIter.is() )
/*N*/     {
/*N*/         uno::Reference< uno::XInterface > xInterface = xServiceManager->createInstance(
/*N*/                             ::rtl::OUString::createFromAscii( SC_BREAKITER_SERVICE ) );
/*N*/         pScriptTypeData->xBreakIter = uno::Reference< i18n::XBreakIterator >( xInterface, uno::UNO_QUERY );
/*N*/ 		DBG_ASSERT( pScriptTypeData->xBreakIter.is(), "can't get BreakIterator" );
/*N*/ 	}
/*N*/     return pScriptTypeData->xBreakIter;
/*N*/ }


/*N*/ BYTE ScDocument::GetStringScriptType( const String& rString )
/*N*/ {
/*N*/ 
/*N*/ 	BYTE nRet = 0;
/*N*/ 	if (rString.Len())
/*N*/ 	{
/*N*/         uno::Reference<i18n::XBreakIterator> xBreakIter = GetBreakIterator();
/*N*/ 		if ( xBreakIter.is() )
/*N*/ 		{
/*N*/ 			::rtl::OUString aText = rString;
/*N*/ 			sal_Int32 nLen = aText.getLength();
/*N*/ 
/*N*/ 			sal_Int32 nPos = 0;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				sal_Int16 nType = xBreakIter->getScriptType( aText, nPos );
/*N*/ 				switch ( nType )
/*N*/ 				{
/*N*/ 					case i18n::ScriptType::LATIN:
/*N*/ 						nRet |= SCRIPTTYPE_LATIN;
/*N*/ 						break;
/*N*/ 					case i18n::ScriptType::ASIAN:
/*N*/ 						nRet |= SCRIPTTYPE_ASIAN;
/*N*/ 						break;
/*N*/ 					case i18n::ScriptType::COMPLEX:
/*N*/ 						nRet |= SCRIPTTYPE_COMPLEX;
/*N*/ 						break;
/*N*/ 					// WEAK is ignored
/*N*/ 				}
/*N*/ 				nPos = xBreakIter->endOfScript( aText, nPos, nType );
/*N*/ 			}
/*N*/ 			while ( nPos >= 0 && nPos < nLen );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ BYTE ScDocument::GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat )
/*N*/ {
/*N*/ 	if ( !pCell )
/*N*/ 		return 0;		// empty
/*N*/ 
/*N*/ 	BYTE nStored = pCell->GetScriptType();
/*N*/ 	if ( nStored != SC_SCRIPTTYPE_UNKNOWN )			// stored value valid?
/*N*/ 		return nStored;								// use stored value
/*N*/ 
/*N*/ 	String aStr;
/*N*/ 	Color* pColor;
/*N*/ 	ScCellFormat::GetString( pCell, nNumberFormat, aStr, &pColor, *xPoolHelper->GetFormTable() );
/*N*/ 
/*N*/ 	BYTE nRet = GetStringScriptType( aStr );
/*N*/ 
/*N*/ 	pCell->SetScriptType( nRet );		// store for later calls
/*N*/ 
/*N*/ 	return nRet;
/*N*/ }

/*N*/ BYTE ScDocument::GetScriptType( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell )
/*N*/ {
/*N*/ 	// if cell is not passed, take from document
/*N*/ 
/*N*/ 	if (!pCell)
/*N*/ 	{
/*N*/ 		pCell = GetCell( ScAddress( nCol, nRow, nTab ) );
/*N*/ 		if ( !pCell )
/*N*/ 			return 0;		// empty
/*N*/ 	}
/*N*/ 
/*N*/ 	// if script type is set, don't have to get number formats
/*N*/ 
/*N*/ 	BYTE nStored = pCell->GetScriptType();
/*N*/ 	if ( nStored != SC_SCRIPTTYPE_UNKNOWN )			// stored value valid?
/*N*/ 		return nStored;								// use stored value
/*N*/ 
/*N*/ 	// include number formats from conditional formatting
/*N*/ 
/*N*/ 	const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
/*N*/ 	if (!pPattern) return 0;
/*N*/ 	const SfxItemSet* pCondSet = NULL;
/*N*/ 	if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
/*N*/ 		pCondSet = GetCondResult( nCol, nRow, nTab );
/*N*/ 
/*N*/ 	ULONG nFormat = pPattern->GetNumberFormat( xPoolHelper->GetFormTable(), pCondSet );
/*N*/ 	return GetCellScriptType( pCell, nFormat );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
