/*************************************************************************
 *
 *  $RCSfile: mathtype.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:51:22 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef MATHTYPE_HXX
#include <mathtype.hxx>
#endif

#ifndef _TOOLS_DEBUG_H
#include <tools/debug.hxx>
#endif

#define APPEND(str,ascii) str.AppendAscii(RTL_CONSTASCII_STRINGPARAM(ascii))

#if 0
String aEmbelList[21] =
{
    " ",
    " ",
    "single dot",
    "double dot",
    "triple dot",
    "single prime",
    "double prime",
    "backwards prime (left of character)",
    "tilde",
    "hat (circumflex)",
    "diagonal slash through character",
    "right arrow",
    "left arrow",
    "double-headed arrow",
    "right single-barbed arrow",
    "left single-barbed arrow",
    "mid-height horizontal bar",
    "over-bar",
    "triple prime",
    "over-arc, concave downward",
    "over-arc, concave upward"
};

String aSelectorList[49] =
{
    "angle brackets",
    "parentheses",
    "braces (curly brackets)",
    "square brackets",
    "vertical bars",
    "double vertical bars",
    "floor brackets",
    "ceiling brackets",
    "left brace, left brace",
    "right brace, right brace",
    "right brace, left brace",
    "left brace, right parenthesis",
    "left parenthesis, right brace",
    "radical",
    "fractions",
    "subscript/superscript",
    "underbar",
    "overbar",
    "left-pointing arrow",
    "right-pointing arrow",
    "left- and right-pointing arrow",
    "single integral",
    "double integral",
    "triple integral",
    "single summation-style integral",
    "double summation-style integral",
    "triple summation-style integral",
    "upper horizontal brace",
    "lower horizontal brace",
    "summation",
    "summation (integral-style limits)",
    "product",
    "product (integral-style limits)",
    "coproduct",
    "coproduct (integral-style limits)",
    "union",
    "union (integral-style limits)",
    "intersection",
    "intersection (integral-style limits)",
    "limit",
    "long division",
    "slash fractions",
    "big integral-style operators",
    "big summation-style operators",
    "leading sub- and superscripts",
    "Dirac delta",
    "under arrow",
    "over arrow",
    "over arc"
};

String aIntegralOpt[2] =
{
    "fixed-size integral",
    "integral expands vertically to fit its contents"
};

String aFenceOpt[3] =
{
    "center fence on math axis",
    "center fence on contents, place math axis of contents on math axis of containing line",
    "center fence on contents, center contents on math axis of containing line"
};

String aTypeFaces[12] =
{
    "",
    "fnTEXT",
    "fnFUNCTION",
    "fnVARIABLE",
    "fnLCGREEK",
    "fnUCGREEK",
    "fnSYMBOL",
    "fnVECTOR",
    "fnNUMBER",
    "fnUSER1",
    "fnUSER2",
    "fnMTEXTRA"
};

String aSizes[7] =
{
    "full",
    "subscript",
    "sub-subscript",
    "symbol",
    "sub-symbol",
    "user 1",
    "user 2"
};
#endif

static sal_Unicode Convert(sal_Unicode nIn)
{
    //Find the best match in accepted unicode for our private area symbols
    static sal_Unicode aStarMathPrivateToUnicode[] =
    {
0x2030, 0xF613, 0xF612, 0x002B, 0x003C, 0x003E, 0xE425, 0xE421, 0xE088, 0x2208,
0x0192, 0x2026, 0x2192, 0x221A, 0x221A, 0x221A, 0xE090, 0x005E, 0x02C7, 0x02D8,
0x00B4, 0x0060, 0x02DC, 0x00AF, 0x0362, 0xE099, 0xE09A, 0x20DB, 0xE09C, 0xE09D,
0x0028, 0x0029, 0x2220, 0x22AF, 0xE0A2, 0xE0A3, 0xE0A4, 0xE0A5, 0xE0A6, 0xE0A7,
0x002F, 0x005C, 0x274F, 0xE0AB, 0x0393, 0x0394, 0x0398, 0x039b, 0x039e, 0x03A0,
0x03a3, 0x03a5, 0x03a6, 0x03a8, 0x03A9, 0x03B1, 0x03B2, 0x03b3, 0x03b4, 0x03b5,
0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
0x03c0, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03b5,
0x03d1, 0x03d6, 0xE0D2, 0x03db, 0x2118, 0x2202, 0x2129, 0xE0D7, 0xE0D8, 0x22A4,
0xE0DA, 0x2190, 0x2191, 0x2193
    };
    if ((nIn >= 0xE080) && (nIn <= 0xE0DD))
        nIn = aStarMathPrivateToUnicode[nIn-0xE080];

    //For whatever unicode glyph that equation editor doesn't ship with that
    //we have a possible match we can munge it to.
    switch (nIn)
    {
        case 0x2223:
            nIn = '|';
            break;
        default:
            break;
    }

    return nIn;
}

void EQNOLEFILEHDR::Read(SvStorageStream *pS)
{
    *pS >> nCBHdr;
    *pS >> nVersion;
    *pS >> nCf;
    *pS >> nCBObject;
    *pS >> nReserved1;
    *pS >> nReserved2;
    *pS >> nReserved3;
    *pS >> nReserved4;
}

void EQNOLEFILEHDR::Write(SvStorageStream *pS)
{
    *pS << nCBHdr;
    *pS << nVersion;
    *pS << nCf;
    *pS << nCBObject;
    *pS << nReserved1;
    *pS << nReserved2;
    *pS << nReserved3;
    *pS << nReserved4;
}

void MathType::Init()
{
    //These are the default MathType sizes
    aSizeTable[0]=12;
    aSizeTable[1]=8;
    aSizeTable[2]=6;
    aSizeTable[3]=24;
    aSizeTable[4]=10;
    aSizeTable[5]=12;
    aSizeTable[6]=12;

    /*
    These are the default MathType italic/bold settings If mathtype is changed
    from its defaults, there is nothing we can do, as this information is not
    stored in the document
    */
    MathTypeFont aFont;
    for(sal_uInt8 i=1;i<=11;i++)
    {
        aFont.nTface = i+128;
        switch (i)
        {
            default:
                aFont.nStyle=0;
                break;
            case 3:
            case 4:
                aFont.nStyle=1;
                break;
            case 7:
                aFont.nStyle=2;
                break;
        }
        aUserStyles.insert(aFont);
    }
}


/*ToDo replace with table rather than switch, returns
 TRUE in the case that the char is just a char, and
 FALSE if the character is an operator which must not be
 placed inside the quote sequence designed to protect
 against being parsed as a keyword

 General solution required to force starmath to handle
 unicode math chars the way it handles its own math
 chars rathar than handle them as text as it will do
 for the default case below, i.e. incorrect spacing
 between math symbols and ordinary text e.g. 1=2 rather
 than 1 = 2
 */
sal_Bool MathType::LookupChar(sal_Unicode nChar,String &rRet,sal_uInt8 nVersion,
    sal_uInt8 nTypeFace)
{
    sal_Bool bRet=FALSE;
    const char *pC = NULL;
    switch(nChar)
    {
        case 0x0000:
            pC = " none ";
            break;
        case 0x00ac:
            pC = " neg ";
            break;
        case 0x00b1:
            pC = " +- ";
            break;
        case '(':
            pC = " \\( ";
            break;
        case ')':
            pC = " \\) ";
            break;
        case '[':
            pC = " \\[ ";
            break;
        case ']':
            pC = " \\] ";
            break;
        case '.':
            pC = " \".\" ";
            break;
        case 0xae:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " rightarrow ";
            else
            {
                rRet.Append(nChar);
                bRet=TRUE;
            }
            break;
        case 0x00fb:
            if ((nVersion < 3) && (nTypeFace == 0x81))
                nChar = 0xDF;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'a':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b1;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'b':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b2;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'l':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bb;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'n':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bd;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'r':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3c1;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 'D':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x394;
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 0xa9:
            if ((nVersion < 3) && (nTypeFace == 0x82))
                nChar = '\'';
            rRet.Append(nChar);
            bRet=TRUE;
            break;
        case 0x00f1:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " \\rangle ";
            else
            {
                rRet.Append(nChar);
                bRet=TRUE;
            }
            break;
        case 0x00a3:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " <= ";
            else
            {
                rRet.Append(nChar);
                bRet=TRUE;
            }
            break;
        case 0x00de:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " drarrow ";
            else
            {
                rRet.Append(nChar);
                bRet=TRUE;
            }
            break;
        case 0x0057:
            if ((nVersion < 3) && (nTypeFace == 0x85))
                pC = " %OMEGA ";
            else
            {
                rRet.Append(nChar);
                bRet=TRUE;
            }
            break;
        case 0x007b:
            pC = " lbrace ";
            break;
        case 0x007c:
            pC = " \\lline ";
            break;
        case 0x007d:
            pC = " rbrace ";
            break;
        case 0x007e:
            pC = " \"~\" ";
            break;
        case 0x2224:
            pC = " ndivides ";
            break;
        case 0x2225:
            pC = " parallel ";
            break;
        case 0x00d7:
            if (nVersion < 3)
                pC = " cdot ";
            else
                pC = " times ";
            break;
        case 0x00f7:
            pC = " div ";
            break;
        case 0x019b:
            pC = " lambdabar ";
            break;
        case 0x2026:
            pC = " dotslow ";
            break;
        case 0x2022:
            pC = " cdot ";
            break;
        case 0x2102:
            pC = " setC ";
            break;
        case 0x210f:
            pC = " hbar ";
            break;
        case 0x2111:
            pC = " Im ";
            break;
        case 0x2115:
            pC = " setN ";
            break;
        case 0x2118:
            pC = " wp ";
            break;
        case 0x211a:
            pC = " setQ ";
            break;
        case 0x211c:
            pC = " Re ";
            break;
        case 0x211d:
            pC = " setR ";
            break;
        case 0x2124:
            pC = " setZ ";
            break;
        case 0x2135:
            pC = " aleph ";
            break;
        case 0x2190:
            pC = " leftarrow ";
            break;
        case 0x2191:
            pC = " uparrow ";
            break;
        case 0x2192:
            pC = " rightarrow ";
            break;
        case 0x0362:
            pC = " widevec ";
            break;
        case 0x2193:
            pC = " downarrow ";
            break;
        case 0x21d0:
            pC = " dlarrow ";
            break;
        case 0x21d2:
            pC = " drarrow ";
            break;
        case 0x21d4:
            pC = " dlrarrow ";
            break;
        case 0x2200:
            pC = " forall ";
            break;
        case 0x2202:
            pC = " partial ";
            break;
        case 0x2203:
            pC = " exists ";
            break;
        case 0x2205:
            pC = " emptyset ";
            break;
        case 0x2207:
            pC = " nabla ";
            break;
        case 0x2208:
            pC = " in ";
            break;
        case 0x2209:
            pC = " notin ";
            break;
        case 0x220d:
            pC = " owns ";
            break;
        case 0x220f:
            pC = " prod ";
            break;
        case 0x2210:
            pC = " coprod ";
            break;
        case 0x2211:
            pC = " sum ";
            break;
        case 0x2212:
            pC = " - ";
            break;
        case 0x2213:
            pC = " -+ ";
            break;
        case 0x2217:
            pC = " * ";
            break;
        case 0x2218:
            pC = " circ ";
            break;
        case 0x221d:
            pC = " prop ";
            break;
        case 0x221e:
            pC = " infinity ";
            break;
        case 0x2227:
            pC = " and ";
            break;
        case 0x2228:
            pC = " or ";
            break;
        case 0x2229:
            pC = " intersection ";
            break;
        case 0x222a:
            pC = " union ";
            break;
        case 0x222b:
            pC = " int ";
            break;
        case 0x222c:
            pC = " iint ";
            break;
        case 0x222d:
            pC = " iiint ";
            break;
        case 0x222e:
            pC = " lint ";
            break;
        case 0x222f:
            pC = " llint ";
            break;
        case 0x2230:
            pC = " lllint ";
            break;
        case 0x2245:
            pC = " simeq ";
            break;
        case 0x2248:
            pC = " approx ";
            break;
        case 0x2260:
            pC = " <> ";
            break;
        case 0x2261:
            pC = " equiv ";
            break;
        case 0x2264:
            pC = " <= ";
            break;
        case 0x2265:
            pC = " >= ";
            break;
        case 0x2282:
            pC = " subset ";
            break;
        case 0x2283:
            pC = " supset ";
            break;
        case 0x2284:
            pC = " nsubset ";
            break;
        case 0x2285:
            pC = " nsupset ";
            break;
        case 0x2286:
            pC = " subseteq ";
            break;
        case 0x2287:
            pC = " supseteq ";
            break;
        case 0x2288:
            pC = " nsubseteq ";
            break;
        case 0x2289:
            pC = " nsupseteq ";
            break;
        case 0x227a:
        case 0x227b:
        case 0x22b2:
        case 0x22b3:
            rRet += ' ';
            rRet.Append(nChar);
            rRet += ' ';
            break;
        case 0x22a5:
            pC = " ortho ";
            break;
        case 0x22c5:
            pC = " cdot ";
            break;
        case 0x22ee:
            pC = " dotsvert ";
            break;
        case 0x22ef:
            pC = " dotsaxis ";
            break;
        case 0x22f0:
            pC = " dotsup ";
            break;
        case 0x22f1:
            pC = " dotsdown ";
            break;
        case 0x2329:
            pC = " langle ";
            break;
        case 0x232a:
            pC = " rangle ";
            break;
        case 0x301a:
            pC = " ldbracket ";
            break;
        case 0x301b:
            pC = " rdbracket ";
            break;
        case 0xe083:
            rRet.Append('+');
            bRet=TRUE;
            break;
        case '^':
        case 0xe091:
            pC = " widehat ";
            break;
        case 0xe096:
            pC = " widetilde ";
            break;
        case 0xe098:
            pC = " widevec ";
            break;
        case 0xE421:
            pC = " geslant ";
            break;
        case 0xE425:
            pC = " leslant ";
            break;
        case 0xeb01:    //no space
        case 0xeb08:    //normal space
            bRet=TRUE;
            break;
        case 0xef04:    //tiny space
        case 0xef05:    //tiny space
        case 0xeb02:    //small space
        case 0xeb04:    //medium space
            rRet.Append('`');
            break;
        case 0xeb05:    //large space
            rRet.Append('~');
            break;
        case 0x3a9:
            pC = " %OMEGA ";
            break;
        default:
            rRet.Append(nChar);
            bRet=TRUE;
            break;
    }
    if (pC)
        rRet.AppendAscii(pC);
    return bRet;
}

void MathTypeFont::AppendStyleToText(String &rRet)
{
    const char *pC = NULL;
    switch (nStyle)
    {
        default:
        case 0:
            break;
        case 1:
            pC = " ital ";
            break;
        case 2:
            pC = " bold ";
            break;
        case 3:
            pC = " bold italic";
            break;
    }
    if (pC)
        rRet.AppendAscii(pC);
}

void MathType::TypeFaceToString(String &rRet,sal_uInt8 nFace)
{
    MathTypeFont aFont(nFace);
    MathTypeFontSet::iterator aItr = aUserStyles.find(aFont);
    if (aItr != aUserStyles.end())
        aFont.nStyle = aItr->nStyle;
    aFont.AppendStyleToText(rRet);
}

int MathType::Parse(SvStorage *pStor)
{
    SvStorageStreamRef xSrc = pStor->OpenStream(
        String::CreateFromAscii("Equation Native"),
        STREAM_STD_READ | STREAM_NOCREATE);
    if ( (!xSrc.Is()) || (SVSTREAM_OK != xSrc->GetError()))
        return NULL;
    pS = &xSrc;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    EQNOLEFILEHDR aHdr;
    aHdr.Read(pS);
    *pS >> nVersion;
    *pS >> nPlatform;
    *pS >> nProduct;
    *pS >> nProdVersion;
    *pS >> nProdSubVersion;

#ifdef STANDALONE
    *pOut << "Format Version is " << int(nVersion) << endl;
    *pOut << "Generating Platform is " << (nPlatform ? "Windows"
        : "Mac") << endl;
    *pOut << "Generating Product is " << (nPlatform ? "Equation Editor"
        : "Equation Editor") << endl;
    *pOut << "Prod Version is " << int(nProdVersion) << "." <<
        int(nProdSubVersion) << endl << endl;
#endif

    int nRet = HandleRecords();
    //little crude hack to close ocassionally open expressions
    //a sophisticated system to determine what expressions are
    //opened is required, but this is as much work as rewriting
    //starmaths internals.
    APPEND(rRet,"{}");

#if OSL_DEBUG_LEVEL > 1
#   ifdef CAOLAN
    //sanity check

    //sigh, theres no point! MathType (in some bizarre subvarient) pads
    //the end of the formula with ENDs (0)'s
    ULONG nEnd = pS->Tell();
    DBG_ASSERT(nEnd == pS->Seek(STREAM_SEEK_TO_END),
        "Possibly unfully parsed formula")
#   endif
#endif
    return nRet;
}

static void lcl_PrependDummyTerm(String &rRet, xub_StrLen &rTextStart)
{
    if ((rRet.GetChar(rTextStart) == '=') &&
        ((rTextStart == 0) ||
        (rRet.GetChar(rTextStart-1) == '{'))
       )
    {
        rRet.InsertAscii(" {}",rTextStart);
        rTextStart+=3;
    }
}

static void lcl_AppendDummyTerm(String &rRet)
{
    sal_Bool bOk=sal_False;
    for(xub_StrLen nI=rRet.Len()-1;nI >= 0; nI--)
    {
        sal_Unicode nChar = rRet.GetChar(nI);
        if (nChar == ' ')
            continue;
        if (rRet.GetChar(nI) != '{')
            bOk=sal_True;
        break;
    }
    if (!bOk)   //No term, use dummy
        APPEND(rRet," {}");
}

/*Fabously complicated as many tokens have to be reordered and generally
 *moved around from mathtypes paradigm to starmaths.*/
int MathType::HandleRecords(int nLevel,sal_uInt8 nSelector,
    sal_uInt8 nVariation, int nMatrixRows,int nMatrixCols)
{
    sal_uInt8 nTag,nRecord;
    sal_uInt8 nTabType,nTabStops;
    sal_uInt16 nTabOffset;
    sal_Char nChar8;
    String sFontName;
    int i,nRet=1,newline=0;
    sal_Bool bSilent=FALSE;
    int nPart=0;
    String sPush,sMainTerm;
    int nSetSize=0,nSetAlign=0;
    int nCurRow=0,nCurCol=0;
    sal_Bool bOpenString=FALSE;
    xub_StrLen nTextStart;
    xub_StrLen nSubSupStartPos;
    xub_StrLen nLastTemplateBracket=STRING_NOTFOUND;

    do
    {
        *pS >> nTag;
        nRecord = nTag&0x0F;

        /*MathType strings can of course include words which
         *are StarMath keywords, the simplest solution is
         to escape strings of greater than len 1 with double
         quotes to avoid scanning the TokenTable for matches

         Unfortunately it may turn out that the string gets
         split during the handling of a character emblishment
         so this special case must be handled in the
         character handler case 2:
         */
        if ((nRecord == CHAR) && (!bSilent) && (!bOpenString))
        {
            bOpenString=TRUE;
            nTextStart = rRet.Len();
        }
        else if ((nRecord != CHAR) && (bOpenString))
        {
            bOpenString=FALSE;
            if ((rRet.Len() - nTextStart) > 1)
            {
                String aStr;
                TypeFaceToString(aStr,nTypeFace);
                aStr += '\"';
                rRet.Insert(aStr,nTextStart);
                rRet += '\"';
            }
            else
            {
                if (nRecord == END)
                {
                    sal_Unicode cChar = 0;
                    xub_StrLen nI = rRet.Len()-1;
                    while (nI && ((cChar = rRet.GetChar(nI)) == ' '))
                        --nI;
                    if ((cChar == '=') || (cChar == '+') || (cChar == '-'))
                        APPEND(rRet,"{}");
                }
            }
        }

        switch(nRecord)
        {
            case LINE:
                {
                    //if (xfLMOVE(nTag))
                    //if (xfLSPACE(nTag))
                    //if (xfRULER(nTag))

                    if (newline>0)
                        APPEND(rRet,"\nnewline\n");
                    if (!(xfNULL(nTag)))
                    {
                        switch (nSelector)
                        {
                        case 0x0:
                            if (nVariation==0)
                                APPEND(rRet," langle ");
                            else if (nVariation==1)
                                APPEND(rRet," \\langle ");
                            break;
                        case 0x1:
                            if (nVariation==0)
                                APPEND(rRet," left (");
                            else if (nVariation==1)
                                APPEND(rRet,"\\(");
                            break;
                        case 0x2:
                            if ((nVariation==0) || (nVariation==1))
                                APPEND(rRet," left lbrace ");
                            else
                                APPEND(rRet," left none ");
                            break;
                        case 0x3:
                            if (nVariation==0)
                                APPEND(rRet," left [");
                            else if (nVariation==1)
                                APPEND(rRet,"\\[");
                            break;
                        case 0x8:
                        case 0xb:
                            APPEND(rRet," \\[");
                            break;
                        case 0x4:
                            if (nVariation==0)
                                APPEND(rRet," lline ");
                            else if (nVariation==1)
                                APPEND(rRet," \\lline ");
                            break;
                        case 0x5:
                            if (nVariation==0)
                                APPEND(rRet," ldline ");
                            else if (nVariation==1)
                                APPEND(rRet," \\ldline ");
                            break;
                        case 0x6:
                            if (nVariation==0)
                                APPEND(rRet," lfloor ");
                            else if (nVariation==1)
                                APPEND(rRet," \\lfloor ");
                            break;
                        case 0x7:
                            if (nVariation==0)
                                APPEND(rRet," lceil ");
                            else if (nVariation==1)
                                APPEND(rRet," \\lceil ");
                            break;
                        case 0x9:
                        case 0xa:
                            APPEND(rRet," \\]");
                            break;
                        case 0xc:
                            APPEND(rRet," \\(");
                            break;
                        case 0xd:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," sqrt");
                                else
                                {
                                    APPEND(rRet," nroot");
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            APPEND(rRet," {");
                            break;
                        case 0xe:
                            if (nPart == 0)
                                APPEND(rRet," { ");


                            if (nPart == 1)
                                APPEND(rRet," over ");
                            APPEND(rRet," {");
                            break;
                        case 0xf:
                            nSubSupStartPos = rRet.Len();
                            if ((nVariation == 0) ||
                                    ((nVariation == 2) && (nPart==1)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                APPEND(rRet," rSup");
                            }
                            else if ((nVariation == 1) ||
                                    ((nVariation == 2) && (nPart==0)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                APPEND(rRet," rSub");
                            }
                            APPEND(rRet," {");
                            break;
                        case 0x10:
                            if (nVariation == 0)
                                APPEND(rRet," {underline ");
                            else if (nVariation == 1)
                                APPEND(rRet," {underline underline ");
                            APPEND(rRet," {");
                            break;
                        case 0x11:
                            if (nVariation == 0)
                                APPEND(rRet," {overline ");
                            else if (nVariation == 1)
                                APPEND(rRet," {overline overline ");
                            APPEND(rRet," {");
                            break;
                        case 0x12:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," widevec ");//left arrow above
                                else if (nVariation == 1)
                                    APPEND(rRet," widevec ");//left arrow below
                                APPEND(rRet," {");
                            }
                            break;
                        case 0x13:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," widevec ");//right arrow above
                                else if (nVariation == 1)
                                    APPEND(rRet," widevec ");//right arrow below
                                APPEND(rRet," {");
                            }
                            break;
                        case 0x14:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," widevec ");//double arrow above
                                else if (nVariation == 1)
                                    APPEND(rRet," widevec ");//double arrow below
                                APPEND(rRet," {");
                            }
                            break;
                        case 0x15:
                            if (nPart == 0)
                            {
                                if ((nVariation == 3) || (nVariation == 4))
                                    APPEND(rRet," lInt");
                                else
                                    APPEND(rRet," Int");
                                if ( (nVariation != 0) && (nVariation != 3))
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 4)) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 2) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 2) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x16:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    APPEND(rRet," llInt");
                                else
                                    APPEND(rRet," iInt");
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x17:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    APPEND(rRet," lllInt");
                                else
                                    APPEND(rRet," iiInt");
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x18:
                            if (nPart == 0)
                            {
                                if (nVariation == 2)
                                    APPEND(rRet," lInt");
                                else
                                    APPEND(rRet," Int");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 2)) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 0) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x19:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," llInt");
                                else
                                    APPEND(rRet," iInt");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if (nPart==1)
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x1a:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    APPEND(rRet," lllInt");
                                else
                                    APPEND(rRet," iiInt");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if (nPart==1)
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x1b:
                        case 0x1c:
                            APPEND(rRet," {");
                            break;
                        case 0x1d:
                            if (nPart == 0)
                            {
                                APPEND(rRet," Sum");
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x1e:
                            if (nPart == 0)
                            {
                                APPEND(rRet," Sum");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x1f:
                            if (nPart == 0)
                            {
                                APPEND(rRet," Prod");
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x20:
                            if (nPart == 0)
                            {
                                APPEND(rRet," Prod");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x21:
                            if (nPart == 0)
                            {
                                APPEND(rRet," coProd");
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x22:
                            if (nPart == 0)
                            {
                                APPEND(rRet," coProd");
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x23:
                            if (nPart == 0)
                            {
                                APPEND(rRet," union"); //union
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x24:
                            if (nPart == 0)
                            {
                                APPEND(rRet," union"); //union
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x25:
                            if (nPart == 0)
                            {
                                APPEND(rRet," intersect"); //intersect
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x26:
                            if (nPart == 0)
                            {
                                APPEND(rRet," intersect"); //intersect
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x27:
                            if ((nVariation == 0) && (nPart==1))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 2) && (nPart==1))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 2) && (nPart==2))
                                APPEND(rRet," cSup");
                            APPEND(rRet," {");
                            break;
                        case 0x28:
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sPush = rRet;
                                    rRet.Erase();
                                }
                            }
                            APPEND(rRet," {");
                            if (nVariation == 0)
                            {
                                if (nPart == 1)
                                    APPEND(rRet,"alignr ");
                            }
                            if (nPart == 0)
                                APPEND(rRet,"\\lline ");
                            if (nVariation == 1)
                                APPEND(rRet,"overline ");
                            break;
                        case 0x29:
                            APPEND(rRet," {");
                            break;
                        case 0x2a:
                            if (nPart == 0)
                            {
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 2) && (nPart==1))
                                APPEND(rRet," rSup");
                            else if ((nVariation == 1) && (nPart==0))
                                APPEND(rRet," rSub");
                            else if ((nVariation == 2) && (nPart==0))
                                APPEND(rRet," rSub");
                            APPEND(rRet," {");
                            break;
                        case 0x2b:
                            if (nPart == 0)
                            {
                                sPush = rRet;
                                rRet.Erase();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 2) && (nPart==1))
                                APPEND(rRet," cSup");
                            else if ((nVariation == 1) && (nPart==0))
                                APPEND(rRet," cSub");
                            else if ((nVariation == 2) && (nPart==0))
                                APPEND(rRet," cSub");
                            APPEND(rRet," {");
                            break;
                        case 0x2c:
                            if (nPart == 0)
                                APPEND(rRet,"\"\"");
                            if ((nVariation == 0)
                                    || ((nVariation == 2) && (nPart==1)))
                                APPEND(rRet," lSup");
                            else if ((nVariation == 1)
                                    || ((nVariation == 2) && (nPart==0)))
                                APPEND(rRet," lSub");
                            APPEND(rRet," {");
                            break;
                        case 0x2d:
                            if (nVariation==0)
                            {
                                if (nPart == 0)
                                    APPEND(rRet," langle ");
                            }
                            else if (nVariation==1)
                            {
                                APPEND(rRet," \\langle ");
                                newline--;
                            }
                            else if (nVariation==2)
                            {
                                APPEND(rRet," \\lline ");
                                newline--;
                            }
                            break;
                        case 0x2e:
                            if (nVariation == 0)
                                APPEND(rRet," widevec ");//left below
                            else if (nVariation == 1)
                                APPEND(rRet," widevec ");//right below
                            else if (nVariation == 2)
                                APPEND(rRet," widevec ");//double headed below
                            APPEND(rRet," {");
                            break;
                        case 0x2f:
                            if (nVariation == 0)
                                APPEND(rRet," widevec ");//left above
                            else if (nVariation == 1)
                                APPEND(rRet," widevec ");//right above
                            else if (nVariation == 2)
                                APPEND(rRet," widevec ");//double headed above
                            APPEND(rRet," {");
                            break;
                        default:
                            break;
                        }
                        int nOldCurSize=nCurSize;
                        xub_StrLen nSizeStartPos = rRet.Len();
                        HandleSize(nLSize,nDSize,nSetSize);
                        nRet = HandleRecords(nLevel+1);
                        while (nSetSize)
                        {
                            sal_Bool bOk=sal_False;
                            xub_StrLen nI = rRet.SearchBackward('{');
                            if (nI != STRING_NOTFOUND)
                            {
                                for(nI=nI+1;nI<rRet.Len();nI++)
                                    if (rRet.GetChar(nI) != ' ')
                                    {
                                        bOk=sal_True;
                                        break;
                                    }
                            }
                            else
                                bOk=sal_True;

                            if (bOk)
                                APPEND(rRet,"} ");
                            else
                                rRet.Erase(nSizeStartPos);
                            nSetSize--;
                            nCurSize=nOldCurSize;
                        }


                        HandleMatrixSeperator(nMatrixRows,nMatrixCols,
                            nCurCol,nCurRow);

                        switch (nSelector)
                        {
                        case 0x0:
                            if (nVariation==0)
                                APPEND(rRet," rangle ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rangle ");
                            break;
                        case 0x1:
                            if (nVariation==0)
                                APPEND(rRet," right )");
                            else if (nVariation==2)
                                APPEND(rRet,"\\)");
                            break;
                        case 0x2:
                            if ((nVariation==0) || (nVariation==2))
                                APPEND(rRet," right rbrace ");
                            else
                                APPEND(rRet," right none ");
                            break;
                        case 0x3:
                            if (nVariation==0)
                                APPEND(rRet," right ]");
                            else if (nVariation==2)
                                APPEND(rRet,"\\]");
                            break;
                        case 0x4:
                            if (nVariation==0)
                                APPEND(rRet," rline ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rline ");
                            break;
                        case 0x5:
                            if (nVariation==0)
                                APPEND(rRet," rdline ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rdline ");
                            break;
                        case 0x6:
                            if (nVariation==0)
                                APPEND(rRet," rfloor ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rfloor ");
                            break;
                        case 0x7:
                            if (nVariation==0)
                                APPEND(rRet," rceil ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rceil ");
                            break;
                        case 0x8:
                        case 0xa:
                            APPEND(rRet,"\\[");
                            break;
                        case 0x9:
                        case 0xc:
                            APPEND(rRet,"\\]");
                            break;
                        case 0xd:
                            APPEND(rRet,"} ");
                            if (nVariation == 1)
                            {
                                if (nPart == 0)
                                {
                                    newline--;
                                    sMainTerm = rRet;
                                    rRet.Erase();
                                }
                                else
                                {
                                    sPush += rRet;
                                    rRet = sPush;
                                    rRet += sMainTerm;
                                }
                            }
                            else
                            {
                                if (nPart == 0)
                                    newline--;
                            }
                            nPart++;
                            break;
                        case 0xb:
                            APPEND(rRet,"\\)");
                            break;
                        case 0xe:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                                newline--;
                            else
                                APPEND(rRet,"} ");
                            nPart++;
                            break;
                        case 0xf:
                            {
                            if ((nPart == 0) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;

                            sal_Bool bOk=sal_False;
                            xub_StrLen nI = rRet.SearchBackward('{');
                            if (nI != STRING_NOTFOUND)
                            {
                                for(nI=nI+1;nI<rRet.Len();nI++)
                                    if (rRet.GetChar(nI) != ' ')
                                    {
                                        bOk=sal_True;
                                        break;
                                    }
                            }
                            else
                                bOk=sal_True;

                            if (bOk)
                                APPEND(rRet,"} ");
                            else
                                rRet.Erase(nSubSupStartPos);
                            nPart++;
                            }
                            break;
                        case 0x2c:
                            if ((nPart == 0) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            APPEND(rRet,"} ");
                            nPart++;
                            break;
                        case 0x2e:
                        case 0x2f:
                            APPEND(rRet,"} ");
                            break;
                        case 0x10:
                        case 0x11:
                            APPEND(rRet,"}} ");
                            break;
                        case 0x12:
                        case 0x13:
                        case 0x14:
                            if (nPart == 0)
                            {
                                newline--;
                                APPEND(rRet,"} ");
                            }
                            nPart++;
                            break;
                        case 0x1b:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                newline--;
                                APPEND(rRet,"overbrace");
                            }
                            nPart++;
                            break;
                        case 0x1c:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                newline--;
                                APPEND(rRet,"underbrace");
                            }
                            nPart++;
                            break;
                        case 0x27:
                            if (nPart==0)
                                newline--;
                            else if ((nPart==1) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            APPEND(rRet,"} ");
                            nPart++;
                            break;
                        case 0x28:
                            APPEND(rRet,"} ");
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sMainTerm = rRet;
                                    rRet.Erase();
                                }
                                else
                                {
                                    sPush += rRet;
                                    rRet = sPush;
                                    APPEND(rRet," over ");
                                    rRet += sMainTerm;
                                }
                            }
                            if (nPart == 0)
                                newline--;
                            nPart++;
                            break;
                        case 0x29:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                newline--;
                                switch (nVariation)
                                {
                                case 1:
                                    APPEND(rRet,"slash");
                                    break;
                                default:
                                    APPEND(rRet,"wideslash");
                                    break;
                                }
                            }
                            nPart++;
                            break;
                        case 0x1d:
                        case 0x1e:
                        case 0x1f:
                        case 0x20:
                        case 0x21:
                        case 0x22:
                        case 0x23:
                        case 0x24:
                        case 0x25:
                        case 0x26:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                if (nVariation != 2)
                                {
                                    sMainTerm = rRet;
                                    rRet.Erase();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 0))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 1))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 1))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            nPart++;
                            break;
                        case 0x15:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 3))
                                {
                                    sMainTerm = rRet;
                                    rRet.Erase();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==4)))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 2))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 2))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            nPart++;
                            break;
                        case 0x16:
                        case 0x17:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 2))
                                {
                                    sMainTerm = rRet;
                                    rRet.Erase();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==3)))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            nPart++;
                            break;
                        case 0x18:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                sMainTerm = rRet;
                                rRet.Erase();
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==2)))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 0))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 0))
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            nPart++;
                            break;
                        case 0x19:
                        case 0x1a:
                            APPEND(rRet,"} ");
                            if (nPart == 0)
                            {
                                sMainTerm = rRet;
                                rRet.Erase();
                                newline--;
                            }
                            else if (nPart == 1)
                            {
                                sPush += rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                                newline--;
                            }
                            nPart++;
                            break;
                        case 0x2a:
                        case 0x2b:
                            APPEND(rRet,"} ");

                            if ((nPart == 0) &&
                                    ((nVariation == 0) || (nVariation == 1)))
                            {
                                sMainTerm = rRet;
                                rRet.Erase();
                                newline--;
                            }
                            else if ((nPart == 0) && (nVariation == 2))
                                newline--;
                            else if ((nPart == 1) && (nVariation == 2))
                            {
                                sMainTerm = rRet;
                                rRet.Erase();
                                newline--;
                            }
                            else if ((nPart == 2) || (((nPart == 1) &&
                                    (nVariation == 0) || (nVariation == 1))))
                            {
                                sPush+=rRet;
                                rRet = sPush;
                                rRet += sMainTerm;
                            }
                            nPart++;
                            break;
                        case 0x2d:
                            if (nVariation==0)
                            {
                                if (nPart == 0)
                                {
                                    newline--; //there is another term to arrive
                                    APPEND(rRet," mline ");
                                }
                                else
                                    APPEND(rRet," rangle ");
                            }
                            else if (nVariation==1)
                                APPEND(rRet," \\lline ");
                            else if (nVariation==2)
                                APPEND(rRet," \\rangle ");
                            nPart++;
                            break;
                        default:
                            break;
                        }
                        bSilent = TRUE; //Skip the optional brackets and/or
                                        //symbols that follow some of these
                                        //records. Foo Data.

                        /*In matrices and piles we cannot seperate equation
                         *lines with the newline keyword*/
                        if (nMatrixCols==0)
                            newline++;
                    }
                }
                break;
            case CHAR:
                nRet = HandleChar(nTextStart,nSetSize,nLevel,nTag,nSelector,
                    nVariation,bSilent);
                 break;
            case TMPL:
                nRet = HandleTemplate(nLevel,nSelector,nVariation,
                    nLastTemplateBracket);
                break;
            case PILE:
                nRet = HandlePile(nSetAlign,nLevel,nSelector,nVariation);
                HandleMatrixSeperator(nMatrixRows,nMatrixCols,nCurCol,nCurRow);
                break;
            case MATRIX:
                nRet = HandleMatrix(nLevel,nSelector,nVariation);
                HandleMatrixSeperator(nMatrixRows,nMatrixCols,nCurCol,nCurRow);
                break;
            case EMBEL:
                HandleEmblishments();
                break;
            case RULER:
                *pS >> nTabStops;
                for (i=0;i<nTabStops;i++)
                {
                    *pS >> nTabType;
                    *pS >> nTabOffset;
                }
                DBG_ASSERT(FALSE,"Not seen in the wild Equation Ruler Field")
                break;
            case FONT:
                {
                    MathTypeFont aFont;
                    *pS >> aFont.nTface;
                    /*
                    The typeface number is the negative (which makes it
                    positive) of the typeface value (unbiased) that appears in
                    CHAR records that might follow a given FONT record
                    */
                    aFont.nTface = 128-aFont.nTface;
                    *pS >> aFont.nStyle;
                    aUserStyles.insert(aFont);
                    sFontName.Erase();
                    do
                    {
                        *pS >> nChar8;
                        sFontName.Append(ByteString::ConvertToUnicode(
                            nChar8,RTL_TEXTENCODING_MS_1252));
                    }
                    while(nChar8);
                }
                break;
            case SIZE:
                HandleSetSize();
                break;
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
                nLSize=nRecord-10;
                break;
            case END:
            default:
                break;
        }
    }
    while (nRecord != END);
    while (nSetSize)
    {
        rRet += '}';
        nSetSize--;
    }
    return nRet;
}

/*Simply determine if we are at the end of a record or the end of a line,
 *with fiddley logic to see if we are in a matrix or a pile or neither

 Note we cannot tell until after the event that this is the last entry
 of a pile, so we must strip the last seperator of a pile after this
 is detected in the PILE handler
 */
void MathType::HandleMatrixSeperator(int nMatrixRows,int nMatrixCols,
    int &rCurCol,int &rCurRow)
{
    if (nMatrixRows!=0)
    {
        if (rCurCol == nMatrixCols-1)
        {
            if (rCurRow != nMatrixRows-1)
                APPEND(rRet," {} ##\n");
            if (nMatrixRows!=-1)
            {
                rCurCol=0;
                rCurRow++;
            }
        }
        else
        {
            APPEND(rRet," {} # ");
            if (nMatrixRows!=-1)
                rCurCol++;
            else
                rRet += '\n';
        }
    }
}

/* set the alignment of the following term, but starmath currently
 * cannot handle vertical alignment */
void MathType::HandleAlign(sal_uInt8 nHAlign,sal_uInt8 nVAlign,int &rSetAlign)
{
    switch(nHAlign)
    {
    case 1:
    default:
        APPEND(rRet,"alignl {");
        break;
    case 2:
        APPEND(rRet,"alignc {");
        break;
    case 3:
        APPEND(rRet,"alignr {");
        break;
    }
#if 0
    switch(nVAlign)
    {
    }
    rSetAlign+=2;
#endif
    rSetAlign++;
}

/* set size of text, complexity due to overuse of signedness as a flag
 * indicator by mathtype file format*/
sal_Bool MathType::HandleSize(sal_Int16 nLSize,sal_Int16 nDSize, int &rSetSize)
{
    sal_Bool bRet=FALSE;
    if (nLSize < 0)
    {
        if ((-nLSize/32 != nDefaultSize) && (-nLSize/32 != nCurSize))
        {
            if (rSetSize)
            {
                rSetSize--;
                rRet += '}';
                bRet=TRUE;
            }
            if (-nLSize/32 != nLastSize)
            {
                nLastSize = nCurSize;
                APPEND(rRet," size ");
                rRet += String::CreateFromInt32(-nLSize/32);
                rRet += '{';
                bRet=TRUE;
                rSetSize++;
            }
            nCurSize = -nLSize/32;
        }
    }
    else
    {
        /*sizetable should theoreticaly be filled with the default sizes
         *of the various font groupings matching starmaths equivalents
         in aTypeFaces, and a test would be done to see if the new font
         size would be the same as what starmath would have chosen for
         itself anyway in which case the size setting could be ignored*/
        nLSize = aSizeTable[nLSize];
        nLSize += nDSize;
        //if (nLSize != nDefaultSize)
        if (nLSize != nCurSize)
        {
            if (rSetSize)
            {
                rSetSize--;
                rRet += '}';
                bRet=TRUE;
            }
            if (nLSize != nLastSize)
            {
                nLastSize = nCurSize;
                APPEND(rRet," size ");
                rRet += String::CreateFromInt32(nLSize);
                rRet += '{';
                bRet=TRUE;
                rSetSize++;
            }
            nCurSize = nLSize;
        }
    }
    return bRet;
}

int MathType::ConvertFromStarMath(SvStorage *pStor)
{
    if (!pTree)
        return 0;

    SvGlobalName aGName(0x0002ce02L, 0x0000, 0x0000,0xc0,0x00,
        0x00,0x00,0x00,0x00,0x00,0x46 );
    pStor->SetClass( aGName, 0, C2S("Microsoft Equation 3.0"));

    static sal_uInt8 __READONLY_DATA aCompObj[] = {
        0x01, 0x00, 0xFE, 0xFF, 0x03, 0x0A, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF, 0x02, 0xCE, 0x02, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x46, 0x17, 0x00, 0x00, 0x00,
        0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66,
        0x74, 0x20, 0x45, 0x71, 0x75, 0x61, 0x74, 0x69,
        0x6F, 0x6E, 0x20, 0x33, 0x2E, 0x30, 0x00, 0x0C,
        0x00, 0x00, 0x00, 0x44, 0x53, 0x20, 0x45, 0x71,
        0x75, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x00, 0x0B,
        0x00, 0x00, 0x00, 0x45, 0x71, 0x75, 0x61, 0x74,
        0x69, 0x6F, 0x6E, 0x2E, 0x33, 0x00, 0xF4, 0x39,
        0xB2, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    SvStorageStreamRef xStor( pStor->OpenStream( C2S("\1CompObj")));
    xStor->Write(aCompObj,sizeof(aCompObj));

    static sal_uInt8 __READONLY_DATA aOle[] = {
        0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        };
    SvStorageStreamRef xStor2( pStor->OpenStream( C2S("\1Ole")));
    xStor2->Write(aOle,sizeof(aOle));
    xStor.Clear();
    xStor2.Clear();

    SvStorageStreamRef xSrc = pStor->OpenStream(C2S("Equation Native"));
    if ( (!xSrc.Is()) || (SVSTREAM_OK != xSrc->GetError()))
        return NULL;

    pS = &xSrc;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    pS->SeekRel(EQNOLEFILEHDR_SIZE); //Skip 28byte Header and fill it in later
    *pS << sal_uInt8(0x03);
    *pS << sal_uInt8(0x01);
    *pS << sal_uInt8(0x01);
    *pS << sal_uInt8(0x03);
    *pS << sal_uInt8(0x00);
    sal_uInt32 nSize = pS->Tell();
    nPendingAttributes=0;

    HandleNodes(pTree);
    *pS << sal_uInt8(END);

    nSize = pS->Tell()-nSize;
    pS->Seek(0);
    EQNOLEFILEHDR aHdr(nSize+4+1);
    aHdr.Write(pS);
    return 1;
}


sal_uInt8 MathType::HandleNodes(SmNode *pNode,int nLevel)
{
    sal_Bool bRet=FALSE;
    switch(pNode->GetType())
    {
        case NATTRIBUT:
            HandleAttributes(pNode,nLevel);
            break;
        case NTEXT:
            HandleText(pNode,nLevel);
            break;
        case NVERTICAL_BRACE:
            HandleVerticalBrace(pNode,nLevel);
            break;
        case NBRACE:
            HandleBrace(pNode,nLevel);
            break;
        case NOPER:
            HandleOperator(pNode,nLevel);
            break;
        case NBINVER:
            HandleFractions(pNode,nLevel);
            break;
        case NROOT:
            HandleRoot(pNode,nLevel);
            break;
        case NSPECIAL:
            {
            SmTextNode *pText=(SmTextNode *)pNode;
            //if the token str and the result text are the same then this
            //is to be seen as text, else assume its a mathchar
            if (pText->GetText() == pText->GetToken().aText)
                HandleText(pText,nLevel);
            else
                HandleMath(pText,nLevel);
            }
            break;
        case NMATH:
            HandleMath(pNode,nLevel);
            break;
        case NSUBSUP:
            HandleSubSupScript(pNode,nLevel);
            break;
        case NEXPRESSION:
            {
            USHORT  nSize = pNode->GetNumSubNodes();
            for (USHORT i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
        case NTABLE:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode,nLevel);
            break;
        case NMATRIX:
            HandleSmMatrix((SmMatrixNode *)pNode,nLevel);
            break;
        case NLINE:
            {
            *pS << sal_uInt8(0x0a);
            *pS << sal_uInt8(LINE);
            USHORT  nSize = pNode->GetNumSubNodes();
            for (USHORT i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END);
            }
            break;
        case NALIGN:
            HandleMAlign(pNode,nLevel);
            break;
        case NBLANK:
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x98);
            if (pNode->GetToken().eType == TSBLANK)
                *pS << sal_uInt16(0xEB04);
            else
                *pS << sal_uInt16(0xEB05);
            break;

            break;
        default:
            {
            USHORT  nSize = pNode->GetNumSubNodes();
            for (USHORT i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
    }
    return bRet;
}


int MathType::StartTemplate(sal_uInt16 nSelector,sal_uInt16 nVariation)
{
    int nOldPending=nPendingAttributes;
    *pS << sal_uInt8(TMPL); //Template
    *pS << sal_uInt8(nSelector); //selector
    *pS << sal_uInt8(nVariation); //variation
    *pS << sal_uInt8(0x00); //options
    *pS << sal_uInt8(LINE);
    //theres just no way we can now handle any character
    //attributes (from mathtypes perspective) centered
    //over an expression but above template attribute
    //such as widevec and similiar constructs
    //we have to drop them
    nPendingAttributes=0;
    return nOldPending;
}

void MathType::EndTemplate(int nOldPendingAttributes)
{
    *pS << sal_uInt8(END); //end line
    *pS << sal_uInt8(END); //end template
    nPendingAttributes=nOldPendingAttributes;
}


void MathType::HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel)
{
    *pS << sal_uInt8(MATRIX);
    *pS << sal_uInt8(0x00); //vAlign ?
    *pS << sal_uInt8(0x00); //h_just
    *pS << sal_uInt8(0x00); //v_just
    *pS << sal_uInt8(pMatrix->GetNumRows()); //v_just
    *pS << sal_uInt8(pMatrix->GetNumCols()); //v_just
    int nBytes=(pMatrix->GetNumRows()+1)*2/8;
    if (((pMatrix->GetNumRows()+1)*2)%8)
        nBytes++;
    for (USHORT j = 0; j < nBytes; j++)
        *pS << sal_uInt8(0x00); //row_parts
    nBytes=(pMatrix->GetNumCols()+1)*2/8;
    if (((pMatrix->GetNumCols()+1)*2)%8)
        nBytes++;
    for (USHORT k = 0; k < nBytes; k++)
        *pS << sal_uInt8(0x00); //col_parts
    USHORT  nSize = pMatrix->GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (SmNode *pTemp = pMatrix->GetSubNode(i))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //end line
        }
    *pS << sal_uInt8(END);
}


//Root Node, PILE equivalent, i.e. vertical stack
void MathType::HandleTable(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();
    //The root of the starmath is a table, if
    //we convert this them each iteration of
    //conversion from starmath to mathtype will
    //add an extra unnecessary level to the
    //mathtype output stack which would grow
    //without bound in a multi step conversion

    if (nLevel == 0)
        *pS << sal_uInt8(0x0A); //initial size

    if ( nLevel || (nSize >1))
    {
        *pS << sal_uInt8(PILE);
        *pS << sal_uInt8(nHAlign); //vAlign ?
        *pS << sal_uInt8(0x01); //hAlign
    }

    for (USHORT i = 0; i < nSize; i++)
        if (SmNode *pTemp = pNode->GetSubNode(i))
        {
            *pS << sal_uInt8(LINE);
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END);
        }
    if (nLevel || (nSize>1))
        *pS << sal_uInt8(END);
}


void MathType::HandleRoot(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();
    SmNode *pTemp;
    *pS << sal_uInt8(TMPL); //Template
    *pS << sal_uInt8(0x0D); //selector
    if (pNode->GetSubNode(0))
        *pS << sal_uInt8(0x01); //variation
    else
        *pS << sal_uInt8(0x00); //variation
    *pS << sal_uInt8(0x00); //options

    /*
    if (pTemp = pNode->GetSubNode(1))
            HandleNodes(pTemp,nLevel+1);
    */
    if (pTemp = pNode->GetSubNode(2))
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pTemp,nLevel+1);
        *pS << sal_uInt8(END);
    }

    if (pTemp = pNode->GetSubNode(0))
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pTemp,nLevel+1);
        *pS << sal_uInt8(END);
    }
    else
        *pS << sal_uInt8(LINE|0x10); //dummy line



    *pS << sal_uInt8(END);
}

sal_uInt8 MathType::HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,
    ULONG *pPos,sal_Bool bTest)
{
    sal_uInt8 nVariation2=0xff;

    if (bTest && pNode->GetSubNode(CSUP+1))
    {
        nVariation2=0;
        if (pNode->GetSubNode(CSUB+1))
            nVariation2=2;
    }
    else if (pNode->GetSubNode(CSUB+1))
        nVariation2=1;

    if (nVariation2!=0xff)
    {
        if (pPos)
            *pPos = pS->Tell();
        *pS << sal_uInt8(TMPL); //Template
        *pS << sal_uInt8(0x2B); //selector
        *pS << nVariation2;
        *pS << sal_uInt8(0x00); //options

        if (pContent)
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pContent,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);

        *pS << sal_uInt8(0x0B);

        SmNode *pTemp;
        if (pTemp = pNode->GetSubNode(CSUB+1))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
        if (bTest && (pTemp = pNode->GetSubNode(CSUP+1)))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
    }
    return nVariation2;
}



/*
 Sub and Sup scripts and another problem area, StarMath
 can have all possible options used at the same time, whereas
 Mathtype cannot. The ordering of the nodes for each system
 is quite different as well leading to some complexity
 */
void MathType::HandleSubSupScript(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();
    SmNode *pTemp;

    sal_uInt8 nVariation=0xff;
    if (pNode->GetSubNode(LSUP+1))
    {
        nVariation=0;
        if (pNode->GetSubNode(LSUB+1))
            nVariation=2;
    }
    else if (pTemp = pNode->GetSubNode(LSUB+1))
        nVariation=1;

    if (nVariation!=0xff)
    {
        *pS << sal_uInt8(TMPL); //Template
        *pS << sal_uInt8(0x2c); //selector
        *pS << nVariation;
        *pS << sal_uInt8(0x00); //options
        *pS << sal_uInt8(0x0B);

        if (pTemp = pNode->GetSubNode(LSUB+1))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
        if (pTemp = pNode->GetSubNode(LSUP+1))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
        *pS << sal_uInt8(END);
        nVariation=0xff;
    }


    sal_uInt8 nVariation2=HandleCScript(pNode,NULL,nLevel);

    if (pTemp = pNode->GetSubNode(0))
    {
//      *pS << sal_uInt8(0x0A);
//      *pS << sal_uInt8(LINE);
        HandleNodes(pTemp,nLevel+1);
//      *pS << sal_uInt8(END);
    }

    if (nVariation2 != 0xff)
        *pS << sal_uInt8(END);

    if (pNode->GetSubNode(RSUP+1))
    {
        nVariation=0;
        if (pNode->GetSubNode(RSUB+1))
            nVariation=2;
    }
    else if (pTemp = pNode->GetSubNode(RSUB+1))
        nVariation=1;

    if (nVariation!=0xff)
    {
        *pS << sal_uInt8(TMPL); //Template
        *pS << sal_uInt8(0x0F); //selector
        *pS << nVariation;
        *pS << sal_uInt8(0x00); //options
        *pS << sal_uInt8(0x0B);

        if (pTemp = pNode->GetSubNode(RSUB+1))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
        if (pTemp = pNode->GetSubNode(RSUP+1))
        {
            *pS << sal_uInt8(LINE); //line
            HandleNodes(pTemp,nLevel+1);
            *pS << sal_uInt8(END); //line
        }
        else
            *pS << sal_uInt8(LINE|0x10);
    *pS << sal_uInt8(END); //line
    }

    //After subscript mathtype will keep the size of
    //normal text at the subscript size, sigh.
    *pS << sal_uInt8(0x0A);
}


void MathType::HandleFractions(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();
    SmNode *pTemp;
    *pS << sal_uInt8(TMPL); //Template
    *pS << sal_uInt8(0x0E); //selector
    *pS << sal_uInt8(0x00); //variation
    *pS << sal_uInt8(0x00); //options

    *pS << sal_uInt8(0x0A);
    *pS << sal_uInt8(LINE); //line
    if (pTemp = pNode->GetSubNode(0))
        HandleNodes(pTemp,nLevel+1);
    *pS << sal_uInt8(END);

    *pS << sal_uInt8(0x0A);
    *pS << sal_uInt8(LINE); //line
    if (pTemp = pNode->GetSubNode(2))
        HandleNodes(pTemp,nLevel+1);
    *pS << sal_uInt8(END);

    *pS << sal_uInt8(END);
}


void MathType::HandleBrace(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();
    SmNode *pTemp;
    SmNode *pLeft=pNode->GetSubNode(0);
    SmNode *pRight=pNode->GetSubNode(2);

    *pS << sal_uInt8(TMPL); //Template
    bReInterpBrace=0;
    sal_uInt8 nBSpec=0x10;
    ULONG nLoc = pS->Tell();
    if (pLeft)
    {
        switch (pLeft->GetToken().eType)
        {
            case TLANGLE:
                *pS << sal_uInt8(tmANGLE); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                break;
            case TLBRACE:
                *pS << sal_uInt8(tmBRACE); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                nBSpec+=3;
                break;
            case TLBRACKET:
                *pS << sal_uInt8(tmBRACK); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                nBSpec+=3;
                break;
            case TLLINE:
                *pS << sal_uInt8(tmBAR); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                nBSpec+=3;
                break;
            case TLDLINE:
                *pS << sal_uInt8(tmDBAR); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                break;
            default:
                *pS << sal_uInt8(tmPAREN); //selector
                *pS << sal_uInt8(0x00); //variation
                *pS << sal_uInt8(0x00); //options
                nBSpec+=3;
                break;
        }
    }

    if (pTemp = pNode->GetSubNode(1))
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pTemp,nLevel+1);
        *pS << sal_uInt8(END); //options
    }
    nSpec=nBSpec;
    if (pLeft)
        HandleNodes(pLeft,nLevel+1);
    if (bReInterpBrace)
    {
        ULONG nLoc2 = pS->Tell();
        pS->Seek(nLoc);
        *pS << sal_uInt8(0x2D);
        pS->Seek(nLoc2);
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x96);
        *pS << sal_uInt16(0xEC07);
        bReInterpBrace=0;
    }
    if (pRight)
        HandleNodes(pRight,nLevel+1);
    nSpec=0x0;
    *pS << sal_uInt8(END);
}


void MathType::HandleVerticalBrace(SmNode *pNode,int nLevel)
{
    SmNode *pTemp;
    *pS << sal_uInt8(TMPL); //Template
    if (pNode->GetToken().eType == TUNDERBRACE)
        *pS << sal_uInt8(tmLHBRACE); //selector
    else
        *pS << sal_uInt8(tmUHBRACE); //selector
    *pS << sal_uInt8(0x01); //variation
    *pS << sal_uInt8(0x00); //options

    if (pTemp = pNode->GetSubNode(0))
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pTemp,nLevel+1);
        *pS << sal_uInt8(END); //options
    }

    if (pTemp = pNode->GetSubNode(2))
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pTemp,nLevel+1);
        *pS << sal_uInt8(END); //options
    }
    *pS << sal_uInt8(END);
}

void MathType::HandleOperator(SmNode *pNode,int nLevel)
{
    USHORT  nSize = pNode->GetNumSubNodes();

    if (HandleLim(pNode,nLevel))
        return;

    ULONG nPos;
    sal_uInt8 nVariation;

    switch (pNode->GetToken().eType)
    {
        case TIINT:
        case TIIINT:
        case TLINT:
        case TLLINT:
        case TLLLINT:
            nVariation=HandleCScript(pNode->GetSubNode(0),
                pNode->GetSubNode(1),nLevel,&nPos,0);
            break;
        default:
            nVariation=HandleCScript(pNode->GetSubNode(0),
                pNode->GetSubNode(1),nLevel,&nPos);
            break;
    }

    sal_uInt8 nOldVariation=nVariation;
    sal_uInt8 nIntVariation=nVariation;

    ULONG nPos2=0;
    if (nVariation != 0xff)
    {
        nPos2 = pS->Tell();
        pS->Seek(nPos);
        if (nVariation == 2)
        {
            nIntVariation=0;
            nVariation = 1;
        }
        else if (nVariation == 0)
            nVariation = 1;
        else if (nVariation == 1)
            nVariation = 0;
    }
    else
    {
        nVariation = 2;
        nIntVariation=0;
    }
    *pS << sal_uInt8(TMPL);
    switch(pNode->GetToken().eType)
    {
    case TINT:
        if (nOldVariation != 0xff)
            *pS << sal_uInt8(0x18); //selector
        else
            *pS << sal_uInt8(0x15); //selector
        *pS << nIntVariation; //variation
        break;
    case TIINT:
        if (nOldVariation != 0xff)
        {
            *pS << sal_uInt8(0x19);
            *pS << sal_uInt8(0x01);
        }
        else
        {
            *pS << sal_uInt8(0x16);
            *pS << sal_uInt8(0x00);
        }
        break;
    case TIIINT:
        if (nOldVariation != 0xff)
        {
            *pS << sal_uInt8(0x1a);
            *pS << sal_uInt8(0x01);
        }
        else
        {
            *pS << sal_uInt8(0x17);
            *pS << sal_uInt8(0x00);
        }
        break;
    case TLINT:
        if (nOldVariation != 0xff)
        {
            *pS << sal_uInt8(0x18);
            *pS << sal_uInt8(0x02);
        }
        else
        {
            *pS << sal_uInt8(0x15);
            *pS << sal_uInt8(0x03);
        }
        break;
    case TLLINT:
        if (nOldVariation != 0xff)
        {
            *pS << sal_uInt8(0x19);
            *pS << sal_uInt8(0x00);
        }
        else
        {
            *pS << sal_uInt8(0x16);
            *pS << sal_uInt8(0x02);
        }
        break;
    case TLLLINT:
        if (nOldVariation != 0xff)
        {
            *pS << sal_uInt8(0x1a);
            *pS << sal_uInt8(0x00);
        }
        else
        {
            *pS << sal_uInt8(0x17);
            *pS << sal_uInt8(0x02);
        }
        break;
    case TSUM:
    default:
        *pS << sal_uInt8(0x1d);
        *pS << nVariation;
        break;
    case TPROD:
        *pS << sal_uInt8(0x1f);
        *pS << nVariation;
        break;
    case TCOPROD:
        *pS << sal_uInt8(0x21);
        *pS << nVariation;
        break;
    }
    *pS << sal_uInt8(0x00); //options

    if (nPos2)
        pS->Seek(nPos2);
    else
    {
        *pS << sal_uInt8(LINE); //line
        HandleNodes(pNode->GetSubNode(1),nLevel+1);
        *pS << sal_uInt8(END); //line
        *pS << sal_uInt8(LINE|0x10);
        *pS << sal_uInt8(LINE|0x10);
    }


    *pS << sal_uInt8(0x0D);
    switch(pNode->GetToken().eType)
    {
    case TSUM:
    default:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x86);
        *pS << sal_uInt16(0x2211);
        break;
    case TPROD:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x86);
        *pS << sal_uInt16(0x220F);
        break;
    case TCOPROD:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x8B);
        *pS << sal_uInt16(0x2210);
        break;
    case TIIINT:
    case TLLLINT:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x86);
        *pS << sal_uInt16(0x222B);
    case TIINT:
    case TLLINT:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x86);
        *pS << sal_uInt16(0x222B);
    case TINT:
    case TLINT:
        *pS << sal_uInt8(CHAR);
        *pS << sal_uInt8(0x86);
        *pS << sal_uInt16(0x222B);
        break;
    }
    *pS << sal_uInt8(END);
    *pS << sal_uInt8(0x0A);
}


int MathType::HandlePile(int &rSetAlign,int nLevel,sal_uInt8 nSelector,
    sal_uInt8 nVariation)
{
    //if (xfLMOVE(nTag))
    *pS >> nHAlign;
    *pS >> nVAlign;

    HandleAlign(nHAlign,nVAlign,rSetAlign);

    APPEND(rRet," stack {\n");
    int nRet = HandleRecords(nLevel+1,nSelector,nVariation,-1,-1);
    rRet.Erase(rRet.Len()-3,2);
    APPEND(rRet,"} ");

    while (rSetAlign)
    {
        APPEND(rRet,"} ");
        rSetAlign--;
    }
    return nRet;
}

int MathType::HandleMatrix(int nLevel,sal_uInt8 nSelector,
    sal_uInt8 nVariation)
{
    sal_uInt8 nH_just,nV_just,nRows,nCols;
    //if (xfLMOVE(nTag))
    *pS >> nVAlign;
    *pS >> nH_just;
    *pS >> nV_just;
    *pS >> nRows;
    *pS >> nCols;
    int nBytes = ((nRows+1)*2)/8;
    if (((nRows+1)*2)%8)
        nBytes++;
    pS->SeekRel(nBytes);
    nBytes = ((nCols+1)*2)/8;
    if (((nCols+1)*2)%8)
        nBytes++;
    pS->SeekRel(nBytes);
    APPEND(rRet," matrix {\n");
    int nRet = HandleRecords(nLevel+1,nSelector,nVariation,nRows,nCols);

    xub_StrLen nI = rRet.SearchBackward('#');
    if ((nI != STRING_NOTFOUND) && (nI > 0))
        if (rRet.GetChar(nI-1) != '#')  //missing column
            APPEND(rRet,"{}");

    APPEND(rRet,"\n} ");
    return nRet;
}

int MathType::HandleTemplate(int nLevel,sal_uInt8 &rSelector,
    sal_uInt8 &rVariation, xub_StrLen &rLastTemplateBracket)
{
    sal_uInt8 nOption; //This appears utterly unused
    //if (xfLMOVE(nTag))
    *pS >> rSelector;
    *pS >> rVariation;
    *pS >> nOption;
    DBG_ASSERT(rSelector < 48,"Selector out of range")
    if ((rSelector >= 21) && (rSelector <=26))
    {
        DBG_ASSERT(nOption < 2,"Option out of range");
    }
    else if ((rSelector >= 0) && (rSelector <=12))
    {
        DBG_ASSERT(nOption < 3,"Option out of range");
    }

    //For the (broken) case where one subscript template ends, and there is
    //another one after it, mathtype handles it as if the second one was
    //inside the first one and renders it as sub of sub
    BOOL bRemove=FALSE;
    if ( (rSelector == 0xf) && (rLastTemplateBracket != STRING_NOTFOUND) )
    {
        bRemove=TRUE;
        for (xub_StrLen nI = rLastTemplateBracket+1; nI < rRet.Len(); nI++ )
            if (rRet.GetChar(nI) != ' ')
            {
                bRemove=FALSE;
                break;
            }
    }

    //suborderlist
    int nRet = HandleRecords(nLevel+1,rSelector,rVariation);

    if (bRemove)
    {
        rRet.Erase(rLastTemplateBracket,1);
        APPEND(rRet,"} ");
        rLastTemplateBracket = STRING_NOTFOUND;
    }
    if (rSelector == 0xf)
        rLastTemplateBracket = rRet.SearchBackward('}');
    else
        rLastTemplateBracket = STRING_NOTFOUND;

    rSelector = -1;
    return nRet;
}

void MathType::HandleEmblishments()
{
    sal_uInt8 nEmbel;
    //if (xfLMOVE(nTag))
    do
    {
        *pS >> nEmbel;
        switch (nEmbel)
        {
        case 0x02:
            APPEND(rRet," dot ");
            break;
        case 0x03:
            APPEND(rRet," ddot ");
            break;
        case 0x04:
            APPEND(rRet," dddot ");
            break;
        case 0x05:
            if (nPostSup == 0)
            {
                APPEND(sPost," sup {}");
                nPostSup = sPost.Len();
            }
            sPost.InsertAscii(" ' ",nPostSup-1);
            nPostSup += 3;
            break;
        case 0x06:
            if (nPostSup == 0)
            {
                APPEND(sPost," sup {}");
                nPostSup = sPost.Len();
            }
            sPost.InsertAscii(" '' ",nPostSup-1);
            nPostSup += 4;
            break;
        case 0x07:
            if (nPostlSup == 0)
            {
                APPEND(sPost," lsup {}");
                nPostlSup = sPost.Len();
            }
            sPost.InsertAscii(" ' ",nPostlSup-1);
            nPostlSup += 3;
            break;
        case 0x08:
            APPEND(rRet," tilde ");
            break;
        case 0x09:
            APPEND(rRet," hat ");
            break;
        case 0x0b:
            APPEND(rRet," vec ");
            break;
        case 0x10:
            APPEND(rRet," overstrike ");
            break;
        case 0x11:
            APPEND(rRet," bar ");
            break;
        case 0x12:
            if (nPostSup == 0)
            {
                APPEND(sPost," sup {}");
                nPostSup = sPost.Len();
            }
            sPost.InsertAscii(" ''' ",nPostSup-1);
            nPostSup += 5;
            break;
        case 0x14:
            APPEND(rRet," breve ");
            break;
        default:
            DBG_ASSERT(nEmbel < 21,"Embel out of range")
            break;
        }
        if (nVersion < 3)
            break;
    }while (nEmbel);
}

void MathType::HandleSetSize()
{
    sal_uInt8 nTemp;
    *pS >> nTemp;
    switch (nTemp)
    {
        case 101:
            *pS >> nLSize;
            nLSize = -nLSize;
            break;
        case 100:
            *pS >> nTemp;
            nLSize = nTemp;
            *pS >> nDSize;
            break;
        default:
            nLSize = nTemp;
            *pS >> nTemp;
            nDSize = nTemp-128;
            break;
    }
}

int MathType::HandleChar(xub_StrLen &rTextStart,int &rSetSize,int nLevel,
    sal_uInt8 nTag,sal_uInt8 nSelector,sal_uInt8 nVariation, sal_Bool bSilent)
{
    sal_Unicode nChar;
    int nRet=1;

    if (xfAUTO(nTag))
    {
    //This is a candidate for function recognition, whatever
    //that is!
    }
    //if (xfLMODE(nTag))

    sal_uInt8 nOldTypeFace = nTypeFace;
    *pS >> nTypeFace;
    if (nVersion < 3)
    {
        sal_uInt8 nChar8;
        *pS >> nChar8;
        nChar = nChar8;
    }
    else
        *pS >> nChar;

    /*
    ##912##
    bad character, old mathtype < 3 has these
    */
    if (nChar < 0x20)
        return nRet;

    if (xfEMBELL(nTag))
    {
        //A bit tricky, the character emblishments for
        //mathtype can all be listed after eachother, in
        //starmath some must go before the character and some
        //must go after. In addition some of the emblishments
        //may repeated and in starmath some of these groups
        //must be gathered together. sPost is the portion that
        //follows the char and nPostSup and nPostlSup are the
        //indexes at which this class of emblishment is
        //collated together
        sPost.Erase();
        nPostSup = nPostlSup = 0;
        int nOriglen=rRet.Len()-rTextStart;
        if ((!bSilent) && ((nOriglen) > 1))
            rRet += '\"';
        nRet = HandleRecords(nLevel+1,nSelector,nVariation);
        if (!bSilent)
        {
            if (nOriglen > 1)
            {
                String aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += '\"';
                rRet.Insert(aStr,rTextStart);

                aStr.Erase();
                TypeFaceToString(aStr,nTypeFace);
                rRet.Append(aStr);
                rRet += '{';
            }
            else
                APPEND(rRet," {");
            rTextStart = rRet.Len();
        }
    }

    if (!bSilent)
    {
        int nOldLen = rRet.Len();
        //nLastSize = nCurSize;
        if (
            HandleSize(nLSize,nDSize,rSetSize) ||
            (nOldTypeFace != nTypeFace)
           )
        {
            if ((nOldLen - rTextStart) > 1)
            {
                rRet.InsertAscii("\"",nOldLen);
                String aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += '\"';
                rRet.Insert(aStr,rTextStart);
            }
            rTextStart = rRet.Len();
        }
        nOldLen = rRet.Len();
        if (!LookupChar(nChar,rRet,nVersion,nTypeFace))
        {
            if (nOldLen - rTextStart > 1)
            {
                rRet.InsertAscii("\"",nOldLen);
                String aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += '\"';
                rRet.Insert(aStr,rTextStart);
            }
            rTextStart = rRet.Len();
        }
        lcl_PrependDummyTerm(rRet, rTextStart);
    }

    if ((xfEMBELL(nTag)) && (!bSilent))
    {
        rRet += '}';
        rRet += sPost;
        rTextStart = rRet.Len();
    }
    return nRet;
}

sal_Bool MathType::HandleLim(SmNode *pNode,int nLevel)
{
    sal_Bool bRet=0;
    //Special case for the "lim" option in StarMath
    if ((pNode->GetToken().eType == TLIM)
        || (pNode->GetToken().eType == TLIMSUP)
        || (pNode->GetToken().eType == TLIMINF)
        )
    {
        if (pNode->GetSubNode(1))
        {
            sal_uInt8 nVariation2=HandleCScript(pNode->GetSubNode(0),NULL,
                nLevel);

            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(LINE); //line
            *pS << sal_uInt8(CHAR|0x10);
            *pS << sal_uInt8(0x82);
            *pS << sal_uInt16('l');
            *pS << sal_uInt8(CHAR|0x10);
            *pS << sal_uInt8(0x82);
            *pS << sal_uInt16('i');
            *pS << sal_uInt8(CHAR|0x10);
            *pS << sal_uInt8(0x82);
            *pS << sal_uInt16('m');

            if (pNode->GetToken().eType == TLIMSUP)
            {
                *pS << sal_uInt8(CHAR); //some space
                *pS << sal_uInt8(0x98);
                *pS << sal_uInt16(0xEB04);

                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('s');
                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('u');
                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('p');
            }
            else if (pNode->GetToken().eType == TLIMINF)
            {
                *pS << sal_uInt8(CHAR); //some space
                *pS << sal_uInt8(0x98);
                *pS << sal_uInt16(0xEB04);

                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('i');
                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('n');
                *pS << sal_uInt8(CHAR|0x10);
                *pS << sal_uInt8(0x82);
                *pS << sal_uInt16('f');
            }


            *pS << sal_uInt8(CHAR); //some space
            *pS << sal_uInt8(0x98);
            *pS << sal_uInt16(0xEB04);

            if (nVariation2 != 0xff)
            {
                *pS << sal_uInt8(END);
                *pS << sal_uInt8(END);
            }
            HandleNodes(pNode->GetSubNode(1),nLevel+1);
            //*pS << sal_uInt8(END); //options
            bRet = 1;
        }
    }
    return bRet;
}

void MathType::HandleMAlign(SmNode *pNode,int nLevel)
{
    sal_uInt8 nPushedHAlign=nHAlign;
    switch(pNode->GetToken().eType)
    {
        case TALIGNC:
            nHAlign=2;
            break;
        case TALIGNR:
            nHAlign=3;
            break;
        default:
            nHAlign=1;
            break;
    }
    USHORT  nSize = pNode->GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (SmNode *pTemp = pNode->GetSubNode(i))
            HandleNodes(pTemp,nLevel+1);
    nHAlign=nPushedHAlign;
}

void MathType::HandleMath(SmNode *pNode,int nLevel)
{
    if (pNode->GetToken().eType == TMLINE)
    {
        *pS << sal_uInt8(END);
        *pS << sal_uInt8(LINE);
        bReInterpBrace=1;
        return;
    }
    SmMathSymbolNode *pTemp=(SmMathSymbolNode *)pNode;
    for(int i=0;i<pTemp->GetText().Len();i++)
    {
        sal_Unicode nArse = Convert(pTemp->GetText().GetChar(i));
        if ((nArse == 0x2224) || (nArse == 0x2288) || (nArse == 0x2285) ||
            (nArse == 0x2289))
        {
            *pS << sal_uInt8(CHAR|0x20);
        }
        else if ((nPendingAttributes) &&
                (i == ((pTemp->GetText().Len()+1)/2)-1))
            {
                *pS << sal_uInt8(0x22);
            }
        else
            *pS << sal_uInt8(CHAR); //char without formula recognition
        //The typeface seems to be MTEXTRA for unicode characters,
        //though how to determine when mathtype chooses one over
        //the other is unknown. This should do the trick
        //nevertheless.
        sal_uInt8 nBias;
        if ( (nArse == 0x2213) || (nArse == 0x2218) ||
            (nArse == 0x210F) || (
                (nArse >= 0x22EE) && (nArse <= 0x22FF)
            ))
        {
            nBias = 0xB; //typeface
        }
        else if ((nArse > 0x2000) || (nArse == 0x00D7))
            nBias = 0x6; //typeface
        else if (nArse == 0x3d1)
            nBias = 0x4;
        else if ((nArse > 0xFF) && ((nArse < 0x393) || (nArse > 0x3c9)))
            nBias = 0xB; //typeface
        else if ((nArse == 0x2F) || (nArse == 0x2225))
            nBias = 0x2; //typeface
        else
            nBias = 0x3; //typeface

        *pS << sal_uInt8(nSpec+nBias+128); //typeface

        if (nArse == 0x2224)
        {
            *pS << sal_uInt16(0x7C);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2225)
            *pS << sal_uInt16(0xEC09);
        else if (nArse == 0xE421)
            *pS << sal_uInt16(0x2265);
        else if (nArse == 0xE425)
            *pS << sal_uInt16(0x2264);
        else if (nArse == 0x226A)
        {
            *pS << sal_uInt16(0x3C);
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x98);
            *pS << sal_uInt16(0xEB01);
            *pS << sal_uInt8(CHAR);
            *pS << sal_uInt8(0x86);
            *pS << sal_uInt16(0x3c);
        }
        else if (nArse == 0x2288)
        {
            *pS << sal_uInt16(0x2286);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2289)
        {
            *pS << sal_uInt16(0x2287);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else if (nArse == 0x2285)
        {
            *pS << sal_uInt16(0x2283);
            *pS << sal_uInt8(EMBEL);
            *pS << sal_uInt8(0x0A);
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
        else
            *pS << nArse;
    }
    nPendingAttributes = 0;
}

void MathType::HandleAttributes(SmNode *pNode,int nLevel)
{
    int nOldPending;
    USHORT  nSize = pNode->GetNumSubNodes();
    SmNode *pTemp;
    SmTextNode *pIsText;

    //SmTextNode *pTemp=(SmTextNode *)pNode;
    //for(int i=0;i<pTemp->GetText().Len();i++)

    if (pTemp = pNode->GetSubNode(0))
    {
        pIsText = (SmTextNode *)pNode->GetSubNode(1);

        switch (pTemp->GetToken().eType)
        {
        case TWIDEVEC:
            //theres just no way we can now handle any character
            //attributes (from mathtypes perspective) centered
            //over an expression but above template attributes
            //such as widevec and similiar constructs
            //we have to drop them
            nOldPending = StartTemplate(0x2f,0x01);
            break;
        case TCHECK: //Not Exportable
        case TACUTE: //Not Exportable
        case TGRAVE: //Not Exportable
        case TCIRCLE: //Not Exportable
        case TWIDETILDE: //Not Exportable
        case TWIDEHAT: //Not Exportable
            break;
        case TUNDERLINE:
            nOldPending = StartTemplate(0x10);
            break;
        case TOVERLINE: //If the next node is not text
                        //or text with more than one char
            if ((pIsText->GetToken().eType != NTEXT) ||
                (pIsText->GetText().Len() > 1))
                nOldPending = StartTemplate(0x11);
            break;
        default:
            nPendingAttributes++;
            break;
        }
    }

    if (pIsText)
        HandleNodes(pIsText,nLevel+1);

    switch (pTemp->GetToken().eType)
    {
        case TWIDEVEC:
        case TUNDERLINE:
            EndTemplate(nOldPending);
            break;
        case TOVERLINE:
            if ((pIsText->GetToken().eType != NTEXT) ||
                (pIsText->GetText().Len() > 1))
                EndTemplate(nOldPending);
            break;
        default:
            break;
    }

    //if there was no suitable place to put the attribute,
    //then we have to just give up on it
    if (nPendingAttributes)
        nPendingAttributes--;
    else
    {
        if ((nInsertion != 0) && (pTemp = pNode->GetSubNode(0)))
        {
            ULONG nPos = pS->Tell();
            nInsertion--;
            pS->Seek(nInsertion);
            switch(pTemp->GetToken().eType)
            {
            case TACUTE: //Not Exportable
            case TGRAVE: //Not Exportable
            case TCIRCLE: //Not Exportable
                break;
            case TCDOT:
                *pS << sal_uInt8(2);
                break;
            case TDDOT:
                *pS << sal_uInt8(3);
                break;
            case TDDDOT:
                *pS << sal_uInt8(4);
                break;
            case TTILDE:
                *pS << sal_uInt8(8);
                break;
            case THAT:
                *pS << sal_uInt8(9);
                break;
            case TVEC:
                *pS << sal_uInt8(11);
                break;
            case TOVERSTRIKE:
                *pS << sal_uInt8(16);
                break;
            case TOVERLINE:
                if ((pIsText->GetToken().eType == NTEXT) &&
                    (pIsText->GetText().Len() == 1))
                    *pS << sal_uInt8(17);
                break;
            case TBREVE:
                *pS << sal_uInt8(20);
                break;
            case TWIDEVEC:
            case TUNDERLINE:
            case TWIDETILDE:
            case TWIDEHAT:
                break;
            case TBAR:
                *pS << sal_uInt8(17);
                break;
            default:
                *pS << sal_uInt8(0x2);
                break;
            }
        pS->Seek(nPos);
        }
    }
}

void MathType::HandleText(SmNode *pNode,int nLevel)
{
    SmTextNode *pTemp=(SmTextNode *)pNode;
    for(int i=0;i<pTemp->GetText().Len();i++)
    {
        if ((nPendingAttributes) &&
            (i == ((pTemp->GetText().Len()+1)/2)-1))
        {
            *pS << sal_uInt8(0x22);     //char, with attributes right
                                //after the character
        }
        else
            *pS << sal_uInt8(CHAR);
            //*pS << sal_uInt8(CHAR|0x10); //char with formula recognition

#if 1
        sal_uInt8 nFace = 0x1;
        if (pNode->GetFont().GetItalic() == ITALIC_NORMAL)
            nFace = 0x3;
        else if (pNode->GetFont().GetWeight() == WEIGHT_BOLD)
            nFace = 0x7;
        *pS << sal_uInt8(nFace+128); //typeface
#else
        if ((pTemp->GetText().GetChar(i) >= '0') &&
            (pTemp->GetText().GetChar(i) <= '9'))
            *pS << sal_uInt8(0x8+128); //typeface
        else
            *pS << sal_uInt8(0x3+128); //typeface
#endif
        sal_uInt16 nChar = pTemp->GetText().GetChar(i);
        *pS << Convert(nChar);

        //Mathtype can only have these sort of character
        //attributes on a single character, starmath can put them
        //anywhere, when the entity involved is a text run this is
        //a large effort to place the character attribute on the
        //central mathtype character so that it does pretty much
        //what the user probably has in mind. The attributes
        //filled in here are dummy ones which are replaced in the
        //ATTRIBUT handler if a suitable location for the
        //attributes was found here. Unfortunately it is
        //possible for starmath to place character attributes on
        //entities which cannot occur in mathtype e.g. a Summation
        //symbol so these attributes may be lost
        if ((nPendingAttributes) &&
            (i == ((pTemp->GetText().Len()+1)/2)-1))
        {
            *pS << sal_uInt8(EMBEL);
            while (nPendingAttributes)
            {
                *pS << sal_uInt8(2);
                //wedge the attributes in here and clear
                //the pending stack
                nPendingAttributes--;
            }
            nInsertion=pS->Tell();
            *pS << sal_uInt8(END); //end embel
            *pS << sal_uInt8(END); //end embel
        }
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
