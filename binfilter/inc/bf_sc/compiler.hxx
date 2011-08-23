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

#include <string.h>		// wg. memcpy() und bitte nicht memory.h (WKC)

#include <tools/mempool.hxx>


#include "token.hxx"

#include <unotools/charclass.hxx>

#include <rtl/ustrbuf.hxx>
#include <hash_map>

namespace binfilter {

//-----------------------------------------------

// Konstanten und Datentypen auch fuer externe Module (Interpreter etc.):

#define MAXCODE					512			// Maximale Anzahl Tokens
#define MAXSTRLEN				256			// Maximale Laenge fuer Eingabestring
                                            // (auch Formeln), "Verlaengerung" durch
                                            // Namensvergabe moeglich
#define MAXJUMPCOUNT			32
#define MAXMATSIZE				100
#define	SC_DOUBLE_MAXVALUE	1.7e307

#define errIllegalChar			501
#define errIllegalArgument		502
#define	errIllegalFPOperation	503
#define errIllegalParameter		504
#define errIllegalJump			505
#define errSeparator			506
#define errPair					507
#define errPairExpected			508
#define errOperatorExpected		509
#define errVariableExpected		510
#define errParameterExpected	511
#define	errCodeOverflow			512
#define errStringOverflow		513
#define	errStackOverflow		514
#define errUnknownState			515
#define	errUnknownVariable		516
#define errUnknownOpCode		517
#define errUnknownStackVariable	518
#define errNoValue				519
#define errUnknownToken			520
#define errNoCode				521
#define	errCircularReference	522
#define errNoConvergence        523
#define errNoRef				524
#define errNoName				525
#define errDoubleRef			526
#define errInterpOverflow		527
// wird nicht angezeigt, temporaer fuer TrackFormulas
// Zelle haengt von einer ab, die errCircularReference ist
#define errTrackFromCircRef		528
// Interpreter internal: existing cell has no value but value queried
#define errCellNoValue          529
// Interpreter: needed AddIn not found
#define errNoAddin              530
// Interpreter: needed Macro not found
#define errNoMacro              531

#define NOVALUE					0x7fff


// flag values of CharTable
#define SC_COMPILER_C_ILLEGAL       0x00000000
#define SC_COMPILER_C_CHAR          0x00000001
#define SC_COMPILER_C_CHAR_BOOL     0x00000002
#define SC_COMPILER_C_CHAR_WORD     0x00000004
#define SC_COMPILER_C_CHAR_VALUE    0x00000008
#define SC_COMPILER_C_CHAR_STRING   0x00000010
#define SC_COMPILER_C_CHAR_DONTCARE 0x00000020
#define SC_COMPILER_C_BOOL          0x00000040
#define SC_COMPILER_C_WORD          0x00000080  // auch in spstring.cxx dekl.
#define SC_COMPILER_C_WORD_SEP      0x00000100  // und rangenam.cxx (WORD und CHAR_WORD)
#define SC_COMPILER_C_VALUE         0x00000200
#define SC_COMPILER_C_VALUE_SEP     0x00000400
#define SC_COMPILER_C_VALUE_EXP     0x00000800
#define SC_COMPILER_C_VALUE_SIGN    0x00001000
#define SC_COMPILER_C_VALUE_VALUE   0x00002000
#define SC_COMPILER_C_STRING_SEP    0x00004000
#define SC_COMPILER_C_NAME_SEP      0x00008000  // es kann nur einen geben! '\''
#define SC_COMPILER_C_CHAR_IDENT    0x00010000  // identifier (built-in function) start
#define SC_COMPILER_C_IDENT         0x00020000  // identifier continuation

#define SC_COMPILER_FILE_TAB_SEP	'#'			// 'Doc'#Tab  auch in global2.cxx


class ScDocument;
class ScMatrix;
class ScRangeData;

// Konstanten und Datentypen compilerintern:


#if 0
/*
    OpCode	 eOp;			// OpCode
    StackVar eType;			// type of data
    USHORT	 nRefCnt;		// reference count
    BOOL	 bRaw;			// nicht per Clone auf passende Groesse getrimmt
 */
#endif

#define SC_TOKEN_FIX_MEMBERS	\
    OpCode	 eOp;				\
    StackVar eType;				\
    USHORT	 nRefCnt;			\
    BOOL	 bRaw;

struct ScDoubleRawToken
{
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union
    {	// union nur um gleiches Alignment wie ScRawToken sicherzustellen
        double		nValue;
        BYTE		cByte;
    };
                DECL_FIXEDMEMPOOL_NEWDEL( ScDoubleRawToken );
};

struct ScRawToken
{
    friend class ScCompiler;
    // Friends, die temporaere Raw-Tokens auf dem Stack benutzen (und deswegen
    // den private dtor brauchen) und wissen, was sie tun..
    friend class ScTokenArray;
    friend class ScInterpreter;
private:
    SC_TOKEN_FIX_MEMBERS
public:
    union {
        double		 nValue;
        ComplRefData aRef;
        BYTE		 cByte;
        ScMatrix* 	 pMat;
        USHORT 		 nIndex;		  		// Index fuer Name-Tabelle
        sal_Unicode	 cStr[ MAXSTRLEN+1 ];	// String (bis 255 Zeichen + 0)
        short  		 nJump[MAXJUMPCOUNT+1];	// If/Chose-Token
    };

                ScRawToken() : bRaw( TRUE ) {}
private:
                ~ScRawToken() {}				//! nur via Delete() loeschen
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScRawToken );
    StackVar	GetType()   const		{ return (StackVar) eType; }
    OpCode		GetOpCode() const		{ return (OpCode)   eOp;   }
    void		NewOpCode( OpCode e )	{ eOp = e; }
    void		IncRef()				{ nRefCnt++;	   }
    void		DecRef()				{ if( !--nRefCnt ) Delete(); }
    USHORT		GetRef() const			{ return nRefCnt; }
    void		Delete();

    // Diese Methoden nur auf Tokens, die nicht im TokenArray stehen,
    // anwenden, da der RefCount auf 0 gesetzt wird!
    void SetOpCode( OpCode eCode );
    void SetString( const sal_Unicode* pStr );
    void SetSingleReference( const SingleRefData& rRef );
    void SetDoubleReference( const ComplRefData& rRef );
    void SetDouble( double fVal );
    void SetName( USHORT n );
    void SetExternal(const sal_Unicode* pStr);

    ScRawToken* Clone() const;		// real copy!
    ScToken* CreateToken() const;	// create typified token
    void Load30( SvStream& );
    void Load( SvStream&, USHORT nVer );

    static xub_StrLen GetStrLen( const sal_Unicode* pStr );	// as long as a "string" is an array
    static size_t GetStrLenBytes( xub_StrLen nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};


class ScRawTokenRef
{
    ScRawToken* p;
public:
    inline ScRawTokenRef() { p = NULL; }
    inline ScRawTokenRef( const ScRawTokenRef& r )	{ if( ( p = r.p ) != NULL ) p->IncRef(); }
    inline ScRawTokenRef( ScRawToken *t )		{ if( ( p = t ) != NULL ) t->IncRef(); }
    inline void Clear()							{ if( p ) p->DecRef(); }
    inline ~ScRawTokenRef()						{ if( p ) p->DecRef(); }
    inline ScRawTokenRef& operator=( const ScRawTokenRef& r ) { return *this = r.p; }
    inline ScRawTokenRef& operator=( ScRawToken* t )
        { if( t ) t->IncRef(); if( p ) p->DecRef(); p = t; return *this; }
    inline BOOL Is() const						{ return p != NULL; }
    inline BOOL	operator ! () const				{ return p == NULL; }
    inline ScRawToken* operator&() const 		{ return p;	}
    inline ScRawToken* operator->() const 		{ return p;	}
    inline ScRawToken& operator*() const 		{ return *p; }
    inline operator ScRawToken*() const 		{ return p; }
};


enum ScRecalcMode40
{							// alter RecalcMode bis einschliesslich 4.0
    RC_NORMAL,
    RC_ALWAYS,
    RC_ONLOAD,				// immer nach dem Laden
    RC_ONLOAD_ONCE,			// einmal nach dem Laden
    RC_FORCED,				// auch wenn Zelle nicht sichtbar
    RC_ONREFMOVE	 		// wenn Referenz bewegt wurde (ab SP3, 05.11.97)
};

// neuer (ab 5.0, 14.01.98) RecalcMode Zugriff nur noch noch via TokenArray
// SetRecalcMode / IsRecalcMode...

typedef BYTE ScRecalcMode;
// von den exklusiven Bits kann nur eins gesetzt sein
// wird von TokenArray SetRecalcMode... Methoden gehandelt
#define RECALCMODE_NORMAL		0x01	// exklusiv
#define RECALCMODE_ALWAYS		0x02	// exklusiv, immer
#define RECALCMODE_ONLOAD		0x04	// exklusiv, immer nach dem Laden
#define RECALCMODE_ONLOAD_ONCE	0x08	// exklusiv, einmal nach dem Laden
#define RECALCMODE_FORCED		0x10	// kombiniert, auch wenn Zelle nicht sichtbar
#define RECALCMODE_ONREFMOVE	0x20	// kombiniert, wenn Referenz bewegt wurde
#define RECALCMODE_EMASK		0x0F	// Maske der exklusiven Bits
// werden neue Bits definiert, muessen ExportRecalcMode40 und AddRecalcMode
// angepasst werden!

class ScTokenArray
{
    friend class ScCompiler;
    friend class ScTokenIterator;

    ScToken**       pCode;                  // Token code array
    ScToken**       pRPN;                   // RPN array
    USHORT          nLen;                   // Length of token array
    USHORT          nRPN;                   // Length of RPN array
    USHORT          nIndex;                 // Current step index
    USHORT          nError;                 // Error code
    short           nRefs;                  // Count of cell references
    ScRecalcMode    nMode;                  // Flags to indicate when to recalc this code
    BOOL            bReplacedSharedFormula; // If code was created by replacing
                            // a shared formula, a temporary flag during
                            // UpdateReference() until StartListeningTo()

    void					Assign( const ScTokenArray& );

    ScToken*				Add( ScToken* );
    void					ImportRecalcMode40( ScRecalcMode40 );
    ScRecalcMode40 			ExportRecalcMode40() const;
    inline	void			SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                            { nMode |= (nBits & ~RECALCMODE_EMASK); }
    inline	ScRecalcMode	GetCombinedBitsRecalcMode() const
                            { return nMode & ~RECALCMODE_EMASK; }
    inline	void			SetMaskedRecalcMode( ScRecalcMode nBits )
                            { nMode = GetCombinedBitsRecalcMode() | nBits; }
                            // in nMode gesetzte exklusive Bits werden genullt,
                            // nVal kann auch kombinierte Bits enthalten,
                            // aber nur ein exklusives Bit darf gesetzt sein!

    BOOL                    ImplGetReference( ScRange& rRange, BOOL bValidOnly ) const;

public:
    ScTokenArray();
    ScTokenArray( const ScTokenArray& );	// Zuweisung mit Referenzen auf Tokens
   ~ScTokenArray();
    ScTokenArray* Clone() const;	// echte Kopie!
    void Clear();
    void DelRPN();
    ScToken* First() { nIndex = 0; return Next(); }
    ScToken* Next();
    ScToken* GetNextReference();
    ScToken* GetNextReferenceRPN();
    ScToken* GetNextReferenceOrName();
    ScToken* GetNextOpCodeRPN( OpCode );
     ScToken* PeekNext();
    ScToken* PeekPrevNoSpaces();	// nur nach First/Next/Last/Prev !
    ScToken* PeekNextNoSpaces();    // nur nach First/Next/Last/Prev !
    ScToken* FirstRPN() { nIndex = 0; return NextRPN(); }
    ScToken* NextRPN();

    BOOL	HasOpCodeRPN( OpCode ) const;
    BOOL    HasNameOrColRowName() const;    // token of type svIndex or opcode ocColRowName
    BOOL    IsReference( ScRange& rRange ) const;       // exactly and only one range (valid or deleted)

    ScToken** GetArray() const  { return pCode; }
    ScToken** GetCode()  const  { return pRPN; }
    USHORT    GetLen() const     { return nLen; }
    USHORT	  GetCodeLen() const { return nRPN; }
    void      Reset()            { nIndex = 0; }
    USHORT 	  GetError() const { return nError; }
    void 	  SetError( USHORT n ) { nError = n; }
    short  	  GetRefs()  const { return nRefs;  }
    void      SetReplacedSharedFormula( BOOL bVal ) { bReplacedSharedFormula = bVal; }
    BOOL      IsReplacedSharedFormula() const       { return bReplacedSharedFormula; }

    inline	ScRecalcMode	GetRecalcMode() const { return nMode; }
            void			AddRecalcMode( ScRecalcMode nBits );
                                // Bits werden nicht direkt gesetzt sondern
                                // geprueft und evtl. nach Prio behandelt,
                                // wenn mehrere exklusive gesetzt sind!

    inline	void			ClearRecalcMode() { nMode = RECALCMODE_NORMAL; }
    inline	void			SetRecalcModeNormal()
                                { SetMaskedRecalcMode( RECALCMODE_NORMAL ); }
    inline	void			SetRecalcModeAlways()
                                { SetMaskedRecalcMode( RECALCMODE_ALWAYS ); }
    inline	void			SetRecalcModeOnLoad()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD ); }
    inline	void			SetRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD_ONCE ); }
    inline	void			SetRecalcModeForced()
                                { nMode |= RECALCMODE_FORCED; }
    inline	void			ClearRecalcModeForced()
                                { nMode &= ~RECALCMODE_FORCED; }
    inline	void			SetRecalcModeOnRefMove()
                                { nMode |= RECALCMODE_ONREFMOVE; }
    inline	void			ClearRecalcModeOnRefMove()
                                { nMode &= ~RECALCMODE_ONREFMOVE; }
    inline	BOOL			IsRecalcModeNormal() const
                                { return (nMode & RECALCMODE_NORMAL) != 0; }
    inline	BOOL			IsRecalcModeAlways() const
                                { return (nMode & RECALCMODE_ALWAYS) != 0; }
    inline	BOOL			IsRecalcModeOnLoad() const
                                { return (nMode & RECALCMODE_ONLOAD) != 0; }
    inline	BOOL			IsRecalcModeOnLoadOnce() const
                                { return (nMode & RECALCMODE_ONLOAD_ONCE) != 0; }
    inline	BOOL			IsRecalcModeForced() const
                                { return (nMode & RECALCMODE_FORCED) != 0; }
    inline	BOOL			IsRecalcModeOnRefMove() const
                                { return (nMode & RECALCMODE_ONREFMOVE) != 0; }

    inline OpCode	GetOuterFuncOpCode();		// OpCode der aeusseren Funktion
    BOOL					HasMatrixDoubleRefOps();

    void Load30( SvStream&, const ScAddress& );
    void Load( SvStream&, USHORT, const ScAddress& );
    void Store( SvStream&, const ScAddress& ) const;

    ScToken* AddToken( const ScRawToken& );
     ScToken* AddToken( const ScToken& );
    ScToken* AddOpCode( OpCode eCode );
    ScToken* AddString( const sal_Unicode* pStr );
    ScToken* AddString( const String& rStr );
    ScToken* AddDouble( double fVal );
    ScToken* AddSingleReference( const SingleRefData& rRef );
    ScToken* AddDoubleReference( const ComplRefData& rRef );
    ScToken* AddExternal( const sal_Unicode* pStr );
    ScToken* AddExternal( const String& rStr );
    ScToken* AddBad( const String& rStr );			// ocBad with String

    // Zuweisung mit Referenzen auf Tokens
     ScTokenArray& operator=( const ScTokenArray& );
};

inline OpCode ScTokenArray::GetOuterFuncOpCode()
{
    if ( pRPN && nRPN )
        return pRPN[nRPN-1]->GetOpCode();
    return ocNone;
}

struct ImpTokenIterator;

class ScTokenIterator
{
    friend class ScInterpreter;		// fuer Jump()
    ImpTokenIterator* pCur;
    void Jump( short nStart, short nNext );
    void Push( ScTokenArray* );
    void Pop();
public:
    ScTokenIterator( const ScTokenArray& );
   ~ScTokenIterator();
    void  Reset();
    const ScToken* Next();
};

struct ScArrayStack;

struct ScStringHashCode
{
    size_t operator()( const String& rStr ) const
    {
        return rtl_ustr_hashCode_WithLength( rStr.GetBuffer(), rStr.Len() );
    }
};
typedef ::std::hash_map< String, OpCode, ScStringHashCode, ::std::equal_to< String > > ScOpCodeHashMap;

class ScCompiler
{
public:
    static String*	pSymbolTableNative;				// array of native symbols, offset==OpCode
    static String*	pSymbolTableEnglish; 			// array of English symbols, offset==OpCode
    static USHORT	nAnzStrings;					// count of symbols
    static ScOpCodeHashMap* pSymbolHashMapNative;   // hash map of native symbols
    static ScOpCodeHashMap* pSymbolHashMapEnglish;  // hash map of English symbols
private:
    static ULONG*   pCharTable;						// array of ASCII character flags
    ScDocument*	pDoc;
    ScAddress	aPos;
    String      cFormula;							// String -> TokenArray
    String		aCorrectedFormula;					// autocorrected Formula
    String		aCorrectedSymbol;					// autocorrected Symbol
    sal_Unicode	cSymbol[MAXSTRLEN];					// current Symbol
    String		aFormula;							// formula source code
    xub_StrLen	nSrcPos;							// tokenizer position (source code)
    ScTokenArray* pArr;
    ScRawTokenRef	pRawToken;
    ScTokenRef	pToken;
    OpCode		eLastOp;
    ScToken** 	pCode;
    ScArrayStack* pStack;
    String*		pSymbolTable;				// which symbol table is used
    ScOpCodeHashMap*    pSymbolHashMap;     // which symbol hash map is used
    USHORT		pc;
    short		nNumFmt;					// bei CompileTokenArray() gesetzt
    short		nMaxTab;					// letzte Tabelle im Doc
    short		nRecursion;					// GetToken Rekursionen
    BOOL		glSubTotal;					// Merkvariablen fuer Zelle
    BOOL		bAutoCorrect;				// AutoCorrection ausfuehren
    BOOL		bCorrected;					// AutoCorrection ausgefuehrt
    BOOL		bCompileForFAP;				//! kein echtes RPN sondern Namen
                                            // werden nicht aufgeloest
    BOOL		bIgnoreErrors;				// bei AutoCorrect und CompileForFAP
                                            // trotz Fehler RPN erzeugen
    BOOL		bCompileXML;
    BOOL		bImportXML;

    BOOL   GetToken();
    BOOL   NextNewToken();
    OpCode NextToken();
    void PutCode( ScToken* );
    void Factor();
    void UnionCutLine();
    void Unary();
    void PowLine();
    void MulDivLine();
    void AddSubLine();
    void ConcatLine();
    void CompareLine();
    void NotLine();
    OpCode Expression();

    void MakeColStr( ::rtl::OUStringBuffer& rBuffer, USHORT nCol );
    void MakeRowStr( ::rtl::OUStringBuffer& rBuffer, USHORT nRow );
    String MakeTabStr( USHORT nTab, String& aDoc );
    void MakeRefStr( ::rtl::OUStringBuffer& rBuffer, ComplRefData& rRefData, BOOL bSingleRef );

    void SetError(USHORT nError);
    xub_StrLen NextSymbol();
    BOOL IsValue( const String& );
    BOOL IsOpCode( const String& );
    BOOL IsOpCode2( const String& );
    BOOL IsString();
    BOOL IsReference( const String& );
    BOOL IsMacro( const String& );
    BOOL IsNamedRange( const String& );
    BOOL IsDBRange( const String& );
    BOOL IsColRowName( const String& );
    void AutoCorrectParsedSymbol();
    void PushTokenArray( ScTokenArray*, BOOL = FALSE );
    void PopTokenArray();
    void SetRelNameReference();
public:
    ScCompiler(ScDocument* pDocument, const ScAddress& );

    ScCompiler( ScDocument* pDocument, const ScAddress&,
                const ScTokenArray& rArr );

    static void Init();
    static void DeInit();
    static void CheckTabQuotes( String& );	// fuer ScAddress::Format()

    // schliesst String in Quotes ein, wenn nicht-alphanumerische Zeichen
    // enthalten sind, bestehende Quotes werden escaped
    static BOOL EnQuote( String& );
    // entfernt Quotes, escaped Quotes werden unescaped
    static BOOL DeQuote( String& );

    //! _entweder_ CompileForFAP _oder_ AutoCorrection, _nicht_ beides
    void			SetCompileForFAP( BOOL bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }
    void 			SetAutoCorrection( BOOL bVal )
                        { bAutoCorrect = bVal; bIgnoreErrors = bVal; }
    void			SetCompileEnglish( BOOL bVal );		// use English SymbolTable
    void			SetCompileXML( BOOL bVal )
                        { bCompileXML = bVal; }
    void			SetImportXML( BOOL bVal )
                        { bImportXML = bVal; }
    BOOL			IsCorrected() { return bCorrected; }
    const String&	GetCorrectedFormula() { return aCorrectedFormula; }
    ScTokenArray* CompileString( const String& rFormula );
    BOOL  CompileTokenArray();
    short GetNumFormatType() { return nNumFmt; }

    ScToken* CreateStringFromToken( String& rFormula, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    ScToken* CreateStringFromToken( ::rtl::OUStringBuffer& rBuffer, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    void CreateStringFromTokenArray( String& rFormula );
    void CreateStringFromTokenArray( ::rtl::OUStringBuffer& rBuffer );

    void MoveRelWrap();

    BOOL UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              short nDx, short nDy, short nDz,
                              BOOL& rChanged);

    ScRangeData* UpdateInsertTab(USHORT nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(USHORT nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);

    /// Is the CharTable initialized? If not call Init() yourself!
    static inline BOOL HasCharTable() { return pCharTable != NULL; }

    /// Access the CharTable flags
    static inline ULONG GetCharTableFlags( sal_Unicode c )
        { return c < 128 ? pCharTable[ UINT8(c) ] : 0; }

    /// If the character is allowed as first character in sheet names or references
    static inline BOOL IsCharWordChar( sal_Unicode c )
        {
            return c < 128 ?
                ((pCharTable[ UINT8(c) ] & SC_COMPILER_C_CHAR_WORD) == SC_COMPILER_C_CHAR_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( c );
        }

    /// If the character is allowed in sheet names or references
    static inline BOOL IsWordChar( sal_Unicode c )
        {
            return c < 128 ?
                ((pCharTable[ UINT8(c) ] & SC_COMPILER_C_WORD) == SC_COMPILER_C_WORD) :
                ScGlobal::pCharClass->isLetterNumeric( c );
        }
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
