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



#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <unotools/localedatawrapper.hxx>
#include <bf_svx/unolingu.hxx>

#include <bf_svtools/urihelper.hxx>
#include <bf_svtools/useroptions.hxx>

#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/doctempl.hxx>
#include <bf_svx/adritem.hxx>


#include <charfmt.hxx>
#include <docstat.hxx>
#include <pagedesc.hxx>
#include <fmtpdsc.hxx>

#include <horiornt.hxx>

#include <rootfrm.hxx>		// AuthorField
#include <pagefrm.hxx>		//
#include <dbmgr.hxx>
#include <docufld.hxx>
#include <flddat.hxx>
#include <docfld.hxx>
#include <pam.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <hints.hxx>
#include <unofldmid.h>
#include <swunohelper.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#define URL_DECODE 	INetURLObject::DECODE_UNAMBIGUOUS
namespace binfilter {
extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
/*--------------------------------------------------------------------
    Beschreibung: SwPageNumberFieldType
 --------------------------------------------------------------------*/

/*N*/ SwPageNumberFieldType::SwPageNumberFieldType()
/*N*/ 	: SwFieldType( RES_PAGENUMBERFLD ),
/*N*/ 	nNumberingType( SVX_NUM_ARABIC ),
/*N*/ 	nNum( 0 ),
/*N*/ 	nMax( USHRT_MAX ),
/*N*/ 	bVirtuell( sal_False )
/*N*/ {
/*N*/ }

/*N*/ String& SwPageNumberFieldType::Expand( sal_uInt32 nFmt, short nOff,
/*N*/ 								const String& rUserStr, String& rRet ) const
/*N*/ {
/*N*/ 	sal_uInt32 nTmpFmt = (SVX_NUM_PAGEDESC == nFmt) ? (sal_uInt32)nNumberingType : nFmt;
/*N*/ 	long nTmp = nNum + nOff;
/*N*/ 
/*N*/ 	if( 0 >= nTmp || SVX_NUM_NUMBER_NONE == nTmpFmt || (!bVirtuell && nTmp > nMax) )
/*N*/ 		rRet = aEmptyStr;
/*N*/ 	else if( SVX_NUM_CHAR_SPECIAL == nTmpFmt )
/*?*/ 		rRet = rUserStr;
/*N*/ 	else
/*N*/ 		rRet = FormatNumber( (sal_uInt16)nTmp, nTmpFmt );
/*N*/ 	return rRet;
/*N*/ }

/*N*/ SwFieldType* SwPageNumberFieldType::Copy() const
/*N*/ {
        SwPageNumberFieldType *pTmp = new SwPageNumberFieldType();
        pTmp->nNum       = nNum;
        pTmp->nMax       = nMax;
        pTmp->nNumberingType = nNumberingType;
        pTmp->bVirtuell  = bVirtuell;
 
        return pTmp;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Verschiedene Expandierung
 --------------------------------------------------------------------*/

/*N*/ void SwPageNumberFieldType::ChangeExpansion( SwDoc* pDoc, sal_uInt16 nPage,
/*N*/ 											sal_uInt16 nNumPages, sal_Bool bVirt,
/*N*/ 											const sal_Int16* pNumFmt )
/*N*/ {
/*N*/ 	nNum = nPage;
/*N*/ 	nMax = nNumPages;
/*N*/ 	if( pNumFmt )
/*N*/ 		nNumberingType = *pNumFmt;
/*N*/ 
/*N*/ 	bVirtuell = sal_False;
/*N*/ 	if( bVirt )
/*N*/ 	{
/*N*/ 		// dann muss das Flag ueberprueft werden, denn das Layout setzt
/*N*/ 		// es NIE zurueck
/*N*/ 		const SfxItemPool &rPool = pDoc->GetAttrPool();
/*N*/ 		const SwFmtPageDesc *pDesc;
/*N*/ 		sal_uInt16 nMaxItems = rPool.GetItemCount( RES_PAGEDESC );
/*N*/ 		for( sal_uInt16 n = 0; n < nMaxItems; ++n )
/*N*/ 			if( 0 != (pDesc = (SwFmtPageDesc*)rPool.GetItem( RES_PAGEDESC, n ) )
/*N*/ 				&& pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
/*N*/ 			{
/*N*/ 				if( pDesc->GetDefinedIn()->ISA( SwCntntNode ))
/*N*/ 				{
/*N*/ 					SwClientIter aIter( *(SwModify*)pDesc->GetDefinedIn() );
/*N*/ 					if( aIter.First( TYPE( SwFrm ) ) )
/*N*/ 					{
/*N*/ 						bVirtuell = sal_True;
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( pDesc->GetDefinedIn()->ISA( SwFmt ))
/*N*/ 				{
/*N*/ 					SwAutoFmtGetDocNode aGetHt( &pDoc->GetNodes() );
/*N*/ 					bVirtuell = !pDesc->GetDefinedIn()->GetInfo( aGetHt );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwPageNumberField
 --------------------------------------------------------------------*/

/*N*/ SwPageNumberField::SwPageNumberField(SwPageNumberFieldType* pTyp,
/*N*/ 									 sal_uInt16 nSub, sal_uInt32 nFmt, short nOff)
/*N*/ 	: SwField(pTyp, nFmt), nOffset(nOff), nSubType(nSub)
/*N*/ {
/*N*/ }

/*N*/ String SwPageNumberField::Expand() const
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 	SwPageNumberFieldType* pFldType = (SwPageNumberFieldType*)GetTyp();
/*N*/ 
/*N*/ 	if( PG_NEXT == nSubType && 1 != nOffset )
/*N*/ 	{
/*?*/ 		if( pFldType->Expand( GetFormat(), 1, sUserStr, sRet ).Len() )
/*?*/ 			pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
/*N*/ 	}
/*N*/ 	else if( PG_PREV == nSubType && -1 != nOffset )
/*N*/ 	{
/*?*/ 		if( pFldType->Expand( GetFormat(), -1, sUserStr, sRet ).Len() )
/*?*/ 			pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pFldType->Expand( GetFormat(), nOffset, sUserStr, sRet );
/*N*/ 	return sRet;
/*N*/ }

/*N*/ SwField* SwPageNumberField::Copy() const
/*N*/ {
/*N*/ 	SwPageNumberField *pTmp =
/*N*/ 		new SwPageNumberField((SwPageNumberFieldType*)GetTyp(), nSubType, GetFormat(), nOffset);
/*N*/ 	pTmp->SetLanguage( GetLanguage() );
/*N*/ 	pTmp->SetUserString( sUserStr );
/*N*/ 	return pTmp;
/*N*/ }

/*N*/ String SwPageNumberField::GetPar2() const
/*N*/ {
/*N*/ 	return String::CreateFromInt32(nOffset);
/*N*/ }

void SwPageNumberField::SetPar2(const String& rStr)
{
    nOffset = (short)rStr.ToInt32();
}

/*N*/ sal_uInt16 SwPageNumberField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

/*-----------------05.03.98 10:25-------------------

--------------------------------------------------*/
/*N*/ BOOL SwPageNumberField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= (sal_Int16)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		rAny <<= nOffset;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		{
/*N*/ 	 		text::PageNumberType eType;
/*N*/ 			eType = text::PageNumberType_CURRENT;
/*N*/ 			if(nSubType == PG_PREV)
/*?*/ 				eType = text::PageNumberType_PREV;
/*N*/ 			else if(nSubType == PG_NEXT)
/*?*/ 				eType = text::PageNumberType_NEXT;
/*N*/ 			rAny.setValue(&eType, ::getCppuType((const text::PageNumberType*)0));
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		rAny <<= OUString(sUserStr);
/*?*/ 		break;
/*?*/ 
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 10:25-------------------

--------------------------------------------------*/
/*N*/ BOOL SwPageNumberField::PutValue( const uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     BOOL bRet = TRUE;
/*N*/ 	sal_Int16 nSet;
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*?*/ 		rAny >>= nSet;
/*?*/ 
/*?*/ 		// TODO: woher kommen die defines?
/*?*/ 		if(nSet <= SVX_NUM_PAGEDESC )
/*?*/ 			SetFormat(nSet);
/*?*/ 		else
/*?*/ 			//exception(wrong_value)
/*?*/ 			;
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_USHORT1:
/*?*/ 		rAny >>= nSet;
/*?*/ 		nOffset = nSet;
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/         switch( SWUnoHelper::GetEnumAsInt32( rAny ) )
/*N*/ 		{
/*N*/ 			case text::PageNumberType_CURRENT:
/*N*/ 				nSubType = PG_RANDOM;
/*N*/ 			break;
/*N*/ 			case text::PageNumberType_PREV:
/*?*/ 				nSubType = PG_PREV;
/*?*/ 			break;
/*?*/             case text::PageNumberType_NEXT:
/*?*/                 nSubType = PG_NEXT;
/*?*/ 			break;
/*?*/ 			default:
/*?*/                 bRet = FALSE;
/*N*/ 		}
/*N*/ 		break;
/*?*/ 	case FIELD_PROP_PAR1:
/*?*/ 		::binfilter::GetString( rAny, sUserStr );
/*?*/ 		break;
/*?*/ 
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/     return bRet;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwAuthorFieldType
 --------------------------------------------------------------------*/

/*N*/ SwAuthorFieldType::SwAuthorFieldType()
/*N*/ 	: SwFieldType( RES_AUTHORFLD )
/*N*/ {
/*N*/ }

/*N*/ String SwAuthorFieldType::Expand(ULONG nFmt) const
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 	SvtUserOptions aOpt;
/*N*/ 	if((nFmt & 0xff) == AF_NAME)
/*N*/ 		sRet = aOpt.GetFullName();
/*N*/ 	else
/*N*/ 		sRet = aOpt.GetID();
/*N*/ 	return sRet;
/*N*/ }

/*N*/ SwFieldType* SwAuthorFieldType::Copy() const
/*N*/ {
        return new SwAuthorFieldType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwAuthorField
 --------------------------------------------------------------------*/

/*N*/ SwAuthorField::SwAuthorField(SwAuthorFieldType* pTyp, sal_uInt32 nFmt)
/*N*/ 	: SwField(pTyp, nFmt)
/*N*/ {
/*N*/ 	aContent = ((SwAuthorFieldType*)GetTyp())->Expand(GetFormat());
/*N*/ }

/*N*/ String SwAuthorField::Expand() const
/*N*/ {
/*N*/ 	if (!IsFixed())
/*N*/ 		((SwAuthorField*)this)->aContent =
/*N*/ 					((SwAuthorFieldType*)GetTyp())->Expand(GetFormat());
/*N*/ 
/*N*/ 	return aContent;
/*N*/ }

/*N*/ SwField* SwAuthorField::Copy() const
/*N*/ {
/*N*/ 	SwAuthorField *pTmp = new SwAuthorField( (SwAuthorFieldType*)GetTyp(),
/*N*/ 												GetFormat());
/*N*/ 	pTmp->SetExpansion(aContent);
/*N*/ 	return pTmp;
/*N*/ }

/*-----------------05.03.98 11:15-------------------

--------------------------------------------------*/
/*N*/ BOOL SwAuthorField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/ 	sal_Bool bVal;
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		bVal = GetFormat() == AF_NAME;
/*N*/ 		rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		bVal = IsFixed();
/*N*/ 		rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		rAny <<= ::rtl::OUString(GetContent());
/*?*/ 		break;
/*?*/ 
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 11:15-------------------

--------------------------------------------------*/
BOOL SwAuthorField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        SetFormat( *(sal_Bool*)rAny.getValue() ? AF_NAME : AF_SHORTCUT );
        break;

    case FIELD_PROP_BOOL2:
        if( *(sal_Bool*)rAny.getValue() )
            SetFormat( GetFormat() | AF_FIXED);
        else
            SetFormat( GetFormat() & ~AF_FIXED);
        break;

    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, aContent );
        break;

    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: SwFileNameFieldType
 --------------------------------------------------------------------*/

/*N*/ SwFileNameFieldType::SwFileNameFieldType(SwDoc *pDocument)
/*N*/ 	: SwFieldType( RES_FILENAMEFLD )
/*N*/ {
/*N*/ 	pDoc = pDocument;
/*N*/ }

/*M*/ String SwFileNameFieldType::Expand(ULONG nFmt) const
/*M*/ {
/*M*/ 	String aRet;
/*M*/ 	const SwDocShell* pDShell = pDoc->GetDocShell();
/*M*/ 	if( pDShell && pDShell->HasName() )
/*M*/ 	{
/*M*/ 		const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
/*M*/ 		switch( nFmt & ~FF_FIXED )
/*M*/ 		{
/*M*/ 			case FF_PATH:
/*M*/ 				{
/*M*/ 					if( INET_PROT_FILE == rURLObj.GetProtocol() )
/*M*/ 					{
/*M*/ 						INetURLObject aTemp(rURLObj);
/*M*/ 						aTemp.removeSegment();
/*M*/                         // #101947# last slash should belong to the pathname
/*M*/ 						aRet = aTemp.PathToFileName();//GetFull();
/*M*/ 					}
/*M*/ 					else
/*M*/ 					{
/*M*/ 						aRet = removePassword(
/*M*/ 									rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
/*M*/ 									INetURLObject::WAS_ENCODED, URL_DECODE );
/*M*/ 						aRet.Erase( aRet.Search( String(rURLObj.GetLastName(
/*M*/ 													URL_DECODE )) ) );
/*M*/ 					}
/*M*/ 				}
/*M*/ 				break;
/*M*/ 
/*M*/ 			case FF_NAME:
/*M*/ 				aRet = rURLObj.GetLastName( URL_DECODE );
/*M*/ 				break;
/*M*/ 
/*M*/ 			case FF_NAME_NOEXT:
/*M*/ 				aRet = rURLObj.GetBase();
/*M*/ 				break;
/*M*/ 
/*M*/ 			default:
/*M*/ 				if( INET_PROT_FILE == rURLObj.GetProtocol() )
/*M*/ 					aRet = rURLObj.GetFull();
/*M*/ 				else
/*M*/ 					aRet = removePassword(
/*M*/ 									rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
/*M*/ 									INetURLObject::WAS_ENCODED, URL_DECODE );
/*M*/ 		}
/*M*/ 	}
/*M*/ 	return aRet;
/*M*/ }

/*N*/ SwFieldType* SwFileNameFieldType::Copy() const
/*N*/ {
            SwFieldType *pTmp = new SwFileNameFieldType(pDoc);
            return pTmp;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwFileNameField
 --------------------------------------------------------------------*/

/*N*/ SwFileNameField::SwFileNameField(SwFileNameFieldType* pTyp, sal_uInt32 nFmt)
/*N*/ 	: SwField(pTyp, nFmt)
/*N*/ {
/*N*/ 	aContent = ((SwFileNameFieldType*)GetTyp())->Expand(GetFormat());
/*N*/ }

/*N*/ String SwFileNameField::Expand() const
/*N*/ {
/*N*/ 	if (!IsFixed())
/*N*/ 		((SwFileNameField*)this)->aContent = ((SwFileNameFieldType*)GetTyp())->Expand(GetFormat());
/*N*/ 
/*N*/ 	return aContent;
/*N*/ }

/*N*/ SwField* SwFileNameField::Copy() const
/*N*/ {
/*N*/ 	SwFileNameField *pTmp =
/*N*/ 		new SwFileNameField((SwFileNameFieldType*)GetTyp(), GetFormat());
/*N*/ 	pTmp->SetExpansion(aContent);
/*N*/ 
/*N*/ 	return pTmp;
/*N*/ }

/*-----------------05.03.98 08:59-------------------

--------------------------------------------------*/
/*N*/ BOOL SwFileNameField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		{
/*N*/         	sal_Int16 nRet;
/*N*/ 			switch( GetFormat() &(~FF_FIXED) )
/*N*/ 			{
/*N*/ 				case FF_PATH:
/*N*/ 					nRet = text::FilenameDisplayFormat::PATH;
/*N*/ 				break;
/*N*/ 				case FF_NAME_NOEXT:
/*N*/ 					nRet = text::FilenameDisplayFormat::NAME;
/*N*/ 				break;
/*N*/ 				case FF_NAME:
/*N*/ 					nRet = text::FilenameDisplayFormat::NAME_AND_EXT;
/*N*/ 				break;
/*N*/ 				default:	nRet = text::FilenameDisplayFormat::FULL;
/*N*/ 			}
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		{
/*N*/ 			BOOL bVal = IsFixed();
/*N*/ 			rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_PAR3:
/*?*/ 		rAny <<= OUString(GetContent());
/*?*/ 		break;
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 09:01-------------------

--------------------------------------------------*/
/*N*/ BOOL SwFileNameField::PutValue( const uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		{
/*N*/ 			//JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
/*N*/ 			//				called with a int32 value! But normally we need
/*N*/ 			//				here only a int16
/*N*/ 			sal_Int32 nType;
/*N*/ 			rAny >>= nType;
/*N*/ 			BOOL bFixed = IsFixed();
/*N*/ 			switch( nType )
/*N*/ 			{
/*N*/ 				case text::FilenameDisplayFormat::PATH:
/*N*/ 					nType = FF_PATH;
/*N*/ 				break;
/*N*/ 				case text::FilenameDisplayFormat::NAME:
/*N*/ 					nType = FF_NAME_NOEXT;
/*N*/ 				break;
/*N*/ 				case text::FilenameDisplayFormat::NAME_AND_EXT:
/*N*/ 					nType = FF_NAME;
/*N*/ 				break;
/*N*/ 				default:	nType = FF_PATHNAME;
/*N*/ 			}
/*N*/ 			if(bFixed)
/*?*/ 				nType |= FF_FIXED;
/*N*/ 			SetFormat(nType);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_BOOL2:
/*?*/ 		if( *(sal_Bool*)rAny.getValue() )
/*?*/ 			SetFormat( GetFormat() | FF_FIXED);
/*?*/ 		else
/*?*/ 			SetFormat( GetFormat() & ~FF_FIXED);
/*?*/ 		break;
/*?*/ 
/*?*/ 	case FIELD_PROP_PAR3:
/*?*/ 		::binfilter::GetString( rAny, aContent );
/*?*/ 		break;
/*?*/ 
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwTemplNameFieldType
 --------------------------------------------------------------------*/

/*N*/ SwTemplNameFieldType::SwTemplNameFieldType(SwDoc *pDocument)
/*N*/ 	: SwFieldType( RES_TEMPLNAMEFLD )
/*N*/ {
/*N*/ 	pDoc = pDocument;
/*N*/ }

/*N*/ String SwTemplNameFieldType::Expand(ULONG nFmt) const
/*N*/ {
/*N*/ 	ASSERT(nFmt >= FF_BEGIN && nFmt < FF_END, "Expand: kein guelt. Fmt!" );
/*N*/ 
/*N*/ 	String aRet;
/*N*/ 	const SfxDocumentInfo* pDInfo = pDoc->GetpInfo();
/*N*/ 
/*N*/ 	if( pDInfo )
/*N*/ 	{
/*N*/ 		if( FF_UI_NAME == nFmt )
/*N*/ 			aRet = pDInfo->GetTemplateName();
/*N*/ 		else if( pDInfo->GetTemplateFileName().Len() )
/*N*/ 		{
/*N*/ 			if( FF_UI_RANGE == nFmt )
/*N*/ 			{
                    DBG_ERROR("Strip!");
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				INetURLObject aPathName( pDInfo->GetTemplateFileName() );
/*N*/ 				if( FF_NAME == nFmt )
/*N*/ 					aRet = aPathName.GetName(URL_DECODE);
/*N*/ 				else if( FF_NAME_NOEXT == nFmt )
/*N*/ 					aRet = aPathName.GetBase();
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( FF_PATH == nFmt )
/*N*/ 					{
/*N*/ 						aPathName.removeSegment();
/*N*/ 						aRet = aPathName.GetFull();
/*N*/ 					}
/*N*/ 					else
/*N*/ 						aRet = aPathName.GetFull();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*N*/ SwFieldType* SwTemplNameFieldType::Copy() const
/*N*/ {
        SwFieldType *pTmp = new SwTemplNameFieldType(pDoc);
        return pTmp;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwTemplNameField
 --------------------------------------------------------------------*/

/*N*/ SwTemplNameField::SwTemplNameField(SwTemplNameFieldType* pTyp, sal_uInt32 nFmt)
/*N*/ 	: SwField(pTyp, nFmt)
/*N*/ {}

/*N*/ String SwTemplNameField::Expand() const
/*N*/ {
/*N*/ 	return((SwTemplNameFieldType*)GetTyp())->Expand(GetFormat());
/*N*/ }

/*N*/ SwField* SwTemplNameField::Copy() const
/*N*/ {
/*N*/ 	SwTemplNameField *pTmp =
/*N*/ 		new SwTemplNameField((SwTemplNameFieldType*)GetTyp(), GetFormat());
/*N*/ 	return pTmp;
/*N*/ }

/*-----------------05.03.98 08:59-------------------

--------------------------------------------------*/
/*N*/ BOOL SwTemplNameField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		{
/*N*/         	sal_Int16 nRet;
/*N*/ 			switch( GetFormat() )
/*N*/ 			{
/*N*/ 				case FF_PATH:		nRet = text::FilenameDisplayFormat::PATH; break;
/*N*/ 				case FF_NAME_NOEXT: nRet = text::FilenameDisplayFormat::NAME; break;
/*N*/ 				case FF_NAME: 		nRet = text::FilenameDisplayFormat::NAME_AND_EXT; break;
/*N*/ 				case FF_UI_RANGE: 	nRet = text::TemplateDisplayFormat::AREA; break;
/*N*/ 				case FF_UI_NAME:    nRet = text::TemplateDisplayFormat::TITLE;  break;
/*N*/ 				default:	nRet = text::FilenameDisplayFormat::FULL;
/*N*/ 
/*N*/ 			}
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 09:01-------------------

--------------------------------------------------*/
/*N*/ BOOL SwTemplNameField::PutValue( const uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		{
/*N*/ 			//JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
/*N*/ 			//				called with a int32 value! But normally we need
/*N*/ 			//				here only a int16
/*N*/ 			sal_Int32 nType;
/*N*/ 			rAny >>= nType;
/*N*/ 			switch( nType )
/*N*/ 			{
/*N*/ 			case text::FilenameDisplayFormat::PATH:
/*N*/ 				SetFormat(FF_PATH);
/*N*/ 			break;
/*N*/ 			case text::FilenameDisplayFormat::NAME:
/*N*/ 				SetFormat(FF_NAME_NOEXT);
/*N*/ 			break;
/*N*/ 			case text::FilenameDisplayFormat::NAME_AND_EXT:
/*N*/ 				SetFormat(FF_NAME);
/*N*/ 			break;
/*N*/ 			case text::TemplateDisplayFormat::AREA	:
/*N*/ 				SetFormat(FF_UI_RANGE);
/*N*/ 			break;
/*N*/ 			case text::TemplateDisplayFormat::TITLE  :
/*N*/ 				SetFormat(FF_UI_NAME);
/*N*/ 			break;
/*N*/ 			default:	SetFormat(FF_PATHNAME);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwDocStatFieldType
 --------------------------------------------------------------------*/

/*N*/ SwDocStatFieldType::SwDocStatFieldType(SwDoc* pDocument)
/*N*/ 	: SwFieldType( RES_DOCSTATFLD ), nNumberingType( SVX_NUM_ARABIC )
/*N*/ {
/*N*/ 	pDoc = pDocument;
/*N*/ }

/*N*/ String SwDocStatFieldType::Expand(sal_uInt16 nSubType, sal_uInt32 nFmt) const
/*N*/ {
/*N*/ 	sal_uInt32 nVal = 0;
/*N*/ 	const SwDocStat& rDStat = pDoc->GetDocStat();
/*N*/ 	switch( nSubType )
/*N*/ 	{
/*N*/ 		case DS_TBL:  nVal = rDStat.nTbl;   break;
/*N*/ 		case DS_GRF:  nVal = rDStat.nGrf;   break;
/*N*/ 		case DS_OLE:  nVal = rDStat.nOLE;   break;
/*N*/ 		case DS_PARA: nVal = rDStat.nPara;  break;
/*N*/ 		case DS_WORD: nVal = rDStat.nWord;  break;
/*N*/ 		case DS_CHAR: nVal = rDStat.nChar;  break;
/*N*/ 		case DS_PAGE:
/*N*/ 			if( pDoc->GetRootFrm() )
/*N*/ 				((SwDocStat	&)rDStat).nPage = pDoc->GetRootFrm()->GetPageNum();
/*N*/ 			nVal = rDStat.nPage;
/*N*/ 			if( SVX_NUM_PAGEDESC == nFmt )
/*?*/ 				nFmt = (sal_uInt32)nNumberingType;
/*N*/ 			break;
/*N*/ 		default:
/*?*/ 			ASSERT( sal_False, "SwDocStatFieldType::Expand: unbekannter SubType" );
/*N*/ 	}
/*N*/ 
/*N*/ 	String sRet;
/*N*/ 	if( nVal <= SHRT_MAX )
/*N*/ 		sRet = FormatNumber( (sal_uInt16)nVal, nFmt );
/*N*/ 	else
/*?*/ 		sRet = String::CreateFromInt32( nVal );
/*N*/ 	return sRet;
/*N*/ }

/*N*/ SwFieldType* SwDocStatFieldType::Copy() const
/*N*/ {
            SwDocStatFieldType *pTmp = new SwDocStatFieldType(pDoc);
            return pTmp;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwDocStatFieldType
                  Aus historischen Gruenden steht in nFormat der
                  SubType
 --------------------------------------------------------------------*/

/*N*/ SwDocStatField::SwDocStatField(SwDocStatFieldType* pTyp, sal_uInt16 nSub, sal_uInt32 nFmt)
/*N*/ 	: SwField(pTyp, nFmt),
/*N*/ 	nSubType(nSub)
/*N*/ {}

/*N*/ String SwDocStatField::Expand() const
/*N*/ {
/*N*/ 	return((SwDocStatFieldType*)GetTyp())->Expand(nSubType, GetFormat());
/*N*/ }

/*N*/ SwField* SwDocStatField::Copy() const
/*N*/ {
/*N*/ 	SwDocStatField *pTmp = new SwDocStatField(
/*N*/ 					(SwDocStatFieldType*)GetTyp(), nSubType, GetFormat() );
/*N*/ 	return pTmp;
/*N*/ }

/*N*/ sal_uInt16 SwDocStatField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

void SwDocStatField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}

/*N*/ void SwDocStatField::ChangeExpansion( const SwFrm* pFrm )
/*N*/ {
/*N*/ 	if( DS_PAGE == nSubType && SVX_NUM_PAGEDESC == GetFormat() )
/*N*/ 		((SwDocStatFieldType*)GetTyp())->SetNumFormat(
/*?*/ 				pFrm->FindPageFrm()->GetPageDesc()->GetNumType().GetNumberingType() );
/*N*/ }

/*-----------------05.03.98 11:38-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDocStatField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_USHORT2:
/*N*/ 		rAny <<= (sal_Int16)GetFormat();
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 11:38-------------------

--------------------------------------------------*/
BOOL SwDocStatField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    BOOL bRet = FALSE;
    nMId &= ~CONVERT_TWIPS;
    switch ( nMId )
    {
    case FIELD_PROP_USHORT2:
        {
            sal_Int16 nSet;
            rAny >>= nSet;
            if(nSet <= SVX_NUM_CHARS_LOWER_LETTER_N &&
                nSet != SVX_NUM_CHAR_SPECIAL &&
                    nSet != SVX_NUM_BITMAP)
            {
                SetFormat(nSet);
                bRet = TRUE;
            }
        }
        break;

    default:
        DBG_ERROR("illegal property");
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: DokumentinfoFields
 --------------------------------------------------------------------*/

/*N*/ SwDocInfoFieldType::SwDocInfoFieldType(SwDoc* pDc)
/*N*/ 	: SwValueFieldType( pDc, RES_DOCINFOFLD )
/*N*/ {
/*N*/ }

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwDocInfoFieldType::Copy() const
/*N*/ {
        SwDocInfoFieldType* pType = new SwDocInfoFieldType(GetDoc());
        return pType;
/*N*/ }

/*N*/ void lcl_GetLocalDataWrapper( ULONG nLang,
/*N*/ 							  LocaleDataWrapper **ppAppLocalData,
/*N*/ 							  LocaleDataWrapper **ppLocalData )
/*N*/ {
/*N*/ 	*ppAppLocalData = &GetAppLocaleData();
/*N*/ 	*ppLocalData = *ppAppLocalData;
/*N*/ 	if( nLang != SvxLocaleToLanguage( (*ppLocalData)->getLocale() ) )
/*N*/ 		*ppLocalData = new LocaleDataWrapper(
/*N*/ 						::legacy_binfilters::getLegacyProcessServiceFactory(),
/*N*/ 						SvxCreateLocale( nLang ) );
/*N*/ }

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwDocInfoFieldType::Expand( sal_uInt16 nSub, sal_uInt32 nFormat,
/*N*/ 									sal_uInt16 nLang) const
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 	LocaleDataWrapper *pAppLocalData = 0, *pLocalData = 0;
/*N*/ 	const SfxDocumentInfo*	pInf = GetDoc()->GetInfo();
/*N*/ 
/*N*/ 	sal_uInt16 nExtSub = nSub & 0xff00;
/*N*/ 	nSub &= 0xff;	// ExtendedSubTypes nicht beachten
/*N*/ 
/*N*/ 	switch(nSub)
/*N*/ 	{
/*N*/ 	case DI_TITEL:	aStr = pInf->GetTitle();	break;
/*N*/ 	case DI_THEMA:	aStr = pInf->GetTheme();	break;
/*N*/ 	case DI_KEYS:	aStr = pInf->GetKeywords(); break;
/*N*/ 	case DI_COMMENT:aStr = pInf->GetComment();	break;
/*N*/ 	case DI_INFO1:
/*N*/ 	case DI_INFO2:
/*N*/ 	case DI_INFO3:
/*N*/ 	case DI_INFO4:	aStr = pInf->GetUserKey(nSub - DI_INFO1).GetWord();break;
/*N*/ 	case DI_DOCNO:	aStr = String::CreateFromInt32(
/*N*/ 												pInf->GetDocumentNumber() );
/*N*/ 					break;
/*N*/ 	case DI_EDIT:
/*N*/ 		if ( !nFormat )
/*N*/ 		{
/*?*/ 			lcl_GetLocalDataWrapper( nLang, &pAppLocalData, &pLocalData );
/*?*/ 			aStr = pLocalData->getTime( pInf->GetTime(), sal_False, sal_False);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             double fVal = Time(pInf->GetTime()).GetTimeInDays();
/*N*/ 			aStr = ExpandValue(fVal, nFormat, nLang);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*N*/ 		{
/*N*/ 			SfxStamp aTmp;
/*N*/ 			aTmp = pInf->GetCreated();
/*N*/ 			if( nSub == DI_CREATE )
/*N*/ 				;		// das wars schon!!
/*N*/ 			else if( nSub == DI_CHANGE &&
/*N*/ 					(pInf->GetChanged().GetTime() != aTmp.GetTime() ||
/*N*/ 					(nExtSub & ~DI_SUB_FIXED) == DI_SUB_AUTHOR &&
/*N*/ 					pInf->GetDocumentNumber() > 1) )
/*N*/ 				aTmp = pInf->GetChanged();
/*N*/ 			else if( nSub == DI_PRINT &&
/*N*/ 					pInf->GetPrinted().GetTime() != aTmp.GetTime() )
/*N*/ 				aTmp = pInf->GetPrinted();
/*N*/ 			else
/*?*/ 				break;
/*N*/ 
/*N*/ 			if (aTmp.IsValid())
/*N*/ 			{
/*N*/ 				switch (nExtSub & ~DI_SUB_FIXED)
/*N*/ 				{
/*N*/ 				case DI_SUB_AUTHOR:
/*N*/ 					aStr = aTmp.GetName();
/*N*/ 					break;
/*N*/ 
/*N*/ 				case DI_SUB_TIME:
/*N*/ 					if (!nFormat)
/*N*/ 					{
/*N*/ 						lcl_GetLocalDataWrapper( nLang, &pAppLocalData,
/*N*/ 														&pLocalData );
/*N*/ 						aStr = pLocalData->getTime( aTmp.GetTime(),
/*N*/ 													sal_False, sal_False);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						// Numberformatter anwerfen!
/*?*/ 						double fVal = SwDateTimeField::GetDateTime( GetDoc(),
/*?*/                                                     aTmp.GetTime());
/*?*/ 						aStr = ExpandValue(fVal, nFormat, nLang);
/*N*/ 					}
/*N*/ 					break;
/*N*/ 
/*N*/ 				case DI_SUB_DATE:
/*N*/ 					if (!nFormat)
/*N*/ 					{
/*N*/ 						lcl_GetLocalDataWrapper( nLang, &pAppLocalData,
/*N*/ 												 &pLocalData );
/*N*/ 						aStr = pLocalData->getDate( aTmp.GetTime() );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						// Numberformatter anwerfen!
/*?*/ 						double fVal = SwDateTimeField::GetDateTime( GetDoc(),
/*?*/                                                     aTmp.GetTime());
/*?*/ 						aStr = ExpandValue(fVal, nFormat, nLang);
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pAppLocalData != pLocalData )
/*N*/ 		delete pLocalData;
/*N*/ 
/*N*/ 	return aStr;
/*N*/ }

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDocInfoField::SwDocInfoField(SwDocInfoFieldType* pType, sal_uInt16 nSub, sal_uInt32 nFmt) :
/*N*/ 	SwValueField(pType, nFmt), nSubType(nSub)
/*N*/ {
/*N*/ 	aContent = ((SwDocInfoFieldType*)GetTyp())->Expand(nSubType, nFmt, GetLanguage());
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwDocInfoField::Expand() const
/*N*/ {
/*N*/ 	if (!IsFixed())	// aContent fuer Umschaltung auf fixed mitpflegen
/*N*/ 		((SwDocInfoField*)this)->aContent = ((SwDocInfoFieldType*)GetTyp())->Expand(nSubType, GetFormat(), GetLanguage());
/*N*/ 
/*N*/ 	return aContent;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwDocInfoField::GetCntnt(sal_Bool bName) const
/*N*/ {
/*N*/ 	if ( bName )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 String aStr(SwFieldType::GetTypeStr(GetTypeId()));
/*N*/ 	}
/*N*/ 	return Expand();
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwField* SwDocInfoField::Copy() const
/*N*/ {
/*N*/ 	SwDocInfoField* pFld = new SwDocInfoField((SwDocInfoFieldType*)GetTyp(), nSubType, GetFormat());
/*N*/     pFld->SetAutomaticLanguage(IsAutomaticLanguage());
/*N*/ 	pFld->aContent = aContent;
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt16 SwDocInfoField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwDocInfoField::SetSubType(sal_uInt16 nSub)
{
    nSubType = nSub;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwDocInfoField::SetLanguage(sal_uInt16 nLng)
/*N*/ {
/*N*/ 	if (!GetFormat())
/*N*/ 		SwField::SetLanguage(nLng);
/*N*/ 	else
/*N*/ 		SwValueField::SetLanguage(nLng);
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwDocInfoField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		rAny <<= OUString(aContent);
/*?*/ 		break;
/*?*/ 
/*?*/ 	case FIELD_PROP_USHORT1:
/*?*/ 		rAny  <<= (sal_Int16)aContent.ToInt32();
/*?*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/ 			sal_Bool bVal = 0 != (nSubType & DI_SUB_FIXED);
/*N*/ 			rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny  <<= (sal_Int32)GetFormat();
/*N*/ 		break;
/*N*/ 
/*?*/ 	case FIELD_PROP_DOUBLE:
/*?*/ 		{
/*?*/ 			double fVal = GetValue();
/*?*/ 			rAny.setValue(&fVal, ::getCppuType(&fVal));
/*?*/ 		}
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_PAR3:
/*?*/ 		rAny <<= ::rtl::OUString(Expand());
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		{
/*N*/ 			sal_uInt16 nExtSub = (nSubType & 0xff00) & ~DI_SUB_FIXED;
/*N*/ 			sal_Bool bVal = (nExtSub == DI_SUB_DATE);
/*N*/ 			rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/         return SwField::QueryValue(rAny, nMId);
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwDocInfoField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    sal_Int32 nValue;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        if( nSubType & DI_SUB_FIXED )
            ::binfilter::GetString( rAny, aContent );
        break;

    case FIELD_PROP_USHORT1:
        if( nSubType & DI_SUB_FIXED )
        {
            rAny >>= nValue;
            aContent = String::CreateFromInt32(nValue);
        }
        break;

    case FIELD_PROP_BOOL1:
        if(*(sal_Bool*)rAny.getValue())
            nSubType |= DI_SUB_FIXED;
        else
            nSubType &= ~DI_SUB_FIXED;
        break;
    case FIELD_PROP_FORMAT:
        {
            rAny >>= nValue;
            if( nValue >= 0)
                SetFormat(nValue);
        }
        break;

    case FIELD_PROP_PAR3:
        ::binfilter::GetString( rAny, aContent );
        break;
    case FIELD_PROP_BOOL2:
        nSubType &= 0xf0ff;
        if(*(sal_Bool*)rAny.getValue())
            nSubType |= DI_SUB_DATE;
        else
            nSubType |= DI_SUB_TIME;
        break;
    default:
        return SwField::PutValue(rAny, nMId);
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: SwHiddenTxtFieldType by JP
 --------------------------------------------------------------------*/

/*N*/ SwHiddenTxtFieldType::SwHiddenTxtFieldType( sal_Bool bSetHidden )
/*N*/ 	: SwFieldType( RES_HIDDENTXTFLD ), bHidden( bSetHidden )
/*N*/ {}

/*N*/ SwFieldType* SwHiddenTxtFieldType::Copy() const
/*N*/ {
        return new SwHiddenTxtFieldType( bHidden );
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwHiddenTxtField::SwHiddenTxtField( SwHiddenTxtFieldType* pFldType,
/*N*/ 									sal_Bool 	bConditional,
/*N*/ 									const 	String& rCond,
/*N*/ 									const	String& rStr,
/*N*/ 									sal_Bool 	bHidden,
/*N*/ 									sal_uInt16  nSub) :
/*N*/ 	SwField( pFldType ), aCond(rCond), bValid(sal_False),
/*N*/ 	bCanToggle(bConditional), bIsHidden(bHidden), nSubType(nSub)
/*N*/ {
/*N*/ 	if(nSubType == TYP_CONDTXTFLD)
/*N*/ 	{
/*N*/ 		sal_uInt16 nPos = 0;
/*N*/ 		aTRUETxt = rStr.GetToken(0, '|', nPos);
/*N*/ 
/*N*/ 		if(nPos != STRING_NOTFOUND)
/*N*/ 		{
/*N*/ 			aFALSETxt = rStr.GetToken(0, '|', nPos);
/*N*/ 			if(nPos != STRING_NOTFOUND)
/*N*/ 			{
/*?*/ 				aContent = rStr.GetToken(0, '|', nPos);
/*?*/ 				bValid = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aTRUETxt = rStr;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwHiddenTxtField::SwHiddenTxtField( SwHiddenTxtFieldType* pFldType,
/*N*/ 									const String& rCond,
/*N*/ 									const String& rTrue,
/*N*/ 									const String& rFalse,
/*N*/ 									sal_uInt16 nSub)
/*N*/ 	: SwField( pFldType ), aCond(rCond), bIsHidden(sal_True), nSubType(nSub),
/*N*/ 	  aTRUETxt(rTrue), aFALSETxt(rFalse), bValid(sal_False)
/*N*/ {
/*N*/ 	bCanToggle	= aCond.Len() > 0;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwHiddenTxtField::Expand() const
/*N*/ {
/*N*/ 	// Type: !Hidden  -> immer anzeigen
/*N*/ 	// 		  Hide	  -> Werte die Bedingung aus
/*N*/ 
/*N*/ 	if( TYP_CONDTXTFLD == nSubType )
/*N*/ 	{
/*N*/ 		if( bValid )
/*?*/ 			return aContent;
/*N*/ 
/*N*/ 		if( bCanToggle && !bIsHidden )
/*?*/ 			return aTRUETxt;
/*N*/ 	}
/*N*/ 	else if( !((SwHiddenTxtFieldType*)GetTyp())->GetHiddenFlag() ||
/*N*/ 		( bCanToggle && bIsHidden ))
/*?*/ 		return aTRUETxt;
/*N*/ 
/*N*/ 	return aFALSETxt;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Aktuellen Field-Value holen und cachen
 --------------------------------------------------------------------*/

/*N*/ void SwHiddenTxtField::Evaluate(SwDoc* pDoc)
/*N*/ {
/*N*/ 	ASSERT(pDoc, Wo ist das Dokument Seniore);
/*N*/ 
/*N*/ 	if( TYP_CONDTXTFLD == nSubType )
/*N*/ 	{
/*N*/ 		SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
/*N*/ 
/*N*/ 		bValid = sal_False;
/*N*/ 		String sTmpName;
/*N*/ 
/*N*/ 		if (bCanToggle && !bIsHidden)
/*N*/ 			sTmpName = aTRUETxt;
/*N*/ 		else
/*N*/ 			sTmpName = aFALSETxt;
/*N*/ 
/*N*/ // OS 21.08.97: #42943# Datenbankausdruecke muessen sich von
/*N*/ // 				einfachem Text unterscheiden. also wird der einfache Text
/*N*/ // 				bevorzugt in Anfuehrungszeichen gesetzt.
/*N*/ // 				Sind diese vorhanden werden umschliessende entfernt.
/*N*/ //				Wenn nicht, dann wird auf die Tauglichkeit als Datenbankname
/*N*/ //				geprueft. Nur wenn zwei oder mehr Punkte vorhanden sind und kein
/*N*/ //				Anfuehrungszeichen enthalten ist, gehen wir von einer DB aus.
/*N*/ 		if(sTmpName.Len() > 1 && sTmpName.GetChar(0) == '\"' &&
/*N*/ 			sTmpName.GetChar((sTmpName.Len() - 1)))
/*N*/ 		{
/*N*/ 			aContent = sTmpName.Copy(1, sTmpName.Len() - 2);
/*N*/ 			bValid = sal_True;
/*N*/ 		}
/*N*/ 		else if(sTmpName.Search('\"') == STRING_NOTFOUND &&
/*N*/ 			sTmpName.GetTokenCount('.') > 2)
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwHiddenTxtField::GetCntnt(sal_Bool bName) const
/*N*/ {
/*N*/ 	if ( bName )
/*N*/ 	{
/*?*/ 		String aStr(SwFieldType::GetTypeStr(nSubType));
/*?*/ 		aStr += ' ';
/*?*/ 		aStr += aCond;
/*?*/ 		aStr += ' ';
/*?*/ 		aStr += aTRUETxt;
/*?*/ 
/*?*/ 		if(nSubType == TYP_CONDTXTFLD)
/*?*/ 		{
/*?*/ static char __READONLY_DATA cTmp[] = " : ";
/*?*/ 			aStr.AppendAscii(cTmp);
/*?*/ 			aStr += aFALSETxt;
/*?*/ 		}
/*?*/ 		return aStr;
/*N*/ 	}
/*N*/ 	return Expand();
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwField* SwHiddenTxtField::Copy() const
/*N*/ {
/*N*/ 	SwHiddenTxtField* pFld =
/*N*/ 		new SwHiddenTxtField((SwHiddenTxtFieldType*)GetTyp(), aCond,
/*N*/ 							  aTRUETxt, aFALSETxt);
/*N*/ 	pFld->bIsHidden = bIsHidden;
/*N*/ 	pFld->bValid	= bValid;
/*N*/ 	pFld->aContent	= aContent;
/*N*/ 	pFld->SetFormat(GetFormat());
/*N*/ 	pFld->nSubType 	= nSubType;
/*N*/ 	return pFld;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

/*N*/ void SwHiddenTxtField::SetPar1(const String& rStr)
/*N*/ {
/*N*/ 	aCond = rStr;
/*N*/ 	bCanToggle = aCond.Len() > 0;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const String& SwHiddenTxtField::GetPar1() const
/*N*/ {
/*N*/ 	return aCond;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: True/False Text
 --------------------------------------------------------------------*/

void SwHiddenTxtField::SetPar2(const String& rStr)
{
    if(nSubType == TYP_CONDTXTFLD)
    {
        sal_uInt16 nPos = rStr.Search('|');
        aTRUETxt = rStr.Copy(0, nPos);

        if(nPos != STRING_NOTFOUND)
            aFALSETxt = rStr.Copy(nPos + 1);
    }
    else
        aTRUETxt = rStr;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwHiddenTxtField::GetPar2() const
/*N*/ {
/*N*/ 	String aRet(aTRUETxt);
/*N*/ 	if(nSubType == TYP_CONDTXTFLD)
/*N*/ 	{
/*N*/ 		aRet += '|';
/*N*/ 		aRet += aFALSETxt;
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt16 SwHiddenTxtField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwHiddenTxtField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/ 	const String* pOut = 0;
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		pOut = &aCond;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		pOut = &aTRUETxt;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR3:
/*N*/ 		pOut = &aFALSETxt;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/         	sal_Bool bHidden = bIsHidden;
/*N*/         	rAny.setValue(&bHidden, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	if( pOut )
/*N*/ 		rAny <<= OUString( *pOut );
/*N*/ 	return sal_True;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwHiddenTxtField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        {
            String sVal;
            SetPar1(binfilter::GetString( rAny, sVal ));
        }
        break;
    case FIELD_PROP_PAR2:
        ::binfilter::GetString( rAny, aTRUETxt );
        break;
    case FIELD_PROP_PAR3:
        ::binfilter::GetString( rAny, aFALSETxt );
        break;
    case FIELD_PROP_BOOL1:
        bIsHidden = *(sal_Bool*)rAny.getValue();
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------


/*--------------------------------------------------------------------
    Beschreibung: Der Feldtyp fuer Zeilenhoehe 0
 --------------------------------------------------------------------*/

/*N*/ SwHiddenParaFieldType::SwHiddenParaFieldType()
/*N*/ 	: SwFieldType( RES_HIDDENPARAFLD )
/*N*/ {
/*N*/ }

/*N*/ SwFieldType* SwHiddenParaFieldType::Copy() const
/*N*/ {
            SwHiddenParaFieldType* pType = new SwHiddenParaFieldType();
            return pType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Das Feld Zeilenhoehe 0
 --------------------------------------------------------------------*/

/*N*/ SwHiddenParaField::SwHiddenParaField(SwHiddenParaFieldType* pType, const String& rStr)
/*N*/ 	: SwField(pType), aCond(rStr)
/*N*/ {
/*N*/ 	bIsHidden = sal_False;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwHiddenParaField::Expand() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwField* SwHiddenParaField::Copy() const
/*N*/ {
/*N*/ 	SwHiddenParaField* pFld = new SwHiddenParaField((SwHiddenParaFieldType*)GetTyp(), aCond);
/*N*/ 	pFld->bIsHidden = bIsHidden;
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }
/*-----------------05.03.98 13:25-------------------

--------------------------------------------------*/
/*N*/ BOOL SwHiddenParaField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		rAny <<= OUString(aCond);
/*N*/ 		break;
/*N*/ 	case  FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/         	sal_Bool bHidden = bIsHidden;
/*N*/         	rAny.setValue(&bHidden, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 13:25-------------------

--------------------------------------------------*/
BOOL SwHiddenParaField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch ( nMId )
    {
    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, aCond );
        break;
    case FIELD_PROP_BOOL1:
        bIsHidden = *(sal_Bool*)rAny.getValue();
        break;

    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

void SwHiddenParaField::SetPar1(const String& rStr)
{
    aCond = rStr;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const String& SwHiddenParaField::GetPar1() const
/*N*/ {
/*N*/ 	return aCond;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: PostIt
 --------------------------------------------------------------------*/

/*N*/ SwPostItFieldType::SwPostItFieldType()
/*N*/ 	: SwFieldType( RES_POSTITFLD )
/*N*/ {}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwPostItFieldType::Copy() const
/*N*/ {
            return new SwPostItFieldType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwPostItFieldType
 --------------------------------------------------------------------*/

/*?*/ SwPostItField::SwPostItField( SwPostItFieldType* pType,
/*?*/ 		const String& rAuthor, const String& rTxt, const Date& rDate )
/*?*/ 	: SwField( pType ), sTxt( rTxt ), sAuthor( rAuthor ), aDate( rDate )
/*?*/ {
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ String SwPostItField::Expand() const
/*?*/ {
/*?*/ 	return aEmptyStr;
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwField* SwPostItField::Copy() const
{
    return new SwPostItField( (SwPostItFieldType*)GetTyp(), sAuthor,
                                sTxt, aDate );
}
/*--------------------------------------------------------------------
    Beschreibung: Author setzen
 --------------------------------------------------------------------*/

void SwPostItField::SetPar1(const String& rStr)
{
    sAuthor = rStr;
}

const String& SwPostItField::GetPar1() const
{
    return sAuthor;
}

/*--------------------------------------------------------------------
    Beschreibung: Text fuers PostIt setzen
 --------------------------------------------------------------------*/

void SwPostItField::SetPar2(const String& rStr)
{
    sTxt = rStr;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
String SwPostItField::GetPar2() const
{
    return sTxt;
}

/*-----------------05.03.98 13:42-------------------

--------------------------------------------------*/
BOOL SwPostItField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString(sAuthor);
        break;
    case FIELD_PROP_PAR2:
        rAny <<= OUString(sTxt);
        break;
    case FIELD_PROP_DATE:
        {
            util::Date aSetDate;
            aSetDate.Day = aDate.GetDay();
            aSetDate.Month = aDate.GetMonth();
            aSetDate.Year = aDate.GetYear();
            rAny.setValue(&aSetDate, ::getCppuType((util::Date*)0));
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------05.03.98 13:42-------------------

--------------------------------------------------*/
BOOL SwPostItField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, sAuthor );
        break;
    case FIELD_PROP_PAR2:
        ::binfilter::GetString( rAny, sTxt );
        break;
    case FIELD_PROP_DATE:
        if( rAny.getValueType() == ::getCppuType((util::Date*)0) )
        {
            util::Date aSetDate = *(util::Date*)rAny.getValue();
            aDate = Date(aSetDate.Day, aSetDate.Month, aSetDate.Year);
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*--------------------------------------------------------------------
    Beschreibung: DokumentinfoFields
 --------------------------------------------------------------------*/

/*N*/ SwExtUserFieldType::SwExtUserFieldType()
/*N*/ 	: SwFieldType( RES_EXTUSERFLD )
/*N*/ {
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwExtUserFieldType::Copy() const
/*N*/ {
            SwExtUserFieldType* pType = new SwExtUserFieldType;
            return pType;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwExtUserFieldType::Expand(sal_uInt16 nSub, sal_uInt32 nFormat) const
/*N*/ {
/*N*/ 	SvxAddressItem aAdr;
/*N*/ 	String aRet( aEmptyStr );
/*N*/ 	sal_uInt16 nRet = USHRT_MAX;
/*N*/ 	switch(nSub)
/*N*/ 	{
/*N*/ 	case EU_FIRSTNAME:		aRet = aAdr.GetFirstName(); break;
/*N*/ 	case EU_NAME:   		aRet = aAdr.GetName();		break;
/*N*/ 	case EU_SHORTCUT: 		aRet = aAdr.GetShortName(); break;
/*N*/ 
/*N*/ 	case EU_COMPANY:  		nRet = POS_COMPANY; 		break;
/*N*/ 	case EU_STREET:			nRet = POS_STREET;			break;
/*N*/ 	case EU_TITLE:  		nRet = POS_TITLE;			break;
/*N*/ 	case EU_POSITION: 		nRet = POS_POSITION;		break;
/*N*/ 	case EU_PHONE_PRIVATE:	nRet = POS_TEL_PRIVATE; 	break;
/*N*/ 	case EU_PHONE_COMPANY:	nRet = POS_TEL_COMPANY; 	break;
/*N*/ 	case EU_FAX:			nRet = POS_FAX;         	break;
/*N*/ 	case EU_EMAIL:			nRet = POS_EMAIL;       	break;
/*N*/ 	case EU_COUNTRY:		nRet = POS_COUNTRY;     	break;
/*N*/ 	case EU_ZIP:			nRet = POS_PLZ;         	break;
/*N*/ 	case EU_CITY:			nRet = POS_CITY;        	break;
/*N*/ 	case EU_STATE:			nRet = POS_STATE;			break;
/*?*/ 	case EU_FATHERSNAME:	nRet = POS_FATHERSNAME;		break;
/*?*/ 	case EU_APARTMENT:		nRet = POS_APARTMENT;		break;
/*?*/ 	default:				ASSERT( !this, "Field unknown");
/*N*/ 	}
/*N*/ 	if( USHRT_MAX != nRet )
/*N*/ 		aRet = aAdr.GetToken( nRet );
/*N*/ 	return aRet;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwExtUserField::SwExtUserField(SwExtUserFieldType* pType, sal_uInt16 nSubTyp, sal_uInt32 nFmt) :
/*N*/ 	SwField(pType, nFmt), nType(nSubTyp)
/*N*/ {
/*N*/ 	aContent = ((SwExtUserFieldType*)GetTyp())->Expand(nType, GetFormat());
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwExtUserField::Expand() const
/*N*/ {
/*N*/ 	if (!IsFixed())
/*N*/ 		((SwExtUserField*)this)->aContent = ((SwExtUserFieldType*)GetTyp())->Expand(nType, GetFormat());
/*N*/ 
/*N*/ 	return aContent;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwField* SwExtUserField::Copy() const
/*N*/ {
/*N*/ 	SwExtUserField* pFld = new SwExtUserField((SwExtUserFieldType*)GetTyp(), nType, GetFormat());
/*N*/ 	pFld->SetExpansion(aContent);
/*N*/ 
/*N*/ 	return pFld;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ sal_uInt16 SwExtUserField::GetSubType() const
/*N*/ {
/*N*/ 	return nType;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwExtUserField::SetSubType(sal_uInt16 nSub)
{
    nType = nSub;
}

/*-----------------05.03.98 14:14-------------------

--------------------------------------------------*/
/*N*/ BOOL SwExtUserField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		rAny <<= OUString(aContent);
/*?*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		{
/*N*/ 			sal_Int16 nTmp = nType;
/*N*/ 			rAny <<= nTmp;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/ 			sal_Bool bTmp = IsFixed();
/*N*/ 			rAny.setValue(&bTmp, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 14:14-------------------

--------------------------------------------------*/
BOOL SwExtUserField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, aContent );
        break;

    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nTmp;
            rAny >>= nTmp;
            nType = nTmp;
        }
        break;
    case FIELD_PROP_BOOL1:
        if( *(sal_Bool*)rAny.getValue() )
            SetFormat(GetFormat() | AF_FIXED);
        else
            SetFormat(GetFormat() & ~AF_FIXED);
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
//-------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: Relatives Seitennummern - Feld
 --------------------------------------------------------------------*/

/*N*/ SwRefPageSetFieldType::SwRefPageSetFieldType()
/*N*/ 	: SwFieldType( RES_REFPAGESETFLD )
/*N*/ {
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwRefPageSetFieldType::Copy() const
/*N*/ {
            return new SwRefPageSetFieldType;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// ueberlagert, weil es nichts zum Updaten gibt!

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung
 --------------------------------------------------------------------*/

/*?*/ SwRefPageSetField::SwRefPageSetField( SwRefPageSetFieldType* pType,
/*?*/ 					short nOff, sal_Bool bFlag )
/*?*/ 	: SwField( pType ), nOffset( nOff ), bOn( bFlag )
/*?*/ {
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ String SwRefPageSetField::Expand() const
/*?*/ {
/*?*/ 	return aEmptyStr;
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwField* SwRefPageSetField::Copy() const
{
    return new SwRefPageSetField( (SwRefPageSetFieldType*)GetTyp(), nOffset, bOn );
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
String SwRefPageSetField::GetPar2() const
{
    return String::CreateFromInt32( GetOffset() );
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwRefPageSetField::SetPar2(const String& rStr)
{
    SetOffset( (short) rStr.ToInt32() );
}

/*-----------------05.03.98 14:52-------------------

--------------------------------------------------*/
BOOL SwRefPageSetField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        rAny.setValue(&bOn, ::getBooleanCppuType());
        break;
    case FIELD_PROP_USHORT1:
        rAny <<= (sal_Int16)nOffset;
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------05.03.98 14:52-------------------

--------------------------------------------------*/
BOOL SwRefPageSetField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        bOn = *(sal_Bool*)rAny.getValue();
        break;
    case FIELD_PROP_USHORT1:
        rAny >>=nOffset;
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*--------------------------------------------------------------------
    Beschreibung: relatives Seitennummern - Abfrage Feld
 --------------------------------------------------------------------*/

/*N*/ SwRefPageGetFieldType::SwRefPageGetFieldType( SwDoc* pDc )
/*N*/ 	: SwFieldType( RES_REFPAGEGETFLD ), nNumberingType( SVX_NUM_ARABIC ), pDoc( pDc )
/*N*/ {
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwRefPageGetFieldType::Copy() const
/*N*/ {
            SwRefPageGetFieldType* pNew = new SwRefPageGetFieldType( pDoc );
            pNew->nNumberingType = nNumberingType;
            return pNew;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwRefPageGetFieldType::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	// Update auf alle GetReferenz-Felder
/*N*/ 	if( !pNew && !pOld && GetDepends() )
/*N*/ 	{
/*?*/ 		// sammel erstmal alle SetPageRefFelder ein.
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 _SetGetExpFlds aTmpLst( 10, 5 );
/*N*/ 	}
/*N*/ 
/*N*/ 	// weiter an die Text-Felder, diese "Expandieren" den Text
/*N*/ 	SwModify::Modify( pOld, pNew );
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung Abfragen
 --------------------------------------------------------------------*/

/*?*/ SwRefPageGetField::SwRefPageGetField( SwRefPageGetFieldType* pType,
/*?*/ 									sal_uInt32 nFmt )
/*?*/ 	: SwField( pType, nFmt )
/*?*/ {
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*?*/ String SwRefPageGetField::Expand() const
/*?*/ {
/*?*/ 	return sTxt;
/*?*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwField* SwRefPageGetField::Copy() const
{
    SwRefPageGetField* pCpy = new SwRefPageGetField(
                        (SwRefPageGetFieldType*)GetTyp(), GetFormat() );
    pCpy->SetText( sTxt );
    return pCpy;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*-----------------05.03.98 14:52-------------------

--------------------------------------------------*/
BOOL SwRefPageGetField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
        case FIELD_PROP_USHORT1:
            rAny <<= (sal_Int16)GetFormat();
        break;
        case FIELD_PROP_PAR1:
            rAny <<= OUString(sTxt);
        break;
        default:
            DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------05.03.98 14:52-------------------

--------------------------------------------------*/
BOOL SwRefPageGetField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
        case FIELD_PROP_USHORT1:
        {
            sal_Int16 nSet;
            rAny >>= nSet;
            if(nSet <= SVX_NUM_PAGEDESC )
                SetFormat(nSet);
            else
                //exception(wrong_value)
                ;
        }
        break;
        case FIELD_PROP_PAR1:
        {
            OUString sTmp;
            rAny >>= sTmp;
            sTxt = sTmp;
        }
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Feld zum Anspringen und Editieren
 --------------------------------------------------------------------*/

/*N*/ SwJumpEditFieldType::SwJumpEditFieldType( SwDoc* pD )
/*N*/ 	: SwFieldType( RES_JUMPEDITFLD ), pDoc( pD ), aDep( this, 0 )
/*N*/ {
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwFieldType* SwJumpEditFieldType::Copy() const
/*N*/ {
            return new SwJumpEditFieldType( pDoc );
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwCharFmt* SwJumpEditFieldType::GetCharFmt()
/*N*/ {
/*N*/ 	SwCharFmt* pFmt = pDoc->GetCharFmtFromPool( RES_POOLCHR_JUMPEDIT );
/*N*/ 
/*N*/ 	// noch nicht registriert ?
/*N*/ 	if( !aDep.GetRegisteredIn() )
/*N*/ 		pFmt->Add( &aDep );		// anmelden
/*N*/ 
/*N*/ 	return pFmt;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwJumpEditField::SwJumpEditField( SwJumpEditFieldType* pTyp, sal_uInt32 nFormat,
/*N*/ 								const String& rTxt, const String& rHelp )
/*N*/ 	: SwField( pTyp, nFormat ), sTxt( rTxt ), sHelp( rHelp )
/*N*/ {
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwJumpEditField::Expand() const
/*N*/ {
/*N*/ 	String sTmp( '<' );
/*N*/ 	sTmp += sTxt;
/*N*/ 	return sTmp += '>';
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwField* SwJumpEditField::Copy() const
/*N*/ {
/*N*/ 	return new SwJumpEditField( (SwJumpEditFieldType*)GetTyp(), GetFormat(),
/*N*/ 								sTxt, sHelp );
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
// Platzhalter-Text

/*N*/ const String& SwJumpEditField::GetPar1() const
/*N*/ {
/*N*/ 	return sTxt;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwJumpEditField::SetPar1(const String& rStr)
{
    sTxt = rStr;
}

// HinweisText
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ String SwJumpEditField::GetPar2() const
/*N*/ {
/*N*/ 	return sHelp;
/*N*/ }
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwJumpEditField::SetPar2(const String& rStr)
{
    sHelp = rStr;
}

/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
/*N*/ BOOL SwJumpEditField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		{
/*N*/ 			sal_Int16 nRet;
/*N*/ 			switch( GetFormat() )
/*N*/ 			{
/*N*/ 			case JE_FMT_TABLE:  nRet = text::PlaceholderType::TABLE; break;
/*N*/ 			case JE_FMT_FRAME:  nRet = text::PlaceholderType::TEXTFRAME; break;
/*N*/ 			case JE_FMT_GRAPHIC:nRet = text::PlaceholderType::GRAPHIC; break;
/*N*/ 			case JE_FMT_OLE:	nRet = text::PlaceholderType::OBJECT; break;
/*N*/ //			case JE_FMT_TEXT:
/*N*/ 			default:
/*N*/ 				nRet = text::PlaceholderType::TEXT; break;
/*N*/ 			}
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1 :
/*N*/ 		rAny <<= OUString(sHelp);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2 :
/*?*/ 		 rAny <<= OUString(sTxt);
/*?*/ 		 break;
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
BOOL SwJumpEditField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_USHORT1:
        {
            //JP 24.10.2001: int32 because in UnoField.cxx a putvalue is
            //              called with a int32 value! But normally we need
            //              here only a int16
            sal_Int32 nSet;
            rAny >>= nSet;
            switch( nSet )
            {
                case text::PlaceholderType::TEXT     : SetFormat(JE_FMT_TEXT); break;
                case text::PlaceholderType::TABLE    : SetFormat(JE_FMT_TABLE); break;
                case text::PlaceholderType::TEXTFRAME: SetFormat(JE_FMT_FRAME); break;
                case text::PlaceholderType::GRAPHIC  : SetFormat(JE_FMT_GRAPHIC); break;
                case text::PlaceholderType::OBJECT   : SetFormat(JE_FMT_OLE); break;
            }
        }
        break;
    case FIELD_PROP_PAR1 :
        ::binfilter::GetString( rAny, sHelp );
        break;
    case FIELD_PROP_PAR2 :
         ::binfilter::GetString( rAny, sTxt);
         break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}


/*--------------------------------------------------------------------
    Beschreibung: Combined Character Fieldtype / Field
 --------------------------------------------------------------------*/

/*N*/ SwCombinedCharFieldType::SwCombinedCharFieldType()
/*N*/ 	: SwFieldType( RES_COMBINED_CHARS )
/*N*/ {
/*N*/ }

/*N*/ SwFieldType* SwCombinedCharFieldType::Copy() const
/*N*/ {
            return new SwCombinedCharFieldType;
/*N*/ }

/* --------------------------------------------------------------------*/

/*?*/ SwCombinedCharField::SwCombinedCharField( SwCombinedCharFieldType* pFTyp,
/*?*/ 											const String& rChars )
/*?*/ 	: SwField( pFTyp, 0 ),
/*?*/ 	sCharacters( rChars.Copy( 0, MAX_COMBINED_CHARACTERS ))
/*?*/ {
/*?*/ }

/*?*/ String	SwCombinedCharField::Expand() const
/*?*/ {
/*?*/ 	return sCharacters;
/*?*/ }

SwField* SwCombinedCharField::Copy() const
{
    return new SwCombinedCharField( (SwCombinedCharFieldType*)GetTyp(),
                                        sCharacters );
}

const String& SwCombinedCharField::GetPar1() const
{
    return sCharacters;
}

void SwCombinedCharField::SetPar1(const String& rStr)
{
    sCharacters = rStr.Copy( 0, MAX_COMBINED_CHARACTERS );
}

BOOL SwCombinedCharField::QueryValue( ::com::sun::star::uno::Any& rAny,
                                        BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= ::rtl::OUString( sCharacters );
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

BOOL SwCombinedCharField::PutValue( const ::com::sun::star::uno::Any& rAny,
                                        BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, sCharacters ).Erase( MAX_COMBINED_CHARACTERS );
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
