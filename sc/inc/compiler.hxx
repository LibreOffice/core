/*************************************************************************
 *
 *  $RCSfile: compiler.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 13:21:35 $
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

#include <string.h>     // memcpy(), please don't use memory.h (WKC, was it some MAC weirdness?)

#ifndef _SVMEMPOOL_HXX
#include <tools/mempool.hxx>
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

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#include <hash_map>

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
    void        Delete();

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
    void Load30( SvStream& );
    void Load( SvStream&, USHORT nVer );

    static xub_StrLen GetStrLen( const sal_Unicode* pStr ); // as long as a "string" is an array
    static size_t GetStrLenBytes( xub_StrLen nLen )
        { return nLen * sizeof(sal_Unicode); }
    static size_t GetStrLenBytes( const sal_Unicode* pStr )
        { return GetStrLenBytes( GetStrLen( pStr ) ); }
};


typedef ScSimpleIntrusiveReference< struct ScRawToken > ScRawTokenRef;


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
    static String*  pSymbolTableNative;             // array of native symbols, offset==OpCode
    static String*  pSymbolTableEnglish;            // array of English symbols, offset==OpCode
    static USHORT   nAnzStrings;                    // count of symbols
    static ScOpCodeHashMap* pSymbolHashMapNative;   // hash map of native symbols
    static ScOpCodeHashMap* pSymbolHashMapEnglish;  // hash map of English symbols
private:
    static ULONG*   pCharTable;                     // array of ASCII character flags
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
    ScTokenRef  pToken;                     // current token
    ScTokenRef  pCurrentFactorToken;        // current factor token (of Factor() method)
    OpCode      eLastOp;
    ScToken**   pCode;
    ScArrayStack* pStack;
    String*     pSymbolTable;               // which symbol table is used
    ScOpCodeHashMap*    pSymbolHashMap;     // which symbol hash map is used
    USHORT      pc;
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
    BOOL        bCompileXML;
    BOOL        bImportXML;

    BOOL   GetToken();
    BOOL   NextNewToken();
    OpCode NextToken();
    void PutCode( ScTokenRef& );
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

    String MakeColStr( SCCOL nCol );
    void MakeColStr( rtl::OUStringBuffer& rBuffer, SCCOL nCol );
    String MakeRowStr( SCROW nRow );
    void MakeRowStr( rtl::OUStringBuffer& rBuffer, SCROW nRow );
    String MakeTabStr( SCTAB nTab, String& aDoc );
    String MakeRefStr( ComplRefData& rRefData, BOOL bSingleRef );
    void MakeRefStr( rtl::OUStringBuffer& rBuffer, ComplRefData& rRefData, BOOL bSingleRef );

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
    static void CheckTabQuotes( String& );  // for ScAddress::Format()

    // Put quotes around string if non-alphanumeric characters are contained,
    // quote characters contained within are escaped by '\\'.
    static BOOL EnQuote( String& );
    // Remove quotes, escaped quotes are unescaped.
    static BOOL DeQuote( String& );

    //! _either_ CompileForFAP _or_ AutoCorrection, _not_ both
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
                              BOOL& rChanged);

    ScRangeData* UpdateReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  BOOL& rChanged);

    /// Only once for converted shared formulas,
    /// token array has to be compiled afterwards.
    void UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange&,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    ScRangeData* UpdateInsertTab(SCTAB nTable, BOOL bIsName );
    ScRangeData* UpdateDeleteTab(SCTAB nTable, BOOL bIsMove, BOOL bIsName, BOOL& bCompile);
    ScRangeData* UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, BOOL bIsName );

    BOOL HasModifiedRange();

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

private:
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

extern String* GetScCompilerpSymbolTableNative(); //CHINA001

#endif
