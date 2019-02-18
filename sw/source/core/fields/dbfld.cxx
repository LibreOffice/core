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
#include <o3tl/any.hxx>
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
static OUString lcl_DBSeparatorConvert(const OUString& aContent)
{
    return aContent.replaceAll(OUStringLiteral1(DB_DELIM), ".");
}

// database field type

SwDBFieldType::SwDBFieldType(SwDoc* pDocPtr, const OUString& rNam, const SwDBData& rDBData ) :
    SwValueFieldType( pDocPtr, SwFieldIds::Database ),
    m_aDBData(rDBData),
    m_sName(rNam),
    m_sColumn(rNam),
    m_nRefCnt(0)
{
    if(!m_aDBData.sDataSource.isEmpty() || !m_aDBData.sCommand.isEmpty())
    {
        m_sName = m_aDBData.sDataSource
            + OUStringLiteral1(DB_DELIM)
            + m_aDBData.sCommand
            + OUStringLiteral1(DB_DELIM)
            + m_sName;
    }
}

SwDBFieldType::~SwDBFieldType()
{
}

SwFieldType* SwDBFieldType::Copy() const
{
    SwDBFieldType* pTmp = new SwDBFieldType(GetDoc(), m_sColumn, m_aDBData);
    return pTmp;
}

OUString SwDBFieldType::GetName() const
{
    return m_sName;
}

void SwDBFieldType::ReleaseRef()
{
    OSL_ENSURE(m_nRefCnt > 0, "RefCount < 0!");

    if (--m_nRefCnt <= 0)
    {
        size_t nPos = GetDoc()->getIDocumentFieldsAccess().GetFieldTypes()->GetPos(this);

        if (nPos != SIZE_MAX)
        {
            GetDoc()->getIDocumentFieldsAccess().RemoveFieldType(nPos);
            delete this;
        }
    }
}

void SwDBFieldType::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= m_sColumn;
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= m_aDBData.nCommandType;
        break;
    default:
        assert(false);
    }
}

void SwDBFieldType::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        {
            OUString sTmp;
            rAny >>= sTmp;
            if( sTmp != m_sColumn )
            {
                m_sColumn = sTmp;
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
        rAny >>= m_aDBData.nCommandType;
        break;
    default:
        assert(false);
    }
}

// database field

SwDBField::SwDBField(SwDBFieldType* pTyp, sal_uInt32 nFormat)
    :   SwValueField(pTyp, nFormat),
        m_nSubType(0),
        m_bIsInBodyText(true),
        m_bValidValue(false),
        m_bInitialized(false)
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
        m_aContent = "<" + static_cast<const SwDBFieldType*>(GetTyp())->GetColumnName() + ">";
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

OUString SwDBField::ExpandImpl(SwRootFrame const*const) const
{
    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        return lcl_DBSeparatorConvert(m_aContent);
    return OUString();
}

std::unique_ptr<SwField> SwDBField::Copy() const
{
    std::unique_ptr<SwDBField> pTmp(new SwDBField(static_cast<SwDBFieldType*>(GetTyp()), GetFormat()));
    pTmp->m_aContent      = m_aContent;
    pTmp->m_bIsInBodyText  = m_bIsInBodyText;
    pTmp->m_bValidValue   = m_bValidValue;
    pTmp->m_bInitialized  = m_bInitialized;
    pTmp->m_nSubType      = m_nSubType;
    pTmp->SetValue(GetValue());
    pTmp->m_sFieldCode = m_sFieldCode;

    return std::unique_ptr<SwField>(pTmp.release());
}

OUString SwDBField::GetFieldName() const
{
    const OUString rDBName = static_cast<SwDBFieldType*>(GetTyp())->GetName();

    OUString sContent( rDBName.getToken(0, DB_DELIM) );

    if (sContent.getLength() > 1)
    {
        sContent += OUStringLiteral1(DB_DELIM)
            + rDBName.getToken(1, DB_DELIM)
            + OUStringLiteral1(DB_DELIM)
            + rDBName.getToken(2, DB_DELIM);
    }
    return lcl_DBSeparatorConvert(sContent);
}

void SwDBField::ChgValue( double d, bool bVal )
{
    m_bValidValue = bVal;
    SetValue(d);

    if( m_bValidValue )
        m_aContent = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(d, GetFormat(), GetLanguage());
}

SwFieldType* SwDBField::ChgTyp( SwFieldType* pNewType )
{
    SwFieldType* pOld = SwValueField::ChgTyp( pNewType );

    static_cast<SwDBFieldType*>(pNewType)->AddRef();
    static_cast<SwDBFieldType*>(pOld)->ReleaseRef();

    return pOld;
}

bool SwDBField::FormatValue( SvNumberFormatter const * pDocFormatter, OUString const &aString, sal_uInt32 nFormat,
                             double &aNumber, sal_Int32 nColumnType, SwDBField *pField )
{
    bool bValidValue = false;

    if( DBL_MAX != aNumber )
    {
        if( DataType::DATE == nColumnType || DataType::TIME == nColumnType ||
            DataType::TIMESTAMP  == nColumnType )
        {
            Date aStandard( 1, 1, 1900 );
            if( pDocFormatter->GetNullDate() != aStandard )
                aNumber += (aStandard - pDocFormatter->GetNullDate());
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
    m_bValidValue = false;
    double nValue = DBL_MAX;
    const SwDBData& aTmpData = GetDBData();

    if(!pMgr || !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, true))
        return ;

    sal_uInt32 nFormat = 0;

    // search corresponding column name
    OUString aColNm( static_cast<SwDBFieldType*>(GetTyp())->GetColumnName() );

    SvNumberFormatter* pDocFormatter = GetDoc()->GetNumberFormatter();
    pMgr->GetMergeColumnCnt(aColNm, GetLanguage(), m_aContent, &nValue);
    if( !( m_nSubType & nsSwExtendedSubType::SUB_OWN_FMT ) )
    {
        nFormat = pMgr->GetColumnFormat( aTmpData.sDataSource, aTmpData.sCommand,
                                        aColNm, pDocFormatter, GetLanguage() );
        SetFormat( nFormat );
    }

    sal_Int32 nColumnType = nValue == DBL_MAX
        ? 0
        : pMgr->GetColumnType(aTmpData.sDataSource, aTmpData.sCommand, aColNm);

    m_bValidValue = FormatValue( pDocFormatter, m_aContent, nFormat, nValue, nColumnType, this );

    if( DBL_MAX != nValue )
        m_aContent = static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(nValue, GetFormat(), GetLanguage());

    m_bInitialized = true;
}

/// get name
OUString SwDBField::GetPar1() const
{
    return static_cast<const SwDBFieldType*>(GetTyp())->GetName();
}

sal_uInt16 SwDBField::GetSubType() const
{
    return m_nSubType;
}

void SwDBField::SetSubType(sal_uInt16 nType)
{
    m_nSubType = nType;
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
        rAny <<= static_cast<sal_Int32>(GetFormat());
        break;
    case FIELD_PROP_PAR1:
        rAny <<= m_aContent;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_sFieldCode;
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
        if( *o3tl::doAccess<bool>(rAny) )
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
        rAny >>= m_aContent;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_sFieldCode;
    break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

// base class for all further database fields

SwDBNameInfField::SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uInt32 nFormat) :
    SwField(pTyp, nFormat),
    m_aDBData(rDBData),
    m_nSubType(0)
{
}

SwDBData SwDBNameInfField::GetDBData(SwDoc* pDoc)
{
    SwDBData aRet;
    if(!m_aDBData.sDataSource.isEmpty())
        aRet = m_aDBData;
    else
        aRet = pDoc->GetDBData();
    return aRet;
}

void SwDBNameInfField::SetDBData(const SwDBData & rDBData)
{
    m_aDBData = rDBData;
}

OUString SwDBNameInfField::GetFieldName() const
{
    OUString sStr( SwField::GetFieldName() );
    if (!m_aDBData.sDataSource.isEmpty())
    {
        sStr += ":"
            + m_aDBData.sDataSource
            + OUStringLiteral1(DB_DELIM)
            + m_aDBData.sCommand;
    }
    return lcl_DBSeparatorConvert(sStr);
}

bool SwDBNameInfField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= m_aDBData.nCommandType;
        break;
    case FIELD_PROP_BOOL2:
        rAny <<= 0 == (GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE);
        break;
    default:
        assert(false);
    }
    return true;
}

bool SwDBNameInfField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny >>= m_aDBData.nCommandType;
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
        assert(false);
    }
    return true;
}

sal_uInt16 SwDBNameInfField::GetSubType() const
{
    return m_nSubType;
}

void SwDBNameInfField::SetSubType(sal_uInt16 nType)
{
    m_nSubType = nType;
}

// next dataset

SwDBNextSetFieldType::SwDBNextSetFieldType()
    : SwFieldType( SwFieldIds::DbNextSet )
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
                                   const SwDBData& rDBData) :
    SwDBNameInfField(pTyp, rDBData), m_aCond(rCond), m_bCondValid(true)
{}

OUString SwDBNextSetField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

std::unique_ptr<SwField> SwDBNextSetField::Copy() const
{
    std::unique_ptr<SwDBNextSetField> pTmp(new SwDBNextSetField(static_cast<SwDBNextSetFieldType*>(GetTyp()),
                                         m_aCond, GetDBData()));
    pTmp->SetSubType(GetSubType());
    pTmp->m_bCondValid = m_bCondValid;
    return std::unique_ptr<SwField>(pTmp.release());
}

void SwDBNextSetField::Evaluate(SwDoc const * pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();
    const SwDBData& rData = GetDBData();
    if( !m_bCondValid ||
            !pMgr || !pMgr->IsDataSourceOpen(rData.sDataSource, rData.sCommand, false))
        return ;
    pMgr->ToNextRecord(rData.sDataSource, rData.sCommand);
}

/// get condition
OUString SwDBNextSetField::GetPar1() const
{
    return m_aCond;
}

/// set condition
void SwDBNextSetField::SetPar1(const OUString& rStr)
{
    m_aCond = rStr;
}

bool SwDBNextSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= m_aCond;
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
        rAny >>= m_aCond;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
    }
    return bRet;
}

// dataset with certain ID

SwDBNumSetFieldType::SwDBNumSetFieldType() :
    SwFieldType( SwFieldIds::DbNumSet )
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
    m_aCond(rCond),
    m_aPar2(rDBNum),
    m_bCondValid(true)
{}

OUString SwDBNumSetField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

std::unique_ptr<SwField> SwDBNumSetField::Copy() const
{
    std::unique_ptr<SwDBNumSetField> pTmp(new SwDBNumSetField(static_cast<SwDBNumSetFieldType*>(GetTyp()),
                                         m_aCond, m_aPar2, GetDBData()));
    pTmp->m_bCondValid = m_bCondValid;
    pTmp->SetSubType(GetSubType());
    return std::unique_ptr<SwField>(pTmp.release());
}

void SwDBNumSetField::Evaluate(SwDoc const * pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();
    const SwDBData& aTmpData = GetDBData();

    if( m_bCondValid && pMgr && pMgr->IsInMerge() &&
                        pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, true))
    {   // condition OK -> adjust current Set
        pMgr->ToRecordId(std::max(static_cast<sal_uInt16>(m_aPar2.toInt32()), sal_uInt16(1))-1);
    }
}

/// get LogDBName
OUString SwDBNumSetField::GetPar1() const
{
    return m_aCond;
}

/// set LogDBName
void SwDBNumSetField::SetPar1(const OUString& rStr)
{
    m_aCond = rStr;
}

/// get condition
OUString SwDBNumSetField::GetPar2() const
{
    return m_aPar2;
}

/// set condition
void SwDBNumSetField::SetPar2(const OUString& rStr)
{
    m_aPar2 = rStr;
}

bool SwDBNumSetField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= m_aCond;
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= m_aPar2.toInt32();
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
        rAny >>= m_aCond;
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nVal = 0;
            rAny >>= nVal;
            m_aPar2 = OUString::number(nVal);
        }
        break;
    default:
        bRet = SwDBNameInfField::PutValue(rAny, nWhichId );
    }
    return bRet;
}

SwDBNameFieldType::SwDBNameFieldType(SwDoc* pDocument)
    : SwFieldType( SwFieldIds::DatabaseName )
{
    m_pDoc = pDocument;
}

OUString SwDBNameFieldType::Expand() const
{
    const SwDBData aData = m_pDoc->GetDBData();
    return aData.sDataSource + "." + aData.sCommand;
}

SwFieldType* SwDBNameFieldType::Copy() const
{
    SwDBNameFieldType *pTmp = new SwDBNameFieldType(m_pDoc);
    return pTmp;
}

// name of the connected database

SwDBNameField::SwDBNameField(SwDBNameFieldType* pTyp, const SwDBData& rDBData)
    : SwDBNameInfField(pTyp, rDBData, 0)
{}

OUString SwDBNameField::ExpandImpl(SwRootFrame const*const) const
{
    if(0 ==(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE))
        return static_cast<SwDBNameFieldType*>(GetTyp())->Expand();
    return OUString();
}

std::unique_ptr<SwField> SwDBNameField::Copy() const
{
    std::unique_ptr<SwDBNameField> pTmp(new SwDBNameField(static_cast<SwDBNameFieldType*>(GetTyp()), GetDBData()));
    pTmp->ChangeFormat(GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSubType(GetSubType());
    return std::unique_ptr<SwField>(pTmp.release());
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
    : SwFieldType( SwFieldIds::DbSetNumber )
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
                                       sal_uInt32 nFormat)
    : SwDBNameInfField(pTyp, rDBData, nFormat), m_nNumber(0)
{}

OUString SwDBSetNumberField::ExpandImpl(SwRootFrame const*const) const
{
    if(0 !=(GetSubType() & nsSwExtendedSubType::SUB_INVISIBLE) || m_nNumber == 0)
        return OUString();
    return FormatNumber(m_nNumber, static_cast<SvxNumType>(GetFormat()));
}

void SwDBSetNumberField::Evaluate(SwDoc const * pDoc)
{
    SwDBManager* pMgr = pDoc->GetDBManager();

    const SwDBData& aTmpData = GetDBData();
    if (!pMgr || !pMgr->IsInMerge() ||
        !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, false))
        return;
    m_nNumber = pMgr->GetSelectedRecordId();
}

std::unique_ptr<SwField> SwDBSetNumberField::Copy() const
{
    std::unique_ptr<SwDBSetNumberField> pTmp(
        new SwDBSetNumberField(static_cast<SwDBSetNumberFieldType*>(GetTyp()), GetDBData(), GetFormat()));
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSetNumber(m_nNumber);
    pTmp->SetSubType(GetSubType());
    return std::unique_ptr<SwField>(pTmp.release());
}

bool SwDBSetNumberField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    bool bRet = true;
    switch( nWhichId )
    {
    case FIELD_PROP_USHORT1:
        rAny <<= static_cast<sal_Int16>(GetFormat());
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= m_nNumber;
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
            if(nSet < css::style::NumberingType::NUMBER_NONE )
                SetFormat(nSet);
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= m_nNumber;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nWhichId );
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
