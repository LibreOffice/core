/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeformula.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:14:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XEFORMULA_HXX
#include "xeformula.hxx"
#endif

#include <list>
#include <map>

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif
#ifndef SC_ADDINCOL_HXX
#include "addincol.hxx"
#endif

#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif
#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XENAME_HXX
#include "xename.hxx"
#endif

// External reference log =====================================================

XclExpRefLogEntry::XclExpRefLogEntry() :
    mpUrl( 0 ),
    mpFirstTab( 0 ),
    mpLastTab( 0 ),
    mnFirstXclTab( EXC_TAB_DELETED ),
    mnLastXclTab( EXC_TAB_DELETED )
{
}

// Formula compiler ===========================================================

/** Type of token class handling. */
enum XclExpTokenClassType
{
    EXC_CLASSTYPE_CELL,         /// Cell formula, shared formula.
    EXC_CLASSTYPE_ARRAY,        /// Array formula, conditional formatting, data validation.
    EXC_CLASSTYPE_NAME          /// Defined name, range list.
};

/** Type of the link manager to be used. */
enum XclExpLinkMgrType
{
    EXC_LINKMGRTYPE_NONE,       /// No link manager, 2D references only.
    EXC_LINKMGRTYPE_LOCAL,      /// Local (per-sheet) link manager.
    EXC_LINKMGRTYPE_GLOBAL      /// Global link manager.
};

// ----------------------------------------------------------------------------

/** Configuration data of the formula compiler. */
struct XclExpCompConfig
{
    XclFormulaType      meType;         /// Type of the formula to be created.
    XclExpTokenClassType meClassType;   /// Token class handling type.
    XclExpLinkMgrType   meLinkMgrType;  /// Link manager to be used.
    bool                mbFromCell;     /// True = Any kind of cell formula (cell, array, shared).
    bool                mb3DRefOnly;    /// True = Only 3D references allowed (e.g. names).
    bool                mbStopAtSep;    /// True = Stop compilation at ocSep in root level.
    bool                mbAllowArrays;  /// True = Allow inline arrays.
};

// ----------------------------------------------------------------------------

/** Working data of the formula compiler. Used to push onto a stack for recursive calls. */
struct XclExpCompData
{
    typedef ::std::list< const ScMatrix* >  ScMatrixList;
    typedef ScfRef< ScMatrixList >          ScMatrixListRef;
    typedef ScfRef< ScTokenArray >          ScTokenArrayRef;

    XclExpCompConfig    maCfg;          /// Configuration for current formula type.
    ScfUInt8Vec         maTokVec;       /// Byte vector containing token data.
    ScTokenArrayRef     mxOwnScTokArr;  /// Own clone of a Calc token array.
    XclTokenArrayIterator maTokArrIt;   /// Iterator in Calc token array.
    XclExpLinkManager*  mpLinkMgr;      /// Link manager for current context (local/global).
    XclExpRefLog*       mpRefLog;       /// Log for external references.
    ScMatrixListRef     mxInlineArr;    /// List of inline arrays (in reverse order)

    const ScAddress*    mpScBasePos;    /// Current cell position of the formula.

    // processing data during compilation
    sal_uInt16          mnLastTokPos;   /// Position of last appended Excel token ID.
    sal_uInt8           mnLastDefClass; /// Default class of last appended Excel token ID.
    sal_uInt8           mnRefExpClass;  /// New class for VAL parameters, if REF is expected.
    sal_uInt8           mnValExpClass;  /// New class for all parameters, if VAL is expected.
    sal_uInt8           mnArrExpClass;  /// New class for all parameters, if ARR is expected.
    bool                mbStopAtSep;    /// True = Stop subexpression creation at an ocSep token.
    bool                mbVolatile;     /// True = Formula contains volatile function.
    bool                mbIsArrExp;     /// True = ARR class is expected somewhere before.
    bool                mbOk;           /// Current state of the compiler.

    explicit            XclExpCompData();
};

XclExpCompData::XclExpCompData() :
    mpLinkMgr( 0 ),
    mpRefLog( 0 ),
    mpScBasePos( 0 ),
    mbOk( false )
{
}

// ----------------------------------------------------------------------------

/** Working data for a processed Calc formula token. */
struct XclExpTokenData
{
    const ScToken*      mpScToken;          /// Currently processed Calc token.
    sal_uInt8           mnSpaces;           /// Number of spaces before the Calc token.

    inline explicit     XclExpTokenData() : mpScToken( 0 ), mnSpaces( 0 ) {}
    inline bool         Is() const { return mpScToken != 0; }
    inline StackVar     GetType() const { return mpScToken ? mpScToken->GetType() : static_cast< StackVar >( svUnknown ); }
    inline OpCode       GetOpCode() const { return mpScToken ? mpScToken->GetOpCode() : static_cast< OpCode >( ocNone ); }
};

// ----------------------------------------------------------------------------

/** Encapsulates all data needed for a call to an external function (macro, add-in). */
struct XclExpExtFuncData
{
    String              maFuncName;         /// Name of the function.
    bool                mbVBasic;           /// True = Visual Basic macro call.
    bool                mbHidden;           /// True = Create hidden defined name.

    inline explicit     XclExpExtFuncData() : mbVBasic( false ), mbHidden( false ) {}
    void                Set( const String& rFuncName, bool bVBasic, bool bHidden );
};

void XclExpExtFuncData::Set( const String& rFuncName, bool bVBasic, bool bHidden )
{
    maFuncName = rFuncName;
    mbVBasic = bVBasic;
    mbHidden = bHidden;
}

// ----------------------------------------------------------------------------

/** Encapsulates all data needed to process an entire function. */
class XclExpFuncData
{
public:
    explicit            XclExpFuncData(
                            const XclExpTokenData& rTokData,
                            const XclFunctionInfo& rFuncInfo,
                            const XclExpExtFuncData& rExtFuncData,
                            sal_uInt8 nExpRetClass );

    inline const ScToken& GetScToken() const { return *mrTokData.mpScToken; }
    inline OpCode       GetOpCode() const { return mrFuncInfo.meOpCode; }
    inline sal_uInt16   GetXclFuncIdx() const { return mrFuncInfo.mnXclFunc; }
    inline bool         IsVolatile() const { return mrFuncInfo.IsVolatile(); }
    inline bool         IsMacroFunc() const { return mrFuncInfo.IsMacroFunc(); }
    inline sal_uInt8    GetSpaces() const { return mrTokData.mnSpaces; }
    inline const XclExpExtFuncData& GetExtFuncData() const { return maExtFuncData; }

    inline sal_uInt8    GetReturnClass() const { return mrFuncInfo.mnRetClass; }
    inline sal_uInt8    GetExpReturnClass() const { return mnExpRetClass; }

    inline sal_uInt8    GetExpParamClass() const { return mrFuncInfo.mpnParamClass[ mnClassIdx ]; }
    void                IncExpParamClassIdx();

    inline sal_uInt8    GetMinParamCount() const { return mrFuncInfo.mnMinParamCount; }
    inline sal_uInt8    GetMaxParamCount() const { return mrFuncInfo.mnMaxParamCount; }
    inline sal_uInt8    GetParamCount() const { return mnParamCount; }
    inline void         IncParamCount() { ++mnParamCount; }

    inline ScfUInt16Vec& GetAttrPosVec() { return maAttrPosVec; }
    inline void         AppendAttrPos( sal_uInt16 nPos ) { maAttrPosVec.push_back( nPos ); }

private:
    ScfUInt16Vec        maAttrPosVec;       /// Token array positions of tAttr tokens.
    const XclExpTokenData& mrTokData;       /// Data about processed function name token.
    const XclFunctionInfo& mrFuncInfo;      /// Constant data about processed function.
    XclExpExtFuncData   maExtFuncData;      /// Data for external functions (macro, add-in).
    sal_uInt8           mnExpRetClass;      /// Expected token class for return value.
    sal_uInt8           mnClassIdx;         /// Index into expected parameter class array of mrFuncInfo.
    sal_uInt8           mnParamCount;       /// Current number of parameters of a function.
};

XclExpFuncData::XclExpFuncData(
        const XclExpTokenData& rTokData, const XclFunctionInfo& rFuncInfo,
        const XclExpExtFuncData& rExtFuncData, sal_uInt8 nExpRetClass ) :
    mrTokData( rTokData ),
    mrFuncInfo( rFuncInfo ),
    maExtFuncData( rExtFuncData ),
    mnExpRetClass( nExpRetClass ),
    mnClassIdx( 0 ),
    mnParamCount( 0 )
{
    DBG_ASSERT( mrTokData.mpScToken, "XclExpFuncData::XclExpFuncData - missing core token" );
    // set name of an add-in function
    if( !maExtFuncData.maFuncName.Len() && dynamic_cast< const ScExternalToken* >( mrTokData.mpScToken ) )
        maExtFuncData.Set( GetScToken().GetExternal(), true, false );
}

void XclExpFuncData::IncExpParamClassIdx()
{
    if( (mnClassIdx + 1 < EXC_FUNCINFO_CLASSCOUNT) && (mrFuncInfo.mpnParamClass[ mnClassIdx + 1 ] != EXC_TOKCLASS_NONE) )
        ++mnClassIdx;
}

// ----------------------------------------------------------------------------

/** Implementation class of the export formula compiler. */
class XclExpFmlaCompImpl : protected XclExpRoot, protected XclTokenArrayHelper, private XclExpCompData
{
public:
    explicit            XclExpFmlaCompImpl( const XclExpRoot& rRoot );

    /** Creates an Excel token array from the passed Calc token array. */
    XclTokenArrayRef    CreateFormula(
                            XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos = 0, XclExpRefLog* pRefLog = 0 );
    /** Creates a single error token containing the passed error code. */
    XclTokenArrayRef    CreateErrorFormula( sal_uInt8 nErrCode );
    /** Creates a single token for a special cell reference. */
    XclTokenArrayRef    CreateSpecialRefFormula( sal_uInt8 nTokenId, const XclAddress& rXclPos );
    /** Creates a single tNameXR token for a reference to an external name. */
    XclTokenArrayRef    CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName );

    /** Returns true, if the passed formula type allows 3D references only. */
    bool                Is3DRefOnly( XclFormulaType eType ) const;

    // ------------------------------------------------------------------------
private:
    const XclExpCompConfig* GetConfigForType( XclFormulaType eType ) const;
    inline sal_uInt16   GetSize() const { return static_cast< sal_uInt16 >( maTokVec.size() ); }

    void                EnterRecursive();
    void                Init( XclFormulaType eType );
    void                Init( XclFormulaType eType, const ScTokenArray& rScTokArr,
                            const ScAddress* pScBasePos, XclExpRefLog* pRefLog );

    void                LeaveRecursive();
    void                FinalizeFormula( ScfUInt8Vec & rExtensionTokens );
    void                AppendInlineArrays( ScfUInt8Vec & rExtensionTokens );
    XclTokenArrayRef    CreateTokenArray( ScfUInt8Vec* pExtensionTokens = NULL );

    // compiler ---------------------------------------------------------------
    // XclExpTokenData: pass-by-value and return-by-value is intended

    const ScToken*      GetNextRawToken();
    const ScToken*      PeekNextRawToken( bool bSkipSpaces ) const;

    bool                GetNextToken( XclExpTokenData& rTokData );
    XclExpTokenData     GetNextToken();

    XclExpTokenData     Expression( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool bStopAtSep );
    XclExpTokenData     SkipExpression( XclExpTokenData aTokData, bool bStopAtSep );

    XclExpTokenData     OrTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     AndTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     CompareTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     ConcatTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     AddSubTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     MulDivTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     PowTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     UnaryPostTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     UnaryPreTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     ListTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass );
    XclExpTokenData     IntersectTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool& rbHasRefOp );
    XclExpTokenData     RangeTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool& rbHasRefOp );
    XclExpTokenData     Factor( XclExpTokenData aTokData, sal_uInt8 nExpClass );

    // formula structure ------------------------------------------------------

    void                ProcessDouble( const XclExpTokenData& rTokData );
    void                ProcessString( const XclExpTokenData& rTokData );
    void                ProcessError( const XclExpTokenData& rTokData );
    void                ProcessMissing( const XclExpTokenData& rTokData );
    void                ProcessBad( const XclExpTokenData& rTokData );
    void                ProcessParentheses( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessBoolean( const XclExpTokenData& rTokData );
    void                ProcessDdeLink( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessExternal( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );

    void                ProcessFunction( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                PrepareFunction( XclExpFuncData& rFuncData );
    void                FinishFunction( XclExpFuncData& rFuncData, sal_uInt8 nCloseSpaces );
    void                FinishIfFunction( XclExpFuncData& rFuncData );
    void                FinishChooseFunction( XclExpFuncData& rFuncData );

    XclExpTokenData     ProcessParam( XclExpTokenData aTokData, XclExpFuncData& rFuncData );
    void                PrepareParam( XclExpFuncData& rFuncData );
    void                FinishParam( XclExpFuncData& rFuncData );
    void                AppendDefaultParam( XclExpFuncData& rFuncData );
    void                AppendTrailingParam( XclExpFuncData& rFuncData );

    // reference handling -----------------------------------------------------

    SCTAB               GetScTab( const SingleRefData& rRefData ) const;
    bool                IsRef2D( const SingleRefData& rRefData ) const;
    bool                IsRef2D( const ComplRefData& rRefData ) const;

    void                ConvertRefData( SingleRefData& rRefData, XclAddress& rXclPos,
                            bool bNatLangRef, bool bTruncMaxCol, bool bTruncMaxRow ) const;
    void                ConvertRefData( ComplRefData& rRefData, XclRange& rXclRange,
                            bool bNatLangRef ) const;

    XclExpRefLogEntry*  GetNewRefLogEntry();
    void                ProcessCellRef( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessRangeRef( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessMatrix( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessDefinedName( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );
    void                ProcessDatabaseArea( const XclExpTokenData& rTokData, sal_uInt8 nExpClass );

    // token identifiers ------------------------------------------------------

    void                SetReplaceTokenClasses();
    void                SetArrExpFlag( bool bIsArrExp );
    void                UpdateArrExpFlag( sal_uInt8 nParamExpClass, sal_uInt8 nFuncRetClass );

    void                AdjustTokenClass( sal_uInt8& rnTokenId, sal_uInt8 nExpClass );
    void                AdjustLastTokenClass( sal_uInt8 nExpClass );
    void                AdjustLastTokenClassForEastereggOp();

    void                AppendOpTokenId( sal_uInt8 nTokenId, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendFuncTokenId( sal_uInt16 nXclFuncIdx, sal_uInt8 nRetClass, sal_uInt8 nExpRetClass, sal_uInt8 nSpaces = 0 );
    void                AppendVarFuncTokenId( sal_uInt16 nXclFuncIdx, sal_uInt8 nRetClass, sal_uInt8 nExpRetClass, sal_uInt8 nParamCount, sal_uInt8 nSpaces = 0 );

    // token vector -----------------------------------------------------------

    void                Append( sal_uInt8 nData );
    void                Append( sal_uInt8 nData, size_t nCount );
    void                Append( sal_uInt16 nData );
    void                Append( sal_uInt32 nData );
    void                Append( double fData );
    void                Append( const String& rString );

    void                AppendAddress( const XclAddress& rXclPos );
    void                AppendRange( const XclRange& rXclRange );

    void                AppendSpaceToken( sal_uInt8 nType, sal_uInt8 nCount );
    void                AppendIntToken( sal_uInt16 nValue, sal_uInt8 nSpaces = 0 );
    void                AppendNumToken( double fValue, sal_uInt8 nSpaces = 0 );
    void                AppendBoolToken( bool bValue, sal_uInt8 nSpaces = 0 );
    void                AppendErrorToken( sal_uInt8 nErrCode, sal_uInt8 nSpaces = 0 );
    void                AppendMissingToken( sal_uInt8 nSpaces = 0 );
    void                AppendNameToken( sal_uInt16 nNameIdx, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendMissingNameToken( const String& rName, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendNameXToken( sal_uInt16 nExtSheet, sal_uInt16 nExtName, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendMacroCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendAddInFuncToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nExpClass, sal_uInt8 nSpaces = 0 );
    void                AppendParenToken( sal_uInt8 nOpenSpaces = 0, sal_uInt8 nCloseSpaces = 0 );
    void                AppendJumpToken( XclExpFuncData& rFuncData, sal_uInt8 nAttrType );

    void                Insert( sal_uInt16 nInsertPos, sal_uInt16 nInsertSize );
    void                Overwrite( sal_uInt16 nWriteToPos, sal_uInt16 nOffset );

    void                UpdateAttrGoto( sal_uInt16 nAttrPos );

    bool                IsSpaceToken( sal_uInt16 nPos ) const;
    void                RemoveTrailingParen();

    // ------------------------------------------------------------------------
private:
    typedef ::std::map< XclFormulaType, XclExpCompConfig >  XclExpCompConfigMap;
    typedef ::std::list< XclExpCompData >                   XclExpCompDataList;

    XclExpCompConfigMap maCfgMap;       /// Compiler configuration map for all formula types.
    XclFunctionProvider maFuncProv;     /// Excel function data provider.
    XclExpCompDataList  maCompDataList; /// List for working data, when compiler is called recursively.
    const XclBiff       meBiff;         /// Cached BIFF version to save GetBiff() calls.
    const SCsCOL        mnMaxAbsCol;    /// Maximum column index.
    const SCsROW        mnMaxAbsRow;    /// Maximum row index.
    const SCsCOL        mnMaxScCol;     /// Maximum column index in Calc itself.
    const SCsROW        mnMaxScRow;     /// Maximum row index in Calc itself.
    const sal_uInt16    mnMaxColMask;   /// Mask to delete invalid bits in column fields.
    const sal_uInt16    mnMaxRowMask;   /// Mask to delete invalid bits in row fields.
    bool                mbRunning;      /// true = compiler already running (for recursive calls).
};

// ----------------------------------------------------------------------------

namespace {

/** The table containing configuration data for all formula types. */
static const XclExpCompConfig spConfigTable[] =
{
    // formula type         token class type      link manager type       inCell 3dOnly StopSp allowArray
    { EXC_FMLATYPE_CELL,    EXC_CLASSTYPE_CELL,   EXC_LINKMGRTYPE_LOCAL,  true,  false, true,  true  },
    { EXC_FMLATYPE_SHARED,  EXC_CLASSTYPE_CELL,   EXC_LINKMGRTYPE_LOCAL,  true,  false, true,  true  },
    { EXC_FMLATYPE_MATRIX,  EXC_CLASSTYPE_ARRAY,  EXC_LINKMGRTYPE_LOCAL,  true,  false, true,  true  },
    { EXC_FMLATYPE_CONDFMT, EXC_CLASSTYPE_ARRAY,  EXC_LINKMGRTYPE_NONE,   false, false, true,  false },
    { EXC_FMLATYPE_DATAVAL, EXC_CLASSTYPE_ARRAY,  EXC_LINKMGRTYPE_NONE,   false, false, true,  false },
    { EXC_FMLATYPE_NAME,    EXC_CLASSTYPE_NAME,   EXC_LINKMGRTYPE_GLOBAL, false, true,  false, true  },
    { EXC_FMLATYPE_CHART,   EXC_CLASSTYPE_NAME,   EXC_LINKMGRTYPE_LOCAL,  false, true,  false, true  },
    { EXC_FMLATYPE_CONTROL, EXC_CLASSTYPE_NAME,   EXC_LINKMGRTYPE_LOCAL,  false, false, true,  false },
    { EXC_FMLATYPE_WQUERY,  EXC_CLASSTYPE_NAME,   EXC_LINKMGRTYPE_LOCAL,  false, true,  false, false },
    { EXC_FMLATYPE_LISTVAL, EXC_CLASSTYPE_NAME,   EXC_LINKMGRTYPE_NONE,   false, false, true,  false }
};

} // namespace

// ----------------------------------------------------------------------------

XclExpFmlaCompImpl::XclExpFmlaCompImpl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maFuncProv( rRoot ),
    meBiff( rRoot.GetBiff() ),
    mnMaxAbsCol( static_cast< SCsCOL >( rRoot.GetXclMaxPos().Col() ) ),
    mnMaxAbsRow( static_cast< SCsROW >( rRoot.GetXclMaxPos().Row() ) ),
    mnMaxScCol( static_cast< SCsCOL >( rRoot.GetScMaxPos().Col() ) ),
    mnMaxScRow( static_cast< SCsROW >( rRoot.GetScMaxPos().Row() ) ),
    mnMaxColMask( static_cast< sal_uInt16 >( rRoot.GetXclMaxPos().Col() ) ),
    mnMaxRowMask( static_cast< sal_uInt16 >( rRoot.GetXclMaxPos().Row() ) ),
    mbRunning( false )
{
    // build the configuration map
    for( const XclExpCompConfig* pEntry = spConfigTable; pEntry != STATIC_TABLE_END( spConfigTable ); ++pEntry )
        maCfgMap[ pEntry->meType ] = *pEntry;
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateFormula( XclFormulaType eType,
        const ScTokenArray& rScTokArr, const ScAddress* pScBasePos, XclExpRefLog* pRefLog )
{
    // initialize the compiler
    Init( eType, rScTokArr, pScBasePos, pRefLog );

    // start compilation, if initialization didn't fail
    if( mbOk )
    {
        XclExpTokenData aTokData( GetNextToken() );
        USHORT nScError = rScTokArr.GetCodeError();
        if( (nScError != 0) && (!aTokData.Is() || (aTokData.GetOpCode() == ocStop)) )
        {
            // #i50253# convert simple ocStop token to error code formula (e.g. =#VALUE!)
            AppendErrorToken( XclTools::GetXclErrorCode( nScError ), aTokData.mnSpaces );
        }
        else if( aTokData.Is() )
        {
            // expected class is VAL in cell and array formulas, and REF in names
            sal_uInt8 nExpClass = (maCfg.meClassType == EXC_CLASSTYPE_NAME) ? EXC_TOKCLASS_REF : EXC_TOKCLASS_VAL;
            aTokData = Expression( aTokData, nExpClass, maCfg.mbStopAtSep );
        }
        else
        {
            DBG_ERRORFILE( "XclExpFmlaCompImpl::CreateFormula - empty token array" );
            mbOk = false;
        }

        /*  Do unknown tokens follow? Calc accepts "comments" following a formula,
            e.g.: =1+1;"This is a comment". Ignore this without error. */
        if( mbOk )
        {
            // #i44907# auto-generated SUBTOTAL formula cells have trailing ocStop token
            bool bUnknownTail = aTokData.Is() && (aTokData.GetOpCode() != ocSep) && (aTokData.GetOpCode() != ocStop);
            DBG_ASSERT( !bUnknownTail, "XclExpFmlaCompImpl::CreateFormula - unknown garbage behind formula" );
            // Tokens left? -> error
            mbOk = !bUnknownTail;
        }
    }

    // finalizing, e.g. add tAttrVolatile token, and storing any inline arrays
    ScfUInt8Vec aExtensionTokens;
    FinalizeFormula( aExtensionTokens );

    return CreateTokenArray( &aExtensionTokens );
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
    AppendOpTokenId( nTokenId, EXC_TOKCLASS_NONE );
    Append( rXclPos.mnRow );
    Append( rXclPos.mnCol );    // do not use AppendAddress(), we always need 16-bit column here
    return CreateTokenArray();
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateNameXFormula( sal_uInt16 nExtSheet, sal_uInt16 nExtName )
{
    Init( EXC_FMLATYPE_NAME );
    AppendNameXToken( nExtSheet, nExtName, EXC_TOKCLASS_NONE );
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
    DBG_ASSERT( aIt != maCfgMap.end(), "XclExpFmlaCompImpl::GetConfigForType - unknown formula type" );
    return (aIt == maCfgMap.end()) ? 0 : &aIt->second;
}

void XclExpFmlaCompImpl::EnterRecursive()
{
    if( mbRunning )
        // compiler invoked recursively - store old working data
        maCompDataList.push_back( static_cast< const XclExpCompData& >( *this ) );
    else
        mbRunning = true;
}

void XclExpFmlaCompImpl::Init( XclFormulaType eType )
{
    // compiler invoked recursively? - store old working data
    EnterRecursive();

    // compiler configuration
    const XclExpCompConfig* pCfg = GetConfigForType( eType );
    mbOk = pCfg != 0;
    DBG_ASSERT( mbOk, "XclExpFmlaCompImpl::Init - unknown formula type" );
    if( mbOk )
    {
        // copy config data to own member
        maCfg = *pCfg;

        // reset per-formula data
        maTokVec.clear();
        mxOwnScTokArr.reset();
        maTokArrIt.Init();
        mpLinkMgr = 0;
        mpRefLog = 0;
        mxInlineArr.reset();

        mpScBasePos = 0;

        // init processing data used during compilation
        mnLastTokPos = SAL_MAX_UINT16;
        mnLastDefClass = EXC_TOKCLASS_NONE;
        mbStopAtSep = false;
        mbVolatile = false;
        mbIsArrExp = false;
        SetReplaceTokenClasses();   // initializes the token class variables for AdjustTokenClass()
    }
}

void XclExpFmlaCompImpl::Init( XclFormulaType eType, const ScTokenArray& rScTokArr,
        const ScAddress* pScBasePos, XclExpRefLog* pRefLog )
{
    // common initialization
    Init( eType );

    // special initialization
    if( mbOk ) switch( maCfg.meType )
    {
        case EXC_FMLATYPE_CELL:
        case EXC_FMLATYPE_MATRIX:
            mbOk = pScBasePos != 0;
            DBG_ASSERT( mbOk, "XclExpFmlaCompImpl::Init - missing cell address" );
            mpScBasePos = pScBasePos;
        break;
        case EXC_FMLATYPE_SHARED:
            mbOk = pScBasePos != 0;
            DBG_ASSERT( mbOk, "XclExpFmlaCompImpl::Init - missing cell address" );
            // clone the passed token array, convert references relative to current cell position
            mxOwnScTokArr.reset( rScTokArr.Clone() );
            ScCompiler::MoveRelWrap( *mxOwnScTokArr, GetDocPtr(), *pScBasePos );
            // don't remember pScBasePos in mpScBasePos, shared formulas use real relative refs
        break;
        default:;
    }

    if( mbOk )
    {
        // link manager to be used
        switch( maCfg.meLinkMgrType )
        {
            case EXC_LINKMGRTYPE_NONE:      mpLinkMgr = 0;                          break;
            case EXC_LINKMGRTYPE_LOCAL:     mpLinkMgr = &GetLocalLinkManager();     break;
            case EXC_LINKMGRTYPE_GLOBAL:    mpLinkMgr = &GetGlobalLinkManager();    break;
        }

        // token array iterator (use cloned token array if present)
        maTokArrIt.Init( mxOwnScTokArr.is() ? *mxOwnScTokArr : rScTokArr, false );
        mpRefLog = pRefLog;
    }
}

void XclExpFmlaCompImpl::LeaveRecursive()
{
    mbRunning = !maCompDataList.empty();
    if( mbRunning )
    {
        // compiler invoked recursively - restore old working data
        static_cast< XclExpCompData& >( *this ) = maCompDataList.back();
        maCompDataList.pop_back();
    }
}

void XclExpFmlaCompImpl::AppendInlineArrays( ScfUInt8Vec& rExtensionTokens )
{
    // The const_cast is needed, otherwise MS and Sun compilers can't promote
    // the non-const iterators obtained via ScMatrixList* to const iterators.
    const ScMatrixList* pList = const_cast< const ScMatrixList* >( mxInlineArr.get() );
    for( ScMatrixList::const_reverse_iterator aIt = pList->rbegin(), aEnd = pList->rend(); aIt != aEnd ; ++aIt )
    {
        const ScMatrix* pMatrix = *aIt;
        SCSIZE nC, nMaxC, nR, nMaxR;

        pMatrix->GetDimensions( nMaxC, nMaxR );

        if( meBiff == EXC_BIFF8 )
        {
            rExtensionTokens.push_back( sal::static_int_cast<const sal_uInt8>( nMaxC - 1 ) );
            rExtensionTokens.resize( rExtensionTokens.size() + 2 );
            ShortToSVBT16( static_cast< USHORT >( nMaxR - 1 ), &*(rExtensionTokens.end() - 2) );
        }
        else
        {
            rExtensionTokens.push_back( static_cast< sal_uInt8 >( (nMaxC == 256) ? 0 : nMaxC ) );
            rExtensionTokens.resize( rExtensionTokens.size() + 2 );
            ShortToSVBT16( static_cast< USHORT >( nMaxR ), &*(rExtensionTokens.end() - 2) );
        }

        for( nR = 0 ; nR < nMaxR ; nR++)
        {
            for( nC = 0 ; nC < nMaxC ; nC++)
            {
                if( pMatrix->IsValue( nC, nR ) )
                {
                    ScMatValType nType;
                    const ScMatrixValue* pVal = pMatrix->Get( nC, nR, nType);

                    if( nType == SC_MATVAL_BOOLEAN )
                    {
                        rExtensionTokens.push_back( EXC_CACHEDVAL_BOOL );

                        rExtensionTokens.resize( rExtensionTokens.size() + 8 );
                        const bool bVal = ! ::rtl::math::approxEqual( pVal->fVal, 0. );
                        UInt32ToSVBT32( bVal ? 1 : 0, &*(rExtensionTokens.end() - 8) );
                        UInt32ToSVBT32( 0, &*(rExtensionTokens.end() - 4) );
                    }
                    else
                    {
                        USHORT nErr = pVal->GetError();
                        if( nErr )
                        {
                            rExtensionTokens.push_back( EXC_CACHEDVAL_ERROR );

                            rExtensionTokens.resize( rExtensionTokens.size() + 8 );
                            UInt32ToSVBT32( XclTools::GetXclErrorCode ( nErr ),
                                            &*(rExtensionTokens.end() - 8) );
                            UInt32ToSVBT32( 0, &*(rExtensionTokens.end() - 4) );
                        }
                        else
                        {
                            rExtensionTokens.push_back( EXC_CACHEDVAL_DOUBLE );

                            const double nVal = pMatrix->GetDouble( nC, nR );
                            rExtensionTokens.resize( rExtensionTokens.size() + 8 );
                            DoubleToSVBT64( nVal, &*(rExtensionTokens.end() - 8) );
                        }
                    }
                }
                else if( pMatrix->IsEmpty( nC, nR ) )
                {
                    rExtensionTokens.push_back( EXC_CACHEDVAL_EMPTY );

                    rExtensionTokens.resize( rExtensionTokens.size() + 8 );
                    UInt32ToSVBT32( 0, &*(rExtensionTokens.end() - 8) );
                    UInt32ToSVBT32( 0, &*(rExtensionTokens.end() - 4) );
                }
                else if( pMatrix->IsString( nC, nR ) )
                {
                    rExtensionTokens.push_back( EXC_CACHEDVAL_STRING );

                    const String & rString = pMatrix->GetString( nC, nR );
                    XclExpStringRef xXclStr = XclExpStringHelper::CreateString(
                        GetRoot(), rString,
                        ((meBiff == EXC_BIFF8) ? EXC_STR_DEFAULT : EXC_STR_8BITLENGTH),
                        EXC_TOK_STR_MAXLEN );
                    size_t nSize = rExtensionTokens.size();
                    rExtensionTokens.resize( nSize + xXclStr->GetSize() );
                    xXclStr->WriteToMem( &rExtensionTokens[ nSize ] );
                }

            }
        }
    }
}

void XclExpFmlaCompImpl::FinalizeFormula( ScfUInt8Vec & rExtensionTokens )
{
    if( mbOk )
    {
        // Volatile? Add a tAttrVolatile token at the beginning of the token array.
        if( mbVolatile )
        {
            // tAttrSpace token can be extended with volatile flag
            if( !IsSpaceToken( 0 ) )
            {
                Insert( 0, 4 );
                maTokVec[ 0 ] = EXC_TOKID_ATTR;
            }
            maTokVec[ 1 ] |= EXC_TOK_ATTR_VOLATILE;
        }

        // Token array too long? -> error
        mbOk = maTokVec.size() <= EXC_TOKARR_MAXLEN;

        // Store any inline arrays
        if( mbOk && mxInlineArr.is() )
            AppendInlineArrays( rExtensionTokens );
    }

    if( !mbOk )
    {
        // Any unrecoverable error? -> Create a =#NA formula.
        maTokVec.clear();
        mbVolatile = false;
        AppendErrorToken( EXC_ERR_NA );
    }
}

XclTokenArrayRef XclExpFmlaCompImpl::CreateTokenArray( ScfUInt8Vec* pExtensionTokens )
{
    // create the Excel token array object before calling LeaveRecursive()
    XclTokenArrayRef xTokArr( new XclTokenArray( maTokVec, mbVolatile, pExtensionTokens ) );

    // compiler invoked recursively? - restore old working data
    LeaveRecursive();

    return xTokArr;
}

// compiler -------------------------------------------------------------------

const ScToken* XclExpFmlaCompImpl::GetNextRawToken()
{
    const ScToken* pScToken = maTokArrIt.Get();
    ++maTokArrIt;
    return pScToken;
}

const ScToken* XclExpFmlaCompImpl::PeekNextRawToken( bool bSkipSpaces ) const
{
    /*  Returns pointer to next raw token in the token array. The token array
        iterator already points to the next token (A call to GetNextToken()
        always increases the iterator), so this function just returns the token
        the iterator points to. To skip space tokens, a copy of the iterator is
        created and set to the passed skip-spaces mode. If spaces have to be
        skipped, and the iterator currently points to a space token, the
        constructor will move it to the next non-space token. */
    XclTokenArrayIterator aTempIt( maTokArrIt, bSkipSpaces );
    return aTempIt.Get();
}

bool XclExpFmlaCompImpl::GetNextToken( XclExpTokenData& rTokData )
{
    rTokData.mpScToken = GetNextRawToken();
    rTokData.mnSpaces = (rTokData.GetOpCode() == ocSpaces) ? rTokData.mpScToken->GetByte() : 0;
    while( rTokData.GetOpCode() == ocSpaces )
        rTokData.mpScToken = GetNextRawToken();
    return rTokData.Is();
}

XclExpTokenData XclExpFmlaCompImpl::GetNextToken()
{
    XclExpTokenData aTokData;
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

XclExpTokenData XclExpFmlaCompImpl::Expression( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool bStopAtSep )
{
    if( mbOk && aTokData.Is() )
    {
        // remember old stop-at-ocSep mode, restored below
        bool bOldStopAtSep = mbStopAtSep;
        mbStopAtSep = bStopAtSep;
        // start compilation of the subexpression
        aTokData = OrTerm( aTokData, nExpClass );
        // restore old stop-at-ocSep mode
        mbStopAtSep = bOldStopAtSep;
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::SkipExpression( XclExpTokenData aTokData, bool bStopAtSep )
{
    while( mbOk && aTokData.Is() && (aTokData.GetOpCode() != ocClose) && (!bStopAtSep || (aTokData.GetOpCode() != ocSep)) )
    {
        if( aTokData.GetOpCode() == ocOpen )
        {
            aTokData = SkipExpression( GetNextToken(), false );
            if( mbOk ) mbOk = aTokData.GetOpCode() == ocClose;
        }
        aTokData = GetNextToken();
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::OrTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = AndTerm( aTokData, nExpClass );
    sal_uInt8 nParamCount = 1;
    while( mbOk && (aTokData.GetOpCode() == ocOr) )
    {
        AdjustLastTokenClassForEastereggOp();   // see comment in this function
        RemoveTrailingParen();
        aTokData = AndTerm( GetNextToken(), EXC_TOKCLASS_REF );
        RemoveTrailingParen();
        ++nParamCount;
        if( mbOk ) mbOk = nParamCount <= EXC_FUNC_MAXPARAM;
    }
    if( mbOk && (nParamCount > 1) )
        AppendVarFuncTokenId( EXC_FUNCID_OR, EXC_TOKCLASS_VAL, nExpClass, nParamCount );
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::AndTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = CompareTerm( aTokData, nExpClass );
    sal_uInt8 nParamCount = 1;
    while( mbOk && (aTokData.GetOpCode() == ocAnd) )
    {
        AdjustLastTokenClassForEastereggOp();   // see comment in this function
        RemoveTrailingParen();
        aTokData = CompareTerm( GetNextToken(), EXC_TOKCLASS_REF );
        RemoveTrailingParen();
        ++nParamCount;
        if( mbOk ) mbOk = nParamCount <= EXC_FUNC_MAXPARAM;
    }
    if( mbOk && (nParamCount > 1) )
        AppendVarFuncTokenId( EXC_FUNCID_AND, EXC_TOKCLASS_VAL, nExpClass, nParamCount );
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::CompareTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = ConcatTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetCompareTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = ConcatTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::ConcatTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = AddSubTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetConcatTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = AddSubTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::AddSubTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = MulDivTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetAddSubTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = MulDivTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::MulDivTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = PowTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetMulDivTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = PowTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::PowTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = UnaryPostTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetPowTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = UnaryPostTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::UnaryPostTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    aTokData = UnaryPreTerm( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetUnaryPostTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, aTokData.mnSpaces );
        GetNextToken( aTokData );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::UnaryPreTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    sal_uInt8 nOpTokenId = mbOk ? lclGetUnaryPreTokenId( aTokData.GetOpCode() ) : EXC_TOKID_NONE;
    if( nOpTokenId != EXC_TOKID_NONE )
    {
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = UnaryPreTerm( GetNextToken(), nExpClass | EXC_TOKCLASS_INOP_FLAG );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
    }
    else
        aTokData = ListTerm( aTokData, nExpClass );
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::ListTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    sal_uInt16 nSubExprPos = GetSize();
    bool bHasRefOp = false;
    aTokData = IntersectTerm( aTokData, nExpClass, bHasRefOp );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetListTokenId( aTokData.GetOpCode(), mbStopAtSep )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( EXC_TOKCLASS_ANY_IN_REFOP );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = IntersectTerm( GetNextToken(), EXC_TOKCLASS_ANY_IN_REFOP, bHasRefOp );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
        bHasRefOp = true;
    }
    if( bHasRefOp )
    {
        // adjust last added token back to REF
        AdjustLastTokenClass( EXC_TOKCLASS_ANY_IN_REFOP );
        // add a tMemFunc token enclosing the entire reference subexpression
        sal_uInt16 nSubExprSize = GetSize() - nSubExprPos;
        Insert( nSubExprPos, 3 );
        maTokVec[ nSubExprPos ] = GetTokenId( EXC_TOKID_MEMFUNC, EXC_TOKCLASS_REF );
        Overwrite( nSubExprPos + 1, nSubExprSize );
        // adjust the tMemFunc token according to passed expected token class
        mnLastTokPos = nSubExprPos;
        AdjustLastTokenClass( nExpClass );
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::IntersectTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool& rbHasRefOp )
{
    aTokData = RangeTerm( aTokData, nExpClass, rbHasRefOp );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetIntersectTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( EXC_TOKCLASS_ANY_IN_REFOP );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = RangeTerm( GetNextToken(), EXC_TOKCLASS_ANY_IN_REFOP, rbHasRefOp );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
        rbHasRefOp = true;
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::RangeTerm( XclExpTokenData aTokData, sal_uInt8 nExpClass, bool& rbHasRefOp )
{
    aTokData = Factor( aTokData, nExpClass );
    sal_uInt8 nOpTokenId = EXC_TOKID_NONE;
    while( mbOk && ((nOpTokenId = lclGetRangeTokenId( aTokData.GetOpCode() )) != EXC_TOKID_NONE) )
    {
        AdjustLastTokenClass( EXC_TOKCLASS_ANY_IN_REFOP );
        sal_uInt8 nSpaces = aTokData.mnSpaces;
        aTokData = Factor( GetNextToken(), EXC_TOKCLASS_ANY_IN_REFOP );
        AppendOpTokenId( nOpTokenId, nExpClass, nSpaces );
        rbHasRefOp = true;
    }
    return aTokData;
}

XclExpTokenData XclExpFmlaCompImpl::Factor( XclExpTokenData aTokData, sal_uInt8 nExpClass )
{
    if( !mbOk || !aTokData.Is() ) return XclExpTokenData();

    StackVar eTokType = aTokData.GetType();
    OpCode eOpCode = aTokData.GetOpCode();

    switch( eTokType )
    {
        case svUnknown:     mbOk = false;                           break;
        case svDouble:      ProcessDouble( aTokData );              break;
        case svString:      ProcessString( aTokData );              break;
#if 0   // erAck
        case svError:       ProcessError( aTokData );               break;
#endif
        case svSingleRef:   ProcessCellRef( aTokData, nExpClass );  break;
        case svDoubleRef:   ProcessRangeRef( aTokData, nExpClass ); break;
        case svMatrix:      ProcessMatrix( aTokData, nExpClass );   break;
        case svExternal:    ProcessExternal( aTokData, nExpClass ); break;

        default:
        {
            switch( eOpCode )
            {
                case ocNone:        /* do nothing */                            break;
                case ocMissing:     ProcessMissing( aTokData );                 break;
                case ocBad:         ProcessBad( aTokData );                     break;
                case ocOpen:        ProcessParentheses( aTokData, nExpClass );  break;
                case ocName:        ProcessDefinedName( aTokData, nExpClass );  break;
                case ocDBArea:      ProcessDatabaseArea( aTokData, nExpClass ); break;
                case ocFalse:
                case ocTrue:        ProcessBoolean( aTokData );                 break;
                case ocDde:         ProcessDdeLink( aTokData, nExpClass );      break;
                default:            ProcessFunction( aTokData, nExpClass );
            }
        }
    }

    return GetNextToken();
}

// formula structure ----------------------------------------------------------

void XclExpFmlaCompImpl::ProcessDouble( const XclExpTokenData& rTokData )
{
    double fValue = rTokData.mpScToken->GetDouble();
    double fInt;
    double fFrac = modf( fValue, &fInt );
    if( (fFrac == 0.0) && (0.0 <= fInt) && (fInt <= 65535.0) )
        AppendIntToken( static_cast< sal_uInt16 >( fInt ), rTokData.mnSpaces );
    else
        AppendNumToken( fValue, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessString( const XclExpTokenData& rTokData )
{
    AppendOpTokenId( EXC_TOKID_STR, EXC_TOKCLASS_NONE, rTokData.mnSpaces );
    Append( rTokData.mpScToken->GetString() );
}

void XclExpFmlaCompImpl::ProcessError( const XclExpTokenData& rTokData )
{
#if 0   // erAck
    AppendErrorToken( XclTools::GetXclErrorCode( rTokData.mpScToken->GetError() ), rTokData.mnSpaces );
#else
    (void)rTokData; // compiler warning
#endif
}

void XclExpFmlaCompImpl::ProcessMissing( const XclExpTokenData& rTokData )
{
    AppendMissingToken( rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessBad( const XclExpTokenData& rTokData )
{
    AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessParentheses( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    XclExpTokenData aTokData( Expression( GetNextToken(), nExpClass, false ) );
    mbOk = aTokData.GetOpCode() == ocClose;
    AppendParenToken( rTokData.mnSpaces, aTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessBoolean( const XclExpTokenData& rTokData )
{
    mbOk = GetNextToken().GetOpCode() == ocOpen;
    if( mbOk ) mbOk = GetNextToken().GetOpCode() == ocClose;
    if( mbOk )
        AppendBoolToken( rTokData.GetOpCode() == ocTrue, rTokData.mnSpaces );
}

namespace {

inline bool lclGetTokenString( String& rString, const XclExpTokenData& rTokData )
{
    bool bIsStr = (rTokData.GetType() == svString) && (rTokData.GetOpCode() == ocPush);
    if( bIsStr )
        rString = rTokData.mpScToken->GetString();
    return bIsStr;
}

} // namespace

void XclExpFmlaCompImpl::ProcessDdeLink( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    String aApplic, aTopic, aItem;

    mbOk = GetNextToken().GetOpCode() == ocOpen;
    if( mbOk ) mbOk = lclGetTokenString( aApplic, GetNextToken() );
    if( mbOk ) mbOk = GetNextToken().GetOpCode() == ocSep;
    if( mbOk ) mbOk = lclGetTokenString( aTopic, GetNextToken() );
    if( mbOk ) mbOk = GetNextToken().GetOpCode() == ocSep;
    if( mbOk ) mbOk = lclGetTokenString( aItem, GetNextToken() );
    if( mbOk ) mbOk = GetNextToken().GetOpCode() == ocClose;
    if( mbOk ) mbOk = aApplic.Len() && aTopic.Len() && aItem.Len();
    if( mbOk )
    {
        sal_uInt16 nExtSheet, nExtName;
        if( mpLinkMgr && mpLinkMgr->InsertDde( nExtSheet, nExtName, aApplic, aTopic, aItem ) )
            AppendNameXToken( nExtSheet, nExtName, nExpClass, rTokData.mnSpaces );
        else
            AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
    }
}

void XclExpFmlaCompImpl::ProcessExternal( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    /*  #i47228# Excel import generates svExternal/ocMacro tokens for invalid
        names and for external/invalid function calls. This function looks for
        the next token in the token array. If it is an opening parenthesis, the
        token is processed as external function call, otherwise as undefined name. */
    const ScToken* pNextScToken = PeekNextRawToken( true );
    if( !pNextScToken || (pNextScToken->GetOpCode() != ocOpen) )
        AppendMissingNameToken( rTokData.mpScToken->GetExternal(), nExpClass, rTokData.mnSpaces );
    else
        ProcessFunction( rTokData, nExpClass );
}

void XclExpFmlaCompImpl::ProcessFunction( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    OpCode eOpCode = rTokData.GetOpCode();
    const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromOpCode( eOpCode );

    XclExpExtFuncData aExtFuncData;

    // no exportable function found - try to create an external macro call
    if( !pFuncInfo && (eOpCode >= SC_OPCODE_START_NO_PAR) )
    {
        const String& rFuncName = ScCompiler::GetStringFromOpCode( eOpCode );
        if( rFuncName.Len() )
        {
            aExtFuncData.Set( rFuncName, true, false );
            pFuncInfo = maFuncProv.GetFuncInfoFromOpCode( ocMacro );
        }
    }

    mbOk = pFuncInfo != 0;
    if( !mbOk ) return;

    // functions simulated by a macro call in file format
    if( pFuncInfo->IsMacroFunc() )
        aExtFuncData.Set( pFuncInfo->GetMacroFuncName(), false, true );

    XclExpFuncData aFuncData( rTokData, *pFuncInfo, aExtFuncData, nExpClass );
    XclExpTokenData aTokData;

    // preparations for special functions, before function processing starts
    PrepareFunction( aFuncData );

    enum { STATE_START, STATE_OPEN, STATE_PARAM, STATE_SEP, STATE_CLOSE, STATE_END }
        eState = STATE_START;
    while( eState != STATE_END ) switch( eState )
    {
        case STATE_START:
            mbOk = GetNextToken( aTokData ) && (aTokData.GetOpCode() == ocOpen);
            eState = mbOk ? STATE_OPEN : STATE_END;
        break;
        case STATE_OPEN:
            mbOk = GetNextToken( aTokData );
            eState = mbOk ? ((aTokData.GetOpCode() == ocClose) ? STATE_CLOSE : STATE_PARAM) : STATE_END;
        break;
        case STATE_PARAM:
            aTokData = ProcessParam( aTokData, aFuncData );
            switch( aTokData.GetOpCode() )
            {
                case ocSep:     eState = STATE_SEP;                 break;
                case ocClose:   eState = STATE_CLOSE;               break;
                default:        mbOk = false;
            }
            if( !mbOk ) eState = STATE_END;
        break;
        case STATE_SEP:
            mbOk = (aFuncData.GetParamCount() < EXC_FUNC_MAXPARAM) && GetNextToken( aTokData );
            eState = mbOk ? STATE_PARAM : STATE_END;
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
    switch( rFuncData.GetOpCode() )
    {
        case ocCot:                     // simulate COT(x) by (1/TAN(x))
        case ocCotHyp:                  // simulate COTH(x) by (1/TANH(x))
            AppendIntToken( 1 );
        break;
        case ocArcCot:                  // simulate ACOT(x) by (PI/2-ATAN(x))
            AppendNumToken( F_PI2 );
        break;
        default:;
    }
}

void XclExpFmlaCompImpl::FinishFunction( XclExpFuncData& rFuncData, sal_uInt8 nCloseSpaces )
{
    // append missing parameters required in Excel, may modify param count
    AppendTrailingParam( rFuncData );

    // check if parameter count fits into the limits of the function
    sal_uInt8 nParamCount = rFuncData.GetParamCount();
    sal_uInt8 nMinCount = rFuncData.GetMinParamCount();
    sal_uInt8 nMaxCount = rFuncData.GetMaxParamCount();
    if( (nMinCount <= nParamCount) && (nParamCount <= nMaxCount) )
    {
        sal_uInt16 nXclFuncIdx = rFuncData.GetXclFuncIdx();
        // first put the tAttrSpace tokens, they must not be included in tAttrGoto handling
        AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP_CLOSE, nCloseSpaces );
        AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP, rFuncData.GetSpaces() );

        // add tAttrGoto tokens for IF or CHOOSE functions
        switch( rFuncData.GetOpCode() )
        {
            case ocIf:
            case ocChose:
                AppendJumpToken( rFuncData, EXC_TOK_ATTR_GOTO );
            break;
            default:;
        }

        // put the tFunc or tFuncVar token (or another special token, e.g. tAttrSum)
        sal_uInt8 nRetClass = rFuncData.GetReturnClass();
        sal_uInt8 nExpRetClass = rFuncData.GetExpReturnClass();
        if( (nXclFuncIdx == EXC_FUNCID_SUM) && (nParamCount == 1) )
        {
            // SUM with only one parameter
            AppendOpTokenId( EXC_TOKID_ATTR, nExpRetClass );
            Append( EXC_TOK_ATTR_SUM );
            Append( sal_uInt16( 0 ) );
        }
        else if( (nMinCount == nMaxCount) && (nXclFuncIdx != EXC_FUNCID_EXTERNCALL) )
        {
            // fixed number of parameters
            AppendOpTokenId( GetTokenId( EXC_TOKID_FUNC, nRetClass ), nExpRetClass );
            Append( nXclFuncIdx );
        }
        else
        {
            // variable number of parameters
            AppendOpTokenId( GetTokenId( EXC_TOKID_FUNCVAR, nRetClass ), nExpRetClass );
            Append( nParamCount );
            Append( nXclFuncIdx );
        }

        // update volatile flag - is set if at least one used function is volatile
        mbVolatile |= rFuncData.IsVolatile();

        // update jump tokens for specific functions, add additional tokens
        switch( rFuncData.GetOpCode() )
        {
            case ocIf:
                FinishIfFunction( rFuncData );
            break;
            case ocChose:
                FinishChooseFunction( rFuncData );
            break;

            case ocCot:                     // simulate COT(x) by (1/TAN(x))
            case ocCotHyp:                  // simulate COTH(x) by (1/TANH(x))
                AppendOpTokenId( EXC_TOKID_DIV, EXC_TOKCLASS_NONE );
                AppendParenToken();
            break;
            case ocArcCot:                  // simulate ACOT(x) by (PI/2-ATAN(x))
                AppendOpTokenId( EXC_TOKID_SUB, EXC_TOKCLASS_NONE );
                AppendParenToken();
            break;

            default:;
        }
    }
    else
        mbOk = false;
}

void XclExpFmlaCompImpl::FinishIfFunction( XclExpFuncData& rFuncData )
{
    sal_uInt16 nParamCount = rFuncData.GetParamCount();
    DBG_ASSERT( (nParamCount == 2) || (nParamCount == 3), "XclExpFmlaCompImpl::FinishIfFunction - wrong parameter count" );
    const ScfUInt16Vec& rAttrPos = rFuncData.GetAttrPosVec();
    DBG_ASSERT( nParamCount == rAttrPos.size(), "XclExpFmlaCompImpl::FinishIfFunction - wrong number of tAttr tokens" );
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
    DBG_ASSERT( nParamCount == rAttrPos.size(), "XclExpFmlaCompImpl::FinishChooseFunction - wrong number of tAttr tokens" );
    // number of choices is parameter count minus 1
    sal_uInt16 nChoices = nParamCount - 1;
    // tAttrChoose token contains number of choices
    Overwrite( rAttrPos[ 0 ] + 2, nChoices );
    // cache position of the jump table (follows number of choices in tAttrChoose token)
    sal_uInt16 nJumpArrPos = rAttrPos[ 0 ] + 4;
    // size of jump table: number of choices, plus 1 for error position
    sal_uInt16 nJumpArrSize = 2 * (nChoices + 1);
    // insert the jump table into the tAttrChoose token
    Insert( nJumpArrPos, nJumpArrSize );
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

XclExpTokenData XclExpFmlaCompImpl::ProcessParam( XclExpTokenData aTokData, XclExpFuncData& rFuncData )
{
    if( rFuncData.GetExpParamClass() == EXC_FUNC_PAR_CALCONLY )
    {
        // skip Calc-only parameter, stop at next ocClose or ocSep
        aTokData = SkipExpression( aTokData, true );
        rFuncData.IncExpParamClassIdx();
    }
    else
    {
        // insert Excel-only parameters, modifies param count and class in rFuncData
        while( rFuncData.GetExpParamClass() == EXC_FUNC_PAR_EXCELONLY )
            AppendDefaultParam( rFuncData );

        // propagate expected ARR class to subsequent subexpressions
        sal_uInt8 nExpClass = rFuncData.GetExpParamClass();
        bool bOldIsArrExp = mbIsArrExp;
        UpdateArrExpFlag( nExpClass, rFuncData.GetReturnClass() );

        // process the parameter, stop at next ocClose or ocSep
        PrepareParam( rFuncData );
        /*  #i37355# insert tMissArg token for missing parameters --
            Excel import filter adds ocMissing token (handled in Factor()),
            but Calc itself does not do this if a new formula is entered. */
        switch( aTokData.GetOpCode() )
        {
            case ocSep:
            case ocClose:   AppendMissingToken();   break;  // empty parameter
            default:        aTokData = Expression( aTokData, nExpClass, true );
        }
        // restore old expected ARR class mode
        SetArrExpFlag( bOldIsArrExp );
        // finalize the parameter and add special tokens, e.g. for IF or CHOOSE parameters
        if( mbOk ) FinishParam( rFuncData );
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

        case ocChose:
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
    // index of this parameter is equal to number of already finished parameters
    sal_uInt8 nParamIdx = rFuncData.GetParamCount();

    // increase parameter count
    rFuncData.IncParamCount();
    // move to next expected parameter class
    rFuncData.IncExpParamClassIdx();

    switch( rFuncData.GetOpCode() )
    {
        case ocArcCotHyp:               // simulate ACOTH(x) by ATANH(1/(x))
            if( nParamIdx == 0 )
            {
                AppendParenToken();
                AppendOpTokenId( EXC_TOKID_DIV, EXC_TOKCLASS_NONE );
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
        case ocAddress:
            AppendBoolToken( true );
        break;
        case ocExternal:
            AppendAddInFuncToken( rFuncData.GetExtFuncData(), EXC_TOKCLASS_REF );
        break;
        case ocMacro:
            AppendMacroCallToken( rFuncData.GetExtFuncData(), EXC_TOKCLASS_REF );
        break;
        default:
        {
            DBG_ASSERT( rFuncData.IsMacroFunc(), "XclExpFmlaCompImpl::AppendDefaultParam - unknown opcode" );
            if( rFuncData.IsMacroFunc() )
                AppendMacroCallToken( rFuncData.GetExtFuncData(), EXC_TOKCLASS_REF );
            else
                AppendMissingToken();   // to keep parameter count valid
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
                // #112262# Excel needs at least two parameters in IF function
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

        default:;
    }
}

// reference handling ---------------------------------------------------------

namespace {

inline bool lclIsRefRel2D( const SingleRefData& rRefData )
{
    return rRefData.IsColRel() || rRefData.IsRowRel();
}

inline bool lclIsRefDel2D( const SingleRefData& rRefData )
{
    return rRefData.IsColDeleted() || rRefData.IsRowDeleted();
}

inline bool lclIsRefRel2D( const ComplRefData& rRefData )
{
    return lclIsRefRel2D( rRefData.Ref1 ) || lclIsRefRel2D( rRefData.Ref2 );
}

inline bool lclIsRefDel2D( const ComplRefData& rRefData )
{
    return lclIsRefDel2D( rRefData.Ref1 ) || lclIsRefDel2D( rRefData.Ref2 );
}

} // namespace

// ----------------------------------------------------------------------------

SCTAB XclExpFmlaCompImpl::GetScTab( const SingleRefData& rRefData ) const
{
    bool bInvTab = rRefData.IsTabDeleted() || (!mpScBasePos && rRefData.IsTabRel());
    return bInvTab ? SCTAB_INVALID : static_cast< SCTAB >( rRefData.nTab );
}

bool XclExpFmlaCompImpl::IsRef2D( const SingleRefData& rRefData ) const
{
    /*  rRefData.IsFlag3D() determines if sheet name is always visible, even on the
        own sheet. If 3D references are allowed, the passed reference does not count
        as 2D reference. If only 2D references are allowed (mpLinkMgr is 0), this
        flag is ignored, thus the passed reference will be handled as 2D reference. */
    return (!mpLinkMgr || !rRefData.IsFlag3D()) && !rRefData.IsTabDeleted() &&
        (rRefData.IsTabRel() ? (rRefData.nRelTab == 0) : (static_cast< SCTAB >( rRefData.nTab ) == GetCurrScTab()));
}

bool XclExpFmlaCompImpl::IsRef2D( const ComplRefData& rRefData ) const
{
    return IsRef2D( rRefData.Ref1 ) && IsRef2D( rRefData.Ref2 );
}

void XclExpFmlaCompImpl::ConvertRefData(
    SingleRefData& rRefData, XclAddress& rXclPos,
    bool bNatLangRef, bool bTruncMaxCol, bool bTruncMaxRow ) const
{
    if( mpScBasePos )
    {
        // *** reference position exists (cell, matrix) - convert to absolute ***
        rRefData.CalcAbsIfRel( *mpScBasePos );

        // convert column index
        SCsCOL& rnScCol = rRefData.nCol;
        if( bTruncMaxCol && (rnScCol == mnMaxScCol) )
            rnScCol = mnMaxAbsCol;
        else if( (rnScCol < 0) || (rnScCol > mnMaxAbsCol) )
            rRefData.SetColDeleted( TRUE );
        rXclPos.mnCol = static_cast< sal_uInt16 >( rnScCol ) & mnMaxColMask;

        // convert row index
        SCsROW& rnScRow = rRefData.nRow;
        if( bTruncMaxRow && (rnScRow == mnMaxScRow) )
            rnScRow = mnMaxAbsRow;
        else if( (rnScRow < 0) || (rnScRow > mnMaxAbsRow) )
            rRefData.SetRowDeleted( TRUE );
        rXclPos.mnRow = static_cast< sal_uInt16 >( rnScRow ) & mnMaxRowMask;
    }
    else
    {
        // *** no reference position (shared, names, condfmt) - use relative values ***

        // convert column index (2-step-cast ScsCOL->sal_Int16->sal_uInt16 to get all bits correctly)
        sal_Int16 nXclRelCol = static_cast< sal_Int16 >( rRefData.IsColRel() ? rRefData.nRelCol : rRefData.nCol );
        rXclPos.mnCol = static_cast< sal_uInt16 >( nXclRelCol ) & mnMaxColMask;

        // convert row index (2-step-cast ScsROW->sal_Int16->sal_uInt16 to get all bits correctly)
        sal_Int16 nXclRelRow = static_cast< sal_Int16 >( rRefData.IsRowRel() ? rRefData.nRelRow : rRefData.nRow );
        rXclPos.mnRow = static_cast< sal_uInt16 >( nXclRelRow ) & mnMaxRowMask;
    }

    // flags for relative column and row
    if( bNatLangRef )
    {
        DBG_ASSERT( meBiff == EXC_BIFF8, "XclExpFmlaCompImpl::ConvertRefData - NLRs only for BIFF8" );
        // Calc does not support absolute reference mode in natural language references
        ::set_flag( rXclPos.mnCol, EXC_TOK_NLR_REL );
    }
    else
    {
        sal_uInt16& rnRelField = (meBiff <= EXC_BIFF5) ? rXclPos.mnRow : rXclPos.mnCol;
        ::set_flag( rnRelField, EXC_TOK_REF_COLREL, rRefData.IsColRel() );
        ::set_flag( rnRelField, EXC_TOK_REF_ROWREL, rRefData.IsRowRel() );
    }
}

void XclExpFmlaCompImpl::ConvertRefData(
        ComplRefData& rRefData, XclRange& rXclRange, bool bNatLangRef ) const
{
    // convert start and end of the range
    ConvertRefData( rRefData.Ref1, rXclRange.maFirst, bNatLangRef, false, false );
    bool bTruncMaxCol = !rRefData.Ref1.IsColDeleted() && (rRefData.Ref1.nCol == 0);
    bool bTruncMaxRow = !rRefData.Ref1.IsRowDeleted() && (rRefData.Ref1.nRow == 0);
    ConvertRefData( rRefData.Ref2, rXclRange.maLast, bNatLangRef, bTruncMaxCol, bTruncMaxRow );
}

XclExpRefLogEntry* XclExpFmlaCompImpl::GetNewRefLogEntry()
{
    if( mpRefLog )
    {
        mpRefLog->resize( mpRefLog->size() + 1 );
        return &mpRefLog->back();
    }
    return 0;
}

void XclExpFmlaCompImpl::ProcessCellRef( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    // get the Excel address components, adjust internal data in aRefData
    bool bNatLangRef = (meBiff == EXC_BIFF8) && mpScBasePos && (rTokData.GetOpCode() == ocColRowName);
    SingleRefData aRefData( rTokData.mpScToken->GetSingleRef() );
    XclAddress aXclPos( ScAddress::UNINITIALIZED );
    ConvertRefData( aRefData, aXclPos, bNatLangRef, false, false );

    if( bNatLangRef )
    {
        DBG_ASSERT( aRefData.IsColRel() != aRefData.IsRowRel(),
            "XclExpFmlaCompImpl::ProcessCellRef - broken natural language reference" );
        // create tNlr token for natural language reference
        sal_uInt8 nSubId = aRefData.IsColRel() ? EXC_TOK_NLR_COLV : EXC_TOK_NLR_ROWV;
        AppendOpTokenId( EXC_TOKID_NLR, nExpClass, rTokData.mnSpaces );
        Append( nSubId );
        AppendAddress( aXclPos );
    }
    else
    {
        // store external cell contents in CRN records
        if( maCfg.mbFromCell && mpLinkMgr && mpScBasePos )
            mpLinkMgr->StoreCell( aRefData );

        // create the tRef, tRefErr, tRefN, tRef3d, or tRefErr3d token
        if( !maCfg.mb3DRefOnly && IsRef2D( aRefData ) )
        {
            // 2D reference (not in defined names, but allowed in range lists)
            sal_uInt8 nBaseId = (!mpScBasePos && lclIsRefRel2D( aRefData )) ? EXC_TOKID_REFN :
                (lclIsRefDel2D( aRefData ) ? EXC_TOKID_REFERR : EXC_TOKID_REF);
            AppendOpTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), nExpClass, rTokData.mnSpaces );
            AppendAddress( aXclPos );
        }
        else if( mpLinkMgr )    // 3D reference
        {
            // 1-based EXTERNSHEET index and 0-based Excel sheet index
            sal_uInt16 nExtSheet, nXclTab;
            mpLinkMgr->FindExtSheet( nExtSheet, nXclTab, GetScTab( aRefData ), GetNewRefLogEntry() );
            // write the token
            sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_REFERR3D : EXC_TOKID_REF3D;
            AppendOpTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), nExpClass, rTokData.mnSpaces );
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

void XclExpFmlaCompImpl::ProcessRangeRef( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    // get the Excel address components, adjust internal data in aRefData
    ComplRefData aRefData( rTokData.mpScToken->GetDoubleRef() );
    XclRange aXclRange( ScAddress::UNINITIALIZED );
    ConvertRefData( aRefData, aXclRange, false );

    // store external cell contents in CRN records
    if( maCfg.mbFromCell && mpLinkMgr && mpScBasePos )
        mpLinkMgr->StoreCellRange( aRefData );

    // create the tArea, tAreaErr, tAreaN, tArea3d, or tAreaErr3d token
    if( !maCfg.mb3DRefOnly && IsRef2D( aRefData ) )
    {
        // 2D reference (not in name formulas, but allowed in range lists)
        sal_uInt8 nBaseId = (!mpScBasePos && lclIsRefRel2D( aRefData )) ? EXC_TOKID_AREAN :
             (lclIsRefDel2D( aRefData ) ? EXC_TOKID_AREAERR : EXC_TOKID_AREA);
        AppendOpTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), nExpClass, rTokData.mnSpaces );
        AppendRange( aXclRange );
    }
    else if( mpLinkMgr )    // 3D reference
    {
        // 1-based EXTERNSHEET index and 0-based Excel sheet indexes
        sal_uInt16 nExtSheet, nFirstXclTab, nLastXclTab;
        mpLinkMgr->FindExtSheet( nExtSheet, nFirstXclTab, nLastXclTab,
            GetScTab( aRefData.Ref1 ), GetScTab( aRefData.Ref2 ), GetNewRefLogEntry() );
        // write the token
        sal_uInt8 nBaseId = lclIsRefDel2D( aRefData ) ? EXC_TOKID_AREAERR3D : EXC_TOKID_AREA3D;
        AppendOpTokenId( GetTokenId( nBaseId, EXC_TOKCLASS_REF ), nExpClass, rTokData.mnSpaces );
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

void XclExpFmlaCompImpl::ProcessDefinedName( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    XclExpNameManager& rNameMgr = GetNameManager();
    sal_uInt16 nNameIdx = rNameMgr.InsertName( rTokData.mpScToken->GetIndex() );
    if( nNameIdx != 0 )
    {
        // global names always with tName token, local names dependent on config
        SCTAB nScTab = rNameMgr.GetScTab( nNameIdx );
        if( (nScTab == SCTAB_GLOBAL) || (!maCfg.mb3DRefOnly && (nScTab == GetCurrScTab())) )
        {
            AppendNameToken( nNameIdx, nExpClass, rTokData.mnSpaces );
        }
        else if( mpLinkMgr )
        {
            // use the same special EXTERNNAME to refer to any local name
            sal_uInt16 nExtSheet = mpLinkMgr->FindExtSheet( EXC_EXTSH_OWNDOC );
            AppendNameXToken( nExtSheet, nNameIdx, nExpClass, rTokData.mnSpaces );
        }
        else
            AppendErrorToken( EXC_ERR_NAME, rTokData.mnSpaces );
        // volatile names (containing volatile functions)
        mbVolatile |= rNameMgr.IsVolatile( nNameIdx );
    }
    else
        AppendErrorToken( EXC_ERR_NAME, rTokData.mnSpaces );
}

void XclExpFmlaCompImpl::ProcessDatabaseArea( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    sal_uInt16 nNameIdx = GetNameManager().InsertDBRange( rTokData.mpScToken->GetIndex() );
    AppendNameToken( nNameIdx, nExpClass, rTokData.mnSpaces );
}

// token identifiers ----------------------------------------------------------

void XclExpFmlaCompImpl::SetReplaceTokenClasses()
{
    /*  REF expected: change VAL to ARR in array and name type, or if ARR is expected somewhere before.
        Example: How does the return class of PI() (default VAL) change in SUM() (expects REF)?
            =SUM(PI())          -> PI() still returns VAL in cell formula
            {=SUM(PI())}        -> PI() returns ARR in array formula
            =MDET(SUM(PI()))    -> expected ARR from MDET(), PI() returns ARR too
     */
    mnRefExpClass = (mbIsArrExp || (maCfg.meClassType != EXC_CLASSTYPE_CELL)) ? EXC_TOKCLASS_ARR : EXC_TOKCLASS_VAL;

    /*  VAL expected: set to ARR, if ARR is expected somewhere before; otherwise set to VAL.
        Example: How does the class of A1 (default REF) change in ABS() (expects VAL)?
            =ABS(A1)            -> A1 is VAL in cell formula
            =MDET(ABS(A1))      -> expected ARR from MDET(), A1 is ARR in cell formula
     */
    mnValExpClass = mbIsArrExp ? EXC_TOKCLASS_ARR : EXC_TOKCLASS_VAL;

    /* ARR expected: always set to ARR. */
    mnArrExpClass = EXC_TOKCLASS_ARR;
}

void XclExpFmlaCompImpl::SetArrExpFlag( bool bIsArrExp )
{
    if( mbIsArrExp != bIsArrExp )
    {
        mbIsArrExp = bIsArrExp;
        SetReplaceTokenClasses();
    }
}

void XclExpFmlaCompImpl::UpdateArrExpFlag( sal_uInt8 nParamExpClass, sal_uInt8 nFuncRetClass )
{
    bool bNewIsArrExp = false;
    switch( maCfg.meClassType )
    {
        case EXC_CLASSTYPE_CELL:
            bNewIsArrExp = nParamExpClass == EXC_TOKCLASS_ARR;
        break;
        case EXC_CLASSTYPE_ARRAY:
            bNewIsArrExp = nParamExpClass != EXC_TOKCLASS_VAL;
        break;
        case EXC_CLASSTYPE_NAME:
            bNewIsArrExp = ((nParamExpClass != EXC_TOKCLASS_VAL) || (nFuncRetClass != EXC_TOKCLASS_REF));
        break;

        default :
        break;
    }
    SetArrExpFlag( mbIsArrExp || bNewIsArrExp );
}

void XclExpFmlaCompImpl::AdjustTokenClass( sal_uInt8& rnTokenId, sal_uInt8 nExpClass )
{
    sal_uInt8 nIsClass = GetTokenClass( rnTokenId );
    if( nIsClass != EXC_TOKCLASS_NONE )
    {
        if( nExpClass == EXC_TOKCLASS_ANY_IN_REFOP )
        {
            // always set to REF, if used by reference operators
            ChangeTokenClass( rnTokenId, EXC_TOKCLASS_REF );
        }
        else
        {
            /*  If a REF token is part of a value operator, it behaves like a VAL token.
                e.g.:   =SUM(A1)    -> SUM() expects REF, A1 is REF.
                        =SUM(A1+A1) -> SUM() expects REF, but both A1 are handled like VAL tokens. */
            if( (nIsClass == EXC_TOKCLASS_REF) && ::get_flag( nExpClass, EXC_TOKCLASS_INOP_FLAG ) )
            {
                ChangeTokenClass( rnTokenId, EXC_TOKCLASS_VAL );
                nIsClass = EXC_TOKCLASS_VAL;    // update nIsClass for following switch
            }

            // change token class according to expected parameter class
            switch( nExpClass )
            {
                case EXC_TOKCLASS_REF:
                case EXC_TOKCLASS_REF_IN_VALOP:
                    if( nIsClass == EXC_TOKCLASS_VAL )
                        ChangeTokenClass( rnTokenId, mnRefExpClass );
                break;
                case EXC_TOKCLASS_VAL:
                case EXC_TOKCLASS_VAL_IN_VALOP:
                    ChangeTokenClass( rnTokenId, mnValExpClass );
                break;
                case EXC_TOKCLASS_ARR:
                case EXC_TOKCLASS_ARR_IN_VALOP:
                    ChangeTokenClass( rnTokenId, mnArrExpClass );
                break;
            }
        }
    }
}

void XclExpFmlaCompImpl::AdjustLastTokenClass( sal_uInt8 nExpClass )
{
    DBG_ASSERT( mnLastTokPos < GetSize(), "XclExpFmlaCompImpl::AdjustLastTokenClass - invalid position" );
    sal_uInt8& rnTokenId = maTokVec[ mnLastTokPos ];
    mnLastDefClass = GetTokenClass( rnTokenId );
    AdjustTokenClass( rnTokenId, nExpClass );
}

void XclExpFmlaCompImpl::AdjustLastTokenClassForEastereggOp()
{
    /*  This very special function cares about the leading subexpression of the
        Calc easteregg operators OR and AND.
        Example: The Calc formula =(A1:A2)OR(0) will be compiled to the Excel
        formula =OR(A1:A2,0). The Excel OR function expects REF parameters to
        be able to process all cells in a range reference. Since this compiler
        didn't know this when processing the (A1:A2) subexpression, it may
        create a tAreaV token from the reference (e.g. if in cell context).
        This would cause Excel to only evaluate cell A1, and to ignore cell A2,
        if the formula is located in cell B1.
        So this function changes the last token back to its default class and
        adjusts it with expected REF class, which is what would happen, if a
        regular OR or AND function is processed. */
    DBG_ASSERT( mnLastTokPos < GetSize(), "XclExpFmlaCompImpl::AdjustLastTokenClassForEastereggOp - invalid position" );
    sal_uInt8& rnTokenId = maTokVec[ mnLastTokPos ];
    if( GetTokenClass( rnTokenId ) != EXC_TOKCLASS_NONE )
    {
        ChangeTokenClass( rnTokenId, mnLastDefClass );
        AdjustTokenClass( rnTokenId, EXC_TOKCLASS_REF );
    }
}

void XclExpFmlaCompImpl::AppendOpTokenId( sal_uInt8 nTokenId, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    AppendSpaceToken( EXC_TOK_ATTR_SPACE_SP, nSpaces );
    mnLastTokPos = GetSize();
    Append( nTokenId );
    AdjustLastTokenClass( nExpClass );
}

void XclExpFmlaCompImpl::AppendFuncTokenId(
        sal_uInt16 nXclFuncIdx, sal_uInt8 nRetClass, sal_uInt8 nExpRetClass, sal_uInt8 nSpaces )
{
    AppendOpTokenId( GetTokenId( EXC_TOKID_FUNC, nRetClass ), nExpRetClass, nSpaces );
    Append( nXclFuncIdx );
}

void XclExpFmlaCompImpl::AppendVarFuncTokenId(
        sal_uInt16 nXclFuncIdx, sal_uInt8 nRetClass, sal_uInt8 nExpRetClass,
        sal_uInt8 nParamCount, sal_uInt8 nSpaces )
{
    AppendOpTokenId( GetTokenId( EXC_TOKID_FUNCVAR, nRetClass ), nExpRetClass, nSpaces );
    Append( nParamCount );
    Append( nXclFuncIdx );
}

// token vector ---------------------------------------------------------------

void XclExpFmlaCompImpl::Append( sal_uInt8 nData )
{
    maTokVec.push_back( nData );
}

void XclExpFmlaCompImpl::Append( sal_uInt8 nData, size_t nCount )
{
    maTokVec.resize( maTokVec.size() + nCount, nData );
}

void XclExpFmlaCompImpl::Append( sal_uInt16 nData )
{
    maTokVec.resize( maTokVec.size() + 2 );
    ShortToSVBT16( nData, &*(maTokVec.end() - 2) );
}

void XclExpFmlaCompImpl::Append( sal_uInt32 nData )
{
    maTokVec.resize( maTokVec.size() + 4 );
    UInt32ToSVBT32( nData, &*(maTokVec.end() - 4) );
}

void XclExpFmlaCompImpl::Append( double fData )
{
    maTokVec.resize( maTokVec.size() + 8 );
    DoubleToSVBT64( fData, &*(maTokVec.end() - 8) );
}

void XclExpFmlaCompImpl::Append( const String& rString )
{
    XclExpStringRef xXclStr = XclExpStringHelper::CreateString(
        GetRoot(), rString, EXC_STR_8BITLENGTH, EXC_TOK_STR_MAXLEN );
    size_t nSize = maTokVec.size();
    maTokVec.resize( nSize + xXclStr->GetSize() );
    xXclStr->WriteToMem( &maTokVec[ nSize ] );
}

void XclExpFmlaCompImpl::AppendAddress( const XclAddress& rXclPos )
{
    Append( rXclPos.mnRow );
    if( meBiff <= EXC_BIFF5 )
        Append( static_cast< sal_uInt8 >( rXclPos.mnCol ) );
    else
        Append( rXclPos.mnCol );
}

void XclExpFmlaCompImpl::AppendRange( const XclRange& rXclRange )
{
    Append( rXclRange.maFirst.mnRow );
    Append( rXclRange.maLast.mnRow );
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
    if( nCount )
    {
        Append( EXC_TOKID_ATTR );
        Append( EXC_TOK_ATTR_SPACE );
        Append( nType );
        Append( nCount );
    }
}

void XclExpFmlaCompImpl::AppendIntToken( sal_uInt16 nValue, sal_uInt8 nSpaces )
{
    AppendOpTokenId( EXC_TOKID_INT, EXC_TOKCLASS_NONE, nSpaces );
    Append( nValue );
}

void XclExpFmlaCompImpl::AppendNumToken( double fValue, sal_uInt8 nSpaces )
{
    AppendOpTokenId( EXC_TOKID_NUM, EXC_TOKCLASS_NONE, nSpaces );
    Append( fValue );
}

void XclExpFmlaCompImpl::AppendBoolToken( bool bValue, sal_uInt8 nSpaces )
{
    AppendOpTokenId( EXC_TOKID_BOOL, EXC_TOKCLASS_NONE, nSpaces );
    Append( bValue ? EXC_TOK_BOOL_TRUE : EXC_TOK_BOOL_FALSE );
}

void XclExpFmlaCompImpl::AppendErrorToken( sal_uInt8 nErrCode, sal_uInt8 nSpaces )
{
    AppendOpTokenId( EXC_TOKID_ERR, EXC_TOKCLASS_NONE, nSpaces );
    Append( nErrCode );
}

void XclExpFmlaCompImpl::AppendMissingToken( sal_uInt8 nSpaces )
{
    AppendOpTokenId( EXC_TOKID_MISSARG, EXC_TOKCLASS_NONE, nSpaces );
}

void XclExpFmlaCompImpl::AppendNameToken( sal_uInt16 nNameIdx, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    if( nNameIdx > 0 )
    {
        AppendOpTokenId( GetTokenId( EXC_TOKID_NAME, EXC_TOKCLASS_REF ), nExpClass, nSpaces );
        Append( nNameIdx );
        Append( 0, (meBiff <= EXC_BIFF5) ? 12 : 2 );
    }
    else
        AppendErrorToken( EXC_ERR_NAME );
}

void XclExpFmlaCompImpl::AppendMissingNameToken( const String& rName, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    sal_uInt16 nNameIdx = GetNameManager().InsertRawName( rName );
    AppendNameToken( nNameIdx, nExpClass, nSpaces );
}

void XclExpFmlaCompImpl::AppendNameXToken( sal_uInt16 nExtSheet, sal_uInt16 nExtName, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    AppendOpTokenId( GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ), nExpClass, nSpaces );
    Append( nExtSheet );
    if( meBiff <= EXC_BIFF5 )
        Append( 0, 8 );
    Append( nExtName );
    Append( 0, (meBiff <= EXC_BIFF5) ? 12 : 2 );
}

void XclExpFmlaCompImpl::AppendMacroCallToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    sal_uInt16 nNameIdx = GetNameManager().InsertMacroCall( rExtFuncData.maFuncName, rExtFuncData.mbVBasic, true, rExtFuncData.mbHidden );
    AppendNameToken( nNameIdx, nExpClass, nSpaces );
}

void XclExpFmlaCompImpl::AppendAddInFuncToken( const XclExpExtFuncData& rExtFuncData, sal_uInt8 nExpClass, sal_uInt8 nSpaces )
{
    String aXclFuncName;
    if( ScGlobal::GetAddInCollection()->GetExcelName( rExtFuncData.maFuncName, GetUILanguage(), aXclFuncName ) )
    {
        sal_uInt16 nExtSheet, nExtName;
        if( mpLinkMgr && mpLinkMgr->InsertAddIn( nExtSheet, nExtName, aXclFuncName ) )
        {
            AppendNameXToken( nExtSheet, nExtName, nExpClass, nSpaces );
            return;
        }
    }
    AppendMacroCallToken( rExtFuncData, nExpClass, nSpaces );
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

void XclExpFmlaCompImpl::Insert( sal_uInt16 nInsertPos, sal_uInt16 nInsertSize )
{
    DBG_ASSERT( nInsertPos < maTokVec.size(), "XclExpFmlaCompImpl::Insert - invalid position" );
    maTokVec.insert( maTokVec.begin() + nInsertPos, nInsertSize, 0 );
}

void XclExpFmlaCompImpl::Overwrite( sal_uInt16 nWriteToPos, sal_uInt16 nOffset )
{
    DBG_ASSERT( static_cast< size_t >( nWriteToPos + 1 ) < maTokVec.size(), "XclExpFmlaCompImpl::Overwrite - invalid position" );
    ShortToSVBT16( nOffset, &maTokVec[ nWriteToPos ] );
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
        (static_cast< size_t >( nPos + 4 ) <= maTokVec.size()) &&
        (maTokVec[ nPos ] == EXC_TOKID_ATTR) &&
        (maTokVec[ nPos + 1 ] == EXC_TOK_ATTR_SPACE);
}

void XclExpFmlaCompImpl::RemoveTrailingParen()
{
    // remove trailing tParen token
    if( !maTokVec.empty() && (maTokVec.back() == EXC_TOKID_PAREN) )
        maTokVec.pop_back();
    // remove remaining tAttrSpace tokens
    while( (maTokVec.size() >= 4) && IsSpaceToken( GetSize() - 4 ) )
        maTokVec.erase( maTokVec.end() - 4, maTokVec.end() );
}

// ============================================================================

namespace {

void lclInitOwnTab( SingleRefData& rRef, const ScAddress& rScPos, SCTAB nCurrScTab, bool b3DRefOnly )
{
    if( b3DRefOnly )
    {
        // no reduction to 2D reference, if global link manager is used
        rRef.SetFlag3D( TRUE );
    }
    else if( rScPos.Tab() == nCurrScTab )
    {
        rRef.SetTabRel( TRUE );
        rRef.nRelTab = 0;
    }
}

void lclPutCellToTokenArray( ScTokenArray& rScTokArr, const ScAddress& rScPos, SCTAB nCurrScTab, bool b3DRefOnly )
{
    SingleRefData aRef;
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
        ComplRefData aRef;
        aRef.InitRange( rScRange );
        lclInitOwnTab( aRef.Ref1, rScRange.aStart, nCurrScTab, b3DRefOnly );
        lclInitOwnTab( aRef.Ref2, rScRange.aEnd, nCurrScTab, b3DRefOnly );
        rScTokArr.AddDoubleReference( aRef );
    }
}

} // namespace

// ----------------------------------------------------------------------------

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
    ULONG nCount = rScRanges.Count();
    if( nCount == 0 )
        return XclTokenArrayRef();

    ScTokenArray aScTokArr;
    SCTAB nCurrScTab = GetCurrScTab();
    bool b3DRefOnly = mxImpl->Is3DRefOnly( eType );
    for( ULONG nIdx = 0; nIdx < nCount; ++nIdx )
    {
        if( nIdx > 0 )
            aScTokArr.AddOpCode( ocUnion );
        lclPutRangeToTokenArray( aScTokArr, *rScRanges.GetObject( nIdx ), nCurrScTab, b3DRefOnly );
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

void XclExpFmlaCompImpl::ProcessMatrix( const XclExpTokenData& rTokData, sal_uInt8 nExpClass )
{
    const ScMatrix* pMatrix = rTokData.mpScToken->GetMatrix();
    if( maCfg.mbAllowArrays && pMatrix )
    {
        SCSIZE nCols, nRows;
        pMatrix->GetDimensions( nCols, nRows );

        AppendOpTokenId( GetTokenId( EXC_TOKID_ARRAY, EXC_TOKCLASS_ARR ), nExpClass, rTokData.mnSpaces );
        Append( static_cast< sal_uInt8 >( (meBiff == EXC_BIFF8) ? (nCols - 1) : nCols ) );
        Append( static_cast< sal_uInt16 >( (meBiff == EXC_BIFF8) ? (nRows - 1) : nRows ) );
        Append( static_cast< sal_uInt32 >( 0 ) );

        if( !mxInlineArr )
            mxInlineArr.reset( new ScMatrixList );
        mxInlineArr->push_front( pMatrix ); // save it for later
    }
    else
    {
        // Array in places that do not allow it (cond fmts, data validation)
        AppendErrorToken( EXC_ERR_NA, rTokData.mnSpaces );
    }
}

// ============================================================================

