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

#ifndef FORMULA_COMPILER_HXX_INCLUDED
#define FORMULA_COMPILER_HXX_INCLUDED

#include "formula/formuladllapi.h"
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <com/sun/star/uno/Sequence.hxx>

#include "formula/opcode.hxx"
#include "formula/grammar.hxx"
#include "formula/token.hxx"
#include "formula/ExternalReferenceHelper.hxx"


#define FORMULA_MAXJUMPCOUNT    32  /* maximum number of jumps (ocChose) */
#define FORMULA_MAXTOKENS     8192  /* maximum number of tokens in formula */


namespace com { namespace sun { namespace star {
    namespace sheet {
        struct FormulaOpCodeMapEntry;
        struct FormulaToken;
    }
}}}


namespace formula
{
    class FormulaTokenArray;

struct FormulaArrayStack
{
    FormulaArrayStack*  pNext;
    FormulaTokenArray*  pArr;
    bool bTemp;
};


struct FORMULA_DLLPUBLIC StringHashCode
{
    size_t operator()( const String& rStr ) const
    {
        return rtl_ustr_hashCode_WithLength( rStr.GetBuffer(), rStr.Len() );
    }
};

typedef ::boost::unordered_map< String, OpCode, StringHashCode, ::std::equal_to< String > > OpCodeHashMap;
typedef ::boost::unordered_map< String, String, StringHashCode, ::std::equal_to< String > > ExternalHashMap;

class FORMULA_DLLPUBLIC FormulaCompiler
{
public:
    FormulaCompiler();
    FormulaCompiler(FormulaTokenArray& _rArr);
    virtual ~FormulaCompiler();

    // SUNWS8 needs a forward declared friend, otherwise members of the outer
    // class are not accessible.
    class OpCodeMap;
    friend class FormulaCompiler::OpCodeMap;

    /** Mappings from strings to OpCodes and vice versa. */
    class FORMULA_DLLPUBLIC OpCodeMap
    {
        OpCodeHashMap         * mpHashMap;                 /// Hash map of symbols, String -> OpCode
        String              *   mpTable;                   /// Array of symbols, OpCode -> String, offset==OpCode
        ExternalHashMap       * mpExternalHashMap;         /// Hash map of ocExternal, Filter String -> AddIn String
        ExternalHashMap       * mpReverseExternalHashMap;  /// Hash map of ocExternal, AddIn String -> Filter String
        FormulaGrammar::Grammar meGrammar;                  /// Grammar, language and reference convention
        sal_uInt16                  mnSymbols;                  /// Count of OpCode symbols
        bool                    mbCore      : 1;            /// If mapping was setup by core, not filters
        bool                    mbEnglish   : 1;            /// If English symbols and external names

        OpCodeMap();                              // prevent usage
        OpCodeMap( const OpCodeMap& );            // prevent usage
        OpCodeMap& operator=( const OpCodeMap& ); // prevent usage

    public:

        OpCodeMap(sal_uInt16 nSymbols, bool bCore, FormulaGrammar::Grammar eGrammar ) :
            mpHashMap( new OpCodeHashMap( nSymbols)),
            mpTable( new String[ nSymbols ]),
            mpExternalHashMap( new ExternalHashMap),
            mpReverseExternalHashMap( new ExternalHashMap),
            meGrammar( eGrammar),
            mnSymbols( nSymbols),
            mbCore( bCore)
        {
            mbEnglish = FormulaGrammar::isEnglish( meGrammar);
        }
        virtual ~OpCodeMap();

        /** Copy mappings from r into this map, effectively replacing this map.

            @param  bOverrideKnownBad
                    If TRUE, override known legacy bad function names with
                    correct ones if the conditions can be derived from the
                    current maps.
         */
        void copyFrom( const OpCodeMap& r, bool bOverrideKnownBad );

        /// Get the symbol String -> OpCode hash map for finds.
        inline const OpCodeHashMap* getHashMap() const { return mpHashMap; }

        /// Get the symbol String -> AddIn String hash map for finds.
        inline const ExternalHashMap* getExternalHashMap() const { return mpExternalHashMap; }

        /// Get the AddIn String -> symbol String hash map for finds.
        inline const ExternalHashMap* getReverseExternalHashMap() const { return mpReverseExternalHashMap; }

        /// Get the symbol string matching an OpCode.
        inline const String& getSymbol( const OpCode eOp ) const
        {
            DBG_ASSERT( sal_uInt16(eOp) < mnSymbols, "OpCodeMap::getSymbol: OpCode out of range");
            if (sal_uInt16(eOp) < mnSymbols)
                return mpTable[ eOp ];
            static String s_sEmpty;
            return s_sEmpty;
        }

        /// Get the grammar.
        inline FormulaGrammar::Grammar getGrammar() const { return meGrammar; }

        /// Get the symbol count.
        inline sal_uInt16 getSymbolCount() const { return mnSymbols; }

        /** Are these English symbols, as opposed to native language (which may
            be English as well)? */
        inline bool isEnglish() const { return mbEnglish; }

        /// Is it an internal core mapping, or setup by filters?
        inline bool isCore() const { return mbCore; }

        /// Is it an ODF 1.1 compatibility mapping?
        inline bool isPODF() const { return FormulaGrammar::isPODF( meGrammar); }

        /// Is it an ODFF / ODF 1.2 mapping?
        inline bool isODFF() const { return FormulaGrammar::isODFF( meGrammar); }

        /// Does it have external symbol/name mappings?
        inline bool hasExternals() const { return !mpExternalHashMap->empty(); }

        /// Put entry of symbol String and OpCode pair.
        void putOpCode( const String & rStr, const OpCode eOp );

        /// Put entry of symbol String and AddIn international String pair.
        void putExternal( const String & rSymbol, const String & rAddIn );

        /** Put entry of symbol String and AddIn international String pair,
            failing silently if rAddIn name already exists. */
        void putExternalSoftly( const String & rSymbol, const String & rAddIn );

        /// Core implementation of XFormulaOpCodeMapper::getMappings()
        ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken >
            createSequenceOfFormulaTokens(const FormulaCompiler& _rCompiler,
                    const ::com::sun::star::uno::Sequence< OUString >& rNames ) const;

        /// Core implementation of XFormulaOpCodeMapper::getAvailableMappings()
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::sheet::FormulaOpCodeMapEntry >
            createSequenceOfAvailableMappings( const FormulaCompiler& _rCompiler,const sal_Int32 nGroup ) const;

        /** The value used in createSequenceOfAvailableMappings() and thus in
            XFormulaOpCodeMapper::getMappings() for an unknown symbol. */
        static sal_Int32 getOpCodeUnknown();

    private:

        /** Conditionally put a mapping in copyFrom() context.

            Does NOT check eOp range!
         */
        void putCopyOpCode( const String& rSymbol, OpCode eOp );
    };

public:
    typedef ::boost::shared_ptr< const OpCodeMap >  OpCodeMapPtr;
    typedef ::boost::shared_ptr< OpCodeMap >        NonConstOpCodeMapPtr;

    /** Get OpCodeMap for formula language.
        @param nLanguage
            One of ::com::sun::star::sheet::FormulaLanguage constants.
        @return Map for nLanguage. If nLanguage is unknown, a NULL map is returned.
     */
    OpCodeMapPtr GetOpCodeMap( const sal_Int32 nLanguage ) const;

    /** Create an internal symbol map from API mapping.
        @param bEnglish
            Use English number parser / formatter instead of native.
     */
    OpCodeMapPtr CreateOpCodeMap(
            const ::com::sun::star::uno::Sequence<
            const ::com::sun::star::sheet::FormulaOpCodeMapEntry > & rMapping,
            bool bEnglish );

    /** Get current OpCodeMap in effect. */
    inline OpCodeMapPtr GetCurrentOpCodeMap() const { return mxSymbols; }

    /** Get OpCode for English symbol.
        Used in XFunctionAccess to create token array.
        @param rName
            Symbol to lookup. MUST be upper case.
     */
    OpCode GetEnglishOpCode( const String& rName ) const;

    sal_uInt16 GetErrorConstant( const String& rName ) const;

    void            SetCompileForFAP( bool bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }

    static bool IsOpCodeVolatile( OpCode eOp );

    static bool DeQuote( String& rStr );


    static const String&    GetNativeSymbol( OpCode eOp );
    static  bool            IsMatrixFunction(OpCode _eOpCode);   // if a function _always_ returns a Matrix

    short GetNumFormatType() const { return nNumFmt; }
    bool  CompileTokenArray();

    void CreateStringFromTokenArray( String& rFormula );
    void CreateStringFromTokenArray( OUStringBuffer& rBuffer );
    FormulaToken* CreateStringFromToken( String& rFormula, FormulaToken* pToken,
                                    bool bAllowArrAdvance = false );
    FormulaToken* CreateStringFromToken( OUStringBuffer& rBuffer, FormulaToken* pToken,
                                    bool bAllowArrAdvance = false );

    void AppendBoolean( OUStringBuffer& rBuffer, bool bVal );
    void AppendDouble( OUStringBuffer& rBuffer, double fVal );
    void AppendString( OUStringBuffer& rBuffer, const String & rStr );

    /** Set symbol map corresponding to one of predefined formula::FormulaGrammar::Grammar,
        including an address reference convention. */
    inline  FormulaGrammar::Grammar   GetGrammar() const { return meGrammar; }

    static void UpdateSeparatorsNative( const OUString& rSep, const OUString& rArrayColSep, const OUString& rArrayRowSep );
    static void ResetNativeSymbols();
    static void SetNativeSymbols( const OpCodeMapPtr& xMap );
protected:
    virtual String FindAddInFunction( const String& rUpperName, bool bLocalFirst ) const;
    virtual void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInMap( NonConstOpCodeMapPtr xMap, FormulaGrammar::Grammar _eGrammar ) const;
    virtual void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const;

    virtual void SetError(sal_uInt16 nError);
    virtual FormulaTokenRef ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2, bool bReuseDoubleRef );
    virtual bool HandleExternalReference(const FormulaToken& _aToken);
    virtual bool HandleRange();
    virtual bool HandleSingleRef();
    virtual bool HandleDbData();

    virtual void CreateStringFromExternal(OUStringBuffer& rBuffer, FormulaToken* pTokenP);
    virtual void CreateStringFromSingleRef(OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromDoubleRef(OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromMatrix(OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromIndex(OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void LocalizeString( String& rName );   // modify rName - input: exact name

    void AppendErrorConstant( OUStringBuffer& rBuffer, sal_uInt16 nError );

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
    void NotLine();
    OpCode Expression();
    void PopTokenArray();
    void PushTokenArray( FormulaTokenArray*, bool = false );

    bool MergeRangeReference( FormulaToken * * const pCode1, FormulaToken * const * const pCode2 );

    String              aCorrectedFormula;      // autocorrected Formula
    String              aCorrectedSymbol;       // autocorrected Symbol

    OpCodeMapPtr        mxSymbols;              // which symbols are used

    FormulaTokenRef     mpToken;                // current token
    FormulaTokenRef     pCurrentFactorToken;    // current factor token (of Factor() method)
    FormulaTokenArray*  pArr;
    ExternalReferenceHelper* pExternalRef;

    FormulaToken**      pCode;
    FormulaArrayStack*  pStack;

    OpCode              eLastOp;
    short               nRecursion;             // GetToken() recursions
    short               nNumFmt;                // set during CompileTokenArray()
    sal_uInt16          pc;                     // program counter

    FormulaGrammar::Grammar meGrammar;          // The grammar used, language plus convention.

    bool                bAutoCorrect;           // whether to apply AutoCorrection
    bool                bCorrected;             // AutoCorrection was applied
    bool                bCompileForFAP;         //! not real RPN but names, for FunctionAutoPilot
                                                // will not be resolved
    bool                bIgnoreErrors;          // on AutoCorrect and CompileForFAP
                                                // ignore errors and create RPN nevertheless
    bool                glSubTotal;             // if code contains one or more subtotal functions
private:
    void InitSymbolsNative() const;    /// only SymbolsNative, on first document creation
    void InitSymbolsEnglish() const;   /// only SymbolsEnglish, maybe later
    void InitSymbolsPODF() const;      /// only SymbolsPODF, on demand
    void InitSymbolsODFF() const;      /// only SymbolsODFF, on demand
    void InitSymbolsEnglishXL() const; /// only SymbolsEnglishXL, on demand

    void loadSymbols(sal_uInt16 _nSymbols,FormulaGrammar::Grammar _eGrammar,NonConstOpCodeMapPtr& _xMap) const;

    static inline void ForceArrayOperator( FormulaTokenRef& rCurr, const FormulaTokenRef& rPrev )
        {
            if ( rPrev && rPrev->HasForceArray() &&
                    rCurr->GetType() == svByte && rCurr->GetOpCode() != ocPush
                    && !rCurr->HasForceArray() )
                rCurr->SetForceArray( true);
        }

    // SUNWS7 needs a forward declared friend, otherwise members of the outer
    // class are not accessible.
    class CurrentFactor;
    friend class FormulaCompiler::CurrentFactor;
    class CurrentFactor
    {
        FormulaTokenRef  pPrevFac;
        FormulaCompiler* pCompiler;
        // not implemented
        CurrentFactor( const CurrentFactor& );
        CurrentFactor& operator=( const CurrentFactor& );
    public:
        explicit CurrentFactor( FormulaCompiler* pComp )
            : pPrevFac( pComp->pCurrentFactorToken )
            , pCompiler( pComp )
            {}
        ~CurrentFactor()
            { pCompiler->pCurrentFactorToken = pPrevFac; }
        // yes, this operator= may modify the RValue
        void operator=( FormulaTokenRef& r )
            {
                ForceArrayOperator( r, pPrevFac);
                pCompiler->pCurrentFactorToken = r;
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


    mutable NonConstOpCodeMapPtr  mxSymbolsODFF;                          // ODFF symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsPODF;                          // ODF 1.1 symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsNative;                        // native symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsEnglish;                       // English symbols
    mutable NonConstOpCodeMapPtr  mxSymbolsEnglishXL;                     // English Excel symbols (for VBA formula parsing)
};
// =============================================================================
} // formula
// =============================================================================

#endif // FORMULA_COMPILER_HXX_INCLUDED


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
