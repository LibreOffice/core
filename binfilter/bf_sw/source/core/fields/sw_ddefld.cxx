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

#ifndef _SVXLINKMGR_HXX
#include <bf_svx/linkmgr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif
namespace binfilter {

extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008

using namespace rtl;

#ifdef PM2
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_IBM_850
#else
#define DDE_TXT_ENCODING 	RTL_TEXTENCODING_MS_1252
#endif

/*--------------------------------------------------------------------
    Beschreibung: Globale Variablen
 --------------------------------------------------------------------*/

/*N*/ class SwIntrnlRefLink : public SwBaseLink
/*N*/ {
/*N*/ 	SwDDEFieldType& rFldType;
/*N*/ public:
/*N*/ 	SwIntrnlRefLink( SwDDEFieldType& rType, USHORT nUpdateType, USHORT nFmt )
/*N*/ 		: SwBaseLink( nUpdateType, nFmt ),
/*N*/ 		rFldType( rType )
/*N*/ 	{}
/*N*/ 

/*N*/ };






/*N*/ SwDDEFieldType::SwDDEFieldType(const String& rName,
/*N*/ 								const String& rCmd, USHORT nUpdateType )
/*N*/ 	: SwFieldType( RES_DDEFLD ),
/*N*/ 	aName( rName ), pDoc( 0 ), nRefCnt( 0 )
/*N*/ {
/*N*/ 	bCRLFFlag = bDeleted = FALSE;
/*N*/ 	refLink = new SwIntrnlRefLink( *this, nUpdateType, FORMAT_STRING );
/*N*/ 	SetCmd( rCmd );
/*N*/ }

/*N*/ SwDDEFieldType::~SwDDEFieldType()
/*N*/ {
/*N*/ 	if( pDoc && !pDoc->IsInDtor() )
/*?*/ 		pDoc->GetLinkManager().Remove( refLink );
/*N*/ 	refLink->Disconnect();
/*N*/ }


/*N*/ SwFieldType* SwDDEFieldType::Copy() const
/*N*/ {
/*N*/ 	SwDDEFieldType* pType = new SwDDEFieldType( aName, GetCmd(), GetType() );
/*N*/ 	pType->aExpansion = aExpansion;
/*N*/ 	pType->bCRLFFlag = bCRLFFlag;
/*N*/ 	pType->bDeleted = bDeleted;
/*N*/ 	pType->SetDoc( pDoc );
/*N*/ 	return pType;
/*N*/ }

/*N*/ const String& SwDDEFieldType::GetName() const
/*N*/ {
/*N*/ 	return aName;
/*N*/ }

/*N*/ void SwDDEFieldType::SetCmd( const String& rStr )
/*N*/ {
/*N*/ 	String sCmd( rStr );
/*N*/ 	xub_StrLen nPos;
/*N*/ 	while( STRING_NOTFOUND != (nPos = sCmd.SearchAscii( "  " )) )
/*?*/ 		sCmd.Erase( nPos, 1 );
/*N*/ 	refLink->SetLinkSourceName( sCmd );
/*N*/ }

/*N*/ String SwDDEFieldType::GetCmd() const
/*N*/ {
/*N*/ 	return refLink->GetLinkSourceName();
/*N*/ }

/*N*/ void SwDDEFieldType::SetDoc( SwDoc* pNewDoc )
/*N*/ {
/*N*/ 	if( pNewDoc == pDoc )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if( pDoc && refLink.Is() )
/*N*/ 	{
/*?*/ 		ASSERT( !nRefCnt, "wie kommen die Referenzen rueber?" );
/*?*/ 		pDoc->GetLinkManager().Remove( refLink );
/*N*/ 	}
/*N*/ 
/*N*/ 	pDoc = pNewDoc;
/*N*/ 	if( pDoc && nRefCnt )
/*N*/ 	{
/*?*/ 		refLink->SetVisible( pDoc->IsVisibleLinks() );
/*?*/ 		pDoc->GetLinkManager().InsertDDELink( refLink );
/*N*/ 	}
/*N*/ }


/*N*/ void SwDDEFieldType::_RefCntChgd()
/*N*/ {
/*N*/ 	if( nRefCnt )
/*N*/ 	{
/*N*/ 		refLink->SetVisible( pDoc->IsVisibleLinks() );
/*N*/ 		pDoc->GetLinkManager().InsertDDELink( refLink );
/*N*/ 		if( pDoc->GetRootFrm() )
/*?*/ 			UpdateNow();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		Disconnect();
/*?*/ 		pDoc->GetLinkManager().Remove( refLink );
/*N*/ 	}
/*N*/ }
/* -----------------------------28.08.00 16:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwDDEFieldType::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const
{
    BYTE nPart = 0;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR2:      nPart = 3; break;
    case FIELD_PROP_PAR4:      nPart = 2; break;
    case FIELD_PROP_SUBTYPE:   nPart = 1; break;
    case FIELD_PROP_BOOL1:
        {
            sal_Bool bSet = GetType() == ::binfilter::LINKUPDATE_ALWAYS ? TRUE : FALSE;
            rVal.setValue(&bSet, ::getBooleanCppuType());
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    if( nPart )
        rVal <<= OUString(GetCmd().GetToken(nPart-1, ::binfilter::cTokenSeperator));
    return TRUE;
}
/* -----------------------------28.08.00 16:23--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwDDEFieldType::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId )
{
    BYTE nPart = 0;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR2:      nPart = 3; break;
    case FIELD_PROP_PAR4:      nPart = 2; break;
    case FIELD_PROP_SUBTYPE:   nPart = 1; break;
    case FIELD_PROP_BOOL1:
        SetType( *(sal_Bool*)rVal.getValue() ? ::binfilter::LINKUPDATE_ALWAYS
                                             : ::binfilter::LINKUPDATE_ONCALL );
        break;
    default:
        DBG_ERROR("illegal property");
    }
    if( nPart )
    {
        String sTmp, sCmd( GetCmd() );
        while(3 > sCmd.GetTokenCount(::binfilter::cTokenSeperator))
            sCmd += ::binfilter::cTokenSeperator;
        sCmd.SetToken( nPart-1, ::binfilter::cTokenSeperator, ::binfilter::GetString( rVal, sTmp ) );
        SetCmd( sCmd );
    }
    return TRUE;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDDEField::SwDDEField( SwDDEFieldType* pType )
/*N*/ 	: SwField(pType)
/*N*/ {
/*N*/ }

/*N*/ SwDDEField::~SwDDEField()
/*N*/ {
/*N*/ 	if( GetTyp()->IsLastDepend() )  					// der Letzte mach das
/*N*/ 		((SwDDEFieldType*)GetTyp())->Disconnect();		// Licht aus
/*N*/ }

/*N*/ String SwDDEField::Expand() const
/*N*/ {
/*N*/ 	xub_StrLen nPos;
/*N*/ 	String aStr( ((SwDDEFieldType*)GetTyp())->GetExpansion() );
/*N*/ 
/*N*/ 	aStr.EraseAllChars( '\r' );
/*N*/ 	while( (nPos = aStr.Search( '\t' )) != STRING_NOTFOUND )
/*?*/ 		aStr.SetChar( nPos, ' ' );
/*N*/ 	while( (nPos = aStr.Search( '\n' )) != STRING_NOTFOUND )
/*?*/ 		aStr.SetChar( nPos, '|' );
/*N*/ 	if( aStr.Len() && ( aStr.GetChar( aStr.Len()-1 ) == '|') )
/*?*/ 		aStr.Erase( aStr.Len()-1, 1 );
/*N*/ 	return aStr;
/*N*/ }

/*N*/ SwField* SwDDEField::Copy() const
/*N*/ {
/*N*/ 	return new SwDDEField((SwDDEFieldType*)GetTyp());
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Parameter des Typen erfragen
                  Name
 --------------------------------------------------------------------*/
const String& SwDDEField::GetPar1() const
{
    return ((SwDDEFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
    Beschreibung: Parameter des Typen erfragen
                  Commando
 --------------------------------------------------------------------*/
String SwDDEField::GetPar2() const
{
    return ((SwDDEFieldType*)GetTyp())->GetCmd();
}

void SwDDEField::SetPar2(const String& rStr)
{
    ((SwDDEFieldType*)GetTyp())->SetCmd(rStr);
}

}
