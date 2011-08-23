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

#include <com/sun/star/i18n/ScriptType.hdl>
#include <unotools/localedatawrapper.hxx>
#include <bf_svx/unolingu.hxx>
#include <bf_svx/scripttypeitem.hxx>

#include <breakit.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 

#include "swtypes.hxx"

namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;


/*N*/ SwBreakIt::SwBreakIt()
/*N*/ 	: pLocale( NULL ), pForbidden( NULL )
/*N*/ {
/*N*/ 	_GetLocale( (LanguageType)GetAppLanguage() );
/*N*/ 	Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	Reference < XInterface > xI = xMSF->createInstance(
/*N*/ 		::rtl::OUString::createFromAscii( "com.sun.star.i18n.BreakIterator" ) );
/*N*/ 	if ( xI.is() )
/*N*/ 	{
/*N*/ 		Any x = xI->queryInterface( ::getCppuType((const Reference< XBreakIterator >*)0) );
/*N*/ 		x >>= xBreak;
/*N*/ 	}
/*N*/ }

/*N*/ void SwBreakIt::_GetLocale( const LanguageType aLang )
/*N*/ {
/*N*/ 	aLast = aLang;
/*N*/ 	delete pLocale;
/*N*/ 	pLocale = new Locale( SvxCreateLocale( aLast ) );
/*N*/ }

/*N*/ void SwBreakIt::_GetForbidden( const LanguageType aLang )
/*N*/ {
/*N*/ 	aForbiddenLang = aLang;
/*N*/ 	Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	LocaleDataWrapper aWrap( xMSF, GetLocale( aLang ) );
/*N*/ 	delete pForbidden;
/*N*/ 	pForbidden = new ForbiddenCharacters( aWrap.getForbiddenCharacters() );
/*N*/ }

/*N*/ USHORT SwBreakIt::GetRealScriptOfText( const String& rTxt,
/*N*/ 										xub_StrLen nPos ) const
/*N*/ {
/*N*/ 	USHORT nScript = ScriptType::WEAK;
/*N*/ 	if( xBreak.is() && rTxt.Len() )
/*N*/ 	{
/*N*/ 		if( nPos && nPos == rTxt.Len() )
/*N*/ 			--nPos;
/*N*/ 		nScript = xBreak->getScriptType( rTxt, nPos );
/*N*/ 		sal_Int32 nChgPos;
/*N*/ 		if( ScriptType::WEAK == nScript && nPos &&
/*N*/ 			0 < (nChgPos = xBreak->beginOfScript( rTxt, nPos, nScript )) )
/*N*/ 			nScript = xBreak->getScriptType( rTxt, nChgPos-1 );
/*N*/ 
/*N*/ 		if( ScriptType::WEAK == nScript && rTxt.Len() >
/*N*/ 			( nChgPos = xBreak->endOfScript( rTxt, nPos, nScript ) ) &&
/*N*/ 			0 <= nChgPos )
/*N*/ 			nScript = xBreak->getScriptType( rTxt, nChgPos );
/*N*/ 	}
/*N*/ 	if( ScriptType::WEAK == nScript )
/*N*/ 	  nScript = GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
/*N*/ 	return nScript;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
