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

#ifndef INCLUDED_SC_INC_COMPILER_HXX
#define INCLUDED_SC_INC_COMPILER_HXX

#include <string.h>

#include <tools/mempool.hxx>
#include "scdllapi.h"
#include "global.hxx"
#include "refdata.hxx"
#include <formula/token.hxx>
#include <formula/grammar.hxx>
#include <unotools/charclass.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/sheet/ExternalLinkInfo.hpp>
#include <vector>

#include <formula/FormulaCompiler.hxx>

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

namespace sc {

class CompileFormulaContext;

}

// constants and data types internal to compiler

/*
    OpCode              eOp;        // OpCode
    formula::StackVar   eType;      // type of data
 */

struct ScRawTokenBase
{
protected:
    OpCode   eOp;
    formula::StackVar eType;
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
    // Friends that use a temporary ScRawToken on the stack (and therefore need
    // the private dtor) and know what they're doing..
    friend class ScTokenArray;
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
        struct {
            rtl_uString* mpData;
            rtl_uString* mpDataIgnoreCase;
        } sharedstring;
        ScMatrix*    pMat;
        sal_uInt16   nError;
        sal_Unicode  cStr[ MAXSTRLEN+1 ];   // string (up to 255 characters + 0)
        short        nJump[ FORMULA_MAXJUMPCOUNT + 1 ];     // If/Chose token
    };

                //! members not initialized
                ScRawToken() {}
private:
                ~ScRawToken() {}                //! only delete via Delete()
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScRawToken );
    formula::StackVar    GetType()   const       { return eType; }
    OpCode      GetOpCode() const       { return eOp; }
    void        NewOpCode( OpCode e )   { eOp = e; }

    // Use these methods only on tokens that are not part of a token array,
    // since the reference count is cleared!
    void SetOpCode( OpCode eCode );
    void SetString( rtl_uString* pData, rtl_uString* pDataIgoreCase );
    void SetSingleReference( const ScSingleRefData& rRef );
    void SetDoubleReference( const ScComplexRefData& rRef );
    void SetDouble( double fVal );
    void SetErrorConstant( sal_uInt16 nErr );

    // These methods are ok to use, reference count not cleared.
    void SetName(bool bGlobal, sal_uInt16 nIndex);
    void SetExternalSingleRef( sal_uInt16 nFileId, const OUString& rTabName, const ScSingleRefData& rRef );
    void SetExternalDoubleRef( sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rRef );
    void SetExternalName( sal_uInt16 nFileId, const OUString& rName );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);

    /** If the token is a non-external reference, determine if the reference is
        valid. If the token is an external reference, return true. Else return
        false. Used only in ScCompiler::NextNewToken() to preserve non-existing
        sheet names in otherwise valid references.
     */
    bool IsValidReference() const;

    formula::FormulaToken* CreateToken() const;   // create typified token
    void Load( SvStream&, sal_uInt16 nVer );

    static sal_Int32 GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
    static size_t GetStrLenBytes( sal_Int32 nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};

class SC_DLLPUBLIC ScCompiler : public formula::FormulaCompiler
{
public:

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

        virtual void makeRefStr(
            OUStringBuffer& rBuffer,
            formula::FormulaGrammar::Grammar eGram,
            const ScAddress& rPos,
            const OUString& rErrRef, const std::vector<OUString>& rTabNames,
            const ScComplexRefData& rRef, bool bSingleRef ) const = 0;

        virtual ::com::sun::star::i18n::ParseResult
                    parseAnyToken( const OUString& rFormula,
                                   sal_Int32 nSrcPos,
                                   const CharClass* pCharClass) const = 0;

        /**
         * Parse the symbol string and pick up the file name and the external
         * range name.
         *
         * @return true on successful parse, or false otherwise.
         */
        virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
                const ScDocument* pDoc,
                const ::com::sun::star::uno::Sequence<
                    com::sun::star::sheet::ExternalLinkInfo>* pExternalLinks ) const = 0;

        virtual OUString makeExternalNameStr( sal_uInt16 nFileId, const OUString& rFile,
                const OUString& rName ) const = 0;

        virtual void makeExternalRefStr(
            OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 nFileId, const OUString& rFileName,
            const OUString& rTabName, const ScSingleRefData& rRef ) const = 0;

        virtual void makeExternalRefStr(
            OUStringBuffer& rBuffer, const ScAddress& rPos,
            sal_uInt16 nFileId, const OUString& rFileName, const std::vector<OUString>& rTabNames,
            const OUString& rTabName, const ScComplexRefData& rRef ) const = 0;

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

    SvNumberFormatter* mpFormatter;

    // For CONV_XL_OOX, may be set via API by MOOXML filter.
    com::sun::star::uno::Sequence<com::sun::star::sheet::ExternalLinkInfo> maExternalLinks;

    sal_Unicode cSymbol[MAXSTRLEN];                 // current Symbol
    OUString    aFormula;                           // formula source code
    sal_Int32   nSrcPos;                            // tokenizer position (source code)
    mutable ScRawToken maRawToken;

    const CharClass*    pCharClass;         // which character classification is used for parseAnyToken
    sal_uInt16      mnPredetectedReference;     // reference when reading ODF, 0 (none), 1 (single) or 2 (double)
    SCsTAB      nMaxTab;                    // last sheet in document
    sal_Int32   mnRangeOpPosInSymbol;       // if and where a range operator is in symbol
    const Convention *pConv;
    ExtendedErrorDetection  meExtendedErrorDetection;
    bool        mbCloseBrackets;            // whether to close open brackets automatically, default TRUE
    bool        mbRewind;                   // whether symbol is to be rewound to some step during lexical analysis
    std::vector<sal_uInt16> maExternalFiles;

    std::vector<OUString> maTabNames;                /// sheet names mangled for the current grammar for output
    std::vector<OUString> &GetSetupTabNames() const; /// get or setup tab names for the current grammar

    bool   NextNewToken(bool bInArray = false);

    virtual void SetError(sal_uInt16 nError) SAL_OVERRIDE;
    sal_Int32 NextSymbol(bool bInArray);
    bool IsValue( const OUString& );
    bool IsOpCode( const OUString&, bool bInArray );
    bool IsOpCode2( const OUString& );
    bool IsString();
    bool IsReference( const OUString& );
    bool IsSingleReference( const OUString& );
    bool IsPredetectedReference(const OUString&);
    bool IsDoubleReference( const OUString& );
    bool IsMacro( const OUString& );
    bool IsNamedRange( const OUString& );
    bool IsExternalNamedRange( const OUString& rSymbol, bool& rbInvalidExternalNameRange );
    bool IsDBRange( const OUString& );
    bool IsColRowName( const OUString& );
    bool IsBoolean( const OUString& );
    void AutoCorrectParsedSymbol();

    void SetRelNameReference();

    /** Obtain range data for ocName token, global or sheet local.

        Prerequisite: rToken is a FormulaIndexToken so IsGlobal() and
        GetIndex() can be called on it. We don't check with RTTI.
     */
    ScRangeData* GetRangeData( const formula::FormulaToken& pToken ) const;

    static void InitCharClassEnglish();

public:
    ScCompiler( sc::CompileFormulaContext& rCxt, const ScAddress& rPos );

    ScCompiler( ScDocument* pDocument, const ScAddress&);

    ScCompiler( sc::CompileFormulaContext& rCxt, const ScAddress& rPos, ScTokenArray& rArr );

    ScCompiler( ScDocument* pDocument, const ScAddress&,ScTokenArray& rArr);

    virtual ~ScCompiler();

public:
    static void DeInit();               /// all

    // for ScAddress::Format()
    static void CheckTabQuotes( OUString& aTabName,
                                const formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO );

    /** Analyzes a string for a 'Doc'#Tab construct, or 'Do''c'#Tab etc..

        @returns the position of the unquoted # hash mark in 'Doc'#Tab, or
                 -1 if none. */
    static sal_Int32 GetDocTabPos( const OUString& rString );

    static bool EnQuote( OUString& rStr );
    sal_Unicode GetNativeAddressSymbol( Convention::SpecialSymbolType eType ) const;

    // Check if it is a valid english function name
    bool IsEnglishSymbol( const OUString& rName );
    bool IsErrorConstant( const OUString& ) const;

    /**
     * When auto correction is set, the jump command reorder must be enabled.
     */
    void SetAutoCorrection( bool bVal );
    void            SetCloseBrackets( bool bVal ) { mbCloseBrackets = bVal; }
    void            SetRefConvention( const Convention *pConvP );
    void            SetRefConvention( const formula::FormulaGrammar::AddressConvention eConv );

    static const Convention* GetRefConvention( formula::FormulaGrammar::AddressConvention eConv );

    /// Set symbol map if not empty.
    void            SetFormulaLanguage( const OpCodeMapPtr & xMap );

    void            SetGrammar( const formula::FormulaGrammar::Grammar eGrammar );

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

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
    void SetExternalLinks(
        const ::com::sun::star::uno::Sequence<
            com::sun::star::sheet::ExternalLinkInfo>& rLinks )
    {
        maExternalLinks = rLinks;
    }

    void            CreateStringFromXMLTokenArray( OUString& rFormula, OUString& rFormulaNmsp );

    void            SetExtendedErrorDetection( ExtendedErrorDetection eVal ) { meExtendedErrorDetection = eVal; }

    bool            IsCorrected() { return bCorrected; }
    const OUString& GetCorrectedFormula() { return aCorrectedFormula; }

    /**
     * Tokenize formula expression string into an array of tokens.
     *
     * @param rFormula formula expression to tokenize.
     *
     * @return heap allocated token array object. The caller <i>must</i>
     *         manage the life cycle of this object.
     */
    ScTokenArray* CompileString( const OUString& rFormula );
    ScTokenArray* CompileString( const OUString& rFormula, const OUString& rFormulaNmsp );
    const ScDocument* GetDoc() const { return pDoc; }
    const ScAddress& GetPos() const { return aPos; }

    void MoveRelWrap( SCCOL nMaxCol, SCROW nMaxRow );
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc, const ScAddress& rPos,
                             SCCOL nMaxCol, SCROW nMaxRow );

    /** If the character is allowed as tested by nFlags (SC_COMPILER_C_...
        bits) for all known address conventions. If more than one bit is given
        in nFlags, all bits must match. If bTestLetterNumeric is false and
        char>=128, no LetterNumeric test is done and false is returned. */
    static bool IsCharFlagAllConventions(
        OUString const & rStr, sal_Int32 nPos, sal_uLong nFlags, bool bTestLetterNumeric = true );

private:
    // FormulaCompiler
    virtual OUString FindAddInFunction( const OUString& rUpperName, bool bLocalFirst ) const SAL_OVERRIDE;
    virtual void fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const SAL_OVERRIDE;
    virtual void fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const SAL_OVERRIDE;
    virtual void fillFromAddInMap( NonConstOpCodeMapPtr xMap, formula::FormulaGrammar::Grammar _eGrammar ) const SAL_OVERRIDE;
    virtual void fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const SAL_OVERRIDE;

    virtual bool HandleExternalReference(const formula::FormulaToken& _aToken) SAL_OVERRIDE;
    virtual bool HandleRange() SAL_OVERRIDE;
    virtual bool HandleSingleRef() SAL_OVERRIDE;
    virtual bool HandleDbData() SAL_OVERRIDE;

    virtual formula::FormulaTokenRef ExtendRangeReference( formula::FormulaToken & rTok1, formula::FormulaToken & rTok2, bool bReuseDoubleRef ) SAL_OVERRIDE;
    virtual void CreateStringFromExternal(OUStringBuffer& rBuffer, formula::FormulaToken* pTokenP) const SAL_OVERRIDE;
    virtual void CreateStringFromSingleRef(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP) const SAL_OVERRIDE;
    virtual void CreateStringFromDoubleRef(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP) const SAL_OVERRIDE;
    virtual void CreateStringFromMatrix( OUStringBuffer& rBuffer, formula::FormulaToken* _pTokenP) const SAL_OVERRIDE;
    virtual void CreateStringFromIndex(OUStringBuffer& rBuffer,formula::FormulaToken* _pTokenP) const SAL_OVERRIDE;
    virtual void LocalizeString( OUString& rName ) const SAL_OVERRIDE;   // modify rName - input: exact name

    virtual bool IsForceArrayParameter( const formula::FormulaToken* pToken, sal_uInt16 nParam ) const;

    /// Access the CharTable flags
    inline sal_uLong GetCharTableFlags( sal_Unicode c, sal_Unicode cLast )
        { return c < 128 ? pConv->getCharTableFlags(c, cLast) : 0; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
