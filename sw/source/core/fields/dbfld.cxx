/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <float.h>
#include <sfx2/app.hxx>
#include <svl/zforlist.hxx>
#include <svx/pageitem.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docary.hxx>
#include <frame.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <docfld.hxx>
#include <expfld.hxx>
#include <txtatr.hxx>
#include <unofldmid.h>
#include <calbck.hxx>

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;

/// replace database separator by dots for display
static OUString lcl_DBTrennConv(const OUString& aContent)
{
    return aContent.replaceAll(OUString(DB_DELIM), ".");
}

// database field type

SwDBFieldType::SwDBFieldType(SwDoc* pDocPtr, const OUString& rNam, const SwDBData& rDBData ) :
    SwValueFieldType( pDocPtr, RES_DBFLD ),
    aDBData(rDBData),
    sName(rNam),
    sColumn(rNam),
    nRefCnt(0)
{
    if(!aDBData.sDataSource.isEmpty() || !aDBData.sCommand.isEmpty())
    {
        sName = aDBData.sDataSource
            + OUString(DB_DELIM)
            + aDBData.sCommand
            + OUString(DB_DELIM)
            + sName;
    }
}

SwDBFieldType::~SwDBFieldType()
{
}

SwFieldType* SwDBFieldType::Copy() const
{
    SwDBFieldType* pTmp = new SwDBFieldType(GetDoc(), sColumn, aDBData);
    return pTmp;
}

OUString SwDBFieldType::GetName() const
{
    return sName;
}

void SwDBFieldType::ReleaseRef()
{
    OSL_ENSURE(nRefCnt > 0, "RefCount < 0!");

    if (--nRefCnt <= 0)
    {
        size_t nPos = GetDoc()->getIDocumentFieldsAccess().GetFieldTypes()->GetPos(this);

        if (nPos != SIZE_MAX)
        {
            GetDoc()->getIDocumentFieldsAccess().RemoveFieldType(nPos);
            delete this;
        }
    }
}

bool SwDBFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= sColumn;
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= aDBData.nCommandType;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwDBFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        {
            OUString sTmp;
            rAny >>= sTmp;
            if( sTmp != sColumn )
            {
                sColumn = sTmp;
                SwIterator<SwFormatField,SwFieldType> aIter( *this );
                SwFormatField* pFormatField = aIter.First();
                while(pFormatField)
                {
                    // field in Undo?
                    SwTextField *pTextField = pFormatField->GetTextField();
                    if(pTextField && pTextField->GetTextNode().GetNodes().IsDocNodes() )
                    {
                        SwDBField* pDBField = static_cast<SwDBField*>(pFormatField->GetField());
                        pDBField->ClearInitialized();
                        pDBField->InitContent();
                    }
                    pFormatField = aIter.Next();
                }
            }
        }
        break;
    case FIELD_PROP_SHORT1:
        rAny >>= aDBData.nCommandType;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// database field

SwDBField::SwDBField(SwDBFieldType* pTyp, sal_uLong nFormat)
    :   SwValueField(pTyp, nFormat),
        nSubType(0),
        bIsInBodyText(true),
        bValidValue(false),
        bInitialized(false)
{
    if (GetTyp())
        static_cast<SwDBFieldType*>(GetTyp())->AddRef();
    InitContent();
}

SwDBField::~SwDBField()
{
    if (GetTyp())
        static_cast<SwDBFieldType*>(GetTyp())->ReleaseRef();
}

void SwDBField::InitContent()
{
    if (!IsInitialized())
    {
        aContent = "<" + static_cast<const SwDBFieldType*>(GetTyp())->GetColumnName() + ">";
    }
}

void SwDBField::InitContent(const OUString& rExpansion)
{
    if (rExpansion.startsWith("<") && rExpansion.endsWith(">"))
    {
        const OUString sColumn( rExpansion.copy( 1, rExpansion.getLength() - 2 ) );
        if( ::GetAppCmpStrIgnore().isEqual( sColumn,
                        static_cast<SwDBFieldType *>(GetTyp())->GetColumnName() ))
        {
            InitContent();
            return;
        }
    }
    SetExpansion( rExpansion );
}

OUString SwDBField::Expand() const
{
    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        return lcl_DBTrennConv(aContent);
    return OUString();
}

SwField* SwDBField::Copy() const
{
    SwDBField *pTmp = new SwDBField(static_cast<SwDBFieldType*>(GetTyp()), GetFormat());
    pTmp->aContent      = aContent;
    pTmp->bIsInBodyText  = bIsInBodyText;
    pTmp->bValidValue   = bValidValue;
    pTmp->bInitialized  = bInitialized;
    pTmp->nSubType      = nSubType;
    pTmp->SetValue(GetValue());
    pTmp->sFieldCode = sFieldCode;

    return pTmp;
}

OUString SwDBField::GetFieldName() const
{
    const OUString rDBName = static_cast<SwDBFieldType*>(GetTyp())->GetName();

    OUString sContent( rDBName.getToken(0, DB_DELIM) );

    if (sContent.getLength() > 1)
    {
        sContent += OUString(DB_DELIM)
            + rDBName.getToken(1, DB_DELIM)
            + OUString(DB_DELIM)
            + rDBName.getToken(2, DB_DELIM);
    }
    return lcl_DBTrennConv(sContent);
}

void SwDBField::ChgValue( double d, bool bVal )
{
    bValidValue = bVal;
    SetValue(d);

    if( bValidValue )
        aContent = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(d, GetFormat(), GetLanguage());
}

SwFieldType* SwDBField::ChgTyp( SwFieldType* pNewType )
{
    SwFieldType* pOld = SwValueField::ChgTyp( pNewType );

    static_cast<SwDBFieldType*>(pNewType)->AddRef();
    static_cast<SwDBFieldType*>(pOld)->ReleaseRef();

    return pOld;
}

bool SwDBField::FormatValue( SvNumberFormatter* pDocFormatter, OUString &aString, sal_uInt32 nFormat,
                             double &aNumber, sal_Int32 nColumnType, SwDBField *pField )
{
    bool bValidValue = false;

    if( DBL_MAX != aNumber )
    {
        if( DataType::DATE == nColumnType || DataType::TIME == nColumnType ||
            DataType::TIMESTAMP  == nColumnType )
        {
            Date aStandard( 1, 1, 1900 );
            if( *pDocFormatter->GetNullDate() != aStandard )
                aNumber += (aStandard - *pDocFormatter->GetNullDate());
        }
        bValidValue = true;
        if( pField )
            pField->SetValue( aNumber );
    }
    else
    {
        SwSbxValue aVal;
        aVal.PutString( aString );

        if (aVal.IsNumeric())
        {
            if( pField )
                pField->SetValue(aVal.GetDouble());
            else
                aNumber = aVal.GetDouble();

            if (nFormat && nFormat != SAL_MAX_UINT32 && !pDocFormatter->IsTextFormat(nFormat))
                bValidValue = true; // because of bug #60339 not for all strings
        }
        else
        {
            // if string length > 0 then true, else false
            if( pField )
                pField->SetValue(aString.isEmpty() ? 0 : 1);
            else
                aNumber = aString.isEmpty() ? 0 : 1;
        }
    }

    return bValidValue;
}

/// get current field value and cache it
void SwDBField::Evaluate()
{
    SwDBManager* pMgr = GetDoc()->GetDBManager();

    // first delete
    bValidValue = false;
    double nValue = DBL_MAX;
    const SwDBData& aTmpData = GetDBData();

    if(!pMgr || !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, true))
        return ;

    sal_uInt32 nFormat = 0;

    // search corresponding column name
    OUString aColNm( static_cast<SwDBFieldType*>(GetTyp())->GetColumnName() );

    SvNumberFormatter* pDocFormatter = GetDoc()->GetNumberFormatter();
    pMgr->GetMergeColumnCnt(aColNm, GetLanguage(), aContent, &nValue);
    if( !( nSubType & nsSwExtendedSubType::SUB_OWN_FMT ) )
        SetFormat( nFormat = pMgr->GetColumnFormat( aTmpData.sDataSource, aTmpData.sCommand,
                                        aColNm, pDocFormatter, GetLanguage() ));

    sal_Int32 nColumnType = nValue == DBL_MAX
        ? 0
        : pMgr->GetColumnType(aTmpData.sDataSource, aTmpData.sCommand, aColNm);

    bValidValue = FormatValue( pDocFormatter, aContent, nFormat, nValue, nColumnType, this );

    if( DBL_MAX != nValue )
        aContent = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(nValue, GetFormat(), GetLanguage());

    bInitialized = true;
}

/// get name
OUString SwDBField::GetPar1() const
{
    return static_cast<const SwDBFieldType*>(GetTyp())->GetName();
}

sal_uInt16 SwDBField::GetSubType() const
{
    return nSubType;
}

void SwDBField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}

bool SwDBField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        rAny <<= 0 == (GetSubType()&nsSwExtendedSubType::SUB_OWN_FMT);
        break;
    case FIELD_PROP_BOOL2:
        rAny <<= 0 == (GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE);
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    case FIELD_PROP_PAR1:
        rAny <<= aContent;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= sFieldCode;
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwDBField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_BOOL1:
        if( *static_cast<sal_Bool const *>(rAny.getValue()) )
            SetSubType(GetSubType()&~nsSwExtendedSubType::SUB_OWN_FMT);
        else
            SetSubType(GetSubType()|nsSwExtendedSubType::SUB_OWN_FMT);
        break;
    case FIELD_PROP_BOOL2:
    {
        sal_uInt16 nSubTyp = GetSubType();
        bool bVisible = false;
        if(!(rAny >>= bVisible))
            return false;
        if(bVisible)
            nSubTyp &= ~nsSwExtendedSubType::SUB_INVISIBLE;
        else
            nSubTyp |= nsSwExtendedSubType::SUB_INVISIBLE;
        SetSubType(nSubTyp);
        //invalidate text node
        if(GetTyp())
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *GetTyp() );
            SwFormatField* pFormatField = aIter.First();
            while(pFormatField)
            {
                SwTextField *pTextField = pFormatField->GetTextField();
                if(pTextField && static_cast<SwDBField*>(pFormatField->GetField()) == this )
                {
                    //notify the change
                    pTextField->NotifyContentChange(*pFormatField);
                    break;
                }
                pFormatField = aIter.Next();
            }
        }
    }
    break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTemp = 0;
            rAny >>= nTemp;
            SetFormat(nTemp);
        }
        break;
    case FIELD_PROP_PAR1:
        rAny >>= aContent;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= sFieldCode;
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// base class for all further database fields

SwDBNameInfField::SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFormat) :
    SwField(pTyp, nFormat),
    aDBData(rDBData),
    nSubType(0)
{
}

SwDBData SwDBNameInfField::GetDBData(SwDoc* pDoc)
{
    SwDBData aRet;
    if(!aDBData.sDataSource.isEmpty())
        aRet = aDBData;
    else
        aRet = pDoc->GetDBData();
    return aRet;
}

void SwDBNameInfField::SetDBData(const SwDBData & rDBData)
{
    aDBData = rDBData;
}

OUString SwDBNameInfField::GetFieldName() const
{
    OUString sStr( SwField::GetFieldName() );
    if (!aDBData.sDataSource.isEmpty())
    {
        sStr += ":"
            + aDBData.sDataSource
            + OUString(DB_DELIM)
            + aDBData.sCommand;
    }
    return lcl_DBTrennConv(sStr);
}

bool SwDBNameInfField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= aDBData.nCommandType;
        break;
    case FIELD_PROP_BOOL2:
        rAny <<= 0 == (GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE);
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwDBNameInfField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
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
        sal_uInt16 nSubTyp = GetSubType();
        bool bVisible = false;
        if(!(rAny >>= bVisible))
            return false;
        if(bVisible)
            nSubTyp &= ~nsSwExtendedSubType::SUB_INVISIBLE;
        else
            nSubTyp |= nsSwExtendedSubType::SUB_INVISIBLE;
        SetSubType(nSubTyp);
    }
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

sal_uInt16 SwDBNameInfField::GetSubType() const
{
    return nSubType;
}

void SwDBNameInfField::SetSubType(sal_uInt16 nType)
{
    nSubType = nType;
}

// next dataset

SwDBNextSetFieldType::SwDBNextSetFieldType()
    : SwFieldType( RES_DBNEXTSETFLD )
{
}

SwFieldType* SwDBNextSetFieldType::Copy() const
{
    SwDBNextSetFieldType* pTmp = new SwDBNextSetFieldType();
    return pTmp;
}

// SwDBSetField

SwDBNextSetField::SwDBNextSetField(SwDBNextSetFieldType* pTyp,
                                   const OUString& rCond,
                                   const OUString& ,
                                   const SwDBData& rDBData) :
    SwDBNameInfField(pTyp, rDBData), aCond(rCond), bCondValid(true)
{}

OUString SwDBNextSetField::Expand() const
{
    return OUString();
}

SwField* SwDBNextSetField::Copy() const
{
    SwDBNextSetField *pTmp = new SwDBNextSetField(static_cast<SwDBNextSetFieldType*>(GetTyp()),
                                         aCond, OUString(), GetDBData());
    pTmp->SetSubType(GetSubType());
    pTmp->bCondValid = bCondValid;
    return pTmp;
}

void SwDBNextSetField::Evaluate(SwDoc* pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();
    const SwDBData& rData = GetDBData();
    if( !bCondValid ||
            !pMgr || !pMgr->IsDataSourceOpen(rData.sDataSource, rData.sCommand, false))
        return ;
    pMgr->ToNextRecord(rData.sDataSource, rData.sCommand);
}

/// get condition
OUString SwDBNextSetField::GetPar1() const
{
    return aCond;
}

/// set condition
void SwDBNextSetField::SetPar1(const OUString& rStr)
{
    aCond = rStr;
}

bool SwDBNextSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= aCond;
        break;
    default:
        bRet = SwDBNameInfField::QueryValue( rAny, nWhichId );
    }
    return bRet;
}

bool SwDBNextSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny >>= aCond;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
    }
    return bRet;
}

// dataset with certain ID

SwDBNumSetFieldType::SwDBNumSetFieldType() :
    SwFieldType( RES_DBNUMSETFLD )
{
}

SwFieldType* SwDBNumSetFieldType::Copy() const
{
    SwDBNumSetFieldType* pTmp = new SwDBNumSetFieldType();
    return pTmp;
}

SwDBNumSetField::SwDBNumSetField(SwDBNumSetFieldType* pTyp,
                                 const OUString& rCond,
                                 const OUString& rDBNum,
                                 const SwDBData& rDBData) :
    SwDBNameInfField(pTyp, rDBData),
    aCond(rCond),
    aPar2(rDBNum),
    bCondValid(true)
{}

OUString SwDBNumSetField::Expand() const
{
    return OUString();
}

SwField* SwDBNumSetField::Copy() const
{
    SwDBNumSetField *pTmp = new SwDBNumSetField(static_cast<SwDBNumSetFieldType*>(GetTyp()),
                                         aCond, aPar2, GetDBData());
    pTmp->bCondValid = bCondValid;
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

void SwDBNumSetField::Evaluate(SwDoc* pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();
    const SwDBData& aTmpData = GetDBData();

    if( bCondValid && pMgr && pMgr->IsInMerge() &&
                        pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, true))
    {   // condition OK -> adjust current Set
        pMgr->ToRecordId(std::max((sal_uInt16)aPar2.toInt32(), sal_uInt16(1))-1);
    }
}

/// get LogDBName
OUString SwDBNumSetField::GetPar1() const
{
    return aCond;
}

/// set LogDBName
void SwDBNumSetField::SetPar1(const OUString& rStr)
{
    aCond = rStr;
}

/// get condition
OUString SwDBNumSetField::GetPar2() const
{
    return aPar2;
}

/// set condition
void SwDBNumSetField::SetPar2(const OUString& rStr)
{
    aPar2 = rStr;
}

bool SwDBNumSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= aCond;
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= aPar2.toInt32();
        break;
    default:
        bRet = SwDBNameInfField::QueryValue(rAny, nWhichId );
    }
    return bRet;
}

bool    SwDBNumSetField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny >>= aCond;
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nVal = 0;
            rAny >>= nVal;
            aPar2 = OUString::number(nVal);
        }
        break;
    default:
        bRet = SwDBNameInfField::PutValue(rAny, nWhichId );
    }
    return bRet;
}

SwDBNameFieldType::SwDBNameFieldType(SwDoc* pDocument)
    : SwFieldType( RES_DBNAMEFLD )
{
    pDoc = pDocument;
}

OUString SwDBNameFieldType::Expand(sal_uLong ) const
{
    const SwDBData aData = pDoc->GetDBData();
    return aData.sDataSource + "." + aData.sCommand;
}

SwFieldType* SwDBNameFieldType::Copy() const
{
    SwDBNameFieldType *pTmp = new SwDBNameFieldType(pDoc);
    return pTmp;
}

// name of the connected database

SwDBNameField::SwDBNameField(SwDBNameFieldType* pTyp, const SwDBData& rDBData)
    : SwDBNameInfField(pTyp, rDBData, 0)
{}

OUString SwDBNameField::Expand() const
{
    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        return static_cast<SwDBNameFieldType*>(GetTyp())->Expand(GetFormat());
    return OUString();
}

SwField* SwDBNameField::Copy() const
{
    SwDBNameField *pTmp = new SwDBNameField(static_cast<SwDBNameFieldType*>(GetTyp()), GetDBData());
    pTmp->ChangeFormat(GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

bool SwDBNameField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    return SwDBNameInfField::QueryValue(rAny, nWhichId );
}

bool SwDBNameField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    return SwDBNameInfField::PutValue(rAny, nWhichId );
}

SwDBSetNumberFieldType::SwDBSetNumberFieldType()
    : SwFieldType( RES_DBSETNUMBERFLD )
{
}

SwFieldType* SwDBSetNumberFieldType::Copy() const
{
    SwDBSetNumberFieldType *pTmp = new SwDBSetNumberFieldType;
    return pTmp;
}

// set-number of the connected database

SwDBSetNumberField::SwDBSetNumberField(SwDBSetNumberFieldType* pTyp,
                                       const SwDBData& rDBData,
                                       sal_uLong nFormat)
    : SwDBNameInfField(pTyp, rDBData, nFormat), nNumber(0)
{}

OUString SwDBSetNumberField::Expand() const
{
    if(0 !=(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE) || nNumber == 0)
        return OUString();
    return FormatNumber(nNumber, GetFormat());
}

void SwDBSetNumberField::Evaluate(SwDoc* pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();

    const SwDBData& aTmpData = GetDBData();
    if (!pMgr || !pMgr->IsInMerge() ||
        !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, false))
        return;
    nNumber = pMgr->GetSelectedRecordId();
}

SwField* SwDBSetNumberField::Copy() const
{
    SwDBSetNumberField *pTmp =
        new SwDBSetNumberField(static_cast<SwDBSetNumberFieldType*>(GetTyp()), GetDBData(), GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSetNumber(nNumber);
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

bool SwDBSetNumberField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        rAny <<= (sal_Int16)GetFormat();
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= nNumber;
        break;
    default:
        bRet = SwDBNameInfField::QueryValue( rAny, nWhichId );
    }
    return bRet;
}

bool SwDBSetNumberField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nSet = 0;
            rAny >>= nSet;
            if(nSet < (sal_Int16) SVX_NUMBER_NONE )
                SetFormat(nSet);
            else {
            }
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nNumber;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
