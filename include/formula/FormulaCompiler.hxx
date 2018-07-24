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

#ifndef INCLUDED_FORMULA_FORMULACOMPILER_HXX
#define INCLUDED_FORMULA_FORMULACOMPILER_HXX

#include <memory>
#include <unordered_map>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>
#include <formula/formuladllapi.h>
#include <formula/grammar.hxx>
#include <formula/opcode.hxx>
#include <formula/token.hxx>
#include <formula/tokenarray.hxx>
#include <formula/types.hxx>
#include <formula/paramclass.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <tools/debug.hxx>

#define FORMULA_MAXJUMPCOUNT    32  /* maximum number of jumps (ocChoose) */
#define FORMULA_MAXTOKENS     8192  /* maximum number of tokens in formula */
#define FORMULA_MAXPARAMS      255  /* maximum number of parameters per function (byte) */
#define FORMULA_MAXPARAMSII      8  /* maximum number of parameters for functions that have implicit intersection ranges */


namespace com { namespace sun { namespace star {
    namespace sheet {
        struct FormulaOpCodeMapEntry;
        struct FormulaToken;
    }
}}}

class CharClass;
enum class FormulaError : sal_uInt16;
enum class SvNumFormatType : sal_Int16;

namespace formula
{

struct FormulaArrayStack
{
    FormulaArrayStack*  pNext;
    FormulaTokenArray*  pArr;
    sal_uInt16          nIndex;
    FormulaTokenRef     mpLastToken;
    bool bTemp;
};

typedef std::unordered_map< OUString, OpCode > OpCodeHashMap;
typedef std::unordered_map< OUString, OUString > ExternalHashMap;

class FORMULA_DLLPUBLIC FormulaCompiler
{
private:
    FormulaCompiler(const FormulaCompiler&) = delete;
    FormulaCompiler& operator=(const FormulaCompiler&) = delete;
public:
    FormulaCompiler(bool bComputeII = false, bool bMatrixFlag = false);
    FormulaCompiler(FormulaTokenArray& _rArr, bool bComputeII = false, bool bMatrixFlag = false);
    virtual ~FormulaCompiler();

    /** Mappings from strings to OpCodes and vice versa. */
    class FORMULA_DLLPUBLIC OpCodeMap final
    {
        OpCodeHashMap           maHashMap;                  /// Hash map of symbols, OUString -> OpCode
        std::unique_ptr<OUString[]> mpTable;                /// Array of symbols, OpCode -> OUString, offset==OpCode
        ExternalHashMap         maExternalHashMap;         /// Hash map of ocExternal, Filter String -> AddIn String
        ExternalHashMap         maReverseExternalHashMap;  /// Hash map of ocExternal, AddIn String -> Filter String
        FormulaGrammar::Grammar meGrammar;                  /// Grammar, language and reference convention
        sal_uInt16              mnSymbols;                  /// Count of OpCode symbols
        bool                    mbCore      : 1;            /// If mapping was setup by core, not filters
        bool                    mbEnglish   : 1;            /// If English symbols and external names

        OpCodeMap( const OpCodeMap& ) = delete;
        OpCodeMap& operator=( const OpCodeMap& ) = delete;

    public:

        OpCodeMap(sal_uInt16 nSymbols, bool bCore, FormulaGrammar::Grammar eGrammar ) :
            maHashMap(nSymbols),
            mpTable( new OUString[ nSymbols ]),
            meGrammar( eGrammar),
            mnSymbols( nSymbols),
            mbCore( bCore),
            mbEnglish ( FormulaGrammar::isEnglish(eGrammar) )
        {
        }

        /** Copy mappings from r into this map, effectively replacing this map.

            Override known legacy bad function names with
            correct ones if the conditions can be derived from the
            current maps.
         */
        void copyFrom( const OpCodeMap& r );

        /// Get the symbol String -> OpCode hash map for finds.
        const OpCodeHashMap& getHashMap() const { return maHashMap; }

        /// Get the symbol String -> AddIn String hash map for finds.
        const ExternalHashMap& getExternalHashMap() const { return maExternalHashMap; }

        /// Get the AddIn String -> symbol String hash map for finds.
        const ExternalHashMap& getReverseExternalHashMap() const { return maReverseExternalHashMap; }

        /// Get the symbol string matching an OpCode.
        const OUString& getSymbol( const OpCode eOp ) const
        {
            DBG_ASSERT( sal_uInt16(eOp) < mnSymbols, "OpCodeMap::getSymbol: OpCode out of range");
            if (sal_uInt16(eOp) < mnSymbols)
                return mpTable[ eOp ];
            static OUString s_sEmpty;
            return s_sEmpty;
        }

        /// Get the first character of the symbol string matching an OpCode.
        sal_Unicode getSymbolChar( const OpCode eOp ) const {  return getSymbol(eOp)[0]; };

        /// Get the grammar.
        FormulaGrammar::Grammar getGrammar() const { return meGrammar; }

        /// Get the symbol count.
        sal_uInt16 getSymbolCount() const { return mnSymbols; }

        /** Are these English symbols, as opposed to native language (which may
            be English as well)? */
        bool isEnglish() const { return mbEnglish; }

        /// Is it an ODF 1.1 compatibility mapping?
        bool isPODF() const { return FormulaGrammar::isPODF( meGrammar); }

        /* TODO: add isAPI() once a FormulaLanguage was added. */

        /// Is it an ODFF / ODF 1.2 mapping?
        bool isODFF() const { return FormulaGrammar::isODFF( meGrammar); }

        /// Is it an OOXML mapping?
        bool isOOXML() const { return FormulaGrammar::isOOXML( meGrammar); }

        /// Does it have external symbol/name mappings?
        bool hasExternals() const { return !maExternalHashMap.empty(); }

        /// Put entry of symbol String and OpCode pair.
        void putOpCode( const OUString & rStr, const OpCode eOp, const CharClass* pCharClass );

        /// Put entry of symbol String and AddIn international String pair.
        void putExternal( const OUString & rSymbol, const OUString & rAddIn );

        /** Put entry of symbol String and AddIn international String pair,
            failing silently if rAddIn name already exists. */
        void putExternalSoftly( const OUString & rSymbol, const OUString & rAddIn );

        /// Core implementation of XFormulaOpCodeMapper::getMappings()
        css::uno::Sequence< css::sheet::FormulaToken >
            createSequenceOfFormulaTokens(const FormulaCompiler& _rCompiler,
                    const css::uno::Sequence< OUString >& rNames ) const;

        /// Core implementation of XFormulaOpCodeMapper::getAvailableMappings()
        css::uno::Sequence< css::sheet::FormulaOpCodeMapEntry >
            createSequenceOfAvailableMappings( const FormulaCompiler& _rCompiler,const sal_Int32 nGroup ) const;

        /** The value used in createSequenceOfAvailableMappings() and thus in
            XFormulaOpCodeMapper::getMappings() for an unknown symbol. */
        static sal_Int32 getOpCodeUnknown();

    private:

        /** Conditionally put a mapping in copyFrom() context.

            Does NOT check eOp range!
         */
        void putCopyOpCode( const OUString& rSymbol, OpCode eOp );
    };

public:
    typedef std::shared_ptr< const OpCodeMap >  OpCodeMapPtr;
    typedef std::shared_ptr< OpCodeMap >        NonConstOpCodeMapPtr;

    /** Get OpCodeMap for formula language.
        @param nLanguage
            One of css::sheet::FormulaLanguage constants.
        @return Map for nLanguage. If nLanguage is unknown, a NULL map is returned.
     */
    OpCodeMapPtr GetOpCodeMap( const sal_Int32 nLanguage ) const;

    /** Create an internal symbol map from API mapping.
        @param bEnglish
            Use English number parser / formatter instead of native.
     */
    static OpCodeMapPtr CreateOpCodeMap(
            const css::uno::Sequence< const css::sheet::FormulaOpCodeMapEntry > & rMapping,
            bool bEnglish );

    /** Get current OpCodeMap in effect. */
    const OpCodeMapPtr& GetCurrentOpCodeMap() const { return mxSymbols; }

    /** Get OpCode for English symbol.
        Used in XFunctionAccess to create token array.
        @param rName
            Symbol to lookup. MUST be upper case.
     */
    OpCode GetEnglishOpCode( const OUString& rName ) const;

    FormulaError GetErrorConstant( const OUString& rName ) const;

    void EnableJumpCommandReorder( bool bEnable );
    void EnableStopOnError( bool bEnable );

    static bool IsOpCodeVolatile( OpCode eOp );
    static bool IsOpCodeJumpCommand( OpCode eOp );

    static bool DeQuote( OUString& rStr );


    static const OUString&  GetNativeSymbol( OpCode eOp );
    static sal_Unicode      GetNativeSymbolChar( OpCode eOp );
    static  bool            IsMatrixFunction(OpCode _eOpCode);   // if a function _always_ returns a Matrix

    SvNumFormatType GetNumFormatType() const { return nNumFmt; }
    bool  CompileTokenArray();

    void CreateStringFromTokenArray( OUString& rFormula );
    void CreateStringFromTokenArray( OUStringBuffer& rBuffer );
    const FormulaToken* CreateStringFromToken( OUString& rFormula, const FormulaToken* pToken );
    const FormulaToken* CreateStringFromToken( OUStringBuffer& rBuffer, const FormulaToken* pToken,
                                    bool bAllowArrAdvance = false );

    void AppendBoolean( OUStringBuffer& rBuffer, bool bVal ) const;
    void AppendDouble( OUStringBuffer& rBuffer, double fVal ) const;
    static void AppendString( OUStringBuffer& rBuffer, const OUString & rStr );

    /** Set symbol map corresponding to one of predefined formula::FormulaGrammar::Grammar,
        including an address reference convention. */
    FormulaGrammar::Grammar   GetGrammar() const { return meGrammar; }

    /** Whether current symbol set and grammar need transformation of Table
        structured references to A1 style references when writing / exporting
        (creating strings).
     */
    bool NeedsTableRefTransformation() const;

    /** If a parameter nParam (0-based) is to be forced to array for OpCode
        eOp, i.e. classified as ParamClass::ForceArray or
        ParamClass::ReferenceOrForceArray type. */
    virtual formula::ParamClass GetForceArrayParameter( const FormulaToken* pToken, sal_uInt16 nParam ) const;

    static void UpdateSeparatorsNative( const OUString& rSep, const OUString& rArrayColSep, const OUString& rArrayRowSep );
    static void ResetNativeSymbols();
    static void SetNativeSymbols( const OpCodeMapPtr& xMap );

    /** Sets the implicit intersection compute flag */
    void SetComputeIIFlag(bool bSet) { mbComputeII = bSet; }

    /** Sets the matrix flag for the formula*/
    void SetMatrixFlag(bool bSet) { mbMatrixFlag = bSet; }

    /** Separators mapped when loading opcodes from the resource, values other
        than RESOURCE_BASE may override the resource strings. Used by OpCodeList
        implementation via loadSymbols().
     */
    enum class SeparatorType
    {
        RESOURCE_BASE,
        SEMICOLON_BASE
    };

protected:
    virtual OUString FindAddInFunction( const OUString& rUpperName, bool bLocalFirst ) const;
    virtual void fillFromAddInCollectionUpperName( const NonConstOpCodeMapPtr& xMap ) const;
    virtual void fillFromAddInMap( const NonConstOpCodeMapPtr& xMap, FormulaGrammar::Grammar _eGrammar ) const;
    virtual void fillFromAddInCollectionEnglishName( const NonConstOpCodeMapPtr& xMap ) const;
    virtual void fillAddInToken(::std::vector< css::sheet::FormulaOpCodeMapEntry >& _rVec, bool _bIsEnglish) const;

    virtual void SetError(FormulaError nError);
    virtual FormulaTokenRef ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2 );
    virtual bool HandleExternalReference(const FormulaToken& _aToken);
    virtual bool HandleRange();
    virtual bool HandleColRowName();
    virtual bool HandleDbData();
    virtual bool HandleTableRef();

    virtual void CreateStringFromExternal( OUStringBuffer& rBuffer, const FormulaToken* pToken ) const;
    virtual void CreateStringFromSingleRef( OUStringBuffer& rBuffer, const FormulaToken* pToken ) const;
    virtual void CreateStringFromDoubleRef( OUStringBuffer& rBuffer, const FormulaToken* pToken ) const;
    virtual void CreateStringFromMatrix( OUStringBuffer& rBuffer, const FormulaToken* pToken ) const;
    virtual void CreateStringFromIndex( OUStringBuffer& rBuffer, const FormulaToken* pToken ) const;
    virtual void LocalizeString( OUString& rName ) const;   // modify rName - input: exact name

    void AppendErrorConstant( OUStringBuffer& rBuffer, FormulaError nError ) const;

    bool   GetToken();
    OpCode NextToken();
    void PutCode( FormulaTokenRef& );
    void Factor();
    void RangeLine();
    void UnionLine();
    void IntersectionLine();
    void UnaryLine();
    void PostOpLine();
    void PowLine();
    void MulDivLine();
    void AddSubLine();
    void ConcatLine();
    void CompareLine();
    OpCode Expression();
    void PopTokenArray();
    void PushTokenArray( FormulaTokenArray*, bool );

    bool MergeRangeReference( FormulaToken * * const pCode1, FormulaToken * const * const pCode2 );

    // Returns whether the opcode has implicit intersection ranges as parameters.
    // Called for (most) opcodes to possibly handle implicit intersection for the parameters.
    virtual void HandleIIOpCode(FormulaToken* /*token*/,
                                FormulaToken*** /*pppToken*/, sal_uInt8 /*nNumParams*/) {}

    OUString            aCorrectedFormula;      // autocorrected Formula
    OUString            aCorrectedSymbol;       // autocorrected Symbol

    OpCodeMapPtr        mxSymbols;              // which symbols are used

    FormulaTokenRef     mpToken;                // current token
    FormulaTokenRef     pCurrentFactorToken;    // current factor token (of Factor() method)
    sal_uInt16          nCurrentFactorParam;    // current factor token's parameter, 1-based
    FormulaTokenArray*  pArr;
    FormulaTokenArrayPlainIterator maArrIterator;
    FormulaTokenRef     mpLastToken;            // last token

    FormulaToken**      pCode;
    FormulaArrayStack*  pStack;

    OpCode              eLastOp;
    short               nRecursion;             // GetToken() recursions
    SvNumFormatType     nNumFmt;                // set during CompileTokenArray()
    sal_uInt16          pc;                     // program counter

    FormulaGrammar::Grammar meGrammar;          // The grammar used, language plus convention.

    bool                bAutoCorrect;           // whether to apply AutoCorrection
    bool                bCorrected;             // AutoCorrection was applied
    bool                glSubTotal;             // if code contains one or more subtotal functions
    bool                needsRPNTokenCheck;     // whether to make FormulaTokenArray check all tokens at the end

    bool mbJumpCommandReorder; /// Whether or not to reorder RPN for jump commands.
    bool mbStopOnError;        /// Whether to stop compilation on first encountered error.

    bool mbComputeII;  // whether to attempt computing implicit intersection ranges while building the RPN array.
    bool mbMatrixFlag; // whether the formula is a matrix formula (needed for II computation)

private:
    void InitSymbolsNative() const;    /// only SymbolsNative, on first document creation
    void InitSymbolsEnglish() const;   /// only SymbolsEnglish, maybe later
    void InitSymbolsPODF() const;      /// only SymbolsPODF, on demand
    void InitSymbolsAPI() const;       /// only SymbolsAPI, on demand
    void InitSymbolsODFF() const;      /// only SymbolsODFF, on demand
    void InitSymbolsEnglishXL() const; /// only SymbolsEnglishXL, on demand
    void InitSymbolsOOXML() const;     /// only SymbolsOOXML, on demand

    void loadSymbols(const std::pair<const char*, int>* pSymbols, FormulaGrammar::Grammar eGrammar, NonConstOpCodeMapPtr& rxMap,
            SeparatorType eSepType = SeparatorType::SEMICOLON_BASE) const;

    /** Check pCurrentFactorToken for nParam's (0-based) ForceArray types and
        set ForceArray at rCurr if so. Set nParam+1 as 1-based
        nCurrentFactorParam for subsequent ForceArrayOperator() calls.
     */
    void CheckSetForceArrayParameter( FormulaTokenRef const & rCurr, sal_uInt8 nParam );

    void ForceArrayOperator( FormulaTokenRef const & rCurr );

    class CurrentFactor
    {
        FormulaTokenRef  pPrevFac;
        sal_uInt16       nPrevParam;
        FormulaCompiler* pCompiler;
        CurrentFactor( const CurrentFactor& ) = delete;
        CurrentFactor& operator=( const CurrentFactor& ) = delete;
    public:
        explicit CurrentFactor( FormulaCompiler* pComp )
            : pPrevFac( pComp->pCurrentFactorToken )
            , nPrevParam( pComp->nCurrentFactorParam )
            , pCompiler( pComp )
            {}
        ~CurrentFactor()
            {
                pCompiler->pCurrentFactorToken = pPrevFac;
                pCompiler->nCurrentFactorParam = nPrevParam;
            }
        // yes, this operator= may modify the RValue
        void operator=( FormulaTokenRef const & r )
            {
                pCompiler->ForceArrayOperator( r );
                pCompiler->pCurrentFactorToken = r;
                pCompiler->nCurrentFactorParam = 0;
            }
        void operator=( FormulaToken* p )
            {
                FormulaTokenRef xTemp( p );
                *this = xTemp;
            }
        operator FormulaTokenRef&()
            { return pCompiler->pCurrentFactorToken; }
        FormulaToken* operator->()
            { return pCompiler->pCurrentFactorToken.operator->(); }
        operator FormulaToken*()
            { return operator->(); }
    };


    mutable NonConstOpCodeMapPtr  mxSymbolsODFF;      // ODFF symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsPODF;      // ODF 1.1 symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsAPI;       // XFunctionAccess API symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsNative;    // native symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsEnglish;   // English symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsEnglishXL; // English Excel symbols (for VBA formula parsing)
    mutable NonConstOpCodeMapPtr  mxSymbolsOOXML;     // Excel OOXML symbols

    static FormulaTokenArray smDummyTokenArray;
};

} // formula


#endif // INCLUDED_FORMULA_FORMULACOMPILER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
