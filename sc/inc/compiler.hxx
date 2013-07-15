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

#ifndef SC_COMPILER_HXX
#define SC_COMPILER_HXX

#include <string.h>

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
#include <boost/unordered_map.hpp>

//-----------------------------------------------

// constants and data types also for external modules (ScInterpreter et al)

#define MAXSTRLEN    1024   /* maximum length of input string of one symbol */

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
#define SC_COMPILER_C_CHAR_ERRCONST   0x01000000  // start character of an error constant ('#')

#define SC_COMPILER_FILE_TAB_SEP      '#'         // 'Doc'#Tab


class ScDocument;
class ScMatrix;
class ScRangeData;
class ScExternalRefManager;
class ScTokenArray;

// constants and data types internal to compiler

/*
    OpCode              eOp;        // OpCode
    formula::StackVar   eType;      // type of data
    sal_uInt16          nRefCnt;    // reference count
    bool                bRaw;       // not cloned yet and trimmed to real size
 */

struct ScRawTokenBase
{
protected:
    OpCode   eOp;
    formula::StackVar eType;
    mutable sal_uInt16   nRefCnt;
    bool     bRaw;
};

struct ScDoubleRawToken: private ScRawTokenBase
{
public:
    union
    {   // union only to assure alignment identical to ScRawToken
        double      nValue;
        struct {
            sal_uInt8        cByte;
            bool        bHasForceArray;
        } sbyte;
    };
                DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRawToken );
};

struct ScRawToken: private ScRawTokenBase
{
    friend class ScCompiler;
    // Friends that use a temporary ScRawToken on the stack (and therefor need
    // the private dtor) and know what they're doing..
    friend class ScTokenArray;
    static sal_uInt16 sbyteOffset();
public:
    union {
        double       nValue;
        struct {
            sal_uInt8        cByte;
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
        struct {
            bool        bGlobal;
            sal_uInt16  nIndex;
        } name;
        ScMatrix*    pMat;
        sal_uInt16   nError;
        sal_Unicode  cStr[ MAXSTRLEN+1 ];   // string (up to 255 characters + 0)
        short        nJump[ FORMULA_MAXJUMPCOUNT + 1 ];     // If/Chose token
    };

                //! other members not initialized
                ScRawToken() { bRaw = true; }
private:
                ~ScRawToken() {}                //! only delete via Delete()
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScRawToken );
    formula::StackVar    GetType()   const       { return (formula::StackVar) eType; }
    OpCode      GetOpCode() const       { return (OpCode)   eOp;   }
    void        NewOpCode( OpCode e )   { eOp = e; }
    void        IncRef()                { nRefCnt++;       }
    void        DecRef()                { if( !--nRefCnt ) Delete(); }
    sal_uInt16      GetRef() const          { return nRefCnt; }
    SC_DLLPUBLIC void       Delete();

    // Use these methods only on tokens that are not part of a token array,
    // since the reference count is cleared!
    void SetOpCode( OpCode eCode );
    void SetString( const sal_Unicode* pStr );
    void SetSingleReference( const ScSingleRefData& rRef );
    void SetDoubleReference( const ScComplexRefData& rRef );
    void SetDouble( double fVal );
    void SetErrorConstant( sal_uInt16 nErr );

    // These methods are ok to use, reference count not cleared.
    void SetName(bool bGlobal, sal_uInt16 nIndex);
    void SetExternalSingleRef( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef );
    void SetExternalDoubleRef( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef );
    void SetExternalName( sal_uInt16 nFileId, const String& rName );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);

    /** If the token is a non-external reference, determine if the reference is
        valid. If the token is an external reference, return true. Else return
        false. Used only in ScCompiler::NextNewToken() to preserve non-existing
        sheet names in otherwise valid references.
     */
    bool IsValidReference() const;

    ScRawToken* Clone() const;      // real copy!
    formula::FormulaToken* CreateToken() const;   // create typified token
    void Load( SvStream&, sal_uInt16 nVer );

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

    enum ExtendedErrorDetection
    {
        EXTENDED_ERROR_DETECTION_NONE = 0,      // no error on unknown symbols, default (interpreter handles it)
        EXTENDED_ERROR_DETECTION_NAME_BREAK,    // name error on unknown symbols and break, pCode incomplete
        EXTENDED_ERROR_DETECTION_NAME_NO_BREAK  // name error on unknown symbols, don't break, continue
    };

    struct Convention
    {
        const formula::FormulaGrammar::AddressConvention meConv;

        Convention( formula::FormulaGrammar::AddressConvention eConvP );
        virtual ~Convention();

        virtual void MakeRefStr( OUStringBuffer&   rBuffer,
                                 const ScCompiler&      rCompiler,
                                 const ScComplexRefData&    rRef,
                                 bool bSingleRef ) const = 0;
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

        virtual void makeExternalRefStr( OUStringBuffer& rBuffer, const ScCompiler& rCompiler,
                                         sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef,
                                         ScExternalRefManager* pRefMgr ) const = 0;

        virtual void makeExternalRefStr( OUStringBuffer& rBuffer, const ScCompiler& rCompiler,
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

        virtual sal_uLong getCharTableFlags( sal_Unicode c, sal_Unicode cLast ) const = 0;

    protected:
        const sal_uLong* mpCharTable;
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
    mutable ScRawTokenRef pRawToken;

    const CharClass*    pCharClass;         // which character classification is used for parseAnyToken
    sal_uInt16      mnPredetectedReference;     // reference when reading ODF, 0 (none), 1 (single) or 2 (double)
    SCsTAB      nMaxTab;                    // last sheet in document
    sal_Int32   mnRangeOpPosInSymbol;       // if and where a range operator is in symbol
    const Convention *pConv;
    EncodeUrlMode   meEncodeUrlMode;
    ExtendedErrorDetection  meExtendedErrorDetection;
    bool        mbCloseBrackets;            // whether to close open brackets automatically, default TRUE
    bool        mbRewind;                   // whether symbol is to be rewound to some step during lexical analysis
    std::vector<sal_uInt16> maExternalFiles;

    bool   NextNewToken(bool bInArray = false);

    virtual void SetError(sal_uInt16 nError);
    xub_StrLen NextSymbol(bool bInArray);
    bool IsValue( const String& );
    bool IsOpCode( const String&, bool bInArray );
    bool IsOpCode2( const String& );
    bool IsString();
    bool IsReference( const String& );
    bool IsSingleReference( const String& );
    bool IsPredetectedReference( const String& );
    bool IsDoubleReference( const String& );
    bool IsMacro( const String& );
    bool IsNamedRange( const String& );
    bool IsExternalNamedRange( const String& rSymbol );
    bool IsDBRange( const String& );
    bool IsColRowName( const String& );
    bool IsBoolean( const String& );
    void AutoCorrectParsedSymbol();

    void SetRelNameReference();

    /** Obtain range data for ocName token, global or sheet local.

        Prerequisite: rToken is a FormulaIndexToken so IsGlobal() and
        GetIndex() can be called on it. We don't check with RTTI.
     */
    ScRangeData* GetRangeData( const formula::FormulaToken& pToken ) const;

    static void InitCharClassEnglish();

public:
    ScCompiler( ScDocument* pDocument, const ScAddress&);

    ScCompiler( ScDocument* pDocument, const ScAddress&,ScTokenArray& rArr);

public:
    static void DeInit();               /// all

    // for ScAddress::Format()
    static void CheckTabQuotes( String& aTabName,
                                const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO );

    /** Analyzes a string for a 'Doc'#Tab construct, or 'Do''c'#Tab etc..

        @returns the position of the unquoted # hash mark in 'Doc'#Tab, or
                 STRING_NOTFOUND if none. */
    static xub_StrLen GetDocTabPos( const String& rString );

    static bool EnQuote( String& rStr );
    sal_Unicode GetNativeAddressSymbol( Convention::SpecialSymbolType eType ) const;

    // Check if it is a valid english function name
    bool IsEnglishSymbol( const String& rName );
    bool IsErrorConstant( const String& ) const;

    //! _either_ CompileForFAP _or_ AutoCorrection, _not_ both
    // #i101512# SetCompileForFAP is in formula::FormulaCompiler
    void            SetAutoCorrection( bool bVal )
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

    void            CreateStringFromXMLTokenArray( OUString& rFormula, OUString& rFormulaNmsp );

    void            SetExtendedErrorDetection( ExtendedErrorDetection eVal ) { meExtendedErrorDetection = eVal; }

    bool            IsCorrected() { return bCorrected; }
    const String&   GetCorrectedFormula() { return aCorrectedFormula; }

    // Use convention from this->aPos by default
    ScTokenArray* CompileString( const OUString& rFormula );
    ScTokenArray* CompileString( const OUString& rFormula, const OUString& rFormulaNmsp );
    const ScDocument* GetDoc() const { return pDoc; }
    const ScAddress& GetPos() const { return aPos; }

    void MoveRelWrap( SCCOL nMaxCol, SCROW nMaxRow );
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc, const ScAddress& rPos,
                             SCCOL nMaxCol, SCROW nMaxRow );

    bool UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                              bool& rChanged, bool bSharedFormula = false, bool bLocal = false);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  bool& rChanged, bool& rRefSizeChanged );

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    ScRangeData* UpdateInsertTab(SCTAB nTable, bool bIsName, SCTAB nNewSheets = 1 );
    ScRangeData* UpdateDeleteTab(SCTAB nTable, bool bIsMove, bool bIsName, bool& bCompile, SCTAB nSheets = 1);
    ScRangeData* UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, bool bIsName );

    bool HasModifiedRange();

    /** If the character is allowed as first character in sheet names or
        references, includes '$' and '?'. */
    static inline bool IsCharWordChar( String const & rStr,
                                       xub_StrLen nPos,
                                       const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            sal_Unicode cLast = nPos > 0 ? rStr.GetChar(nPos-1) : 0;
            if (c < 128)
            {
                return pConventions[eConv] ? (pConventions[eConv]->getCharTableFlags(c, cLast) & SC_COMPILER_C_CHAR_WORD) == SC_COMPILER_C_CHAR_WORD :
                    false;   // no convention => assume invalid
            }
            else
                return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

    /** If the character is allowed in sheet names, thus may be part of a
        reference, includes '$' and '?' and such. */
    static inline bool IsWordChar( String const & rStr,
                                   xub_StrLen nPos,
                                   const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO )
        {
            sal_Unicode c = rStr.GetChar( nPos );
            sal_Unicode cLast = nPos > 0 ? rStr.GetChar(nPos-1) : 0;
            if (c < 128)
            {
                return pConventions[eConv] ? (pConventions[eConv]->getCharTableFlags(c, cLast) & SC_COMPILER_C_WORD) == SC_COMPILER_C_WORD :
                    false;   // convention not known => assume invalid
            }
            else
                return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
        }

    /** If the character is allowed as tested by nFlags (SC_COMPILER_C_...
        bits) for all known address conventions. If more than one bit is given
        in nFlags, all bits must match. If bTestLetterNumeric is false and
        char>=128, no LetterNumeric test is done and false is returned. */
    static inline bool IsCharFlagAllConventions( String const & rStr,
                                                 xub_StrLen nPos,
                                                 sal_uLong nFlags,
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
    virtual String FindAddInFunction( const String& rUpperName, bool bLocalFirst ) const;
    virtual void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const;
    virtual void fillFromAddInMap( NonConstOpCodeMapPtr xMap, formula::FormulaGrammar::Grammar _eGrammar ) const;
    virtual void fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const;

    virtual bool HandleExternalReference(const formula::FormulaToken& _aToken);
    virtual bool HandleRange();
    virtual bool HandleSingleRef();
    virtual bool HandleDbData();

    virtual formula::FormulaTokenRef ExtendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, bool bReuseDoubleRef );
    virtual void CreateStringFromExternal(OUStringBuffer& rBuffer, formula::FormulaToken* pTokenP);
    virtual void CreateStringFromSingleRef(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromDoubleRef(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromMatrix( OUStringBuffer& rBuffer, formula::FormulaToken* _pTokenP);
    virtual void CreateStringFromIndex(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP);
    virtual void LocalizeString( String& rName );   // modify rName - input: exact name

    /// Access the CharTable flags
    inline sal_uLong GetCharTableFlags( sal_Unicode c, sal_Unicode cLast )
        { return c < 128 ? pConv->getCharTableFlags(c, cLast) : 0; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
