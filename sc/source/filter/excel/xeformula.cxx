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

#include <list>
#include <map>
#include "addincol.hxx"
#include "compiler.hxx"
#include "document.hxx"
#include "externalrefmgr.hxx"
#include "rangelst.hxx"
#include "token.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"
#include "xeformula.hxx"
#include "xehelper.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xestream.hxx"

using namespace ::formula;

// External reference log =====================================================

XclExpRefLogEntry::XclExpRefLogEntry() :
    mpUrl( nullptr ),
    mpFirstTab( nullptr ),
    mpLastTab( nullptr ),
    mnFirstXclTab( EXC_TAB_DELETED ),
    mnLastXclTab( EXC_TAB_DELETED )
{
}

// Formula compiler ===========================================================

namespace {

/** Wrapper structure for a processed Calc formula token with additional
    settings (whitespaces). */
struct XclExpScToken
{
    const FormulaToken* mpScToken;          /// Currently processed Calc token.
    sal_uInt8           mnSpaces;           /// Number of spaces before the Calc token.

    inline explicit     XclExpScToken() : mpScToken( nullptr ), mnSpaces( 0 ) {}
    inline bool         Is() const { return mpScToken != nullptr; }
    inline StackVar     GetType() const { return mpScToken ? mpScToken->GetType() : static_cast< StackVar >( svUnknown ); }
    inline OpCode       GetOpCode() const { return mpScToken ? mpScToken->GetOpCode() : static_cast< OpCode >( ocNone ); }
};

/** Effective token class conversion types. */
enum XclExpClassConv
{
    EXC_CLASSCONV_ORG,          /// Keep original class of the token.
    EXC_CLASSCONV_VAL,          /// Convert ARR tokens to VAL class (REF remains uncahnged).
    EXC_CLASSCONV_ARR           /// Convert VAL tokens to ARR class (REF remains uncahnged).
};

/** Token class conversion and position of a token in the token array. */
struct XclExpTokenConvInfo
{
    sal_uInt16          mnTokPos;       /// Position of the token in the token array.
    XclFuncParamConv    meConv;         /// Token class conversion type.
    bool                mbValType;      /// Data type (false = REFTYPE, true = VALTYPE).
};

/** Vector of token position and conversion for all operands of an operator,
    or for all parameters of a function. */
struct XclExpOperandList : public std::vector< XclExpTokenConvInfo >
{
    inline explicit     XclExpOperandList() { reserve( 2 ); }
    void                AppendOperand( sal_uInt16 nTokPos, XclFuncParamConv eConv, bool bValType );
};

void XclExpOperandList::AppendOperand( sal_uInt16 nTokPos, XclFuncParamConv eConv, bool bValType )
{
    resize( size() + 1 );
    XclExpTokenConvInfo& rConvInfo = back();
    rConvInfo.mnTokPos = nTokPos;
    rConvInfo.meConv = eConv;
    rConvInfo.mbValType = bValType;
}

typedef std::shared_ptr< XclExpOperandList > XclExpOperandListRef;
typedef std::vector< XclExpOperandListRef > XclExpOperandListVector;

/** Encapsulates all data needed for a call to an external function (macro, add-in). */
struct XclExpExtFuncData
{
    OUString            maFuncName;         /// Name of the function.
    bool                mbVBasic;           /// True = Visual Basic macro call.
    bool                mbHidden;           /// True = Create hidden defined name.

    inline explicit     XclExpExtFuncData() : mbVBasic( false ), mbHidden( false ) {}
    void                Set( const OUString& rFuncName, bool bVBasic, bool bHidden );
};

void XclExpExtFuncData::Set( const OUString& rFuncName, bool bVBasic, bool bHidden )
{
    maFuncName = rFuncName;
    mbVBasic = bVBasic;
    mbHidden = bHidden;
}

/** Encapsulates all data needed to process an entire function. */
class XclExpFuncData
{
public:
    explicit            XclExpFuncData(
                            const XclExpScToken& rTokData,
                            const XclFunctionInfo& rFuncInfo,
                            const XclExpExtFuncData& rExtFuncData );

    inline const FormulaToken& GetScToken() const { return *mrTokData.mpScToken; }
    inline OpCode       GetOpCode() const { return mrFuncInfo.meOpCode; }
    inline sal_uInt16   GetXclFuncIdx() const { return mrFuncInfo.mnXclFunc; }
    inline bool         IsVolatile() const { return mrFuncInfo.IsVolatile(); }
    inline bool         IsFixedParamCount() const { return mrFuncInfo.IsFixedParamCount(); }
    inline bool         IsAddInEquivalent() const { return mrFuncInfo.IsAddInEquivalent(); }
    inline bool         IsMacroFunc() const { return mrFuncInfo.IsMacroFunc(); }
    inline sal_uInt8    GetSpaces() const { return mrTokData.mnSpaces; }
    inline const XclExpExtFuncData& GetExtFuncData() const { return maExtFuncData; }
    inline sal_uInt8    GetReturnClass() const { return mrFuncInfo.mnRetClass; }

    const XclFuncParamInfo& GetParamInfo() const;
    bool                IsCalcOnlyParam() const;
    bool                IsExcelOnlyParam() const;
    void                IncParamInfoIdx();

    inline sal_uInt8    GetMinParamCount() const { return mrFuncInfo.mnMinParamCount; }
    inline sal_uInt8    GetMaxParamCount() const { return mrFuncInfo.mnMaxParamCount; }
    inline sal_uInt8    GetParamCount() const { return static_cast< sal_uInt8 >( mxOperands->size() ); }
    void                FinishParam( sal_uInt16 nTokPos );
    inline XclExpOperandListRef GetOperandList() const { return mxOperands; }

    inline ScfUInt16Vec& GetAttrPosVec() { return maAttrPosVec; }
    inline void         AppendAttrPos( sal_uInt16 nPos ) { maAttrPosVec.push_back( nPos ); }

private:
    ScfUInt16Vec        maAttrPosVec;       /// Token array positions of tAttr tokens.
    const XclExpScToken& mrTokData;         /// Data about processed function name token.
    const XclFunctionInfo& mrFuncInfo;      /// Constant data about processed function.
    XclExpExtFuncData   maExtFuncData;      /// Data for external functions (macro, add-in).
    XclExpOperandListRef mxOperands;        /// Class conversion and position of all parameters.
    const XclFuncParamInfo* mpParamInfo;    /// Information for current parameter.
};

XclExpFuncData::XclExpFuncData( const XclExpScToken& rTokData,
        const XclFunctionInfo& rFuncInfo, const XclExpExtFuncData& rExtFuncData ) :
    mrTokData( rTokData ),
    mrFuncInfo( rFuncInfo ),
    maExtFuncData( rExtFuncData ),
    mxOperands( new XclExpOperandList ),
    mpParamInfo( rFuncInfo.mpParamInfos )
{
    OSL_ENSURE( mrTokData.mpScToken, "XclExpFuncData::XclExpFuncData - missing core token" );
    // set name of an add-in function
    if( (maExtFuncData.maFuncName.isEmpty()) && dynamic_cast< const FormulaExternalToken* >( mrTokData.mpScToken ) )
        maExtFuncData.Set( GetScToken().GetExternal(), true, false );
}

const XclFuncParamInfo& XclExpFuncData::GetParamInfo() const
{
    static const XclFuncParamInfo saInvalidInfo = { EXC_PARAM_NONE, EXC_PARAMCONV_ORG, false };
    return mpParamInfo ? *mpParamInfo : saInvalidInfo;
}

bool XclExpFuncData::IsCalcOnlyParam() const
{
    return mpParamInfo && (mpParamInfo->meValid == EXC_PARAM_CALCONLY);
}

bool XclExpFuncData::IsExcelOnlyParam() const
{
    return mpParamInfo && (mpParamInfo->meValid == EXC_PARAM_EXCELONLY);
}

void XclExpFuncData::IncParamInfoIdx()
{
    if( mpParamInfo )
    {
        // move pointer to next entry, if something explicit follows
        if( (static_cast< size_t >( mpParamInfo - mrFuncInfo.mpParamInfos + 1 ) < EXC_FUNCINFO_PARAMINFO_COUNT) && (mpParamInfo[ 1 ].meValid != EXC_PARAM_NONE) )
            ++mpParamInfo;
        // if last parameter type is 'Excel-only' or 'Calc-only', do not repeat it
        else if( IsExcelOnlyParam() || IsCalcOnlyParam() )
            mpParamInfo = nullptr;
        // points to last info, but parameter pairs expected, move to previous info
        else if( mrFuncInfo.IsParamPairs() )
            --mpParamInfo;
        // otherwise: repeat last parameter class
    }
}

void XclExpFuncData::FinishParam( sal_uInt16 nTokPos )
{
    // write token class conversion info for this parameter
    const XclFuncParamInfo& rParamInfo = GetParamInfo();
    mxOperands->AppendOperand( nTokPos, rParamInfo.meConv, rParamInfo.mbValType );
    // move to next parameter info structure
    IncParamInfoIdx();
}

// compiler configuration -----------------------------------------------------

/** Type of token class handling. */
enum XclExpFmlaClassType
{
    EXC_CLASSTYPE_CELL,         /// Cell formula, shared formula.
    EXC_CLASSTYPE_ARRAY,        /// Array formula, conditional formatting, data validation.
    EXC_CLASSTYPE_NAME          /// Defined name, range list.
};

/** Configuration data of the formula compiler. */
struct XclExpCompConfig
{
    XclFormulaType      meType;         /// Type of the formula to be created.
    XclExpFmlaClassType meClassType;    /// Token class handling type.
    bool                mbLocalLinkMgr; /// True = local (per-sheet) link manager, false = global.
    bool                mbFromCell;     /// True = Any kind of cell formula (cell, array, shared).
    bool                mb3DRefOnly;    /// True = Only 3D references allowed (e.g. names).
    bool                mbAllowArrays;  /// True = Allow inline arrays.
};

/** The table containing configuration data for all formula types. */
static const XclExpCompConfig spConfigTable[] =
{
    // formula type         token class type     lclLM  inCell 3dOnly allowArray
    { EXC_FMLATYPE_CELL,    EXC_CLASSTYPE_CELL,  true,  true,  false, true  },
    { EXC_FMLATYPE_SHARED,  EXC_CLASSTYPE_CELL,  true,  true,  false, true  },
    { EXC_FMLATYPE_MATRIX,  EXC_CLASSTYPE_ARRAY, true,  true,  false, true  },
    { EXC_FMLATYPE_CONDFMT, EXC_CLASSTYPE_ARRAY, true,  false, false, false },
    { EXC_FMLATYPE_DATAVAL, EXC_CLASSTYPE_ARRAY, true,  false, false, false },
    { EXC_FMLATYPE_NAME,    EXC_CLASSTYPE_NAME,  false, false, true,  true  },
    { EXC_FMLATYPE_CHART,   EXC_CLASSTYPE_NAME,  true,  false, true,  true  },
    { EXC_FMLATYPE_CONTROL, EXC_CLASSTYPE_NAME,  true,  false, false, false },
    { EXC_FMLATYPE_WQUERY,  EXC_CLASSTYPE_NAME,  true,  false, true,  false },
    { EXC_FMLATYPE_LISTVAL, EXC_CLASSTYPE_NAME,  true,  false, false, false }
};

/** Working data of the formula compiler. Used to push onto a stack for recursive calls. */
struct XclExpCompData
{
    typedef std::shared_ptr< ScTokenArray > ScTokenArrayRef;

    const XclExpCompConfig& mrCfg;          /// Configuration for current formula type.
    ScTokenArrayRef     mxOwnScTokArr;      /// Own clone of a Calc token array.
    XclTokenArrayIterator maTokArrIt;       /// Iterator in Calc token array.
    XclExpLinkManager*  mpLinkMgr;          /// Link manager for current context (local/global).
    XclExpRefLog*       mpRefLog;           /// Log for external references.
    const ScAddress*    mpScBasePos;        /// Current cell position of the formula.

    ScfUInt8Vec         maTokVec;           /// Byte vector containing token data.
    ScfUInt8Vec         maExtDataVec;       /// Byte vector containing extended data (arrays, stacked NLRs).
    XclExpOperandListVector maOpListVec;    /// Formula structure, maps operators to their operands.
    ScfUInt16Vec        maOpPosStack;       /// Stack with positions of operand tokens waiting for an operator.
    bool                mbStopAtSep;        /// True = Stop subexpression creation at an ocSep token.
    bool                mbVolatile;         /// True = Formula contains volatile function.
    bool                mbOk;               /// Current state of the compiler.

    explicit            XclExpCompData( const XclExpCompConfig* pCfg );
};

XclExpCompData::XclExpCompData( const XclExpCompConfig* pCfg ) :
    mrCfg( pCfg ? *pCfg : spConfigTable[ 0 ] ),
    mpLinkMgr( nullptr ),
    mpRefLog( nullptr ),
    mpScBasePos( nullptr ),
    mbStopAtSep( false ),
    mbVolatile( false ),
    mbOk( pCfg != nullptr )
{
    OSL_ENSURE( pCfg, "XclExpFmlaCompImpl::Init - unknown formula type" );
}

} // namespace

/** Implementation class of the export formula compiler. */
class XclExpFmlaCompImpl : protected XclExpRoot, protected XclTokenArrayHelper
{
public:
    explicit            XclExpFmlaCompImpl( const XclExpRoot& rRoot );

    /** Creates an Excel token array from the passed Calc token array. */
    XclTokenArrayRef    CreateFormula(
                            XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos = nullptr, XclExpRefLog* pRefLog = nullptr );
    /** Creates a single error token containing the passed error code. */
    XclTokenArrayRef    CreateErrorFormula( sal_uInt8 nErrCode );
    /** Creates a single token for a special cell reference. */
    XclTokenArrayRef    CreateSpecialRefFormula( sal_uInt8 nTokenId, const XclAddress& rXclPos );
    /** Creates a single tNameXR token for a reference to an external name. */
    XclTokenArrayRef    CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName );

    /** Returns true, if the passed formula type allows 3D references only. */
    bool                Is3DRefOnly( XclFormulaType eType ) const;

    bool IsRef2D( const ScSingleRefData& rRefData, bool bCheck3DFlag ) const;
    bool IsRef2D( const ScComplexRefData& rRefData, bool bCheck3DFlag ) const;

private:
    const XclExpCompConfig* GetConfigForType( XclFormulaType eType ) const;
    inline sal_uInt16   GetSize() const { return static_cast< sal_uInt16 >( mxData->maTokVec.size() ); }

    void                Init( XclFormulaType eType );
    void                Init( XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos, XclExpRefLog* pRefLog );

    void                RecalcTokenClasses();
    void                RecalcTokenClass( const XclExpTokenConvInfo& rConvInfo, XclFuncParamConv ePrevConv, XclExpClassConv ePrevClassConv, bool bWasRefClass );

    void                FinalizeFormula();
    XclTokenArrayRef    CreateTokenArray();

    // compiler ---------------------------------------------------------------
    // XclExpScToken: pass-by-value and return-by-value is intended

    const FormulaToken* GetNextRawToken();
    const FormulaToken* PeekNextRawToken( bool bSkipSpaces ) const;

    bool                GetNextToken( XclExpScToken& rTokData );
    XclExpScToken       GetNextToken();

    XclExpScToken       Expression( XclExpScToken aTokData, bool bInParentheses, bool bStopAtSep );
    XclExpScToken       SkipExpression( XclExpScToken aTokData, bool bStopAtSep );

    XclExpScToken       OrTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       AndTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       CompareTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       ConcatTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       AddSubTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       MulDivTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       PowTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       UnaryPostTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       UnaryPreTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       ListTerm( XclExpScToken aTokData, bool bInParentheses );
    XclExpScToken       IntersectTerm( XclExpScToken aTokData, bool& rbHasRefOp );
    XclExpScToken       RangeTerm( XclExpScToken aTokData, bool& rbHasRefOp );
    XclExpScToken       Factor( XclExpScToken aTokData );

    // formula structure ------------------------------------------------------

    void                ProcessDouble( const XclExpScToken& rTokData );
    void                ProcessString( const XclExpScToken& rTokData );
    void                ProcessMissing( const XclExpScToken& rTokData );
    void                ProcessBad( const XclExpScToken& rTokData );
    void                ProcessParentheses( const XclExpScToken& rTokData );
    void                ProcessBoolean( const XclExpScToken& rTokData );
    void                ProcessDdeLink( const XclExpScToken& rTokData );
    void                ProcessExternal( const XclExpScToken& rTokData );
    void                ProcessMatrix( const XclExpScToken& rTokData );

    void                ProcessFunction( const XclExpScToken& rTokData );
    void                PrepareFunction( XclExpFuncData& rFuncData );
    void                FinishFunction( XclExpFuncData& rFuncData, sal_uInt8 nCloseSpaces );
    void                FinishIfFunction( XclExpFuncData& rFuncData );
    void                FinishChooseFunction( XclExpFuncData& rFuncData );

    XclExpScToken       ProcessParam( XclExpScToken aTokData, XclExpFuncData& rFuncData );
    void                PrepareParam( XclExpFuncData& rFuncData );
    void                FinishParam( XclExpFuncData& rFuncData );
    void                AppendDefaultParam( XclExpFuncData& rFuncData );
    void                AppendTrailingParam( XclExpFuncData& rFuncData );

    // reference handling -----------------------------------------------------

    SCTAB               GetScTab( const ScSingleRefData& rRefData ) const;

    void                ConvertRefData( ScSingleRefData& rRefData, XclAddress& rXclPos,
                            bool bNatLangRef, bool bTruncMaxCol, bool bTruncMaxRow ) const;
    void                ConvertRefData( ScComplexRefData& rRefData, XclRange& rXclRange,
                            bool bNatLangRef ) const;

    XclExpRefLogEntry*  GetNewRefLogEntry();
    void                ProcessCellRef( const XclExpScToken& rTokData );
    void                ProcessRangeRef( const XclExpScToken& rTokData );
    void                ProcessExternalCellRef( const XclExpScToken& rTokData );
    void                ProcessExternalRangeRef( const XclExpScToken& rTokData );
    void                ProcessDefinedName( const XclExpScToken& rTokData );
    void                ProcessExternalName( const XclExpScToken& rTokData );

    // token vector -----------------------------------------------------------

    void                PushOperandPos( sal_uInt16 nTokPos );
    void                PushOperatorPos( sal_uInt16 nTokPos, const XclExpOperandListRef& rxOperands );
    sal_uInt16          PopOperandPos();

    void                Append( sal_uInt8 nData );
    void                Append( sal_uInt8 nData, size_t nCount );
    void                Append( sal_uInt16 nData );
    void                Append( sal_uInt32 nData );
    void                Append( double fData );
    void                Append( const OUString& rString );

    void                AppendAddress( const XclAddress& rXclPos );
    void                AppendRange( const XclRange& rXclRange );

    void                AppendSpaceToken( sal_uInt8 nType, sal_uInt8 nCount );

    void                AppendOperandTokenId( sal_uInt8 nTokenId, sal_uInt8 nSpaces = 0 );
    void                AppendIntToken( sal_uInt16 nValue, sal_uInt8 nSpaces = 0 );
    void                AppendNumToken( double fValue, sal_uInt8 nSpaces = 0 );
    void                AppendBoolToken( bool bValue, sal_uInt8 nSpaces = 0 );
    void                AppendErrorToken( sal_uInt8 nErrCode, sal_uInt8 nSpaces = 0 );
    void                AppendMissingToken( sal_uInt8 nSpaces = 0 );
    void                AppendNameToken( sal_uInt16 nNameIdx, sal_uInt8 nSpaces = 0 );
    void                AppendMissingNameToken( const OUString& rName, sal_uInt8 nSpaces = 0 );
    void                AppendNameXToken( sal_uInt16 nExtSheet, sal_uInt16 nExtName, sal_uInt8 nSpaces = 0 );
    void                AppendMacroCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces = 0 );
    void                AppendAddInCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces = 0 );
    void                AppendEuroToolCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces = 0 );

    void                AppendOperatorTokenId( sal_uInt8 nTokenId, const XclExpOperandListRef& rxOperands, sal_uInt8 nSpaces = 0 );
    void                AppendUnaryOperatorToken( sal_uInt8 nTokenId, sal_uInt8 nSpaces = 0 );
    void                AppendBinaryOperatorToken( sal_uInt8 nTokenId, bool bValType, sal_uInt8 nSpaces = 0 );
    void                AppendLogicalOperatorToken( sal_uInt16 nXclFuncIdx, sal_uInt8 nOpCount );
    void                AppendFuncToken( const XclExpFuncData& rFuncData );

    void                AppendParenToken( sal_uInt8 nOpenSpaces = 0, sal_uInt8 nCloseSpaces = 0 );
    void                AppendJumpToken( XclExpFuncData& rFuncData, sal_uInt8 nAttrType );

    void                InsertZeros( sal_uInt16 nInsertPos, sal_uInt16 nInsertSize );
    void                Overwrite( sal_uInt16 nWriteToPos, sal_uInt16 nOffset );

    void                UpdateAttrGoto( sal_uInt16 nAttrPos );

    bool                IsSpaceToken( sal_uInt16 nPos ) const;
    void                RemoveTrailingParen();

    void                AppendExt( sal_uInt8 nData );
    void                AppendExt( sal_uInt8 nData, size_t nCount );
    void                AppendExt( sal_uInt16 nData );
    void                AppendExt( double fData );
    void                AppendExt( const OUString& rString );

private:
    typedef std::map< XclFormulaType, XclExpCompConfig >  XclExpCompConfigMap;
    typedef std::shared_ptr< XclExpCompData >             XclExpCompDataRef;
    typedef std::vector< XclExpCompDataRef >              XclExpCompDataVector;

    XclExpCompConfigMap maCfgMap;       /// Compiler configuration map for all formula types.
    XclFunctionProvider maFuncProv;     /// Excel function data provider.
    XclExpCompDataRef   mxData;         /// Working data for current formula.
    XclExpCompDataVector maDataStack;   /// Stack for working data, when compiler is called recursively.
    const XclBiff       meBiff;         /// Cached BIFF version to save GetBiff() calls.
    const SCsCOL        mnMaxAbsCol;    /// Maximum column index.
    const SCsROW        mnMaxAbsRow;    /// Maximum row index.
    const SCsCOL        mnMaxScCol;     /// Maximum column index in Calc itself.
    const SCsROW        mnMaxScRow;     /// Maximum row index in Calc itself.
    const sal_uInt16    mnMaxColMask;   /// Mask to delete invalid bits in column fields.
    const sal_uInt32    mnMaxRowMask;   /// Mask to delete invalid bits in row fields.
};

XclExpFmlaCompImpl::XclExpFmlaCompImpl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maFuncProv( rRoot ),
    meBiff( rRoot.GetBiff() ),
    mnMaxAbsCol( static_cast< SCsCOL >( rRoot.GetXclMaxPos().Col() ) ),
    mnMaxAbsRow( static_cast< SCsROW >( rRoot.GetXclMaxPos().Row() ) ),
    mnMaxScCol( static_cast< SCsCOL >( rRoot.GetScMaxPos().Col() ) ),
    mnMaxScRow( static_cast< SCsROW >( rRoot.GetScMaxPos().Row() ) ),
    mnMaxColMask( static_cast< sal_uInt16 >( rRoot.GetXclMaxPos().Col() ) ),
    mnMaxRowMask( static_cast< sal_uInt32 >( rRoot.GetXclMaxPos().Row() ) )
{
    // build the configuration map
    for( const XclExpCompConfig* pEntry = spConfigTable; pEntry != STATIC_ARRAY_END( spConfigTable ); ++pEntry )
        maCfgMap[ pEntry->meType ] = *pEntry;
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateFormula( XclFormulaType eType,
        const ScTokenArray& rScTokArr, const ScAddress* pScBasePos, XclExpRefLog* pRefLog )
{
    // initialize the compiler
    Init( eType, rScTokArr, pScBasePos, pRefLog );

    // start compilation, if initialization didn't fail
    if( mxData->mbOk )
    {
        XclExpScToken aTokData( GetNextToken() );
        sal_uInt16 nScError = rScTokArr.GetCodeError();
        if( (nScError != 0) && (!aTokData.Is() || (aTokData.GetOpCode() == ocStop)) )
        {
            // #i50253# convert simple ocStop token to error code formula (e.g. =#VALUE!)
            AppendErrorToken( XclTools::GetXclErrorCode( nScError ), aTokData.mnSpaces );
        }
        else if( aTokData.Is() )
        {
            aTokData = Expression( aTokData, false, false );
        }
        else
        {
            OSL_FAIL( "XclExpFmlaCompImpl::CreateFormula - empty token array" );
            mxData->mbOk = false;
        }

        if( mxData->mbOk )
        {
            // #i44907# auto-generated SUBTOTAL formula cells have trailing ocStop token
            mxData->mbOk = !aTokData.Is() || (aTokData.GetOpCode() == ocStop);
            OSL_ENSURE( mxData->mbOk, "XclExpFmlaCompImpl::CreateFormula - unknown garbage behind formula" );
        }
    }

    // finalize (add tAttrVolatile token, calculate all token classes)
    RecalcTokenClasses();
    FinalizeFormula();

    // leave recursive call, create and return the final token array
    return CreateTokenArray();
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateErrorFormula( sal_uInt8 nErrCode )
{
    Init( EXC_FMLATYPE_NAME );
    AppendErrorToken( nErrCode );
    return CreateTokenArray();
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateSpecialRefFormula( sal_uInt8 nTokenId, const XclAddress& rXclPos )
{
    Init( EXC_FMLATYPE_NAME );
    AppendOperandTokenId( nTokenId );
    Append( static_cast<sal_uInt16>(rXclPos.mnRow) );
    Append( rXclPos.mnCol );    // do not use AppendAddress(), we always need 16-bit column here
    return CreateTokenArray();
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName )
{
    Init( EXC_FMLATYPE_NAME );
    AppendNameXToken( nExtSheet, nExtName );
    return CreateTokenArray();
}

bool XclExpFmlaCompImpl::Is3DRefOnly( XclFormulaType eType ) const
{
    const XclExpCompConfig* pCfg = GetConfigForType( eType );
    return pCfg && pCfg->mb3DRefOnly;
}

// private --------------------------------------------------------------------

const XclExpCompConfig* XclExpFmlaCompImpl::GetConfigForType( XclFormulaType eType ) const
{
    XclExpCompConfigMap::const_iterator aIt = maCfgMap.find( eType );
    OSL_ENSURE( aIt != maCfgMap.end(), "XclExpFmlaCompImpl::GetConfigForType - unknown formula type" );
    return (aIt == maCfgMap.end()) ? nullptr : &aIt->second;
}

void XclExpFmlaCompImpl::Init( XclFormulaType eType )
{
    // compiler invoked recursively? - store old working data
    if( mxData.get() )
        maDataStack.push_back( mxData );
    // new compiler working data structure
    mxData.reset( new XclExpCompData( GetConfigForType( eType ) ) );
}

void XclExpFmlaCompImpl::Init( XclFormulaType eType, const ScTokenArray& rScTokArr,
        const ScAddress* pScBasePos, XclExpRefLog* pRefLog )
{
    // common initialization
    Init( eType );

    // special initialization
    if( mxData->mbOk ) switch( mxData->mrCfg.meType )
    {
        case EXC_FMLATYPE_CELL:
        case EXC_FMLATYPE_MATRIX:
        case EXC_FMLATYPE_CHART:
            mxData->mbOk = pScBasePos != nullptr;
            OSL_ENSURE( mxData->mbOk, "XclExpFmlaCompImpl::Init - missing cell address" );
            mxData->mpScBasePos = pScBasePos;
        break;
        case EXC_FMLATYPE_SHARED:
            mxData->mbOk = pScBasePos != nullptr;
            assert(mxData->mbOk && "XclExpFmlaCompImpl::Init - missing cell address");
            if (mxData->mbOk)
            {
                // clone the passed token array, convert references relative to current cell position
                mxData->mxOwnScTokArr.reset( rScTokArr.Clone() );
                ScCompiler::MoveRelWrap( *mxData->mxOwnScTokArr, &GetDocRef(), *pScBasePos, MAXCOL, MAXROW );
                // don't remember pScBasePos in mxData->mpScBasePos, shared formulas use real relative refs
            }
        break;
        default:;
    }

    if( mxData->mbOk )
    {
        // link manager to be used
        mxData->mpLinkMgr = mxData->mrCfg.mbLocalLinkMgr ? &GetLocalLinkManager() : &GetGlobalLinkManager();

        // token array iterator (use cloned token array if present)
        mxData->maTokArrIt.Init( mxData->mxOwnScTokArr ? *mxData->mxOwnScTokArr : rScTokArr, false );
        mxData->mpRefLog = pRefLog;
    }
}

void XclExpFmlaCompImpl::RecalcTokenClasses()
{
    if( mxData->mbOk )
    {
        mxData->mbOk = mxData->maOpPosStack.size() == 1;
        OSL_ENSURE( mxData->mbOk, "XclExpFmlaCompImpl::RecalcTokenClasses - position of root token expected on stack" );
        if( mxData->mbOk )
        {
            /*  Cell and array formulas start with VAL conversion and VALTYPE
                parameter type, defined names start with ARR conversion and
                REFTYPE parameter type for the root token. */
            XclExpOperandList aOperands;
            bool bNameFmla = mxData->mrCfg.meClassType == EXC_CLASSTYPE_NAME;
            XclFuncParamConv eParamConv = bNameFmla ? EXC_PARAMCONV_ARR : EXC_PARAMCONV_VAL;
            XclExpClassConv eClassConv = bNameFmla ? EXC_CLASSCONV_ARR : EXC_CLASSCONV_VAL;
            XclExpTokenConvInfo aConvInfo = { PopOperandPos(), eParamConv, !bNameFmla };
            RecalcTokenClass( aConvInfo, eParamConv, eClassConv, bNameFmla );
        }

        // clear operand vectors (calls to the expensive InsertZeros() may follow)
        mxData->maOpListVec.clear();
        mxData->maOpPosStack.clear();
    }
}

void XclExpFmlaCompImpl::RecalcTokenClass( const XclExpTokenConvInfo& rConvInfo,
        XclFuncParamConv ePrevConv, XclExpClassConv ePrevClassConv, bool bWasRefClass )
{
    OSL_ENSURE( rConvInfo.mnTokPos < GetSize(), "XclExpFmlaCompImpl::RecalcTokenClass - invalid token position" );
    sal_uInt8& rnTokenId = mxData->maTokVec[ rConvInfo.mnTokPos ];
    sal_uInt8 nTokClass = GetTokenClass( rnTokenId );

    // REF tokens in VALTYPE parameters behave like VAL tokens
    if( rConvInfo.mbValType && (nTokClass == EXC_TOKCLASS_REF) )
        ChangeTokenClass( rnTokenId, nTokClass = EXC_TOKCLASS_VAL );

    // replace RPO conversion of operator with parent conversion
    XclFuncParamConv eConv = (rConvInfo.meConv == EXC_PARAMCONV_RPO) ? ePrevConv : rConvInfo.meConv;

    // find the effective token class conversion to be performed for this token
    XclExpClassConv eClassConv = EXC_CLASSCONV_ORG;
    switch( eConv )
    {
        case EXC_PARAMCONV_ORG:
            // conversion is forced independent of parent conversion
            eClassConv = EXC_CLASSCONV_ORG;
        break;
        case EXC_PARAMCONV_VAL:
            // conversion is forced independent of parent conversion
            eClassConv = EXC_CLASSCONV_VAL;
        break;
        case EXC_PARAMCONV_ARR:
            // conversion is forced independent of parent conversion
            eClassConv = EXC_CLASSCONV_ARR;
        break;
        case EXC_PARAMCONV_RPT:
            switch( ePrevConv )
            {
                case EXC_PARAMCONV_ORG:
                case EXC_PARAMCONV_VAL:
                case EXC_PARAMCONV_ARR:
                    /*  If parent token has REF class (REF token in REFTYPE
                        function parameter), then RPT does not repeat the
                        previous explicit ORG or ARR conversion, but always
                        falls back to VAL conversion. */
                    eClassConv = bWasRefClass ? EXC_CLASSCONV_VAL : ePrevClassConv;
                break;
                case EXC_PARAMCONV_RPT:
                    // nested RPT repeats the previous effective conversion
                    eClassConv = ePrevClassConv;
                break;
                case EXC_PARAMCONV_RPX:
                    /*  If parent token has REF class (REF token in REFTYPE
                        function parameter), then RPX repeats the previous
                        effective conversion (which will be either ORG or ARR,
                        but never VAL), otherwise falls back to ORG conversion. */
                    eClassConv = bWasRefClass ? ePrevClassConv : EXC_CLASSCONV_ORG;
                break;
                case EXC_PARAMCONV_RPO: // does not occur
                break;
            }
        break;
        case EXC_PARAMCONV_RPX:
            /*  If current token still has REF class, set previous effective
                conversion as current conversion. This will not have an effect
                on the REF token but is needed for RPT parameters of this
                function that want to repeat this conversion type. If current
                token is VAL or ARR class, the previous ARR conversion will be
                repeated on the token, but VAL conversion will not. */
            eClassConv = ((nTokClass == EXC_TOKCLASS_REF) || (ePrevClassConv == EXC_CLASSCONV_ARR)) ?
                ePrevClassConv : EXC_CLASSCONV_ORG;
        break;
        case EXC_PARAMCONV_RPO: // does not occur (see above)
        break;
    }

    // do the token class conversion
    switch( eClassConv )
    {
        case EXC_CLASSCONV_ORG:
            /*  Cell formulas: leave the current token class. Cell formulas
                are the only type of formulas where all tokens can keep
                their original token class.
                Array and defined name formulas: convert VAL to ARR. */
            if( (mxData->mrCfg.meClassType != EXC_CLASSTYPE_CELL) && (nTokClass == EXC_TOKCLASS_VAL) )
                ChangeTokenClass( rnTokenId, nTokClass = EXC_TOKCLASS_ARR );
        break;
        case EXC_CLASSCONV_VAL:
            // convert ARR to VAL
            if( nTokClass == EXC_TOKCLASS_ARR )
                ChangeTokenClass( rnTokenId, nTokClass = EXC_TOKCLASS_VAL );
        break;
        case EXC_CLASSCONV_ARR:
            // convert VAL to ARR
            if( nTokClass == EXC_TOKCLASS_VAL )
                ChangeTokenClass( rnTokenId, nTokClass = EXC_TOKCLASS_ARR );
        break;
    }

    // do conversion for nested operands, if token is an operator or function
    if( rConvInfo.mnTokPos < mxData->maOpListVec.size() )
        if( const XclExpOperandList* pOperands = mxData->maOpListVec[ rConvInfo.mnTokPos ].get() )
            for( XclExpOperandList::const_iterator aIt = pOperands->begin(), aEnd = pOperands->end(); aIt != aEnd; ++aIt )
                RecalcTokenClass( *aIt, eConv, eClassConv, nTokClass == EXC_TOKCLASS_REF );
}

void XclExpFmlaCompImpl::FinalizeFormula()
{
    if( mxData->mbOk )
    {
        // Volatile? Add a tAttrVolatile token at the beginning of the token array.
        if( mxData->mbVolatile )
        {
            // tAttrSpace token can be extended with volatile flag
            if( !IsSpaceToken( 0 ) )
            {
                InsertZeros( 0, 4 );
                mxData->maTokVec[ 0 ] = EXC_TOKID_ATTR;
            }
            mxData->maTokVec[ 1 ] |= EXC_TOK_ATTR_VOLATILE;
        }

        // Token array too long? -> error
        mxData->mbOk = mxData->maTokVec.size() <= EXC_TOKARR_MAXLEN;
    }

    if( !mxData->mbOk )
    {
        // Any unrecoverable error? -> Create a =#NA formula.
        mxData->maTokVec.clear();
        mxData->maExtDataVec.clear();
        mxData->mbVolatile = false;
        AppendErrorToken( EXC_ERR_NA );
    }
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateTokenArray()
{
    // create the Excel token array from working data before resetting mxData
    OSL_ENSURE( mxData->mrCfg.mbAllowArrays || mxData->maExtDataVec.empty(), "XclExpFmlaCompImpl::CreateTokenArray - unexpected extended data" );
    if( !mxData->mrCfg.mbAllowArrays )
        mxData->maExtDataVec.clear();
    XclTokenArrayRef xTokArr( new XclTokenArray( mxData->maTokVec, mxData->maExtDataVec, mxData->mbVolatile ) );
    mxData.reset();

    // compiler invoked recursively? - restore old working data
    if( !maDataStack.empty() )
    {
        mxData = maDataStack.back();
        maDataStack.pop_back();
    }

    return xTokArr;
}

// compiler -------------------------------------------------------------------

const FormulaToken* XclExpFmlaCompImpl::GetNextRawToken()
{
    const FormulaToken* pScToken = mxData->maTokArrIt.Get();
    ++mxData->maTokArrIt;
    return pScToken;
}

const FormulaToken* XclExpFmlaCompImpl::PeekNextRawToken( bool bSkipSpaces ) const
{
    /*  Returns pointer to next raw token in the token array. The token array
        iterator already points to the next token (A call to GetNextToken()
        always increases the iterator), so this function just returns the token
        the iterator points to. To skip space tokens, a copy of the iterator is
        created and set to the passed skip-spaces mode. If spaces have to be
        skipped, and the iterator currently points to a space token, the
        constructor will move it to the next non-space token. */
    XclTokenArrayIterator aTempIt( mxData->maTokArrIt, bSkipSpaces );
    return aTempIt.Get();
}

bool XclExpFmlaCompImpl::GetNextToken( XclExpScToken& rTokData )
{
    rTokData.mpScToken = GetNextRawToken();
    rTokData.mnSpaces = (rTokData.GetOpCode() == ocSpaces) ? rTokData.mpScToken->GetByte() : 0;
    while( rTokData.GetOpCode() == ocSpaces )
        rTokData.mpScToken = GetNextRawToken();
    return rTokData.Is();
}

XclExpScToken XclExpFmlaCompImpl::GetNextToken()
{
    XclExpScToken aTokData;
    GetNextToken( aTokData );
    return aTokData;
}

namespace {

/** Returns the Excel token ID of a comparison operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetCompareTokenId( OpCode eOpCode )
{
    switch( eOpCode )
    {
        case ocLess:            return EXC_TOKID_LT;
        case ocLessEqual:       return EXC_TOKID_LE;
        case ocEqual:           return EXC_TOKID_EQ;
        case ocGreaterEqual:    return EXC_TOKID_GE;
        case ocGreater:         return EXC_TOKID_GT;
        case ocNotEqual:        return EXC_TOKID_NE;
        default:;
    }
    return EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a string concatenation operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetConcatTokenId( OpCode eOpCode )
{
    return (eOpCode == ocAmpersand) ? EXC_TOKID_CONCAT : EXC_TOKID_NONE;
}

/** Returns the Excel token ID of an addition/subtraction operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetAddSubTokenId( OpCode eOpCode )
{
    switch( eOpCode )
    {
        case ocAdd:     return EXC_TOKID_ADD;
        case ocSub:     return EXC_TOKID_SUB;
        default:;
    }
    return EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a multiplication/division operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetMulDivTokenId( OpCode eOpCode )
{
    switch( eOpCode )
    {
        case ocMul:     return EXC_TOKID_MUL;
        case ocDiv:     return EXC_TOKID_DIV;
        default:;
    }
    return EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a power operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetPowTokenId( OpCode eOpCode )
{
    return (eOpCode == ocPow) ? EXC_TOKID_POWER : EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a trailing unary operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetUnaryPostTokenId( OpCode eOpCode )
{
    return (eOpCode == ocPercentSign) ? EXC_TOKID_PERCENT : EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a leading unary operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetUnaryPreTokenId( OpCode eOpCode )
{
    switch( eOpCode )
    {
        case ocAdd:     return EXC_TOKID_UPLUS;     // +(1)
        case ocNeg:     return EXC_TOKID_UMINUS;    // NEG(1)
        case ocNegSub:  return EXC_TOKID_UMINUS;    // -(1)
        default:;
    }
    return EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a reference list operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetListTokenId( OpCode eOpCode, bool bStopAtSep )
{
    return ((eOpCode == ocUnion) || (!bStopAtSep && (eOpCode == ocSep))) ? EXC_TOKID_LIST : EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a reference intersection operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetIntersectTokenId( OpCode eOpCode )
{
    return (eOpCode == ocIntersect) ? EXC_TOKID_ISECT : EXC_TOKID_NONE;
}

/** Returns the Excel token ID of a reference range operator or EXC_TOKID_NONE. */
inline sal_uInt8 lclGetRangeTokenId( OpCode eOpCode )
{
    return (eOpCode == ocRange) ? EXC_TOKID_RANGE : EXC_TOKID_NONE;
}

} // namespace

XclExpScToken XclExpFmlaCompImpl::Expression( XclExpScToken aTokData, bool bInParentheses, bool bStopAtSep )
{
    if( mxData->mbOk && aTokData.Is() )
    {
        // remember old stop-at-ocSep mode, restored below
        bool bOldStopAtSep = mxData->mbStopAtSep;
        mxData->mbStopAtSep = bStopAtSep;
        // start compilation of the subexpression
        aTokData = OrTerm( aTokData, bInParentheses );
        // restore old stop-at-ocSep mode
        mxData->mbStopAtSep = bOldStopAtSep;
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::SkipExpression( XclExpScToken aTokData, bool bStopAtSep )
{
    while( mxData->mbOk && aTokData.Is() && (aTokData.GetOpCode() != ocClose) && (!bStopAtSep || (aTokData.GetOpCode() != ocSep)) )
    {
        if( aTokData.GetOpCode() == ocOpen )
        {
            aTokData = SkipExpression( GetNextToken(), false );
            if( mxData->mbOk ) mxData->mbOk = aTokData.GetOpCode() == ocClose;
        }
        aTokData = GetNextToken();
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::OrTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = AndTerm( aTokData, bInParentheses );
    sal_uInt8 nParamCount = 1;
    while( mxData->mbOk && (aTokData.GetOpCode() == ocOr) )
    {
        RemoveTrailingParen();
        aTokData = AndTerm( GetNextToken(), bInParentheses );
        RemoveTrailingParen();
        ++nParamCount;
        if( mxData->mbOk ) mxData->mbOk = nParamCount <= EXC_FUNC_MAXPARAM;
    }
    if( mxData->mbOk && (nParamCount > 1) )
        AppendLogicalOperatorToken( EXC_FUNCID_OR, nParamCount );
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::AndTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = CompareTerm( aTokData, bInParentheses );
    sal_uInt8 nParamCount = 1;
    while( mxData->mbOk && (aTokData.GetOpCode() == ocAnd) )
    {
        RemoveTrailingParen();
        aTokData = CompareTerm( GetNextToken(), bInParentheses );
        RemoveTrailingParen();
        ++nParamCount;
        if( mxData->mbOk ) mxData->mbOk = nParamCount <= EXC_FUNC_MAXPARAM;
    }
    if( mxData->mbOk && (nParamCount > 1) )
        AppendLogicalOperatorToken( EXC_FUNCID_AND, nParamCount );
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::CompareTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = ConcatTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetCompareTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = ConcatTerm( GetNextToken(), bInParentheses );
        AppendBinaryOperatorToken( nOpTokenId, true, nSpaces );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::ConcatTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = AddSubTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetConcatTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = AddSubTerm( GetNextToken(), bInParentheses );
        AppendBinaryOperatorToken( nOpTokenId, true, nSpaces );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::AddSubTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = MulDivTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetAddSubTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = MulDivTerm( GetNextToken(), bInParentheses );
        AppendBinaryOperatorToken( nOpTokenId, true, nSpaces );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::MulDivTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = PowTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetMulDivTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = PowTerm( GetNextToken(), bInParentheses );
        AppendBinaryOperatorToken( nOpTokenId, true, nSpaces );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::PowTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = UnaryPostTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetPowTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = UnaryPostTerm( GetNextToken(), bInParentheses );
        AppendBinaryOperatorToken( nOpTokenId, true, nSpaces );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::UnaryPostTerm( XclExpScToken aTokData, bool bInParentheses )
{
    aTokData = UnaryPreTerm( aTokData, bInParentheses );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetUnaryPostTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AppendUnaryOperatorToken( nOpTokenId, aTokData.mnSpaces );
        GetNextToken( aTokData );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::UnaryPreTerm( XclExpScToken aTokData, bool bInParentheses )
{
    sal_uInt8 nOpTokenId = mxData->mbOk ? lclGetUnaryPreTokenId( aTokData.GetOpCode() ) : EXC_TOKID_NONE;
    if( nOpTokenId != EXC_TOKID_NONE )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = UnaryPreTerm( GetNextToken(), bInParentheses );
        AppendUnaryOperatorToken( nOpTokenId, nSpaces );
    }
    else
    {
        aTokData = ListTerm( aTokData, bInParentheses );
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::ListTerm( XclExpScToken aTokData, bool bInParentheses )
{
    sal_uInt16 nSubExprPos = GetSize();
    bool bHasAnyRefOp = false;
    bool bHasListOp = false;
    aTokData = IntersectTerm( aTokData, bHasAnyRefOp );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetListTokenId( aTokData.GetOpCode(), mxData->mbStopAtSep )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = IntersectTerm( GetNextToken(), bHasAnyRefOp );
        AppendBinaryOperatorToken( nOpTokenId, false, nSpaces );
        bHasAnyRefOp = bHasListOp = true;
    }
    if( bHasAnyRefOp )
    {
        // add a tMemFunc token enclosing the entire reference subexpression
        sal_uInt16 nSubExprSize = GetSize() - nSubExprPos;
        InsertZeros( nSubExprPos, 3 );
        mxData->maTokVec[ nSubExprPos ] = GetTokenId( EXC_TOKID_MEMFUNC, EXC_TOKCLASS_REF );
        Overwrite( nSubExprPos + 1, nSubExprSize );
        // update the operand/operator stack (set the list expression as operand of the tMemFunc)
        XclExpOperandListRef xOperands( new XclExpOperandList );
        xOperands->AppendOperand( PopOperandPos(), EXC_PARAMCONV_VAL, false );
        PushOperatorPos( nSubExprPos, xOperands );
    }
    // #i86439# enclose list operator into parentheses, e.g. Calc's =AREAS(A1~A2) to Excel's =AREAS((A1;A2))
    if( bHasListOp && !bInParentheses )
        AppendParenToken();
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::IntersectTerm( XclExpScToken aTokData, bool& rbHasRefOp )
{
    aTokData = RangeTerm( aTokData, rbHasRefOp );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetIntersectTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = RangeTerm( GetNextToken(), rbHasRefOp );
        AppendBinaryOperatorToken( nOpTokenId, false, nSpaces );
        rbHasRefOp = true;
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::RangeTerm( XclExpScToken aTokData, bool& rbHasRefOp )
{
    aTokData = Factor( aTokData );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mxData->mbOk && ((nOpTokenId = lclGetRangeTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = Factor( GetNextToken() );
        AppendBinaryOperatorToken( nOpTokenId, false, nSpaces );
        rbHasRefOp = true;
    }
    return aTokData;
}

XclExpScToken XclExpFmlaCompImpl::Factor( XclExpScToken aTokData )
{
    if( !mxData->mbOk || !aTokData.Is() ) return XclExpScToken();

    switch( aTokData.GetType() )
    {
        case svUnknown:             mxData->mbOk = false;                   break;
        case svDouble:              ProcessDouble( aTokData );              break;
        case svString:              ProcessString( aTokData );              break;
        case svSingleRef:           ProcessCellRef( aTokData );             break;
        case svDoubleRef:           ProcessRangeRef( aTokData );            break;
        case svExternalSingleRef:   ProcessExternalCellRef( aTokData );     break;
        case svExternalDoubleRef:   ProcessExternalRangeRef( aTokData );    break;
        case svExternalName:        ProcessExternalName( aTokData );        break;
        case svMatrix:              ProcessMatrix( aTokData );              break;
        case svExternal:            ProcessExternal( aTokData );            break;

        default: switch( aTokData.GetOpCode() )
        {
            case ocNone:        /* do nothing */                    break;
            case ocMissing:     ProcessMissing( aTokData );         break;
            case ocBad:         ProcessBad( aTokData );             break;
            case ocOpen:        ProcessParentheses( aTokData );     break;
            case ocName:        ProcessDefinedName( aTokData );     break;
            case ocFalse:
            case ocTrue:        ProcessBoolean( aTokData );         break;
            case ocDde:         ProcessDdeLink( aTokData );         break;
            default:            ProcessFunction( aTokData );
        }
    }

    return GetNextToken();
}

// formula structure ----------------------------------------------------------

void XclExpFmlaCompImpl::ProcessDouble( const XclExpScToken& rTokData )
{
    double fValue = rTokData.mpScToken->GetDouble();
    double fInt;
    double fFrac = modf( fValue, &fInt );
    if( (fFrac == 0.0) && (0.0 <= fInt) && (fInt <= 65535.0) )
        AppendIntToken( static_cast< sal_uInt16 >( fInt ), rTokData.mnSpaces );
    else
        AppendNumToken( fValue, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessString( const XclExpScToken& rTokData )
{
    AppendOperandTokenId( EXC_TOKID_STR, rTokData.mnSpaces );
    Append( rTokData.mpScToken->GetString().getString() );
}

void XclExpFmlaCompImpl::ProcessMissing( const XclExpScToken& rTokData )
{
    AppendMissingToken( rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessBad( const XclExpScToken& rTokData )
{
    AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessParentheses( const XclExpScToken& rTokData )
{
    XclExpScToken aTokData = Expression( GetNextToken(), true, false );
    mxData->mbOk = aTokData.GetOpCode() == ocClose;
    AppendParenToken( rTokData.mnSpaces, aTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessBoolean( const XclExpScToken& rTokData )
{
    mxData->mbOk = GetNextToken().GetOpCode() == ocOpen;
    if( mxData->mbOk ) mxData->mbOk = GetNextToken().GetOpCode() == ocClose;
    if( mxData->mbOk )
        AppendBoolToken( rTokData.GetOpCode() == ocTrue, rTokData.mnSpaces );
}

namespace {

inline bool lclGetTokenString( OUString& rString, const XclExpScToken& rTokData )
{
    bool bIsStr = (rTokData.GetType() == svString) && (rTokData.GetOpCode() == ocPush);
    if( bIsStr )
        rString = rTokData.mpScToken->GetString().getString();
    return bIsStr;
}

} // namespace

void XclExpFmlaCompImpl::ProcessDdeLink( const XclExpScToken& rTokData )
{
    OUString aApplic, aTopic, aItem;

    mxData->mbOk = GetNextToken().GetOpCode() == ocOpen;
    if( mxData->mbOk ) mxData->mbOk = lclGetTokenString( aApplic, GetNextToken() );
    if( mxData->mbOk ) mxData->mbOk = GetNextToken().GetOpCode() == ocSep;
    if( mxData->mbOk ) mxData->mbOk = lclGetTokenString( aTopic, GetNextToken() );
    if( mxData->mbOk ) mxData->mbOk = GetNextToken().GetOpCode() == ocSep;
    if( mxData->mbOk ) mxData->mbOk = lclGetTokenString( aItem, GetNextToken() );
    if( mxData->mbOk ) mxData->mbOk = GetNextToken().GetOpCode() == ocClose;
    if( mxData->mbOk ) mxData->mbOk = !aApplic.isEmpty() && !aTopic.isEmpty() && !aItem.isEmpty();
    if( mxData->mbOk )
    {
        sal_uInt16 nExtSheet(0), nExtName(0);
        if( mxData->mpLinkMgr && mxData->mpLinkMgr->InsertDde( nExtSheet, nExtName, aApplic, aTopic, aItem ) )
            AppendNameXToken( nExtSheet, nExtName, rTokData.mnSpaces );
        else
            AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessExternal( const XclExpScToken& rTokData )
{
    /*  #i47228# Excel import generates svExternal/ocMacro tokens for invalid
        names and for external/invalid function calls. This function looks for
        the next token in the token array. If it is an opening parenthesis, the
        token is processed as external function call, otherwise as undefined name. */
    const FormulaToken* pNextScToken = PeekNextRawToken( true );
    if( !pNextScToken || (pNextScToken->GetOpCode() != ocOpen) )
        AppendMissingNameToken( rTokData.mpScToken->GetExternal(), rTokData.mnSpaces );
    else
        ProcessFunction( rTokData );
}

void XclExpFmlaCompImpl::ProcessMatrix( const XclExpScToken& rTokData )
{
    const ScMatrix* pMatrix = rTokData.mpScToken->GetMatrix();
    if( pMatrix && mxData->mrCfg.mbAllowArrays )
    {
        SCSIZE nScCols, nScRows;
        pMatrix->GetDimensions( nScCols, nScRows );
        OSL_ENSURE( (nScCols > 0) && (nScRows > 0), "XclExpFmlaCompImpl::ProcessMatrix - invalid matrix size" );
        sal_uInt16 nCols = ::limit_cast< sal_uInt16 >( nScCols, 0, 256 );
        sal_uInt16 nRows = ::limit_cast< sal_uInt16 >( nScRows, 0, 1024 );

        // create the tArray token
        AppendOperandTokenId( GetTokenId( EXC_TOKID_ARRAY, EXC_TOKCLASS_ARR ), rTokData.mnSpaces );
        Append( static_cast< sal_uInt8 >( (meBiff == EXC_BIFF8) ? (nCols - 1) : nCols ) );
        Append( static_cast< sal_uInt16 >( (meBiff == EXC_BIFF8) ? (nRows - 1) : nRows ) );
        Append( static_cast< sal_uInt32 >( 0 ) );

        // create the extended data containing the array values
        AppendExt( static_cast< sal_uInt8 >( (meBiff == EXC_BIFF8) ? (nCols - 1) : nCols ) );
        AppendExt( static_cast< sal_uInt16 >( (meBiff == EXC_BIFF8) ? (nRows - 1) : nRows ) );
        for( SCSIZE nScRow = 0; nScRow < nScRows; ++nScRow )
        {
            for( SCSIZE nScCol = 0; nScCol < nScCols; ++nScCol )
            {
                ScMatrixValue nMatVal = pMatrix->Get( nScCol, nScRow );
                if( ScMatrix::IsValueType( nMatVal.nType ) )    // value, boolean, or error
                {
                    if( ScMatrix::IsBooleanType( nMatVal.nType ) )
                    {
                        AppendExt( EXC_CACHEDVAL_BOOL );
                        AppendExt( static_cast< sal_uInt8 >( nMatVal.GetBoolean() ? 1 : 0 ) );
                        AppendExt( 0, 7 );
                    }
                    else if( sal_uInt16 nErr = nMatVal.GetError() )
                    {
                        AppendExt( EXC_CACHEDVAL_ERROR );
                        AppendExt( XclTools::GetXclErrorCode( nErr ) );
                        AppendExt( 0, 7 );
                    }
                    else
                    {
                        AppendExt( EXC_CACHEDVAL_DOUBLE );
                        AppendExt( nMatVal.fVal );
                    }
                }
                else    // string or empty
                {
                    const OUString aStr( nMatVal.GetString().getString());
                    if( aStr.isEmpty() )
                    {
                        AppendExt( EXC_CACHEDVAL_EMPTY );
                        AppendExt( 0, 8 );
                    }
                    else
                    {
                        AppendExt( EXC_CACHEDVAL_STRING );
                        AppendExt( aStr );
                    }
                }
            }
        }
    }
    else
    {
        // array in places that do not allow it (cond fmts, data validation)
        AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessFunction( const XclExpScToken& rTokData )
{
    OpCode eOpCode = rTokData.GetOpCode();
    const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromOpCode( eOpCode );

    XclExpExtFuncData aExtFuncData;

    // no exportable function found - try to create an external macro call
    if( !pFuncInfo && (eOpCode >= SC_OPCODE_START_NO_PAR) )
    {
        const OUString& rFuncName = ScCompiler::GetNativeSymbol( eOpCode );
        if( !rFuncName.isEmpty() )
        {
            aExtFuncData.Set( rFuncName, true, false );
            pFuncInfo = maFuncProv.GetFuncInfoFromOpCode( ocMacro );
        }
    }

    mxData->mbOk = pFuncInfo != nullptr;
    if( !mxData->mbOk ) return;

    // internal functions equivalent to an existing add-in
    if( pFuncInfo->IsAddInEquivalent() )
        aExtFuncData.Set( pFuncInfo->GetAddInEquivalentFuncName(), true, false );
    // functions simulated by a macro call in file format
    else if( pFuncInfo->IsMacroFunc() )
        aExtFuncData.Set( pFuncInfo->GetMacroFuncName(), false, true );

    XclExpFuncData aFuncData( rTokData, *pFuncInfo, aExtFuncData );
    XclExpScToken aTokData;

    // preparations for special functions, before function processing starts
    PrepareFunction( aFuncData );

    enum { STATE_START, STATE_OPEN, STATE_PARAM, STATE_SEP, STATE_CLOSE, STATE_END }
        eState = STATE_START;
    while( eState != STATE_END ) switch( eState )
    {
        case STATE_START:
            mxData->mbOk = GetNextToken( aTokData ) && (aTokData.GetOpCode() == ocOpen);
            eState = mxData->mbOk ? STATE_OPEN : STATE_END;
        break;
        case STATE_OPEN:
            mxData->mbOk = GetNextToken( aTokData );
            eState = mxData->mbOk ? ((aTokData.GetOpCode() == ocClose) ? STATE_CLOSE : STATE_PARAM) : STATE_END;
        break;
        case STATE_PARAM:
            aTokData = ProcessParam( aTokData, aFuncData );
            switch( aTokData.GetOpCode() )
            {
                case ocSep:     eState = STATE_SEP;                 break;
                case ocClose:   eState = STATE_CLOSE;               break;
                default:        mxData->mbOk = false;
            }
            if( !mxData->mbOk ) eState = STATE_END;
        break;
        case STATE_SEP:
            mxData->mbOk = (aFuncData.GetParamCount() < EXC_FUNC_MAXPARAM) && GetNextToken( aTokData );
            eState = mxData->mbOk ? STATE_PARAM : STATE_END;
        break;
        case STATE_CLOSE:
            FinishFunction( aFuncData, aTokData.mnSpaces );
            eState = STATE_END;
        break;
        default:;
    }
}

void XclExpFmlaCompImpl::PrepareFunction( XclExpFuncData& rFuncData )
{
    // For OOXML these are not rewritten anymore.
    if (GetOutput() != EXC_OUTPUT_XML_2007)
    {
        switch( rFuncData.GetOpCode() )
        {
            case ocCosecant:                // simulate CSC(x) by (1/SIN(x))
            case ocSecant:                  // simulate SEC(x) by (1/COS(x))
            case ocCot:                     // simulate COT(x) by (1/TAN(x))
            case ocCosecantHyp:             // simulate CSCH(x) by (1/SINH(x))
            case ocSecantHyp:               // simulate SECH(x) by (1/COSH(x))
            case ocCotHyp:                  // simulate COTH(x) by (1/TANH(x))
                AppendIntToken( 1 );
                break;
            case ocArcCot:                  // simulate ACOT(x) by (PI/2-ATAN(x))
                AppendNumToken( F_PI2 );
                break;
            default:;
        }
    }
}

void XclExpFmlaCompImpl::FinishFunction( XclExpFuncData& rFuncData, sal_uInt8 nCloseSpaces )
{
    // append missing parameters required in Excel, may modify param count
    AppendTrailingParam( rFuncData );

    // check if parameter count fits into the limits of the function
    sal_uInt8 nParamCount = rFuncData.GetParamCount();
    if( (rFuncData.GetMinParamCount() <= nParamCount) && (nParamCount <= rFuncData.GetMaxParamCount()) )
    {
        // first put the tAttrSpace tokens, they must not be included in tAttrGoto handling
        AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP_CLOSE, nCloseSpaces );
        AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP, rFuncData.GetSpaces() );

        // add tAttrGoto tokens for IF or CHOOSE functions
        switch( rFuncData.GetOpCode() )
        {
            case ocIf:
            case ocChoose:
                AppendJumpToken( rFuncData, EXC_TOK_ATTR_GOTO );
            break;
            default:;
        }

        // put the tFunc or tFuncVar token (or another special token, e.g. tAttrSum)
        AppendFuncToken( rFuncData );

        // update volatile flag - is set if at least one used function is volatile
        mxData->mbVolatile |= rFuncData.IsVolatile();

        // update jump tokens for specific functions, add additional tokens
        switch( rFuncData.GetOpCode() )
        {
            case ocIf:
                FinishIfFunction( rFuncData );
            break;
            case ocChoose:
                FinishChooseFunction( rFuncData );
            break;

            case ocCosecant:                // simulate CSC(x) by (1/SIN(x))
            case ocSecant:                  // simulate SEC(x) by (1/COS(x))
            case ocCot:                     // simulate COT(x) by (1/TAN(x))
            case ocCosecantHyp:             // simulate CSCH(x) by (1/SINH(x))
            case ocSecantHyp:               // simulate SECH(x) by (1/COSH(x))
            case ocCotHyp:                  // simulate COTH(x) by (1/TANH(x))
                // For OOXML not rewritten anymore.
                if (GetOutput() != EXC_OUTPUT_XML_2007)
                {
                    AppendBinaryOperatorToken( EXC_TOKID_DIV, true );
                    AppendParenToken();
                }
            break;
            case ocArcCot:                  // simulate ACOT(x) by (PI/2-ATAN(x))
                // For OOXML not rewritten anymore.
                if (GetOutput() != EXC_OUTPUT_XML_2007)
                {
                    AppendBinaryOperatorToken( EXC_TOKID_SUB, true );
                    AppendParenToken();
                }
            break;

            default:;
        }
    }
    else
        mxData->mbOk = false;
}

void XclExpFmlaCompImpl::FinishIfFunction( XclExpFuncData& rFuncData )
{
    sal_uInt16 nParamCount = rFuncData.GetParamCount();
    OSL_ENSURE( (nParamCount == 2) || (nParamCount == 3), "XclExpFmlaCompImpl::FinishIfFunction - wrong parameter count" );
    const ScfUInt16Vec& rAttrPos = rFuncData.GetAttrPosVec();
    OSL_ENSURE( nParamCount == rAttrPos.size(), "XclExpFmlaCompImpl::FinishIfFunction - wrong number of tAttr tokens" );
    // update tAttrIf token following the condition parameter
    Overwrite( rAttrPos[ 0 ] + 2, static_cast< sal_uInt16 >( rAttrPos[ 1 ] - rAttrPos[ 0 ] ) );
    // update the tAttrGoto tokens following true and false parameters
    UpdateAttrGoto( rAttrPos[ 1 ] );
    if( nParamCount == 3 )
        UpdateAttrGoto( rAttrPos[ 2 ] );
}

void XclExpFmlaCompImpl::FinishChooseFunction( XclExpFuncData& rFuncData )
{
    sal_uInt16 nParamCount = rFuncData.GetParamCount();
    ScfUInt16Vec& rAttrPos = rFuncData.GetAttrPosVec();
    OSL_ENSURE( nParamCount == rAttrPos.size(), "XclExpFmlaCompImpl::FinishChooseFunction - wrong number of tAttr tokens" );
    // number of choices is parameter count minus 1
    sal_uInt16 nChoices = nParamCount - 1;
    // tAttrChoose token contains number of choices
    Overwrite( rAttrPos[ 0 ] + 2, nChoices );
    // cache position of the jump table (follows number of choices in tAttrChoose token)
    sal_uInt16 nJumpArrPos = rAttrPos[ 0 ] + 4;
    // size of jump table: number of choices, plus 1 for error position
    sal_uInt16 nJumpArrSize = 2 * (nChoices + 1);
    // insert the jump table into the tAttrChoose token
    InsertZeros( nJumpArrPos, nJumpArrSize );
    // update positions of tAttrGoto tokens after jump table insertion
    sal_uInt16 nIdx;
    for( nIdx = 1; nIdx < nParamCount; ++nIdx )
        rAttrPos[ nIdx ] = rAttrPos[ nIdx ] + nJumpArrSize;
    // update the tAttrGoto tokens (they contain a value one-less to real distance)
    for( nIdx = 1; nIdx < nParamCount; ++nIdx )
        UpdateAttrGoto( rAttrPos[ nIdx ] );
    // update the distances in the jump table
    Overwrite( nJumpArrPos, nJumpArrSize );
    for( nIdx = 1; nIdx < nParamCount; ++nIdx )
        Overwrite( nJumpArrPos + 2 * nIdx, static_cast< sal_uInt16 >( rAttrPos[ nIdx ] + 4 - nJumpArrPos ) );
}

XclExpScToken XclExpFmlaCompImpl::ProcessParam( XclExpScToken aTokData, XclExpFuncData& rFuncData )
{
    if( rFuncData.IsCalcOnlyParam() )
    {
        // skip Calc-only parameter, stop at next ocClose or ocSep
        aTokData = SkipExpression( aTokData, true );
        rFuncData.IncParamInfoIdx();
    }
    else
    {
        // insert Excel-only parameters, modifies param count and class in rFuncData
        while( rFuncData.IsExcelOnlyParam() )
            AppendDefaultParam( rFuncData );

        // process the parameter, stop at next ocClose or ocSep
        PrepareParam( rFuncData );
        /*  #i37355# insert tMissArg token for missing parameters --
            Excel import filter adds ocMissing token (handled in Factor()),
            but Calc itself does not do this if a new formula is entered. */
        switch( aTokData.GetOpCode() )
        {
            case ocSep:
            case ocClose:   AppendMissingToken();   break;  // empty parameter
            default:        aTokData = Expression( aTokData, false, true );
        }
        // finalize the parameter and add special tokens, e.g. for IF or CHOOSE parameters
        if( mxData->mbOk ) FinishParam( rFuncData );
    }
    return aTokData;
}

void XclExpFmlaCompImpl::PrepareParam( XclExpFuncData& rFuncData )
{
    // index of this parameter is equal to number of already finished parameters
    sal_uInt8 nParamIdx = rFuncData.GetParamCount();

    switch( rFuncData.GetOpCode() )
    {
        case ocIf:
            switch( nParamIdx )
            {
                // add a tAttrIf token before true-parameter (second parameter)
                case 1:     AppendJumpToken( rFuncData, EXC_TOK_ATTR_IF );      break;
                // add a tAttrGoto token before false-parameter (third parameter)
                case 2:     AppendJumpToken( rFuncData, EXC_TOK_ATTR_GOTO );    break;
            }
        break;

        case ocChoose:
            switch( nParamIdx )
            {
                // do nothing for first parameter
                case 0:                                                         break;
                // add a tAttrChoose token before first value parameter (second parameter)
                case 1:     AppendJumpToken( rFuncData, EXC_TOK_ATTR_CHOOSE );  break;
                // add a tAttrGoto token before other value parameters
                default:    AppendJumpToken( rFuncData, EXC_TOK_ATTR_GOTO );
            }
        break;

        case ocArcCotHyp:               // simulate ACOTH(x) by ATANH(1/(x))
            if( nParamIdx == 0 )
                AppendIntToken( 1 );
        break;
        default:;
    }
}

void XclExpFmlaCompImpl::FinishParam( XclExpFuncData& rFuncData )
{
    // increase parameter count, update operand stack
    rFuncData.FinishParam( PopOperandPos() );

    // append more tokens for parameters of some special functions
    sal_uInt8 nParamIdx = rFuncData.GetParamCount() - 1;
    switch( rFuncData.GetOpCode() )
    {
        case ocArcCotHyp:               // simulate ACOTH(x) by ATANH(1/(x))
            if( nParamIdx == 0 )
            {
                AppendParenToken();
                AppendBinaryOperatorToken( EXC_TOKID_DIV, true );
            }
        break;
        default:;
    }
}

void XclExpFmlaCompImpl::AppendDefaultParam( XclExpFuncData& rFuncData )
{
    // prepare parameters of some special functions
    PrepareParam( rFuncData );

    switch( rFuncData.GetOpCode() )
    {
        case ocExternal:
            AppendAddInCallToken( rFuncData.GetExtFuncData() );
        break;
        case ocEuroConvert:
            AppendEuroToolCallToken( rFuncData.GetExtFuncData() );
        break;
        case ocMacro:
            AppendMacroCallToken( rFuncData.GetExtFuncData() );
        break;
        default:
        {
            if( rFuncData.IsAddInEquivalent() )
            {
                AppendAddInCallToken( rFuncData.GetExtFuncData() );
            }
            else if( rFuncData.IsMacroFunc() )
            {
                // Do not write the OOXML <definedName> element for new _xlfn.
                // prefixed functions.
                if (GetOutput() == EXC_OUTPUT_XML_2007)
                    AppendNameToken( 0 );     // dummy to keep parameter count valid
                else
                    AppendMacroCallToken( rFuncData.GetExtFuncData() );
            }
            else
            {
                SAL_WARN( "sc.filter", "XclExpFmlaCompImpl::AppendDefaultParam - unknown opcode" );
                AppendMissingToken();   // to keep parameter count valid
            }
        }
    }

    // update parameter count, add special parameter tokens
    FinishParam( rFuncData );
}

void XclExpFmlaCompImpl::AppendTrailingParam( XclExpFuncData& rFuncData )
{
    sal_uInt8 nParamCount = rFuncData.GetParamCount();
    switch( rFuncData.GetOpCode() )
    {
        case ocIf:
            if( nParamCount == 1 )
            {
                // Excel needs at least two parameters in IF function
                PrepareParam( rFuncData );
                AppendBoolToken( true );
                FinishParam( rFuncData );
            }
        break;

        case ocRound:
        case ocRoundUp:
        case ocRoundDown:
            if( nParamCount == 1 )
            {
                // ROUND, ROUNDUP, ROUNDDOWN functions are fixed to 2 parameters in Excel
                PrepareParam( rFuncData );
                AppendIntToken( 0 );
                FinishParam( rFuncData );
            }
        break;

        case ocIndex:
            if( nParamCount == 1 )
            {
                // INDEX function needs at least 2 parameters in Excel
                PrepareParam( rFuncData );
                AppendMissingToken();
                FinishParam( rFuncData );
            }
        break;

        case ocExternal:
        case ocMacro:
            // external or macro call without parameters needs the external name reference
            if( nParamCount == 0 )
                AppendDefaultParam( rFuncData );
        break;

        case ocGammaDist:
            if( nParamCount == 3 )
            {
                // GAMMADIST function needs 4 parameters in Excel
                PrepareParam( rFuncData );
                AppendIntToken( 1 );
                FinishParam( rFuncData );
            }
        break;

        case ocPoissonDist:
            if( nParamCount == 2 )
            {
                // POISSON function needs 3 parameters in Excel
                PrepareParam( rFuncData );
                AppendIntToken( 1 );
                FinishParam( rFuncData );
            }
        break;

        case ocNormDist:
            if( nParamCount == 3 )
            {
                // NORMDIST function needs 4 parameters in Excel
                PrepareParam( rFuncData );
                AppendBoolToken( true );
                FinishParam( rFuncData );
            }
        break;

        case ocLogNormDist:
            switch( nParamCount )
             {
                // LOGNORMDIST function needs 3 parameters in Excel
                case 1:
                    PrepareParam( rFuncData );
                    AppendIntToken( 0 );
                    FinishParam( rFuncData );
                    SAL_FALLTHROUGH; // add next default parameter
                case 2:
                    PrepareParam( rFuncData );
                    AppendIntToken( 1 );
                    FinishParam( rFuncData );
                    break;
                default:;
             }

        break;

        default:
            // #i108420# function without parameters stored as macro call needs the external name reference
            if( (nParamCount == 0) && rFuncData.IsMacroFunc() )
                AppendDefaultParam( rFuncData );

    }
}

// reference handling ---------------------------------------------------------

namespace {

inline bool lclIsRefRel2D( const ScSingleRefData& rRefData )
{
    return rRefData.IsColRel() || rRefData.IsRowRel();
}

inline bool lclIsRefDel2D( const ScSingleRefData& rRefData )
{
    return rRefData.IsColDeleted() || rRefData.IsRowDeleted();
}

inline bool lclIsRefRel2D( const ScComplexRefData& rRefData )
{
    return lclIsRefRel2D( rRefData.Ref1 ) || lclIsRefRel2D( rRefData.Ref2 );
}

inline bool lclIsRefDel2D( const ScComplexRefData& rRefData )
{
    return lclIsRefDel2D( rRefData.Ref1 ) || lclIsRefDel2D( rRefData.Ref2 );
}

} // namespace

SCTAB XclExpFmlaCompImpl::GetScTab( const ScSingleRefData& rRefData ) const
{
    if (rRefData.IsTabDeleted())
        return SCTAB_INVALID;

    if (!rRefData.IsTabRel())
        // absolute address
        return rRefData.Tab();

    if (!mxData->mpScBasePos)
        return SCTAB_INVALID;

    return rRefData.toAbs(*mxData->mpScBasePos).Tab();
}

bool XclExpFmlaCompImpl::IsRef2D( const ScSingleRefData& rRefData, bool bCheck3DFlag ) const
{
    /*  rRefData.IsFlag3D() determines if sheet name is always visible, even on
        the own sheet. If 3D references are allowed, the passed reference does
        not count as 2D reference. */

    if (bCheck3DFlag && rRefData.IsFlag3D())
        return false;

    if (rRefData.IsTabDeleted())
        return false;

    if (rRefData.IsTabRel())
        return rRefData.Tab() == 0;
    else
        return rRefData.Tab() == GetCurrScTab();
}

bool XclExpFmlaCompImpl::IsRef2D( const ScComplexRefData& rRefData, bool bCheck3DFlag ) const
{
    return IsRef2D(rRefData.Ref1, bCheck3DFlag) && IsRef2D(rRefData.Ref2, bCheck3DFlag);
}

void XclExpFmlaCompImpl::ConvertRefData(
    ScSingleRefData& rRefData, XclAddress& rXclPos,
    bool bNatLangRef, bool bTruncMaxCol, bool bTruncMaxRow ) const
{
    if( mxData->mpScBasePos )
    {
        // *** reference position exists (cell, matrix) - convert to absolute ***
        ScAddress aAbs = rRefData.toAbs(*mxData->mpScBasePos);

        // convert column index
        if (bTruncMaxCol && (aAbs.Col() == mnMaxScCol))
            aAbs.SetCol(mnMaxAbsCol);
        else if ((aAbs.Col() < 0) || (aAbs.Col() > mnMaxAbsCol))
            rRefData.SetColDeleted(true);
        rXclPos.mnCol = static_cast<sal_uInt16>(aAbs.Col()) & mnMaxColMask;

        // convert row index
        if (bTruncMaxRow && (aAbs.Row() == mnMaxScRow))
            aAbs.SetRow(mnMaxAbsRow);
        else if ((aAbs.Row() < 0) || (aAbs.Row() > mnMaxAbsRow))
            rRefData.SetRowDeleted(true);
        rXclPos.mnRow = static_cast<sal_uInt32>(aAbs.Row()) & mnMaxRowMask;

        // Update the reference.
        rRefData.SetAddress(aAbs, *mxData->mpScBasePos);
    }
    else
    {
        // *** no reference position (shared, names, condfmt) - use relative values ***

        // convert column index (2-step-cast ScsCOL->sal_Int16->sal_uInt16 to get all bits correctly)
        sal_Int16 nXclRelCol = static_cast<sal_Int16>(rRefData.Col());
        rXclPos.mnCol = static_cast< sal_uInt16 >( nXclRelCol ) & mnMaxColMask;

        // convert row index (2-step-cast ScsROW->sal_Int16->sal_uInt16 to get all bits correctly)
        sal_Int16 nXclRelRow = static_cast<sal_Int32>(rRefData.Row());
        rXclPos.mnRow = static_cast< sal_uInt32 >( nXclRelRow ) & mnMaxRowMask;
    }

    // flags for relative column and row
    if( bNatLangRef )
    {
        OSL_ENSURE( meBiff == EXC_BIFF8, "XclExpFmlaCompImpl::ConvertRefData - NLRs only for BIFF8" );
        // Calc does not support absolute reference mode in natural language references
        ::set_flag( rXclPos.mnCol, EXC_TOK_NLR_REL );
    }
    else
    {
        sal_uInt16 rnRelRow = rXclPos.mnRow;
        sal_uInt16& rnRelField = (meBiff <= EXC_BIFF5) ? rnRelRow : rXclPos.mnCol;
        ::set_flag( rnRelField, EXC_TOK_REF_COLREL, rRefData.IsColRel() );
        ::set_flag( rnRelField, EXC_TOK_REF_ROWREL, rRefData.IsRowRel() );
    }
}

void XclExpFmlaCompImpl::ConvertRefData(
        ScComplexRefData& rRefData, XclRange& rXclRange, bool bNatLangRef ) const
{
    // convert start and end of the range
    ConvertRefData( rRefData.Ref1, rXclRange.maFirst, bNatLangRef, false, false );
    bool bTruncMaxCol = !rRefData.Ref1.IsColDeleted() && (rXclRange.maFirst.mnCol == 0);
    bool bTruncMaxRow = !rRefData.Ref1.IsRowDeleted() && (rXclRange.maFirst.mnRow == 0);
    ConvertRefData( rRefData.Ref2, rXclRange.maLast, bNatLangRef, bTruncMaxCol, bTruncMaxRow );
}

XclExpRefLogEntry* XclExpFmlaCompImpl::GetNewRefLogEntry()
{
    if( mxData->mpRefLog )
    {
        mxData->mpRefLog->resize( mxData->mpRefLog->size() + 1 );
        return &mxData->mpRefLog->back();
    }
    return nullptr;
}

void XclExpFmlaCompImpl::ProcessCellRef( const XclExpScToken& rTokData )
{
    // get the Excel address components, adjust internal data in aRefData
    bool bNatLangRef = (meBiff == EXC_BIFF8) && mxData->mpScBasePos && (rTokData.GetOpCode() == ocColRowName);
    ScSingleRefData aRefData = *rTokData.mpScToken->GetSingleRef();
    XclAddress aXclPos( ScAddress::UNINITIALIZED );
    ConvertRefData( aRefData, aXclPos, bNatLangRef, false, false );

    if( bNatLangRef )
    {
        OSL_ENSURE( aRefData.IsColRel() != aRefData.IsRowRel(),
            "XclExpFmlaCompImpl::ProcessCellRef - broken natural language reference" );
        // create tNlr token for natural language reference
        sal_uInt8 nSubId = aRefData.IsColRel() ? EXC_TOK_NLR_COLV : EXC_TOK_NLR_ROWV;
        AppendOperandTokenId( EXC_TOKID_NLR, rTokData.mnSpaces );
        Append( nSubId );
        AppendAddress( aXclPos );
    }
    else
    {
        // store external cell contents in CRN records
        if( mxData->mrCfg.mbFromCell && mxData->mpLinkMgr && mxData->mpScBasePos )
            mxData->mpLinkMgr->StoreCell(aRefData, *mxData->mpScBasePos);

        // create the tRef, tRefErr, tRefN, tRef3d, or tRefErr3d token
        if (!mxData->mrCfg.mb3DRefOnly && IsRef2D(aRefData, mxData->mpLinkMgr != nullptr))
        {
            // 2D reference (not in defined names, but allowed in range lists)
            sal_uInt8 nBaseId = (!mxData->mpScBasePos && lclIsRefRel2D( aRefData )) ? EXC_TOKID_REFN :
                (lclIsRefDel2D( aRefData ) ? EXC_TOKID_REFERR : EXC_TOKID_REF);
            AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
            AppendAddress( aXclPos );
        }
        else if( mxData->mpLinkMgr )    // 3D reference
        {
            // 1-based EXTERNSHEET index and 0-based Excel sheet index
            sal_uInt16 nExtSheet, nXclTab;
            mxData->mpLinkMgr->FindExtSheet( nExtSheet, nXclTab, GetScTab( aRefData ), GetNewRefLogEntry() );
            // write the token
            sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_REFERR3D : EXC_TOKID_REF3D;
            AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
            Append( nExtSheet );
            if( meBiff <= EXC_BIFF5 )
            {
                Append( 0, 8 );
                Append( nXclTab );
                Append( nXclTab );
            }
            AppendAddress( aXclPos );
        }
        else
        {
            // 3D ref in cond. format, or 2D ref in name
            AppendErrorToken( EXC_ERR_REF, rTokData.mnSpaces );
        }
    }
}

void XclExpFmlaCompImpl::ProcessRangeRef( const XclExpScToken& rTokData )
{
    // get the Excel address components, adjust internal data in aRefData
    ScComplexRefData aRefData = *rTokData.mpScToken->GetDoubleRef();
    XclRange aXclRange( ScAddress::UNINITIALIZED );
    ConvertRefData( aRefData, aXclRange, false );

    // store external cell contents in CRN records
    if( mxData->mrCfg.mbFromCell && mxData->mpLinkMgr && mxData->mpScBasePos )
        mxData->mpLinkMgr->StoreCellRange(aRefData, *mxData->mpScBasePos);

    // create the tArea, tAreaErr, tAreaN, tArea3d, or tAreaErr3d token
    if (!mxData->mrCfg.mb3DRefOnly && IsRef2D(aRefData, mxData->mpLinkMgr != nullptr))
    {
        // 2D reference (not in name formulas, but allowed in range lists)
        sal_uInt8 nBaseId = (!mxData->mpScBasePos && lclIsRefRel2D( aRefData )) ? EXC_TOKID_AREAN :
             (lclIsRefDel2D( aRefData ) ? EXC_TOKID_AREAERR : EXC_TOKID_AREA);
        AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
        AppendRange( aXclRange );
    }
    else if( mxData->mpLinkMgr )    // 3D reference
    {
        // 1-based EXTERNSHEET index and 0-based Excel sheet indexes
        sal_uInt16 nExtSheet, nFirstXclTab, nLastXclTab;
        mxData->mpLinkMgr->FindExtSheet( nExtSheet, nFirstXclTab, nLastXclTab,
            GetScTab( aRefData.Ref1 ), GetScTab( aRefData.Ref2 ), GetNewRefLogEntry() );
        // write the token
        sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_AREAERR3D : EXC_TOKID_AREA3D;
        AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
        Append( nExtSheet );
        if( meBiff <= EXC_BIFF5 )
        {
            Append( 0, 8 );
            Append( nFirstXclTab );
            Append( nLastXclTab );
        }
        AppendRange( aXclRange );
    }
    else
    {
        // 3D ref in cond. format, or 2D ref in name
        AppendErrorToken( EXC_ERR_REF, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessExternalCellRef( const XclExpScToken& rTokData )
{
    if( mxData->mpLinkMgr )
    {
        // get the Excel address components, adjust internal data in aRefData
        ScSingleRefData aRefData = *rTokData.mpScToken->GetSingleRef();
        XclAddress aXclPos( ScAddress::UNINITIALIZED );
        ConvertRefData( aRefData, aXclPos, false, false, false );

        // store external cell contents in CRN records
        sal_uInt16 nFileId = rTokData.mpScToken->GetIndex();
        OUString aTabName = rTokData.mpScToken->GetString().getString();
        if( mxData->mrCfg.mbFromCell && mxData->mpScBasePos )
            mxData->mpLinkMgr->StoreCell(nFileId, aTabName, aRefData.toAbs(*mxData->mpScBasePos));

        // 1-based EXTERNSHEET index and 0-based Excel sheet indexes
        sal_uInt16 nExtSheet, nFirstSBTab, nLastSBTab;
        mxData->mpLinkMgr->FindExtSheet( nFileId, aTabName, 1, nExtSheet, nFirstSBTab, nLastSBTab, GetNewRefLogEntry() );
        // write the token
        sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_REFERR3D : EXC_TOKID_REF3D;
        AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
        Append( nExtSheet );
        if( meBiff <= EXC_BIFF5 )
        {
            Append( 0, 8 );
            Append( nFirstSBTab );
            Append( nLastSBTab );
        }
        AppendAddress( aXclPos );
    }
    else
    {
        AppendErrorToken( EXC_ERR_REF, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessExternalRangeRef( const XclExpScToken& rTokData )
{
    if( mxData->mpLinkMgr )
    {
        // get the Excel address components, adjust internal data in aRefData
        ScComplexRefData aRefData = *rTokData.mpScToken->GetDoubleRef();
        XclRange aXclRange( ScAddress::UNINITIALIZED );
        ConvertRefData( aRefData, aXclRange, false );

        // store external cell contents in CRN records
        sal_uInt16 nFileId = rTokData.mpScToken->GetIndex();
        OUString aTabName = rTokData.mpScToken->GetString().getString();
        if( mxData->mrCfg.mbFromCell && mxData->mpScBasePos )
            mxData->mpLinkMgr->StoreCellRange(nFileId, aTabName, aRefData.toAbs(*mxData->mpScBasePos));

        // 1-based EXTERNSHEET index and 0-based Excel sheet indexes
        sal_uInt16 nExtSheet, nFirstSBTab, nLastSBTab;
        sal_uInt16 nTabSpan = static_cast<sal_uInt16>(aRefData.Ref2.Tab() - aRefData.Ref1.Tab() + 1);
        mxData->mpLinkMgr->FindExtSheet(
            nFileId, aTabName, nTabSpan, nExtSheet, nFirstSBTab, nLastSBTab, GetNewRefLogEntry());
        // write the token
        sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_AREAERR3D : EXC_TOKID_AREA3D;
        AppendOperandTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), rTokData.mnSpaces );
        Append( nExtSheet );
        if( meBiff <= EXC_BIFF5 )
        {
            Append( 0, 8 );
            Append( nFirstSBTab );
            Append( nLastSBTab );
        }
        AppendRange( aXclRange );
    }
    else
    {
        AppendErrorToken( EXC_ERR_REF, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessDefinedName( const XclExpScToken& rTokData )
{
    SCTAB nTab = SCTAB_GLOBAL;
    bool bGlobal = rTokData.mpScToken->IsGlobal();
    if (!bGlobal)
        nTab = GetCurrScTab();

    XclExpNameManager& rNameMgr = GetNameManager();
    sal_uInt16 nNameIdx = rNameMgr.InsertName(nTab, rTokData.mpScToken->GetIndex());
    if( nNameIdx != 0 )
    {
        // global names always with tName token, local names dependent on config
        SCTAB nScTab = rNameMgr.GetScTab( nNameIdx );
        if( (nScTab == SCTAB_GLOBAL) || (!mxData->mrCfg.mb3DRefOnly && (nScTab == GetCurrScTab())) )
        {
            AppendNameToken( nNameIdx, rTokData.mnSpaces );
        }
        else if( mxData->mpLinkMgr )
        {
            // use the same special EXTERNNAME to refer to any local name
            sal_uInt16 nExtSheet = mxData->mpLinkMgr->FindExtSheet( EXC_EXTSH_OWNDOC );
            AppendNameXToken( nExtSheet, nNameIdx, rTokData.mnSpaces );
        }
        else
            AppendErrorToken( EXC_ERR_NAME, rTokData.mnSpaces );
        // volatile names (containing volatile functions)
        mxData->mbVolatile |= rNameMgr.IsVolatile( nNameIdx );
    }
    else
        AppendErrorToken( EXC_ERR_NAME, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessExternalName( const XclExpScToken& rTokData )
{
    if( mxData->mpLinkMgr )
    {
        ScExternalRefManager& rExtRefMgr = *GetDoc().GetExternalRefManager();
        sal_uInt16 nFileId = rTokData.mpScToken->GetIndex();
        OUString aName = rTokData.mpScToken->GetString().getString();
        ScExternalRefCache::TokenArrayRef xArray = rExtRefMgr.getRangeNameTokens( nFileId, aName );
        if( xArray.get() )
        {
            // store external cell contents in CRN records
            if( mxData->mpScBasePos )
            {
                for( FormulaToken* pScToken = xArray->First(); pScToken; pScToken = xArray->Next() )
                {
                    if( pScToken->IsExternalRef() )
                    {
                        switch( pScToken->GetType() )
                        {
                            case svExternalSingleRef:
                            {
                                ScSingleRefData aRefData = *pScToken->GetSingleRef();
                                mxData->mpLinkMgr->StoreCell(
                                    nFileId, pScToken->GetString().getString(), aRefData.toAbs(*mxData->mpScBasePos));
                            }
                            break;
                            case svExternalDoubleRef:
                            {
                                ScComplexRefData aRefData = *pScToken->GetDoubleRef();
                                mxData->mpLinkMgr->StoreCellRange(
                                    nFileId, pScToken->GetString().getString(), aRefData.toAbs(*mxData->mpScBasePos));
                            }
                            default:
                                ;   // nothing, avoid compiler warning
                        }
                    }
                }
            }

            // insert the new external name and create the tNameX token
            sal_uInt16 nExtSheet = 0, nExtName = 0;
            const OUString* pFile = rExtRefMgr.getExternalFileName( nFileId );
            if( pFile && mxData->mpLinkMgr->InsertExtName( nExtSheet, nExtName, *pFile, aName, xArray ) )
            {
                AppendNameXToken( nExtSheet, nExtName, rTokData.mnSpaces );
                return;
            }
        }
    }

    // on any error: create a #NAME? error
    AppendErrorToken( EXC_ERR_NAME, rTokData.mnSpaces );
}

// token vector ---------------------------------------------------------------

void XclExpFmlaCompImpl::PushOperandPos( sal_uInt16 nTokPos )
{
    mxData->maOpPosStack.push_back( nTokPos );
}

void XclExpFmlaCompImpl::PushOperatorPos( sal_uInt16 nTokPos, const XclExpOperandListRef& rxOperands )
{
    PushOperandPos( nTokPos );
    OSL_ENSURE( rxOperands.get(), "XclExpFmlaCompImpl::AppendOperatorTokenId - missing operand list" );
    if( mxData->maOpListVec.size() <= nTokPos )
        mxData->maOpListVec.resize( nTokPos + 1, XclExpOperandListRef() );
    mxData->maOpListVec[ nTokPos ] = rxOperands;
}

sal_uInt16 XclExpFmlaCompImpl::PopOperandPos()
{
    OSL_ENSURE( !mxData->mbOk || !mxData->maOpPosStack.empty(), "XclExpFmlaCompImpl::PopOperandPos - token stack broken" );
    mxData->mbOk &= !mxData->maOpPosStack.empty();
    if( mxData->mbOk )
    {
        sal_uInt16 nTokPos = mxData->maOpPosStack.back();
        mxData->maOpPosStack.pop_back();
        return nTokPos;
    }
    return 0;
}

namespace {

inline void lclAppend( ScfUInt8Vec& orVector, sal_uInt16 nData )
{
    orVector.resize( orVector.size() + 2 );
    ShortToSVBT16( nData, &*(orVector.end() - 2) );
}

inline void lclAppend( ScfUInt8Vec& orVector, sal_uInt32 nData )
{
    orVector.resize( orVector.size() + 4 );
    UInt32ToSVBT32( nData, &*(orVector.end() - 4) );
}

inline void lclAppend( ScfUInt8Vec& orVector, double fData )
{
    orVector.resize( orVector.size() + 8 );
    DoubleToSVBT64( fData, &*(orVector.end() - 8) );
}

inline void lclAppend( ScfUInt8Vec& orVector, const XclExpRoot& rRoot, const OUString& rString, XclStrFlags nStrFlags )
{
    XclExpStringRef xXclStr = XclExpStringHelper::CreateString( rRoot, rString, nStrFlags, EXC_TOK_STR_MAXLEN );
    size_t nSize = orVector.size();
    orVector.resize( nSize + xXclStr->GetSize() );
    xXclStr->WriteToMem( &orVector[ nSize ] );
}

} // namespace

void XclExpFmlaCompImpl::Append( sal_uInt8 nData )
{
    mxData->maTokVec.push_back( nData );
}

void XclExpFmlaCompImpl::Append( sal_uInt8 nData, size_t nCount )
{
    mxData->maTokVec.resize( mxData->maTokVec.size() + nCount, nData );
}

void XclExpFmlaCompImpl::Append( sal_uInt16 nData )
{
    lclAppend( mxData->maTokVec, nData );
}

void XclExpFmlaCompImpl::Append( sal_uInt32 nData )
{
    lclAppend( mxData->maTokVec, nData );
}

void XclExpFmlaCompImpl::Append( double fData )
{
    lclAppend( mxData->maTokVec, fData );
}

void XclExpFmlaCompImpl::Append( const OUString& rString )
{
    lclAppend( mxData->maTokVec, GetRoot(), rString, EXC_STR_8BITLENGTH );
}

void XclExpFmlaCompImpl::AppendAddress( const XclAddress& rXclPos )
{
    Append( static_cast<sal_uInt16>(rXclPos.mnRow) );
    if( meBiff <= EXC_BIFF5 )
        Append( static_cast< sal_uInt8 >( rXclPos.mnCol ) );
    else
        Append( rXclPos.mnCol );
}

void XclExpFmlaCompImpl::AppendRange( const XclRange& rXclRange )
{
    Append( static_cast<sal_uInt16>(rXclRange.maFirst.mnRow) );
    Append( static_cast<sal_uInt16>(rXclRange.maLast.mnRow) );
    if( meBiff <= EXC_BIFF5 )
    {
        Append( static_cast< sal_uInt8 >( rXclRange.maFirst.mnCol ) );
        Append( static_cast< sal_uInt8 >( rXclRange.maLast.mnCol ) );
    }
    else
    {
        Append( rXclRange.maFirst.mnCol );
        Append( rXclRange.maLast.mnCol );
    }
}

void XclExpFmlaCompImpl::AppendSpaceToken( sal_uInt8 nType, sal_uInt8 nCount )
{
    if( nCount > 0 )
    {
        Append( EXC_TOKID_ATTR );
        Append( EXC_TOK_ATTR_SPACE );
        Append( nType );
        Append( nCount );
    }
}

void XclExpFmlaCompImpl::AppendOperandTokenId( sal_uInt8 nTokenId, sal_uInt8 nSpaces )
{
    AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP, nSpaces );
    PushOperandPos( GetSize() );
    Append( nTokenId );
}

void XclExpFmlaCompImpl::AppendIntToken( sal_uInt16 nValue, sal_uInt8 nSpaces )
{
    AppendOperandTokenId( EXC_TOKID_INT, nSpaces );
    Append( nValue );
}

void XclExpFmlaCompImpl::AppendNumToken( double fValue, sal_uInt8 nSpaces )
{
    AppendOperandTokenId( EXC_TOKID_NUM, nSpaces );
    Append( fValue );
}

void XclExpFmlaCompImpl::AppendBoolToken( bool bValue, sal_uInt8 nSpaces )
{
    AppendOperandTokenId( EXC_TOKID_BOOL, nSpaces );
    Append( bValue ? EXC_TOK_BOOL_TRUE : EXC_TOK_BOOL_FALSE );
}

void XclExpFmlaCompImpl::AppendErrorToken( sal_uInt8 nErrCode, sal_uInt8 nSpaces )
{
    AppendOperandTokenId( EXC_TOKID_ERR, nSpaces );
    Append( nErrCode );
}

void XclExpFmlaCompImpl::AppendMissingToken( sal_uInt8 nSpaces )
{
    AppendOperandTokenId( EXC_TOKID_MISSARG, nSpaces );
}

void XclExpFmlaCompImpl::AppendNameToken( sal_uInt16 nNameIdx, sal_uInt8 nSpaces )
{
    if( nNameIdx > 0 )
    {
        AppendOperandTokenId( GetTokenId( EXC_TOKID_NAME, EXC_TOKCLASS_REF ), nSpaces );
        Append( nNameIdx );
        Append( 0, (meBiff <= EXC_BIFF5) ? 12 : 2 );
    }
    else
        AppendErrorToken( EXC_ERR_NAME );
}

void XclExpFmlaCompImpl::AppendMissingNameToken( const OUString& rName, sal_uInt8 nSpaces )
{
    sal_uInt16 nNameIdx = GetNameManager().InsertRawName( rName );
    AppendNameToken( nNameIdx, nSpaces );
}

void XclExpFmlaCompImpl::AppendNameXToken( sal_uInt16 nExtSheet, sal_uInt16 nExtName, sal_uInt8 nSpaces )
{
    AppendOperandTokenId( GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ), nSpaces );
    Append( nExtSheet );
    if( meBiff <= EXC_BIFF5 )
        Append( 0, 8 );
    Append( nExtName );
    Append( 0, (meBiff <= EXC_BIFF5) ? 12 : 2 );
}

void XclExpFmlaCompImpl::AppendMacroCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces )
{
    sal_uInt16 nNameIdx = GetNameManager().InsertMacroCall( rExtFuncData.maFuncName, rExtFuncData.mbVBasic, true, rExtFuncData.mbHidden );
    AppendNameToken( nNameIdx, nSpaces );
}

void XclExpFmlaCompImpl::AppendAddInCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces )
{
    OUString aXclFuncName;
    if( mxData->mpLinkMgr && ScGlobal::GetAddInCollection()->GetExcelName( rExtFuncData.maFuncName, GetUILanguage(), aXclFuncName ) )
    {
        sal_uInt16 nExtSheet, nExtName;
        if( mxData->mpLinkMgr->InsertAddIn( nExtSheet, nExtName, aXclFuncName ) )
        {
            AppendNameXToken( nExtSheet, nExtName, nSpaces );
            return;
        }
    }
    AppendMacroCallToken( rExtFuncData, nSpaces );
}

void XclExpFmlaCompImpl::AppendEuroToolCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nSpaces )
{
    sal_uInt16 nExtSheet(0), nExtName(0);
    if( mxData->mpLinkMgr && mxData->mpLinkMgr->InsertEuroTool( nExtSheet, nExtName, rExtFuncData.maFuncName ) )
        AppendNameXToken( nExtSheet, nExtName, nSpaces );
    else
        AppendMacroCallToken( rExtFuncData, nSpaces );
}

void XclExpFmlaCompImpl::AppendOperatorTokenId( sal_uInt8 nTokenId, const XclExpOperandListRef& rxOperands, sal_uInt8 nSpaces )
{
    AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP, nSpaces );
    PushOperatorPos( GetSize(), rxOperands );
    Append( nTokenId );
}

void XclExpFmlaCompImpl::AppendUnaryOperatorToken( sal_uInt8 nTokenId, sal_uInt8 nSpaces )
{
    XclExpOperandListRef xOperands( new XclExpOperandList );
    xOperands->AppendOperand( PopOperandPos(), EXC_PARAMCONV_RPO, true );
    AppendOperatorTokenId( nTokenId, xOperands, nSpaces );
}

void XclExpFmlaCompImpl::AppendBinaryOperatorToken( sal_uInt8 nTokenId, bool bValType, sal_uInt8 nSpaces )
{
    XclExpOperandListRef xOperands( new XclExpOperandList );
    xOperands->AppendOperand( PopOperandPos(), EXC_PARAMCONV_RPO, bValType );
    xOperands->AppendOperand( PopOperandPos(), EXC_PARAMCONV_RPO, bValType );
    AppendOperatorTokenId( nTokenId, xOperands, nSpaces );
}

void XclExpFmlaCompImpl::AppendLogicalOperatorToken( sal_uInt16 nXclFuncIdx, sal_uInt8 nOpCount )
{
    XclExpOperandListRef xOperands( new XclExpOperandList );
    for( sal_uInt8 nOpIdx = 0; nOpIdx < nOpCount; ++nOpIdx  )
        xOperands->AppendOperand( PopOperandPos(), EXC_PARAMCONV_RPX, false );
    AppendOperatorTokenId( GetTokenId( EXC_TOKID_FUNCVAR, EXC_TOKCLASS_VAL ), xOperands );
    Append( nOpCount );
    Append( nXclFuncIdx );
}

void XclExpFmlaCompImpl::AppendFuncToken( const XclExpFuncData& rFuncData )
{
    sal_uInt16 nXclFuncIdx = rFuncData.GetXclFuncIdx();
    sal_uInt8 nParamCount = rFuncData.GetParamCount();
    sal_uInt8 nRetClass = rFuncData.GetReturnClass();

    if( (nXclFuncIdx == EXC_FUNCID_SUM) && (nParamCount == 1) )
    {
        // SUM with only one parameter
        AppendOperatorTokenId( EXC_TOKID_ATTR, rFuncData.GetOperandList() );
        Append( EXC_TOK_ATTR_SUM );
        Append( sal_uInt16( 0 ) );
    }
    else if( rFuncData.IsFixedParamCount() )
    {
        // fixed number of parameters
        AppendOperatorTokenId( GetTokenId( EXC_TOKID_FUNC, nRetClass ), rFuncData.GetOperandList() );
        Append( nXclFuncIdx );
    }
    else
    {
        // variable number of parameters
        AppendOperatorTokenId( GetTokenId( EXC_TOKID_FUNCVAR, nRetClass ), rFuncData.GetOperandList() );
        Append( nParamCount );
        Append( nXclFuncIdx );
    }
}

void XclExpFmlaCompImpl::AppendParenToken( sal_uInt8 nOpenSpaces, sal_uInt8 nCloseSpaces )
{
    AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP_OPEN, nOpenSpaces );
    AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP_CLOSE, nCloseSpaces );
    Append( EXC_TOKID_PAREN );
}

void XclExpFmlaCompImpl::AppendJumpToken( XclExpFuncData& rFuncData, sal_uInt8 nAttrType )
{
    // store the start position of the token
    rFuncData.AppendAttrPos( GetSize() );
    // create the tAttr token
    Append( EXC_TOKID_ATTR );
    Append( nAttrType );
    Append( sal_uInt16( 0 ) );  // placeholder that will be updated later
}

void XclExpFmlaCompImpl::InsertZeros( sal_uInt16 nInsertPos, sal_uInt16 nInsertSize )
{
    // insert zeros into the token array
    OSL_ENSURE( nInsertPos < mxData->maTokVec.size(), "XclExpFmlaCompImpl::Insert - invalid position" );
    mxData->maTokVec.insert( mxData->maTokVec.begin() + nInsertPos, nInsertSize, 0 );

    // update positions of operands waiting for an operator
    for( ScfUInt16Vec::iterator aIt = mxData->maOpPosStack.begin(), aEnd = mxData->maOpPosStack.end(); aIt != aEnd; ++aIt )
        if( nInsertPos <= *aIt )
            *aIt = *aIt + nInsertSize;

    // update operand lists of all operator tokens
    if( nInsertPos < mxData->maOpListVec.size() )
        mxData->maOpListVec.insert( mxData->maOpListVec.begin() + nInsertPos, nInsertSize, XclExpOperandListRef() );
    for( XclExpOperandListVector::iterator aIt = mxData->maOpListVec.begin(), aEnd = mxData->maOpListVec.end(); aIt != aEnd; ++aIt )
        if( aIt->get() )
            for( XclExpOperandList::iterator aIt2 = (*aIt)->begin(), aEnd2 = (*aIt)->end(); aIt2 != aEnd2; ++aIt2 )
                if( nInsertPos <= aIt2->mnTokPos )
                    aIt2->mnTokPos = aIt2->mnTokPos + nInsertSize;
}

void XclExpFmlaCompImpl::Overwrite( sal_uInt16 nWriteToPos, sal_uInt16 nOffset )
{
    OSL_ENSURE( static_cast< size_t >( nWriteToPos + 1 ) < mxData->maTokVec.size(), "XclExpFmlaCompImpl::Overwrite - invalid position" );
    ShortToSVBT16( nOffset, &mxData->maTokVec[ nWriteToPos ] );
}

void XclExpFmlaCompImpl::UpdateAttrGoto( sal_uInt16 nAttrPos )
{
    /*  tAttrGoto contains distance from end of tAttr token to position behind
        the function token (for IF or CHOOSE function), which is currently at
        the end of the token array. Additionally this distance is decreased by
        one, for whatever reason. So we have to subtract 4 and 1 from the
        distance between the tAttr token start and the end of the token array. */
    Overwrite( nAttrPos + 2, static_cast< sal_uInt16 >( GetSize() - nAttrPos - 5 ) );
}

bool XclExpFmlaCompImpl::IsSpaceToken( sal_uInt16 nPos ) const
{
    return
        (static_cast< size_t >( nPos + 4 ) <= mxData->maTokVec.size()) &&
        (mxData->maTokVec[ nPos ] == EXC_TOKID_ATTR) &&
        (mxData->maTokVec[ nPos + 1 ] == EXC_TOK_ATTR_SPACE);
}

void XclExpFmlaCompImpl::RemoveTrailingParen()
{
    // remove trailing tParen token
    if( !mxData->maTokVec.empty() && (mxData->maTokVec.back() == EXC_TOKID_PAREN) )
        mxData->maTokVec.pop_back();
    // remove remaining tAttrSpace tokens
    while( (mxData->maTokVec.size() >= 4) && IsSpaceToken( GetSize() - 4 ) )
        mxData->maTokVec.erase( mxData->maTokVec.end() - 4, mxData->maTokVec.end() );
}

void XclExpFmlaCompImpl::AppendExt( sal_uInt8 nData )
{
    mxData->maExtDataVec.push_back( nData );
}

void XclExpFmlaCompImpl::AppendExt( sal_uInt8 nData, size_t nCount )
{
    mxData->maExtDataVec.resize( mxData->maExtDataVec.size() + nCount, nData );
}

void XclExpFmlaCompImpl::AppendExt( sal_uInt16 nData )
{
    lclAppend( mxData->maExtDataVec, nData );
}

void XclExpFmlaCompImpl::AppendExt( double fData )
{
    lclAppend( mxData->maExtDataVec, fData );
}

void XclExpFmlaCompImpl::AppendExt( const OUString& rString )
{
    lclAppend( mxData->maExtDataVec, GetRoot(), rString, (meBiff == EXC_BIFF8) ? EXC_STR_DEFAULT : EXC_STR_8BITLENGTH );
}

namespace {

void lclInitOwnTab( ScSingleRefData& rRef, const ScAddress& rScPos, SCTAB nCurrScTab, bool b3DRefOnly )
{
    if( b3DRefOnly )
    {
        // no reduction to 2D reference, if global link manager is used
        rRef.SetFlag3D(true);
    }
    else if( rScPos.Tab() == nCurrScTab )
    {
        rRef.SetRelTab(0);
    }
}

void lclPutCellToTokenArray( ScTokenArray& rScTokArr, const ScAddress& rScPos, SCTAB nCurrScTab, bool b3DRefOnly )
{
    ScSingleRefData aRef;
    aRef.InitAddress( rScPos );
    lclInitOwnTab( aRef, rScPos, nCurrScTab, b3DRefOnly );
    rScTokArr.AddSingleReference( aRef );
}

void lclPutRangeToTokenArray( ScTokenArray& rScTokArr, const ScRange& rScRange, SCTAB nCurrScTab, bool b3DRefOnly )
{
    if( rScRange.aStart == rScRange.aEnd )
    {
        lclPutCellToTokenArray( rScTokArr, rScRange.aStart, nCurrScTab, b3DRefOnly );
    }
    else
    {
        ScComplexRefData aRef;
        aRef.InitRange( rScRange );
        lclInitOwnTab( aRef.Ref1, rScRange.aStart, nCurrScTab, b3DRefOnly );
        lclInitOwnTab( aRef.Ref2, rScRange.aEnd, nCurrScTab, b3DRefOnly );
        rScTokArr.AddDoubleReference( aRef );
    }
}

} // namespace

XclExpFormulaCompiler::XclExpFormulaCompiler( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mxImpl( new XclExpFmlaCompImpl( rRoot ) )
{
}

XclExpFormulaCompiler::~XclExpFormulaCompiler()
{
}

XclTokenArrayRef XclExpFormulaCompiler::CreateFormula(
        XclFormulaType eType, const ScTokenArray& rScTokArr,
        const ScAddress* pScBasePos, XclExpRefLog* pRefLog )
{
    return mxImpl->CreateFormula( eType, rScTokArr, pScBasePos, pRefLog );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateFormula( XclFormulaType eType, const ScAddress& rScPos )
{
    ScTokenArray aScTokArr;
    lclPutCellToTokenArray( aScTokArr, rScPos, GetCurrScTab(), mxImpl->Is3DRefOnly( eType ) );
    return mxImpl->CreateFormula( eType, aScTokArr );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateFormula( XclFormulaType eType, const ScRange& rScRange )
{
    ScTokenArray aScTokArr;
    lclPutRangeToTokenArray( aScTokArr, rScRange, GetCurrScTab(), mxImpl->Is3DRefOnly( eType ) );
    return mxImpl->CreateFormula( eType, aScTokArr );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateFormula( XclFormulaType eType, const ScRangeList& rScRanges )
{
    size_t nCount = rScRanges.size();
    if( nCount == 0 )
        return XclTokenArrayRef();

    ScTokenArray aScTokArr;
    SCTAB nCurrScTab = GetCurrScTab();
    bool b3DRefOnly = mxImpl->Is3DRefOnly( eType );
    for( size_t nIdx = 0; nIdx < nCount; ++nIdx )
    {
        if( nIdx > 0 )
            aScTokArr.AddOpCode( ocUnion );
        lclPutRangeToTokenArray( aScTokArr, *rScRanges[ nIdx ], nCurrScTab, b3DRefOnly );
    }
    return mxImpl->CreateFormula( eType, aScTokArr );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateErrorFormula( sal_uInt8 nErrCode )
{
    return mxImpl->CreateErrorFormula( nErrCode );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateSpecialRefFormula(
        sal_uInt8 nTokenId, const XclAddress& rXclPos )
{
    return mxImpl->CreateSpecialRefFormula( nTokenId, rXclPos );
}

XclTokenArrayRef XclExpFormulaCompiler::CreateNameXFormula(
        sal_uInt16 nExtSheet, sal_uInt16 nExtName )
{
    return mxImpl->CreateNameXFormula( nExtSheet, nExtName );
}

bool XclExpFormulaCompiler::IsRef2D( const ScSingleRefData& rRefData ) const
{
    return mxImpl->IsRef2D(rRefData, true);
}

bool XclExpFormulaCompiler::IsRef2D( const ScComplexRefData& rRefData ) const
{
    return mxImpl->IsRef2D(rRefData, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
