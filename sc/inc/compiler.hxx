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

#ifndef SC_COMPILER_HXX
#define SC_COMPILER_HXX

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif
#include <tools/mempool.hxx>
#include "scdllapi.h"
#include "global.hxx"
#include "refdata.hxx"
#include "formula/token.hxx"
#include "formula/grammar.hxx"
#include <unotools/charclass.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/sheet/ExternalLinkInfo.hpp>
#include <vector>

#include <formula/FormulaCompiler.hxx>

#include <boost/intrusive_ptr.hpp>

#ifndef INCLUDED_HASH_MAP
#include <boost/unordered_map.hpp>
#define INCLUDED_HASH_MAP
#endif

//-----------------------------------------------

// constants and data types also for external modules (ScInterpreter et al)

#define MAXCODE      512    /* maximum number of tokens in formula */
#define MAXSTRLEN    1024   /* maximum length of input string of one symbol */
#define MAXJUMPCOUNT 32     /* maximum number of jumps (ocChose) */

// flag values of CharTable
#define SC_COMPILER_C_ILLEGAL         0x00000000
#define SC_COMPILER_C_CHAR            0x00000001
#define SC_COMPILER_C_CHAR_BOOL       0x00000002
#define SC_COMPILER_C_CHAR_WORD       0x00000004
#define SC_COMPILER_C_CHAR_VALUE      0x00000008
#define SC_COMPILER_C_CHAR_STRING     0x00000010
#define SC_COMPILER_C_CHAR_DONTCARE   0x00000020
#define SC_COMPILER_C_BOOL            0x00000040
#define SC_COMPILER_C_WORD            0x00000080
#define SC_COMPILER_C_WORD_SEP        0x00000100
#define SC_COMPILER_C_VALUE           0x00000200
#define SC_COMPILER_C_VALUE_SEP       0x00000400
#define SC_COMPILER_C_VALUE_EXP       0x00000800
#define SC_COMPILER_C_VALUE_SIGN      0x00001000
#define SC_COMPILER_C_VALUE_VALUE     0x00002000
#define SC_COMPILER_C_STRING_SEP      0x00004000
#define SC_COMPILER_C_NAME_SEP        0x00008000  // there can be only one! '\''
#define SC_COMPILER_C_CHAR_IDENT      0x00010000  // identifier (built-in function) or reference start
#define SC_COMPILER_C_IDENT           0x00020000  // identifier or reference continuation
#define SC_COMPILER_C_ODF_LBRACKET    0x00040000  // ODF '[' reference bracket
#define SC_COMPILER_C_ODF_RBRACKET    0x00080000  // ODF ']' reference bracket
#define SC_COMPILER_C_ODF_LABEL_OP    0x00100000  // ODF '!!' automatic intersection of labels
#define SC_COMPILER_C_ODF_NAME_MARKER 0x00200000  // ODF '$$' marker that starts a defined (range) name
#define SC_COMPILER_C_CHAR_NAME       0x00400000  // start character of a defined name
#define SC_COMPILER_C_NAME            0x00800000  // continuation character of a defined name

#define SC_COMPILER_FILE_TAB_SEP      '#'         // 'Doc'#Tab


class ScDocument;
class ScMatrix;
class ScRangeData;
class ScExternalRefManager;
class ScTokenArray;

// constants and data types internal to compiler

/*
    OpCode   eOp;           // OpCode
    formula::StackVar eType;         // type of data
    USHORT   nRefCnt;       // reference count
    BOOL     bRaw;          // not cloned yet and trimmed to real size
 */

#define SC_TOKEN_FIX_MEMBERS    \
    OpCode   eOp;               \
    formula::StackVar eType;    \
    mutable USHORT   nRefCnt;   \
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
        ScComplexRefData aRef;
        struct {
            sal_uInt16      nFileId;
            sal_Unicode     cTabName[MAXSTRLEN+1];
            ScComplexRefData    aRef;
        } extref;
        struct {
            sal_uInt16  nFileId;
            sal_Unicode cName[MAXSTRLEN+1];
        } extname;
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
    formula::StackVar    GetType()   const       { return (formula::StackVar) eType; }
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
    void SetSingleReference( const ScSingleRefData& rRef );
    void SetDoubleReference( const ScComplexRefData& rRef );
    void SetDouble( double fVal );

    // These methods are ok to use, reference count not cleared.
    void SetName( USHORT n );
    void SetExternalSingleRef( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef );
    void SetExternalDoubleRef( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef );
    void SetExternalName( sal_uInt16 nFileId, const String& rName );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);

    ScRawToken* Clone() const;      // real copy!
    formula::FormulaToken* CreateToken() const;   // create typified token
    void Load( SvStream&, USHORT nVer );

    static xub_StrLen GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
    static size_t GetStrLenBytes( xub_StrLen nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};

inline void intrusive_ptr_add_ref(ScRawToken* p)
{
    p->IncRef();
}

inline void intrusive_ptr_release(ScRawToken* p)
{
    p->DecRef();
}

typedef ::boost::intrusive_ptr<ScRawToken> ScRawTokenRef;

class SC_DLLPUBLIC ScCompiler : public formula::FormulaCompiler
{
public:

    enum EncodeUrlMode
    {
        ENCODE_BY_GRAMMAR,
        ENCODE_ALWAYS,
        ENCODE_NEVER,
    };

    struct Convention
    {
        const formula::FormulaGrammar::AddressConvention meConv;

        Convention( formula::FormulaGrammar::AddressConvention eConvP );
        virtual ~Convention();

        virtual void MakeRefStr( rtl::OUStringBuffer&   rBuffer,
                                 const ScCompiler&      rCompiler,
                                 const ScComplexRefData&    rRef,
                                 BOOL bSingleRef ) const = 0;
        virtual ::com::sun::star::i18n::ParseResult
                    parseAnyToken( const String& rFormula,
                                   xub_StrLen nSrcPos,
                                   const CharClass* pCharClass) const = 0;

        /**
         * Parse the symbol string and pick up the file name and the external
         * range name.
         *
         * @return true on successful parse, or false otherwise.
         */
        virtual bool parseExternalName( const String& rSymbol, String& rFile, String& rName,
                const ScDocument* pDoc,
                const ::com::sun::star::uno::Sequence<
                    const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks ) const = 0;

        virtual String makeExternalNameStr( const String& rFile, const String& rName ) const = 0;

        virtual void makeExternalRefStr( ::rtl::OUStringBuffer& rBuffer, const ScCompiler& rCompiler,
                                         sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef,
                                         ScExternalRefManager* pRefMgr ) const = 0;

        virtual void makeExternalRefStr( ::rtl::OUStringBuffer& rBuffer, const ScCompiler& rCompiler,
                                         sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef,
                                         ScExternalRefManager* pRefMgr ) const = 0;

        enum SpecialSymbolType
        {
            /**
             * Character between sheet name and address.  In OOO A1 this is
             * '.', while XL A1 and XL R1C1 this is '!'.
             */
            SHEET_SEPARATOR,

            /**
             * In OOO A1, a sheet name may be prefixed with '$' to indicate an
             * absolute sheet position.
             */
            ABS_SHEET_PREFIX
        };
        virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const = 0;

        virtual ULONG getCharTableFlags( sal_Unicode c, sal_Unicode cLast ) const = 0;

    protected:
        const ULONG* mpCharTable;
    };
    friend struct Convention;

private:


    static CharClass            *pCharClassEnglish;                      // character classification for en_US locale
    static const Convention     *pConventions[ formula::FormulaGrammar::CONV_LAST ];

    static const Convention * const pConvOOO_A1;
    static const Convention * const pConvOOO_A1_ODF;
    static const Convention * const pConvXL_A1;
    static const Convention * const pConvXL_R1C1;
    static const Convention * const pConvXL_OOX;

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

    // For CONV_XL_OOX, may be set via API by MOOXML filter.
    ::com::sun::star::uno::Sequence< const ::com::sun::star::sheet::ExternalLinkInfo > maExternalLinks;

    sal_Unicode cSymbol[MAXSTRLEN];                 // current Symbol
    String      aFormula;                           // formula source code
    xub_StrLen  nSrcPos;                            // tokenizer position (source code)
    ScRawTokenRef   pRawToken;

    const CharClass*    pCharClass;         // which character classification is used for parseAnyToken
    USHORT      mnPredetectedReference;     // reference when reading ODF, 0 (none), 1 (single) or 2 (double)
    SCsTAB      nMaxTab;                    // last sheet in document
    sal_Int32   mnRangeOpPosInSymbol;       // if and where a range operator is in symbol
    const Convention *pConv;
    EncodeUrlMode   meEncodeUrlMode;
    bool        mbCloseBrackets;            // whether to close open brackets automatically, default TRUE
    bool        mbExtendedErrorDetection;
    bool        mbRewind;                   // whether symbol is to be rewound to some step during lexical analysis

    BOOL   NextNewToken(bool bInArray = false);

    virtual void SetError(USHORT nError);
    xub_StrLen NextSymbol(bool bInArray);
    BOOL IsValue( const String& );
    BOOL IsOpCode( const String&, bool bInArray );
    BOOL IsOpCode2( const String& );
    BOOL IsString();
    BOOL IsReference( const String& );
    BOOL IsSingleReference( const String& );
    BOOL IsPredetectedReference( const String& );
    BOOL IsDoubleReference( const String& );
    BOOL IsMacro( const String& );
    BOOL IsNamedRange( const String& );
    bool IsExternalNamedRange( const String& rSymbol );
    BOOL IsDBRange( const String& );
    BOOL IsColRowName( const String& );
    BOOL IsBoolean( const String& );
    void AutoCorrectParsedSymbol();

    void SetRelNameReference();

    static void InitCharClassEnglish();

public:
    ScCompiler( ScDocument* pDocument, const ScAddress&);

    ScCompiler( ScDocument* pDocument, const ScAddress&,ScTokenArray& rArr);

public:
    static void DeInit();               /// all

    // for ScAddress::Format()
    static void CheckTabQuotes( String& aTabName,
                                const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO );

    static BOOL EnQuote( String& rStr );
    sal_Unicode GetNativeAddressSymbol( Convention::SpecialSymbolType eType ) const;


    // Check if it is a valid english function name
    bool IsEnglishSymbol( const String& rName );

    //! _either_ CompileForFAP _or_ AutoCorrection, _not_ both
    // #i101512# SetCompileForFAP is in formula::FormulaCompiler
    void            SetAutoCorrection( BOOL bVal )
                        { bAutoCorrect = bVal; bIgnoreErrors = bVal; }
    void            SetCloseBrackets( bool bVal ) { mbCloseBrackets = bVal; }
    void            SetRefConvention( const Convention *pConvP );
    void            SetRefConvention( const formula::FormulaGrammar::AddressConvention eConv );

    /// Set symbol map if not empty.
    void            SetFormulaLanguage( const OpCodeMapPtr & xMap );

    void            SetGrammar( const formula::FormulaGrammar::Grammar eGrammar );

    EncodeUrlMode   GetEncodeUrlMode() const;
private:
    /** Set grammar and reference convention from within SetFormulaLanguage()
        or SetGrammar().

        @param eNewGrammar
            The new grammar to be set and the associated reference convention.

        @param eOldGrammar
            The previous grammar that was active before SetFormulaLanguage().
     */
    void            SetGrammarAndRefConvention(
                        const formula::FormulaGrammar::Grammar eNewGrammar,
                        const formula::FormulaGrammar::Grammar eOldGrammar );
public:

    /// Set external link info for ScAddress::CONV_XL_OOX.
    inline  void    SetExternalLinks(
            const ::com::sun::star::uno::Sequence<
            const ::com::sun::star::sheet::ExternalLinkInfo > & rLinks )
    {
        maExternalLinks = rLinks;
    }

    void            CreateStringFromXMLTokenArray( String& rFormula, String& rFormulaNmsp );

    void            SetExtendedErrorDetection( bool bVal ) { mbExtendedErrorDetection = bVal; }

    BOOL            IsCorrected() { return bCorrected; }
    const String&   GetCorrectedFormula() { return aCorrectedFormula; }

    // Use convention from this->aPos by default
    ScTokenArray* CompileString( const String& rFormula );
    ScTokenArray* CompileString( const String& rFormula, const String& rFormulaNmsp );
    const ScDocument* GetDoc() const { return pDoc; }
    const ScAddress& GetPos() const { return aPos; }

    void MoveRelWrap( SCCOL nMaxCol, SCROW nMaxRow );
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc, const ScAddress& rPos,
                             SCCOL nMaxCol, SCROW nMaxRow );

    BOOL UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                              BOOL& rChanged, BOOL bSharedFormula = FALSE);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  bool& rChanged, bool& rRefSizeChanged );

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    ScRangeData* UpdateInsertTab(SCTAB nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(SCTAB nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);
    ScRangeData* UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, BOOL bIsName );

    BOOL HasModifiedRange();

    /** If the character is allowed as first character in sheet names or
        references, includes '$' and '?'. */
    static inline BOOL IsCharWordChar( String const & rStr,
                                       xub_StrLen nPos,
                                       const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            sal_Unicode cLast = nPos > 0 ? rStr.GetChar(nPos-1) : 0;
            if (c < 128)
            {
                return pConventions[eConv] ? static_cast<BOOL>(
                        (pConventions[eConv]->getCharTableFlags(c, cLast) & SC_COMPILER_C_CHAR_WORD) == SC_COMPILER_C_CHAR_WORD) :
                    FALSE;   // no convention => assume invalid
            }
            else
                return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

    /** If the character is allowed in sheet names, thus may be part of a
        reference, includes '$' and '?' and such. */
    static inline BOOL IsWordChar( String const & rStr,
                                   xub_StrLen nPos,
                                   const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            sal_Unicode cLast = nPos > 0 ? rStr.GetChar(nPos-1) : 0;
            if (c < 128)
            {
                return pConventions[eConv] ? static_cast<BOOL>(
                        (pConventions[eConv]->getCharTableFlags(c, cLast) & SC_COMPILER_C_WORD) == SC_COMPILER_C_WORD) :
                    FALSE;   // convention not known => assume invalid
            }
            else
                return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

    /** If the character is allowed as tested by nFlags (SC_COMPILER_C_...
        bits) for all known address conventions. If more than one bit is given
        in nFlags, all bits must match. If bTestLetterNumeric is FALSE and
        char>=128, no LetterNumeric test is done and FALSE is returned. */
    static inline bool IsCharFlagAllConventions( String const & rStr,
                                                 xub_StrLen nPos,
                                                 ULONG nFlags,
                                                 bool bTestLetterNumeric = true )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            sal_Unicode cLast = nPos > 0 ? rStr.GetChar( nPos-1 ) : 0;
            if (c < 128)
            {
                for ( int nConv = formula::FormulaGrammar::CONV_UNSPECIFIED;
                        ++nConv < formula::FormulaGrammar::CONV_LAST; )
                {
                    if (pConventions[nConv] &&
                            ((pConventions[nConv]->getCharTableFlags(c, cLast) & nFlags) != nFlags))
                        return false;
                    // convention not known => assume valid
                }
                return true;
            }
            else if (bTestLetterNumeric)
                return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
            else
                return false;
        }

private:
    // FormulaCompiler
    virtual String FindAddInFunction( const String& rUpperName, BOOL bLocalFirst ) const;
    virtual void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInMap( NonConstOpCodeMapPtr xMap, formula::FormulaGrammar::Grammar _eGrammar ) const;
    virtual void fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const;

    virtual BOOL HandleExternalReference(const formula::FormulaToken& _aToken);
    virtual BOOL HandleRange();
    virtual BOOL HandleSingleRef();
    virtual BOOL HandleDbData();

    virtual formula::FormulaTokenRef ExtendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, bool bReuseDoubleRef );
    virtual void CreateStringFromExternal(rtl::OUStringBuffer& rBuffer, formula::FormulaToken* pTokenP);
    virtual void CreateStringFromSingleRef(rtl::OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromDoubleRef(rtl::OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromMatrix( rtl::OUStringBuffer& rBuffer, formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromIndex(rtl::OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void LocalizeString( String& rName );   // modify rName - input: exact name
    virtual BOOL IsImportingXML() const;

    /// Access the CharTable flags
    inline ULONG GetCharTableFlags( sal_Unicode c, sal_Unicode cLast )
        { return c < 128 ? pConv->getCharTableFlags(c, cLast) : 0; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
