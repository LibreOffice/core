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


#define MAXJUMPCOUNT 32     /* maximum number of jumps (ocChose) */
#define MAXCODE      512    /* maximum number of tokens in formula */


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
    sal_Bool bTemp;
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

        void copyFrom( const OpCodeMap& r );

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
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames ) const;

        /// Core implementation of XFormulaOpCodeMapper::getAvailableMappings()
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::sheet::FormulaOpCodeMapEntry >
            createSequenceOfAvailableMappings( const FormulaCompiler& _rCompiler,const sal_Int32 nGroup ) const;

        /** The value used in createSequenceOfAvailableMappings() and thus in
            XFormulaOpCodeMapper::getMappings() for an unknown symbol. */
        static sal_Int32 getOpCodeUnknown();
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

    /** Get OpCode for English symbol.
        Used in XFunctionAccess to create token array.
        @param rName
            Symbol to lookup. MUST be upper case.
     */
    OpCode GetEnglishOpCode( const String& rName ) const;

    void            SetCompileForFAP( sal_Bool bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }

    static bool IsOpCodeVolatile( OpCode eOp );

    static sal_Bool DeQuote( String& rStr );


    static const String&    GetNativeSymbol( OpCode eOp );
    static  sal_Bool            IsMatrixFunction(OpCode _eOpCode);   // if a function _always_ returns a Matrix

    short GetNumFormatType() const { return nNumFmt; }
    sal_Bool  CompileTokenArray();

    void CreateStringFromTokenArray( String& rFormula );
    void CreateStringFromTokenArray( rtl::OUStringBuffer& rBuffer );
    FormulaToken* CreateStringFromToken( String& rFormula, FormulaToken* pToken,
                                    sal_Bool bAllowArrAdvance = sal_False );
    FormulaToken* CreateStringFromToken( rtl::OUStringBuffer& rBuffer, FormulaToken* pToken,
                                    sal_Bool bAllowArrAdvance = sal_False );

    void AppendBoolean( rtl::OUStringBuffer& rBuffer, bool bVal );
    void AppendDouble( rtl::OUStringBuffer& rBuffer, double fVal );
    void AppendString( rtl::OUStringBuffer& rBuffer, const String & rStr );

    /** Set symbol map corresponding to one of predefined formula::FormulaGrammar::Grammar,
        including an address reference convention. */
    inline  FormulaGrammar::Grammar   GetGrammar() const { return meGrammar; }

    static void UpdateSeparatorsNative( const rtl::OUString& rSep, const rtl::OUString& rArrayColSep, const rtl::OUString& rArrayRowSep );
    static void ResetNativeSymbols();
    static void SetNativeSymbols( const OpCodeMapPtr& xMap );
protected:
    virtual String FindAddInFunction( const String& rUpperName, sal_Bool bLocalFirst ) const;
    virtual void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInMap( NonConstOpCodeMapPtr xMap, FormulaGrammar::Grammar _eGrammar ) const;
    virtual void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const;

    virtual void SetError(sal_uInt16 nError);
    virtual FormulaTokenRef ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2, bool bReuseDoubleRef );
    virtual sal_Bool HandleExternalReference(const FormulaToken& _aToken);
    virtual sal_Bool HandleRange();
    virtual sal_Bool HandleSingleRef();
    virtual sal_Bool HandleDbData();

    virtual void CreateStringFromExternal(rtl::OUStringBuffer& rBuffer, FormulaToken* pTokenP);
    virtual void CreateStringFromSingleRef(rtl::OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromDoubleRef(rtl::OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromMatrix(rtl::OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void CreateStringFromIndex(rtl::OUStringBuffer& rBuffer,FormulaToken* pTokenP);
    virtual void LocalizeString( String& rName );   // modify rName - input: exact name
    virtual sal_Bool IsImportingXML() const;

    sal_Bool   GetToken();
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
    void PushTokenArray( FormulaTokenArray*, sal_Bool = sal_False );

    bool MergeRangeReference( FormulaToken * * const pCode1, FormulaToken * const * const pCode2 );

    String              aCorrectedFormula;          // autocorrected Formula
    String              aCorrectedSymbol;           // autocorrected Symbol

    OpCodeMapPtr        mxSymbols;                  // which symbols are used

    FormulaTokenRef     pToken;                     // current token
    FormulaTokenRef     pCurrentFactorToken;        // current factor token (of Factor() method)
    FormulaTokenArray*  pArr;
    ExternalReferenceHelper* pExternalRef;

    FormulaToken**      pCode;
    FormulaArrayStack*  pStack;

    OpCode              eLastOp;
    short               nRecursion;                 // GetToken() recursions
    short               nNumFmt;                    // set during CompileTokenArray()
    sal_uInt16              pc;

    FormulaGrammar::Grammar
                        meGrammar;          // The grammar used, language plus convention.

    sal_Bool                bAutoCorrect;               // whether to apply AutoCorrection
    sal_Bool                bCorrected;                 // AutoCorrection was applied
    sal_Bool                bCompileForFAP;             //! not real RPN but names, for FunctionAutoPilot,
                                                    // will not be resolved
    sal_Bool                bIgnoreErrors;              // on AutoCorrect and CompileForFAP
                                                    // ignore errors and create RPN nevertheless
    sal_Bool                glSubTotal;                 // if code contains one or more subtotal functions
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
