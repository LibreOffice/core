/*************************************************************************
 *
 *  $RCSfile: compiler.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2001-02-22 14:28:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_COMPILER_HXX
#define SC_COMPILER_HXX

#include <string.h>     // wg. memcpy() und bitte nicht memory.h (WKC)

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_TOKEN_HXX
#include "token.hxx"
#endif

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif


//-----------------------------------------------

// Konstanten und Datentypen auch fuer externe Module (Interpreter etc.):

#define MAXCODE                 512         // Maximale Anzahl Tokens
#define MAXSTRLEN               256         // Maximale Laenge fuer Eingabestring
                                            // (auch Formeln), "Verlaengerung" durch
                                            // Namensvergabe moeglich
#define MAXJUMPCOUNT            32
#define MAXMATSIZE              100
#define SC_DOUBLE_MAXVALUE  1.7e307

#define errIllegalChar          501
#define errIllegalArgument      502
#define errIllegalFPOperation   503
#define errIllegalParameter     504
#define errIllegalJump          505
#define errSeparator            506
#define errPair                 507
#define errPairExpected         508
#define errOperatorExpected     509
#define errVariableExpected     510
#define errParameterExpected    511
#define errCodeOverflow         512
#define errStringOverflow       513
#define errStackOverflow        514
#define errUnknownState         515
#define errUnknownVariable      516
#define errUnknownOpCode        517
#define errUnknownStackVariable 518
#define errNoValue              519
#define errUnknownToken         520
#define errNoCode               521
#define errCircularReference    522
#define errNoConvergence        523
#define errNoRef                524
#define errNoName               525
#define errDoubleRef            526
#define errInterpOverflow       527
// wird nicht angezeigt, temporaer fuer TrackFormulas
// Zelle haengt von einer ab, die errCircularReference ist
#define errTrackFromCircRef     528

#define NOVALUE                 0x7fff


// flag values of CharTable
#define SC_COMPILER_C_ILLEGAL       0x0000
#define SC_COMPILER_C_CHAR          0x0001
#define SC_COMPILER_C_CHAR_BOOL     0x0002
#define SC_COMPILER_C_CHAR_WORD     0x0004
#define SC_COMPILER_C_CHAR_VALUE    0x0008
#define SC_COMPILER_C_CHAR_STRING   0x0010
#define SC_COMPILER_C_CHAR_DONTCARE 0x0020
#define SC_COMPILER_C_BOOL          0x0040
#define SC_COMPILER_C_WORD          0x0080      // auch in spstring.cxx dekl.
#define SC_COMPILER_C_WORD_SEP      0x0100      // und rangenam.cxx (WORD und CHAR_WORD)
#define SC_COMPILER_C_VALUE         0x0200
#define SC_COMPILER_C_VALUE_SEP     0x0400
#define SC_COMPILER_C_VALUE_EXP     0x0800
#define SC_COMPILER_C_VALUE_SIGN    0x1000
#define SC_COMPILER_C_VALUE_VALUE   0x2000
#define SC_COMPILER_C_STRING_SEP    0x4000
#define SC_COMPILER_C_NAME_SEP      0x8000      // es kann nur einen geben! '\''

#define SC_COMPILER_FILE_TAB_SEP    '#'         // 'Doc'#Tab  auch in global2.cxx


class ScDocument;
class ScMatrix;
class ScRangeData;

// Konstanten und Datentypen compilerintern:


#if 0
/*
    OpCode   eOp;           // OpCode
    StackVar eType;         // type of data
    USHORT   nRefCnt;       // reference count
    BOOL     bRaw;          // nicht per Clone auf passende Groesse getrimmt
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
    {   // union nur um gleiches Alignment wie ScRawToken sicherzustellen
        double      nValue;
        BYTE        cByte;
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
        double       nValue;
        ComplRefData aRef;
        BYTE         cByte;
        ScMatrix*    pMat;
        USHORT       nIndex;                // Index fuer Name-Tabelle
        sal_Unicode  cStr[ MAXSTRLEN+1 ];   // String (bis 255 Zeichen + 0)
        short        nJump[MAXJUMPCOUNT+1]; // If/Chose-Token
    };

                ScRawToken() : bRaw( TRUE ) {}
private:
                ~ScRawToken() {}                //! nur via Delete() loeschen
public:
                DECL_FIXEDMEMPOOL_NEWDEL( ScRawToken );
    StackVar    GetType()   const       { return (StackVar) eType; }
    OpCode      GetOpCode() const       { return (OpCode)   eOp;   }
    void        NewOpCode( OpCode e )   { eOp = e; }
    void        IncRef()                { nRefCnt++;       }
    void        DecRef()                { if( !--nRefCnt ) Delete(); }
    USHORT      GetRef() const          { return nRefCnt; }
    void        Delete();

    // Diese Methoden nur auf Tokens, die nicht im TokenArray stehen,
    // anwenden, da der RefCount auf 0 gesetzt wird!
    void SetOpCode( OpCode eCode );
    void SetString( const sal_Unicode* pStr );
    void SetSingleReference( const SingleRefData& rRef );
    void SetDoubleReference( const ComplRefData& rRef );
    void SetByte( BYTE c );
    void SetDouble( double fVal );
    void SetInt( int nVal );
    void SetName( USHORT n );
    void SetMatrix( ScMatrix* p );
    void SetExternal(const sal_Unicode* pStr);
    // Diese Methoden sind hingegen OK.
    ComplRefData& GetReference();
    void SetReference( ComplRefData& rRef );

    ScRawToken* Clone() const;      // real copy!
    ScToken* CreateToken() const;   // create typified token
    void Load30( SvStream& );
    void Load( SvStream&, USHORT nVer );

    static xub_StrLen GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
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
    inline ScRawTokenRef( const ScRawTokenRef& r )  { if( ( p = r.p ) != NULL ) p->IncRef(); }
    inline ScRawTokenRef( ScRawToken *t )       { if( ( p = t ) != NULL ) t->IncRef(); }
    inline void Clear()                         { if( p ) p->DecRef(); }
    inline ~ScRawTokenRef()                     { if( p ) p->DecRef(); }
    inline ScRawTokenRef& operator=( const ScRawTokenRef& r ) { return *this = r.p; }
    inline ScRawTokenRef& operator=( ScRawToken* t )
        { if( t ) t->IncRef(); if( p ) p->DecRef(); p = t; return *this; }
    inline BOOL Is() const                      { return p != NULL; }
    inline BOOL operator ! () const             { return p == NULL; }
    inline ScRawToken* operator&() const        { return p; }
    inline ScRawToken* operator->() const       { return p; }
    inline ScRawToken& operator*() const        { return *p; }
    inline operator ScRawToken*() const         { return p; }
};


enum ScRecalcMode40
{                           // alter RecalcMode bis einschliesslich 4.0
    RC_NORMAL,
    RC_ALWAYS,
    RC_ONLOAD,              // immer nach dem Laden
    RC_ONLOAD_ONCE,         // einmal nach dem Laden
    RC_FORCED,              // auch wenn Zelle nicht sichtbar
    RC_ONREFMOVE            // wenn Referenz bewegt wurde (ab SP3, 05.11.97)
};

// neuer (ab 5.0, 14.01.98) RecalcMode Zugriff nur noch noch via TokenArray
// SetRecalcMode / IsRecalcMode...

typedef BYTE ScRecalcMode;
// von den exklusiven Bits kann nur eins gesetzt sein
// wird von TokenArray SetRecalcMode... Methoden gehandelt
#define RECALCMODE_NORMAL       0x01    // exklusiv
#define RECALCMODE_ALWAYS       0x02    // exklusiv, immer
#define RECALCMODE_ONLOAD       0x04    // exklusiv, immer nach dem Laden
#define RECALCMODE_ONLOAD_ONCE  0x08    // exklusiv, einmal nach dem Laden
#define RECALCMODE_FORCED       0x10    // kombiniert, auch wenn Zelle nicht sichtbar
#define RECALCMODE_ONREFMOVE    0x20    // kombiniert, wenn Referenz bewegt wurde
#define RECALCMODE_EMASK        0x0F    // Maske der exklusiven Bits
// werden neue Bits definiert, muessen ExportRecalcMode40 und AddRecalcMode
// angepasst werden!

class ScTokenArray
{
    friend class ScCompiler;
    friend class ScTokenIterator;

    ScToken** pCode;        // Token->Array
    ScToken** pRPN;         // RPN-Array
    USHORT nLen;            // Laenge des TokenArrays
    USHORT nRPN;            // Laenge des RPN-Arrays
    USHORT nIndex;          // aktueller Step-Index
    USHORT nError;          // Fehlercode
    short  nRefs;           // Anzahl Referenzen
    ScRecalcMode nMode;     // wann muss berechnet werden?

    void                    Assign( const ScTokenArray& );

    ScToken*                Add( ScToken* );
    void                    ImportRecalcMode40( ScRecalcMode40 );
    ScRecalcMode40          ExportRecalcMode40() const;
    inline  void            SetCombinedBitsRecalcMode( ScRecalcMode nBits )
                            { nMode |= (nBits & ~RECALCMODE_EMASK); }
    inline  ScRecalcMode    GetCombinedBitsRecalcMode() const
                            { return nMode & ~RECALCMODE_EMASK; }
    inline  void            SetMaskedRecalcMode( ScRecalcMode nBits )
                            { nMode = GetCombinedBitsRecalcMode() | nBits; }
                            // in nMode gesetzte exklusive Bits werden genullt,
                            // nVal kann auch kombinierte Bits enthalten,
                            // aber nur ein exklusives Bit darf gesetzt sein!

public:
    ScTokenArray();
    ScTokenArray( const ScTokenArray& );    // Zuweisung mit Referenzen auf Tokens
   ~ScTokenArray();
    ScTokenArray* Clone() const;    // echte Kopie!
    void Clear();
    void DelRPN();
    ScToken* First() { nIndex = 0; return Next(); }
    ScToken* Next();
    ScToken* GetNextName();
    ScToken* GetNextDBArea();
    ScToken* GetNextReference();
    ScToken* GetNextReferenceRPN();
    ScToken* GetNextReferenceOrName();
    ScToken* GetNextColRowName();
    ScToken* GetNextOpCodeRPN( OpCode );
    ScToken* PeekNext();
    ScToken* PeekPrevNoSpaces();    // nur nach First/Next/Last/Prev !
    ScToken* PeekNextNoSpaces();    // nur nach First/Next/Last/Prev !
    ScToken* FirstRPN() { nIndex = 0; return NextRPN(); }
    ScToken* NextRPN();
    ScToken* LastRPN() { nIndex = nRPN; return PrevRPN(); }
    ScToken* PrevRPN();

    BOOL    HasOpCodeRPN( OpCode ) const;
    BOOL    HasName() const;        // token of type svIndex
    BOOL    IsReference( ScRange& rRange ) const;       // exactly and only one range
    BOOL    GetTableOpRefs(                             // exactly and only one multiple operation
                ScAddress& rFormula,
                ScAddress& rColFirstPos, ScAddress& rColRelPos,
                ScAddress& rRowFirstPos, ScAddress& rRowRelPos,
                BOOL& rbIsMode2 ) const;

    ScToken** GetArray() const  { return pCode; }
    ScToken** GetCode()  const  { return pRPN; }
    USHORT    GetLen() const     { return nLen; }
    USHORT    GetCodeLen() const { return nRPN; }
    void      Reset()            { nIndex = 0; }
    USHORT    GetError() const { return nError; }
    void      SetError( USHORT n ) { nError = n; }
    short     GetRefs()  const { return nRefs;  }

    inline  ScRecalcMode    GetRecalcMode() const { return nMode; }
            void            AddRecalcMode( ScRecalcMode nBits );
                                // Bits werden nicht direkt gesetzt sondern
                                // geprueft und evtl. nach Prio behandelt,
                                // wenn mehrere exklusive gesetzt sind!

    inline  void            ClearRecalcMode() { nMode = RECALCMODE_NORMAL; }
    inline  void            SetRecalcModeNormal()
                                { SetMaskedRecalcMode( RECALCMODE_NORMAL ); }
    inline  void            SetRecalcModeAlways()
                                { SetMaskedRecalcMode( RECALCMODE_ALWAYS ); }
    inline  void            SetRecalcModeOnLoad()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD ); }
    inline  void            SetRecalcModeOnLoadOnce()
                                { SetMaskedRecalcMode( RECALCMODE_ONLOAD_ONCE ); }
    inline  void            SetRecalcModeForced()
                                { nMode |= RECALCMODE_FORCED; }
    inline  void            ClearRecalcModeForced()
                                { nMode &= ~RECALCMODE_FORCED; }
    inline  void            SetRecalcModeOnRefMove()
                                { nMode |= RECALCMODE_ONREFMOVE; }
    inline  void            ClearRecalcModeOnRefMove()
                                { nMode &= ~RECALCMODE_ONREFMOVE; }
    inline  BOOL            IsRecalcModeNormal() const
                                { return (nMode & RECALCMODE_NORMAL) != 0; }
    inline  BOOL            IsRecalcModeAlways() const
                                { return (nMode & RECALCMODE_ALWAYS) != 0; }
    inline  BOOL            IsRecalcModeOnLoad() const
                                { return (nMode & RECALCMODE_ONLOAD) != 0; }
    inline  BOOL            IsRecalcModeOnLoadOnce() const
                                { return (nMode & RECALCMODE_ONLOAD_ONCE) != 0; }
    inline  BOOL            IsRecalcModeForced() const
                                { return (nMode & RECALCMODE_FORCED) != 0; }
    inline  BOOL            IsRecalcModeOnRefMove() const
                                { return (nMode & RECALCMODE_ONREFMOVE) != 0; }

    inline OpCode   GetOuterFuncOpCode();       // OpCode der aeusseren Funktion
                // ermittelt Ausdehnung direkt angrenzender Refs
                // nur auf echte Funcs wie z.B. GetOuterFuncOpCode() == ocSum anwenden!
    BOOL        GetAdjacentExtendOfOuterFuncRefs( USHORT& nExtend,
                    const ScAddress& rPos, ScDirection );

                            // Operatoren +,-,*,/,^,&,=,<>,<,>,<=,>=
                            // mit DoubleRef in Formel?
    BOOL                    HasMatrixDoubleRefOps();

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
    ScToken* AddName( USHORT n );
    ScToken* AddMatrix( ScMatrix* p );
    ScToken* AddExternal( const sal_Unicode* pStr );
    ScToken* AddExternal( const String& rStr );
    ScToken* AddColRowName( const SingleRefData& rRef );
    ScToken* AddBad( const sal_Unicode* pStr );     // ocBad with String
    ScToken* AddBad( const String& rStr );          // ocBad with String

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
    friend class ScInterpreter;     // fuer Jump()
    ImpTokenIterator* pCur;
    void Jump( short nStart, short nNext );
    void Push( ScTokenArray* );
    void Pop();
public:
    ScTokenIterator( const ScTokenArray& );
   ~ScTokenIterator();
    void  Reset();
    const ScToken* First();
    const ScToken* Next();
};

struct ScArrayStack;

class ScCompiler
{
public:
    static String*  pSymbolTableNative;             // Liste der Symbole: String[]
    static String*  pSymbolTableEnglish;            // Liste der Symbole English
    static USHORT   nAnzStrings;                    // Anzahl der Symbole
private:
    static USHORT*  pCharTable;                     // char[];
    ScDocument* pDoc;
    ScAddress   aPos;
    String      cFormula;                           // String -> TokenArray
    String      aCorrectedFormula;                  // autocorrected Formula
    String      aCorrectedSymbol;                   // autocorrected Symbol
    sal_Unicode cSymbol[MAXSTRLEN];                 // current Symbol
    String      aFormula;                           // formula source code
    xub_StrLen  nSrcPos;                            // tokenizer position (source code)
    ScTokenArray* pArr;
    ScRawTokenRef   pRawToken;
    ScTokenRef  pToken;
    OpCode      eLastOp;
    ScToken**   pCode;
    ScArrayStack* pStack;
    String*     pSymbolTable;               // welche SymbolTable benutzt wird
    USHORT      pc;
    short       nNumFmt;                    // bei CompileTokenArray() gesetzt
    short       nMaxTab;                    // letzte Tabelle im Doc
    short       nRecursion;                 // GetToken Rekursionen
    BOOL        glSubTotal;                 // Merkvariablen fuer Zelle
    BOOL        bAutoCorrect;               // AutoCorrection ausfuehren
    BOOL        bCorrected;                 // AutoCorrection ausgefuehrt
    BOOL        bCompileForFAP;             //! kein echtes RPN sondern Namen
                                            // werden nicht aufgeloest
    BOOL        bIgnoreErrors;              // bei AutoCorrect und CompileForFAP
                                            // trotz Fehler RPN erzeugen
    BOOL        bCompileXML;
    BOOL        bImportXML;

    BOOL   GetToken();
    BOOL   NextNewToken();
    OpCode NextToken();
    void PutCode( ScRawToken* );
    void PutCode( ScToken* );
    void Factor();
    void UnionCutLine();
    void Unary();
    void PowLine();
    void MulDivLine();
    void AddSubLine();
    void CompareLine();
    void NotLine();
    OpCode Expression();

    String MakeColStr( USHORT nCol );
    String MakeRowStr( USHORT nRow );
    String MakeTabStr( USHORT nTab, String& aDoc );
    String MakeRefStr( ComplRefData& rRefData, BOOL bSingleRef );

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
    void AdjustReference( SingleRefData& r );
    void PushTokenArray( ScTokenArray*, BOOL = FALSE );
    void PopTokenArray();
    void SetRelNameReference();
public:
    ScCompiler(ScDocument* pDocument, const ScAddress& );

    ScCompiler( ScDocument* pDocument, const ScAddress&,
                const ScTokenArray& rArr );

    static void Init();
    static void DeInit();
    static void CheckTabQuotes( String& );  // fuer ScAddress::Format()

    // schliesst String in Quotes ein, wenn nicht-alphanumerische Zeichen
    // enthalten sind, bestehende Quotes werden escaped
    static BOOL EnQuote( String& );
    // entfernt Quotes, escaped Quotes werden unescaped
    static BOOL DeQuote( String& );

    //! _entweder_ CompileForFAP _oder_ AutoCorrection, _nicht_ beides
    void            SetCompileForFAP( BOOL bVal )
                        { bCompileForFAP = bVal; bIgnoreErrors = bVal; }
    void            SetAutoCorrection( BOOL bVal )
                        { bAutoCorrect = bVal; bIgnoreErrors = bVal; }
    void            SetCompileEnglish( BOOL bVal );     // use English SymbolTable
    void            SetCompileXML( BOOL bVal )
                        { bCompileXML = bVal; }
    void            SetImportXML( BOOL bVal )
                        { bImportXML = bVal; }
    BOOL            IsCorrected() { return bCorrected; }
    const String&   GetCorrectedFormula() { return aCorrectedFormula; }
    ScTokenArray* CompileString( const String& rFormula );
    BOOL  CompileTokenArray();
    short GetNumFormatType() { return nNumFmt; }

    ScToken* CreateStringFromToken( String& rFormula, ScToken* pToken,
                                    BOOL bAllowArrAdvance = FALSE );
    void CreateStringFromTokenArray( String& rFormula );

    void MoveRelWrap();
    static void MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc,
                             const ScAddress& rPos );

    BOOL UpdateNameReference( UpdateRefMode eUpdateRefMode,
                              const ScRange&,
                              short nDx, short nDy, short nDz,
                              BOOL& rChanged);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  short nDx, short nDy, short nDz,
                                  BOOL& rChanged);

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  short nDx, short nDy, short nDz );

    ScRangeData* UpdateInsertTab(USHORT nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(USHORT nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);
    ScRangeData* UpdateMoveTab(USHORT nOldPos, USHORT nNewPos, BOOL bIsName );

    BOOL HasModifiedRange();

    /// Is the CharTable initialized? If not call Init() yourself!
    static inline BOOL HasCharTable() { return pCharTable != NULL; }

    /// Access the CharTable flags
    static inline USHORT GetCharTableFlags( sal_Unicode c )
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

#endif
