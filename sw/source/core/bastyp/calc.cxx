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
#include <cctype>
#include <cfloat>
#include <climits>
#include <memory>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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

using namespace ::com::sun::star;

// shortcut
#define RESOURCE SwViewShell::GetShellRes()

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
struct _CalcOp
{
    union{
        const sal_Char* pName;
        const OUString* pUName;
    };
    SwCalcOper eOp;
};

_CalcOp const aOpTable[] = {
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
static int SAL_CALL OperatorCompare( const void *pFirst, const void *pSecond)
{
    int nRet = 0;
    if( CALC_NAME == static_cast<const _CalcOp*>(pFirst)->eOp )
    {
        if( CALC_NAME == static_cast<const _CalcOp*>(pSecond)->eOp )
            nRet = static_cast<const _CalcOp*>(pFirst)->pUName->compareTo(
                   *static_cast<const _CalcOp*>(pSecond)->pUName );
        else
            nRet = static_cast<const _CalcOp*>(pFirst)->pUName->compareToAscii(
                   static_cast<const _CalcOp*>(pSecond)->pName );
    }
    else
    {
        if( CALC_NAME == static_cast<const _CalcOp*>(pSecond)->eOp )
            nRet = -1 * static_cast<const _CalcOp*>(pSecond)->pUName->compareToAscii(
                        static_cast<const _CalcOp*>(pFirst)->pName );
        else
            nRet = strcmp( static_cast<const _CalcOp*>(pFirst)->pName,
                           static_cast<const _CalcOp*>(pSecond)->pName );
    }
    return nRet;
}
}// extern "C"

_CalcOp* FindOperator( const OUString& rSrch )
{
    _CalcOp aSrch;
    aSrch.pUName = &rSrch;
    aSrch.eOp = CALC_NAME;

    return static_cast<_CalcOp*>(bsearch( static_cast<void*>(&aSrch),
                              static_cast<void const *>(aOpTable),
                              sizeof( aOpTable ) / sizeof( _CalcOp ),
                              sizeof( _CalcOp ),
                              OperatorCompare ));
}

SwHash* Find( const OUString& rStr, SwHash* const * ppTable,
              sal_uInt16 nTableSize, sal_uInt16* pPos )
{
    sal_uLong ii = 0;
    for( sal_Int32 n = 0; n < rStr.getLength(); ++n )
    {
        ii = ii << 1 ^ rStr[n];
    }
    ii %= nTableSize;

    if( pPos )
        *pPos = static_cast<sal_uInt16>(ii);

    for( SwHash* pEntry = *(ppTable+ii); pEntry; pEntry = pEntry->pNext )
    {
        if( rStr == pEntry->aStr )
        {
            return pEntry;
        }
    }
    return nullptr;
}

inline LanguageType GetDocAppScriptLang( SwDoc& rDoc )
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
        Date* pNull = pFormatter->GetNullDate();
        Date aDate( nDate >> 24, (nDate& 0x00FF0000) >> 16, nDate& 0x0000FFFF );
        nRet = aDate - *pNull;
    }
    return nRet;
}

SwCalc::SwCalc( SwDoc& rD )
    : aErrExpr( OUString(), SwSbxValue(), nullptr )
    , nCommandPos(0)
    , rDoc( rD )
    , pLclData( m_aSysLocale.GetLocaleDataPtr() )
    , pCharClass( &GetAppCharClass() )
    , nListPor( 0 )
    , eCurrOper( CALC_NAME )
    , eCurrListOper( CALC_NAME )
    , eError( CALC_NOERR )
{
    aErrExpr.aStr = "~C_ERR~";
    memset( VarTable, 0, sizeof(VarTable) );
    LanguageType eLang = GetDocAppScriptLang( rDoc );

    if( eLang != pLclData->getLanguageTag().getLanguageType() ||
        eLang != pCharClass->getLanguageTag().getLanguageType() )
    {
        LanguageTag aLanguageTag( eLang );
        pCharClass = new CharClass( ::comphelper::getProcessComponentContext(), aLanguageTag );
        pLclData = new LocaleDataWrapper( aLanguageTag );
    }

    sCurrSym = comphelper::string::strip(pLclData->getCurrSymbol(), ' ');
    sCurrSym  = pCharClass->lowercase( sCurrSym );

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

    const SwDocStat& rDocStat = rDoc.getIDocumentStatistics().GetDocStat();

    SwSbxValue nVal;
    OUString sTmpStr;
    sal_uInt16 n;

    for( n = 0; n < 25; ++n )
    {
        sTmpStr = OUString::createFromAscii(sNTypeTab[n]);
        VarTable[ aHashValue[ n ] ] = new SwCalcExp( sTmpStr, nVal, nullptr );
    }

    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 0 ] ])->nValue.PutBool( false );
    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 1 ] ])->nValue.PutBool( true );
    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 2 ] ])->nValue.PutDouble( F_PI );
    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 3 ] ])->nValue.PutDouble( 2.7182818284590452354 );

    for( n = 0; n < 3; ++n )
        static_cast<SwCalcExp*>(VarTable[ aHashValue[ n + 4 ] ])->nValue.PutLong( rDocStat.*aDocStat1[ n ]  );
    for( n = 0; n < 4; ++n )
        static_cast<SwCalcExp*>(VarTable[ aHashValue[ n + 7 ] ])->nValue.PutLong( rDocStat.*aDocStat2[ n ]  );

    SvtUserOptions& rUserOptions = SW_MOD()->GetUserOptions();

    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 11 ] ])->nValue.PutString( rUserOptions.GetFirstName() );
    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 12 ] ])->nValue.PutString( rUserOptions.GetLastName() );
    static_cast<SwCalcExp*>(VarTable[ aHashValue[ 13 ] ])->nValue.PutString( rUserOptions.GetID() );

    for( n = 0; n < 11; ++n )
        static_cast<SwCalcExp*>(VarTable[ aHashValue[ n + 14 ] ])->nValue.PutString(
                                        rUserOptions.GetToken( aAdrToken[ n ] ));

    nVal.PutString( rUserOptions.GetToken( aAdrToken[ 11 ] ));
    sTmpStr = OUString::createFromAscii(sNTypeTab[25]);
    VarTable[ aHashValue[ 25 ] ]->pNext = new SwCalcExp( sTmpStr, nVal, nullptr );

} // SwCalc::SwCalc

SwCalc::~SwCalc()
{
    for( sal_uInt16 n = 0; n < TBLSZ; ++n )
        delete VarTable[n];

    if( pLclData != m_aSysLocale.GetLocaleDataPtr() )
        delete pLclData;
    if( pCharClass != &GetAppCharClass() )
        delete pCharClass;
}

SwSbxValue SwCalc::Calculate( const OUString& rStr )
{
    eError = CALC_NOERR;
    SwSbxValue nResult;

    if( rStr.isEmpty() )
        return nResult;

    nListPor = 0;
    eCurrListOper = CALC_PLUS; // default: sum

    sCommand = rStr;
    nCommandPos = 0;

    while( (eCurrOper = GetToken()) != CALC_ENDCALC && eError == CALC_NOERR )
        nResult = Expr();

    if( eError )
        nResult.PutDouble( DBL_MAX );

    return nResult;
}

//TODO: provide documentation
/** ???

  @param rVal ???
  @param bRound In previous times <bRound> had a default value of <true>.
                There it should be only changed when calculating table cells,
                so that no rounding errors would occur while composing a formula.
                Now this parameter is ignored.
  @return ???
*/
OUString SwCalc::GetStrResult( const SwSbxValue& rVal, bool bRound )
{
    if( !rVal.IsDouble() )
    {
        return rVal.GetOUString();
    }
    return GetStrResult( rVal.GetDouble(), bRound );
}

OUString SwCalc::GetStrResult( double nValue, bool )
{
    if( nValue >= DBL_MAX )
        switch( eError )
        {
        case CALC_SYNTAX    :   return RESOURCE->aCalc_Syntax;
        case CALC_ZERODIV   :   return RESOURCE->aCalc_ZeroDiv;
        case CALC_BRACK     :   return RESOURCE->aCalc_Brack;
        case CALC_POWERR    :   return RESOURCE->aCalc_Pow;
        case CALC_VARNFND   :   return RESOURCE->aCalc_VarNFnd;
        case CALC_OVERFLOW  :   return RESOURCE->aCalc_Overflow;
        case CALC_WRONGTIME :   return RESOURCE->aCalc_WrongTime;
        default             :   return RESOURCE->aCalc_Default;
        }

    const sal_Int32 nDecPlaces = 15;
    OUString aRetStr( ::rtl::math::doubleToUString(
                        nValue,
                        rtl_math_StringFormat_Automatic,
                        nDecPlaces,
                        pLclData->getNumDecimalSep()[0],
                        true ));
    return aRetStr;
}

SwCalcExp* SwCalc::VarInsert( const OUString &rStr )
{
    OUString aStr = pCharClass->lowercase( rStr );
    return VarLook( aStr, true );
}

SwCalcExp* SwCalc::VarLook( const OUString& rStr, bool bIns )
{
    aErrExpr.nValue.SetVoidValue(false);

    sal_uInt16 ii = 0;
    OUString aStr = pCharClass->lowercase( rStr );

    SwHash* pFnd = Find( aStr, VarTable, TBLSZ, &ii );

    if( !pFnd )
    {
        // then check doc
        SwHash* const * ppDocTable = rDoc.getIDocumentFieldsAccess().GetUpdateFields().GetFieldTypeTable();
        for( SwHash* pEntry = *(ppDocTable+ii); pEntry; pEntry = pEntry->pNext )
        {
            if( aStr == pEntry->aStr )
            {
                // then insert here
                pFnd = new SwCalcExp( aStr, SwSbxValue(),
                                    static_cast<SwCalcFieldType*>(pEntry)->pFieldType );
                pFnd->pNext = *(VarTable+ii);
                *(VarTable+ii) = pFnd;
                break;
            }
        }
    }

    if( pFnd )
    {
        SwCalcExp* pFndExp = static_cast<SwCalcExp*>(pFnd);

        if( pFndExp->pFieldType && pFndExp->pFieldType->Which() == RES_USERFLD )
        {
            SwUserFieldType* pUField = const_cast<SwUserFieldType*>(static_cast<const SwUserFieldType*>(pFndExp->pFieldType));
            if( nsSwGetSetExpType::GSE_STRING & pUField->GetType() )
            {
                pFndExp->nValue.PutString( pUField->GetContent() );
            }
            else if( !pUField->IsValid() )
            {
                // Save the current values...
                sal_uInt16          nOld_ListPor        = nListPor;
                SwSbxValue      nOld_LastLeft       = nLastLeft;
                SwSbxValue      nOld_NumberValue    = nNumberValue;
                sal_Int32      nOld_CommandPos     = nCommandPos;
                SwCalcOper      eOld_CurrOper       = eCurrOper;
                SwCalcOper      eOld_CurrListOper   = eCurrListOper;

                pFndExp->nValue.PutDouble( pUField->GetValue( *this ) );

                // ...and write them back.
                nListPor        = nOld_ListPor;
                nLastLeft       = nOld_LastLeft;
                nNumberValue    = nOld_NumberValue;
                nCommandPos     = nOld_CommandPos;
                eCurrOper       = eOld_CurrOper;
                eCurrListOper   = eOld_CurrListOper;
            }
            else
            {
                pFndExp->nValue.PutDouble( pUField->GetValue() );
            }
        }
        return pFndExp;
    }

    // At this point the "real" case variable has to be used
    OUString const sTmpName( ::ReplacePoint(rStr) );

    if( !bIns )
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        SwDBManager *pMgr = rDoc.GetDBManager();

        OUString sDBName(GetDBName( sTmpName ));
        OUString sSourceName(sDBName.getToken(0, DB_DELIM));
        OUString sTableName(sDBName.getToken(0, ';').getToken(1, DB_DELIM));
        if( pMgr && !sSourceName.isEmpty() && !sTableName.isEmpty() &&
            pMgr->OpenDataSource(sSourceName, sTableName))
        {
            OUString sColumnName( GetColumnName( sTmpName ));
            OSL_ENSURE(!sColumnName.isEmpty(), "Missing DB column name");

            OUString sDBNum( SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD) );
            sDBNum = pCharClass->lowercase(sDBNum);

            // Initialize again because this doesn't happen in docfld anymore for
            // elements != RES_DBFLD. E.g. if there is an expression field before
            // an DB_Field in a document.
            VarChange( sDBNum, pMgr->GetSelectedRecordId(sSourceName, sTableName));

            if( sDBNum.equalsIgnoreAsciiCase(sColumnName) )
            {
                aErrExpr.nValue.PutLong(long(pMgr->GetSelectedRecordId(sSourceName, sTableName)));
                return &aErrExpr;
            }

            sal_uLong nTmpRec = 0;
            if( nullptr != ( pFnd = Find( sDBNum, VarTable, TBLSZ ) ) )
                nTmpRec = static_cast<SwCalcExp*>(pFnd)->nValue.GetULong();

            OUString sResult;
            double nNumber = DBL_MAX;

            long nLang = pLclData->getLanguageTag().getLanguageType();
            if(pMgr->GetColumnCnt( sSourceName, sTableName, sColumnName,
                                    nTmpRec, nLang, sResult, &nNumber ))
            {
                if (nNumber != DBL_MAX)
                    aErrExpr.nValue.PutDouble( nNumber );
                else
                    aErrExpr.nValue.PutString( sResult );

                return &aErrExpr;
            }
        }
        else
#endif
        {
            //data source was not available - set return to "NoValue"
            aErrExpr.nValue.SetVoidValue(true);
        }
        // NEVER save!
        return &aErrExpr;
    }

    SwCalcExp* pNewExp = new SwCalcExp( aStr, SwSbxValue(), nullptr );
    pNewExp->pNext = VarTable[ ii ];
    VarTable[ ii ] = pNewExp;

    OUString sColumnName( GetColumnName( sTmpName ));
    OSL_ENSURE( !sColumnName.isEmpty(), "Missing DB column name" );
    if( sColumnName.equalsIgnoreAsciiCase(
                            SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) ))
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        SwDBManager *pMgr = rDoc.GetDBManager();
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
    OUString aStr = pCharClass->lowercase( rStr );

    sal_uInt16 nPos = 0;
    SwCalcExp* pFnd = static_cast<SwCalcExp*>(Find( aStr, VarTable, TBLSZ, &nPos ));

    if( !pFnd )
    {
        pFnd = new SwCalcExp( aStr, SwSbxValue( rValue ), nullptr );
        pFnd->pNext = VarTable[ nPos ];
        VarTable[ nPos ] = pFnd;
    }
    else
    {
        pFnd->nValue = rValue;
    }
}

bool SwCalc::Push( const SwUserFieldType* pUserFieldType )
{
    if( aRekurStack.end() != std::find(aRekurStack.begin(), aRekurStack.end(), pUserFieldType ) )
        return false;

    aRekurStack.push_back( pUserFieldType );
    return true;
}

void SwCalc::Pop()
{
    OSL_ENSURE( aRekurStack.size(), "SwCalc: Pop on an invalid pointer" );

    aRekurStack.pop_back();
}

SwCalcOper SwCalc::GetToken()
{
#if OSL_DEBUG_LEVEL > 1
    // static for switch back to the "old" implementation of the calculator
    // which doesn't use the I18N routines.
    static int nUseOld = 0;
    if( !nUseOld )
    {
#endif

    if( nCommandPos >= sCommand.getLength() )
        return eCurrOper = CALC_ENDCALC;

    using namespace ::com::sun::star::i18n;
    {
        // Parse any token.
        ParseResult aRes = pCharClass->parseAnyToken( sCommand, nCommandPos,
                                                      coStartFlags, OUString(),
                                                      coContFlags, OUString());

        bool bSetError = true;
        sal_Int32 nRealStt = nCommandPos + aRes.LeadingWhiteSpace;
        if( aRes.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER) )
        {
            nNumberValue.PutDouble( aRes.Value );
            eCurrOper = CALC_NUMBER;
            bSetError = false;
        }
        else if( aRes.TokenType & KParseType::IDENTNAME )
        {
            OUString aName( sCommand.copy( nRealStt,
                            aRes.EndPos - nRealStt ) );
            //#101436#: The variable may contain a database name. It must not be
            // converted to lower case! Instead all further comparisons must be
            // done case-insensitive
            OUString sLowerCaseName = pCharClass->lowercase( aName );
            // catch currency symbol
            if( sLowerCaseName == sCurrSym )
            {
                nCommandPos = aRes.EndPos;
                return GetToken(); // call again
            }

            // catch operators
            _CalcOp* pFnd = ::FindOperator( sLowerCaseName );
            if( pFnd )
            {
                switch( ( eCurrOper = static_cast<_CalcOp*>(pFnd)->eOp ) )
                {
                case CALC_SUM:
                case CALC_MEAN:
                    eCurrListOper = CALC_PLUS;
                    break;
                case CALC_MIN:
                    eCurrListOper = CALC_MIN_IN;
                    break;
                case CALC_MAX:
                    eCurrListOper = CALC_MAX_IN;
                    break;
                case CALC_DATE:
                    eCurrListOper = CALC_MONTH;
                    break;
                default:
                    break;
                }
                nCommandPos = aRes.EndPos;
                return eCurrOper;
            }
            aVarName = aName;
            eCurrOper = CALC_NAME;
            bSetError = false;
        }
        else if ( aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING )
        {
            nNumberValue.PutString( aRes.DequotedNameOrString );
            eCurrOper = CALC_NUMBER;
            bSetError = false;
        }
        else if( aRes.TokenType & KParseType::ONE_SINGLE_CHAR )
        {
            OUString aName( sCommand.copy( nRealStt,
                              aRes.EndPos - nRealStt ));
            if( 1 == aName.getLength() )
            {
                bSetError = false;
                sal_Unicode ch = aName[0];
                switch( ch )
                {
                case ';':
                    if( CALC_MONTH == eCurrListOper || CALC_DAY == eCurrListOper )
                    {
                        eCurrOper = eCurrListOper;
                        break;
                    }
                    SAL_FALLTHROUGH;
                case '\n':
                    eCurrOper = CALC_PRINT;
                    break;

                case '%':
                case '^':
                case '*':
                case '/':
                case '+':
                case '-':
                case '(':
                case ')':
                    eCurrOper = SwCalcOper(ch);
                    break;

                case '=':
                case '!':
                    {
                        SwCalcOper eTmp2;
                        if( '=' == ch )
                        {
                            eCurrOper = SwCalcOper('=');
                            eTmp2 = CALC_EQ;
                        }
                        else
                        {
                            eCurrOper = CALC_NOT;
                            eTmp2 = CALC_NEQ;
                        }

                        if( aRes.EndPos < sCommand.getLength() &&
                            '=' == sCommand[aRes.EndPos] )
                        {
                            eCurrOper = eTmp2;
                            ++aRes.EndPos;
                        }
                    }
                    break;

                case cListDelim:
                    eCurrOper = eCurrListOper;
                    break;

                case '[':
                    if( aRes.EndPos < sCommand.getLength() )
                    {
                        aVarName.clear();
                        sal_Int32 nFndPos = aRes.EndPos,
                                  nSttPos = nFndPos;

                        do {
                            if( -1 != ( nFndPos =
                                sCommand.indexOf( ']', nFndPos )) )
                            {
                                // ignore the ]
                                if ('\\' == sCommand[nFndPos-1])
                                {
                                    aVarName += sCommand.copy( nSttPos,
                                                    nFndPos - nSttPos - 1 );
                                    nSttPos = ++nFndPos;
                                }
                                else
                                    break;
                            }
                        } while( nFndPos != -1 );

                        if( nFndPos != -1 )
                        {
                            if( nSttPos != nFndPos )
                                aVarName += sCommand.copy( nSttPos,
                                                    nFndPos - nSttPos );
                            aRes.EndPos = nFndPos + 1;
                            eCurrOper = CALC_NAME;
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
            OUString aName( sCommand.copy( nRealStt,
                                         aRes.EndPos - nRealStt ));
            if( !aName.isEmpty() )
            {
                sal_Unicode ch = aName[0];

                bSetError = true;
                if ('<' == ch || '>' == ch)
                {
                    bSetError = false;

                    SwCalcOper eTmp2 = ('<' == ch) ? CALC_LEQ : CALC_GEQ;
                    eCurrOper = ('<' == ch) ? CALC_LES : CALC_GRE;

                    if( 2 == aName.getLength() && '=' == aName[1] )
                        eCurrOper = eTmp2;
                    else if( 1 != aName.getLength() )
                        bSetError = true;
                }
            }
        }
        else if( nRealStt == sCommand.getLength() )
        {
            eCurrOper = CALC_ENDCALC;
            bSetError = false;
        }

        if( bSetError )
        {
            eError = CALC_SYNTAX;
            eCurrOper = CALC_PRINT;
        }
        nCommandPos = aRes.EndPos;
    };

#if OSL_DEBUG_LEVEL > 1
#define NextCh( s, n )  (nCommandPos < sCommand.getLength() ? sCommand[nCommandPos++] : 0)

    }
    else
    {
        sal_Unicode ch;
        sal_Unicode cTSep = pLclData->getNumThousandSep()[0],
                    cDSep = pLclData->getNumDecimalSep()[0];

        do {
            if( 0 == ( ch = NextCh( sCommand, nCommandPos ) ) )
                return eCurrOper = CALC_ENDCALC;
        } while ( ch == '\t' || ch == ' ' || ch == cTSep );

        if( ch == cDSep )
            ch = '.';

        switch( ch )
        {
        case ';':
            if( CALC_MONTH == eCurrListOper || CALC_DAY == eCurrListOper )
            {
                eCurrOper = eCurrListOper;
                break;
            } // else .. no break
        case '\n':
            {
                sal_Unicode c;
                while( nCommandPos < sCommand.getLength() &&
                       ( ( c = sCommand[nCommandPos] ) == ' ' ||
                       c == '\t' || c == '\x0a' || c == '\x0d' ))
                {
                    ++nCommandPos;
                }
                eCurrOper = CALC_PRINT;
            }
            break;

        case '%':
        case '^':
        case '*':
        case '/':
        case '+':
        case '-':
        case '(':
        case ')':
            eCurrOper = SwCalcOper(ch);
            break;

        case '=':
            if( '=' == sCommand[nCommandPos] )
            {
                ++nCommandPos;
                eCurrOper = CALC_EQ;
            }
            else
            {
                eCurrOper = SwCalcOper(ch);
            }
            break;

        case '!':
            if( '=' == sCommand[nCommandPos] )
            {
                ++nCommandPos;
                eCurrOper = CALC_NEQ;
            }
            else
            {
                eCurrOper = CALC_NOT;
            }
            break;

        case '>':
        case '<':
            eCurrOper = '>' == ch  ? CALC_GRE : CALC_LES;
            if( '=' == (ch = sCommand[nCommandPos] ) )
            {
                ++nCommandPos;
                eCurrOper = CALC_GRE == eCurrOper ? CALC_GEQ : CALC_LEQ;
            }
            else if( ' ' != ch )
            {
                eError = CALC_SYNTAX;
                eCurrOper = CALC_PRINT;
            }
            break;

        case cListDelim :
            eCurrOper = eCurrListOper;
            break;

        case '0':   case '1':   case '2':   case '3':   case '4':
        case '5':   case '6':   case '7':   case '8':   case '9':
        case ',':
        case '.':
            {
                double nVal;
                --nCommandPos; //  back to the first char
                if( Str2Double( sCommand, nCommandPos, nVal, pLclData ))
                {
                    nNumberValue.PutDouble( nVal );
                    eCurrOper = CALC_NUMBER;
                }
                else
                {
                    // erroneous number
                    eError = CALC_SYNTAX;
                    eCurrOper = CALC_PRINT;
                }
            }
            break;

        case '[':
            {
                OUString aStr;
                bool bIgnore = false;
                do {
                    while( 0 != ( ch = NextCh( sCommand, nCommandPos  )) &&
                           ch != ']' )
                    {
                        if( !bIgnore && '\\' == ch )
                            bIgnore = true;
                        else if( bIgnore )
                            bIgnore = false;
                        aStr += OUString(ch);
                    }

                    if( !bIgnore )
                        break;

                    aStr = aStr.replaceAt(aStr.getLength() - 1, 1, OUString(ch));
                } while( ch );

                aVarName = aStr;
                eCurrOper = CALC_NAME;
            }
            break;

        case '"':
            {
                sal_Int32 nStt = nCommandPos;
                while( 0 != ( ch = NextCh( sCommand, nCommandPos ) ) &&
                       '"' != ch )
                {
                    ;
                }

                sal_Int32 nLen = nCommandPos - nStt;
                if( '"' == ch )
                    --nLen;
                nNumberValue.PutString( sCommand.copy( nStt, nLen ));
                eCurrOper = CALC_NUMBER;
            }
            break;

        default:
            if (ch && (pCharClass->isLetter( sCommand, nCommandPos - 1) ||
                '_' == ch))
            {
                sal_Int32 nStt = nCommandPos-1;
                while( 0 != (ch = NextCh( sCommand, nCommandPos )) &&
                       (pCharClass->isLetterNumeric( sCommand, nCommandPos - 1) ||
                       ch == '_' || ch == '.' ) )
                {
                    ;
                }

                if( ch )
                    --nCommandPos;

                OUString aStr( sCommand.copy( nStt, nCommandPos-nStt ));
                aStr = pCharClass->lowercase( aStr );

                // catch currency symbol
                if( aStr == sCurrSym )
                    return GetToken();  // call again

                // catch operators
                _CalcOp* pFnd = ::FindOperator( aStr );
                if( pFnd )
                {
                    switch( ( eCurrOper = static_cast<_CalcOp*>(pFnd)->eOp ) )
                    {
                    case CALC_SUM :
                    case CALC_MEAN :
                        eCurrListOper = CALC_PLUS;
                        break;
                    case CALC_MIN :
                        eCurrListOper = CALC_MIN_IN;
                        break;
                    case CALC_MAX :
                        eCurrListOper = CALC_MAX_IN;
                        break;
                    case CALC_DATE :
                        eCurrListOper = CALC_MONTH;
                        break;
                    default :
                        break;
                    }
                    return eCurrOper;
                }
                aVarName = aStr;
                eCurrOper = CALC_NAME;
            }
            else
            {
                eError = CALC_SYNTAX;
                eCurrOper = CALC_PRINT;
            }
            break;
        }
    }
#endif
    return eCurrOper;
}

SwSbxValue SwCalc::Term()
{
    SwSbxValue left( Prim() );
    nLastLeft = left;
    for(;;)
    {
        sal_uInt16 nSbxOper = USHRT_MAX;

        switch( eCurrOper )
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
                left.PutBool( (bL && !bR) || (!bL && bR) );
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
                sal_Int32 nYear = (sal_Int32) floor( left.GetDouble() );
                nYear = nYear & 0x0000FFFF;
                sal_Int32 nMonth = (sal_Int32) floor( e.GetDouble() );
                nMonth = ( nMonth & 0x000000FF ) << 16;
                left.PutLong( nMonth + nYear );
                eCurrOper = CALC_DAY;
            }
            break;
        case CALC_DAY:
            {
                GetToken();
                SwSbxValue e = Prim();
                sal_Int32 nYearMonth = (sal_Int32) floor( left.GetDouble() );
                nYearMonth = nYearMonth & 0x00FFFFFF;
                sal_Int32 nDay = (sal_Int32) floor( e.GetDouble() );
                nDay = ( nDay & 0x000000FF ) << 24;
                left = lcl_ConvertToDateValue( rDoc, nDay + nYearMonth );
            }
            break;
        case CALC_ROUND:
            {
                GetToken();
                SwSbxValue e = Prim();

                double fVal = 0;
                double fFac = 1;
                sal_Int32 nDec = (sal_Int32) floor( e.GetDouble() );
                if( nDec < -20 || nDec > 20 )
                {
                    eError = CALC_OVERFLOW;
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
            SbxOperator eSbxOper = (SbxOperator)nSbxOper;

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
                    eError = CALC_ZERODIV;
                else
                    left.Compute( eSbxOper, aRight );
            }
        }
    }
}

extern "C" typedef double (*pfCalc)( double );

SwSbxValue SwCalc::Prim()
{
    SwSbxValue nErg;

    pfCalc pFnc = nullptr;

    bool bChkTrig = false, bChkPow = false;

    switch( eCurrOper )
    {
    case CALC_SIN:  pFnc = &sin;  break;
    case CALC_COS:  pFnc = &cos;  break;
    case CALC_TAN:  pFnc = &tan;  break;
    case CALC_ATAN: pFnc = &atan; break;
    case CALC_ASIN: pFnc = &asin; bChkTrig = true; break;
    case CALC_ACOS: pFnc = &acos; bChkTrig = true; break;

    case CALC_NOT:
        {
            GetToken();
            nErg = Prim();
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
        }
        break;

    case CALC_NUMBER:
        if( GetToken() == CALC_PHD )
        {
            double aTmp = nNumberValue.GetDouble();
            aTmp *= 0.01;
            nErg.PutDouble( aTmp );
            GetToken();
        }
        else if( eCurrOper == CALC_NAME )
        {
            eError = CALC_SYNTAX;
        }
        else
        {
            nErg = nNumberValue;
            bChkPow = true;
        }
        break;

    case CALC_NAME:
        switch(SwCalcOper eOper = GetToken())
        {
            case CALC_ASSIGN:
                {
                    SwCalcExp* n = VarInsert(aVarName);
                    GetToken();
                    nErg = n->nValue = Expr();
                }
                break;
            default:
                nErg = VarLook(aVarName)->nValue;
                // Explicitly disallow unknown function names (followed by "("),
                // allow unknown variable names (equal to zero)
                if (nErg.IsVoidValue() && (eOper == CALC_LP))
                    eError = CALC_SYNTAX;
                else
                    bChkPow = true;
        }
        break;

    case CALC_MINUS:
        GetToken();
        nErg.PutDouble( -(Prim().GetDouble()) );
        break;

    case CALC_LP:
        {
            GetToken();
            nErg = Expr();
            if( eCurrOper != CALC_RP )
            {
                eError = CALC_BRACK;
            }
            else
            {
                GetToken();
                bChkPow = true; // in order for =(7)^2 to work
            }
        }
        break;

    case CALC_MEAN:
        {
            nListPor = 1;
            GetToken();
            nErg = Expr();
            double aTmp = nErg.GetDouble();
            aTmp /= nListPor;
            nErg.PutDouble( aTmp );
        }
        break;

    case CALC_SQRT:
        {
            GetToken();
            nErg = Prim();
            if( nErg.GetDouble() < 0 )
                eError = CALC_OVERFLOW;
            else
                nErg.PutDouble( sqrt( nErg.GetDouble() ));
        }
        break;

    case CALC_SUM:
    case CALC_DATE:
    case CALC_MIN:
    case CALC_MAX:
        GetToken();
        nErg = Expr();
        break;

    case CALC_ENDCALC:
        nErg.Clear();
        break;

    default:
        eError = CALC_SYNTAX;
        break;
    }

    if( pFnc )
    {
        GetToken();
        double nVal = Prim().GetDouble();
        if( !bChkTrig || ( nVal > -1 && nVal < 1 ) )
            nErg.PutDouble( (*pFnc)( nVal ) );
        else
            eError = CALC_OVERFLOW;
    }

    if( bChkPow && eCurrOper == CALC_POW )
    {
        double dleft = nErg.GetDouble();
        GetToken();
        double right = Prim().GetDouble();

        double fraction, integer;
        fraction = modf( right, &integer );
        if( ( dleft < 0.0 && 0.0 != fraction ) ||
            ( 0.0 == dleft && right < 0.0 ) )
        {
            eError = CALC_OVERFLOW;
            nErg.Clear();
        }
        else
        {
            dleft = pow(dleft, right );
            if( dleft == HUGE_VAL )
            {
                eError = CALC_POWERR;
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
    SwSbxValue left = Term(), right;
    nLastLeft = left;
    for(;;)
    {
        switch(eCurrOper)
        {
        case CALC_PLUS:
            GetToken();
            left.MakeDouble();
            ( right = Term() ).MakeDouble();
            left.Compute( SbxPLUS, right );
            nListPor++;
            break;

        case CALC_MINUS:
            GetToken();
            left.MakeDouble();
            ( right = Term() ).MakeDouble();
            left.Compute( SbxMINUS, right );
            break;

        default:
            return left;
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
    SwDBData aData = rDoc.GetDBData();
    OUString sRet = aData.sDataSource;
    sRet += OUString(DB_DELIM);
    sRet += aData.sCommand;
    return sRet;
}

namespace
{
    static bool lcl_Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                                double& rVal,
                                const LocaleDataWrapper* const pLclData )
    {
        OSL_ASSERT(pLclData);
        const sal_Unicode nCurrCmdPos = rCommandPos;
        rtl_math_ConversionStatus eStatus;
        const sal_Unicode* pEnd;
        rVal = rtl_math_uStringToDouble( rCommand.getStr() + rCommandPos,
                                         rCommand.getStr() + rCommand.getLength(),
                                         pLclData->getNumDecimalSep()[0],
                                         pLclData->getNumThousandSep()[0],
                                         &eStatus,
                                         &pEnd );
        rCommandPos = static_cast<sal_Int32>(pEnd - rCommand.getStr());

        return rtl_math_ConversionStatus_Ok == eStatus &&
               nCurrCmdPos != rCommandPos;
    }
}

bool SwCalc::Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                         double& rVal, const LocaleDataWrapper* const pLclData )
{
    const SvtSysLocale aSysLocale;
    return lcl_Str2Double( rCommand, rCommandPos, rVal,
            pLclData ? pLclData : aSysLocale.GetLocaleDataPtr() );
}

bool SwCalc::Str2Double( const OUString& rCommand, sal_Int32& rCommandPos,
                         double& rVal, SwDoc* const pDoc )
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

    bool const bRet = lcl_Str2Double( rCommand, rCommandPos, rVal,
            (pLclD.get()) ? pLclD.get() : aSysLocale.GetLocaleDataPtr() );

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
    , pNext(nullptr)
{
}

SwHash::~SwHash()
{
    delete pNext;
}

void DeleteHashTable( SwHash **ppHashTable, sal_uInt16 nCount )
{
    for ( sal_uInt16 i = 0; i < nCount; ++i )
        delete *(ppHashTable+i);
    delete [] ppHashTable;
}

SwCalcExp::SwCalcExp(const OUString& rStr, const SwSbxValue& rVal,
                      const SwFieldType* pType)
    : SwHash(rStr)
    , nValue(rVal)
    , pFieldType(pType)
{
}

SwSbxValue::~SwSbxValue()
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
