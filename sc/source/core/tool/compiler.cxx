/*************************************************************************
 *
 *  $RCSfile: compiler.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-05 14:53:14 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/rcid.h>
#include <tools/intn.hxx>
#include <tools/rc.hxx>
#include <tools/solar.h>
#include <unotools/charclass.hxx>
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "compiler.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "document.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "cell.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"


String* ScCompiler::pSymbolTableNative = NULL;          // Liste der Symbole
String* ScCompiler::pSymbolTableEnglish = NULL;         // Liste der Symbole English
USHORT  ScCompiler::nAnzStrings = 0;                    // Anzahl der Symbole
USHORT* ScCompiler::pCharTable = 0;

enum ScanState
{
    ssGetChar, ssGetBool, ssGetString,
    ssSkipString, ssStop
};

struct ScArrayStack
{
    ScArrayStack* pNext;
    ScTokenArray* pArr;
    BOOL bTemp;
};

static sal_Char* pInternal[ 5 ] = { "GAME", "SPEW", "TTT", "STARCALCTEAM", "ANTWORT" };


/////////////////////////////////////////////////////////////////////////

short lcl_GetRetFormat( OpCode eOpCode )
{
    switch (eOpCode)
    {
        case ocEqual:
        case ocNotEqual:
        case ocLess:
        case ocGreater:
        case ocLessEqual:
        case ocGreaterEqual:
        case ocAnd:
        case ocOr:
        case ocNot:
        case ocTrue:
        case ocFalse:
        case ocIsEmpty:
        case ocIsString:
        case ocIsNonString:
        case ocIsLogical:
        case ocIsRef:
        case ocIsValue:
        case ocIsFormula:
        case ocIsNV:
        case ocIsErr:
        case ocIsError:
        case ocIsEven:
        case ocIsOdd:
        case ocExact:
            return NUMBERFORMAT_LOGICAL;
        case ocGetActDate:
        case ocGetDate:
        case ocEasterSunday :
            return NUMBERFORMAT_DATE;
        case ocGetActTime:
            return NUMBERFORMAT_DATETIME;
        case ocGetTime:
            return NUMBERFORMAT_TIME;
        case ocNBW:
        case ocBW:
        case ocDIA:
        case ocGDA:
        case ocGDA2:
        case ocVBD:
        case ocLIA:
        case ocRMZ:
        case ocZW:
        case ocZinsZ:
        case ocKapz:
        case ocKumZinsZ:
        case ocKumKapZ:
            return NUMBERFORMAT_CURRENCY;
        case ocZins:
        case ocIKV:
        case ocMIRR:
        case ocZGZ:
        case ocEffektiv:
        case ocNominal:
            return NUMBERFORMAT_PERCENT;
//      case ocSum:
//      case ocSumSQ:
//      case ocProduct:
//      case ocAverage:
//          return -1;
        default:
            return NUMBERFORMAT_NUMBER;
    }
    return NUMBERFORMAT_NUMBER;
}

/////////////////////////////////////////////////////////////////////////

class ScOpCodeList : public Resource        // temp object fuer Resource
{
public:
    ScOpCodeList( USHORT, String[] );
};

ScOpCodeList::ScOpCodeList( USHORT nRID, String pSymbolTable[] )
        :
        Resource( ScResId( nRID ) )
{
    for (USHORT i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; i++)
    {
        ScResId aRes(i);
        aRes.SetRT(RSC_STRING);
        if (IsAvailableRes(aRes))
            pSymbolTable[i] = aRes;
    }
    FreeResource();
}


void ScCompiler::Init()
{
    pSymbolTableNative = new String[SC_OPCODE_LAST_OPCODE_ID+1];
    ScOpCodeList aOpCodeListNative( RID_SC_FUNCTION_NAMES, pSymbolTableNative );
    nAnzStrings = SC_OPCODE_LAST_OPCODE_ID+1;

    pCharTable = new USHORT [128];
    USHORT i;
    for (i = 0; i < 128; i++)
        pCharTable[i] = SC_COMPILER_C_ILLEGAL;
    /*   */     pCharTable[32] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* ! */     pCharTable[33] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* " */     pCharTable[34] = SC_COMPILER_C_CHAR_STRING | SC_COMPILER_C_STRING_SEP;
    /* # */     pCharTable[35] = SC_COMPILER_C_WORD_SEP;
    /* $ */     pCharTable[36] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
    /* % */     pCharTable[37] = SC_COMPILER_C_VALUE;
    /* & */     pCharTable[38] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* ' */     pCharTable[39] = SC_COMPILER_C_NAME_SEP;
    /* ( */     pCharTable[40] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* ) */     pCharTable[41] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* * */     pCharTable[42] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* + */     pCharTable[43] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
    /* , */     pCharTable[44] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE;
    /* - */     pCharTable[45] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
    /* . */     pCharTable[46] = SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE;
    /* / */     pCharTable[47] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    for (i = 48; i < 58; i++)
    /* 0-9 */   pCharTable[i] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_WORD | SC_COMPILER_C_VALUE
                                     | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_VALUE;
    /* : */     pCharTable[58] = SC_COMPILER_C_WORD;
    /* ; */     pCharTable[59] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* < */     pCharTable[60] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* = */     pCharTable[61] = SC_COMPILER_C_CHAR | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* > */     pCharTable[62] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* ? */     pCharTable[63] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
    /* @ */     // FREI
    for (i = 65; i < 91; i++)
    /* A-Z */   pCharTable[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
    /* [ */     // FREI
    /* \ */     // FREI
    /* ] */     // FREI
    /* ^ */     pCharTable[94] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
    /* _ */     pCharTable[95] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
    /* ` */     // FREI
    for (i = 97; i < 123; i++)
    /* a-z */   pCharTable[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD;
    /* { */     // FREI
    /* | */     // FREI
    /* } */     // FREI
    /* ~ */     // FREI
    /* 127 */   // FREI
}

void ScCompiler::DeInit()
{
    if (pSymbolTableNative)
    {
        delete [] pSymbolTableNative;
        pSymbolTableNative = NULL;
    }
    if (pSymbolTableEnglish)
    {
        delete [] pSymbolTableEnglish;
        pSymbolTableEnglish = NULL;
    }
    delete [] pCharTable;
    pCharTable = NULL;
}

void ScCompiler::SetCompileEnglish( BOOL bCompileEnglish )
{
    if ( bCompileEnglish )
    {
        if ( !pSymbolTableEnglish )
        {
            pSymbolTableEnglish = new String[SC_OPCODE_LAST_OPCODE_ID+1];
            ScOpCodeList aOpCodeListEnglish( RID_SC_FUNCTION_NAMES_ENGLISH,
                pSymbolTableEnglish );
        }
        pSymbolTable = pSymbolTableEnglish;
    }
    else
        pSymbolTable = pSymbolTableNative;
}

//-----------------------Funktionen der Klasse ScCompiler----------------------

ScCompiler::ScCompiler( ScDocument* pDocument, const ScAddress& rPos,
                        const ScTokenArray& rArr )
        :
        aPos( rPos ),
        pSymbolTable( pSymbolTableNative ),
        nRecursion(0),
        bAutoCorrect( FALSE ),
        bCorrected( FALSE ),
        bCompileForFAP( FALSE ),
        bIgnoreErrors( FALSE ),
        bCompileXML( FALSE ),
        bImportXML ( FALSE )
{
    if (!nAnzStrings)
        Init();
    pArr = (ScTokenArray*) &rArr;
    pDoc = pDocument;
    nMaxTab = pDoc->GetTableCount() - 1;
    pStack = NULL;
    nNumFmt = NUMBERFORMAT_UNDEFINED;
}

ScCompiler::ScCompiler(ScDocument* pDocument, const ScAddress& rPos )
        :
        aPos( rPos ),
        pSymbolTable( pSymbolTableNative ),
        nRecursion(0),
        bAutoCorrect( FALSE ),
        bCorrected( FALSE ),
        bCompileForFAP( FALSE ),
        bIgnoreErrors( FALSE ),
        bCompileXML( FALSE ),
        bImportXML ( FALSE )
{
    if (!nAnzStrings)
        Init();
    pDoc = pDocument;
    nMaxTab = pDoc->GetTableCount() - 1;
    pStack = NULL;
    nNumFmt = NUMBERFORMAT_UNDEFINED;
}


String ScCompiler::MakeColStr( USHORT nCol )
{
    if ( nCol > MAXCOL )
        return ScGlobal::GetRscString(STR_NO_REF_TABLE);
    else
    {
        if (nCol < 26)
            return String( 'A' + (sal_uChar) nCol ) ;
        else
        {
            String aString;
            sal_Unicode* pCol = aString.AllocBuffer( 2 );
            USHORT nLoCol = nCol % 26;
            USHORT nHiCol = (nCol / 26) - 1;
            pCol[0] = 'A' + (sal_uChar)nHiCol;
            pCol[1] = 'A' + (sal_uChar)nLoCol;
            // terminating null character is set in AllocBuffer
            return aString;
        }
    }
}

String ScCompiler::MakeRowStr( USHORT nRow )
{
    if ( nRow > MAXROW )
        return ScGlobal::GetRscString(STR_NO_REF_TABLE);
    else
        return String::CreateFromInt32( nRow + 1 );
}

String ScCompiler::MakeTabStr( USHORT nTab, String& aDoc )
{
    String aString;
    if (!pDoc->GetName(nTab, aString))
        aString = ScGlobal::GetRscString(STR_NO_REF_TABLE);
    else
    {
        if ( aString.GetChar(0) == '\'' )
        {   // "'Doc'#Tab"
            xub_StrLen nPos, nLen = 1;
            while( (nPos = aString.Search( '\'', nLen )) != STRING_NOTFOUND )
                nLen = nPos + 1;
            if ( aString.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
            {
                aDoc = aString.Copy( 0, nLen + 1 );
                aString.Erase( 0, nLen + 1 );
                aDoc = INetURLObject::decode( aDoc, INET_HEX_ESCAPE,
                    INetURLObject::DECODE_UNAMBIGUOUS );
            }
            else
                aDoc.Erase();
        }
        else
            aDoc.Erase();
        CheckTabQuotes( aString );
    }
    aString += '.';
    return aString;
}

void ScCompiler::CheckTabQuotes( String& rString )
{
    register const sal_Unicode* p = rString.GetBuffer();
    register const sal_Unicode* const pEnd = p + rString.Len();
    while ( p < pEnd )
    {
        if( !IsWordChar( *p ) )
        {
            rString.Insert( '\'', 0 );
            rString += '\'';
            break;
        }
        p++;
    }
}

String ScCompiler::MakeRefStr( ComplRefData& rRef, BOOL bSingleRef )
{
    String aNewRef;
    if (bCompileXML)
        aNewRef = '[';
    ComplRefData aRef( rRef );
    // falls abs/rel nicht separat: Relativ- in Abs-Referenzen wandeln!
//  AdjustReference( aRef.Ref1 );
//  if( !bSingleRef )
//      AdjustReference( aRef.Ref2 );
    aRef.Ref1.CalcAbsIfRel( aPos );
    if( !bSingleRef )
        aRef.Ref2.CalcAbsIfRel( aPos );
    if( aRef.Ref1.IsFlag3D() )
    {
        if (aRef.Ref1.IsTabDeleted())
        {
            if (!aRef.Ref1.IsTabRel())
                aNewRef += '$';
            aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
            aNewRef += '.';
        }
        else
        {
            String aDoc;
            String aRefStr( MakeTabStr( aRef.Ref1.nTab, aDoc ) );
            aNewRef += aDoc;
            if (!aRef.Ref1.IsTabRel()) aNewRef += '$';
            aNewRef += aRefStr;
        }
    }
    else if (bCompileXML)
        aNewRef += '.';
    if (!aRef.Ref1.IsColRel())
        aNewRef += '$';
    if ( aRef.Ref1.IsColDeleted() )
        aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
    else
        aNewRef += MakeColStr( aRef.Ref1.nCol );
    if (!aRef.Ref1.IsRowRel())
        aNewRef += '$';
    if ( aRef.Ref1.IsRowDeleted() )
        aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
    else
        aNewRef += MakeRowStr( aRef.Ref1.nRow );
    if (!bSingleRef)
    {
        aNewRef += ':';
        if (aRef.Ref2.IsFlag3D() || aRef.Ref2.nTab != aRef.Ref1.nTab)
        {
            if (aRef.Ref2.IsTabDeleted())
            {
                if (!aRef.Ref2.IsTabRel())
                    aNewRef += '$';
                aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
                aNewRef += '.';
            }
            else
            {
                String aDoc;
                String aRefStr( MakeTabStr( aRef.Ref2.nTab, aDoc ) );
                aNewRef += aDoc;
                if (!aRef.Ref2.IsTabRel())
                    aNewRef += '$';
                aNewRef += aRefStr;
            }
        }
        else if (bCompileXML)
            aNewRef += '.';
        if (!aRef.Ref2.IsColRel())
            aNewRef += '$';
        if ( aRef.Ref2.IsColDeleted() )
            aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
        else
            aNewRef += MakeColStr( aRef.Ref2.nCol );
        if (!aRef.Ref2.IsRowRel())
            aNewRef += '$';
        if ( aRef.Ref2.IsRowDeleted() )
            aNewRef += ScGlobal::GetRscString(STR_NO_REF_TABLE);
        else
            aNewRef += MakeRowStr( aRef.Ref2.nRow );
    }
    if (bCompileXML)
        aNewRef += ']';
    return aNewRef;
}

//---------------------------------------------------------------------------

void ScCompiler::SetError(USHORT nError)
{
    if( !pArr->GetError() )
        pArr->nError = nError;
}


sal_Unicode* lcl_UnicodeStrNCpy( sal_Unicode* pDst, const sal_Unicode* pSrc, xub_StrLen nMax )
{
    const sal_Unicode* const pStop = pDst + nMax;
    while ( *pSrc && pDst < pStop )
    {
        *pDst++ = *pSrc++;
    }
    *pDst = 0;
    return pDst;
}


//---------------------------------------------------------------------------
// NextSymbol
//---------------------------------------------------------------------------
// Zerlegt die Formel in einzelne Symbole fuer die weitere
// Verarbeitung (Turing-Maschine).
//---------------------------------------------------------------------------
// Ausgangs Zustand = GetChar
//---------------+-------------------+-----------------------+---------------
// Alter Zustand | gelesenes Zeichen | Aktion                | Neuer Zustand
//---------------+-------------------+-----------------------+---------------
// GetChar       | ;()+-*/^=&        | Symbol=Zeichen        | Stop
//               | <>                | Symbol=Zeichen        | GetBool
//               | $ Buchstabe       | Symbol=Zeichen        | GetWord
//               | Ziffer            | Symbol=Zeichen        | GetValue
//               | "                 | Keine                 | GetString
//               | Sonst             | Keine                 | GetChar
//---------------+-------------------+-----------------------+---------------
// GetBool       | =>                | Symbol=Symbol+Zeichen | Stop
//               | Sonst             | Dec(CharPos)          | Stop
//---------------+-------------------+-----------------------+---------------
// GetWord       | SepSymbol         | Dec(CharPos)          | Stop
//               | ()+-*/^=<>&~      |                       |
//               | Leerzeichen       | Dec(CharPos)          | Stop
//               | $_:.              |                       |
//               | Buchstabe,Ziffer  | Symbol=Symbol+Zeichen | GetWord
//               | Sonst             | Fehler                | Stop
//---------------|-------------------+-----------------------+---------------
// GetValue      | ;()*/^=<>&        |                       |
//               | Leerzeichen       | Dec(CharPos)          | Stop
//               | Ziffer E+-%,.     | Symbol=Symbol+Zeichen | GetValue
//               | Sonst             | Fehler                | Stop
//---------------+-------------------+-----------------------+---------------
// GetString     | "                 | Keine                 | Stop
//               | Sonst             | Symbol=Symbol+Zeichen | GetString
//---------------+-------------------+-----------------------+---------------

xub_StrLen ScCompiler::NextSymbol()
{
    cSymbol[MAXSTRLEN-1] = 0;       // Stopper
    sal_Unicode* pSym = cSymbol;
    const sal_Unicode* const pStart = aFormula.GetBuffer();
    const sal_Unicode* pSrc = pStart + nSrcPos;
    BOOL bi18n = FALSE;
    sal_Unicode c = *pSrc;
    sal_Unicode cLast = 0;
    BOOL bQuote = FALSE;
    ScanState eState = ssGetChar;
    xub_StrLen nSpaces = 0;
    // try to parse simple tokens before calling i18n parser
    while ((c != 0) && (eState != ssStop) )
    {
        pSrc++;
        USHORT nMask = GetCharTableFlags( c );
        switch (eState)
        {
            case ssGetChar :
            {
                if( nMask & SC_COMPILER_C_CHAR )
                {
                    *pSym++ = c;
                    eState = ssStop;
                }
                else if( nMask & SC_COMPILER_C_CHAR_BOOL )
                {
                    *pSym++ = c;
                    eState = ssGetBool;
                }
                else if( nMask & SC_COMPILER_C_CHAR_STRING )
                {
                    *pSym++ = c;
                    eState = ssGetString;
                }
                else if( nMask & SC_COMPILER_C_CHAR_DONTCARE )
                {
                    nSpaces++;
                }
                else
                {
                    bi18n = TRUE;
                    eState = ssStop;
                }
            }
            break;
            case ssGetBool :
            {
                if( nMask & SC_COMPILER_C_BOOL )
                {
                    *pSym++ = c;
                    eState = ssStop;
                }
                else
                {
                    pSrc--;
                    eState = ssStop;
                }
            }
            break;
            case ssGetString :
            {
                if( nMask & SC_COMPILER_C_STRING_SEP )
                {
                    if ( !bQuote )
                    {
                        if ( *pSrc == '"' )
                            bQuote = TRUE;      // "" => literal "
                        else
                            eState = ssStop;
                    }
                    else
                        bQuote = FALSE;
                }
                if ( !bQuote )
                {
                    if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                    {
                        SetError(errStringOverflow);
                        eState = ssSkipString;
                    }
                    else
                        *pSym++ = c;
                }
            }
            break;
            case ssSkipString:
                if( nMask & SC_COMPILER_C_STRING_SEP )
                    eState = ssStop;
                break;
        }
        cLast = c;
        c = *pSrc;
    }
    if ( bi18n )
    {
        using namespace ::com::sun::star::i18n;
        nSrcPos += nSpaces;
        sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
        sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT |
            KParseTokens::ASC_COLON;
        // '?' allowed in range names because of Xcl :-/
        String aAddAllowed( '?' );
        String aSymbol;
        USHORT nErr = 0;
        do
        {
            bi18n = FALSE;
            // special case  $'sheetname'
            if ( pStart[nSrcPos] == '$' && pStart[nSrcPos+1] == '\'' )
                aSymbol += pStart[nSrcPos++];

            ParseResult aRes = ScGlobal::pCharClass->parseAnyToken( aFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );

            if ( !aRes.TokenType )
                SetError( nErr = errIllegalChar );      // parsed chars as string
            if ( aRes.EndPos <= nSrcPos )
            {   // ?!?
                SetError( nErr = errIllegalChar );
                nSrcPos = aFormula.Len();
                aSymbol.Erase();
            }
            else
            {
                aSymbol.Append( pStart + nSrcPos, aRes.EndPos - nSrcPos );
                nSrcPos = (xub_StrLen) aRes.EndPos;
                if ( aRes.TokenType & KParseType::SINGLE_QUOTE_NAME )
                {   // special cases  'sheetname'.  'filename'#
                    c = pStart[nSrcPos];
                    bi18n = (c == '.' || c == SC_COMPILER_FILE_TAB_SEP);
                    if ( bi18n )
                        aSymbol += pStart[nSrcPos++];
                }
                else if ( aRes.TokenType & KParseType::IDENTNAME )
                {   // special cases  reference:[$]'sheetname'
                    c = aSymbol.GetChar( aSymbol.Len()-1 );
                    bi18n = ((c == ':' || c == '$') && pStart[nSrcPos] == '\'');
                }
            }
        } while ( bi18n && !nErr );
        xub_StrLen nLen = aSymbol.Len();
        if ( nLen >= MAXSTRLEN )
        {
            SetError( errStringOverflow );
            nLen = MAXSTRLEN-1;
        }
        lcl_UnicodeStrNCpy( cSymbol, aSymbol.GetBuffer(), nLen );
    }
    else
    {
        nSrcPos = pSrc - pStart;
        *pSym = 0;
    }
    if ( bAutoCorrect )
        aCorrectedSymbol = cSymbol;
    return nSpaces;
}

//---------------------------------------------------------------------------
// Symbol in Token Umwandeln
//---------------------------------------------------------------------------

BOOL ScCompiler::IsOpCode( const String& rName )
{
    BOOL bFound = FALSE;

    for( USHORT i = 0; i < nAnzStrings && !bFound; i++ )
        bFound = (pSymbolTable[i] == rName);

    if (bFound)
    {
        ScRawToken aToken;
        aToken.SetOpCode( (OpCode) --i );
        pRawToken = aToken.Clone();
    }
    else
    {
        USHORT nIndex;
        bFound = ScGlobal::GetFuncCollection()->SearchFunc(cSymbol, nIndex);
        if( bFound )
        {
            ScRawToken aToken;
            aToken.SetExternal( cSymbol );
            pRawToken = aToken.Clone();
        }
        else
        {
            // bLocalFirst=FALSE for english
            String aIntName = ScGlobal::GetAddInCollection()->
                    FindFunction( cSymbol, ( pSymbolTable != pSymbolTableEnglish ) );
            if (aIntName.Len())
            {
                ScRawToken aToken;
                aToken.SetExternal( aIntName.GetBuffer() );     // international name
                pRawToken = aToken.Clone();
                bFound = TRUE;
            }
        }
    }
    if ( bFound && pRawToken->GetOpCode() == ocSub &&
        (eLastOp == ocOpen || eLastOp == ocSep ||
         (eLastOp > ocEndDiv && eLastOp < ocEndBinOp /*ocEndUnOp*/)))
        pRawToken->NewOpCode( ocNegSub );
    return bFound;
}

BOOL ScCompiler::IsOpCode2( const String& rName )
{
    BOOL bFound = FALSE;

    for( USHORT i = ocInternalBegin; i <= ocInternalEnd && !bFound; i++ )
        bFound = rName.EqualsAscii( pInternal[ i-ocInternalBegin ] );

    if (bFound)
    {
        ScRawToken aToken;
        aToken.SetOpCode( (OpCode) --i );
        pRawToken = aToken.Clone();
    }
    return bFound;
}

BOOL ScCompiler::IsValue( const String& rSym )
{
    double fVal;
    ULONG nIndex = ( pSymbolTable == pSymbolTableEnglish ?
        pDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US ) : 0 );
//  ULONG nIndex = 0;
////    ULONG nIndex = pDoc->GetFormatTable()->GetStandardIndex(ScGlobal::eLnge);
    if (pDoc->GetFormatTable()->IsNumberFormat( rSym, nIndex, fVal ) )
    {
        USHORT nType = pDoc->GetFormatTable()->GetType(nIndex);
        const sal_Unicode* p = aFormula.GetBuffer() + nSrcPos;
        while( *p == ' ' )
            p++;
        if ( *p == '(' && nType == NUMBERFORMAT_LOGICAL)
            return FALSE;
        else if( aFormula.GetChar(nSrcPos) == '.' )
            // Numerischer Tabellenname?
            return FALSE;
        else
        {
            if( nType == NUMBERFORMAT_TEXT )
                // HACK: Die Zahl ist zu gross!
                SetError( errIllegalArgument );
            ScRawToken aToken;
            aToken.SetDouble( fVal );
            pRawToken = aToken.Clone();
            return TRUE;
        }
    }
    else
        return FALSE;
}

BOOL ScCompiler::IsString()
{
    register const sal_Unicode* p = cSymbol;
    while ( *p )
        p++;
    xub_StrLen nLen = p - cSymbol - 1;
    BOOL bQuote = ((cSymbol[0] == '"') && (cSymbol[nLen] == '"'));
    if ((bQuote ? nLen-2 : nLen) > MAXSTRLEN-1)
    {
        SetError(errStringOverflow);
        return FALSE;
    }
    if ( bQuote )
    {
        cSymbol[nLen] = '\0';
        ScRawToken aToken;
        aToken.SetString( cSymbol+1 );
        pRawToken = aToken.Clone();
        return TRUE;
    }
    return FALSE;
}

BOOL ScCompiler::IsReference( const String& rName )
{
    // wird jetzt vor IsValue aufgerufen, kein #REF! aus Zahlen machen
    // numerischer Tabellenname muss allerdings durchgehen
    // englisches 1.E2 oder 1.E+2 ist wiederum Zahl 100, 1.E-2 ist 0,01
    sal_Unicode ch1 = rName.GetChar(0);
    sal_Unicode cDecSep = ( pSymbolTable == pSymbolTableEnglish ? '.' :
        ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0) );
    if ( ch1 == cDecSep )
        return FALSE;
    BOOL bMyAlpha;
    sal_Unicode ch2;
    xub_StrLen nPos = rName.Search( '.' );
    const sal_Unicode* pTabSep = (nPos == STRING_NOTFOUND ? NULL :
        rName.GetBuffer() + nPos);
    if ( pTabSep )
    {
        ch2 = pTabSep[1];       // vielleicht Col-Bezeichner
        bMyAlpha = CharClass::isAsciiAlpha( rName.GetChar( nPos+1 ) ) || ch2 == '$';
    }
    else
    {
        ch2 = 0;
        bMyAlpha = FALSE;
    }
    // welcher Hirni hat bloss den . zum Tabellenseparator gemacht!?!
    BOOL bDigit1 = CharClass::isAsciiNumeric( ch1 );
    if ( cDecSep == '.' )
    {
        if ( ( bDigit1 && ( pTabSep ?
                (!bMyAlpha || ((ch2 == 'E' || ch2 == 'e')   // E + - digit
                    && (GetCharTableFlags( pTabSep[2] ) & SC_COMPILER_C_VALUE_EXP))) :
                TRUE ) ) )
            return FALSE;
    }
    else
    {
        if ( bDigit1 && (pTabSep ? !bMyAlpha : TRUE) )
            return FALSE;
    }
    ScRange aRange( aPos, aPos );
    USHORT nFlags = aRange.Parse( rName, pDoc );
    if( nFlags & SCA_VALID )
    {
        ScRawToken aToken;
        ComplRefData aRef;
        aRef.InitRange( aRange );
        aRef.Ref1.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
        aRef.Ref1.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
        aRef.Ref1.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB) )
            aRef.Ref1.SetTabDeleted( TRUE );        // #REF!
        aRef.Ref1.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
        aRef.Ref2.SetColRel( (nFlags & SCA_COL2_ABSOLUTE) == 0 );
        aRef.Ref2.SetRowRel( (nFlags & SCA_ROW2_ABSOLUTE) == 0 );
        aRef.Ref2.SetTabRel( (nFlags & SCA_TAB2_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB2) )
            aRef.Ref2.SetTabDeleted( TRUE );        // #REF!
        aRef.Ref2.SetFlag3D( ( nFlags & SCA_TAB2_3D ) != 0 );
        aRef.CalcRelFromAbs( aPos );
        aToken.SetDoubleReference( aRef );
        pRawToken = aToken.Clone();
    }
    else
    {
        ScAddress aAddr( aPos );
        nFlags = aAddr.Parse( rName, pDoc );
        // Irgend etwas muss gueltig sein,
        // damit Tabelle1.blah oder blah.a1 als (falsche) ref erkannt wird
        if( nFlags & ( SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB ) )
        {
            ScRawToken aToken;
            SingleRefData aRef;
            aRef.InitAddress( aAddr );
            aRef.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
            aRef.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
            aRef.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
            aRef.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
            // Die Referenz ist wirklich ungueltig!
            if( !( nFlags & SCA_VALID ) )
            {
                if( !( nFlags & SCA_VALID_COL ) )
                    aRef.nCol = MAXCOL+1;
                if( !( nFlags & SCA_VALID_ROW ) )
                    aRef.nRow = MAXROW+1;
                if( !( nFlags & SCA_VALID_TAB ) )
                    aRef.nTab = MAXTAB+3;
                nFlags |= SCA_VALID;
            }
            aRef.CalcRelFromAbs( aPos );
            aToken.SetSingleReference( aRef );
            pRawToken = aToken.Clone();
        }
    }
    return ( nFlags & SCA_VALID ) != 0;
}

BOOL ScCompiler::IsMacro( const String& rName )
{
    StarBASIC* pObj = 0;
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();

    SfxApplication* pSfxApp = SFX_APP();
    pSfxApp->EnterBasicCall();              // Dok-Basic anlegen etc.

    if( pDocSh )//XXX
        pObj = pDocSh->GetBasic();
    else
        pObj = pSfxApp->GetBasic();

    SbxMethod* pMeth = (SbxMethod*) pObj->Find( rName, SbxCLASS_METHOD );
    if( !pMeth )
    {
        pSfxApp->LeaveBasicCall();
        return FALSE;
    }
    // Es sollte schon eine BASIC-Function sein!
    if( pMeth->GetType() == SbxVOID
     || ( pMeth->IsFixed() && pMeth->GetType() == SbxEMPTY )
     || !pMeth->ISA(SbMethod) )
    {
        pSfxApp->LeaveBasicCall();
        return FALSE;
    }
    ScRawToken aToken;
    aToken.SetExternal( rName.GetBuffer() );
    aToken.eOp = ocMacro;
    pRawToken = aToken.Clone();
    pSfxApp->LeaveBasicCall();
    return TRUE;
}

BOOL ScCompiler::IsNamedRange( const String& rName )
{
    USHORT n;
    ScRangeName* pRangeName = pDoc->GetRangeName();
    if (pRangeName->SearchName( rName, n ) )
    {
        ScRangeData* pData = (*pRangeName)[n];
        ScRawToken aToken;
        aToken.SetName( pData->GetIndex() );
        pRawToken = aToken.Clone();
        return TRUE;
    }
    else
        return FALSE;
}

BOOL ScCompiler::IsDBRange( const String& rName )
{
    USHORT n;
    ScDBCollection* pDBColl = pDoc->GetDBCollection();
    if (pDBColl->SearchName( rName, n ) )
    {
        ScDBData* pData = (*pDBColl)[n];
        ScRawToken aToken;
        aToken.SetName( pData->GetIndex() );
        aToken.eOp = ocDBArea;
        pRawToken = aToken.Clone();
        return TRUE;
    }
    else
        return FALSE;
}

BOOL ScCompiler::IsColRowName( const String& rName )
{
    BOOL bInList = FALSE;
    BOOL bFound = FALSE;
    SingleRefData aRef;
    String aName( rName );
    DeQuote( aName );
    USHORT nThisTab = aPos.Tab();
    for ( short jThisTab = 1; jThisTab >= 0 && !bInList; jThisTab-- )
    {   // #50300# zuerst Bereiche auf dieser Tabelle pruefen, falls doppelte Namen
        for ( short jRow=0; jRow<2 && !bInList; jRow++ )
        {
            ScRangePairList* pRL;
            if ( !jRow )
                pRL = pDoc->GetColNameRanges();
            else
                pRL = pDoc->GetRowNameRanges();
            for ( ScRangePair* pR = pRL->First(); pR && !bInList; pR = pRL->Next() )
            {
                const ScRange& rNameRange = pR->GetRange(0);
                if ( jThisTab && !(rNameRange.aStart.Tab() <= nThisTab &&
                        nThisTab <= rNameRange.aEnd.Tab()) )
                    continue;   // for
                ScCellIterator aIter( pDoc, rNameRange );
                for ( ScBaseCell* pCell = aIter.GetFirst(); pCell && !bInList;
                        pCell = aIter.GetNext() )
                {
                    // GPF wenn Zelle via CompileNameFormula auf Zelle ohne Code
                    // trifft und HasStringData/Interpret/Compile ausgefuehrt wird
                    // und das ganze dann auch noch rekursiv..
                    // ausserdem wird *diese* Zelle hier nicht angefasst, da noch
                    // kein RPN existiert
                    CellType eType = pCell->GetCellType();
                    BOOL bOk = (eType == CELLTYPE_FORMULA ?
                        ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen() > 0
                        && ((ScFormulaCell*)pCell)->aPos != aPos    // noIter
                        : TRUE );
                    if ( bOk && pCell->HasStringData() )
                    {
                        String aStr;
                        switch ( eType )
                        {
                            case CELLTYPE_STRING:
                                ((ScStringCell*)pCell)->GetString( aStr );
                            break;
                            case CELLTYPE_FORMULA:
                                ((ScFormulaCell*)pCell)->GetString( aStr );
                            break;
                            case CELLTYPE_EDIT:
                                ((ScEditCell*)pCell)->GetString( aStr );
                            break;
                        }
                        if ( ScGlobal::pScInternational->CompareEqual(
                                aStr, aName, INTN_COMPARE_IGNORECASE ) )
                        {
                            aRef.InitFlags();
                            aRef.nCol = aIter.GetCol();
                            aRef.nRow = aIter.GetRow();
                            aRef.nTab = aIter.GetTab();
                            if ( !jRow )
                                aRef.SetColRel( TRUE );     // ColName
                            else
                                aRef.SetRowRel( TRUE );     // RowName
                            aRef.CalcRelFromAbs( aPos );
                            bInList = bFound = TRUE;
                        }
                    }
                }
            }
        }
    }
    if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
    {   // in der aktuellen Tabelle suchen
        long nDistance, nMax;
        long nMyCol = (long) aPos.Col();
        long nMyRow = (long) aPos.Row();
        BOOL bTwo = FALSE;
        ScAddress aOne( 0, 0, aPos.Tab() );
        ScAddress aTwo( MAXCOL, MAXROW, aPos.Tab() );
        ScCellIterator aIter( pDoc, ScRange( aOne, aTwo ) );
        for ( ScBaseCell* pCell = aIter.GetFirst(); pCell; pCell = aIter.GetNext() )
        {
            if ( bFound )
            {   // aufhoeren wenn alles andere weiter liegt
                if ( nMax < (long)aIter.GetCol() )
                    break;      // aIter
            }
            CellType eType = pCell->GetCellType();
            BOOL bOk = (eType == CELLTYPE_FORMULA ?
                ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen() > 0
                && ((ScFormulaCell*)pCell)->aPos != aPos    // noIter
                : TRUE );
            if ( bOk && pCell->HasStringData() )
            {
                String aStr;
                switch ( eType )
                {
                    case CELLTYPE_STRING:
                        ((ScStringCell*)pCell)->GetString( aStr );
                    break;
                    case CELLTYPE_FORMULA:
                        ((ScFormulaCell*)pCell)->GetString( aStr );
                    break;
                    case CELLTYPE_EDIT:
                        ((ScEditCell*)pCell)->GetString( aStr );
                    break;
                }
                if ( ScGlobal::pScInternational->CompareEqual(
                        aStr, aName, INTN_COMPARE_IGNORECASE ) )
                {
                    USHORT nCol = aIter.GetCol();
                    USHORT nRow = aIter.GetRow();
                    long nC = nMyCol - nCol;
                    long nR = nMyRow - nRow;
                    if ( bFound )
                    {
                        long nD = nC * nC + nR * nR;
                        if ( nD < nDistance )
                        {
                            if ( nC < 0 || nR < 0 )
                            {   // rechts oder unterhalb
                                bTwo = TRUE;
                                aTwo.Set( nCol, nRow, aIter.GetTab() );
                                nMax = Max( nMyCol + Abs( nC ), nMyRow + Abs( nR ) );
                                nDistance = nD;
                            }
                            else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
                            {   // links oben, nur wenn nicht weiter oberhalb
                                // des bisherigen und nMyRow darunter
                                // (CellIter geht spaltenweise!)
                                bTwo = FALSE;
                                aOne.Set( nCol, nRow, aIter.GetTab() );
                                nMax = Max( nMyCol + nC, nMyRow + nR );
                                nDistance = nD;
                            }
                        }
                    }
                    else
                    {
                        aOne.Set( nCol, nRow, aIter.GetTab() );
                        nDistance = nC * nC + nR * nR;
                        nMax = Max( nMyCol + Abs( nC ), nMyRow + Abs( nR ) );
                    }
                    bFound = TRUE;
                }
            }
        }
        if ( bFound )
        {
            ScAddress aAdr;
            if ( bTwo )
            {
                if ( nMyCol >= (long)aOne.Col() && nMyRow >= (long)aOne.Row() )
                    aAdr = aOne;        // links oben hat Vorrang
                else
                {
                    if ( nMyCol < (long)aOne.Col() )
                    {   // zwei rechts
                        if ( nMyRow >= (long)aTwo.Row() )
                            aAdr = aTwo;        // direkt rechts
                        else
                            aAdr = aOne;
                    }
                    else
                    {   // zwei unten oder unten und rechts, der naechstgelegene
                        long nC1 = nMyCol - aOne.Col();
                        long nR1 = nMyRow - aOne.Row();
                        long nC2 = nMyCol - aTwo.Col();
                        long nR2 = nMyRow - aTwo.Row();
                        if ( nC1 * nC1 + nR1 * nR1 <= nC2 * nC2 + nR2 * nR2 )
                            aAdr = aOne;
                        else
                            aAdr = aTwo;
                    }
                }
            }
            else
                aAdr = aOne;
            aRef.InitAddress( aAdr );
            if ( (aRef.nRow != MAXROW && pDoc->HasStringData(
                    aRef.nCol, aRef.nRow + 1, aRef.nTab ))
              || (aRef.nRow != 0 && pDoc->HasStringData(
                    aRef.nCol, aRef.nRow - 1, aRef.nTab )) )
                aRef.SetRowRel( TRUE );     // RowName
            else
                aRef.SetColRel( TRUE );     // ColName
            aRef.CalcRelFromAbs( aPos );
        }
    }
    if ( bFound )
    {
        ScRawToken aToken;
        aToken.SetSingleReference( aRef );
        aToken.eOp = ocColRowName;
        pRawToken = aToken.Clone();
        return TRUE;
    }
    else
        return FALSE;
}

//---------------------------------------------------------------------------

void ScCompiler::AutoCorrectParsedSymbol()
{
    xub_StrLen nPos = aCorrectedSymbol.Len();
    if ( nPos )
    {
        nPos--;
        const sal_Unicode cQuote = '\"';
        const sal_Unicode cx = 'x';
        const sal_Unicode cX = 'X';
        sal_Unicode c1 = aCorrectedSymbol.GetChar( 0 );
        sal_Unicode c2 = aCorrectedSymbol.GetChar( nPos );
        if ( c1 == cQuote && c2 != cQuote  )
        {   // "...
            // was kein Wort bildet gehoert nicht dazu.
            // Don't be pedantic: c < 128 should be sufficient here.
            while ( nPos && ((aCorrectedSymbol.GetChar(nPos) < 128) &&
                    ((GetCharTableFlags( aCorrectedSymbol.GetChar(nPos) ) &
                    (SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_DONTCARE)) == 0)) )
                nPos--;
            if ( nPos == MAXSTRLEN - 2 )
                aCorrectedSymbol.SetChar( nPos, cQuote );   // '"' als 255. Zeichen
            else
                aCorrectedSymbol.Insert( cQuote, nPos + 1 );
            bCorrected = TRUE;
        }
        else if ( c1 != cQuote && c2 == cQuote )
        {   // ..."
            aCorrectedSymbol.Insert( cQuote, 0 );
            bCorrected = TRUE;
        }
        else if ( nPos == 0 && (c1 == cx || c1 == cX) )
        {   // x => *
            aCorrectedSymbol = pSymbolTable[ocMul];
            bCorrected = TRUE;
        }
        else if ( (GetCharTableFlags( c1 ) & SC_COMPILER_C_CHAR_VALUE)
               && (GetCharTableFlags( c2 ) & SC_COMPILER_C_CHAR_VALUE) )
        {
            xub_StrLen nXcount;
            if ( (nXcount = aCorrectedSymbol.GetTokenCount( cx )) > 1 )
            {   // x => *
                xub_StrLen nIndex = 0;
                sal_Unicode c = pSymbolTable[ocMul].GetChar(0);
                while ( (nIndex = aCorrectedSymbol.SearchAndReplace(
                        cx, c, nIndex )) != STRING_NOTFOUND )
                    nIndex++;
                bCorrected = TRUE;
            }
            if ( (nXcount = aCorrectedSymbol.GetTokenCount( cX )) > 1 )
            {   // X => *
                xub_StrLen nIndex = 0;
                sal_Unicode c = pSymbolTable[ocMul].GetChar(0);
                while ( (nIndex = aCorrectedSymbol.SearchAndReplace(
                        cX, c, nIndex )) != STRING_NOTFOUND )
                    nIndex++;
                bCorrected = TRUE;
            }
        }
        else
        {
            String aSymbol( aCorrectedSymbol );
            String aDoc;
            xub_StrLen nPos;
            if ( aSymbol.GetChar(0) == '\''
              && ((nPos = aSymbol.SearchAscii( "'#" )) != STRING_NOTFOUND) )
            {   // 'Doc'# abspalten, kann d:\... und sonstwas sein
                aDoc = aSymbol.Copy( 0, nPos + 2 );
                aSymbol.Erase( 0, nPos + 2 );
            }
            xub_StrLen nRefs = aSymbol.GetTokenCount( ':' );
            BOOL bColons;
            if ( nRefs > 2 )
            {   // doppelte oder zuviele ':'? B:2::C10 => B2:C10
                bColons = TRUE;
                xub_StrLen nIndex = 0;
                String aTmp1( aSymbol.GetToken( 0, ':', nIndex ) );
                xub_StrLen nLen1 = aTmp1.Len();
                String aSym, aTmp2;
                BOOL bLastAlp, bNextNum;
                bLastAlp = bNextNum = TRUE;
                xub_StrLen nStrip = 0;
                xub_StrLen nCount = nRefs;
                for ( xub_StrLen j=1; j<nCount; j++ )
                {
                    aTmp2 = aSymbol.GetToken( 0, ':', nIndex );
                    xub_StrLen nLen2 = aTmp2.Len();
                    if ( nLen1 || nLen2 )
                    {
                        if ( nLen1 )
                        {
                            aSym += aTmp1;
                            bLastAlp = CharClass::isAsciiAlpha( aTmp1 );
                        }
                        if ( nLen2 )
                        {
                            bNextNum = CharClass::isAsciiNumeric( aTmp2 );
                            if ( bLastAlp == bNextNum && nStrip < 1 )
                            {   // muss abwechselnd nur Zahl/String sein,
                                // nur innerhalb einer Ref strippen
                                nRefs--;
                                nStrip++;
                            }
                            else
                            {
                                xub_StrLen nSymLen = aSym.Len();
                                if ( nSymLen
                                  && (aSym.GetChar( nSymLen - 1 ) != ':') )
                                    aSym += ':';
                                nStrip = 0;
                            }
                            bLastAlp = !bNextNum;
                        }
                        else
                        {   // ::
                            nRefs--;
                            if ( nLen1 )
                            {   // B10::C10 ? naechste Runde ':' anhaengen
                                if ( !bLastAlp && !CharClass::isAsciiNumeric( aTmp1 ) )
                                    nStrip++;
                            }
                            bNextNum = !bLastAlp;
                        }
                        aTmp1 = aTmp2;
                        nLen1 = nLen2;
                    }
                    else
                        nRefs--;
                }
                aSymbol = aSym;
                aSymbol += aTmp1;
            }
            else
                bColons = FALSE;
            if ( nRefs && nRefs <= 2 )
            {   // Referenzdreher? 4A => A4 etc.
                String aTab[2], aRef[2];
                if ( nRefs == 2 )
                {
                    aRef[0] = aSymbol.GetToken( 0, ':' );
                    aRef[1] = aSymbol.GetToken( 1, ':' );
                }
                else
                    aRef[0] = aSymbol;

                BOOL bChanged = FALSE;
                BOOL bOk = TRUE;
                USHORT nMask = SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW;
                for ( int j=0; j<nRefs; j++ )
                {
                    xub_StrLen nTmp = 0;
                    xub_StrLen nPos = STRING_NOTFOUND;
                    while ( (nTmp = aRef[j].Search( '.', nTmp )) != STRING_NOTFOUND )
                        nPos = nTmp++;      // der letzte zaehlt
                    if ( nPos != STRING_NOTFOUND )
                    {
                        aTab[j] = aRef[j].Copy( 0, nPos + 1 );  // mit '.'
                        aRef[j].Erase( 0, nPos + 1 );
                    }
                    String aOld( aRef[j] );
                    String aStr2;
                    const sal_Unicode* p = aRef[j].GetBuffer();
                    while ( *p && CharClass::isAsciiNumeric( *p ) )
                        aStr2 += *p++;
                    aRef[j] = String( p );
                    aRef[j] += aStr2;
                    if ( bColons || aRef[j] != aOld )
                    {
                        bChanged = TRUE;
                        ScAddress aAdr;
                        bOk &= ((aAdr.Parse( aRef[j], pDoc ) & nMask) == nMask);
                    }
                }
                if ( bChanged && bOk )
                {
                    aCorrectedSymbol = aDoc;
                    aCorrectedSymbol += aTab[0];
                    aCorrectedSymbol += aRef[0];
                    if ( nRefs == 2 )
                    {
                        aCorrectedSymbol += ':';
                        aCorrectedSymbol += aTab[1];
                        aCorrectedSymbol += aRef[1];
                    }
                    bCorrected = TRUE;
                }
            }
        }
    }
}

BOOL ScCompiler::NextNewToken()
{
    xub_StrLen nSpaces = NextSymbol();
    ScRawToken aToken;
    if( cSymbol[0] )
    {
        if( nSpaces )
        {
            aToken.SetOpCode( ocSpaces );
            aToken.cByte = (BYTE) ( nSpaces > 255 ? 255 : nSpaces );
            if( !pArr->AddToken( aToken ) )
            {
                SetError(errCodeOverflow); return FALSE;
            }
        }
        if( !IsString() )
        {
            String aTmpStr( cSymbol[0] );
            BOOL bMayBeFuncName;
            if ( ScGlobal::pCharClass->isLetter( aTmpStr, 0 ) )
            {   // Einem Namen muss eine Klammer folgen
                const sal_Unicode* p = aFormula.GetBuffer() + nSrcPos;
                while( *p == ' ' )
                    p++;
                bMayBeFuncName = ( *p == '(' );
            }
            else
                bMayBeFuncName = TRUE;      // Operatoren etc.
            String aOrg( cSymbol ); // evtl. Dateinamen in IsReference erhalten
            String aUpper( aOrg );
            ScGlobal::pCharClass->toUpper( aUpper );
            // Spalte DM konnte nicht referiert werden, IsReference vor IsValue
            // #42016# italian ARCTAN.2 gab #REF! => IsOpCode vor IsReference
            if ( !(bMayBeFuncName && IsOpCode( aUpper ))
              && !IsReference( aOrg )
              && !IsValue( aUpper )
              && !IsNamedRange( aUpper )
              && !IsDBRange( aUpper )
              && !IsColRowName( aUpper )
              && !(bMayBeFuncName && IsMacro( aUpper ))
              && !(bMayBeFuncName && IsOpCode2( aUpper )) )
            {
                SetError( errNoName );
                ScGlobal::pCharClass->toLower( aUpper );
                aToken.SetString( aUpper.GetBuffer() );
                aToken.NewOpCode( ocBad );
                pRawToken = aToken.Clone();
                if ( bAutoCorrect )
                    AutoCorrectParsedSymbol();
            }
        }
        return TRUE;
    }
    else
        return FALSE;
}

ScTokenArray* ScCompiler::CompileString( const String& rFormula )
{
    ScTokenArray aArr;
    pArr = &aArr;
    aFormula = rFormula;
    aFormula.EraseLeadingChars();
    aFormula.EraseTrailingChars();
    nSrcPos = 0;
    bCorrected = FALSE;
    if ( bAutoCorrect )
    {
        aCorrectedFormula.Erase();
        aCorrectedSymbol.Erase();
    }
    BYTE nForced = 0;       // ==Formel forciert Recalc auch wenn nicht sichtbar
    if( aFormula.GetChar(nSrcPos) == '=' )
    {
        nSrcPos++;
        nForced++;
        if ( bAutoCorrect )
            aCorrectedFormula += '=';
    }
    if( aFormula.GetChar(nSrcPos) == '=' )
    {
        nSrcPos++;
        nForced++;
        if ( bAutoCorrect )
            aCorrectedFormula += '=';
    }
    short nBrackets = 0;
    eLastOp = ocOpen;
    while( NextNewToken() )
    {
        if( pRawToken->GetOpCode() == ocOpen )
            nBrackets++;
        else if( pRawToken->GetOpCode() == ocClose )
        {
            if( !nBrackets )
            {
                SetError( errPairExpected );
                if ( bAutoCorrect )
                {
                    bCorrected = TRUE;
                    aCorrectedSymbol.Erase();
                }
            }
            else
                nBrackets--;
        }
        if( !pArr->Add( pRawToken->CreateToken() ) )
        {
            SetError(errCodeOverflow); break;
        }
        eLastOp = pRawToken->GetOpCode();
        if ( bAutoCorrect )
            aCorrectedFormula += aCorrectedSymbol;
    }
    if ( eLastOp != ocBad )
    {   // bei ocBad ist der Rest der Formel String, es wuerden zuviele
        // Klammern erscheinen
        ScByteToken aToken( ocClose );
        while( nBrackets-- )
        {
            if( !pArr->AddToken( aToken ) )
            {
                SetError(errCodeOverflow); break;
            }
            if ( bAutoCorrect )
                aCorrectedFormula += pSymbolTable[ocClose];
        }
    }
    if ( nForced >= 2 )
        pArr->SetRecalcModeForced();
    // pArr merken, falls danach CompileTokenArray() kommt
    return pArr = new ScTokenArray( aArr );
}

void ScCompiler::PushTokenArray( ScTokenArray* pa, BOOL bTemp )
{
    if ( bAutoCorrect && !pStack )
    {   // #61426# don't merge stacked subroutine code into entered formula
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol.Erase();
    }
    ScArrayStack* p = new ScArrayStack;
    p->pNext      = pStack;
    p->pArr       = pArr;
    p->bTemp      = bTemp;
    pStack        = p;
    pArr          = pa;
}

void ScCompiler::PopTokenArray()
{
    if( pStack )
    {
        ScArrayStack* p = pStack;
        pStack = p->pNext;
        p->pArr->nRefs += pArr->nRefs;
        // special RecalcMode aus SharedFormula uebernehmen
        if ( pArr->IsRecalcModeAlways() )
            p->pArr->SetRecalcModeAlways();
        else if ( !pArr->IsRecalcModeNormal() && p->pArr->IsRecalcModeNormal() )
            p->pArr->SetMaskedRecalcMode( pArr->GetRecalcMode() );
        p->pArr->SetCombinedBitsRecalcMode( pArr->GetRecalcMode() );
        if( p->bTemp )
            delete pArr;
        pArr = p->pArr;
        delete p;
    }
}

BOOL ScCompiler::GetToken()
{
    if ( bAutoCorrect && !pStack )
    {   // #61426# don't merge stacked subroutine code into entered formula
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol.Erase();
    }
    BOOL bStop = FALSE;
    if( pArr->GetError() && !bIgnoreErrors )
        bStop = TRUE;
    else
    {
        short nWasColRowName;
        if ( pArr->nIndex
          && pArr->pCode[ pArr->nIndex-1 ]->GetOpCode() == ocColRowName )
             nWasColRowName = 1;
        else
             nWasColRowName = 0;
        pToken = pArr->Next();
        while( pToken && pToken->GetOpCode() == ocSpaces )
        {
            if ( nWasColRowName )
                nWasColRowName++;
            if ( bAutoCorrect && !pStack )
                CreateStringFromToken( aCorrectedFormula, pToken, FALSE );
            pToken = pArr->Next();
        }
        if ( bAutoCorrect && !pStack && pToken )
            CreateStringFromToken( aCorrectedSymbol, pToken, FALSE );
        if( !pToken )
        {
            if( pStack )
            {
                PopTokenArray();
                return GetToken();
            }
            else
                bStop = TRUE;
        }
        else
        {
            if ( nWasColRowName >= 2 && pToken->GetOpCode() == ocColRowName )
            {   // aus einem ocSpaces ein ocIntersect im RPN machen
                pToken = new ScByteToken( ocIntersect );
                pArr->nIndex--;     // ganz schweinisch..
            }
        }
    }
    if( bStop )
    {
        pToken = new ScByteToken( ocStop );
        return FALSE;
    }
    if( pToken->GetOpCode() == ocSubTotal )
        glSubTotal = TRUE;
    else if( pToken->GetOpCode() == ocName )
    {
        ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex( pToken->GetIndex() );
        if (pRangeData)
        {
            USHORT nErr = pRangeData->GetErrCode();
            if( nErr )
                SetError( errNoName );
            else if ( !bCompileForFAP )
            {
                ScTokenArray* pNew;
                // #35168# Bereichsformel klammern
                // #37680# aber nur wenn nicht schon Klammern da,
                // geklammerte ocSep geht nicht, z.B. SUMME((...;...))
                // und wenn nicht direkt zwischen ocSep/Klammer,
                // z.B. SUMME(...;(...;...)) nicht, SUMME(...;(...)*3) ja
                // kurz: wenn kein eigenstaendiger Ausdruck
                ScToken* p1 = pArr->PeekPrevNoSpaces();
                ScToken* p2 = pArr->PeekNextNoSpaces();
                OpCode eOp1 = (p1 ? p1->GetOpCode() : ocSep);
                OpCode eOp2 = (p2 ? p2->GetOpCode() : ocSep);
                BOOL bBorder1 = (eOp1 == ocSep || eOp1 == ocOpen);
                BOOL bBorder2 = (eOp2 == ocSep || eOp2 == ocClose);
                BOOL bAddPair = !(bBorder1 && bBorder2);
                if ( bAddPair )
                {
                    pNew = new ScTokenArray;
                    pNew->AddOpCode( ocClose );
                    PushTokenArray( pNew, TRUE );
                    pNew->Reset();
                }
                pNew = pRangeData->GetCode()->Clone();
                PushTokenArray( pNew, TRUE );
                if( pRangeData->HasReferences() )
                {
                    SetRelNameReference();
                    MoveRelWrap();
                }
                pNew->Reset();
                if ( bAddPair )
                {
                    pNew = new ScTokenArray;
                    pNew->AddOpCode( ocOpen );
                    PushTokenArray( pNew, TRUE );
                    pNew->Reset();
                }
                return GetToken();
            }
        }
        else
            SetError(errNoName);
    }
    else if( pToken->GetOpCode() == ocColRowName )
    {
        SingleRefData& rRef = pToken->GetSingleRef();
        rRef.CalcAbsIfRel( aPos );
        if ( !rRef.Valid() )
        {
            SetError( errNoRef );
            return TRUE;
        }
        USHORT nCol = rRef.nCol;
        USHORT nRow = rRef.nRow;
        USHORT nTab = rRef.nTab;
        ScAddress aLook( nCol, nRow, nTab );
        BOOL bColName = rRef.IsColRel();
        USHORT nMyCol = aPos.Col();
        USHORT nMyRow = aPos.Row();
        BOOL bInList = FALSE;
        BOOL bValidName = FALSE;
        ScRangePairList* pRL = (bColName ?
            pDoc->GetColNameRanges() : pDoc->GetRowNameRanges());
        ScRange aRange;
        for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
        {
            if ( pR->GetRange(0).In( aLook ) )
            {
                bInList = bValidName = TRUE;
                aRange = pR->GetRange(1);
                if ( bColName )
                {
                    aRange.aStart.SetCol( nCol );
                    aRange.aEnd.SetCol( nCol );
                }
                else
                {
                    aRange.aStart.SetRow( nRow );
                    aRange.aEnd.SetRow( nRow );
                }
                break;  // for
            }
        }
        if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
        {   // automagically oder durch kopieren entstanden und NamePos nicht in Liste
            BOOL bString = pDoc->HasStringData( nCol, nRow, nTab );
            if ( !bString && !pDoc->GetCell( aLook ) )
                bString = TRUE;     // leere Zelle ist ok
            if ( bString )
            {   //! korrespondiert mit ScInterpreter::ScColRowNameAuto
                bValidName = TRUE;
                if ( bColName )
                {   // ColName
                    USHORT nStartRow = nRow + 1;
                    if ( nStartRow > MAXROW )
                        nStartRow = MAXROW;
                    USHORT nMaxRow = MAXROW;
                    if ( nMyCol == nCol )
                    {   // Formelzelle in gleicher Col
                        if ( nMyRow == nStartRow )
                        {   // direkt unter dem Namen den Rest nehmen
                            nStartRow++;
                            if ( nStartRow > MAXROW )
                                nStartRow = MAXROW;
                        }
                        else if ( nMyRow > nStartRow )
                        {   // weiter unten vom Namen bis zur Formelzelle
                            nMaxRow = nMyRow - 1;
                        }
                    }
                    for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
                    {   // naechster definierter ColNameRange unten ist Row-Begrenzung
                        const ScRange& rRange = pR->GetRange(1);
                        if ( rRange.aStart.Col() <= nCol && nCol <= rRange.aEnd.Col() )
                        {   // gleicher Col Bereich
                            USHORT nTmp = rRange.aStart.Row();
                            if ( nStartRow < nTmp && nTmp <= nMaxRow )
                                nMaxRow = nTmp - 1;
                        }
                    }
                    aRange.aStart.Set( nCol, nStartRow, nTab );
                    aRange.aEnd.Set( nCol, nMaxRow, nTab );
                }
                else
                {   // RowName
                    USHORT nStartCol = nCol + 1;
                    if ( nStartCol > MAXCOL )
                        nStartCol = MAXCOL;
                    USHORT nMaxCol = MAXCOL;
                    if ( nMyRow == nRow )
                    {   // Formelzelle in gleicher Row
                        if ( nMyCol == nStartCol )
                        {   // direkt neben dem Namen den Rest nehmen
                            nStartCol++;
                            if ( nStartCol > MAXCOL )
                                nStartCol = MAXCOL;
                        }
                        else if ( nMyCol > nStartCol )
                        {   // weiter rechts vom Namen bis zur Formelzelle
                            nMaxCol = nMyCol - 1;
                        }
                    }
                    for ( ScRangePair* pR = pRL->First(); pR; pR = pRL->Next() )
                    {   // naechster definierter RowNameRange rechts ist Col-Begrenzung
                        const ScRange& rRange = pR->GetRange(1);
                        if ( rRange.aStart.Row() <= nRow && nRow <= rRange.aEnd.Row() )
                        {   // gleicher Row Bereich
                            USHORT nTmp = rRange.aStart.Col();
                            if ( nStartCol < nTmp && nTmp <= nMaxCol )
                                nMaxCol = nTmp - 1;
                        }
                    }
                    aRange.aStart.Set( nStartCol, nRow, nTab );
                    aRange.aEnd.Set( nMaxCol, nRow, nTab );
                }
            }
        }
        if ( bValidName )
        {
            // Und nun der Zauber zur Unterscheidung zwischen
            // Bereich und einer einzelnen Zelle daraus, die
            // positionsabhaengig von der Formelzelle gewaehlt wird.
            // Ist ein direkter Nachbar ein binaerer Operator (ocAdd etc.)
            // so wird eine SingleRef passend zur Col/Row generiert,
            // ocColRowName bzw. ocIntersect als Nachbar => Range.
            // Spezialfall: Beschriftung gilt fuer eine einzelne Zelle,
            // dann wird eine positionsunabhaengige SingleRef generiert.
            BOOL bSingle = (aRange.aStart == aRange.aEnd);
            BOOL bFound;
            if ( bSingle )
                bFound = TRUE;
            else
            {
                ScToken* p1 = pArr->PeekPrevNoSpaces();
                ScToken* p2 = pArr->PeekNextNoSpaces();
                // Anfang/Ende einer Formel => Single
                OpCode eOp1 = p1 ? p1->GetOpCode() : ocAdd;
                OpCode eOp2 = p2 ? p2->GetOpCode() : ocAdd;
                if ( eOp1 != ocColRowName && eOp1 != ocIntersect
                    && eOp2 != ocColRowName && eOp2 != ocIntersect )
                {
                    if ( (ocEndDiv < eOp1 && eOp1 < ocEndBinOp)
                        || (ocEndDiv < eOp2 && eOp2 < ocEndBinOp) )
                        bSingle = TRUE;
                }
                if ( bSingle )
                {   // Col bzw. Row muss zum Range passen
                    if ( bColName )
                    {
                        bFound = (aRange.aStart.Row() <= nMyRow
                            && nMyRow <= aRange.aEnd.Row());
                        if ( bFound )
                            aRange.aStart.SetRow( nMyRow );
                    }
                    else
                    {
                        bFound = (aRange.aStart.Col() <= nMyCol
                            && nMyCol <= aRange.aEnd.Col());
                        if ( bFound )
                            aRange.aStart.SetCol( nMyCol );
                    }
                }
                else
                    bFound = TRUE;
            }
            if ( !bFound )
                SetError(errNoRef);
            else if ( !bCompileForFAP )
            {
                ScTokenArray* pNew = new ScTokenArray;
                if ( bSingle )
                {
                    SingleRefData aRefData;
                    aRefData.InitAddress( aRange.aStart );
                    if ( bColName )
                        aRefData.SetColRel( TRUE );
                    else
                        aRefData.SetRowRel( TRUE );
                    aRefData.CalcRelFromAbs( aPos );
                    pNew->AddSingleReference( aRefData );
                }
                else
                {
                    ComplRefData aRefData;
                    aRefData.InitRange( aRange );
                    if ( bColName )
                    {
                        aRefData.Ref1.SetColRel( TRUE );
                        aRefData.Ref2.SetColRel( TRUE );
                    }
                    else
                    {
                        aRefData.Ref1.SetRowRel( TRUE );
                        aRefData.Ref2.SetRowRel( TRUE );
                    }
                    aRefData.CalcRelFromAbs( aPos );
                    if ( bInList )
                        pNew->AddDoubleReference( aRefData );
                    else
                    {   // automagically
                        pNew->Add( new ScDoubleRefToken( ocColRowNameAuto,
                            aRefData ) );
                    }
                }
                PushTokenArray( pNew, TRUE );
                pNew->Reset();
                return GetToken();
            }
        }
        else
            SetError(errNoName);
    }
    else if( pToken->GetOpCode() == ocDBArea )
    {
        ScDBData* pDBData = pDoc->GetDBCollection()->FindIndex( pToken->GetIndex() );
        if ( !pDBData )
            SetError(errNoName);
        else if ( !bCompileForFAP )
        {
            ComplRefData aRefData;
            aRefData.InitFlags();
            pDBData->GetArea(   (USHORT&) aRefData.Ref1.nTab,
                                (USHORT&) aRefData.Ref1.nCol,
                                (USHORT&) aRefData.Ref1.nRow,
                                (USHORT&) aRefData.Ref2.nCol,
                                (USHORT&) aRefData.Ref2.nRow);
            aRefData.Ref2.nTab    = aRefData.Ref1.nTab;
            aRefData.CalcRelFromAbs( aPos );
            ScTokenArray* pNew = new ScTokenArray;
            pNew->AddDoubleReference( aRefData );
            PushTokenArray( pNew, TRUE );
            pNew->Reset();
            return GetToken();
        }
    }
    else if( pToken->GetType() == svSingleRef )
    {
//      if (!pDoc->HasTable( pToken->aRef.Ref1.nTab ) )
//          SetError(errNoRef);
        pArr->nRefs++;
    }
    else if( pToken->GetType() == svDoubleRef )
    {
//      if (!pDoc->HasTable( pToken->aRef.Ref1.nTab ) ||
//          !pDoc->HasTable( pToken->aRef.Ref2.nTab ))
//          SetError(errNoRef);
        pArr->nRefs++;
    }
    return TRUE;
}

OpCode ScCompiler::NextToken()
{
    if( !GetToken() )
        return ocStop;
    OpCode eOp = pToken->GetOpCode();
    // #38815# CompileTokenArray mit zurueckgesetztem Fehler gibt wieder Fehler
    if ( eOp == ocBad )
        SetError( errNoName );
    // Vor einem Push muss ein Operator kommen
    if ( (eOp == ocPush || eOp == ocColRowNameAuto) &&
            !( (eLastOp == ocOpen) || (eLastOp == ocSep) ||
                ((eLastOp > ocEndDiv) && (eLastOp < ocEndUnOp))) )
        SetError(errOperatorExpected);
    // Operator und Plus = Operator
    BOOL bLastOp = ( eLastOp == ocOpen || eLastOp == ocSep ||
          (eLastOp > ocEndDiv && eLastOp < ocEndUnOp)
        );
    if( bLastOp && eOp == ocAdd )
        eOp = NextToken();
    else
    {
        // Vor einem Operator darf kein weiterer Operator stehen
        // Aber AND, OR ist OK
        if ( eOp != ocAnd && eOp != ocOr
          && ( eOp > ocEndDiv && eOp < ocEndBinOp )
          && ( eLastOp == ocOpen || eLastOp == ocSep
            || (eLastOp > ocEndDiv && eLastOp < ocEndUnOp)) )
        {
            SetError(errVariableExpected);
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == eLastOp || eLastOp == ocOpen )
                {   // doppelten Operator verwerfen
                    aCorrectedSymbol.Erase();
                    bCorrected = TRUE;
                }
                else
                {
                    xub_StrLen nPos = aCorrectedFormula.Len();
                    if ( nPos )
                    {
                        nPos--;
                        sal_Unicode c = aCorrectedFormula.GetChar( nPos );
                        switch ( eOp )
                        {   // Operatoren vertauschen
                            case ocGreater:
                                if ( c == pSymbolTable[ocEqual].GetChar(0) )
                                {   // >= ist richtig statt =>
                                    aCorrectedFormula.SetChar( nPos,
                                        pSymbolTable[ocGreater].GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = TRUE;
                                }
                            break;
                            case ocLess:
                                if ( c == pSymbolTable[ocEqual].GetChar(0) )
                                {   // <= ist richtig statt =<
                                    aCorrectedFormula.SetChar( nPos,
                                        pSymbolTable[ocLess].GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = TRUE;
                                }
                                else if ( c == pSymbolTable[ocGreater].GetChar(0) )
                                {   // <> ist richtig statt ><
                                    aCorrectedFormula.SetChar( nPos,
                                        pSymbolTable[ocLess].GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = TRUE;
                                }
                            break;
                            case ocMul:
                                if ( c == pSymbolTable[ocSub].GetChar(0) )
                                {   // *- statt -*
                                    aCorrectedFormula.SetChar( nPos,
                                        pSymbolTable[ocMul].GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = TRUE;
                                }
                            break;
                            case ocDiv:
                                if ( c == pSymbolTable[ocSub].GetChar(0) )
                                {   // /- statt -/
                                    aCorrectedFormula.SetChar( nPos,
                                        pSymbolTable[ocDiv].GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = TRUE;
                                }
                            break;
                        }
                    }
                }
            }
        }
        eLastOp = eOp;
    }
    return eOp;
}

//---------------------------------------------------------------------------

BOOL ScCompiler::CompileTokenArray()
{
    glSubTotal = FALSE;
    bCorrected = FALSE;
    if( !pArr->nError || bIgnoreErrors )
    {
        if ( bAutoCorrect )
        {
            aCorrectedFormula.Erase();
            aCorrectedSymbol.Erase();
        }
        pArr->nRefs = 0;    // wird neu gezaehlt
        pArr->DelRPN();
        pStack = NULL;
        ScToken* pData[ MAXCODE ];
        pCode = pData;
        BOOL bWasForced = pArr->IsRecalcModeForced();
        if ( bWasForced )
        {
            if ( bAutoCorrect )
                aCorrectedFormula = '=';
        }
        pArr->ClearRecalcMode();
        pArr->Reset();
        eLastOp = ocOpen;
        pc = 0;
        NextToken();
        Expression();

        USHORT nErrorBeforePop = pArr->nError;

        while( pStack )
            PopTokenArray();
        if( pc )
        {
            pArr->pRPN = new ScToken*[ pc ];
            pArr->nRPN = pc;
            memcpy( pArr->pRPN, pData, pc * sizeof( ScToken* ) );
        }

        if( !pArr->nError && nErrorBeforePop )
            pArr->nError = nErrorBeforePop;             // einmal Fehler, immer Fehler

        if( pArr->nError && !bIgnoreErrors )
            pArr->DelRPN();

        if ( bWasForced )
            pArr->SetRecalcModeForced();
    }
    if( nNumFmt == NUMBERFORMAT_UNDEFINED )
        nNumFmt = NUMBERFORMAT_NUMBER;
    return glSubTotal;
}

//---------------------------------------------------------------------------
// Token in den Code Eintragen
//---------------------------------------------------------------------------

void ScCompiler::PutCode( ScRawToken* p )
{
    PutCode( p->CreateToken() );
}

void ScCompiler::PutCode( ScToken* p )
{
    if( pc >= MAXCODE )
        SetError(errCodeOverflow);
    if( pArr->GetError() && !bCompileForFAP )
        return;
    *pCode++ = p; pc++;
    p->IncRef();
}

//---------------------------------------------------------------------------
// UPN-Erzeugung (durch Rekursion)
//---------------------------------------------------------------------------

void ScCompiler::Factor()
{
    if ( pArr->GetError() && !bIgnoreErrors )
        return;
    ScTokenRef pFacToken;
    OpCode eOp = pToken->GetOpCode();
    if( eOp == ocPush || eOp == ocColRowNameAuto || eOp == ocMatRef ||
            eOp == ocDBArea
            || (bCompileForFAP && ((eOp == ocName) || (eOp == ocDBArea)
            || (eOp == ocColRowName) || (eOp == ocBad)))
        )
    {
        PutCode( pToken );
        eOp = NextToken();
        if( eOp == ocOpen )
        {
            // PUSH( ist ein Fehler, der durch eine unbekannte
            // Funktion hervorgerufen wird.
            SetError(
                ( pToken->GetType() == svString
               || pToken->GetType() == svSingleRef )
               ? errNoName : errOperatorExpected );
            if ( bAutoCorrect && !pStack )
            {   // Multiplikation annehmen
                aCorrectedFormula += pSymbolTable[ocMul];
                bCorrected = TRUE;
                NextToken();
                eOp = Expression();
                if( eOp != ocClose )
                    SetError(errPairExpected);
                else
                    eOp = NextToken();
            }
        }
    }
    else if( eOp == ocOpen )
    {
        NextToken();
        eOp = Expression();
        if( eOp != ocClose )
            SetError(errPairExpected);
        else
            eOp = NextToken();
    }
    else
    {
        if( nNumFmt == NUMBERFORMAT_UNDEFINED )
            nNumFmt = lcl_GetRetFormat( eOp );
        if( eOp > ocEndUnOp && eOp < ocEndNoPar)
        {
            // Diese Funktionen muessen immer neu berechnet werden
            switch( eOp )
            {
                case ocRandom:
                case ocGetActDate:
                case ocGetActTime:
                    pArr->SetRecalcModeAlways();
                break;
            }
            pFacToken = pToken;
            eOp = NextToken();
            if (eOp != ocOpen)
            {
                SetError(errPairExpected);
                PutCode( pFacToken );
            }
            else
            {
                eOp = NextToken();
                if (eOp != ocClose)
                    SetError(errPairExpected);
                PutCode(pFacToken);
                eOp = NextToken();
            }
        }
        // Spezialfall NICHT() und NEG()
        else if( eOp == ocNot || eOp == ocNeg
              || ( eOp > ocEndNoPar && eOp < ocEnd1Par) )
        {
            pFacToken = pToken;
            eOp = NextToken();
            if( nNumFmt == NUMBERFORMAT_UNDEFINED && eOp == ocNot )
                nNumFmt = NUMBERFORMAT_LOGICAL;
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError(errPairExpected);
            if (eOp != ocClose)
                SetError(errPairExpected);
            else if ( !pArr->GetError() )
                pFacToken->SetByte( 1 );
            PutCode( pFacToken );
            eOp = NextToken();
        }
        else if ((eOp > ocEnd1Par && eOp < ocEnd2Par)
                || eOp == ocExternal
                || eOp == ocMacro
                || eOp == ocAnd
                || eOp == ocOr
                || ( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
                || (bCompileForFAP && ((eOp == ocIf) || (eOp == ocChose)))
            )
        {
            OpCode eFuncOp = eOp;
            pFacToken = pToken;
            eOp = NextToken();
            BOOL bNoParam = FALSE;
            BOOL bNoPair = FALSE;
            BYTE nMultiAreaSep = 0;
            if (eOp == ocOpen)
            {
                eOp = NextToken();
                if ( eFuncOp == ocIndex && eOp == ocOpen )
                {   // Mehrfachbereiche
                    BYTE SepCount = 0;
                    do
                    {
                        eOp = NextToken();
                        if ( eOp != ocClose )
                        {
                            SepCount++;
                            eOp = Expression();
                        }
                    } while ( (eOp == ocSep) && (!pArr->GetError() || bIgnoreErrors) );
                    if ( eOp != ocClose )
                        SetError(errPairExpected);
                    eOp = NextToken();
                    if ( eOp == ocSep )
                    {
                        nMultiAreaSep = 1;
                        eOp = NextToken();
                    }
                    if ( SepCount == 0 )
                    {
                        if ( eOp == ocClose )
                            bNoParam = TRUE;
                    }
                    else
                    {
                        PutCode( new ScByteToken( ocPush, SepCount ) );
                        if ( eOp != ocClose )
                            eOp = Expression();
                    }
                }
                else
                {
                    if (eOp == ocClose)
                        bNoParam = TRUE;
                    else
                        eOp = Expression();
                }
            }
            else
            {
                if (pFacToken->GetOpCode() == ocMultiArea)
                {
                    bNoPair = TRUE;
                    NextToken();
                    eOp = Expression();
                }
                else
                    SetError(errPairExpected);
            }
            BYTE SepCount = nMultiAreaSep;
            if( !bNoParam )
            {
                SepCount++;
                while ( (eOp == ocSep) && (!pArr->GetError() || bIgnoreErrors) )
                {
                    SepCount++;
                    NextToken();
                    eOp = Expression();
                }
            }
            if ((eOp != ocClose) && !bNoPair )
                SetError(errPairExpected);
            else
                eOp = NextToken();
            pFacToken->SetByte( SepCount );
            PutCode( pFacToken );
            // Diese Funktionen muessen immer neu berechnet werden
            switch( eFuncOp )
            {
                // ocIndirect muesste sonst bei jedem Interpret StopListening
                // und StartListening fuer ein RefTripel ausfuehren
                case ocIndirect:
                // ocOffset und ocIndex liefern indirekte Refs
                case ocOffset:
                case ocIndex:
                    pArr->SetRecalcModeAlways();
                break;
                // Functions recalculated on every document load.
                // Don't use SetRecalcModeOnLoad which would override ModeAlways
                case ocConvert :
                    pArr->AddRecalcMode( RECALCMODE_ONLOAD );
                break;
                // wird die referierte verschoben, aendert sich der Wert
                case ocColumn :
                case ocRow :
                    pArr->SetRecalcModeOnRefMove();
                break;
            }
        }
        else if (eOp == ocIf || eOp == ocChose)
        {
            // Die PC-Staende sind -1
            pFacToken = pToken;
            if ( eOp == ocIf )
                pFacToken->GetJump()[ 0 ] = 3;  // if, else, behind
            else
                pFacToken->GetJump()[ 0 ] = MAXJUMPCOUNT+1;
            eOp = NextToken();
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError(errPairExpected);
            short nJumpCount = 0;
            PutCode( pFacToken );
            // #36253# bei AutoCorrect (da pArr->nError ignoriert wird)
            // unbegrenztes ocIf gibt GPF weil ScRawToken::Clone den JumpBuffer
            // anhand von nJump[0]*2+2 alloziert, was bei ocIf 3*2+2 ist
            const short nJumpMax =
                (pFacToken->GetOpCode() == ocIf ? 3 : MAXJUMPCOUNT);
            while ( (nJumpCount < (MAXJUMPCOUNT - 1)) && (eOp == ocSep)
                    && (!pArr->GetError() || bIgnoreErrors) )
            {
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[nJumpCount] = pc-1;
                NextToken();
                eOp = Expression();
                PutCode( pToken );      // Als Terminator des Teilausdrucks
            }
            if (eOp != ocClose)
                SetError(errPairExpected);
            else
            {
                eOp = NextToken();
                // auch ohne AutoCorrect gibt es hier ohne nJumpMax bei
                // mehr als 3 Parametern in ocIf einen Ueberschreiber,
                // das war auch schon in der 312 so (jaja, die Tester..)
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[ nJumpCount ] = pc-1;
                if ((pFacToken->GetOpCode() == ocIf && (nJumpCount > 3)) ||
                                 (nJumpCount >= MAXJUMPCOUNT))
                    SetError(errIllegalParameter);
                else
                    pFacToken->GetJump()[ 0 ] = nJumpCount;
            }
        }
        else if ( eOp == ocBad )
        {
            SetError( errNoName );
        }
        else if ( eOp == ocClose )
        {
            SetError( errParameterExpected );
        }
        else if ( eOp == ocMissing )
        {   // #84460# May occur if imported from Xcl.
            // The real value for missing parameters depends on the function
            // where it is used, interpreter would have to handle this.
            // If it does remove this error case here, that could also be the
            // time to generate ocMissing in between subsequent ocSep.
            // Xcl import should map missings to values if possible.
            SetError( errParameterExpected );
        }
        else if ( eOp == ocSep )
        {   // Subsequent ocSep
            SetError( errParameterExpected );
            if ( bAutoCorrect && !pStack )
            {
                aCorrectedSymbol.Erase();
                bCorrected = TRUE;
            }
        }
        else
        {
            SetError( errUnknownToken );
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == ocStop )
                {   // trailing operator w/o operand
                    xub_StrLen nLen = aCorrectedFormula.Len();
                    if ( nLen )
                        aCorrectedFormula.Erase( nLen - 1 );
                    aCorrectedSymbol.Erase();
                    bCorrected = TRUE;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

void ScCompiler::Unary()
{
    if( pToken->GetOpCode() == ocAdd )
        GetToken();
    else if ( pToken->GetOpCode() > ocNot && pToken->GetOpCode() < ocEndUnOp )
    {
        ScTokenRef p = pToken;
        NextToken();
        Factor();
        PutCode( p );
    }
    else
        Factor();
}

void ScCompiler::PowLine()
{
    Unary();
    while (pToken->GetOpCode() == ocPow)
    {
        ScTokenRef p = pToken;
        NextToken();
        Unary();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void ScCompiler::UnionCutLine()
{
    PowLine();
    while (pToken->GetOpCode() == ocIntersect)
    {
        ScTokenRef p = pToken;
        NextToken();
        PowLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void ScCompiler::MulDivLine()
{
    UnionCutLine();
    while (pToken->GetOpCode() == ocMul || pToken->GetOpCode() == ocDiv)
    {
        ScTokenRef p = pToken;
        NextToken();
        UnionCutLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void ScCompiler::AddSubLine()
{
    MulDivLine();
    while (pToken->GetOpCode() == ocAdd || pToken->GetOpCode() == ocSub
                                || pToken->GetOpCode() == ocAmpersand)
    {
        ScTokenRef p = pToken;
        NextToken();
        MulDivLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void ScCompiler::CompareLine()
{
    AddSubLine();
    while (pToken->GetOpCode() >= ocEqual && pToken->GetOpCode() <= ocGreaterEqual)
    {
        ScTokenRef p = pToken;
        NextToken();
        AddSubLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void ScCompiler::NotLine()
{
    CompareLine();
    while (pToken->GetOpCode() == ocNot)
    {
        ScTokenRef p = pToken;
        NextToken();
        CompareLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

OpCode ScCompiler::Expression()
{
    static const short nRecursionMax = 42;
    if ( ++nRecursion > nRecursionMax )
    {
        SetError( errStackOverflow );
        return ocStop;      //! stattdessen Token generieren?
    }
    NotLine();
    while (pToken->GetOpCode() == ocAnd || pToken->GetOpCode() == ocOr)
    {
        ScTokenRef p = pToken;
        pToken->SetByte( 2 );       // 2 parameters!
        NextToken();
        NotLine();
        PutCode(p);
    }
    --nRecursion;
    return pToken->GetOpCode();
}

//-----------------------------------------------------------------------------

BOOL ScCompiler::HasModifiedRange()
{
    pArr->Reset();
    for ( ScToken* t = pArr->Next(); t; t = pArr->Next() )
    {
        OpCode eOpCode = t->GetOpCode();
        if ( eOpCode == ocName )
        {
            ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex(t->GetIndex());

            if (pRangeData && pRangeData->IsModified())
                return TRUE;
        }
        else if ( eOpCode == ocDBArea )
        {
            ScDBData* pDBData = pDoc->GetDBCollection()->FindIndex(t->GetIndex());

            if (pDBData && pDBData->IsModified())
                return TRUE;
        }
    }
    return FALSE;
}

//-----------------------------------------------------------------------------

short lcl_adjval( short n, short pos, short max, BOOL bRel )
{
    max++;
    if( bRel ) n += pos;
    if( n < 0 ) n += max;
    else
    if( n >= max ) n -= max;
    if( bRel ) n -= pos;
    return n;
}

void ScCompiler::AdjustReference( SingleRefData& r )
{
    if( r.IsColRel() )
        r.nCol = lcl_adjval( r.nCol, aPos.Col(), MAXCOL, r.IsColRel() );
    if( r.IsRowRel() )
        r.nRow = lcl_adjval( r.nRow, aPos.Row(), MAXROW, r.IsRowRel() );
    if( r.IsTabRel() )
        r.nTab = lcl_adjval( r.nTab, aPos.Tab(), nMaxTab,r.IsTabRel() );
}

// Referenz aus benanntem Bereich mit relativen Angaben

void ScCompiler::SetRelNameReference()
{
    pArr->Reset();
    for( ScToken* t = pArr->GetNextReference(); t;
                  t = pArr->GetNextReference() )
    {
        SingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
            rRef1.SetRelName( TRUE );
        if ( t->GetType() == svDoubleRef )
        {
            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
                rRef2.SetRelName( TRUE );
        }
    }
}

// nur relative aus RangeName mit Wrap an Position anpassen
void ScCompiler::MoveRelWrap()
{
    pArr->Reset();
    for( ScToken* t = pArr->GetNextReference(); t;
                  t = pArr->GetNextReference() )
    {
        if ( t->GetType() == svSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, aPos, SingleDoubleRefModifier( t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, aPos, t->GetDoubleRef() );
    }
}

// static
// nur relative aus RangeName mit Wrap an Position anpassen
void ScCompiler::MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc,
            const ScAddress& rPos )
{
    rArr.Reset();
    for( ScToken* t = rArr.GetNextReference(); t;
                  t = rArr.GetNextReference() )
    {
        if ( t->GetType() == svSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, rPos, SingleDoubleRefModifier( t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, rPos, t->GetDoubleRef() );
    }
}

ScRangeData* ScCompiler::UpdateReference(UpdateRefMode eUpdateRefMode,
                                 const ScAddress& rOldPos, const ScRange& r,
                                 short nDx, short nDy, short nDz,
                                 BOOL& rChanged)
{
    rChanged = FALSE;
    if ( eUpdateRefMode == URM_COPY )
        return NULL;                    // mit RelRefs ok, auch SharedFormulas
    else
    {
        ScRangeData* pRangeData = NULL;
        ScToken* t;
        pArr->Reset();
        for( t = pArr->GetNextReferenceOrName(); t;
             t = pArr->GetNextReferenceOrName() )
        {
            if( t->GetOpCode() == ocName )
            {
                ScRangeData* pName = pDoc->GetRangeName()->FindIndex( t->GetIndex() );
                if (pName && pName->HasType(RT_SHAREDMOD))
                {
                    pRangeData = pName;     // => neu kompilieren etc.
                    rChanged = TRUE;
                }
            }
            else if( t->GetType() != svIndex )  // es kann ein DB-Bereich sein !!!
            {
                t->CalcAbsIfRel( rOldPos );
                if ( t->GetType() == svSingleRef )
                {
                    if ( ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                            r, nDx, nDy, nDz,
                            SingleDoubleRefModifier( t->GetSingleRef() ).Ref() )
                            != UR_NOTHING
                        )
                        rChanged = TRUE;
                }
                else
                {
                    if ( ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                                r, nDx, nDy, nDz, t->GetDoubleRef() )
                            != UR_NOTHING
                        )
                        rChanged = TRUE;
                }
            }
        }
        pArr->Reset();
        for( t = pArr->GetNextReferenceRPN(); t;
             t = pArr->GetNextReferenceRPN() )
        {
            if ( t->GetRef() == 1 )
            {   // bei nRefCnt>1 bereits im Code angepasst
                if ( t->GetType() == svSingleRef )
                {
                    SingleRefData& rRef = t->GetSingleRef();
                    SingleDoubleRefModifier aMod( rRef );
                    if ( rRef.IsRelName() )
                    {
                        ScRefUpdate::MoveRelWrap( pDoc, aPos, aMod.Ref() );
                        rChanged = TRUE;
                    }
                    else
                    {
                        aMod.Ref().CalcAbsIfRel( rOldPos );
                        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                                    r, nDx, nDy, nDz, aMod.Ref() )
                                != UR_NOTHING
                            )
                            rChanged = TRUE;
                    }
                }
                else
                {
                    ComplRefData& rRef = t->GetDoubleRef();
                    if ( rRef.Ref1.IsRelName() || rRef.Ref2.IsRelName() )
                    {
                        ScRefUpdate::MoveRelWrap( pDoc, aPos, rRef );
                        rChanged = TRUE;
                    }
                    else
                    {
                        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                                    r, nDx, nDy, nDz, rRef )
                                != UR_NOTHING
                            )
                            rChanged = TRUE;
                    }
                }
            }
        }
        return pRangeData;
    }
}

BOOL ScCompiler::UpdateNameReference(UpdateRefMode eUpdateRefMode,
                                     const ScRange& r,
                                     short nDx, short nDy, short nDz,
                                     BOOL& rChanged)
{
    BOOL bRet = FALSE;                      // wird gesetzt, wenn rel-Ref
    rChanged = FALSE;
    pArr->Reset();
    for( ScToken* t = pArr->GetNextReference(); t;
                  t = pArr->GetNextReference() )
    {
        SingleDoubleRefModifier& rMod = (t->GetType() == svSingleRef ?
            SingleDoubleRefModifier( t->GetSingleRef() ) :
            SingleDoubleRefModifier( t->GetDoubleRef() ));
        ComplRefData& rRef = rMod.Ref();
        if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() &&
                (!rRef.Ref1.IsFlag3D() || !rRef.Ref1.IsTabRel()) &&
            ( t->GetType() == svSingleRef ||
            (!rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel() &&
                (!rRef.Ref2.IsFlag3D() || !rRef.Ref2.IsTabRel()))))
        {
            if (ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                                     r, nDx, nDy, nDz, rRef ) != UR_NOTHING )
                rChanged = TRUE;
        }
        else
            bRet = TRUE;
    }
    return bRet;
}


void ScCompiler::UpdateSharedFormulaReference( UpdateRefMode eUpdateRefMode,
                                  const ScAddress& rOldPos, const ScRange& r,
                                  short nDx, short nDy, short nDz )
{
    if ( eUpdateRefMode == URM_COPY )
        return ;
    else
    {
        ScToken* t;
        pArr->Reset();
        for( t = pArr->GetNextReference(); t;
             t = pArr->GetNextReference() )
        {
            if( t->GetType() != svIndex )   // es kann ein DB-Bereich sein !!!
            {
                t->CalcAbsIfRel( rOldPos );
                // Absolute references have been already adjusted in the named
                // shared formula itself prior to breaking the shared formula
                // and calling this function. Don't readjust them again.
                SingleDoubleRefModifier& rMod = (t->GetType() == svSingleRef ?
                    SingleDoubleRefModifier( t->GetSingleRef() ) :
                    SingleDoubleRefModifier( t->GetDoubleRef() ));
                ComplRefData& rRef = rMod.Ref();
                ComplRefData aBkp = rRef;
                ScRefUpdate::Update( pDoc, eUpdateRefMode, aPos,
                                            r, nDx, nDy, nDz, rRef );
                // restore absolute parts
                if ( !aBkp.Ref1.IsColRel() )
                {
                    rRef.Ref1.nCol = aBkp.Ref1.nCol;
                    rRef.Ref1.nRelCol = aBkp.Ref1.nRelCol;
                    rRef.Ref1.SetColDeleted( aBkp.Ref1.IsColDeleted() );
                }
                if ( !aBkp.Ref1.IsRowRel() )
                {
                    rRef.Ref1.nRow = aBkp.Ref1.nRow;
                    rRef.Ref1.nRelRow = aBkp.Ref1.nRelRow;
                    rRef.Ref1.SetRowDeleted( aBkp.Ref1.IsRowDeleted() );
                }
                if ( !aBkp.Ref1.IsTabRel() )
                {
                    rRef.Ref1.nTab = aBkp.Ref1.nTab;
                    rRef.Ref1.nRelTab = aBkp.Ref1.nRelTab;
                    rRef.Ref1.SetTabDeleted( aBkp.Ref1.IsTabDeleted() );
                }
                if ( t->GetType() == svDoubleRef )
                {
                    if ( !aBkp.Ref2.IsColRel() )
                    {
                        rRef.Ref2.nCol = aBkp.Ref2.nCol;
                        rRef.Ref2.nRelCol = aBkp.Ref2.nRelCol;
                        rRef.Ref2.SetColDeleted( aBkp.Ref2.IsColDeleted() );
                    }
                    if ( !aBkp.Ref2.IsRowRel() )
                    {
                        rRef.Ref2.nRow = aBkp.Ref2.nRow;
                        rRef.Ref2.nRelRow = aBkp.Ref2.nRelRow;
                        rRef.Ref2.SetRowDeleted( aBkp.Ref2.IsRowDeleted() );
                    }
                    if ( !aBkp.Ref2.IsTabRel() )
                    {
                        rRef.Ref2.nTab = aBkp.Ref2.nTab;
                        rRef.Ref2.nRelTab = aBkp.Ref2.nRelTab;
                        rRef.Ref2.SetTabDeleted( aBkp.Ref2.IsTabDeleted() );
                    }
                }
            }
        }
    }
}


ScRangeData* ScCompiler::UpdateInsertTab( USHORT nTable, BOOL bIsName )
{
    ScRangeData* pRangeData = NULL;
    USHORT nTab;
    USHORT nPosTab = aPos.Tab();    // _nach_ evtl. Increment!
    USHORT nOldPosTab = ((nPosTab > nTable) ? (nPosTab - 1) : nPosTab);
    BOOL bIsRel = FALSE;
    ScToken* t;
    pArr->Reset();
    if (bIsName)
        t = pArr->GetNextReference();
    else
        t = pArr->GetNextReferenceOrName();
    while( t )
    {
        if( t->GetOpCode() == ocName )
        {
            if (!bIsName)
            {
                ScRangeData* pName = pDoc->GetRangeName()->FindIndex(t->GetIndex());
                if (pName && pName->HasType(RT_SHAREDMOD))
                    pRangeData = pName;
            }
        }
        else if( t->GetType() != svIndex )  // es kann ein DB-Bereich sein !!!
        {
            if ( !(bIsName && t->GetSingleRef().IsTabRel()) )
            {   // Namen nur absolute anpassen
                SingleRefData& rRef = t->GetSingleRef();
                if ( rRef.IsTabRel() )
                    nTab = rRef.nRelTab + nOldPosTab;
                else
                    nTab = rRef.nTab;
                if ( nTable <= nTab )
                    rRef.nTab = nTab + 1;
                rRef.nRelTab = rRef.nTab - nPosTab;
            }
            else
                bIsRel = TRUE;
            if ( t->GetType() == svDoubleRef )
            {
                if ( !(bIsName && t->GetDoubleRef().Ref2.IsTabRel()) )
                {   // Namen nur absolute anpassen
                    SingleRefData& rRef = t->GetDoubleRef().Ref2;
                    if ( rRef.IsTabRel() )
                        nTab = rRef.nRelTab + nOldPosTab;
                    else
                        nTab = rRef.nTab;
                    if ( nTable <= nTab )
                        rRef.nTab = nTab + 1;
                    rRef.nRelTab = rRef.nTab - nPosTab;
                }
                else
                    bIsRel = TRUE;
            }
            if ( bIsName && bIsRel )
                pRangeData = (ScRangeData*) this;   // wird in rangenam nicht dereferenziert
        }
        if (bIsName)
            t = pArr->GetNextReference();
        else
            t = pArr->GetNextReferenceOrName();
    }
    if ( !bIsName )
    {
        pArr->Reset();
        for ( t = pArr->GetNextReferenceRPN(); t;
              t = pArr->GetNextReferenceRPN() )
        {
            if ( t->GetRef() == 1 )
            {
                SingleRefData& rRef1 = t->GetSingleRef();
                if ( !(rRef1.IsRelName() && rRef1.IsTabRel()) )
                {   // Namen nur absolute anpassen
                    if ( rRef1.IsTabRel() )
                        nTab = rRef1.nRelTab + nOldPosTab;
                    else
                        nTab = rRef1.nTab;
                    if ( nTable <= nTab )
                        rRef1.nTab = nTab + 1;
                    rRef1.nRelTab = rRef1.nTab - nPosTab;
                }
                if ( t->GetType() == svDoubleRef )
                {
                    SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                    if ( !(rRef2.IsRelName() && rRef2.IsTabRel()) )
                    {   // Namen nur absolute anpassen
                        if ( rRef2.IsTabRel() )
                            nTab = rRef2.nRelTab + nOldPosTab;
                        else
                            nTab = rRef2.nTab;
                        if ( nTable <= nTab )
                            rRef2.nTab = nTab + 1;
                        rRef2.nRelTab = rRef2.nTab - nPosTab;
                    }
                }
            }
        }
    }
    return pRangeData;
}

ScRangeData* ScCompiler::UpdateDeleteTab(USHORT nTable, BOOL bIsMove, BOOL bIsName,
                                 BOOL& rChanged)
{
    ScRangeData* pRangeData = NULL;
    USHORT nTab, nTab2;
    USHORT nPosTab = aPos.Tab();         // _nach_ evtl. Decrement!
    USHORT nOldPosTab = ((nPosTab >= nTable) ? (nPosTab + 1) : nPosTab);
    rChanged = FALSE;
    BOOL bIsRel = FALSE;
    ScToken* t;
    pArr->Reset();
    if (bIsName)
        t = pArr->GetNextReference();
    else
        t = pArr->GetNextReferenceOrName();
    while( t )
    {
        if( t->GetOpCode() == ocName )
        {
            if (!bIsName)
            {
                ScRangeData* pName = pDoc->GetRangeName()->FindIndex(t->GetIndex());
                if (pName && pName->HasType(RT_SHAREDMOD))
                    pRangeData = pName;
            }
            rChanged = TRUE;
        }
        else if( t->GetType() != svIndex )  // es kann ein DB-Bereich sein !!!
        {
            if ( !(bIsName && t->GetSingleRef().IsTabRel()) )
            {   // Namen nur absolute anpassen
                SingleRefData& rRef = t->GetSingleRef();
                if ( rRef.IsTabRel() )
                    nTab = rRef.nRelTab + nOldPosTab;
                else
                    nTab = rRef.nTab;
                if ( nTable < nTab )
                {
                    rRef.nTab = nTab - 1;
                    rChanged = TRUE;
                }
                else if ( nTable == nTab )
                {
                    if ( t->GetType() == svDoubleRef )
                    {
                        SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                        if ( rRef2.IsTabRel() )
                            nTab2 = rRef2.nRelTab + nOldPosTab;
                        else
                            nTab2 = rRef2.nTab;
                        if ( nTab == nTab2
                          || (nTab+1) >= pDoc->GetTableCount() )
                        {
                            rRef.nTab = MAXTAB+1;
                            rRef.SetTabDeleted( TRUE );
                        }
                        // else: nTab zeigt spaeter auf jetziges nTable+1
                        // => Bereich verkleinert
                    }
                    else
                    {
                        rRef.nTab = MAXTAB+1;
                        rRef.SetTabDeleted( TRUE );
                    }
                    rChanged = TRUE;
                }
                rRef.nRelTab = rRef.nTab - nPosTab;
            }
            else
                bIsRel = TRUE;
            if ( t->GetType() == svDoubleRef )
            {
                if ( !(bIsName && t->GetDoubleRef().Ref2.IsTabRel()) )
                {   // Namen nur absolute anpassen
                    SingleRefData& rRef = t->GetDoubleRef().Ref2;
                    if ( rRef.IsTabRel() )
                        nTab = rRef.nRelTab + nOldPosTab;
                    else
                        nTab = rRef.nTab;
                    if ( nTable < nTab )
                    {
                        rRef.nTab = nTab - 1;
                        rChanged = TRUE;
                    }
                    else if ( nTable == nTab )
                    {
                        if ( !t->GetDoubleRef().Ref1.IsTabDeleted() )
                            rRef.nTab = nTab - 1;   // Bereich verkleinern
                        else
                        {
                            rRef.nTab = MAXTAB+1;
                            rRef.SetTabDeleted( TRUE );
                        }
                        rChanged = TRUE;
                    }
                    rRef.nRelTab = rRef.nTab - nPosTab;
                }
                else
                    bIsRel = TRUE;
            }
            if ( bIsName && bIsRel )
                pRangeData = (ScRangeData*) this;   // wird in rangenam nicht dereferenziert
        }
        if (bIsName)
            t = pArr->GetNextReference();
        else
            t = pArr->GetNextReferenceOrName();
    }
    if ( !bIsName )
    {
        pArr->Reset();
        for ( t = pArr->GetNextReferenceRPN(); t;
              t = pArr->GetNextReferenceRPN() )
        {
            if ( t->GetRef() == 1 )
            {
                SingleRefData& rRef1 = t->GetSingleRef();
                if ( !(rRef1.IsRelName() && rRef1.IsTabRel()) )
                {   // Namen nur absolute anpassen
                    if ( rRef1.IsTabRel() )
                        nTab = rRef1.nRelTab + nOldPosTab;
                    else
                        nTab = rRef1.nTab;
                    if ( nTable < nTab )
                    {
                        rRef1.nTab = nTab - 1;
                        rChanged = TRUE;
                    }
                    else if ( nTable == nTab )
                    {
                        if ( t->GetType() == svDoubleRef )
                        {
                            SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                            if ( rRef2.IsTabRel() )
                                nTab2 = rRef2.nRelTab + nOldPosTab;
                            else
                                nTab2 = rRef2.nTab;
                            if ( nTab == nTab2
                              || (nTab+1) >= pDoc->GetTableCount() )
                            {
                                rRef1.nTab = MAXTAB+1;
                                rRef1.SetTabDeleted( TRUE );
                            }
                            // else: nTab zeigt spaeter auf jetziges nTable+1
                            // => Bereich verkleinert
                        }
                        else
                        {
                            rRef1.nTab = MAXTAB+1;
                            rRef1.SetTabDeleted( TRUE );
                        }
                        rChanged = TRUE;
                    }
                    rRef1.nRelTab = rRef1.nTab - nPosTab;
                }
                if ( t->GetType() == svDoubleRef )
                {
                    SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                    if ( !(rRef2.IsRelName() && rRef2.IsTabRel()) )
                    {   // Namen nur absolute anpassen
                        if ( rRef2.IsTabRel() )
                            nTab = rRef2.nRelTab + nOldPosTab;
                        else
                            nTab = rRef2.nTab;
                        if ( nTable < nTab )
                        {
                            rRef2.nTab = nTab - 1;
                            rChanged = TRUE;
                        }
                        else if ( nTable == nTab )
                        {
                            if ( !rRef1.IsTabDeleted() )
                                rRef2.nTab = nTab - 1;  // Bereich verkleinern
                            else
                            {
                                rRef2.nTab = MAXTAB+1;
                                rRef2.SetTabDeleted( TRUE );
                            }
                            rChanged = TRUE;
                        }
                        rRef2.nRelTab = rRef2.nTab - nPosTab;
                    }
                }
            }
        }
    }
    return pRangeData;
}

// aPos.Tab() muss bereits angepasst sein!
ScRangeData* ScCompiler::UpdateMoveTab( USHORT nOldTab, USHORT nNewTab,
        BOOL bIsName )
{
    ScRangeData* pRangeData = NULL;
    INT16 nTab;

    USHORT nStart, nEnd;
    short nDir;                         // Richtung, die die anderen wandern
    if ( nOldTab < nNewTab )
    {
        nDir = -1;
        nStart = nOldTab;
        nEnd = nNewTab;
    }
    else
    {
        nDir = 1;
        nStart = nNewTab;
        nEnd = nOldTab;
    }
    USHORT nPosTab = aPos.Tab();        // aktuelle Tabelle
    USHORT nOldPosTab;                  // vorher war's die
    if ( nPosTab == nNewTab )
        nOldPosTab = nOldTab;           // look, it's me!
    else if ( nPosTab < nStart || nEnd < nPosTab )
        nOldPosTab = nPosTab;           // wurde nicht bewegt
    else
        nOldPosTab = nPosTab - nDir;    // einen verschoben

    BOOL bIsRel = FALSE;
    ScToken* t;
    pArr->Reset();
    if (bIsName)
        t = pArr->GetNextReference();
    else
        t = pArr->GetNextReferenceOrName();
    while( t )
    {
        if( t->GetOpCode() == ocName )
        {
            if (!bIsName)
            {
                ScRangeData* pName = pDoc->GetRangeName()->FindIndex(t->GetIndex());
                if (pName && pName->HasType(RT_SHAREDMOD))
                    pRangeData = pName;
            }
        }
        else if( t->GetType() != svIndex )  // es kann ein DB-Bereich sein !!!
        {
            SingleRefData& rRef1 = t->GetSingleRef();
            if ( !(bIsName && rRef1.IsTabRel()) )
            {   // Namen nur absolute anpassen
                if ( rRef1.IsTabRel() )
                    nTab = rRef1.nRelTab + nOldPosTab;
                else
                    nTab = rRef1.nTab;
                if ( nTab == nOldTab )
                    rRef1.nTab = nNewTab;
                else if ( nStart <= nTab && nTab <= nEnd )
                    rRef1.nTab = nTab + nDir;
                rRef1.nRelTab = rRef1.nTab - nPosTab;
            }
            else
                bIsRel = TRUE;
            if ( t->GetType() == svDoubleRef )
            {
                SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                if ( !(bIsName && rRef2.IsTabRel()) )
                {   // Namen nur absolute anpassen
                    if ( rRef2.IsTabRel() )
                        nTab = rRef2.nRelTab + nOldPosTab;
                    else
                        nTab = rRef2.nTab;
                    if ( nTab == nOldTab )
                        rRef2.nTab = nNewTab;
                    else if ( nStart <= nTab && nTab <= nEnd )
                        rRef2.nTab = nTab + nDir;
                    rRef2.nRelTab = rRef2.nTab - nPosTab;
                }
                else
                    bIsRel = TRUE;
                INT16 nTab1, nTab2;
                if ( rRef1.IsTabRel() )
                    nTab1 = rRef1.nRelTab + nPosTab;
                else
                    nTab1 = rRef1.nTab;
                if ( rRef2.IsTabRel() )
                    nTab2 = rRef2.nRelTab + nPosTab;
                else
                    nTab2 = rRef1.nTab;
                if ( nTab2 < nTab1 )
                {   // PutInOrder
                    rRef1.nTab = nTab2;
                    rRef2.nTab = nTab1;
                    rRef1.nRelTab = rRef1.nTab - nPosTab;
                    rRef2.nRelTab = rRef2.nTab - nPosTab;
                }
            }
            if ( bIsName && bIsRel )
                pRangeData = (ScRangeData*) this;   // wird in rangenam nicht dereferenziert
        }
        if (bIsName)
            t = pArr->GetNextReference();
        else
            t = pArr->GetNextReferenceOrName();
    }
    if ( !bIsName )
    {
        short nMaxTabMod = (short) pDoc->GetTableCount();
        pArr->Reset();
        for ( t = pArr->GetNextReferenceRPN(); t;
              t = pArr->GetNextReferenceRPN() )
        {
            if ( t->GetRef() == 1 )
            {
                SingleRefData& rRef1 = t->GetSingleRef();
                if ( rRef1.IsRelName() && rRef1.IsTabRel() )
                {   // RelName evtl. wrappen, wie lcl_MoveItWrap in refupdat.cxx
                    nTab = rRef1.nRelTab + nPosTab;
                    if ( nTab < 0 )
                        nTab += nMaxTabMod;
                    else if ( nTab > nMaxTab )
                        nTab -= nMaxTabMod;
                    rRef1.nRelTab = nTab - nPosTab;
                }
                else
                {
                    if ( rRef1.IsTabRel() )
                        nTab = rRef1.nRelTab + nOldPosTab;
                    else
                        nTab = rRef1.nTab;
                    if ( nTab == nOldTab )
                        rRef1.nTab = nNewTab;
                    else if ( nStart <= nTab && nTab <= nEnd )
                        rRef1.nTab = nTab + nDir;
                    rRef1.nRelTab = rRef1.nTab - nPosTab;
                }
                if( t->GetType() == svDoubleRef )
                {
                    SingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                    if ( rRef2.IsRelName() && rRef2.IsTabRel() )
                    {   // RelName evtl. wrappen, wie lcl_MoveItWrap in refupdat.cxx
                        nTab = rRef2.nRelTab + nPosTab;
                        if ( nTab < 0 )
                            nTab += nMaxTabMod;
                        else if ( nTab > nMaxTab )
                            nTab -= nMaxTabMod;
                        rRef2.nRelTab = nTab - nPosTab;
                    }
                    else
                    {
                        if ( rRef2.IsTabRel() )
                            nTab = rRef2.nRelTab + nOldPosTab;
                        else
                            nTab = rRef2.nTab;
                        if ( nTab == nOldTab )
                            rRef2.nTab = nNewTab;
                        else if ( nStart <= nTab && nTab <= nEnd )
                            rRef2.nTab = nTab + nDir;
                        rRef2.nRelTab = rRef2.nTab - nPosTab;
                    }
                    INT16 nTab1, nTab2;
                    if ( rRef1.IsTabRel() )
                        nTab1 = rRef1.nRelTab + nPosTab;
                    else
                        nTab1 = rRef1.nTab;
                    if ( rRef2.IsTabRel() )
                        nTab2 = rRef2.nRelTab + nPosTab;
                    else
                        nTab2 = rRef1.nTab;
                    if ( nTab2 < nTab1 )
                    {   // PutInOrder
                        rRef1.nTab = nTab2;
                        rRef2.nTab = nTab1;
                        rRef1.nRelTab = rRef1.nTab - nPosTab;
                        rRef2.nRelTab = rRef2.nTab - nPosTab;
                    }
                }
            }
        }
    }
    return pRangeData;
}


ScToken* ScCompiler::CreateStringFromToken( String& rFormula, ScToken* pToken,
        BOOL bAllowArrAdvance )
{
    BOOL bNext = TRUE;
    BOOL bSpaces = FALSE;
    ScToken* t = pToken;
    OpCode eOp = t->GetOpCode();
    if( eOp >= ocAnd && eOp <= ocOr )
    {
        // AND, OR infix?
        if ( bAllowArrAdvance )
            t = pArr->Next();
        else
            t = pArr->PeekNext();
        bNext = FALSE;
        bSpaces = ( !t || t->GetOpCode() != ocOpen );
    }
    if( bSpaces )
        rFormula += ' ';

    if( eOp == ocSpaces )
        rFormula.Expand( rFormula.Len() + t->GetByte() );
    else if( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
        rFormula.AppendAscii( pInternal[ eOp - ocInternalBegin ] );
    else if( (USHORT) eOp < nAnzStrings)        // Keyword:
        rFormula += pSymbolTable[eOp];
    else
    {
        DBG_ERROR("Unbekannter OpCode");
        rFormula += ScGlobal::GetRscString(STR_NO_NAME_REF);
    }
    if( bNext ) switch( t->GetType() )
    {
        case svDouble:
        {
            String aStr;
            ULONG nIndex = ( pSymbolTable == pSymbolTableEnglish ?
                pDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US ) : 0 );
            pDoc->GetFormatTable()->
                    GetInputLineString(t->GetDouble(),nIndex,aStr);
            rFormula += aStr;
        }
        break;
        case svString:
            if( eOp == ocBad )
                rFormula += t->GetString();
            else
            {
                if (bImportXML)
                    rFormula += t->GetString();
                else
                {
                    rFormula += '"';
                    if ( ScGlobal::UnicodeStrChr( t->GetString().GetBuffer(), '"' ) == NULL )
                        rFormula += t->GetString();
                    else
                    {
                        String aStr( t->GetString() );
                        xub_StrLen nPos = 0;
                        while ( (nPos = aStr.Search( '"', nPos)) != STRING_NOTFOUND )
                        {
                            aStr.Insert( '"', nPos );
                            nPos += 2;
                        }
                        rFormula += aStr;
                    }
                    rFormula += '"';
                }
            }
            break;
        case svSingleRef:
        {
            SingleRefData& rRef = t->GetSingleRef();
            ComplRefData aRef;
            aRef.Ref1 = aRef.Ref2 = rRef;
            if ( eOp == ocColRowName )
            {
                rRef.CalcAbsIfRel( aPos );
                if ( pDoc->HasStringData( rRef.nCol, rRef.nRow, rRef.nTab ) )
                {
                    String aStr;
                    pDoc->GetString( rRef.nCol, rRef.nRow, rRef.nTab, aStr );
                    EnQuote( aStr );
                    rFormula += aStr;
                }
                else
                {
                    rFormula += ScGlobal::GetRscString(STR_NO_NAME_REF);
                    rFormula += MakeRefStr( aRef, TRUE );
                }
            }
            else
            {
                rFormula += MakeRefStr( aRef, TRUE );
            }
        }
            break;
        case svDoubleRef:
            rFormula += MakeRefStr( t->GetDoubleRef(), FALSE );
            break;
        case svIndex:
        {
            String aStr;
            switch ( eOp )
            {
                case ocName:
                {
                    ScRangeData* pData = pDoc->GetRangeName()->FindIndex(t->GetIndex());
                    if (pData)
                    {
                        if (pData->HasType(RT_SHARED))
                            pData->UpdateSymbol( aStr, aPos,
                                        pSymbolTable == pSymbolTableEnglish,
                                        bCompileXML );
                        else
                            pData->GetName(aStr);
                    }
                }
                break;
                case ocDBArea:
                {
                    ScDBData* pDBData = pDoc->GetDBCollection()->FindIndex(t->GetIndex());
                    if (pDBData)
                        pDBData->GetName(aStr);
                }
                break;
            }
            if ( !aStr.Len() )
                rFormula += ScGlobal::GetRscString(STR_NO_NAME_REF);
            else
                rFormula += aStr;
            break;
        }
        case svExternal:
        {
            //  show translated name of StarOne AddIns
            String aAddIn( t->GetExternal() );
            if ( pSymbolTable != pSymbolTableEnglish )
                ScGlobal::GetAddInCollection()->LocalizeString( aAddIn );
            rFormula += aAddIn;
        }
            break;
        case svByte:
        case svJump:
        case svMissing:
            break;      // Opcodes
        default:
            DBG_ERROR("ScCompiler:: GetStringFromToken errUnknownVariable");
    }                                           // of switch
    if( bSpaces )
        rFormula += ' ';
    if ( bAllowArrAdvance )
    {
        if( bNext )
            t = pArr->Next();
        return t;
    }
    return pToken;
}


void ScCompiler::CreateStringFromTokenArray( String& rFormula )
{
    rFormula.Erase();
    if( !pArr->GetLen() )
        return;
    if ( pArr->IsRecalcModeForced() )
        rFormula += '=';
    ScToken* t = pArr->First();
    while( t )
        t = CreateStringFromToken( rFormula, t, TRUE );
}


BOOL ScCompiler::EnQuote( String& rStr )
{
    sal_Int32 nType = ScGlobal::pCharClass->getStringType( rStr, 0, rStr.Len() );
    if ( !CharClass::isNumericType( nType )
            && CharClass::isAlphaNumericType( nType ) )
        return FALSE;
    xub_StrLen nPos = 0;
    while ( (nPos = rStr.Search( '\'', nPos)) != STRING_NOTFOUND )
    {
        rStr.Insert( '\\', nPos );
        nPos += 2;
    }
    rStr.Insert( '\'', 0 );
    rStr += '\'';
    return TRUE;
}


BOOL ScCompiler::DeQuote( String& rStr )
{
    xub_StrLen nLen = rStr.Len();
    if ( nLen > 1 && rStr.GetChar(0) == '\'' && rStr.GetChar( nLen-1 ) == '\'' )
    {
        rStr.Erase( nLen-1, 1 );
        rStr.Erase( 0, 1 );
        xub_StrLen nPos = 0;
        while ( (nPos = rStr.SearchAscii( "\\\'", nPos)) != STRING_NOTFOUND )
        {
            rStr.Erase( nPos, 1 );
            ++nPos;
        }
        return TRUE;
    }
    return FALSE;
}


