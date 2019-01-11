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

#include <calc.hxx>
#include <cfloat>
#include <climits>
#include <memory>
#include <string_view>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <cstdlib>
#include <dbfld.hxx>
#include <dbmgr.hxx>
#include <docfld.hxx>
#include <docstat.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <expfld.hxx>
#include <hintids.hxx>
#include <o3tl/temporary.hxx>
#include <osl/diagnose.hxx>
#include <rtl/math.hxx>
#include <shellres.hxx>
#include <svl/zforlist.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/useroptions.hxx>
#include <usrfld.hxx>
#include <viewsh.hxx>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>

using namespace ::com::sun::star;

const sal_Char sCalc_Add[]  =   "add";
const sal_Char sCalc_Sub[]  =   "sub";
const sal_Char sCalc_Mul[]  =   "mul";
const sal_Char sCalc_Div[]  =   "div";
const sal_Char sCalc_Phd[]  =   "phd";
const sal_Char sCalc_Sqrt[] =   "sqrt";
const sal_Char sCalc_Pow[]  =   "pow";
const sal_Char sCalc_Or[]   =   "or";
const sal_Char sCalc_Xor[]  =   "xor";
const sal_Char sCalc_And[]  =   "and";
const sal_Char sCalc_Not[]  =   "not";
const sal_Char sCalc_Eq[]   =   "eq";
const sal_Char sCalc_Neq[]  =   "neq";
const sal_Char sCalc_Leq[]  =   "leq";
const sal_Char sCalc_Geq[]  =   "geq";
const sal_Char sCalc_L[]    =   "l";
const sal_Char sCalc_G[]    =   "g";
const sal_Char sCalc_Sum[]  =   "sum";
const sal_Char sCalc_Mean[] =   "mean";
const sal_Char sCalc_Min[]  =   "min";
const sal_Char sCalc_Max[]  =   "max";
const sal_Char sCalc_Sin[]  =   "sin";
const sal_Char sCalc_Cos[]  =   "cos";
const sal_Char sCalc_Tan[]  =   "tan";
const sal_Char sCalc_Asin[] =   "asin";
const sal_Char sCalc_Acos[] =   "acos";
const sal_Char sCalc_Atan[] =   "atan";
const sal_Char sCalc_Round[]=   "round";
const sal_Char sCalc_Date[] =   "date";

// ATTENTION: sorted list of all operators
struct CalcOp
{
    union{
        const sal_Char* pName;
        const OUString* pUName;
    };
    SwCalcOper eOp;
};

CalcOp const aOpTable[] = {
/* ACOS */    {{sCalc_Acos},       CALC_ACOS},  // Arc cosine
/* ADD */     {{sCalc_Add},        CALC_PLUS},  // Addition
/* AND */     {{sCalc_And},        CALC_AND},   // log. AND
/* ASIN */    {{sCalc_Asin},       CALC_ASIN},  // Arc sine
/* ATAN */    {{sCalc_Atan},       CALC_ATAN},  // Arc tangent
/* COS */     {{sCalc_Cos},        CALC_COS},   // Cosine
/* DATE */    {{sCalc_Date},       CALC_DATE},  // Date
/* DIV */     {{sCalc_Div},        CALC_DIV},   // Division
/* EQ */      {{sCalc_Eq},         CALC_EQ},    // Equality
/* G */       {{sCalc_G},          CALC_GRE},   // Greater than
/* GEQ */     {{sCalc_Geq},        CALC_GEQ},   // Greater or equal
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
/* ROUND */   {{sCalc_Round},      CALC_ROUND}, // Rounding
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
    ( coStartFlags | i18n::KParseTokens::ASC_DOT )
        & ~i18n::KParseTokens::IGNORE_LEADING_WS;

extern "C" {
static int OperatorCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( CALC_NAME == static_cast<const CalcOp*>(pFirst)->eOp )
    {
        if( CALC_NAME == static_cast<const CalcOp*>(pSecond)->eOp )
            nRet = static_cast<const CalcOp*>(pFirst)->pUName->compareTo(
                   *static_cast<const CalcOp*>(pSecond)->pUName );
        else
            nRet = static_cast<const CalcOp*>(pFirst)->pUName->compareToAscii(
                   static_cast<const CalcOp*>(pSecond)->pName );
    }
    else
    {
        if( CALC_NAME == static_cast<const CalcOp*>(pSecond)->eOp )
            nRet = -1 * static_cast<const CalcOp*>(pSecond)->pUName->compareToAscii(
                        static_cast<const CalcOp*>(pFirst)->pName );
        else
            nRet = strcmp( static_cast<const CalcOp*>(pFirst)->pName,
                           static_cast<const CalcOp*>(pSecond)->pName );
    }
    return nRet;
}
}// extern "C"

CalcOp* FindOperator( const OUString& rSrch )
{
    CalcOp aSrch;
    aSrch.pUName = &rSrch;
    aSrch.eOp = CALC_NAME;

    return static_cast<CalcOp*>(bsearch( static_cast<void*>(&aSrch),
                              static_cast<void const *>(aOpTable),
                              SAL_N_ELEMENTS( aOpTable ),
                              sizeof( CalcOp ),
                              OperatorCompare ));
}

static LanguageType GetDocAppScriptLang( SwDoc const & rDoc )
{
    return static_cast<const SvxLanguageItem&>(rDoc.GetDefault(
               GetWhichOfScript( RES_CHRATR_LANGUAGE,
                                 SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ))
            )).GetLanguage();
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
    : m_aVarTable(TBLSZ)
    , m_aErrExpr( OUString(), SwSbxValue(), nullptr )
    , m_nCommandPos(0)
    , m_rDoc( rD )
    , m_pLocaleDataWrapper( m_aSysLocale.GetLocaleDataPtr() )
    , m_pCharClass( &GetAppCharClass() )
    , m_nListPor( 0 )
    , m_eCurrOper( CALC_NAME )
    , m_eCurrListOper( CALC_NAME )
    , m_eError( SwCalcError::NONE )
{
    m_aErrExpr.aStr = "~C_ERR~";
    LanguageType eLang = GetDocAppScriptLang( m_rDoc );

    if( eLang != m_pLocaleDataWrapper->getLanguageTag().getLanguageType() ||
        eLang != m_pCharClass->getLanguageTag().getLanguageType() )
    {
        LanguageTag aLanguageTag( eLang );
        m_pCharClass = new CharClass( ::comphelper::getProcessComponentContext(), aLanguageTag );
        m_pLocaleDataWrapper = new LocaleDataWrapper( aLanguageTag );
    }

    m_sCurrSym = comphelper::string::strip(m_pLocaleDataWrapper->getCurrSymbol(), ' ');
    m_sCurrSym  = m_pCharClass->lowercase( m_sCurrSym );

    static sal_Char const
        sNType0[] = "false",
        sNType1[] = "true",
        sNType2[] = "pi",
        sNType3[] = "e",
        sNType4[] = "tables",
        sNType5[] = "graf",
        sNType6[] = "ole",
        sNType7[] = "page",
        sNType8[] = "para",
        sNType9[] = "word",
        sNType10[]= "char",

        sNType11[] = "user_firstname" ,
        sNType12[] = "user_lastname" ,
        sNType13[] = "user_initials" ,
        sNType14[] = "user_company" ,
        sNType15[] = "user_street" ,
        sNType16[] = "user_country" ,
        sNType17[] = "user_zipcode" ,
        sNType18[] = "user_city" ,
        sNType19[] = "user_title" ,
        sNType20[] = "user_position" ,
        sNType21[] = "user_tel_work" ,
        sNType22[] = "user_tel_home" ,
        sNType23[] = "user_fax" ,
        sNType24[] = "user_email" ,
        sNType25[] = "user_state" ,
        sNType26[] = "graph"
        ;
    static const sal_Char* const sNTypeTab[ 27 ] =
    {
        sNType0, sNType1, sNType2, sNType3, sNType4, sNType5,
        sNType6, sNType7, sNType8, sNType9, sNType10, sNType11,
        sNType12, sNType13, sNType14, sNType15, sNType16, sNType17,
        sNType18, sNType19, sNType20, sNType21, sNType22, sNType23,
        sNType24,

        // those have two HashIds
        sNType25, sNType26
    };
    static sal_uInt16 const aHashValue[ 27 ] =
    {
        34, 38, 43,  7, 18, 32, 22, 29, 30, 33,  3,
        28, 24, 40,  9, 11, 26, 45,  4, 23, 36, 44, 19,  5,  1,
        // those have two HashIds
        11, 38
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

#if TBLSZ != 47
#error Did you adjust all hash values?
#endif

    const SwDocStat& rDocStat = m_rDoc.getIDocumentStatistics().GetDocStat();

    SwSbxValue nVal;
    OUString sTmpStr;
    sal_uInt16 n;

    for( n = 0; n < 25; ++n )
    {
        sTmpStr = OUString::createFromAscii(sNTypeTab[n]);
        m_aVarTable[ aHashValue[ n ] ].reset( new SwCalcExp( sTmpStr, nVal, nullptr ) );
    }

    m_aVarTable[ aHashValue[ 0 ] ]->nValue.PutBool( false );
    m_aVarTable[ aHashValue[ 1 ] ]->nValue.PutBool( true );
    m_aVarTable[ aHashValue[ 2 ] ]->nValue.PutDouble( F_PI );
    m_aVarTable[ aHashValue[ 3 ] ]->nValue.PutDouble( 2.7182818284590452354 );

    for( n = 0; n < 3; ++n )
        m_aVarTable[ aHashValue[ n + 4 ] ]->nValue.PutLong( rDocStat.*aDocStat1[ n ]  );
    for( n = 0; n < 4; ++n )
        m_aVarTable[ aHashValue[ n + 7 ] ]->nValue.PutLong( rDocStat.*aDocStat2[ n ]  );

    SvtUserOptions& rUserOptions = SW_MOD()->GetUserOptions();

    m_aVarTable[ aHashValue[ 11 ] ]->nValue.PutString( rUserOptions.GetFirstName() );
    m_aVarTable[ aHashValue[ 12 ] ]->nValue.PutString( rUserOptions.GetLastName() );
    m_aVarTable[ aHashValue[ 13 ] ]->nValue.PutString( rUserOptions.GetID() );

    for( n = 0; n < 11; ++n )
        m_aVarTable[ aHashValue[ n + 14 ] ]->nValue.PutString(
                                        rUserOptions.GetToken( aAdrToken[ n ] ));

    nVal.PutString( rUserOptions.GetToken( aAdrToken[ 11 ] ));
    sTmpStr = OUString::createFromAscii(sNTypeTab[25]);
    m_aVarTable[ aHashValue[ 25 ] ]->pNext.reset( new SwCalcExp( sTmpStr, nVal, nullptr ) );

} // SwCalc::SwCalc

SwCalc::~SwCalc() COVERITY_NOEXCEPT_FALSE
{
    if( m_pLocaleDataWrapper != m_aSysLocale.GetLocaleDataPtr() )
        delete m_pLocaleDataWrapper;
    if( m_pCharClass != &GetAppCharClass() )
        delete m_pCharClass;
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

    while( (m_eCurrOper = GetToken()) != CALC_ENDCALC && m_eError == SwCalcError::NONE )
        nResult = Expr();

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
                        m_pLocaleDataWrapper->getNumDecimalSep()[0],
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

    sal_uInt16 ii = 0;
    OUString aStr = m_pCharClass->lowercase( rStr );

    SwCalcExp* pFnd = m_aVarTable.Find(aStr, &ii);

    if( !pFnd )
    {
        // then check doc
        SwHashTable<SwCalcFieldType> const & rDocTable = m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().GetFieldTypeTable();
        for( SwHash* pEntry = rDocTable[ii].get(); pEntry; pEntry = pEntry->pNext.get() )
        {
            if( aStr == pEntry->aStr )
            {
                // then insert here
                pFnd = new SwCalcExp( aStr, SwSbxValue(),
                                    static_cast<SwCalcFieldType*>(pEntry)->pFieldType );
                pFnd->pNext = std::move( m_aVarTable[ii] );
                m_aVarTable[ii].reset(pFnd);
                break;
            }
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
                SwSbxValue nLastLeft = m_nLastLeft;
                SwSbxValue nNumberValue = m_nNumberValue;
                sal_Int32 nCommandPos = m_nCommandPos;
                SwCalcOper eCurrOper = m_eCurrOper;
                SwCalcOper eCurrListOper = m_eCurrListOper;
                OUString sCurrCommand = m_sCommand;

                pFnd->nValue.PutDouble( pUField->GetValue( *this ) );

                // ...and write them back.
                m_nListPor = nListPor;
                m_nLastLeft = nLastLeft;
                m_nNumberValue = nNumberValue;
                m_nCommandPos = nCommandPos;
                m_eCurrOper = eCurrOper;
                m_eCurrListOper = eCurrListOper;
                m_sCommand = sCurrCommand;
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
#if HAVE_FEATURE_DBCONNECTIVITY
        SwDBManager *pMgr = m_rDoc.GetDBManager();

        OUString sDBName(GetDBName( sTmpName ));
        OUString sSourceName(sDBName.getToken(0, DB_DELIM));
        OUString sTableName(sDBName.getToken(0, ';').getToken(1, DB_DELIM));
        if( pMgr && !sSourceName.isEmpty() && !sTableName.isEmpty() &&
            pMgr->OpenDataSource(sSourceName, sTableName))
        {
            OUString sColumnName( GetColumnName( sTmpName ));
            OSL_ENSURE(!sColumnName.isEmpty(), "Missing DB column name");

            OUString sDBNum( SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD) );
            sDBNum = m_pCharClass->lowercase(sDBNum);

            // Initialize again because this doesn't happen in docfld anymore for
            // elements != SwFieldIds::Database. E.g. if there is an expression field before
            // an DB_Field in a document.
            const sal_uInt32 nTmpRec = pMgr->GetSelectedRecordId(sSourceName, sTableName);
            VarChange(sDBNum, nTmpRec);

            if( sDBNum.equalsIgnoreAsciiCase(sColumnName) )
            {
                m_aErrExpr.nValue.PutULong(nTmpRec);
                return &m_aErrExpr;
            }

            OUString sResult;
            double nNumber = DBL_MAX;

            LanguageType nLang = m_pLocaleDataWrapper->getLanguageTag().getLanguageType();
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

    SwCalcExp* pNewExp = new SwCalcExp( aStr, SwSbxValue(), nullptr );
    pNewExp->pNext = std::move( m_aVarTable[ ii ] );
    m_aVarTable[ ii ].reset( pNewExp );

    OUString sColumnName( GetColumnName( sTmpName ));
    OSL_ENSURE( !sColumnName.isEmpty(), "Missing DB column name" );
    if( sColumnName.equalsIgnoreAsciiCase(
                            SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) ))
    {
#if HAVE_FEATURE_DBCONNECTIVITY
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

    sal_uInt16 nPos = 0;
    SwCalcExp* pFnd = m_aVarTable.Find( aStr, &nPos );

    if( !pFnd )
    {
        pFnd = new SwCalcExp( aStr, rValue, nullptr );
        pFnd->pNext = std::move( m_aVarTable[ nPos ] );
        m_aVarTable[ nPos ].reset( pFnd );
    }
    else
    {
        pFnd->nValue = rValue;
    }
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

CharClass* SwCalc::GetCharClass()
{
    return m_pCharClass;
}

SwCalcOper SwCalc::GetToken()
{
    if( m_nCommandPos >= m_sCommand.getLength() )
        return m_eCurrOper = CALC_ENDCALC;

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
            OUString aName( m_sCommand.copy( nRealStt,
                              aRes.EndPos - nRealStt ));
            if( 1 == aName.getLength() )
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
                            if( -1 != ( nFndPos =
                                m_sCommand.indexOf( ']', nFndPos )) )
                            {
                                // ignore the ]
                                if ('\\' == m_sCommand[nFndPos-1])
                                {
                                    m_aVarName.append(std::u16string_view(m_sCommand).substr(nSttPos,
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
                                m_aVarName.append(std::u16string_view(m_sCommand).substr(nSttPos,
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
            OUString aName( m_sCommand.copy( nRealStt,
                                         aRes.EndPos - nRealStt ));
            if( !aName.isEmpty() )
            {
                sal_Unicode ch = aName[0];

                bSetError = true;
                if ('<' == ch || '>' == ch)
                {
                    bSetError = false;

                    SwCalcOper eTmp2 = ('<' == ch) ? CALC_LEQ : CALC_GEQ;
                    m_eCurrOper = ('<' == ch) ? CALC_LES : CALC_GRE;

                    if( 2 == aName.getLength() && '=' == aName[1] )
                        m_eCurrOper = eTmp2;
                    else if( 1 != aName.getLength() )
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
            break;
        case CALC_COS:
            SAL_INFO("sw.calc", "cos");
            return StdFunc(&cos, false);
            break;
        case CALC_TAN:
            SAL_INFO("sw.calc", "tan");
            return StdFunc(&tan, false);
            break;
        case CALC_ATAN:
            SAL_INFO("sw.calc", "atan");
            return StdFunc(&atan, false);
            break;
        case CALC_ASIN:
            SAL_INFO("sw.calc", "asin");
            return StdFunc(&asin, true);
            break;
        case CALC_ACOS:
            SAL_INFO("sw.calc", "acos");
            return StdFunc(&acos, true);
            break;
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
            break;
        }
        case CALC_NUMBER:
        {
            SAL_INFO("sw.calc", "number: " << m_nNumberValue.GetDouble());
            SwSbxValue nErg;
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
            break;
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
            break;
        }
        case CALC_MINUS:
        {
            SAL_INFO("sw.calc", "-");
            SwSbxValue nErg;
            GetToken();
            nErg.PutDouble( -(Prim().GetDouble()) );
            return nErg;
            break;
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
            break;
        }
        case CALC_RP:
            // ignore, see tdf#121962
            SAL_INFO("sw.calc", ")");
            break;
        case CALC_MEAN:
        {
            SAL_INFO("sw.calc", "mean");
            m_nListPor = 1;
            GetToken();
            SwSbxValue nErg = Expr();
            double aTmp = nErg.GetDouble();
            aTmp /= m_nListPor;
            nErg.PutDouble( aTmp );
            return nErg;
            break;
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
            break;
        }
        case CALC_SUM:
        case CALC_DATE:
        case CALC_MIN:
        case CALC_MAX:
        {
            SAL_INFO("sw.calc", "sum/date/min/max");
            GetToken();
            SwSbxValue nErg = Expr();
            return nErg;
            break;
        }
        case CALC_ENDCALC:
        {
            SAL_INFO("sw.calc", "endcalc");
            SwSbxValue nErg;
            nErg.Clear();
            return nErg;
            break;
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

OUString SwCalc::GetDBName(const OUString& rName)
{
    sal_Int32 nPos = rName.indexOf(DB_DELIM);
    if( -1 != nPos )
    {
        nPos = rName.indexOf(DB_DELIM, nPos + 1);

        if( -1 != nPos )
            return rName.copy( 0, nPos );
    }
    SwDBData aData = m_rDoc.GetDBData();
    return aData.sDataSource + OUStringLiteral1(DB_DELIM) + aData.sCommand;
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
    return lcl_Str2Double( rCommand, rCommandPos, rVal, aSysLocale.GetLocaleDataPtr() );
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
                                     pLclD ? pLclD.get() : aSysLocale.GetLocaleDataPtr());

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

SwHash::SwHash(const OUString& rStr)
    : aStr(rStr)
{
}

SwHash::~SwHash()
{
}

SwCalcExp::SwCalcExp(const OUString& rStr, const SwSbxValue& rVal,
                      const SwFieldType* pType)
    : SwHash(rStr)
    , nValue(rVal)
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

#ifdef STANDALONE_HASHCALC

// this is example code how to create hash values in the CTOR:

#include <stdio.h>
void main()
{
    static sal_Char
        sNType0[] = "false",    sNType1[] = "true",     sNType2[] = "pi",
        sNType3[] = "e",        sNType4[] = "tables",   sNType5[] = "graf",
        sNType6[] = "ole",      sNType7[] = "page",     sNType8[] = "para",
        sNType9[] = "word",     sNType10[]= "char",
        sNType11[] = "user_company" ,       sNType12[] = "user_firstname" ,
        sNType13[] = "user_lastname" ,      sNType14[] = "user_initials",
        sNType15[] = "user_street" ,        sNType16[] = "user_country" ,
        sNType17[] = "user_zipcode" ,       sNType18[] = "user_city" ,
        sNType19[] = "user_title" ,         sNType20[] = "user_position" ,
        sNType21[] = "user_tel_home",       sNType22[] = "user_tel_work",
        sNType23[] = "user_fax" ,           sNType24[] = "user_email" ,
        sNType25[] = "user_state",          sNType26[] = "graph"
        ;

    static const sal_Char* sNTypeTab[ 27 ] =
    {
        sNType0, sNType1, sNType2, sNType3, sNType4, sNType5,
        sNType6, sNType7, sNType8, sNType9, sNType10, sNType11,
        sNType12, sNType13, sNType14, sNType15, sNType16, sNType17,
        sNType18, sNType19, sNType20, sNType21, sNType22, sNType23,
        sNType24, sNType25, sNType26
    };

    const unsigned short nTableSize = 47;
    int aArr[ nTableSize ] = { 0 };
    sal_Char ch;

    for( int n = 0; n < 27; ++n )
    {
        unsigned int ii = 0;
        const sal_Char* pp = sNTypeTab[ n ];

        while( *pp )
        {
            ii = ii << 1 ^ *pp++;
        }
        ii %= nTableSize;

        ch = aArr[ ii ] ? 'X' : ' ';
        aArr[ ii ] = 1;
        printf( "%-20s -> %3u [%c]\n", sNTypeTab[ n ], ii, ch );
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
