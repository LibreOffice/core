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

// include ---------------------------------------------------------------

#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <bf_svtools/useroptions.hxx>
#endif
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "adritem.hxx"
#include "svxids.hrc"
namespace binfilter {

#define INVALID_TOKEN ((USHORT)0xFFFF)

// -----------------------------------------------------------------------

/*?*/ String ConvertToStore_Impl( const String& rText )
/*?*/ {
/*?*/ 	String sRet;
/*?*/ 	USHORT i = 0;
/*?*/ 
/*?*/ 	while ( i < rText.Len() )
/*?*/ 	{
/*?*/ 		if ( rText.GetChar(i) == '\\' || rText.GetChar(i) == '#' )
/*?*/ 			sRet += '\\';
/*?*/ 		sRet += rText.GetChar(i++);
/*?*/ 	}
/*?*/ 	return sRet;
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1(SvxAddressItem, SfxStringItem);

// -----------------------------------------------------------------------

/*N*/ SvxAddressItem::SvxAddressItem( USHORT nWhich ) :
/*N*/ 
/*N*/ 	SfxStringItem( nWhich, String() )
/*N*/ 
/*N*/ {
/*N*/ 	SvtUserOptions aUserOpt;
/*N*/ 
/*N*/ 	aName = aUserOpt.GetLastName();
/*N*/ 	aFirstName = aUserOpt.GetFirstName();
/*N*/ 	aShortName = aUserOpt.GetID();
/*N*/ 
/*N*/ 	SetToken( POS_COMPANY, aUserOpt.GetCompany() );
/*N*/ 	SetToken( POS_STREET, aUserOpt.GetStreet() );
/*N*/ 	SetToken( POS_CITY, aUserOpt.GetCity() );
/*N*/ 	SetToken( POS_STATE, aUserOpt.GetState() );
/*N*/ 	SetToken( POS_PLZ, aUserOpt.GetZip() );
/*N*/ 	SetToken( POS_COUNTRY, aUserOpt.GetCountry() );
/*N*/ 	SetToken( POS_POSITION, aUserOpt.GetPosition() );
/*N*/ 	SetToken( POS_TITLE, aUserOpt.GetTitle() );
/*N*/ 	SetToken( POS_TEL_PRIVATE, aUserOpt.GetTelephoneHome() );
/*N*/ 	SetToken( POS_TEL_COMPANY, aUserOpt.GetTelephoneWork() );
/*N*/ 	SetToken( POS_FAX, aUserOpt.GetFax() );
/*N*/ 	SetToken( POS_EMAIL, aUserOpt.GetEmail() );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxAddressItem::SvxAddressItem( const SvxAddressItem& rItem ) :
/*N*/ 
/*N*/ 	SfxStringItem( rItem.Which(), rItem.GetValue() ),
/*N*/ 
/*N*/ 	aName		( rItem.aName ),
/*N*/ 	aFirstName	( rItem.aFirstName ),
/*N*/ 	aShortName	( rItem.aShortName )
/*N*/ 
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvxAddressItem::SvxAddressItem( const String &rAdress,
/*N*/ 								const String &rShortName,
/*N*/ 								const String &rFirstName,
/*N*/ 								const String &rName,
/*N*/ 								USHORT nWhich ) :
/*N*/ 
/*N*/ 	SfxStringItem( nWhich, rAdress ),
/*N*/ 
/*N*/ 	aName		( rName ),
/*N*/ 	aFirstName	( rFirstName ),
/*N*/ 	aShortName	( rShortName )
/*N*/ 
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ BOOL SvxAddressItem::SetToken( USHORT nPos, const String &rVal )
/*N*/ {
/*N*/ 	// nach dem 'nPos'-ten Token suchen
/*N*/ 	String aAdress( GetValue() );
/*N*/ 	USHORT nStart, nEnd = 0;
/*N*/ 	for ( USHORT nNo = 0; nNo <= nPos; ++nNo )
/*N*/ 	{
/*N*/ 		// End-'#' finden
/*N*/ 		for ( nStart = nEnd;
/*N*/ 			  nEnd < aAdress.Len() && aAdress.GetChar(nEnd) != '#';
/*N*/ 			  ++nEnd )
/*N*/ 			if ( aAdress.GetChar(nEnd) == '\\' )
/*N*/ 				++nEnd;
/*N*/ 
/*N*/ 		// aAdress[nEnd] == '#' oder am Ende, also eine Position weiter gehen
/*N*/ 		++nEnd;
/*N*/ 
/*N*/ 		// ggf. fehlende '#' auff"ullen
/*N*/ 		if ( nNo < nPos && nEnd >= aAdress.Len() )
/*N*/ 			aAdress += '#';
/*N*/ 	}
/*N*/ 
/*N*/ 	// gefunden
/*N*/ 	aAdress.Erase( nStart, nEnd-nStart-1 );
/*N*/ 	aAdress.Insert( ::binfilter::ConvertToStore_Impl( rVal ), nStart );
/*N*/ 	SetValue( aAdress );
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ String SvxAddressItem::GetToken( USHORT nPos ) const
/*N*/ {
/*N*/ 	String sRet, sEmpty;
/*N*/ 	const String &rAdress = GetValue();
/*N*/ 	USHORT i = 0, nTxt = 0;
/*N*/ 
/*N*/ 	while ( i < rAdress.Len() )
/*N*/ 	{
/*N*/ 		while ( i < rAdress.Len() && rAdress.GetChar(i) != '#' )
/*N*/ 		{
/*N*/ 			if ( rAdress.GetChar(i) == '\\' )
/*N*/ 				i++;
/*N*/ 			sRet += rAdress.GetChar(i++);
/*N*/ 		}
/*N*/ 
/*N*/ 		// rAdress[i] == '#' oder am Ende, also eine Position weiter gehen
/*N*/ 		i++;
/*N*/ 
/*N*/ 		if ( nPos == nTxt )
/*N*/ 			return sRet;
/*N*/ 		else if ( i >= rAdress.Len() )
/*N*/ 			return sEmpty;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			sRet.Erase();
/*N*/ 			nTxt++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sEmpty;
/*N*/ }

// -----------------------------------------------------------------------


}
