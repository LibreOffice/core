/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: compiler.hxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:15:10 $
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

#ifndef SC_COMPILER_HXX
#define SC_COMPILER_HXX

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif

#ifndef _SVMEMPOOL_HXX
#include <tools/mempool.hxx>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_TOKENARRAY_HXX
#include "tokenarray.hxx"
#endif
#ifndef SC_TOKEN_HXX
#include "token.hxx"
#endif
#ifndef SC_INTRUREF_HXX
#include "intruref.hxx"
#endif
#ifndef SC_GRAMMAR_HXX
#include "grammar.hxx"
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct FormulaOpCodeMapEntry;
        struct FormulaToken;
    }
}}}

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef INCLUDED_HASH_MAP
#include <hash_map>
#define INCLUDED_HASH_MAP
#endif

//-----------------------------------------------

// constants and data types also for external modules (ScInterpreter et al)

#define MAXCODE      512    /* maximum number of tokens in formula */
#define MAXSTRLEN    256    /* maximum length of input string of one symbol */
#define MAXJUMPCOUNT 32     /* maximum number of jumps (ocChose) */

// flag values of CharTable
#define SC_COMPILER_C_ILLEGAL       0x00000000
#define SC_COMPILER_C_CHAR          0x00000001
#define SC_COMPILER_C_CHAR_BOOL     0x00000002
#define SC_COMPILER_C_CHAR_WORD     0x00000004
#define SC_COMPILER_C_CHAR_VALUE    0x00000008
#define SC_COMPILER_C_CHAR_STRING   0x00000010
#define SC_COMPILER_C_CHAR_DONTCARE 0x00000020
#define SC_COMPILER_C_BOOL          0x00000040
#define SC_COMPILER_C_WORD          0x00000080
#define SC_COMPILER_C_WORD_SEP      0x00000100
#define SC_COMPILER_C_VALUE         0x00000200
#define SC_COMPILER_C_VALUE_SEP     0x00000400
#define SC_COMPILER_C_VALUE_EXP     0x00000800
#define SC_COMPILER_C_VALUE_SIGN    0x00001000
#define SC_COMPILER_C_VALUE_VALUE   0x00002000
#define SC_COMPILER_C_STRING_SEP    0x00004000
#define SC_COMPILER_C_NAME_SEP      0x00008000  // there can be only one! '\''
#define SC_COMPILER_C_CHAR_IDENT    0x00010000  // identifier (built-in function) start
#define SC_COMPILER_C_IDENT         0x00020000  // identifier continuation
#define SC_COMPILER_C_ODF_LBRACKET  0x00040000  // ODF '[' reference bracket
#define SC_COMPILER_C_ODF_RBRACKET  0x00080000  // ODF ']' reference bracket

#define SC_COMPILER_FILE_TAB_SEP    '#'         // 'Doc'#Tab


class ScDocument;
class ScMatrix;
class ScRangeData;

// constants and data types internal to compiler

#if 0
/*
    OpCode   eOp;           // OpCode
    StackVar eType;         // type of data
    USHORT   nRefCnt;       // reference count
    BOOL     bRaw;          // not cloned yet and trimmed to real size
 */
#endif

#define SC_TOKEN_FIX_MEMBERS    \
    OpCode   eOp;               \
    StackVar eType;             \
    USHORT   nRefCnt;           \
    BOOL     bRaw;

struct ScDoubleRawToken
{
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union
    {   // union only to assure alignment identical to ScRawToken
        double      nValue;
        struct {
            BYTE        cByte;
            bool        bHasForceArray;
        } sbyte;
    };
                DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRawToken );
};

struct ScRawToken
{
    friend class ScCompiler;
    // Friends that use a temporary ScRawToken on the stack (and therefor need
    // the private dtor) and know what they're doing..
    friend class ScTokenArray;
    friend USHORT lcl_ScRawTokenOffset();
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union {
        double       nValue;
        struct {
            BYTE        cByte;
            bool        bHasForceArray;
        } sbyte;
        ComplRefData aRef;
        ScMatrix*    pMat;
        USHORT       nIndex;                // index into name collection
        sal_Unicode  cStr[ MAXSTRLEN+1 ];   // string (up to 255 characters + 0)
        short        nJump[MAXJUMPCOUNT+1]; // If/Chose token
    };

                //! other members not initialized
                ScRawToken() : bRaw( TRUE ) {}
private:
                ~ScRawToken() {}                //! only delete via Delete()
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScRawToken );
    StackVar    GetType()   const       { return (StackVar) eType; }
    OpCode      GetOpCode() const       { return (OpCode)   eOp;   }
    void        NewOpCode( OpCode e )   { eOp = e; }
    void        IncRef()                { nRefCnt++;       }
    void        DecRef()                { if( !--nRefCnt ) Delete(); }
    USHORT      GetRef() const          { return nRefCnt; }
    SC_DLLPUBLIC void       Delete();

    // Use these methods only on tokens that are not part of a token array,
    // since the reference count is cleared!
    void SetOpCode( OpCode eCode );
    void SetString( const sal_Unicode* pStr );
    void SetSingleReference( const SingleRefData& rRef );
    void SetDoubleReference( const ComplRefData& rRef );
    void SetDouble( double fVal );
    void SetInt( int nVal );
    void SetName( USHORT n );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);
    // These methods are ok to use, reference count not cleared.
    ComplRefData& GetReference();
    void SetReference( ComplRefData& rRef );

    ScRawToken* Clone() const;      // real copy!
    ScToken* CreateToken() const;   // create typified token
    void Load( SvStream&, USHORT nVer );

    static xub_StrLen GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
    static size_t GetStrLenBytes( xub_StrLen nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};


typedef ScSimpleIntrusiveReference< struct ScRawToken > ScRawTokenRef;


struct ScArrayStack;

typedef ::std::hash_map< String, OpCode, ScStringHashCode, ::std::equal_to< String > > ScOpCodeHashMap;
typedef ::std::hash_map< String, String, ScStringHashCode, ::std::equal_to< String > > ScExternalHashMap;


class SC_DLLPUBLIC ScCompiler
{
public:
    struct Convention
    {
        const ScAddress::Convention meConv;
        const ULONG*                mpCharTable;


        Convention( ScAddress::Convention eConvP );
        virtual ~Convention();

        virtual void MakeRefStr( rtl::OUStringBuffer&   rBuffer,
                                 const ScCompiler&      rCompiler,
                                 const ComplRefData&    rRef,
                                 BOOL bSingleRef ) const = 0;
        virtual ::com::sun::star::i18n::ParseResult
                    parseAnyToken( const String& rFormula,
                                   xub_StrLen nSrcPos,
                                   const CharClass* pCharClass) const = 0;
    };
    friend struct Convention;

    /** Mappings from strings to OpCodes and vice versa. */
    class OpCodeMap
    {
        ScOpCodeHashMap         *mpHashMap;                 /// Hash map of symbols, String -> OpCode
        String                  *mpTable;                   /// Array of symbols, OpCode -> String, offset==OpCode
        ScExternalHashMap       *mpExternalHashMap;         /// Hash map of ocExternal, Filter String -> AddIn String
        ScExternalHashMap       *mpReverseExternalHashMap;  /// Hash map of ocExternal, AddIn String -> Filter String
        ScGrammar::Grammar      meGrammar;                  /// Grammar, language and reference convention
        USHORT                  mnSymbols;                  /// Count of OpCode symbols
        bool                    mbCore      : 1;            /// If mapping was setup by core, not filters
        bool                    mbEnglish   : 1;            /// If English symbols and external names

        OpCodeMap();                              // prevent usage
        OpCodeMap( const OpCodeMap& );            // prevent usage
        OpCodeMap& operator=( const OpCodeMap& ); // prevent usage

    public:

        OpCodeMap( USHORT nSymbols, bool bCore, ScGrammar::Grammar eGrammar ) :
            mpHashMap( new ScOpCodeHashMap( nSymbols)),
            mpTable( new String[ nSymbols ]),
            mpExternalHashMap( new ScExternalHashMap),
            mpReverseExternalHashMap( new ScExternalHashMap),
            meGrammar( eGrammar),
            mnSymbols( nSymbols),
            mbCore( bCore)
        {
            mbEnglish = ScGrammar::isEnglish( meGrammar);
        }
        ~OpCodeMap()
        {
            delete mpReverseExternalHashMap;
            delete mpExternalHashMap;
            delete [] mpTable;
            delete mpHashMap;
        }

        /// Get the symbol String -> OpCode hash map for finds.
        inline const ScOpCodeHashMap* getHashMap() const { return mpHashMap; }

        /// Get the symbol String -> AddIn String hash map for finds.
        inline const ScExternalHashMap* getExternalHashMap() const { return mpExternalHashMap; }

        /// Get the AddIn String -> symbol String hash map for finds.
        inline const ScExternalHashMap* getReverseExternalHashMap() const { return mpReverseExternalHashMap; }

        /// Get the symbol string matching an OpCode.
        inline const String& getSymbol( const OpCode eOp ) const
        {
            DBG_ASSERT( USHORT(eOp) < mnSymbols, "OpCodeMap::getSymbol: OpCode out of range");
            if (USHORT(eOp) < mnSymbols)
                return mpTable[ eOp ];
            return EMPTY_STRING;
        }

        /// Get the grammar.
        inline ScGrammar::Grammar getGrammar() const { return meGrammar; }

        /// Get the symbol count.
        inline USHORT getSymbolCount() const { return mnSymbols; }

        /** Are these English symbols, as opposed to native language (which may
            be English as well)? */
        inline bool isEnglish() const { return mbEnglish; }

        /// Is it an internal core mapping, or setup by filters?
        inline bool isCore() const { return mbCore; }

        /// Is it an ODF 1.1 compatibility mapping?
        inline bool isPODF() const { return ScGrammar::isPODF( meGrammar); }

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
            createSequenceOfFormulaTokens(
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames ) const;

        /// Core implementation of XFormulaOpCodeMapper::getAvailableMappings()
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::sheet::FormulaOpCodeMapEntry >
            createSequenceOfAvailableMappings( const sal_Int32 nGroup ) const;

        /** The value used in createSequenceOfAvailableMappings() and thus in
            XFormulaOpCodeMapper::getMappings() for an unknown symbol. */
        static inline sal_Int32 getOpCodeUnknown()
        {
            return kOpCodeUnknown;
        }

private:

        static const sal_Int32 kOpCodeUnknown = -1;

    };

    typedef ::boost::shared_ptr< const OpCodeMap > OpCodeMapPtr;
    typedef ::boost::shared_ptr< OpCodeMap > NonConstOpCodeMapPtr;

private:

    static NonConstOpCodeMapPtr  mxSymbolsODFF;                          // ODFF symbols
    static NonConstOpCodeMapPtr  mxSymbolsPODF;                          // ODF 1.1 symbols
    static NonConstOpCodeMapPtr  mxSymbolsNative;                        // native symbols
    static NonConstOpCodeMapPtr  mxSymbolsEnglish;                       // English symbols
    static CharClass            *pCharClassEnglish;                      // character classification for en_US locale
    static const Convention     *pConventions[ ScAddress::CONV_LAST ];

    static const Convention * const pConvOOO_A1;
    static const Convention * const pConvOOO_A1_ODF;
    static const Convention * const pConvXL_A1;
    static const Convention * const pConvXL_R1C1;

    static struct AddInMap
    {
        const char* pODFF;
        const char* pEnglish;
        bool        bMapDupToInternal;      // when writing ODFF
        const char* pOriginal;              // programmatical name
        const char* pUpper;                 // upper case programmatical name
    } maAddInMap[];
    static const AddInMap* GetAddInMap();
    static size_t GetAddInMapCount();

    ScDocument* pDoc;
    ScAddress   aPos;

    String      aCorrectedFormula;                  // autocorrected Formula
    String      aCorrectedSymbol;                   // autocorrected Symbol
    sal_Unicode cSymbol[MAXSTRLEN];                 // current Symbol
    String      aFormula;                           // formula source code
    xub_StrLen  nSrcPos;                            // tokenizer position (source code)
    ScTokenArray* pArr;
    ScRawTokenRef   pRawToken;
    ScTokenRef  pToken;                     // current token
    ScTokenRef  pCurrentFactorToken;        // current factor token (of Factor() method)
    OpCode      eLastOp;
    ScToken**   pCode;
    ScArrayStack* pStack;
    OpCodeMapPtr mxSymbols;                 // which symbols are used
    const CharClass*    pCharClass;         // which character classification is used for parseAnyToken
    USHORT      pc;
    USHORT      mnPredetectedReference;     // reference when reading ODF, 0 (none), 1 (single) or 2 (double)
    short       nNumFmt;                    // set during CompileTokenArray()
    SCsTAB      nMaxTab;                    // last sheet in document
    short       nRecursion;                 // GetToken() recursions
    BOOL        glSubTotal;                 // if code contains one or more subtotal functions
    BOOL        bAutoCorrect;               // whether to apply AutoCorrection
    BOOL        bCorrected;                 // AutoCorrection was applied
    BOOL        bCompileForFAP;             //! not real RPN but names, for FunctionAutoPilot,
                                            // will not be resolved
    BOOL        bIgnoreErrors;              // on AutoCorrect and CompileForFAP
                                            // ignore errors and create RPN nevertheless
    const Convention *pConv;
    bool        mbCloseBrackets;            // whether to close open brackets automatically, default TRUE
    ScGrammar::Grammar  meGrammar;          // The grammar used, language plus convention.

    BOOL   GetToken();
    BOOL   NextNewToken(bool bAllowBooleans = false);
    OpCode NextToken();
    void PutCode( ScTokenRef& );
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

    void SetError(USHORT nError);
    xub_StrLen NextSymbol();
    BOOL IsValue( const String& );
    BOOL IsOpCode( const String& );
    BOOL IsOpCode2( const String& );
    BOOL IsString();
    BOOL IsReference( const String& );
    BOOL IsSingleReference( const String& );
    BOOL IsPredetectedReference( const String& );
    BOOL IsDoubleReference( const String& );
    BOOL IsMacro( const String& );
    BOOL IsNamedRange( const String& );
    BOOL IsDBRange( const String& );
    BOOL IsColRowName( const String& );
    BOOL IsBoolean( const String& );
    void AutoCorrectParsedSymbol();
    void AdjustReference( SingleRefData& r );
    void PushTokenArray( ScTokenArray*, BOOL = FALSE );
    void PopTokenArray();
    void SetRelNameReference();
    bool MergeRangeReference( ScToken * * const pCode1, ScToken * const * const pCode2 );
    void CreateStringFromScMatrix( rtl::OUStringBuffer& rBuffer, const ScMatrix* pMatrix );

    void AppendBoolean( rtl::OUStringBuffer& rBuffer, bool bVal );
    void AppendDouble( rtl::OUStringBuffer& rBuffer, double fVal );
    void AppendString( rtl::OUStringBuffer& rBuffer, const String & rStr );

    static void InitCharClassEnglish();

public:
    ScCompiler( ScDocument* pDocument, const ScAddress&,
                const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );

    ScCompiler( ScDocument* pDocument, const ScAddress&,
                ScTokenArray& rArr,
                const ScGrammar::Grammar eGrammar = ScGrammar::GRAM_DEFAULT );

    static bool IsInitialized()
    {
        return mxSymbolsNative.get() && mxSymbolsEnglish.get();
    }
    static void InitSymbolsNative();    /// only SymbolsNative, on first document creation
    static void InitSymbolsEnglish();   /// only SymbolsEnglish, maybe later
private:
    static void InitSymbolsPODF();      /// only SymbolsPODF, on demand
    static void InitSymbolsODFF();      /// only SymbolsODFF, on demand
    static void fillFromAddInMap( NonConstOpCodeMapPtr xMap, size_t nSymbolOffset );
    static void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap );
    static void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap );
public:
    static void DeInit();               /// all

    // for ScAddress::Format()
    static void CheckTabQuotes( String& aTabName,
                                const ScAddress::Convention eConv = ScAddress::CONV_OOO );

    static BOOL EnQuote( String& rStr );
    static BOOL DeQuote( String& rStr );

    static const String& GetNativeSymbol( OpCode eOp )
    {
        return mxSymbolsNative->getSymbol( eOp );
    }

    /** Get OpCode for English symbol.
        Used in XFunctionAccess to create token array.
        @param rName
            Symbol to lookup. MUST be upper case.
     */
    static OpCode GetEnglishOpCode( const String& rName );

    /** Get OpCodeMap for formula language.
        @param nLanguage
            One of ::com::sun::star::sheet::FormulaLanguage constants.
        @return Map for nLanguage. If nLanguage is unknown, a NULL map is returned.
     */
    static OpCodeMapPtr GetOpCodeMap( const sal_Int32 nLanguage );

    //! _either_ CompileForFAP _or_ AutoCorrection, _not_ both
    void            SetCompileForFAP( BOOL bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }
    void            SetAutoCorrection( BOOL bVal )
                        { bAutoCorrect = bVal; bIgnoreErrors = bVal; }
    void            SetCloseBrackets( bool bVal ) { mbCloseBrackets = bVal; }
    void            SetRefConvention( const Convention *pConvP );
    void            SetRefConvention( const ScAddress::Convention eConv );

    /** Create an internal symbol map from API mapping.
        @param bEnglish
            Use English number parser / formatter instead of native.
     */
    static OpCodeMapPtr CreateOpCodeMap(
            const ::com::sun::star::uno::Sequence<
            const ::com::sun::star::sheet::FormulaOpCodeMapEntry > & rMapping,
            bool bEnglish );

    /// Set symbol map if not empty.
    void            SetFormulaLanguage( const OpCodeMapPtr & xMap );

    /** Set symbol map corresponding to one of predefined ScGrammar::Grammar,
        including an address reference convention. */
    void            SetGrammar( const ScGrammar::Grammar eGrammar );
    inline  ScGrammar::Grammar   GetGrammar() const { return meGrammar; }

    BOOL            IsCorrected() { return bCorrected; }
    const String&   GetCorrectedFormula() { return aCorrectedFormula; }

    // Use convention from this->aPos by default
    ScTokenArray* CompileString( const String& rFormula,
                                 ScAddress::Convention eConv = ScAddress::CONV_UNSPECIFIED );
    BOOL  CompileTokenArray();
    short GetNumFormatType() { return nNumFmt; }
    const ScDocument* GetDoc() const { return pDoc; }
    const ScAddress& GetPos() const { return aPos; }

    static const String& GetStringFromOpCode( OpCode eOpCode );

    ScToken* CreateStringFromToken( String& rFormula, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    ScToken* CreateStringFromToken( rtl::OUStringBuffer& rBuffer, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    void CreateStringFromTokenArray( String& rFormula );
    void CreateStringFromTokenArray( rtl::OUStringBuffer& rBuffer );

    void MoveRelWrap();
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc,
                             const ScAddress& rPos );

    BOOL UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                              BOOL& rChanged, BOOL bSharedFormula = FALSE);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  BOOL& rChanged, BOOL& rRefSizeChanged );

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    ScRangeData* UpdateInsertTab(SCTAB nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(SCTAB nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);
    ScRangeData* UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, BOOL bIsName );

    BOOL HasModifiedRange();

    /// If the character is allowed as first character in sheet names or references
    static inline BOOL IsCharWordChar( String const & rStr,
                                       xub_StrLen nPos,
                                       const ScAddress::Convention eConv = ScAddress::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            return c < 128 ?
                static_cast<BOOL>(
                    (pConventions[eConv]->mpCharTable[ UINT8(c) ] & SC_COMPILER_C_CHAR_WORD) == SC_COMPILER_C_CHAR_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

    /// If the character is allowed in sheet names or references
    static inline BOOL IsWordChar( String const & rStr,
                                   xub_StrLen nPos,
                                   const ScAddress::Convention eConv = ScAddress::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            return c < 128 ?
                static_cast<BOOL>(
                    (pConventions[eConv]->mpCharTable[ UINT8(c) ] & SC_COMPILER_C_WORD) == SC_COMPILER_C_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

private:
    /// Access the CharTable flags
    inline ULONG GetCharTableFlags( sal_Unicode c )
        { return c < 128 ? pConv->mpCharTable[ UINT8(c) ] : 0; }

    static inline void ForceArrayOperator( ScTokenRef& rCurr, const ScTokenRef& rPrev )
        {
            if ( rPrev.Is() && rPrev->HasForceArray() &&
                    rCurr->GetType() == svByte && rCurr->GetOpCode() != ocPush
                    && !rCurr->HasForceArray() )
                rCurr->SetForceArray( true);
        }
    // SUNWS7 needs a forward declared friend, otherwise members of the outer
    // class are not accessible.
    class CurrentFactor;
    friend class ScCompiler::CurrentFactor;
    class CurrentFactor
    {
        ScTokenRef  pPrevFac;
        ScCompiler* pCompiler;
        // not implemented
        CurrentFactor( const CurrentFactor& );
        CurrentFactor& operator=( const CurrentFactor& );
    public:
        explicit CurrentFactor( ScCompiler* pComp )
            : pPrevFac( pComp->pCurrentFactorToken )
            , pCompiler( pComp )
            {}
        ~CurrentFactor()
            { pCompiler->pCurrentFactorToken = pPrevFac; }
        // yes, this operator= may modify the RValue
        void operator=( ScTokenRef& r )
            {
                ForceArrayOperator( r, pPrevFac);
                pCompiler->pCurrentFactorToken = r;
            }
        void operator=( ScToken* p )
            {
                ScTokenRef xTemp( p );
                *this = xTemp;
            }
        operator ScTokenRef&()
            { return pCompiler->pCurrentFactorToken; }
        ScToken* operator->()
            { return pCompiler->pCurrentFactorToken.operator->(); }
        operator ScToken*()
            { return operator->(); }
    };
};

SC_DLLPUBLIC String GetScCompilerNativeSymbol( OpCode eOp ); //CHINA001

#endif
