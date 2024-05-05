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

#include <config_features.h>
#include <config_fuzzers.h>

#include <calc.hxx>
#include <cfloat>
#include <climits>
#include <memory>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <cstdlib>
#include <dbmgr.hxx>
#include <docfld.hxx>
#include <docstat.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <editeng/langitem.hxx>
#include <expfld.hxx>
#include <hintids.hxx>
#include <o3tl/temporary.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <shellres.hxx>
#include <svl/numformat.hxx>
#include <svl/languageoptions.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/useroptions.hxx>
#include <usrfld.hxx>
#include <utility>
#include <viewsh.hxx>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>

using namespace ::com::sun::star;


// ATTENTION: sorted list of all operators
struct CalcOp
{
    OUString aName;
    SwCalcOper eOp;
};

CalcOp constexpr aOpTable[] = {
/* ABS */     {{sCalc_Abs},        CALC_ABS},   // Abs (since LibreOffice 7.1)
/* ACOS */    {{sCalc_Acos},       CALC_ACOS},  // Arc cosine
/* ADD */     {{sCalc_Add},        CALC_PLUS},  // Addition
/* AND */     {{sCalc_And},        CALC_AND},   // log. AND
/* ASIN */    {{sCalc_Asin},       CALC_ASIN},  // Arc sine
/* ATAN */    {{sCalc_Atan},       CALC_ATAN},  // Arc tangent
/* AVERAGE */ {{sCalc_Average},    CALC_AVERAGE}, // Average (since LibreOffice 7.1)
/* COS */     {{sCalc_Cos},        CALC_COS},   // Cosine
/* COUNT */   {{sCalc_Count},      CALC_COUNT}, // Count (since LibreOffice 7.1)
/* DATE */    {{sCalc_Date},       CALC_DATE},  // Date
/* DIV */     {{sCalc_Div},        CALC_DIV},   // Division
/* EQ */      {{sCalc_Eq},         CALC_EQ},    // Equality
/* G */       {{sCalc_G},          CALC_GRE},   // Greater than
/* GEQ */     {{sCalc_Geq},        CALC_GEQ},   // Greater or equal
/* INT */     {{sCalc_Int},        CALC_INT},   // Int (since LibreOffice 7.4)
/* L */       {{sCalc_L},          CALC_LES},   // Less than
/* LEQ */     {{sCalc_Leq},        CALC_LEQ},   // Less or equal
/* MAX */     {{sCalc_Max},        CALC_MAX},   // Maximum value
/* MEAN */    {{sCalc_Mean},       CALC_MEAN},  // Mean
/* MIN */     {{sCalc_Min},        CALC_MIN},   // Minimum value
/* MUL */     {{sCalc_Mul},        CALC_MUL},   // Multiplication
/* NEQ */     {{sCalc_Neq},        CALC_NEQ},   // Not equal
/* NOT */     {{sCalc_Not},        CALC_NOT},   // log. NOT
/* OR */      {{sCalc_Or},         CALC_OR},    // log. OR
/* PHD */     {{sCalc_Phd},        CALC_PHD},   // Percentage
/* POW */     {{sCalc_Pow},        CALC_POW},   // Exponentiation
/* PRODUCT */ {{sCalc_Product},    CALC_PRODUCT}, // Product (since LibreOffice 7.1)
/* ROUND */   {{sCalc_Round},      CALC_ROUND}, // Rounding
/* SIGN */    {{sCalc_Sign},       CALC_SIGN},  // Sign (since LibreOffice 7.1)
/* SIN */     {{sCalc_Sin},        CALC_SIN},   // Sine
/* SQRT */    {{sCalc_Sqrt},       CALC_SQRT},  // Square root
/* SUB */     {{sCalc_Sub},        CALC_MINUS}, // Subtraction
/* SUM */     {{sCalc_Sum},        CALC_SUM},   // Sum
/* TAN */     {{sCalc_Tan},        CALC_TAN},   // Tangent
/* XOR */     {{sCalc_Xor},        CALC_XOR}    // log. XOR
};

double const nRoundVal[] = {
    5.0e+0, 0.5e+0, 0.5e-1, 0.5e-2, 0.5e-3, 0.5e-4, 0.5e-5, 0.5e-6,
    0.5e-7, 0.5e-8, 0.5e-9, 0.5e-10,0.5e-11,0.5e-12,0.5e-13,0.5e-14,
    0.5e-15,0.5e-16
};

// First character may be any alphabetic or underscore.
const sal_Int32 coStartFlags =
        i18n::KParseTokens::ANY_LETTER_OR_NUMBER |
        i18n::KParseTokens::ASC_UNDERSCORE |
        i18n::KParseTokens::IGNORE_LEADING_WS;

// Continuing characters may be any alphanumeric, underscore, or dot.
const sal_Int32 coContFlags =
    (coStartFlags | i18n::KParseTokens::ASC_DOT | i18n::KParseTokens::GROUP_SEPARATOR_IN_NUMBER)
        & ~i18n::KParseTokens::IGNORE_LEADING_WS;

extern "C" {
static int OperatorCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( CALC_NAME == static_cast<const CalcOp*>(pFirst)->eOp )
    {
        nRet = static_cast<const CalcOp*>(pFirst)->aName.compareTo(
                static_cast<const CalcOp*>(pSecond)->aName );
    }
    else
    {
        if( CALC_NAME == static_cast<const CalcOp*>(pSecond)->eOp )
            nRet = -1 * static_cast<const CalcOp*>(pSecond)->aName.compareTo(
                        static_cast<const CalcOp*>(pFirst)->aName );
        else
            nRet = static_cast<const CalcOp*>(pFirst)->aName.compareTo(
                     static_cast<const CalcOp*>(pSecond)->aName );
    }
    return nRet;
}
}// extern "C"

CalcOp* FindOperator( const OUString& rSrch )
{
    CalcOp aSrch;
    aSrch.aName = rSrch;
    aSrch.eOp = CALC_NAME;

    return static_cast<CalcOp*>(bsearch( static_cast<void*>(&aSrch),
                              static_cast<void const *>(aOpTable),
                              SAL_N_ELEMENTS( aOpTable ),
                              sizeof( CalcOp ),
                              OperatorCompare ));
}

// static
LanguageType SwCalc::GetDocAppScriptLang( SwDoc const & rDoc )
{
    TypedWhichId<SvxLanguageItem> nWhich =
               GetWhichOfScript( RES_CHRATR_LANGUAGE,
                                 SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ));
    return rDoc.GetDefault(nWhich).GetLanguage();
}

static double lcl_ConvertToDateValue( SwDoc& rDoc, sal_Int32 nDate )
{
    double nRet = 0;
    SvNumberFormatter* pFormatter = rDoc.GetNumberFormatter();
    if( pFormatter )
    {
        const Date& rNull = pFormatter->GetNullDate();
        Date aDate( nDate >> 24, (nDate& 0x00FF0000) >> 16, nDate& 0x0000FFFF );
        nRet = aDate - rNull;
    }
    return nRet;
}

SwCalc::SwCalc( SwDoc& rD )
    : m_aErrExpr( SwSbxValue(), nullptr )
    , m_nCommandPos(0)
    , m_rDoc( rD )
    , m_pCharClass( &GetAppCharClass() )
    , m_nListPor( 0 )
    , m_bHasNumber( false )
    , m_eCurrOper( CALC_NAME )
    , m_eCurrListOper( CALC_NAME )
    , m_eError( SwCalcError::NONE )
{
    LanguageType eLang = GetDocAppScriptLang( m_rDoc );
    LanguageTag aLanguageTag( eLang );

    if( eLang != m_pCharClass->getLanguageTag().getLanguageType() )
    {
        m_pCharClass = new CharClass( ::comphelper::getProcessComponentContext(), aLanguageTag );
    }
    m_xLocaleDataWrapper.reset(new LocaleDataWrapper( std::move(aLanguageTag) ));

    m_sCurrSym = comphelper::string::strip(m_xLocaleDataWrapper->getCurrSymbol(), ' ');
    m_sCurrSym  = m_pCharClass->lowercase( m_sCurrSym );

    static constexpr OUString sNTypeTab[]
    {
        u"false"_ustr,
        u"true"_ustr,
        u"pi"_ustr,
        u"e"_ustr,
        u"tables"_ustr,
        u"graf"_ustr,
        u"ole"_ustr,
        u"page"_ustr,
        u"para"_ustr,
        u"word"_ustr,
        u"char"_ustr,

        u"user_firstname"_ustr,
        u"user_lastname"_ustr,
        u"user_initials"_ustr,
        u"user_company"_ustr,
        u"user_street"_ustr,
        u"user_country"_ustr,
        u"user_zipcode"_ustr,
        u"user_city"_ustr,
        u"user_title"_ustr,
        u"user_position"_ustr,
        u"user_tel_work"_ustr,
        u"user_tel_home"_ustr,
        u"user_fax"_ustr,
        u"user_email"_ustr,
        u"user_state"_ustr,
        u"graph"_ustr
    };
    static UserOptToken const aAdrToken[ 12 ] =
    {
        UserOptToken::Company, UserOptToken::Street, UserOptToken::Country, UserOptToken::Zip,
        UserOptToken::City, UserOptToken::Title, UserOptToken::Position, UserOptToken::TelephoneWork,
        UserOptToken::TelephoneHome, UserOptToken::Fax, UserOptToken::Email, UserOptToken::State
    };

    static sal_uInt16 SwDocStat::* const aDocStat1[ 3 ] =
    {
        &SwDocStat::nTable, &SwDocStat::nGrf, &SwDocStat::nOLE
    };
    static sal_uLong SwDocStat::* const aDocStat2[ 4 ] =
    {
        &SwDocStat::nPage, &SwDocStat::nPara,
        &SwDocStat::nWord, &SwDocStat::nChar
    };

    const SwDocStat& rDocStat = m_rDoc.getIDocumentStatistics().GetDocStat();

    SwSbxValue nVal;
    sal_uInt16 n;

    for( n = 0; n < 25; ++n )
        m_aVarTable.insert( { sNTypeTab[n], SwCalcExp( nVal, nullptr ) } );

    m_aVarTable.find( sNTypeTab[ 0 ] )->second.nValue.PutBool( false );
    m_aVarTable.find( sNTypeTab[ 1 ] )->second.nValue.PutBool( true );
    m_aVarTable.find( sNTypeTab[ 2 ] )->second.nValue.PutDouble( M_PI );
    m_aVarTable.find( sNTypeTab[ 3 ] )->second.nValue.PutDouble( M_E );

    for( n = 0; n < 3; ++n )
        m_aVarTable.find( sNTypeTab[ n + 4 ] )->second.nValue.PutLong( rDocStat.*aDocStat1[ n ]  );
    for( n = 0; n < 4; ++n )
        m_aVarTable.find( sNTypeTab[ n + 7 ] )->second.nValue.PutLong( rDocStat.*aDocStat2[ n ]  );

    SvtUserOptions& rUserOptions = SW_MOD()->GetUserOptions();

    m_aVarTable.find( sNTypeTab[ 11 ] )->second.nValue.PutString( rUserOptions.GetFirstName() );
    m_aVarTable.find( sNTypeTab[ 12 ] )->second.nValue.PutString( rUserOptions.GetLastName() );
    m_aVarTable.find( sNTypeTab[ 13 ] )->second.nValue.PutString( rUserOptions.GetID() );

    for( n = 0; n < 11; ++n )
        m_aVarTable.find( sNTypeTab[ n + 14 ] )->second.nValue.PutString(
                                        rUserOptions.GetToken( aAdrToken[ n ] ));

    nVal.PutString( rUserOptions.GetToken( aAdrToken[ 11 ] ));
    m_aVarTable.insert( { sNTypeTab[ 25 ], SwCalcExp( nVal, nullptr ) } );

} // SwCalc::SwCalc

void SwCalc::ImplDestroy()
{
    if( m_pCharClass != &GetAppCharClass() )
        delete m_pCharClass;
}

SwCalc::~SwCalc()
{
    suppress_fun_call_w_exception(ImplDestroy());
}

SwSbxValue SwCalc::Calculate( const OUString& rStr )
{
    m_eError = SwCalcError::NONE;
    SwSbxValue nResult;

    if( rStr.isEmpty() )
        return nResult;

    m_nListPor = 0;
    m_eCurrListOper = CALC_PLUS; // default: sum

    m_sCommand = rStr;
    m_nCommandPos = 0;

    for (;;)
    {
        m_eCurrOper = GetToken();
        if (m_eCurrOper == CALC_ENDCALC || m_eError != SwCalcError::NONE )
            break;
        nResult = Expr();
    }

    if( m_eError != SwCalcError::NONE)
        nResult.PutDouble( DBL_MAX );

    return nResult;
}

OUString SwCalc::GetStrResult( const SwSbxValue& rVal )
{
    if( !rVal.IsDouble() )
    {
        return rVal.GetOUString();
    }
    return GetStrResult( rVal.GetDouble() );
}

OUString SwCalc::GetStrResult( double nValue )
{
    if( nValue >= DBL_MAX )
        switch( m_eError )
        {
        case SwCalcError::Syntax          :   return SwViewShell::GetShellRes()->aCalc_Syntax;
        case SwCalcError::DivByZero       :   return SwViewShell::GetShellRes()->aCalc_ZeroDiv;
        case SwCalcError::FaultyBrackets  :   return SwViewShell::GetShellRes()->aCalc_Brack;
        case SwCalcError::OverflowInPower :   return SwViewShell::GetShellRes()->aCalc_Pow;
        case SwCalcError::Overflow        :   return SwViewShell::GetShellRes()->aCalc_Overflow;
        default                           :   return SwViewShell::GetShellRes()->aCalc_Default;
        }

    const sal_Int32 nDecPlaces = 15;
    OUString aRetStr( ::rtl::math::doubleToUString(
                        nValue,
                        rtl_math_StringFormat_Automatic,
                        nDecPlaces,
                        m_xLocaleDataWrapper->getNumDecimalSep()[0],
                        true ));
    return aRetStr;
}

SwCalcExp* SwCalc::VarInsert( const OUString &rStr )
{
    OUString aStr = m_pCharClass->lowercase( rStr );
    return VarLook( aStr, true );
}

SwCalcExp* SwCalc::VarLook( const OUString& rStr, bool bIns )
{
    m_aErrExpr.nValue.SetVoidValue(false);

    OUString aStr = m_pCharClass->lowercase( rStr );
    SwCalcExp* pFnd = nullptr;
    auto it = m_aVarTable.find(aStr);
    if (it != m_aVarTable.end())
        pFnd = &it->second;

    if( !pFnd )
    {
        // then check doc
        std::unordered_multimap<OUString, const SwFieldType*> & rDocTable
            = m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().GetFieldTypeTable();
        auto docIt = rDocTable.find(aStr);
        if (docIt != rDocTable.end())
        {
            const SwFieldType* pFieldType = docIt->second;
            it = m_aVarTable.insert( { aStr, SwCalcExp( SwSbxValue(), pFieldType ) } ).first;
            pFnd = &it->second;
        }
    }

    if( pFnd )
    {
        if( pFnd->pFieldType && pFnd->pFieldType->Which() == SwFieldIds::User )
        {
            SwUserFieldType* pUField = const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFnd->pFieldType));
            if( nsSwGetSetExpType::GSE_STRING & pUField->GetType() )
            {
                pFnd->nValue.PutString( pUField->GetContent() );
            }
            else if( !pUField->IsValid() )
            {
                // Save the current values...
                sal_uInt16 nListPor = m_nListPor;
                bool bHasNumber = m_bHasNumber;
                SwSbxValue nLastLeft = m_nLastLeft;
                SwSbxValue nNumberValue = m_nNumberValue;
                sal_Int32 nCommandPos = m_nCommandPos;
                SwCalcOper eCurrOper = m_eCurrOper;
                SwCalcOper eCurrListOper = m_eCurrListOper;
                OUString sCurrCommand = m_sCommand;

                pFnd->nValue.PutDouble( pUField->GetValue( *this ) );

                // ...and write them back.
                m_nListPor = nListPor;
                m_bHasNumber = bHasNumber;
                m_nLastLeft = std::move(nLastLeft);
                m_nNumberValue = std::move(nNumberValue);
                m_nCommandPos = nCommandPos;
                m_eCurrOper = eCurrOper;
                m_eCurrListOper = eCurrListOper;
                m_sCommand = std::move(sCurrCommand);
            }
            else
            {
                pFnd->nValue.PutDouble( pUField->GetValue() );
            }
        }
        else if ( !pFnd->pFieldType && pFnd->nValue.IsDBvalue() )
        {
            if ( pFnd->nValue.IsString() )
                m_aErrExpr.nValue.PutString( pFnd->nValue.GetOUString() );
            else if ( pFnd->nValue.IsDouble() )
                m_aErrExpr.nValue.PutDouble( pFnd->nValue.GetDouble() );
            pFnd = &m_aErrExpr;
        }
        return pFnd;
    }

    // At this point the "real" case variable has to be used
    OUString const sTmpName( ::ReplacePoint(rStr) );

    if( !bIns )
    {
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
        SwDBManager *pMgr = m_rDoc.GetDBManager();
        OUString sDBName(GetDBName( sTmpName ));
        OUString sSourceName(sDBName.getToken(0, DB_DELIM));
        OUString sTableName(sDBName.getToken(0, ';').getToken(1, DB_DELIM));
        if( pMgr && !sSourceName.isEmpty() && !sTableName.isEmpty() &&
            pMgr->OpenDataSource(sSourceName, sTableName))
        {
            OUString sColumnName( GetColumnName( sTmpName ));
            OSL_ENSURE(!sColumnName.isEmpty(), "Missing DB column name");

            OUString sDBNum( SwFieldType::GetTypeStr(SwFieldTypesEnum::DatabaseSetNumber) );
            sDBNum = m_pCharClass->lowercase(sDBNum);

            // Initialize again because this doesn't happen in docfld anymore for
            // elements != SwFieldIds::Database. E.g. if there is an expression field before
            // a DB_Field in a document.
            const sal_uInt32 nTmpRec = pMgr->GetSelectedRecordId(sSourceName, sTableName);
            VarChange(sDBNum, nTmpRec);

            if( sDBNum.equalsIgnoreAsciiCase(sColumnName) )
            {
                m_aErrExpr.nValue.PutULong(nTmpRec);
                return &m_aErrExpr;
            }

            OUString sResult;
            double nNumber = DBL_MAX;

            LanguageType nLang = m_xLocaleDataWrapper->getLanguageTag().getLanguageType();
            if(pMgr->GetColumnCnt( sSourceName, sTableName, sColumnName,
                                    nTmpRec, nLang, sResult, &nNumber ))
            {
                if (nNumber != DBL_MAX)
                    m_aErrExpr.nValue.PutDouble( nNumber );
                else
                    m_aErrExpr.nValue.PutString( sResult );

                return &m_aErrExpr;
            }
        }
        else
#endif
        {
            //data source was not available - set return to "NoValue"
            m_aErrExpr.nValue.SetVoidValue(true);
        }
        // NEVER save!
        return &m_aErrExpr;
    }

    SwCalcExp* pNewExp = &m_aVarTable.insert( { aStr, SwCalcExp( SwSbxValue(), nullptr ) } ).first->second;

    OUString sColumnName( GetColumnName( sTmpName ));
    OSL_ENSURE( !sColumnName.isEmpty(), "Missing DB column name" );
    if( sColumnName.equalsIgnoreAsciiCase(
                            SwFieldType::GetTypeStr( SwFieldTypesEnum::DatabaseSetNumber ) ))
    {
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
        SwDBManager *pMgr = m_rDoc.GetDBManager();
        OUString sDBName(GetDBName( sTmpName ));
        OUString sSourceName(sDBName.getToken(0, DB_DELIM));
        OUString sTableName(sDBName.getToken(0, ';').getToken(1, DB_DELIM));
        if( pMgr && !sSourceName.isEmpty() && !sTableName.isEmpty() &&
            pMgr->OpenDataSource(sSourceName, sTableName) &&
            !pMgr->IsInMerge())
        {
            pNewExp->nValue.PutULong( pMgr->GetSelectedRecordId(sSourceName, sTableName));
        }
        else
#endif
        {
            pNewExp->nValue.SetVoidValue(true);
        }
    }

    return pNewExp;
}

void SwCalc::VarChange( const OUString& rStr, double nValue )
{
    SwSbxValue aVal( nValue );
    VarChange( rStr, aVal );
}

void SwCalc::VarChange( const OUString& rStr, const SwSbxValue& rValue )
{
    OUString aStr = m_pCharClass->lowercase( rStr );

    auto it = m_aVarTable.find( aStr );
    if (it != m_aVarTable.end())
        it->second.nValue = rValue;
    else
        m_aVarTable.insert( { aStr, SwCalcExp( rValue, nullptr ) } );
}

bool SwCalc::Push( const SwUserFieldType* pUserFieldType )
{
    if( m_aRekurStack.end() != std::find(m_aRekurStack.begin(), m_aRekurStack.end(), pUserFieldType ) )
        return false;

    m_aRekurStack.push_back( pUserFieldType );
    return true;
}

void SwCalc::Pop()
{
    OSL_ENSURE( m_aRekurStack.size(), "SwCalc: Pop on an invalid pointer" );

    m_aRekurStack.pop_back();
}

const CharClass* SwCalc::GetCharClass() const
{
    return m_pCharClass;
}

void SwCalc::SetCharClass(const LanguageTag& rLanguageTag)
{
    m_pCharClass = new CharClass( ::comphelper::getProcessComponentContext(), rLanguageTag );
}

SwCalcOper SwCalc::GetToken()
{
    if( m_nCommandPos >= m_sCommand.getLength() )
    {
        m_eCurrOper = CALC_ENDCALC;
        return m_eCurrOper;
    }

    using namespace ::com::sun::star::i18n;
    {
        // Parse any token.
        ParseResult aRes = m_pCharClass->parseAnyToken( m_sCommand, m_nCommandPos,
                                                      coStartFlags, OUString(),
                                                      coContFlags, OUString());

        bool bSetError = true;
        sal_Int32 nRealStt = m_nCommandPos + aRes.LeadingWhiteSpace;
        if( aRes.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER) )
        {
            m_nNumberValue.PutDouble( aRes.Value );
            m_eCurrOper = CALC_NUMBER;
            bSetError = false;
        }
        else if( aRes.TokenType & KParseType::IDENTNAME )
        {
            OUString aName( m_sCommand.copy( nRealStt,
                            aRes.EndPos - nRealStt ) );
            //#101436#: The variable may contain a database name. It must not be
            // converted to lower case! Instead all further comparisons must be
            // done case-insensitive
            OUString sLowerCaseName = m_pCharClass->lowercase( aName );
            // catch currency symbol
            if( sLowerCaseName == m_sCurrSym )
            {
                m_nCommandPos = aRes.EndPos;
                return GetToken(); // call again
            }

            // catch operators
            CalcOp* pFnd = ::FindOperator( sLowerCaseName );
            if( pFnd )
            {
                m_eCurrOper = pFnd->eOp;
                switch( m_eCurrOper )
                {
                case CALC_SUM:
                case CALC_MEAN:
                case CALC_AVERAGE:
                case CALC_COUNT:
                    m_eCurrListOper = CALC_PLUS;
                    break;
                case CALC_MIN:
                    m_eCurrListOper = CALC_MIN_IN;
                    break;
                case CALC_MAX:
                    m_eCurrListOper = CALC_MAX_IN;
                    break;
                case CALC_DATE:
                    m_eCurrListOper = CALC_MONTH;
                    break;
                case CALC_PRODUCT:
                    m_eCurrListOper = CALC_MUL;
                    break;
                default:
                    break;
                }
                m_nCommandPos = aRes.EndPos;
                return m_eCurrOper;
            }
            m_aVarName = aName;
            m_eCurrOper = CALC_NAME;
            bSetError = false;
        }
        else if ( aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING )
        {
            m_nNumberValue.PutString( aRes.DequotedNameOrString );
            m_eCurrOper = CALC_NUMBER;
            bSetError = false;
        }
        else if( aRes.TokenType & KParseType::ONE_SINGLE_CHAR )
        {
            std::u16string_view aName( m_sCommand.subView( nRealStt,
                              aRes.EndPos - nRealStt ));
            if( 1 == aName.size() )
            {
                bSetError = false;
                sal_Unicode ch = aName[0];
                switch( ch )
                {
                case ';':
                    if( CALC_MONTH == m_eCurrListOper || CALC_DAY == m_eCurrListOper )
                    {
                        m_eCurrOper = m_eCurrListOper;
                        break;
                    }
                    [[fallthrough]];
                case '\n':
                    m_eCurrOper = CALC_PRINT;
                    break;

                case '%':
                case '^':
                case '*':
                case '/':
                case '+':
                case '-':
                case '(':
                case ')':
                    m_eCurrOper = SwCalcOper(ch);
                    break;

                case '=':
                case '!':
                    {
                        SwCalcOper eTmp2;
                        if( '=' == ch )
                        {
                            m_eCurrOper = SwCalcOper('=');
                            eTmp2 = CALC_EQ;
                        }
                        else
                        {
                            m_eCurrOper = CALC_NOT;
                            eTmp2 = CALC_NEQ;
                        }

                        if( aRes.EndPos < m_sCommand.getLength() &&
                            '=' == m_sCommand[aRes.EndPos] )
                        {
                            m_eCurrOper = eTmp2;
                            ++aRes.EndPos;
                        }
                    }
                    break;

                case cListDelim:
                    m_eCurrOper = m_eCurrListOper;
                    break;

                case '[':
                    if( aRes.EndPos < m_sCommand.getLength() )
                    {
                        m_aVarName.setLength(0);
                        sal_Int32 nFndPos = aRes.EndPos,
                                  nSttPos = nFndPos;

                        do {
                            nFndPos = m_sCommand.indexOf( ']', nFndPos );
                            if( -1 != nFndPos )
                            {
                                // ignore the ]
                                if ('\\' == m_sCommand[nFndPos-1])
                                {
                                    m_aVarName.append(m_sCommand.subView(nSttPos,
                                                    nFndPos - nSttPos - 1) );
                                    nSttPos = ++nFndPos;
                                }
                                else
                                    break;
                            }
                        } while( nFndPos != -1 );

                        if( nFndPos != -1 )
                        {
                            if( nSttPos != nFndPos )
                                m_aVarName.append(m_sCommand.subView(nSttPos,
                                                    nFndPos - nSttPos) );
                            aRes.EndPos = nFndPos + 1;
                            m_eCurrOper = CALC_NAME;
                        }
                        else
                            bSetError = true;
                    }
                    else
                    {
                        bSetError = true;
                    }
                    break;

                default:
                    bSetError = true;
                    break;
                }
            }
        }
        else if( aRes.TokenType & KParseType::BOOLEAN )
        {
            std::u16string_view aName( m_sCommand.subView( nRealStt,
                                         aRes.EndPos - nRealStt ));
            if( !aName.empty() )
            {
                sal_Unicode ch = aName[0];

                bSetError = true;
                if ('<' == ch || '>' == ch)
                {
                    bSetError = false;

                    SwCalcOper eTmp2 = ('<' == ch) ? CALC_LEQ : CALC_GEQ;
                    m_eCurrOper = ('<' == ch) ? CALC_LES : CALC_GRE;

                    if( 2 == aName.size() && '=' == aName[1] )
                        m_eCurrOper = eTmp2;
                    else if( 1 != aName.size() )
                        bSetError = true;
                }
            }
        }
        else if( nRealStt == m_sCommand.getLength() )
        {
            m_eCurrOper = CALC_ENDCALC;
            bSetError = false;
        }

        if( bSetError )
        {
            m_eError = SwCalcError::Syntax;
            m_eCurrOper = CALC_PRINT;
        }
        m_nCommandPos = aRes.EndPos;
    };

    return m_eCurrOper;
}

SwSbxValue SwCalc::Term()
{
    SwSbxValue left( Prim() );
    m_nLastLeft = left;
    for(;;)
    {
        sal_uInt16 nSbxOper = USHRT_MAX;

        switch( m_eCurrOper )
        {
        case CALC_AND:
            {
                GetToken();
                bool bB = Prim().GetBool();
                left.PutBool( left.GetBool() && bB );
            }
            break;
        case CALC_OR:
            {
                GetToken();
                bool bB = Prim().GetBool();
                left.PutBool( left.GetBool() || bB );
            }
            break;
        case CALC_XOR:
            {
                GetToken();
                bool bR = Prim().GetBool();
                bool bL = left.GetBool();
                left.PutBool(bL != bR);
            }
            break;

        case CALC_EQ:   nSbxOper = SbxEQ;   break;
        case CALC_NEQ:  nSbxOper = SbxNE;   break;
        case CALC_LEQ:  nSbxOper = SbxLE;   break;
        case CALC_GEQ:  nSbxOper = SbxGE;   break;
        case CALC_GRE:  nSbxOper = SbxGT;   break;
        case CALC_LES:  nSbxOper = SbxLT;   break;

        case CALC_MUL:  nSbxOper = SbxMUL;  break;
        case CALC_DIV:  nSbxOper = SbxDIV;  break;

        case CALC_MIN_IN:
            {
                GetToken();
                SwSbxValue e = Prim();
                left = left.GetDouble() < e.GetDouble() ? left : e;
            }
            break;
        case CALC_MAX_IN:
            {
                GetToken();
                SwSbxValue e = Prim();
                left = left.GetDouble() > e.GetDouble() ? left : e;
            }
            break;
        case CALC_MONTH:
            {
                GetToken();
                SwSbxValue e = Prim();
                sal_Int32 nYear = static_cast<sal_Int32>(floor( left.GetDouble() ));
                nYear = nYear & 0x0000FFFF;
                sal_Int32 nMonth = static_cast<sal_Int32>(floor( e.GetDouble() ));
                nMonth = ( nMonth & 0x000000FF ) << 16;
                left.PutLong( nMonth + nYear );
                m_eCurrOper = CALC_DAY;
            }
            break;
        case CALC_DAY:
            {
                GetToken();
                SwSbxValue e = Prim();
                sal_Int32 nYearMonth = static_cast<sal_Int32>(floor( left.GetDouble() ));
                nYearMonth = nYearMonth & 0x00FFFFFF;
                sal_Int32 nDay = static_cast<sal_Int32>(floor( e.GetDouble() ));
                nDay = ( nDay & 0x000000FF ) << 24;
                left = lcl_ConvertToDateValue( m_rDoc, nDay + nYearMonth );
            }
            break;
        case CALC_ROUND:
            {
                GetToken();
                SwSbxValue e = Prim();

                double fVal = 0;
                double fFac = 1;
                sal_Int32 nDec = static_cast<sal_Int32>(floor( e.GetDouble() ));
                if( nDec < -20 || nDec > 20 )
                {
                    m_eError = SwCalcError::Overflow;
                    left.Clear();
                    return left;
                }
                fVal = left.GetDouble();
                if( nDec >= 0)
                {
                    for (sal_Int32 i = 0; i < nDec; ++i )
                        fFac *= 10.0;
                }
                else
                {
                    for (sal_Int32 i = 0; i < -nDec; ++i )
                        fFac /= 10.0;
                }

                fVal *= fFac;
                bool bSign;
                if (fVal < 0.0)
                {
                    fVal *= -1.0;
                    bSign = true;
                }
                else
                {
                    bSign = false;
                }

                // rounding
                double fNum = fVal; // find the exponent
                int nExp = 0;
                if( fNum > 0 )
                {
                    while( fNum < 1.0 )
                    {
                        fNum *= 10.0;
                        --nExp;
                    }
                    while( fNum >= 10.0 )
                    {
                        fNum /= 10.0;
                        ++nExp;
                    }
                }
                nExp = 15 - nExp;
                if( nExp > 15 )
                    nExp = 15;
                else if( nExp <= 1 )
                    nExp = 0;
                fVal = floor( fVal+ 0.5 + nRoundVal[ nExp ] );

                if (bSign)
                    fVal *= -1.0;

                fVal /= fFac;

                left.PutDouble( fVal );
            }
            break;

//#77448# (=2*3^2 != 18)

        default:
            return left;
        }

        if( USHRT_MAX != nSbxOper )
        {
            // #i47706: cast to SbxOperator AFTER comparing to USHRT_MAX
            SbxOperator eSbxOper = static_cast<SbxOperator>(nSbxOper);

            GetToken();
            if( SbxEQ <= eSbxOper && eSbxOper <= SbxGE )
            {
                left.PutBool( left.Compare( eSbxOper, Prim() ));
            }
            else
            {
                SwSbxValue aRight( Prim() );
                aRight.MakeDouble();
                left.MakeDouble();

                if( SbxDIV == eSbxOper && !aRight.GetDouble() )
                    m_eError = SwCalcError::DivByZero;
                else
                    left.Compute( eSbxOper, aRight );
            }
        }
    }
}

SwSbxValue SwCalc::StdFunc(pfCalc pFnc, bool bChkTrig)
{
    SwSbxValue nErg;
    GetToken();
    double nVal = Prim().GetDouble();
    if( !bChkTrig || ( nVal > -1 && nVal < 1 ) )
        nErg.PutDouble( (*pFnc)( nVal ) );
    else
        m_eError = SwCalcError::Overflow;
    return nErg;
}

SwSbxValue SwCalc::PrimFunc(bool &rChkPow)
{
    rChkPow = false;

    switch (m_eCurrOper)
    {
        case CALC_SIN:
            SAL_INFO("sw.calc", "sin");
            return StdFunc(&sin, false);
        case CALC_COS:
            SAL_INFO("sw.calc", "cos");
            return StdFunc(&cos, false);
        case CALC_TAN:
            SAL_INFO("sw.calc", "tan");
            return StdFunc(&tan, false);
        case CALC_ATAN:
            SAL_INFO("sw.calc", "atan");
            return StdFunc(&atan, false);
        case CALC_ASIN:
            SAL_INFO("sw.calc", "asin");
            return StdFunc(&asin, true);
        case CALC_ACOS:
            SAL_INFO("sw.calc", "acos");
            return StdFunc(&acos, true);
        case CALC_ABS:
            SAL_INFO("sw.calc", "abs");
            return StdFunc(&abs, false);
        case CALC_SIGN:
        {
            SAL_INFO("sw.calc", "sign");
            SwSbxValue nErg;
            GetToken();
            double nVal = Prim().GetDouble();
            nErg.PutDouble( int(0 < nVal) - int(nVal < 0) );
            return nErg;
        }
        case CALC_INT:
        {
            SAL_INFO("sw.calc", "int");
            SwSbxValue nErg;
            GetToken();
            sal_Int32 nVal = static_cast<sal_Int32>( Prim().GetDouble() );
            nErg.PutDouble( nVal );
            return nErg;
        }
        case CALC_NOT:
        {
            SAL_INFO("sw.calc", "not");
            GetToken();
            SwSbxValue nErg = Prim();
            if( SbxSTRING == nErg.GetType() )
            {
                nErg.PutBool( nErg.GetOUString().isEmpty() );
            }
            else if(SbxBOOL == nErg.GetType() )
            {
                nErg.PutBool(!nErg.GetBool());
            }
            // Evaluate arguments manually so that the binary NOT below does not
            // get called. We want a BOOLEAN NOT.
            else if (nErg.IsNumeric())
            {
                nErg.PutLong( nErg.GetDouble() == 0.0 ? 1 : 0 );
            }
            else
            {
                OSL_FAIL( "unexpected case. computing binary NOT" );
                //!! computes a binary NOT
                nErg.Compute( SbxNOT, nErg );
            }
            return nErg;
        }
        case CALC_NUMBER:
        {
            SAL_INFO("sw.calc", "number: " << m_nNumberValue.GetDouble());
            SwSbxValue nErg;
            m_bHasNumber = true;
            if( GetToken() == CALC_PHD )
            {
                double aTmp = m_nNumberValue.GetDouble();
                aTmp *= 0.01;
                nErg.PutDouble( aTmp );
                GetToken();
            }
            else if( m_eCurrOper == CALC_NAME )
            {
                m_eError = SwCalcError::Syntax;
            }
            else
            {
                nErg = m_nNumberValue;
                rChkPow = true;
            }
            return nErg;
        }
        case CALC_NAME:
        {
            SAL_INFO("sw.calc", "name");
            SwSbxValue nErg;
            switch(SwCalcOper eOper = GetToken())
            {
                case CALC_ASSIGN:
                {
                    SwCalcExp* n = VarInsert(m_aVarName.toString());
                    GetToken();
                    nErg = n->nValue = Expr();
                    break;
                }
                default:
                    nErg = VarLook(m_aVarName.toString())->nValue;
                    // Explicitly disallow unknown function names (followed by "("),
                    // allow unknown variable names (equal to zero)
                    if (nErg.IsVoidValue() && (eOper == CALC_LP))
                        m_eError = SwCalcError::Syntax;
                    else
                        rChkPow = true;
                    break;
            }
            return nErg;
        }
        case CALC_MINUS:
        {
            SAL_INFO("sw.calc", "-");
            SwSbxValue nErg;
            GetToken();
            nErg.PutDouble( -(Prim().GetDouble()) );
            return nErg;
        }
        case CALC_LP:
        {
            SAL_INFO("sw.calc", "(");
            GetToken();
            SwSbxValue nErg = Expr();
            if( m_eCurrOper != CALC_RP )
            {
                m_eError = SwCalcError::FaultyBrackets;
            }
            else
            {
                GetToken();
                rChkPow = true; // in order for =(7)^2 to work
            }
            return nErg;
        }
        case CALC_RP:
            // ignore, see tdf#121962
            SAL_INFO("sw.calc", ")");
            break;
        case CALC_MEAN:
        case CALC_AVERAGE:
        {
            SAL_INFO("sw.calc", "mean");
            m_nListPor = 1;
            m_bHasNumber = CALC_MEAN == m_eCurrOper;
            GetToken();
            SwSbxValue nErg = Expr();
            double aTmp = nErg.GetDouble();
            aTmp /= m_nListPor;
            if ( !m_bHasNumber )
                m_eError = SwCalcError::DivByZero;
            else
                nErg.PutDouble( aTmp );
            return nErg;
        }
        case CALC_COUNT:
        {
            SAL_INFO("sw.calc", "count");
            m_nListPor = 1;
            m_bHasNumber = false;
            GetToken();
            SwSbxValue nErg = Expr();
            nErg.PutDouble( m_bHasNumber ? m_nListPor : 0 );
            return nErg;
        }
        case CALC_SQRT:
        {
            SAL_INFO("sw.calc", "sqrt");
            GetToken();
            SwSbxValue nErg = Prim();
            if( nErg.GetDouble() < 0 )
                m_eError = SwCalcError::Overflow;
            else
                nErg.PutDouble( sqrt( nErg.GetDouble() ));
            return nErg;
        }
        case CALC_SUM:
        case CALC_PRODUCT:
        case CALC_DATE:
        case CALC_MIN:
        case CALC_MAX:
        {
            SAL_INFO("sw.calc", "sum/product/date/min/max");
            GetToken();
            SwSbxValue nErg = Expr();
            return nErg;
        }
        case CALC_ENDCALC:
        {
            SAL_INFO("sw.calc", "endcalc");
            SwSbxValue nErg;
            nErg.Clear();
            return nErg;
        }
        default:
            SAL_INFO("sw.calc", "syntax error");
            m_eError = SwCalcError::Syntax;
            break;
    }

    return SwSbxValue();
}

SwSbxValue SwCalc::Prim()
{
    bool bChkPow;
    SwSbxValue nErg = PrimFunc(bChkPow);

    if (bChkPow && m_eCurrOper == CALC_POW)
    {
        double dleft = nErg.GetDouble();
        GetToken();
        double right = Prim().GetDouble();

        double fraction;
        fraction = modf( right, &o3tl::temporary(double()) );
        if( ( dleft < 0.0 && 0.0 != fraction ) ||
            ( 0.0 == dleft && right < 0.0 ) )
        {
            m_eError = SwCalcError::Overflow;
            nErg.Clear();
        }
        else
        {
            dleft = pow(dleft, right );
            if( dleft == HUGE_VAL )
            {
                m_eError = SwCalcError::OverflowInPower;
                nErg.Clear();
            }
            else
            {
                nErg.PutDouble( dleft );
            }
        }
    }

    return nErg;
}

SwSbxValue  SwCalc::Expr()
{
    SwSbxValue left = Term();
    m_nLastLeft = left;
    for(;;)
    {
        switch(m_eCurrOper)
        {
            case CALC_PLUS:
            {
                GetToken();
                left.MakeDouble();
                SwSbxValue right(Term());
                right.MakeDouble();
                left.Compute(SbxPLUS, right);
                m_nListPor++;
                break;
            }
            case CALC_MINUS:
            {
                GetToken();
                left.MakeDouble();
                SwSbxValue right(Term());
                right.MakeDouble();
                left.Compute(SbxMINUS, right);
                break;
            }
            default:
            {
                return left;
            }
        }
    }
}

OUString SwCalc::GetColumnName(const OUString& rName)
{
    sal_Int32 nPos = rName.indexOf(DB_DELIM);
    if( -1 != nPos )
    {
        nPos = rName.indexOf(DB_DELIM, nPos + 1);

        if( -1 != nPos )
            return rName.copy(nPos + 1);
    }
    return rName;
}

OUString SwCalc::GetDBName(std::u16string_view rName)
{
    size_t nPos = rName.find(DB_DELIM);
    if( std::u16string_view::npos != nPos )
    {
        nPos = rName.find(DB_DELIM, nPos + 1);

        if( std::u16string_view::npos != nPos )
            return OUString(rName.substr( 0, nPos ));
    }
    SwDBData aData = m_rDoc.GetDBData();
    return aData.sDataSource + OUStringChar(DB_DELIM) + aData.sCommand;
}

namespace
{
    bool lcl_Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                                double& rVal,
                                const LocaleDataWrapper* const pLclData )
    {
        assert(pLclData);
        const sal_Unicode nCurrCmdPos = rCommandPos;
        rtl_math_ConversionStatus eStatus;
        const sal_Unicode* pEnd;
        rVal = pLclData->stringToDouble( rCommand.getStr() + rCommandPos,
                                         rCommand.getStr() + rCommand.getLength(),
                                         true,
                                         &eStatus,
                                         &pEnd );
        rCommandPos = static_cast<sal_Int32>(pEnd - rCommand.getStr());

        return rtl_math_ConversionStatus_Ok == eStatus &&
               nCurrCmdPos != rCommandPos;
    }
}

bool SwCalc::Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                         double& rVal )
{
    const SvtSysLocale aSysLocale;
    return lcl_Str2Double( rCommand, rCommandPos, rVal, &aSysLocale.GetLocaleData() );
}

bool SwCalc::Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                         double& rVal, SwDoc const * const pDoc )
{
    const SvtSysLocale aSysLocale;
    std::unique_ptr<const LocaleDataWrapper> pLclD;
    if( pDoc )
    {
        LanguageType eLang = GetDocAppScriptLang( *pDoc );
        if (eLang != aSysLocale.GetLanguageTag().getLanguageType())
        {
            pLclD.reset( new LocaleDataWrapper( LanguageTag( eLang )) );
        }
    }

    bool const bRet = lcl_Str2Double(rCommand, rCommandPos, rVal,
                                     pLclD ? pLclD.get() : &aSysLocale.GetLocaleData());

    return bRet;
}

bool SwCalc::IsValidVarName( const OUString& rStr, OUString* pValidName )
{
    bool bRet = false;
    using namespace ::com::sun::star::i18n;
    {
        // Parse any token.
        ParseResult aRes = GetAppCharClass().parseAnyToken( rStr, 0,
                                                    coStartFlags, OUString(),
                                                     coContFlags, OUString() );

        if( aRes.TokenType & KParseType::IDENTNAME )
        {
            bRet = aRes.EndPos == rStr.getLength();
            if( pValidName )
            {
                *pValidName = rStr.copy( aRes.LeadingWhiteSpace,
                                         aRes.EndPos - aRes.LeadingWhiteSpace );
            }
        }
        else if( pValidName )
            pValidName->clear();
    }
    return bRet;
}

SwCalcExp::SwCalcExp(SwSbxValue aVal, const SwFieldType* pType)
    : nValue(std::move(aVal))
    , pFieldType(pType)
{
}

bool SwSbxValue::GetBool() const
{
    return SbxSTRING == GetType() ? !GetOUString().isEmpty()
                                  : SbxValue::GetBool();
}

double SwSbxValue::GetDouble() const
{
    double nRet;
    if( SbxSTRING == GetType() )
    {
        sal_Int32 nStt = 0;
        SwCalc::Str2Double( GetOUString(), nStt, nRet );
    }
    else if (IsBool())
    {
        nRet = GetBool() ? 1.0 : 0.0;
    }
    else
    {
        nRet = SbxValue::GetDouble();
    }
    return nRet;
}

SwSbxValue& SwSbxValue::MakeDouble()
{
    if( GetType() == SbxSTRING  || GetType() == SbxBOOL )
        PutDouble( GetDouble() );
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
