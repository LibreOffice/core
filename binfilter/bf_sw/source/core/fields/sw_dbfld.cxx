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

#include <float.h>

#include <bf_svx/pageitem.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <fmtfld.hxx>
#include <txtfld.hxx>

#include <horiornt.hxx>

#include <docary.hxx>
#include <ndtxt.hxx>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <docfld.hxx>
#include <unofldmid.h>
namespace binfilter {
extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;
using namespace ::rtl;

/*--------------------------------------------------------------------
    Beschreibung: Datenbanktrenner durch Punkte fuer Anzeige ersetzen
 --------------------------------------------------------------------*/

/*N*/ String lcl_DBTrennConv(const String& aContent)
/*N*/ {
/*N*/ 	String sTmp(aContent);
/*N*/ 	sal_Unicode* pStr = sTmp.GetBufferAccess();
/*N*/ 	for( USHORT i = sTmp.Len(); i; --i, ++pStr )
/*N*/ 		if( DB_DELIM == *pStr )
/*N*/ 			*pStr = '.';
/*N*/ 	return sTmp;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: DatenbankFeldTyp
 --------------------------------------------------------------------*/

/*N*/ SwDBFieldType::SwDBFieldType(SwDoc* pDocPtr, const String& rNam, const SwDBData& rDBData ) :
/*N*/ 	SwValueFieldType( pDocPtr, RES_DBFLD ),
/*N*/ 	aDBData(rDBData),
/*N*/ 	nRefCnt(0),
/*N*/ 	sColumn(rNam)
/*N*/ {
/*N*/     if(aDBData.sDataSource.getLength() || aDBData.sCommand.getLength())
/*N*/     {
/*N*/         sName =  aDBData.sDataSource;
/*N*/         sName += DB_DELIM;
/*N*/         sName += (String)aDBData.sCommand;
/*N*/         sName += DB_DELIM;
/*N*/     }
/*N*/ 	sName += GetColumnName();
/*N*/ }
//------------------------------------------------------------------------------

/*N*/ SwFieldType* SwDBFieldType::Copy() const
/*N*/ {
/*N*/ 	SwDBFieldType* pTmp = new SwDBFieldType(GetDoc(), sColumn, aDBData);
/*N*/ 	return pTmp;
/*N*/ }

//------------------------------------------------------------------------------
/*N*/ const String& SwDBFieldType::GetName() const
/*N*/ {
/*N*/ 	return sName;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ void SwDBFieldType::ReleaseRef()
/*N*/ {
/*N*/ 	ASSERT(nRefCnt > 0, "RefCount kleiner 0!");
/*N*/ 
/*N*/ 	if (--nRefCnt <= 0)
/*N*/ 	{
/*N*/ 		USHORT nPos = GetDoc()->GetFldTypes()->GetPos(this);
/*N*/ 
/*N*/ 		if (nPos != USHRT_MAX)
/*N*/ 		{
/*N*/ 			GetDoc()->RemoveFldType(nPos);
/*N*/ 			delete this;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/* -----------------24.02.99 14:51-------------------
 *
 * --------------------------------------------------*/
/*N*/ BOOL SwDBFieldType::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		rAny <<= aDBData.sDataSource;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR4:
/*N*/ 		rAny <<= aDBData.sCommand;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		rAny <<= OUString(sColumn);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SHORT1:
/*N*/ 		rAny <<= aDBData.nCommandType;
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/* -----------------24.02.99 14:51-------------------
 *
 * --------------------------------------------------*/
/*N*/ BOOL SwDBFieldType::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR2:
/*?*/ 		rAny >>= aDBData.sDataSource;
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_PAR4:
/*?*/ 		rAny >>= aDBData.sCommand;
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_PAR1:
/*?*/ 		{
/*?*/ 			String sTmp;
/*?*/ 			::binfilter::GetString( rAny, sTmp );
/*?*/ 			if( sTmp != sColumn )
/*?*/ 			{
/*?*/ 				sColumn = sTmp;
/*?*/ 				SwClientIter aIter( *this );
/*?*/ 				SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
/*?*/ 				while(pFld)
/*?*/ 				{
/*?*/ 					// Feld im Undo?
/*?*/ 					SwTxtFld *pTxtFld = pFld->GetTxtFld();
/*?*/ 					if(pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
/*?*/ 					{
/*?*/ 						SwDBField* pDBField = (SwDBField*)pFld->GetFld();
/*?*/ 						pDBField->ClearInitialized();
/*?*/ 						pDBField->InitContent();
/*?*/  					}
/*?*/ 					pFld = (SwFmtFld*)aIter.Next();
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_SHORT1:
/*N*/ 		rAny >>= aDBData.nCommandType;
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwDBField
 --------------------------------------------------------------------*/

/*N*/ SwDBField::SwDBField(SwDBFieldType* pTyp, ULONG nFmt)
/*N*/ 	: 	SwValueField(pTyp, nFmt),
/*N*/ 		bValidValue(FALSE),
/*N*/ 		bIsInBodyTxt(TRUE),
/*N*/ 		bInitialized(FALSE),
/*N*/ 		nSubType(0)
/*N*/ {
/*N*/ 	if (GetTyp())
/*N*/ 		((SwDBFieldType*)GetTyp())->AddRef();
/*N*/ 	InitContent();
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwDBField::~SwDBField()
/*N*/ {
/*N*/ 	if (GetTyp())
/*N*/ 		((SwDBFieldType*)GetTyp())->ReleaseRef();
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ void SwDBField::InitContent()
/*N*/ {
/*N*/ 	if (!IsInitialized())
/*N*/ 	{
/*N*/ 		aContent = '<';
/*N*/ 		aContent += ((SwDBFieldType*)GetTyp())->GetColumnName();
/*N*/ 		aContent += '>';
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ void SwDBField::InitContent(const String& rExpansion)
/*N*/ {
/*N*/ 	if (rExpansion.Len() > 2)
/*N*/ 	{
/*N*/ 		if (rExpansion.GetChar(0) == '<' &&
/*N*/ 			rExpansion.GetChar(rExpansion.Len() - 1) == '>')
/*N*/ 		{
/*N*/ 			String sColumn( rExpansion.Copy( 1, rExpansion.Len() - 2 ) );
/*N*/ 			if( ::binfilter::GetAppCmpStrIgnore().isEqual( sColumn,
/*N*/ 							((SwDBFieldType *)GetTyp())->GetColumnName() ))
/*N*/ 			{
/*N*/ 				InitContent();
/*N*/ 				return;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SetExpansion( rExpansion );
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ String SwDBField::GetOldContent()
/*N*/ {
/*N*/ 	String sOldExpand = Expand();
/*N*/ 	String sNewExpand = sOldExpand;
/*N*/ 	BOOL bOldInit = bInitialized;
/*N*/ 
/*N*/ 	bInitialized = FALSE;
/*N*/ 	InitContent();
/*N*/ 	bInitialized = bOldInit;
/*N*/ 
/*N*/ 	if( ::binfilter::GetAppCmpStrIgnore().isEqual( sNewExpand, Expand() ) )
/*N*/ 	{
/*N*/ 		sNewExpand = '<';
/*N*/ 		sNewExpand += ((SwDBFieldType *)GetTyp())->GetColumnName();
/*N*/ 		sNewExpand += '>';
/*N*/ 	}
/*N*/ 	SetExpansion( sOldExpand );
/*N*/ 
/*N*/ 	return sNewExpand;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ String SwDBField::Expand() const
/*N*/ {
/*N*/     String sRet;
/*N*/     if(0 ==(GetSubType() & SUB_INVISIBLE))
/*N*/         sRet = lcl_DBTrennConv(aContent);
/*N*/     return sRet;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwField* SwDBField::Copy() const
/*N*/ {
/*N*/ 	SwDBField *pTmp = new SwDBField((SwDBFieldType*)GetTyp(), GetFormat());
/*N*/ 	pTmp->aContent 		= aContent;
/*N*/ 	pTmp->bIsInBodyTxt 	= bIsInBodyTxt;
/*N*/ 	pTmp->bValidValue 	= bValidValue;
/*N*/ 	pTmp->bInitialized	= bInitialized;
/*N*/ 	pTmp->nSubType		= nSubType;
/*N*/ 	pTmp->SetValue(GetValue());
/*N*/ 
/*N*/ 	return pTmp;
/*N*/ }


//------------------------------------------------------------------------------

void SwDBField::ChgValue( double d, BOOL bVal )
{
    bValidValue = bVal;
    SetValue(d);

    if( bValidValue )
        aContent = ((SwValueFieldType*)GetTyp())->ExpandValue(d, GetFormat(), GetLanguage());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFieldType* SwDBField::ChgTyp( SwFieldType* pNewType )
{
    SwFieldType* pOld = SwValueField::ChgTyp( pNewType );

    ((SwDBFieldType*)pNewType)->AddRef();
    ((SwDBFieldType*)pOld)->ReleaseRef();

    return pOld;
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuellen Field-Value holen und chachen
 --------------------------------------------------------------------*/

/*N*/ void SwDBField::Evaluate()
 /*N*/ {    DBG_ERROR("STRIP");
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Namen erfragen
 --------------------------------------------------------------------*/

const String& SwDBField::GetPar1() const
{
    return ((SwDBFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ USHORT SwDBField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ void SwDBField::SetSubType(USHORT nType)
/*N*/ {
/*N*/ 	nSubType = nType;
/*N*/ }

/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/ 			BOOL bTemp = 0 == (GetSubType()&SUB_OWN_FMT);
/*N*/ 			rAny.setValue(&bTemp, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/     case FIELD_PROP_BOOL2:
/*N*/     {
/*N*/         sal_Bool bVal = 0 == (GetSubType() & SUB_INVISIBLE);
/*N*/         rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/     }
/*N*/     break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= (sal_Int32)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		rAny <<= OUString(aContent);
/*?*/ 		break;
/*?*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ 
/*N*/ }
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		if( *(sal_Bool*)rAny.getValue() )
/*N*/ 			SetSubType(GetSubType()&~SUB_OWN_FMT);
/*N*/ 		else
/*N*/ 			SetSubType(GetSubType()|SUB_OWN_FMT);
/*N*/ 		break;
/*N*/     case FIELD_PROP_BOOL2:
/*N*/     {    
/*N*/         USHORT nSubType = GetSubType();
/*N*/         sal_Bool bVisible;
/*N*/         if(!(rAny >>= bVisible))
/*N*/             return FALSE;
/*N*/         if(bVisible)
/*N*/             nSubType &= ~SUB_INVISIBLE;
/*N*/         else
/*N*/             nSubType |= SUB_INVISIBLE;
/*N*/         SetSubType(nSubType);
/*N*/         //invalidate text node 
/*N*/         if(GetTyp())
/*N*/         {        
/*N*/             SwClientIter aIter( *GetTyp() );
/*N*/             SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
/*N*/             while(pFld)
/*N*/             {
/*N*/                 SwTxtFld *pTxtFld = pFld->GetTxtFld();
/*N*/                 if(pTxtFld && (SwDBField*)pFld->GetFld() == this )
/*N*/                 {
/*N*/                     //notify the change
/*N*/                     pTxtFld->NotifyContentChange(*pFld);
/*N*/                     break;
/*N*/                 }
/*N*/                 pFld = (SwFmtFld*)aIter.Next();
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/     break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		{
/*N*/ 			sal_Int32 nTemp;
/*N*/ 			rAny >>= nTemp;
/*N*/ 			SetFormat(nTemp);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		::binfilter::GetString( rAny, aContent );
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Basisklasse fuer alle weiteren Datenbankfelder
 --------------------------------------------------------------------*/

/*N*/ SwDBNameInfField::SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, ULONG nFmt) :
/*N*/ 	SwField(pTyp, nFmt),
/*N*/     aDBData(rDBData),
/*N*/     nSubType(0)
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwDBData SwDBNameInfField::GetDBData(SwDoc* pDoc)
/*N*/ {
/*N*/ 	SwDBData aRet;
/*N*/ 	if(aDBData.sDataSource.getLength())
/*N*/ 		aRet = aDBData;
/*N*/ 	else
/*N*/ 		aRet = pDoc->GetDBData();
/*N*/ 	return aRet;
/*N*/ }

//------------------------------------------------------------------------------

/*-----------------06.03.98 16:55-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBNameInfField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		rAny <<= aDBData.sDataSource;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		rAny <<= aDBData.sCommand;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SHORT1:
/*N*/ 		rAny <<= aDBData.nCommandType;
/*N*/ 		break;
/*N*/     case FIELD_PROP_BOOL2:
/*N*/     {
/*N*/         sal_Bool bVal = 0 == (GetSubType() & SUB_INVISIBLE);
/*N*/         rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/     }
/*N*/     break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*-----------------06.03.98 16:55-------------------

--------------------------------------------------*/
BOOL SwDBNameInfField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny >>= aDBData.nCommandType;
        break;
    case FIELD_PROP_BOOL2:
    {    
        USHORT nSubType = GetSubType();
        sal_Bool bVisible;
        if(!(rAny >>= bVisible))
            return FALSE;
        if(bVisible)
            nSubType &= ~SUB_INVISIBLE;
        else
            nSubType |= SUB_INVISIBLE;
        SetSubType(nSubType);
    }
    break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/* -----------------4/10/2003 15:03------------------

 --------------------------------------------------*/
/*N*/ USHORT SwDBNameInfField::GetSubType() const
/*N*/ {
/*N*/     return nSubType;
/*N*/ }        
/* -----------------4/10/2003 15:03------------------

 --------------------------------------------------*/
/*N*/ void SwDBNameInfField::SetSubType(USHORT nType)
/*N*/ {
/*N*/     nSubType = nType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: NaechsterDatensatz
 --------------------------------------------------------------------*/

/*N*/ SwDBNextSetFieldType::SwDBNextSetFieldType()
/*N*/ 	: SwFieldType( RES_DBNEXTSETFLD )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwFieldType* SwDBNextSetFieldType::Copy() const
/*N*/ {
            SwDBNextSetFieldType* pTmp = new SwDBNextSetFieldType();
            return pTmp;
/*N*/ }
/*--------------------------------------------------------------------
    Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

/*N*/ SwDBNextSetField::SwDBNextSetField(SwDBNextSetFieldType* pTyp,
/*N*/ 								   const String& rCond,
/*N*/ 								   const String& rDummy,
/*N*/ 								   const SwDBData& rDBData) :
/*N*/ 	SwDBNameInfField(pTyp, rDBData), aCond(rCond), bCondValid(TRUE)
/*N*/ {}

//------------------------------------------------------------------------------

/*N*/ String SwDBNextSetField::Expand() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwField* SwDBNextSetField::Copy() const
/*N*/ {
/*N*/ 	SwDBNextSetField *pTmp = new SwDBNextSetField((SwDBNextSetFieldType*)GetTyp(),
/*N*/ 										 aCond, aEmptyStr, GetDBData());
/*N*/     pTmp->SetSubType(GetSubType());
/*N*/ 	pTmp->bCondValid = bCondValid;
/*N*/ 	return pTmp;
/*N*/ }
//------------------------------------------------------------------------------

/*N*/ void SwDBNextSetField::Evaluate(SwDoc* pDoc)
/*N*/ {
/*N*/ 	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
/*N*/ 	const SwDBData& rData = GetDBData();
/*N*/ 	if( !bCondValid ||
/*N*/             !pMgr || !pMgr->IsDataSourceOpen(rData.sDataSource, rData.sCommand, sal_False))
/*N*/ 		return ;
/*N*/     pMgr->ToNextRecord(rData.sDataSource, rData.sCommand);
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Bedingung
 --------------------------------------------------------------------*/

/*N*/ const String& SwDBNextSetField::GetPar1() const
/*N*/ {
/*N*/ 	return aCond;
/*N*/ }

void SwDBNextSetField::SetPar1(const String& rStr)
{
    aCond = rStr;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBNextSetField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR3:
/*N*/ 		rAny <<= OUString(aCond);
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		bRet = SwDBNameInfField::QueryValue( rAny, nMId );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNextSetField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        ::binfilter::GetString( rAny, aCond );
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nMId );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
/*
String SwDBNextSetField::GetPar2() const
{
    return GetDBName();
}

void SwDBNextSetField::SetPar2(const String& rStr)
{
    GetDBName() = rStr;
}
*/

/*--------------------------------------------------------------------
    Beschreibung: Datensatz mit bestimmter ID
 --------------------------------------------------------------------*/

/*N*/ SwDBNumSetFieldType::SwDBNumSetFieldType() :
/*N*/ 	SwFieldType( RES_DBNUMSETFLD )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwFieldType* SwDBNumSetFieldType::Copy() const
/*N*/ {
        SwDBNumSetFieldType* pTmp = new SwDBNumSetFieldType();
        return pTmp;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

/*N*/ SwDBNumSetField::SwDBNumSetField(SwDBNumSetFieldType* pTyp,
/*N*/ 								 const String& rCond,
/*N*/ 								 const String& rDBNum,
/*N*/ 								 const SwDBData& rDBData) :
/*N*/ 	SwDBNameInfField(pTyp, rDBData),
/*N*/ 	aCond(rCond),
/*N*/ 	aPar2(rDBNum),
/*N*/ 	bCondValid(TRUE)
/*N*/ {}

//------------------------------------------------------------------------------

/*N*/ String SwDBNumSetField::Expand() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwField* SwDBNumSetField::Copy() const
/*N*/ {
/*N*/ 	SwDBNumSetField *pTmp = new SwDBNumSetField((SwDBNumSetFieldType*)GetTyp(),
/*N*/ 										 aCond, aPar2, GetDBData());
/*N*/ 	pTmp->bCondValid = bCondValid;
/*N*/     pTmp->SetSubType(GetSubType());
/*N*/ 	return pTmp;
/*N*/ }

/*N*/ void SwDBNumSetField::Evaluate(SwDoc* pDoc)
/*N*/ {
/*N*/ 	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
/*N*/ 	const SwDBData& aTmpData = GetDBData();
/*N*/ 
/*N*/ 	if( bCondValid && pMgr && pMgr->IsInMerge() &&
/*N*/                         pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_True))
/*N*/ 	{	// Bedingug OK -> aktuellen Set einstellen
/*N*/ 		pMgr->ToRecordId(Max((USHORT)aPar2.ToInt32(), USHORT(1))-1);
/*N*/ 	}
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: LogDBName
 --------------------------------------------------------------------*/

/*N*/ const String& SwDBNumSetField::GetPar1() const
/*N*/ {
/*N*/ 	return aCond;
/*N*/ }

void SwDBNumSetField::SetPar1(const String& rStr)
{
    aCond = rStr;
}

/*--------------------------------------------------------------------
    Beschreibung: Bedingung
 --------------------------------------------------------------------*/

/*N*/ String SwDBNumSetField::GetPar2() const
/*N*/ {
/*N*/ 	return aPar2;
/*N*/ }

void SwDBNumSetField::SetPar2(const String& rStr)
{
    aPar2 = rStr;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBNumSetField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR3:
/*N*/ 		rAny <<= OUString(aCond);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= (sal_Int32)aPar2.ToInt32();
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		bRet = SwDBNameInfField::QueryValue(rAny, nMId );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL    SwDBNumSetField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        ::binfilter::GetString( rAny, aCond );
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nVal;
            rAny >>= nVal;
            aPar2 = String::CreateFromInt32(nVal);
        }
        break;
    default:
        bRet = SwDBNameInfField::PutValue(rAny, nMId );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

/*N*/ SwDBNameFieldType::SwDBNameFieldType(SwDoc* pDocument)
/*N*/ 	: SwFieldType( RES_DBNAMEFLD )
/*N*/ {
/*N*/ 	pDoc = pDocument;
/*N*/ }
//------------------------------------------------------------------------------

/*N*/ String SwDBNameFieldType::Expand(ULONG nFmt) const
/*N*/ {
/*N*/ 	ASSERT( nFmt >= FF_BEGIN && nFmt < FF_END, "Expand: kein guelt. Fmt!" );
/*N*/ 	const SwDBData aData = pDoc->GetDBData();
/*N*/ 	String sRet(aData.sDataSource);
/*N*/ 	sRet += '.';
/*N*/ 	sRet += (String)aData.sCommand;
/*N*/ 	return sRet;
/*N*/ }
//------------------------------------------------------------------------------

/*N*/ SwFieldType* SwDBNameFieldType::Copy() const
/*N*/ {
        SwDBNameFieldType *pTmp = new SwDBNameFieldType(pDoc);
        return pTmp;
/*N*/ }

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: Name der angedockten DB
 --------------------------------------------------------------------*/

/*N*/ SwDBNameField::SwDBNameField(SwDBNameFieldType* pTyp, const SwDBData& rDBData, ULONG nFmt)
/*N*/ 	: SwDBNameInfField(pTyp, rDBData, nFmt)
/*N*/ {}

//------------------------------------------------------------------------------

/*N*/ String SwDBNameField::Expand() const
/*N*/ {
/*N*/     String sRet;
/*N*/     if(0 ==(GetSubType() & SUB_INVISIBLE))
/*N*/         sRet = ((SwDBNameFieldType*)GetTyp())->Expand(GetFormat());
/*N*/     return sRet;
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwField* SwDBNameField::Copy() const
/*N*/ {
/*N*/ 	SwDBNameField *pTmp = new SwDBNameField((SwDBNameFieldType*)GetTyp(), GetDBData());
/*N*/ 	pTmp->ChangeFormat(GetFormat());
/*N*/ 	pTmp->SetLanguage(GetLanguage());
/*N*/     pTmp->SetSubType(GetSubType());
/*N*/ 	return pTmp;
/*N*/ }

/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBNameField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/ 	return SwDBNameInfField::QueryValue(rAny, nMId );
/*N*/ }
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNameField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
{
    return SwDBNameInfField::PutValue(rAny, nMId );
}
/*--------------------------------------------------------------------
    Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

/*N*/ SwDBSetNumberFieldType::SwDBSetNumberFieldType()
/*N*/ 	: SwFieldType( RES_DBSETNUMBERFLD )
/*N*/ {
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ SwFieldType* SwDBSetNumberFieldType::Copy() const
/*N*/ {
        SwDBSetNumberFieldType *pTmp = new SwDBSetNumberFieldType;
        return pTmp;
/*N*/ }

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: SetNumber der angedockten DB
 --------------------------------------------------------------------*/

/*N*/ SwDBSetNumberField::SwDBSetNumberField(SwDBSetNumberFieldType* pTyp,
/*N*/ 									   const SwDBData& rDBData,
/*N*/ 									   ULONG nFmt)
/*N*/ 	: SwDBNameInfField(pTyp, rDBData, nFmt), nNumber(0)
/*N*/ {}

//------------------------------------------------------------------------------

/*N*/ String SwDBSetNumberField::Expand() const
/*N*/ {
/*N*/     if(0 !=(GetSubType() & SUB_INVISIBLE) || nNumber == 0)
/*?*/ 		return aEmptyStr;
/*N*/ 	else
/*N*/ 		return FormatNumber((USHORT)nNumber, GetFormat());
/*N*/ 	//return(nNumber == 0 ? aEmptyStr : FormatNumber(nNumber, GetFormat()));
/*N*/ }

//------------------------------------------------------------------------------

/*N*/ void SwDBSetNumberField::Evaluate(SwDoc* pDoc)
/*N*/ {
/*N*/ 	SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
/*N*/ 
/*N*/     const SwDBData& aTmpData = GetDBData();
/*N*/     if (!pMgr || !pMgr->IsInMerge() || 
/*N*/         !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_False))
/*N*/         return;
/*N*/     nNumber = pMgr->GetSelectedRecordId();
/*N*/ }


//------------------------------------------------------------------------------

/*N*/ SwField* SwDBSetNumberField::Copy() const
/*N*/ {
/*N*/ 	SwDBSetNumberField *pTmp =
/*N*/ 		new SwDBSetNumberField((SwDBSetNumberFieldType*)GetTyp(), GetDBData(), GetFormat());
/*N*/ 	pTmp->SetLanguage(GetLanguage());
/*N*/ 	pTmp->SetSetNumber(nNumber);
/*N*/     pTmp->SetSubType(GetSubType());
/*N*/ 	return pTmp;
/*N*/ }
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
/*N*/ BOOL SwDBSetNumberField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		rAny <<= (sal_Int16)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= nNumber;
/*N*/ 		break;
/*N*/ 	default:
/*N*/ 		bRet = SwDBNameInfField::QueryValue( rAny, nMId );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
BOOL SwDBSetNumberField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
{
    BOOL bRet = TRUE;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nSet;
            rAny >>= nSet;
            if(nSet < (INT16) SVX_NUMBER_NONE )
                SetFormat(nSet);
            else
                //exception(wrong_value)
                ;
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nNumber;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nMId );
    }
    return bRet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
