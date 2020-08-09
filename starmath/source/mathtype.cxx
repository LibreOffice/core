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

#include "mathtype.hxx"
#include "mathtype2.hxx"

#include <filter/msfilter/classids.hxx>
#include <osl/diagnose.h>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <sal/log.hxx>

#include "eqnolefilehdr.hxx"
#include <node.hxx>
#include <stdlib.h>

void MathType::Init()
{
    //These are the default MathType sizes
    aSizeTable.push_back(12);
    aSizeTable.push_back(8);
    aSizeTable.push_back(6);
    aSizeTable.push_back(24);
    aSizeTable.push_back(10);
    aSizeTable.push_back(12);
    aSizeTable.push_back(12);

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
 sal_True in the case that the char is just a char, and
 sal_False if the character is an operator which must not be
 placed inside the quote sequence designed to protect
 against being parsed as a keyword

 General solution required to force starmath to handle
 unicode math chars the way it handles its own math
 chars rather than handle them as text as it will do
 for the default case below, i.e. incorrect spacing
 between math symbols and ordinary text e.g. 1=2 rather
 than 1 = 2
 */
bool MathType::LookupChar(sal_Unicode nChar,OUStringBuffer &rRet,sal_uInt8 nVersion,
    sal_uInt8 nTypeFace)
{
    bool bRet=false;
    const char *pC = nullptr;
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
                rRet.append(OUStringChar(nChar));
                bRet=true;
            }
            break;
        case 0x00fb:
            if ((nVersion < 3) && (nTypeFace == 0x81))
                nChar = 0xDF;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'a':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b1;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'b':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b2;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'l':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bb;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'n':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bd;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'r':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3c1;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 'D':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x394;
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 0xa9:
            if ((nVersion < 3) && (nTypeFace == 0x82))
                nChar = '\'';
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
        case 0x00f1:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " \\rangle ";
            else
            {
                rRet.append(OUStringChar(nChar));
                bRet=true;
            }
            break;
        case 0x00a3:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " <= ";
            else
            {
                rRet.append(OUStringChar(nChar));
                bRet=true;
            }
            break;
        case 0x00de:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " drarrow ";
            else
            {
                rRet.append(OUStringChar(nChar));
                bRet=true;
            }
            break;
        case 0x0057:
            if ((nVersion < 3) && (nTypeFace == 0x85))
                pC = " %OMEGA ";
            else
            {
                rRet.append(OUStringChar(nChar));
                bRet=true;
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
        case 0x2204:
            pC = " notexists ";
            break;
        case 0x2205:
            pC = " emptyset ";
            break;
        case 0x2207:
            pC = " nabla ";
            break;
        case 0x2112:
            pC = " laplace ";
            break;
        case 0x2208: // in
        case 0x2209: // notin
            rRet.append(" func ").append(OUStringChar(nChar)).append(" ");
            break;
        case 0x220d: // owns
            rRet.append(u" func \u220b ");
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

        case 0x227A:
            pC = " prec ";
            break;
        case 0x227B:
            pC = " succ ";
            break;
        case 0x227C:
            pC = " preccurlyeq ";
            break;
        case 0x227D:
            pC = " succcurlyeq ";
            break;
        case 0x227E:
            pC = " precsim ";
            break;
        case 0x227F:
            pC = " succsim ";
            break;
        case 0x2280:
            pC = " nprec ";
            break;
        case 0x2281:
            pC = " nsucc ";
            break;

        case 0x2282: // subset
        case 0x2283: // supset
        case 0x2284: // nsubset
        case 0x2285: // nsupset
        case 0x2286: // subseteq
        case 0x2287: // supseteq
        case 0x2288: // nsubseteq
        case 0x2289: // nsupseteq
        case 0x22b2: // NORMAL SUBGROUP OF
        case 0x22b3: // CONTAINS AS NORMAL SUBGROUP
            rRet.append(" func ").append(OUStringChar(nChar)).append(" ");
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
        case MS_LANGLE:
        case MS_LMATHANGLE:
            pC = " langle ";
            break;
        case MS_RANGLE:
        case MS_RMATHANGLE:
            pC = " rangle ";
            break;
        case 0x301a:
            pC = " ldbracket ";
            break;
        case 0x301b:
            pC = " rdbracket ";
            break;
        case 0xe083:
            rRet.append("+");
            bRet=true;
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
            bRet=true;
            break;
        case 0xef04:    //tiny space
        case 0xef05:    //tiny space
        case 0xeb02:    //small space
        case 0xeb04:    //medium space
            rRet.append("`");
            break;
        case 0xeb05:    //large space
            rRet.append("~");
            break;
        case 0x3a9:
            pC = " %OMEGA ";
            break;
        default:
            rRet.append(OUStringChar(nChar));
            bRet=true;
            break;
    }
    if (pC)
        rRet.appendAscii(pC);
    return bRet;
}

void MathTypeFont::AppendStyleToText(OUString &rRet)
{
    const char *pC = nullptr;
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
        rRet += OUString::createFromAscii( pC );
}

void MathType::TypeFaceToString(OUString &rTxt,sal_uInt8 nFace)
{
    MathTypeFont aFont(nFace);
    MathTypeFontSet::iterator aItr = aUserStyles.find(aFont);
    if (aItr != aUserStyles.end())
        aFont.nStyle = aItr->nStyle;
    aFont.AppendStyleToText(rTxt);
}

bool MathType::Parse(SotStorage *pStor)
{
    tools::SvRef<SotStorageStream> xSrc = pStor->OpenSotStream(
        "Equation Native",
        StreamMode::STD_READ);
    if ( (!xSrc.is()) || (ERRCODE_NONE != xSrc->GetError()))
        return false;
    return Parse(xSrc.get());
}

bool MathType::Parse(SvStream* pStream)
{
    pS = pStream;
    pS->SetEndian( SvStreamEndian::LITTLE );

    EQNOLEFILEHDR aHdr;
    aHdr.Read(pS);
    sal_uInt8 nProdVersion;
    sal_uInt8 nProdSubVersion;
    sal_uInt8 nPlatform;
    sal_uInt8 nProduct;
    pS->ReadUChar( nVersion );
    pS->ReadUChar( nPlatform );
    pS->ReadUChar( nProduct );
    pS->ReadUChar( nProdVersion );
    pS->ReadUChar( nProdSubVersion );

    if (nVersion > 3)   // allow only supported versions of MathType to be parsed
        return false;

    bool bRet = HandleRecords(0);
    //little crude hack to close occasionally open expressions
    //a sophisticated system to determine what expressions are
    //opened is required, but this is as much work as rewriting
    //starmaths internals.
    rRet.append("{}");

    return bRet;
}

static void lcl_PrependDummyTerm(OUStringBuffer &rRet, sal_Int32 &rTextStart)
{
    if ((rTextStart < rRet.getLength()) &&
        (rRet[rTextStart] == '=') &&
        ((rTextStart == 0) || (rRet[ rTextStart-1 ] == '{'))
       )
    {
        rRet.insert(rTextStart, " {}");
        rTextStart+=3;
    }
}

static void lcl_AppendDummyTerm(OUStringBuffer &rRet)
{
    bool bOk=false;
    for(int nI=rRet.getLength()-1;nI >= 0; nI--)
    {
        sal_Int32 nIdx = sal::static_int_cast< sal_Int32 >(nI);
        sal_Unicode nChar = rRet[nIdx];
        if (nChar == ' ')
            continue;
        if (rRet[nIdx] != '{')
            bOk=true;
        break;
    }
    if (!bOk)   //No term, use dummy
        rRet.append(" {}");
}

void MathType::HandleNudge()
{
    sal_uInt8 nXNudge;
    pS->ReadUChar( nXNudge );
    sal_uInt8 nYNudge;
    pS->ReadUChar( nYNudge );
    if (nXNudge == 128 && nYNudge == 128)
    {
        sal_uInt16 nXLongNudge;
        sal_uInt16 nYLongNudge;
        pS->ReadUInt16( nXLongNudge );
        pS->ReadUInt16( nYLongNudge );
    }
}

/* Fabulously complicated as many tokens have to be reordered and generally
 * moved around from mathtypes paradigm to starmaths. */
bool MathType::HandleRecords(int nLevel, sal_uInt8 nSelector,
    sal_uInt8 nVariation, int nMatrixRows, int nMatrixCols)
{
    //depth-protect
    if (nLevel > 1024)
        return false;

    sal_uInt8 nTag,nRecord;
    sal_uInt8 nTabType,nTabStops;
    sal_uInt16 nTabOffset;
    int i, newline=0;
    bool bSilent=false;
    int nPart=0;
    OUString sPush,sMainTerm;
    int nSetSize=0,nSetAlign=0;
    int nCurRow=0,nCurCol=0;
    bool bOpenString=false;
    sal_Int32 nTextStart = 0;
    sal_Int32 nSubSupStartPos = 0;
    sal_Int32 nLastTemplateBracket=-1;
    bool bRet = true;

    do
    {
        nTag = 0;
        pS->ReadUChar( nTag );
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
        if ((nRecord == CHAR) && (!bOpenString))
        {
            bOpenString=true;
            nTextStart = rRet.getLength();
        }
        else if ((nRecord != CHAR) && bOpenString)
        {
            bOpenString=false;
            if ((rRet.getLength() - nTextStart) > 1)
            {
                OUString aStr;
                TypeFaceToString(aStr,nTypeFace);
                aStr += "\"";
                rRet.insert(nTextStart,aStr);
                rRet.append("\"");
            }
            else if (nRecord == END && !rRet.isEmpty())
            {
                sal_Unicode cChar = 0;
                sal_Int32 nI = rRet.getLength()-1;
                while (nI)
                {
                    cChar = rRet[nI];
                    if (cChar != ' ')
                        break;
                    --nI;
                }
                if ((cChar == '=') || (cChar == '+') || (cChar == '-'))
                    rRet.append("{}");
            }
        }

        switch(nRecord)
        {
            case LINE:
                {
                    if (xfLMOVE(nTag))
                        HandleNudge();

                    if (newline>0)
                        rRet.append("\nnewline\n");
                    if (!(xfNULL(nTag)))
                    {
                        switch (nSelector)
                        {
                        case tmANGLE:
                            if (nVariation==0)
                                rRet.append(" langle ");
                            else if (nVariation==1)
                                rRet.append(" \\langle ");
                            break;
                        case tmPAREN:
                            if (nVariation==0)
                                rRet.append(" left (");
                            else if (nVariation==1)
                                rRet.append("\\(");
                            break;
                        case tmBRACE:
                            if ((nVariation==0) || (nVariation==1))
                                rRet.append(" left lbrace ");
                            else
                                rRet.append(" left none ");
                            break;
                        case tmBRACK:
                            if (nVariation==0)
                                rRet.append(" left [");
                            else if (nVariation==1)
                                rRet.append("\\[");
                            break;
                        case tmLBLB:
                        case tmLBRP:
                            rRet.append(" \\[");
                            break;
                        case tmBAR:
                            if (nVariation==0)
                                rRet.append(" lline ");
                            else if (nVariation==1)
                                rRet.append(" \\lline ");
                            break;
                        case tmDBAR:
                            if (nVariation==0)
                                rRet.append(" ldline ");
                            else if (nVariation==1)
                                rRet.append(" \\ldline ");
                            break;
                        case tmFLOOR:
                            if (nVariation == 0 || nVariation & 0x01) // tvFENCE_L
                                rRet.append(" left lfloor ");
                            else
                                rRet.append(" left none ");
                            break;
                        case tmCEILING:
                            if (nVariation==0)
                                rRet.append(" lceil ");
                            else if (nVariation==1)
                                rRet.append(" \\lceil ");
                            break;
                        case tmRBRB:
                        case tmRBLB:
                            rRet.append(" \\]");
                            break;
                        case tmLPRB:
                            rRet.append(" \\(");
                            break;
                        case tmROOT:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" sqrt");
                                else
                                {
                                    rRet.append(" nroot");
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            rRet.append(" {");
                            break;
                        case tmFRACT:
                            if (nPart == 0)
                                rRet.append(" { ");


                            if (nPart == 1)
                                rRet.append(" over ");
                            rRet.append(" {");
                            break;
                        case tmSCRIPT:
                            nSubSupStartPos = rRet.getLength();
                            if ((nVariation == 0) ||
                                    ((nVariation == 2) && (nPart==1)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                rRet.append(" rSup");
                            }
                            else if ((nVariation == 1) ||
                                    ((nVariation == 2) && (nPart==0)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                rRet.append(" rSub");
                            }
                            rRet.append(" {");
                            break;
                        case tmUBAR:
                            if (nVariation == 0)
                                rRet.append(" {underline ");
                            else if (nVariation == 1)
                                rRet.append(" {underline underline ");
                            rRet.append(" {");
                            break;
                        case tmOBAR:
                            if (nVariation == 0)
                                rRet.append(" {overline ");
                            else if (nVariation == 1)
                                rRet.append(" {overline overline ");
                            rRet.append(" {");
                            break;
                        case tmLARROW:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" widevec ");//left arrow above
                                else if (nVariation == 1)
                                    rRet.append(" widevec ");//left arrow below
                                rRet.append(" {");
                            }
                            break;
                        case tmRARROW:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" widevec ");//right arrow above
                                else if (nVariation == 1)
                                    rRet.append(" widevec ");//right arrow below
                                rRet.append(" {");
                            }
                            break;
                        case tmBARROW:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" widevec ");//double arrow above
                                else if (nVariation == 1)
                                    rRet.append(" widevec ");//double arrow below
                                rRet.append(" {");
                            }
                            break;
                        case tmSINT:
                            if (nPart == 0)
                            {
                                if ((nVariation == 3) || (nVariation == 4))
                                    rRet.append(" lInt");
                                else
                                    rRet.append(" Int");
                                if ( (nVariation != 0) && (nVariation != 3))
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 4)) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 2) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 2) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmDINT:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    rRet.append(" llInt");
                                else
                                    rRet.append(" iInt");
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmTINT:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    rRet.append(" lllInt");
                                else
                                    rRet.append(" iiInt");
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmSSINT:
                            if (nPart == 0)
                            {
                                if (nVariation == 2)
                                    rRet.append(" lInt");
                                else
                                    rRet.append(" Int");
                                sPush = rRet.makeStringAndClear();
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 2)) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 0) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmDSINT:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" llInt");
                                else
                                    rRet.append(" iInt");
                                sPush = rRet.makeStringAndClear();
                            }
                            if (nPart==1)
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmTSINT:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet.append(" lllInt");
                                else
                                    rRet.append(" iiInt");
                                sPush = rRet.makeStringAndClear();
                            }
                            if (nPart==1)
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmUHBRACE:
                        case tmLHBRACE:
                            rRet.append(" {");
                            break;
                        case tmSUM:
                            if (nPart == 0)
                            {
                                rRet.append(" Sum");
                                if (nVariation != 2)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmISUM:
                            if (nPart == 0)
                            {
                                rRet.append(" Sum");
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmPROD:
                            if (nPart == 0)
                            {
                                rRet.append(" Prod");
                                if (nVariation != 2)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmIPROD:
                            if (nPart == 0)
                            {
                                rRet.append(" Prod");
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmCOPROD:
                            if (nPart == 0)
                            {
                                rRet.append(" coProd");
                                if (nVariation != 2)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmICOPROD:
                            if (nPart == 0)
                            {
                                rRet.append(" coProd");
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmUNION:
                            if (nPart == 0)
                            {
                                rRet.append(" union"); //union
                                if (nVariation != 2)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmIUNION:
                            if (nPart == 0)
                            {
                                rRet.append(" union"); //union
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmINTER:
                            if (nPart == 0)
                            {
                                rRet.append(" intersect"); //intersect
                                if (nVariation != 2)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmIINTER:
                            if (nPart == 0)
                            {
                                rRet.append(" intersect"); //intersect
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" rSub");
                            else if ((nVariation == 1) && (nPart==2))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmLIM:
                            if ((nVariation == 0) && (nPart==1))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 2) && (nPart==1))
                                rRet.append(" cSub");
                            else if ((nVariation == 2) && (nPart==2))
                                rRet.append(" cSup");
                            rRet.append(" {");
                            break;
                        case tmLDIV:
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sPush = rRet.makeStringAndClear();
                                }
                            }
                            rRet.append(" {");
                            if (nVariation == 0)
                            {
                                if (nPart == 1)
                                    rRet.append("alignr ");
                            }
                            if (nPart == 0)
                                rRet.append("\\lline ");
                            if (nVariation == 1)
                                rRet.append("overline ");
                            break;
                        case tmSLFRACT:
                            rRet.append(" {");
                            break;
                        case tmINTOP:
                            if (nPart == 0)
                            {
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                rRet.append(" rSup");
                            else if ((nVariation == 2) && (nPart==1))
                                rRet.append(" rSup");
                            else if ((nVariation == 1) && (nPart==0))
                                rRet.append(" rSub");
                            else if ((nVariation == 2) && (nPart==0))
                                rRet.append(" rSub");
                            rRet.append(" {");
                            break;
                        case tmSUMOP:
                            if (nPart == 0)
                            {
                                sPush = rRet.makeStringAndClear();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                rRet.append(" cSup");
                            else if ((nVariation == 2) && (nPart==1))
                                rRet.append(" cSup");
                            else if ((nVariation == 1) && (nPart==0))
                                rRet.append(" cSub");
                            else if ((nVariation == 2) && (nPart==0))
                                rRet.append(" cSub");
                            rRet.append(" {");
                            break;
                        case tmLSCRIPT:
                            if (nPart == 0)
                                rRet.append("\"\"");
                            if ((nVariation == 0)
                                    || ((nVariation == 2) && (nPart==1)))
                                rRet.append(" lSup");
                            else if ((nVariation == 1)
                                    || ((nVariation == 2) && (nPart==0)))
                                rRet.append(" lSub");
                            rRet.append(" {");
                            break;
                        case tmDIRAC:
                            if (nVariation==0)
                            {
                                if (nPart == 0)
                                    rRet.append(" langle ");
                            }
                            else if (nVariation==1)
                            {
                                rRet.append(" \\langle ");
                                newline--;
                            }
                            else if (nVariation==2)
                            {
                                rRet.append(" \\lline ");
                                newline--;
                            }
                            break;
                        case tmUARROW:
                            if (nVariation == 0)
                                rRet.append(" widevec ");//left below
                            else if (nVariation == 1)
                                rRet.append(" widevec ");//right below
                            else if (nVariation == 2)
                                rRet.append(" widevec ");//double headed below
                            rRet.append(" {");
                            break;
                        case tmOARROW:
                            if (nVariation == 0)
                                rRet.append(" widevec ");//left above
                            else if (nVariation == 1)
                                rRet.append(" widevec ");//right above
                            else if (nVariation == 2)
                                rRet.append(" widevec ");//double headed above
                            rRet.append(" {");
                            break;
                        default:
                            break;
                        }
                        sal_Int16 nOldCurSize=nCurSize;
                        sal_Int32 nSizeStartPos = rRet.getLength();
                        HandleSize( nLSize, nDSize, nSetSize );
                        bRet = HandleRecords( nLevel+1 );
                        while (nSetSize)
                        {
                            bool bOk=false;
                            sal_Int32 nI = rRet.lastIndexOf('{');
                            if (nI != -1)
                            {
                                for(nI=nI+1;nI<rRet.getLength();nI++)
                                    if (rRet[nI] != ' ')
                                    {
                                        bOk=true;
                                        break;
                                    }
                            }
                            else
                                bOk=true;

                            if (bOk)
                                rRet.append("} ");
                            else if (rRet.getLength() > nSizeStartPos)
                                rRet = rRet.truncate(nSizeStartPos);
                            nSetSize--;
                            nCurSize=nOldCurSize;
                        }


                        HandleMatrixSeparator(nMatrixRows,nMatrixCols,
                            nCurCol,nCurRow);

                        switch (nSelector)
                        {
                        case tmANGLE:
                            if (nVariation==0)
                                rRet.append(" rangle ");
                            else if (nVariation==2)
                                rRet.append(" \\rangle ");
                            break;
                        case tmPAREN:
                            if (nVariation==0)
                                rRet.append(" right )");
                            else if (nVariation==2)
                                rRet.append("\\)");
                            break;
                        case tmBRACE:
                            if ((nVariation==0) || (nVariation==2))
                                rRet.append(" right rbrace ");
                            else
                                rRet.append(" right none ");
                            break;
                        case tmBRACK:
                            if (nVariation==0)
                                rRet.append(" right ]");
                            else if (nVariation==2)
                                rRet.append("\\]");
                            break;
                        case tmBAR:
                            if (nVariation==0)
                                rRet.append(" rline ");
                            else if (nVariation==2)
                                rRet.append(" \\rline ");
                            break;
                        case tmDBAR:
                            if (nVariation==0)
                                rRet.append(" rdline ");
                            else if (nVariation==2)
                                rRet.append(" \\rdline ");
                            break;
                        case tmFLOOR:
                            if (nVariation == 0 || nVariation & 0x02) // tvFENCE_R
                                rRet.append(" right rfloor ");
                            else
                                rRet.append(" right none ");
                            break;
                        case tmCEILING:
                            if (nVariation==0)
                                rRet.append(" rceil ");
                            else if (nVariation==2)
                                rRet.append(" \\rceil ");
                            break;
                        case tmLBLB:
                        case tmRBLB:
                            rRet.append("\\[");
                            break;
                        case tmRBRB:
                        case tmLPRB:
                            rRet.append("\\]");
                            break;
                        case tmROOT:
                            rRet.append("} ");
                            if (nVariation == 1)
                            {
                                if (nPart == 0)
                                {
                                    newline--;
                                    sMainTerm = rRet.makeStringAndClear();
                                }
                                else if (nPart == 1)
                                {
                                    rRet.insert(0, sPush);
                                    rRet.append(sMainTerm);
                                    sPush.clear();
                                    sMainTerm.clear();
                                }
                            }
                            else
                            {
                                if (nPart == 0)
                                    newline--;
                            }
                            nPart++;
                            break;
                        case tmLBRP:
                            rRet.append("\\)");
                            break;
                        case tmFRACT:
                            rRet.append("} ");
                            if (nPart == 0)
                                newline--;
                            else
                                rRet.append("} ");
                            nPart++;
                            break;
                        case tmSCRIPT:
                            {
                            if ((nPart == 0) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;

                            bool bOk=false;
                            sal_Int32 nI = rRet.lastIndexOf('{');
                            if (nI != -1)
                            {
                                for(nI=nI+1;nI<rRet.getLength();nI++)
                                    if (rRet[nI] != ' ')
                                    {
                                        bOk=true;
                                        break;
                                    }
                            }
                            else
                                bOk=true;

                            if (bOk)
                                rRet.append("} ");
                            else if (rRet.getLength() > nSubSupStartPos)
                                rRet = rRet.truncate(nSubSupStartPos);
                            nPart++;
                            }
                            break;
                        case tmLSCRIPT:
                            if ((nPart == 0) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            rRet.append("} ");
                            nPart++;
                            break;
                        case tmUARROW:
                        case tmOARROW:
                            rRet.append("} ");
                            break;
                        case tmUBAR:
                        case tmOBAR:
                            rRet.append("}} ");
                            break;
                        case tmLARROW:
                        case tmRARROW:
                        case tmBARROW:
                            if (nPart == 0)
                            {
                                newline--;
                                rRet.append("} ");
                            }
                            nPart++;
                            break;
                        case tmUHBRACE:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                newline--;
                                rRet.append("overbrace");
                            }
                            nPart++;
                            break;
                        case tmLHBRACE:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                newline--;
                                rRet.append("underbrace");
                            }
                            nPart++;
                            break;
                        case tmLIM:
                            if (nPart==0)
                                newline--;
                            else if ((nPart==1) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            rRet.append("} ");
                            nPart++;
                            break;
                        case tmLDIV:
                            rRet.append("} ");
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sMainTerm = rRet.makeStringAndClear();
                                }
                                else if (nPart == 1)
                                {
                                    rRet.insert(0, sPush);
                                    rRet.append(" over ").append(sMainTerm);
                                    sPush.clear();
                                    sMainTerm.clear();
                                }
                            }
                            if (nPart == 0)
                                newline--;
                            nPart++;
                            break;
                        case tmSLFRACT:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                newline--;
                                switch (nVariation)
                                {
                                case 1:
                                    rRet.append("slash");
                                    break;
                                default:
                                    rRet.append("wideslash");
                                    break;
                                }
                            }
                            nPart++;
                            break;
                        case tmSUM:
                        case tmISUM:
                        case tmPROD:
                        case tmIPROD:
                        case tmCOPROD:
                        case tmICOPROD:
                        case tmUNION:
                        case tmIUNION:
                        case tmINTER:
                        case tmIINTER:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                if (nVariation != 2)
                                {
                                    sMainTerm = rRet.makeStringAndClear();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 0))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 1))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 1))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            nPart++;
                            break;
                        case tmSINT:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 3))
                                {
                                    sMainTerm = rRet.makeStringAndClear();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==4)))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 2))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 2))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            nPart++;
                            break;
                        case tmDINT:
                        case tmTINT:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 2))
                                {
                                    sMainTerm = rRet.makeStringAndClear();
                                }
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==3)))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            nPart++;
                            break;
                        case tmSSINT:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                sMainTerm = rRet.makeStringAndClear();
                                newline--;
                            }
                            else if ((nPart == 1) &&
                                    ((nVariation == 1) || (nVariation==2)))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            else if ((nPart == 1) && (nVariation == 0))
                                newline--;
                            else if ((nPart == 2) && (nVariation == 0))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            nPart++;
                            break;
                        case tmDSINT:
                        case tmTSINT:
                            rRet.append("} ");
                            if (nPart == 0)
                            {
                                sMainTerm = rRet.makeStringAndClear();
                                newline--;
                            }
                            else if (nPart == 1)
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                                newline--;
                            }
                            nPart++;
                            break;
                        case tmINTOP:
                        case tmSUMOP:
                            rRet.append("} ");

                            if ((nPart == 0) &&
                                    ((nVariation == 0) || (nVariation == 1)))
                            {
                                sMainTerm = rRet.makeStringAndClear();
                                newline--;
                            }
                            else if ((nPart == 0) && (nVariation == 2))
                                newline--;
                            else if ((nPart == 1) && (nVariation == 2))
                            {
                                sMainTerm = rRet.makeStringAndClear();
                                newline--;
                            }
                            else if ((nPart == 2) || ((nPart == 1) &&
                                    (nVariation == 0 || nVariation == 1)))
                            {
                                rRet.insert(0, sPush);
                                rRet.append(sMainTerm);
                                sPush.clear();
                                sMainTerm.clear();
                            }
                            nPart++;
                            break;
                        case tmDIRAC:
                            if (nVariation==0)
                            {
                                if (nPart == 0)
                                {
                                    newline--; //there is another term to arrive
                                    rRet.append(" mline ");
                                }
                                else
                                    rRet.append(" rangle ");
                            }
                            else if (nVariation==1)
                                rRet.append(" \\lline ");
                            else if (nVariation==2)
                                rRet.append(" \\rangle ");
                            nPart++;
                            break;
                        default:
                            break;
                        }
                        bSilent = true; //Skip the optional brackets and/or
                                        //symbols that follow some of these
                                        //records. Foo Data.

                        /*In matrices and piles we cannot separate equation
                         *lines with the newline keyword*/
                        if (nMatrixCols==0)
                            newline++;
                    }
                }
                break;
            case CHAR:
                if (xfLMOVE(nTag))
                    HandleNudge();
                bRet = HandleChar( nTextStart, nSetSize, nLevel, nTag, nSelector, nVariation, bSilent );
                break;
            case TMPL:
                if (xfLMOVE(nTag))
                    HandleNudge();
                bRet = HandleTemplate( nLevel, nSelector, nVariation, nLastTemplateBracket );
                break;
            case PILE:
                if (xfLMOVE(nTag))
                    HandleNudge();
                bRet = HandlePile( nSetAlign, nLevel, nSelector, nVariation );
                HandleMatrixSeparator( nMatrixRows, nMatrixCols, nCurCol, nCurRow );
                break;
            case MATRIX:
                if (xfLMOVE(nTag))
                    HandleNudge();
                bRet = HandleMatrix( nLevel, nSelector, nVariation );
                HandleMatrixSeparator( nMatrixRows, nMatrixCols, nCurCol, nCurRow );
                break;
            case EMBEL:
                if (xfLMOVE(nTag))
                    HandleNudge();
                HandleEmblishments();
                break;
            case RULER:
                pS->ReadUChar( nTabStops );
                for (i=0;i<nTabStops;i++)
                {
                    pS->ReadUChar( nTabType );
                    pS->ReadUInt16( nTabOffset );
                }
                SAL_WARN("starmath", "Not seen in the wild Equation Ruler Field");
                break;
            case FONT:
                {
                    MathTypeFont aFont;
                    pS->ReadUChar( aFont.nTface );
                    /*
                    The typeface number is the negative (which makes it
                    positive) of the typeface value (unbiased) that appears in
                    CHAR records that might follow a given FONT record
                    */
                    aFont.nTface = 128-aFont.nTface;
                    pS->ReadUChar( aFont.nStyle );
                    aUserStyles.insert(aFont);
                    // read font name
                    while(true)
                    {
                        char nChar8(0);
                        pS->ReadChar( nChar8 );
                        if (nChar8 == 0)
                            break;
                    }
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
    while (nRecord != END && !pS->eof());
    while (nSetSize)
    {
        rRet.append("}");
        nSetSize--;
    }
    return bRet;
}

/*Simply determine if we are at the end of a record or the end of a line,
 *with fiddly logic to see if we are in a matrix or a pile or neither

 Note we cannot tell until after the event that this is the last entry
 of a pile, so we must strip the last separator of a pile after this
 is detected in the PILE handler
 */
void MathType::HandleMatrixSeparator(int nMatrixRows,int nMatrixCols,
    int &rCurCol,int &rCurRow)
{
    if (nMatrixRows==0)
        return;

    if (rCurCol == nMatrixCols-1)
    {
        if (rCurRow != nMatrixRows-1)
            rRet.append(" {} ##\n");
        if (nMatrixRows!=-1)
        {
            rCurCol=0;
            rCurRow++;
        }
    }
    else
    {
        rRet.append(" {} # ");
        if (nMatrixRows!=-1)
            rCurCol++;
        else
            rRet.append("\n");
    }
}

/* set the alignment of the following term, but starmath currently
 * cannot handle vertical alignment */
void MathType::HandleAlign(sal_uInt8 nHorAlign, int &rSetAlign)
{
    switch(nHorAlign)
    {
    case 1:
    default:
        rRet.append("alignl {");
        break;
    case 2:
        rRet.append("alignc {");
        break;
    case 3:
        rRet.append("alignr {");
        break;
    }
    rSetAlign++;
}

/* set size of text, complexity due to overuse of signedness as a flag
 * indicator by mathtype file format*/
bool MathType::HandleSize(sal_Int16 nLstSize,sal_Int16 nDefSize, int &rSetSize)
{
    const sal_Int16 nDefaultSize = 12;
    bool bRet=false;
    if (nLstSize < 0)
    {
        if ((-nLstSize/32 != nDefaultSize) && (-nLstSize/32 != nCurSize))
        {
            if (rSetSize)
            {
                rSetSize--;
                rRet.append("}");
                bRet=true;
            }
            if (-nLstSize/32 != nLastSize)
            {
                nLastSize = nCurSize;
                rRet.append(" size ");
                rRet.append(OUString::number(-nLstSize/32));
                rRet.append("{");
                bRet=true;
                rSetSize++;
            }
            nCurSize = -nLstSize/32;
        }
    }
    else
    {
        /*sizetable should theoretically be filled with the default sizes
         *of the various font groupings matching starmaths equivalents
         in aTypeFaces, and a test would be done to see if the new font
         size would be the same as what starmath would have chosen for
         itself anyway in which case the size setting could be ignored*/
        nLstSize = aSizeTable.at(nLstSize);
        nLstSize = nLstSize + nDefSize;
        if (nLstSize != nCurSize)
        {
            if (rSetSize)
            {
                rSetSize--;
                rRet.append("}");
                bRet=true;
            }
            if (nLstSize != nLastSize)
            {
                nLastSize = nCurSize;
                rRet.append(" size ");
                rRet.append(OUString::number(nLstSize));
                rRet.append("{");
                bRet=true;
                rSetSize++;
            }
            nCurSize = nLstSize;
        }
    }
    return bRet;
}

bool MathType::ConvertFromStarMath( SfxMedium& rMedium )
{
    if (!pTree)
        return false;

    SvStream *pStream = rMedium.GetOutStream();
    if ( pStream )
    {
        tools::SvRef<SotStorage> pStor = new SotStorage( pStream, false );

        SvGlobalName aGName(MSO_EQUATION3_CLASSID);
        pStor->SetClass( aGName, SotClipboardFormatId::NONE, "Microsoft Equation 3.0");

        static sal_uInt8 const aCompObj[] = {
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
        tools::SvRef<SotStorageStream> xStor( pStor->OpenSotStream("\1CompObj"));
        xStor->WriteBytes(aCompObj, sizeof(aCompObj));

        static sal_uInt8 const aOle[] = {
            0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
            };
        tools::SvRef<SotStorageStream> xStor2( pStor->OpenSotStream("\1Ole"));
        xStor2->WriteBytes(aOle, sizeof(aOle));
        xStor.clear();
        xStor2.clear();

        tools::SvRef<SotStorageStream> xSrc = pStor->OpenSotStream("Equation Native");
        if ( (!xSrc.is()) || (ERRCODE_NONE != xSrc->GetError()))
            return false;

        pS = xSrc.get();
        pS->SetEndian( SvStreamEndian::LITTLE );

        pS->SeekRel(EQNOLEFILEHDR_SIZE); //Skip 28byte Header and fill it in later
        pS->WriteUChar( 0x03 );
        pS->WriteUChar( 0x01 );
        pS->WriteUChar( 0x01 );
        pS->WriteUChar( 0x03 );
        pS->WriteUChar( 0x00 );
        sal_uInt32 nSize = pS->Tell();
        nPendingAttributes=0;

        HandleNodes(pTree, 0);
        pS->WriteUChar( END );

        nSize = pS->Tell()-nSize;
        pS->Seek(0);
        EQNOLEFILEHDR aHdr(nSize+4+1);
        aHdr.Write(pS);

        pStor->Commit();
    }

    return true;
}


void MathType::HandleNodes(SmNode *pNode,int nLevel)
{
    switch(pNode->GetType())
    {
        case SmNodeType::Attribut:
            HandleAttributes(pNode,nLevel);
            break;
        case SmNodeType::Text:
            HandleText(pNode);
            break;
        case SmNodeType::VerticalBrace:
            HandleVerticalBrace(pNode,nLevel);
            break;
        case SmNodeType::Brace:
            HandleBrace(pNode,nLevel);
            break;
        case SmNodeType::Oper:
            HandleOperator(pNode,nLevel);
            break;
        case SmNodeType::BinVer:
            HandleFractions(pNode,nLevel);
            break;
        case SmNodeType::Root:
            HandleRoot(pNode,nLevel);
            break;
        case SmNodeType::Special:
            {
            SmTextNode *pText = static_cast<SmTextNode *>(pNode);
            //if the token str and the result text are the same then this
            //is to be seen as text, else assume it's a mathchar
            if (pText->GetText() == pText->GetToken().aText)
                HandleText(pText);
            else
                HandleMath(pText);
            }
            break;
        case SmNodeType::Math:
        case SmNodeType::MathIdent:
            HandleMath(pNode);
            break;
        case SmNodeType::SubSup:
            HandleSubSupScript(pNode,nLevel);
            break;
        case SmNodeType::Expression:
        {
            size_t nSize = pNode->GetNumSubNodes();
            for (size_t i = 0; i < nSize; ++i)
            {
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
        }
        case SmNodeType::Table:
            //Root Node, PILE equivalent, i.e. vertical stack
            HandleTable(pNode,nLevel);
            break;
        case SmNodeType::Matrix:
            HandleSmMatrix(static_cast<SmMatrixNode *>(pNode),nLevel);
            break;
        case SmNodeType::Line:
        {
            pS->WriteUChar( 0x0a );
            pS->WriteUChar( LINE );
            size_t nSize = pNode->GetNumSubNodes();
            for (size_t i = 0; i < nSize; ++i)
            {
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            pS->WriteUChar( END );
            break;
        }
        case SmNodeType::Align:
            HandleMAlign(pNode,nLevel);
            break;
        case SmNodeType::Blank:
            pS->WriteUChar( CHAR );
            pS->WriteUChar( 0x98 );
            if (pNode->GetToken().eType == TSBLANK)
                pS->WriteUInt16( 0xEB04 );
            else
                pS->WriteUInt16( 0xEB05 );
            break;
        default:
        {
            size_t nSize = pNode->GetNumSubNodes();
            for (size_t i = 0; i < nSize; ++i)
            {
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
        }
    }
}


int MathType::StartTemplate(sal_uInt16 nSelector,sal_uInt16 nVariation)
{
    int nOldPending=nPendingAttributes;
    pS->WriteUChar( TMPL ); //Template
    pS->WriteUChar( nSelector ); //selector
    pS->WriteUChar( nVariation ); //variation
    pS->WriteUChar( 0x00 ); //options
    pS->WriteUChar( LINE );
    //there's just no way we can now handle any character
    //attributes (from mathtypes perspective) centered
    //over an expression but above template attribute
    //such as widevec and similar constructs
    //we have to drop them
    nPendingAttributes=0;
    return nOldPending;
}

void MathType::EndTemplate(int nOldPendingAttributes)
{
    pS->WriteUChar( END ); //end line
    pS->WriteUChar( END ); //end template
    nPendingAttributes=nOldPendingAttributes;
}


void MathType::HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel)
{
    pS->WriteUChar( MATRIX );
    pS->WriteUChar( 0x00 ); //vAlign ?
    pS->WriteUChar( 0x00 ); //h_just
    pS->WriteUChar( 0x00 ); //v_just
    pS->WriteUChar( pMatrix->GetNumRows() ); //v_just
    pS->WriteUChar( pMatrix->GetNumCols() ); //v_just
    int nBytes=(pMatrix->GetNumRows()+1)*2/8;
    if (((pMatrix->GetNumRows()+1)*2)%8)
        nBytes++;
    for (int j = 0; j < nBytes; j++)
        pS->WriteUChar( 0x00 ); //row_parts
    nBytes=(pMatrix->GetNumCols()+1)*2/8;
    if (((pMatrix->GetNumCols()+1)*2)%8)
        nBytes++;
    for (int k = 0; k < nBytes; k++)
        pS->WriteUChar( 0x00 ); //col_parts
    size_t nSize = pMatrix->GetNumSubNodes();
    for (size_t i = 0; i < nSize; ++i)
    {
        if (SmNode *pTemp = pMatrix->GetSubNode(i))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //end line
        }
    }
    pS->WriteUChar( END );
}


//Root Node, PILE equivalent, i.e. vertical stack
void MathType::HandleTable(SmNode *pNode,int nLevel)
{
    size_t nSize = pNode->GetNumSubNodes();
    //The root of the starmath is a table, if
    //we convert this them each iteration of
    //conversion from starmath to mathtype will
    //add an extra unnecessary level to the
    //mathtype output stack which would grow
    //without bound in a multi step conversion

    if (nLevel == 0)
        pS->WriteUChar( 0x0A ); //initial size

    if ( nLevel || (nSize >1))
    {
        pS->WriteUChar( PILE );
        pS->WriteUChar( nHAlign ); //vAlign ?
        pS->WriteUChar( 0x01 ); //hAlign
    }

    for (size_t i = 0; i < nSize; ++i)
    {
        if (SmNode *pTemp = pNode->GetSubNode(i))
        {
            pS->WriteUChar( LINE );
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END );
        }
    }
    if (nLevel || (nSize>1))
        pS->WriteUChar( END );
}


void MathType::HandleRoot(SmNode *pNode,int nLevel)
{
    SmNode *pTemp;
    pS->WriteUChar( TMPL ); //Template
    pS->WriteUChar( 0x0D ); //selector
    if (pNode->GetSubNode(0))
        pS->WriteUChar( 0x01 ); //variation
    else
        pS->WriteUChar( 0x00 ); //variation
    pS->WriteUChar( 0x00 ); //options

    if (nullptr != (pTemp = pNode->GetSubNode(2)))
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pTemp,nLevel+1);
        pS->WriteUChar( END );
    }

    if (nullptr != (pTemp = pNode->GetSubNode(0)))
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pTemp,nLevel+1);
        pS->WriteUChar( END );
    }
    else
        pS->WriteUChar( LINE|0x10 ); //dummy line


    pS->WriteUChar( END );
}

sal_uInt8 MathType::HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,
    sal_uInt64 *pPos,bool bTest)
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
        pS->WriteUChar( TMPL ); //Template
        pS->WriteUChar( 0x2B ); //selector
        pS->WriteUChar( nVariation2 );
        pS->WriteUChar( 0x00 ); //options

        if (pContent)
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pContent,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );

        pS->WriteUChar( 0x0B );

        SmNode *pTemp;
        if (nullptr != (pTemp = pNode->GetSubNode(CSUB+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
        if (bTest && nullptr != (pTemp = pNode->GetSubNode(CSUP+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
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
    sal_uInt8 nVariation=0xff;
    if (pNode->GetSubNode(LSUP+1))
    {
        nVariation=0;
        if (pNode->GetSubNode(LSUB+1))
            nVariation=2;
    }
    else if ( nullptr != pNode->GetSubNode(LSUB+1) )
        nVariation=1;

    SmNode *pTemp;
    if (nVariation!=0xff)
    {
        pS->WriteUChar( TMPL ); //Template
        pS->WriteUChar( 0x2c ); //selector
        pS->WriteUChar( nVariation );
        pS->WriteUChar( 0x00 ); //options
        pS->WriteUChar( 0x0B );

        if (nullptr != (pTemp = pNode->GetSubNode(LSUB+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
        if (nullptr != (pTemp = pNode->GetSubNode(LSUP+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
        pS->WriteUChar( END );
        nVariation=0xff;
    }


    sal_uInt8 nVariation2=HandleCScript(pNode,nullptr,nLevel);

    if (nullptr != (pTemp = pNode->GetSubNode(0)))
    {
        HandleNodes(pTemp,nLevel+1);
    }

    if (nVariation2 != 0xff)
        pS->WriteUChar( END );

    if (nullptr != (pNode->GetSubNode(RSUP+1)))
    {
        nVariation=0;
        if (pNode->GetSubNode(RSUB+1))
            nVariation=2;
    }
    else if (nullptr != pNode->GetSubNode(RSUB+1))
        nVariation=1;

    if (nVariation!=0xff)
    {
        pS->WriteUChar( TMPL ); //Template
        pS->WriteUChar( 0x0F ); //selector
        pS->WriteUChar( nVariation );
        pS->WriteUChar( 0x00 ); //options
        pS->WriteUChar( 0x0B );

        if (nullptr != (pTemp = pNode->GetSubNode(RSUB+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
        if (nullptr != (pTemp = pNode->GetSubNode(RSUP+1)))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //line
        }
        else
            pS->WriteUChar( LINE|0x10 );
        pS->WriteUChar( END ); //line
    }

    //After subscript mathtype will keep the size of
    //normal text at the subscript size, sigh.
    pS->WriteUChar( 0x0A );
}


void MathType::HandleFractions(SmNode *pNode,int nLevel)
{
    SmNode *pTemp;
    pS->WriteUChar( TMPL ); //Template
    pS->WriteUChar( 0x0E ); //selector
    pS->WriteUChar( 0x00 ); //variation
    pS->WriteUChar( 0x00 ); //options

    pS->WriteUChar( 0x0A );
    pS->WriteUChar( LINE ); //line
    if (nullptr != (pTemp = pNode->GetSubNode(0)))
        HandleNodes(pTemp,nLevel+1);
    pS->WriteUChar( END );

    pS->WriteUChar( 0x0A );
    pS->WriteUChar( LINE ); //line
    if (nullptr != (pTemp = pNode->GetSubNode(2)))
        HandleNodes(pTemp,nLevel+1);
    pS->WriteUChar( END );

    pS->WriteUChar( END );
}


void MathType::HandleBrace(SmNode *pNode,int nLevel)
{
    SmNode *pTemp;
    SmNode *pLeft=pNode->GetSubNode(0);
    SmNode *pRight=pNode->GetSubNode(2);

    pS->WriteUChar( TMPL ); //Template
    bIsReInterpBrace=false;
    sal_uInt8 nBSpec=0x10;
    auto nLoc = pS->Tell();
    if (pLeft)
    {
        switch (pLeft->GetToken().eType)
        {
            case TLANGLE:
                pS->WriteUChar( tmANGLE ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                break;
            case TLBRACE:
                pS->WriteUChar( tmBRACE ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                nBSpec+=3;
                break;
            case TLBRACKET:
                pS->WriteUChar( tmBRACK ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                nBSpec+=3;
                break;
            case TLFLOOR:
                pS->WriteUChar( tmFLOOR ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                break;
            case TLLINE:
                pS->WriteUChar( tmBAR ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                nBSpec+=3;
                break;
            case TLDLINE:
                pS->WriteUChar( tmDBAR ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                break;
            default:
                pS->WriteUChar( tmPAREN ); //selector
                pS->WriteUChar( 0 ); //variation
                pS->WriteUChar( 0 ); //options
                nBSpec+=3;
                break;
        }
    }

    if (nullptr != (pTemp = pNode->GetSubNode(1)))
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pTemp,nLevel+1);
        pS->WriteUChar( END ); //options
    }
    nSpec=nBSpec;
    if (pLeft)
        HandleNodes(pLeft,nLevel+1);
    if (bIsReInterpBrace)
    {
        auto nLoc2 = pS->Tell();
        pS->Seek(nLoc);
        pS->WriteUChar( 0x2D );
        pS->Seek(nLoc2);
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x96 );
        pS->WriteUInt16( 0xEC07 );
        bIsReInterpBrace=false;
    }
    if (pRight)
        HandleNodes(pRight,nLevel+1);
    nSpec=0x0;
    pS->WriteUChar( END );
}


void MathType::HandleVerticalBrace(SmNode *pNode,int nLevel)
{
    SmNode *pTemp;
    pS->WriteUChar( TMPL ); //Template
    if (pNode->GetToken().eType == TUNDERBRACE)
        pS->WriteUChar( tmLHBRACE ); //selector
    else
        pS->WriteUChar( tmUHBRACE ); //selector
    pS->WriteUChar( 0 ); //variation
    pS->WriteUChar( 0 ); //options

    if (nullptr != (pTemp = pNode->GetSubNode(0)))
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pTemp,nLevel+1);
        pS->WriteUChar( END ); //options
    }

    if (nullptr != (pTemp = pNode->GetSubNode(2)))
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pTemp,nLevel+1);
        pS->WriteUChar( END ); //options
    }
    pS->WriteUChar( END );
}

void MathType::HandleOperator(SmNode *pNode,int nLevel)
{
    if (HandleLim(pNode,nLevel))
        return;

    sal_uInt64 nPos;
    sal_uInt8 nVariation;

    switch (pNode->GetToken().eType)
    {
        case TIINT:
        case TIIINT:
        case TLINT:
        case TLLINT:
        case TLLLINT:
            nVariation=HandleCScript(pNode->GetSubNode(0),
                pNode->GetSubNode(1),nLevel,&nPos,false);
            break;
        default:
            nVariation=HandleCScript(pNode->GetSubNode(0),
                pNode->GetSubNode(1),nLevel,&nPos);
            break;
    }

    sal_uInt8 nOldVariation=nVariation;
    sal_uInt8 nIntVariation=nVariation;

    sal_uInt64 nPos2=0;
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
    pS->WriteUChar( TMPL );
    switch(pNode->GetToken().eType)
    {
    case TINT:
    case TINTD:
        if (nOldVariation != 0xff)
            pS->WriteUChar( 0x18 ); //selector
        else
            pS->WriteUChar( 0x15 ); //selector
        pS->WriteUChar( nIntVariation ); //variation
        break;
    case TIINT:
        if (nOldVariation != 0xff)
        {
            pS->WriteUChar( 0x19 );
            pS->WriteUChar( 0x01 );
        }
        else
        {
            pS->WriteUChar( 0x16 );
            pS->WriteUChar( 0x00 );
        }
        break;
    case TIIINT:
        if (nOldVariation != 0xff)
        {
            pS->WriteUChar( 0x1a );
            pS->WriteUChar( 0x01 );
        }
        else
        {
            pS->WriteUChar( 0x17 );
            pS->WriteUChar( 0x00 );
        }
        break;
    case TLINT:
        if (nOldVariation != 0xff)
        {
            pS->WriteUChar( 0x18 );
            pS->WriteUChar( 0x02 );
        }
        else
        {
            pS->WriteUChar( 0x15 );
            pS->WriteUChar( 0x03 );
        }
        break;
    case TLLINT:
        if (nOldVariation != 0xff)
        {
            pS->WriteUChar( 0x19 );
            pS->WriteUChar( 0x00 );
        }
        else
        {
            pS->WriteUChar( 0x16 );
            pS->WriteUChar( 0x02 );
        }
        break;
    case TLLLINT:
        if (nOldVariation != 0xff)
        {
            pS->WriteUChar( 0x1a );
            pS->WriteUChar( 0x00 );
        }
        else
        {
            pS->WriteUChar( 0x17 );
            pS->WriteUChar( 0x02 );
        }
        break;
    case TSUM:
    default:
        pS->WriteUChar( 0x1d );
        pS->WriteUChar( nVariation );
        break;
    case TPROD:
        pS->WriteUChar( 0x1f );
        pS->WriteUChar( nVariation );
        break;
    case TCOPROD:
        pS->WriteUChar( 0x21 );
        pS->WriteUChar( nVariation );
        break;
    }
    pS->WriteUChar( 0 ); //options

    if (nPos2)
        pS->Seek(nPos2);
    else
    {
        pS->WriteUChar( LINE ); //line
        HandleNodes(pNode->GetSubNode(1),nLevel+1);
        pS->WriteUChar( END ); //line
        pS->WriteUChar( LINE|0x10 );
        pS->WriteUChar( LINE|0x10 );
    }

    pS->WriteUChar( 0x0D );
    switch(pNode->GetToken().eType)
    {
    case TSUM:
    default:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x2211 );
        break;
    case TPROD:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x220F );
        break;
    case TCOPROD:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x8B );
        pS->WriteUInt16( 0x2210 );
        break;
    case TIIINT:
    case TLLLINT:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x222B );
        [[fallthrough]];
    case TIINT:
    case TLLINT:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x222B );
        [[fallthrough]];
    case TINT:
    case TINTD:
    case TLINT:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x222B );
        break;
    }
    pS->WriteUChar( END );
    pS->WriteUChar( 0x0A );
}


bool MathType::HandlePile(int &rSetAlign, int nLevel, sal_uInt8 nSelector, sal_uInt8 nVariation)
{
    sal_uInt8 nVAlign;
    pS->ReadUChar( nHAlign );
    pS->ReadUChar( nVAlign );

    HandleAlign(nHAlign, rSetAlign);

    rRet.append(" stack {\n");
    bool bRet = HandleRecords( nLevel+1, nSelector, nVariation, -1, -1 );
    int nRemoveFrom = rRet.getLength() >= 3 ? rRet.getLength() - 3 : 0;
    rRet.remove(nRemoveFrom, 2);
    rRet.append("} ");

    while (rSetAlign)
    {
        rRet.append("} ");
        rSetAlign--;
    }
    return bRet;
}

bool MathType::HandleMatrix(int nLevel, sal_uInt8 nSelector, sal_uInt8 nVariation)
{
    sal_uInt8 nH_just,nV_just,nRows,nCols,nVAlign;
    pS->ReadUChar( nVAlign );
    pS->ReadUChar( nH_just );
    pS->ReadUChar( nV_just );
    pS->ReadUChar( nRows );
    pS->ReadUChar( nCols );
    int nBytes = ((nRows+1)*2)/8;
    if (((nRows+1)*2)%8)
        nBytes++;
    pS->SeekRel(nBytes);
    nBytes = ((nCols+1)*2)/8;
    if (((nCols+1)*2)%8)
        nBytes++;
    pS->SeekRel(nBytes);
    rRet.append(" matrix {\n");
    bool bRet = HandleRecords( nLevel+1, nSelector, nVariation, nRows, nCols );

    sal_Int32 nI = rRet.lastIndexOf('#');
    if (nI > 0)
        if (rRet[nI-1] != '#')  //missing column
            rRet.append("{}");

    rRet.append("\n} ");
    return bRet;
}

bool MathType::HandleTemplate(int nLevel, sal_uInt8 &rSelector,
    sal_uInt8 &rVariation, sal_Int32 &rLastTemplateBracket)
{
    sal_uInt8 nOption; //This appears utterly unused
    pS->ReadUChar( rSelector );
    pS->ReadUChar( rVariation );
    pS->ReadUChar( nOption );
    OSL_ENSURE(rSelector < 48,"Selector out of range");
    if ((rSelector >= 21) && (rSelector <=26))
    {
        OSL_ENSURE(nOption < 2,"Option out of range");
    }
    else if (rSelector <= 12)
    {
        OSL_ENSURE(nOption < 3,"Option out of range");
    }

    //For the (broken) case where one subscript template ends, and there is
    //another one after it, mathtype handles it as if the second one was
    //inside the first one and renders it as sub of sub
    bool bRemove=false;
    if ( (rSelector == 0xf) && (rLastTemplateBracket != -1) )
    {
        bRemove=true;
        for (sal_Int32 nI = rLastTemplateBracket+1; nI < rRet.getLength(); nI++ )
            if (rRet[nI] != ' ')
            {
                bRemove=false;
                break;
            }
    }

    //suborderlist
    bool bRet = HandleRecords( nLevel+1, rSelector, rVariation );

    if (bRemove)
    {
        if (rLastTemplateBracket < rRet.getLength())
            rRet.remove(rLastTemplateBracket, 1);
        rRet.append("} ");
        rLastTemplateBracket = -1;
    }
    if (rSelector == 0xf)
        rLastTemplateBracket = rRet.lastIndexOf('}');
    else
        rLastTemplateBracket = -1;

    rSelector = sal::static_int_cast< sal_uInt8 >(-1);
    return bRet;
}

void MathType::HandleEmblishments()
{
    sal_uInt8 nEmbel;
    do
    {
        pS->ReadUChar( nEmbel );
        if (!pS->good())
            break;
        switch (nEmbel)
        {
        case 0x02:
            rRet.append(" dot ");
            break;
        case 0x03:
            rRet.append(" ddot ");
            break;
        case 0x04:
            rRet.append(" dddot ");
            break;
        case 0x05:
            if (!nPostSup)
            {
                sPost.append(" sup {}");
                nPostSup = sPost.getLength();
            }
            sPost.insert(nPostSup-1," ' ");
            nPostSup += 3;
            break;
        case 0x06:
            if (!nPostSup)
            {
                sPost.append(" sup {}");
                nPostSup = sPost.getLength();
            }
            sPost.insert(nPostSup-1," '' ");
            nPostSup += 4;
            break;
        case 0x07:
            if (!nPostlSup)
            {
                sPost.append(" lsup {}");
                nPostlSup = sPost.getLength();
            }
            sPost.insert(nPostlSup-1," ' ");
            nPostlSup += 3;
            break;
        case 0x08:
            rRet.append(" tilde ");
            break;
        case 0x09:
            rRet.append(" hat ");
            break;
        case 0x0b:
            rRet.append(" vec ");
            break;
        case 0x10:
            rRet.append(" overstrike ");
            break;
        case 0x11:
            rRet.append(" bar ");
            break;
        case 0x12:
            if (!nPostSup)
            {
                sPost.append(" sup {}");
                nPostSup = sPost.getLength();
            }
            sPost.insert(nPostSup-1," ''' ");
            nPostSup += 5;
            break;
        case 0x14:
            rRet.append(" breve ");
            break;
        default:
            OSL_ENSURE(nEmbel < 21,"Embel out of range");
            break;
        }
        if (nVersion < 3)
            break;
    }while (nEmbel);
}

void MathType::HandleSetSize()
{
    sal_uInt8 nTemp;
    pS->ReadUChar( nTemp );
    switch (nTemp)
    {
        case 101:
            pS->ReadInt16( nLSize );
            nLSize = -nLSize;
            break;
        case 100:
            pS->ReadUChar( nTemp );
            nLSize = nTemp;
            pS->ReadInt16( nDSize );
            break;
        default:
            nLSize = nTemp;
            pS->ReadUChar( nTemp );
            nDSize = nTemp-128;
            break;
    }
}

bool MathType::HandleChar(sal_Int32 &rTextStart, int &rSetSize, int nLevel,
    sal_uInt8 nTag, sal_uInt8 nSelector, sal_uInt8 nVariation, bool bSilent)
{
    sal_Unicode nChar(0);
    bool bRet = true;

    if (xfAUTO(nTag))
    {
    //This is a candidate for function recognition, whatever
    //that is!
    }

    sal_uInt8 nOldTypeFace = nTypeFace;
    pS->ReadUChar( nTypeFace );
    if (nVersion < 3)
    {
        sal_uInt8 nChar8(0);
        pS->ReadUChar( nChar8 );
        nChar = nChar8;
    }
    else
        pS->ReadUtf16( nChar );

    /*
    bad character, old mathtype < 3 has these
    */
    if (nChar < 0x20)
        return bRet;

    if (xfEMBELL(nTag))
    {
        //A bit tricky, the character emblishments for
        //mathtype can all be listed after each other, in
        //starmath some must go before the character and some
        //must go after. In addition some of the emblishments
        //may repeated and in starmath some of these groups
        //must be gathered together. sPost is the portion that
        //follows the char and nPostSup and nPostlSup are the
        //indexes at which this class of emblishment is
        //collated together
        sPost = "";
        nPostSup = nPostlSup = 0;
        int nOriglen=rRet.getLength()-rTextStart;
        rRet.append(" {");  // #i24340# make what would be "vec {A}_n" become "{vec {A}}_n"
        if ((!bSilent) && (nOriglen > 1))
            rRet.append("\"");
        bRet = HandleRecords( nLevel+1, nSelector, nVariation );
        if (!bSilent)
        {
            if (nOriglen > 1)
            {
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet.insert(std::min(rTextStart, rRet.getLength()), aStr);

                aStr.clear();
                TypeFaceToString(aStr,nTypeFace);
                rRet.append(aStr).append("{");
            }
            else
                rRet.append(" {");
            rTextStart = rRet.getLength();
        }
    }

    if (!bSilent)
    {
        sal_Int32 nOldLen = rRet.getLength();
        if (
            HandleSize(nLSize,nDSize,rSetSize) ||
            (nOldTypeFace != nTypeFace)
           )
        {
            if ((nOldLen - rTextStart) > 1)
            {
                rRet.insert(nOldLen, "\"");
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet.insert(rTextStart,aStr);
            }
            rTextStart = rRet.getLength();
        }
        nOldLen = rRet.getLength();
        if (!LookupChar(nChar,rRet,nVersion,nTypeFace))
        {
            if (nOldLen - rTextStart > 1)
            {
                rRet.insert(nOldLen, "\"");
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet.insert(rTextStart, aStr);
            }
            rTextStart = rRet.getLength();
        }
        lcl_PrependDummyTerm(rRet, rTextStart);
    }

    if ((xfEMBELL(nTag)) && (!bSilent))
    {
        rRet.append("}}").append(sPost);  // #i24340# make what would be "vec {A}_n" become "{vec {A}}_n"
        rTextStart = rRet.getLength();
    }
    return bRet;
}

bool MathType::HandleLim(SmNode *pNode,int nLevel)
{
    bool bRet=false;
    //Special case for the "lim" option in StarMath
    if ((pNode->GetToken().eType == TLIM)
        || (pNode->GetToken().eType == TLIMSUP)
        || (pNode->GetToken().eType == TLIMINF)
        )
    {
        if (pNode->GetSubNode(1))
        {
            sal_uInt8 nVariation2=HandleCScript(pNode->GetSubNode(0),nullptr,
                nLevel);

            pS->WriteUChar( 0x0A );
            pS->WriteUChar( LINE ); //line
            pS->WriteUChar( CHAR|0x10 );
            pS->WriteUChar( 0x82 );
            pS->WriteUInt16( 'l' );
            pS->WriteUChar( CHAR|0x10 );
            pS->WriteUChar( 0x82 );
            pS->WriteUInt16( 'i' );
            pS->WriteUChar( CHAR|0x10 );
            pS->WriteUChar( 0x82 );
            pS->WriteUInt16( 'm' );

            if (pNode->GetToken().eType == TLIMSUP)
            {
                pS->WriteUChar( CHAR ); //some space
                pS->WriteUChar( 0x98 );
                pS->WriteUInt16( 0xEB04 );

                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 's' );
                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 'u' );
                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 'p' );
            }
            else if (pNode->GetToken().eType == TLIMINF)
            {
                pS->WriteUChar( CHAR ); //some space
                pS->WriteUChar( 0x98 );
                pS->WriteUInt16( 0xEB04 );

                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 'i' );
                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 'n' );
                pS->WriteUChar( CHAR|0x10 );
                pS->WriteUChar( 0x82 );
                pS->WriteUInt16( 'f' );
            }


            pS->WriteUChar( CHAR ); //some space
            pS->WriteUChar( 0x98 );
            pS->WriteUInt16( 0xEB04 );

            if (nVariation2 != 0xff)
            {
                pS->WriteUChar( END );
                pS->WriteUChar( END );
            }
            HandleNodes(pNode->GetSubNode(1),nLevel+1);
            bRet = true;
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
    size_t nSize = pNode->GetNumSubNodes();
    for (size_t i = 0; i < nSize; ++i)
    {
        if (SmNode *pTemp = pNode->GetSubNode(i))
            HandleNodes(pTemp,nLevel+1);
    }
    nHAlign=nPushedHAlign;
}

void MathType::HandleMath(SmNode *pNode)
{
    if (pNode->GetToken().eType == TMLINE)
    {
        pS->WriteUChar( END );
        pS->WriteUChar( LINE );
        bIsReInterpBrace=true;
        return;
    }
    SmMathSymbolNode *pTemp = static_cast<SmMathSymbolNode *>(pNode);
    for(sal_Int32 i=0;i<pTemp->GetText().getLength();i++)
    {
        sal_Unicode nArse = SmTextNode::ConvertSymbolToUnicode(pTemp->GetText()[i]);
        if ((nArse == 0x2224) || (nArse == 0x2288) || (nArse == 0x2285) ||
            (nArse == 0x2289))
        {
            pS->WriteUChar( CHAR|0x20 );
        }
        else if (nPendingAttributes &&
                (i == ((pTemp->GetText().getLength()+1)/2)-1))
            {
                pS->WriteUChar( 0x22 );
            }
        else
            pS->WriteUChar( CHAR ); //char without formula recognition
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
        else if ((nArse == 0x2F) || (nArse == 0x2225))
            nBias = 0x2; //typeface
        else if ((nArse > 0x2000) || (nArse == 0x00D7))
            nBias = 0x6; //typeface
        else if (nArse == 0x3d1)
            nBias = 0x4;
        else if ((nArse > 0xFF) && ((nArse < 0x393) || (nArse > 0x3c9)))
            nBias = 0xB; //typeface
        else
            nBias = 0x3; //typeface

        pS->WriteUChar( nSpec+nBias+128 ); //typeface

        if (nArse == 0x2224)
        {
            pS->WriteUInt16( 0x7C );
            pS->WriteUChar( EMBEL );
            pS->WriteUChar( 0x0A );
            pS->WriteUChar( END ); //end embel
            pS->WriteUChar( END ); //end embel
        }
        else if (nArse == 0x2225)
            pS->WriteUInt16( 0xEC09 );
        else if (nArse == 0xE421)
            pS->WriteUInt16( 0x2265 );
        else if (nArse == 0x230A)
            pS->WriteUInt16( 0xF8F0 );
        else if (nArse == 0x230B)
            pS->WriteUInt16( 0xF8FB );
        else if (nArse == 0xE425)
            pS->WriteUInt16( 0x2264 );
        else if (nArse == 0x226A)
        {
            pS->WriteUInt16( 0x3C );
            pS->WriteUChar( CHAR );
            pS->WriteUChar( 0x98 );
            pS->WriteUInt16( 0xEB01 );
            pS->WriteUChar( CHAR );
            pS->WriteUChar( 0x86 );
            pS->WriteUInt16( 0x3c );
        }
        else if (nArse == 0x2288)
        {
            pS->WriteUInt16( 0x2286 );
            pS->WriteUChar( EMBEL );
            pS->WriteUChar( 0x0A );
            pS->WriteUChar( END ); //end embel
            pS->WriteUChar( END ); //end embel
        }
        else if (nArse == 0x2289)
        {
            pS->WriteUInt16( 0x2287 );
            pS->WriteUChar( EMBEL );
            pS->WriteUChar( 0x0A );
            pS->WriteUChar( END ); //end embel
            pS->WriteUChar( END ); //end embel
        }
        else if (nArse == 0x2285)
        {
            pS->WriteUInt16( 0x2283 );
            pS->WriteUChar( EMBEL );
            pS->WriteUChar( 0x0A );
            pS->WriteUChar( END ); //end embel
            pS->WriteUChar( END ); //end embel
        }
        else
            pS->WriteUInt16( nArse );
    }
    nPendingAttributes = 0;
}

void MathType::HandleAttributes(SmNode *pNode,int nLevel)
{
    int nOldPending = 0;
    SmNode *pTemp       = nullptr;
    SmTextNode *pIsText = nullptr;

    if (nullptr != (pTemp = pNode->GetSubNode(0)))
    {
        pIsText = static_cast<SmTextNode *>(pNode->GetSubNode(1));

        switch (pTemp->GetToken().eType)
        {
        case TWIDEVEC:
            //there's just no way we can now handle any character
            //attributes (from mathtypes perspective) centered
            //over an expression but above template attributes
            //such as widevec and similar constructs
            //we have to drop them
            nOldPending = StartTemplate(0x2f,0x01);
            break;
        case TCHECK: //Not Exportable
        case TACUTE: //Not Exportable
        case TGRAVE: //Not Exportable
        case TCIRCLE: //Not Exportable
        case TWIDEHARPOON: //Not Exportable
        case TWIDETILDE: //Not Exportable
        case TWIDEHAT: //Not Exportable
            break;
        case TUNDERLINE:
            nOldPending = StartTemplate(0x10);
            break;
        case TOVERLINE: //If the next node is not text
                        //or text with more than one char
            if ((pIsText->GetToken().eType != TTEXT) ||
                (pIsText->GetText().getLength() > 1))
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
            if ((pIsText->GetToken().eType != TTEXT) ||
                (pIsText->GetText().getLength() > 1))
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
        if ((nInsertion != 0) && nullptr != (pTemp = pNode->GetSubNode(0)))
        {
            auto nPos = pS->Tell();
            nInsertion--;
            pS->Seek(nInsertion);
            switch(pTemp->GetToken().eType)
            {
            case TACUTE: //Not Exportable
            case TGRAVE: //Not Exportable
            case TCIRCLE: //Not Exportable
                break;
            case TCDOT:
                pS->WriteUChar( 2 );
                break;
            case TDDOT:
                pS->WriteUChar( 3 );
                break;
            case TDDDOT:
                pS->WriteUChar( 4 );
                break;
            case TTILDE:
                pS->WriteUChar( 8 );
                break;
            case THAT:
                pS->WriteUChar( 9 );
                break;
            case TVEC:
                pS->WriteUChar( 11 );
                break;
            case TOVERSTRIKE:
                pS->WriteUChar( 16 );
                break;
            case TOVERLINE:
                if ((pIsText->GetToken().eType == TTEXT) &&
                    (pIsText->GetText().getLength() == 1))
                    pS->WriteUChar( 17 );
                break;
            case TBREVE:
                pS->WriteUChar( 20 );
                break;
            case TWIDEVEC:
            case TWIDEHARPOON:
            case TUNDERLINE:
            case TWIDETILDE:
            case TWIDEHAT:
                break;
            case TBAR:
                pS->WriteUChar( 17 );
                break;
            default:
                pS->WriteUChar( 2 );
                break;
            }
            pS->Seek(nPos);
        }
    }
}

void MathType::HandleText(SmNode *pNode)
{
    SmTextNode *pTemp = static_cast<SmTextNode *>(pNode);
    for(sal_Int32 i=0;i<pTemp->GetText().getLength();i++)
    {
        if (nPendingAttributes &&
            (i == ((pTemp->GetText().getLength()+1)/2)-1))
        {
            pS->WriteUChar( 0x22 );     //char, with attributes right
                                //after the character
        }
        else
            pS->WriteUChar( CHAR );

        sal_uInt8 nFace = 0x1;
        if (pNode->GetFont().GetItalic() == ITALIC_NORMAL)
            nFace = 0x3;
        else if (pNode->GetFont().GetWeight() == WEIGHT_BOLD)
            nFace = 0x7;
        pS->WriteUChar( nFace+128 ); //typeface
        sal_uInt16 nChar = pTemp->GetText()[i];
        pS->WriteUInt16( SmTextNode::ConvertSymbolToUnicode(nChar) );

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
        if (nPendingAttributes &&
            (i == ((pTemp->GetText().getLength()+1)/2)-1))
        {
            pS->WriteUChar( EMBEL );
            while (nPendingAttributes)
            {
                pS->WriteUChar( 2 );
                //wedge the attributes in here and clear
                //the pending stack
                nPendingAttributes--;
            }
            nInsertion=pS->Tell();
            pS->WriteUChar( END ); //end embel
            pS->WriteUChar( END ); //end embel
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportMathType(SvStream &rStream)
{
    OUStringBuffer sText;
    MathType aEquation(sText);
    bool bRet = false;
    try
    {
        bRet = aEquation.Parse(&rStream);
    }
    catch (const std::out_of_range&)
    {
    }
    return bRet;
}

using namespace MathType2Namespace;

/**
  * Greek
  */

// Greek
const MathType2 aGreekMathTypeTable[] =
{
    // Capital
    { MS_UALPHA,         TGREEK,     TG::Character,     "char Alpha",       "Alpha",       "&Alpha;",       "\\Alpha"},
    { MS_UBETA,          TGREEK,     TG::Character,     "char Beta",        "Beta",        "&Beta;",        "\\Beta"},
    { MS_UGAMMA,         TGREEK,     TG::Character,     "char Gamma",       "Gamma",       "&Gamma;",       "\\Gamma"},
    { MS_UDELTA,         TGREEK,     TG::Character,     "char Delta",       "Delta",       "&Delta;",       "\\Delta"},
    { MS_UEPSILON,       TGREEK,     TG::Character,     "char Epsilon",     "Epsilon",     "&Epsilon;",     "\\Epsilon"},
    { MS_UZETA,          TGREEK,     TG::Character,     "char Zeta",        "Zeta",        "&Zeta;",        "\\Zeta"},
    { MS_UETA,           TGREEK,     TG::Character,     "char Eta",         "Eta",         "&Eta;",         "\\Eta"},
    { MS_UTHETA,         TGREEK,     TG::Character,     "char Theta",       "Theta",       "&Theta;",       "\\Theta"},
    { MS_UIOTA,          TGREEK,     TG::Character,     "char Iota",        "Iota",        "&Iota;",        "\\Iota"},
    { MS_UKAPPA,         TGREEK,     TG::Character,     "char Kappa",       "Kappa",       "&Kappa;",       "\\Kappa"},
    { MS_ULAMBDA,        TGREEK,     TG::Character,     "char Lambda",      "Lambda",      "&Lambda;",      "\\Lambda"},
    { MS_UMU,            TGREEK,     TG::Character,     "char Mu",          "Mu",          "&Mu;",          "\\Mu"},
    { MS_UMU,            TGREEK,     TG::Character,     "char My",          "My",          "&Mu;",          "\\Mu"},
    { MS_UNU,            TGREEK,     TG::Character,     "char Nu",          "Nu",          "&Nu;",          "\\Nu"},
    { MS_UXI,            TGREEK,     TG::Character,     "char Xi",          "Xi",          "&Xi;",          "\\Xi"},
    { MS_UOMICRON,       TGREEK,     TG::Character,     "char Omicron",     "Omicron",     "&Omicron;",     "\\Omicron"},
    { MS_UPI,            TGREEK,     TG::Character,     "char Pi",          "Pi",          "&Pi;",          "\\Pi"},
    { MS_URHO,           TGREEK,     TG::Character,     "char Rho",         "Rho",         "&Rho;",         "\\Rho"},
    { MS_USIGMA,         TGREEK,     TG::Character,     "char Sigma",       "Sigma",       "&Sigma;",       "\\Sigma"},
    { MS_UTAU,           TGREEK,     TG::Character,     "char Tau",         "Tau",         "&Tau;",         "\\Tau"},
    { MS_UUPSILON,       TGREEK,     TG::Character,     "char Upsilon",     "Upsilon",     "&Upsilon;",     "\\Upsilon"},
    { MS_UUPSILON,       TGREEK,     TG::Character,     "char Ypsilon",     "Ypsilon",     "&Upsilon;",     "\\Upsilon"},
    { MS_UPHI,           TGREEK,     TG::Character,     "char Phi",         "Phi",         "&Phi;",         "\\Phi"},
    { MS_UPSI,           TGREEK,     TG::Character,     "char Psi",         "Psi",         "&Psi;",         "\\Psi"},
    { MS_UCHI,           TGREEK,     TG::Character,     "char Chi",         "Chi",         "&Chi;",         "\\Chi"},
    { MS_UOMEGA,         TGREEK,     TG::Character,     "char Omega",       "Omega",       "&Omega;",       "\\Omega"},
    // Lower case
    { MS_LALPHA,         TGREEK,     TG::Character,     "char alpha",       "alpha",       "&alpha;",       "\\alpha"},
    { MS_LBETA,          TGREEK,     TG::Character,     "char beta",        "beta",        "&beta;",        "\\beta"},
    { MS_LGAMMA,         TGREEK,     TG::Character,     "char gamma",       "gamma",       "&gamma;",       "\\gamma"},
    { MS_LDELTA,         TGREEK,     TG::Character,     "char delta",       "delta",       "&delta;",       "\\delta"},
    { MS_LEPSILON,       TGREEK,     TG::Character,     "char epsilon",     "epsilon",     "&epsilon;",     "\\epsilon"},
    { MS_LZETA,          TGREEK,     TG::Character,     "char zeta",        "zeta",        "&zeta;",        "\\zeta"},
    { MS_LETA,           TGREEK,     TG::Character,     "char eta",         "eta",         "&eta;",         "\\eta"},
    { MS_LTHETA,         TGREEK,     TG::Character,     "char theta",       "theta",       "&theta;",       "\\theta"},
    { MS_LIOTA,          TGREEK,     TG::Character,     "char iota",        "iota",        "&iota;",        "\\iota"},
    { MS_LKAPPA,         TGREEK,     TG::Character,     "char kappa",       "kappa",       "&kappa;",       "\\kappa"},
    { MS_LLAMBDA,        TGREEK,     TG::Character,     "char lambda",      "lambda",      "&lambda;",      "\\lambda"},
    { MS_LMU,            TGREEK,     TG::Character,     "char mu",          "mu",          "&mu;",          "\\mu"},
    { MS_LMU,            TGREEK,     TG::Character,     "char my",          "my",          "&mu;",          "\\mu"},
    { MS_LNU,            TGREEK,     TG::Character,     "char nu",          "nu",          "&nu;",          "\\nu"},
    { MS_LXI,            TGREEK,     TG::Character,     "char xi",          "xi",          "&xi;",          "\\xi"},
    { MS_LOMICRON,       TGREEK,     TG::Character,     "char omicron",     "omicron",     "&omicron;",     "\\omicron"},
    { MS_LPI,            TGREEK,     TG::Character,     "char pi",          "pi",          "&pi;",          "\\pi"},
    { MS_LRHO,           TGREEK,     TG::Character,     "char rho",         "rho",         "&rho;",         "\\rho"},
    { MS_LSIGMA,         TGREEK,     TG::Character,     "char sigma",       "sigma",       "&sigma;",       "\\sigma"},
    { MS_LTAU,           TGREEK,     TG::Character,     "char tau",         "tau",         "&tau;",         "\\tau"},
    { MS_LUPSILON,       TGREEK,     TG::Character,     "char upsilon",     "upsilon",     "&upsilon;",     "\\upsilon"},
    { MS_LUPSILON,       TGREEK,     TG::Character,     "char ypsilon",     "ypsilon",     "&upsilon;",     "\\upsilon"},
    { MS_LPHI,           TGREEK,     TG::Character,     "char phi",         "phi",         "&phi;",         "\\phi"},
    { MS_LPSI,           TGREEK,     TG::Character,     "char psi",         "psi",         "&psi;",         "\\psi"},
    { MS_LCHI,           TGREEK,     TG::Character,     "char chi",         "chi",         "&chi;",         "\\chi"},
    { MS_LOMEGA,         TGREEK,     TG::Character,     "char omega",       "omega",       "&omega;",       "\\omega"},
    //varletter
    { MS_UTHETAS,        TGREEK,     TG::Character,     "char Vartheta",    "Vartheta",    "&#;",           "\\Vartheta"},
    { MS_EPSILONLUNATE,  TGREEK,     TG::Character,     "char varepsilon",  "varepsilon",  "&#;",           "\\varepsilon"},
    { MS_LTHETAS,        TGREEK,     TG::Character,     "char vartheta",    "vartheta",    "&#;",           "\\vartheta"},
    { MS_LKAPPAS,        TGREEK,     TG::Character,     "char varkappa",    "varkappa",    "&#;",           "\\varkappa"},
    { MS_LPHIS,          TGREEK,     TG::Character,     "char varphi",      "varphi",      "&#;",           "\\varphi"},
    { MS_LRHOS,          TGREEK,     TG::Character,     "char varrho",      "varrho",      "&#;",           "\\varrho"},
    { MS_LPIS,           TGREEK,     TG::Character,     "char varpi",       "varpi",       "&#;",           "\\varpi"},
    //Back epsilon
    { MS_UBEPSILON,      TBACKEPSILON,     TG::Standalone,     "char backepsilon",       "backepsilon",       "&#;",      nullptr}
};

// Greek bold
const MathType2 aGreekBoldMathTypeTable[] =
{
    // Capital
    { MS_B_UALPHA,        TGREEK,   TG::Character,    "char bAlpha",      "bAlpha",       "&#",    "\\boldsymbol{\\Alpha}"},
    { MS_B_UBETA,         TGREEK,   TG::Character,    "char bBeta",       "bBeta",        "&#",    "\\boldsymbol{\\Beta}"},
    { MS_B_UGAMMA,        TGREEK,   TG::Character,    "char bGamma",      "bGamma",       "&#",    "\\boldsymbol{\\Gamma}"},
    { MS_B_UDELTA,        TGREEK,   TG::Character,    "char bDelta",      "bDelta",       "&#",    "\\boldsymbol{\\Delta}"},
    { MS_B_UEPSILON,      TGREEK,   TG::Character,    "char bEpsilon",    "bEpsilon",     "&#",    "\\boldsymbol{\\Epsilon}"},
    { MS_B_UZETA,         TGREEK,   TG::Character,    "char bZeta",       "bZeta",        "&#",    "\\boldsymbol{\\Zeta}"},
    { MS_B_UETA,          TGREEK,   TG::Character,    "char bEta",        "bEta",         "&#",    "\\boldsymbol{\\Eta}"},
    { MS_B_UTHETA,        TGREEK,   TG::Character,    "char bTheta",      "bTheta",       "&#",    "\\boldsymbol{\\Theta}"},
    { MS_B_UIOTA,         TGREEK,   TG::Character,    "char bIota",       "bIota",        "&#",    "\\boldsymbol{\\Iota}"},
    { MS_B_UKAPPA,        TGREEK,   TG::Character,    "char bKappa",      "bKappa",       "&#",    "\\boldsymbol{\\Kappa}"},
    { MS_B_ULAMBDA,       TGREEK,   TG::Character,    "char bLambda",     "bLambda",      "&#",    "\\boldsymbol{\\Lambda}"},
    { MS_B_UMU,           TGREEK,   TG::Character,    "char bMu",         "bMu",          "&#",    "\\boldsymbol{\\Mu}"},
    { MS_B_UMU,           TGREEK,   TG::Character,    "char bMy",         "bMy",          "&#",    "\\boldsymbol{\\Mu}"},
    { MS_B_UNU,           TGREEK,   TG::Character,    "char bNu",         "bNu",          "&#",    "\\boldsymbol{\\Nu}"},
    { MS_B_UXI,           TGREEK,   TG::Character,    "char bXi",         "bXi",          "&#",    "\\boldsymbol{\\Xi}"},
    { MS_B_UOMICRON,      TGREEK,   TG::Character,    "char bOmicron",    "bOmicron",     "&#",    "\\boldsymbol{\\Omicron}"},
    { MS_B_UPI,           TGREEK,   TG::Character,    "char bPi",         "bPi",          "&#",    "\\boldsymbol{\\Pi}"},
    { MS_B_URHO,          TGREEK,   TG::Character,    "char bRho",        "bRho",         "&#",    "\\boldsymbol{\\Rho}"},
    { MS_B_USIGMA,        TGREEK,   TG::Character,    "char bSigma",      "bSigma",       "&#",    "\\boldsymbol{\\Sigma}"},
    { MS_B_UTAU,          TGREEK,   TG::Character,    "char bTau",        "bTau",         "&#",    "\\boldsymbol{\\Tau}"},
    { MS_B_UUPSILON,      TGREEK,   TG::Character,    "char bUpsion",     "bUpsilon",     "&#",    "\\boldsymbol{\\Upsilon}"},
    { MS_B_UUPSILON,      TGREEK,   TG::Character,    "char bYpsilon",    "bYpsilon",     "&#",    "\\boldsymbol{\\Upsilon}"},
    { MS_B_UPHI,          TGREEK,   TG::Character,    "char bPhi",        "bPhi",         "&#",    "\\boldsymbol{\\Phi}"},
    { MS_B_UPSI,          TGREEK,   TG::Character,    "char bPsi",        "bPsi",         "&#",    "\\boldsymbol{\\Psi}"},
    { MS_B_UCHI,          TGREEK,   TG::Character,    "char bChi",        "bChi",         "&#",    "\\boldsymbol{\\Chi}"},
    { MS_B_UOMEGA,        TGREEK,   TG::Character,    "char bOmega",      "bOmega",       "&#",    "\\boldsymbol{\\Omega}"},
    // Lower case
    { MS_B_LALPHA,        TGREEK,   TG::Character,    "char balpha",         "balpha",       "&#",    "\\boldsymbol{\\alpha}"},
    { MS_B_LBETA,         TGREEK,   TG::Character,    "char bbeta",          "bbeta",        "&#",    "\\boldsymbol{\\beta}"},
    { MS_B_LGAMMA,        TGREEK,   TG::Character,    "char bgamma",         "bgamma",       "&#",    "\\boldsymbol{\\gamma}"},
    { MS_B_LDELTA,        TGREEK,   TG::Character,    "char bdelta",         "bdelta",       "&#",    "\\boldsymbol{\\delta}"},
    { MS_B_LEPSILON,      TGREEK,   TG::Character,    "char bepsilon"  ,     "bepsilon",     "&#",    "\\boldsymbol{\\epsilon}"},
    { MS_B_LZETA,         TGREEK,   TG::Character,    "char bzeta",          "bzeta",        "&#",    "\\boldsymbol{\\zeta}"},
    { MS_B_LETA,          TGREEK,   TG::Character,    "char beta",           "beta",         "&#",    "\\boldsymbol{\\eta}"},
    { MS_B_LTHETA,        TGREEK,   TG::Character,    "char btheta",         "btheta",       "&#",    "\\boldsymbol{\\theta}"},
    { MS_B_LIOTA,         TGREEK,   TG::Character,    "char biota",          "biota",        "&#",    "\\boldsymbol{\\iota}"},
    { MS_B_LKAPPA,        TGREEK,   TG::Character,    "char bkappa",         "bkappa",       "&#",    "\\boldsymbol{\\kappa}"},
    { MS_B_LLAMBDA,       TGREEK,   TG::Character,    "char blambda",        "blambda",      "&#",    "\\boldsymbol{\\lambda}"},
    { MS_B_LMU,           TGREEK,   TG::Character,    "char bmu",            "bmu",          "&#",    "\\boldsymbol{\\mu}"},
    { MS_B_LMU,           TGREEK,   TG::Character,    "char bmy",            "bmy",          "&#",    "\\boldsymbol{\\mu}"},
    { MS_B_LNU,           TGREEK,   TG::Character,    "char bnu",            "bnu",          "&#",    "\\boldsymbol{\\nu}"},
    { MS_B_LXI,           TGREEK,   TG::Character,    "char bxi",            "bxi",          "&#",    "\\boldsymbol{\\xi}"},
    { MS_B_LOMICRON,      TGREEK,   TG::Character,    "char bomicron",       "bomicron",     "&#",    "\\boldsymbol{\\omicron}"},
    { MS_B_LPI,           TGREEK,   TG::Character,    "char bpi",            "bpi",          "&#",    "\\boldsymbol{\\pi}"},
    { MS_B_LRHO,          TGREEK,   TG::Character,    "char brho",           "brho",         "&#",    "\\boldsymbol{\\rho}"},
    { MS_B_LSIGMA,        TGREEK,   TG::Character,    "char bsigma",         "bsigma",       "&#",    "\\boldsymbol{\\sigma}"},
    { MS_B_LTAU,          TGREEK,   TG::Character,    "char btau",           "btau",         "&#",    "\\boldsymbol{\\tau}"},
    { MS_B_LUPSILON,      TGREEK,   TG::Character,    "char bupsilon",       "bupsilon",     "&#",    "\\boldsymbol{\\upsilon}"},
    { MS_B_LUPSILON,      TGREEK,   TG::Character,    "char bypsilon",       "bypsilon",     "&#",    "\\boldsymbol{\\upsilon}"},
    { MS_B_LPHI,          TGREEK,   TG::Character,    "char bphi",           "bphi",         "&#",    "\\boldsymbol{\\phi}"},
    { MS_B_LPSI,          TGREEK,   TG::Character,    "char bpsi",           "bpsi",         "&#",    "\\boldsymbol{\\psi}"},
    { MS_B_LCHI,          TGREEK,   TG::Character,    "char bchi",           "bchi",         "&#",    "\\boldsymbol{\\chi}"},
    { MS_B_LOMEGA,        TGREEK,   TG::Character,    "char bomega",         "bomega",       "&#",    "\\boldsymbol{\\omega}"},
    //varletter
    { MS_B_UTHETAS,       TGREEK,   TG::Character,    "char bVartheta",      "bVartheta",    "&#;",   "\\boldsymbol{\\Vartheta}"},
    { MS_B_EPSILONLUNATE, TGREEK,   TG::Character,    "char bvarepsilon",    "bvarepsilon",  "&#;",   "\\boldsymbol{\\varepsilon}"},
    { MS_B_LTHETAS,       TGREEK,   TG::Character,    "char bvartheta",      "bvartheta",    "&#;",   "\\boldsymbol{\\vartheta}"},
    { MS_B_LKAPPAS,       TGREEK,   TG::Character,    "char bvarkappa",      "bvarkappa",    "&#;",   "\\boldsymbol{\\varkappa}"},
    { MS_B_LPHIS,         TGREEK,   TG::Character,    "char bvarphi",        "bvarphi",      "&#;",   "\\boldsymbol{\\varphi}"},
    { MS_B_LRHOS,         TGREEK,   TG::Character,    "char bvarrho",        "bvarrho",      "&#;",   "\\boldsymbol{\\varrho}"},
    { MS_B_LPIS,          TGREEK,   TG::Character,    "char bvarpi",         "bvarpi",       "&#;",   "\\boldsymbol{\\varpi}"}
};

// Greek ital
const MathType2 aGreekItalMathTypeTable[] =
{
    // Capital
    { MS_I_UALPHA,        TGREEK,   TG::Character,    "char iAlpha",      "iAlpha",       "&#",    "\\emph{\\Alpha}"},
    { MS_I_UBETA,         TGREEK,   TG::Character,    "char iBeta",       "iBeta",        "&#",    "\\emph{\\Beta}"},
    { MS_I_UGAMMA,        TGREEK,   TG::Character,    "char iGamma",      "iGamma",       "&#",    "\\emph{\\Gamma}"},
    { MS_I_UDELTA,        TGREEK,   TG::Character,    "char iDelta",      "iDelta",       "&#",    "\\emph{\\Delta}"},
    { MS_I_UEPSILON,      TGREEK,   TG::Character,    "char iEpsilon",    "iEpsilon",     "&#",    "\\emph{\\Epsilon}"},
    { MS_I_UZETA,         TGREEK,   TG::Character,    "char iZeta",       "iZeta",        "&#",    "\\emph{\\Zeta}"},
    { MS_I_UETA,          TGREEK,   TG::Character,    "char iEta",        "iEta",         "&#",    "\\emph{\\Eta}"},
    { MS_I_UTHETA,        TGREEK,   TG::Character,    "char iTheta",      "iTheta",       "&#",    "\\emph{\\Theta}"},
    { MS_I_UIOTA,         TGREEK,   TG::Character,    "char iIota",       "iIota",        "&#",    "\\emph{\\Iota}"},
    { MS_I_UKAPPA,        TGREEK,   TG::Character,    "char iKappa",      "iKappa",       "&#",    "\\emph{\\Kappa}"},
    { MS_I_ULAMBDA,       TGREEK,   TG::Character,    "char iLambda",     "iLambda",      "&#",    "\\emph{\\Lambda}"},
    { MS_I_UMU,           TGREEK,   TG::Character,    "char iMu",         "iMu",          "&#",    "\\emph{\\Mu}"},
    { MS_I_UMU,           TGREEK,   TG::Character,    "char iMy",         "iMy",          "&#",    "\\emph{\\Mu}"},
    { MS_I_UNU,           TGREEK,   TG::Character,    "char iNu",         "iNu",          "&#",    "\\emph{\\Nu}"},
    { MS_I_UXI,           TGREEK,   TG::Character,    "char iXi",         "iXi",          "&#",    "\\emph{\\Xi}"},
    { MS_I_UOMICRON,      TGREEK,   TG::Character,    "char iOmicron",    "iOmicron",     "&#",    "\\emph{\\Omicron}"},
    { MS_I_UPI,           TGREEK,   TG::Character,    "char iPi",         "iPi",          "&#",    "\\emph{\\Pi}"},
    { MS_I_URHO,          TGREEK,   TG::Character,    "char iRho",        "iRho",         "&#",    "\\emph{\\Rho}"},
    { MS_I_USIGMA,        TGREEK,   TG::Character,    "char iSigma",      "iSigma",       "&#",    "\\emph{\\Sigma}"},
    { MS_I_UTAU,          TGREEK,   TG::Character,    "char iTau",        "iTau",         "&#",    "\\emph{\\Tau}"},
    { MS_I_UUPSILON,      TGREEK,   TG::Character,    "char iUpsion",     "iUpsilon",     "&#",    "\\emph{\\Upsilon}"},
    { MS_I_UUPSILON,      TGREEK,   TG::Character,    "char iYpsilon",    "iYpsilon",     "&#",    "\\emph{\\Upsilon}"},
    { MS_I_UPHI,          TGREEK,   TG::Character,    "char iPhi",        "iPhi",         "&#",    "\\emph{\\Phi}"},
    { MS_I_UPSI,          TGREEK,   TG::Character,    "char iPsi",        "iPsi",         "&#",    "\\emph{\\Psi}"},
    { MS_I_UCHI,          TGREEK,   TG::Character,    "char iChi",        "iChi",         "&#",    "\\emph{\\Chi}"},
    { MS_I_UOMEGA,        TGREEK,   TG::Character,    "char iOmega",      "iOmega",       "&#",    "\\emph{\\Omega}"},
    // Lower case
    { MS_I_LALPHA,        TGREEK,   TG::Character,    "char ialpha",         "ialpha",       "&#",    "\\emph{\\alpha}"},
    { MS_I_LBETA,         TGREEK,   TG::Character,    "char ibeta",          "ibeta",        "&#",    "\\emph{\\beta}"},
    { MS_I_LGAMMA,        TGREEK,   TG::Character,    "char igamma",         "igamma",       "&#",    "\\emph{\\gamma}"},
    { MS_I_LDELTA,        TGREEK,   TG::Character,    "char idelta",         "idelta",       "&#",    "\\emph{\\delta}"},
    { MS_I_LEPSILON,      TGREEK,   TG::Character,    "char iepsilon"  ,     "iepsilon",     "&#",    "\\emph{\\epsilon}"},
    { MS_I_LZETA,         TGREEK,   TG::Character,    "char izeta",          "izeta",        "&#",    "\\emph{\\zeta}"},
    { MS_I_LETA,          TGREEK,   TG::Character,    "char ieta",           "ieta",         "&#",    "\\emph{\\eta}"},
    { MS_I_LTHETA,        TGREEK,   TG::Character,    "char itheta",         "itheta",       "&#",    "\\emph{\\theta}"},
    { MS_I_LIOTA,         TGREEK,   TG::Character,    "char iiota",          "iiota",        "&#",    "\\emph{\\iota}"},
    { MS_I_LKAPPA,        TGREEK,   TG::Character,    "char ikappa",         "ikappa",       "&#",    "\\emph{\\kappa}"},
    { MS_I_LLAMBDA,       TGREEK,   TG::Character,    "char ilambda",        "ilambda",      "&#",    "\\emph{\\lambda}"},
    { MS_I_LMU,           TGREEK,   TG::Character,    "char imu",            "imu",          "&#",    "\\emph{\\mu}"},
    { MS_I_LMU,           TGREEK,   TG::Character,    "char imy",            "imy",          "&#",    "\\emph{\\mu}"},
    { MS_I_LNU,           TGREEK,   TG::Character,    "char inu",            "inu",          "&#",    "\\emph{\\nu}"},
    { MS_I_LXI,           TGREEK,   TG::Character,    "char ixi",            "ixi",          "&#",    "\\emph{\\xi}"},
    { MS_I_LOMICRON,      TGREEK,   TG::Character,    "char iomicron",       "iomicron",     "&#",    "\\emph{\\omicron}"},
    { MS_I_LPI,           TGREEK,   TG::Character,    "char ipi",            "ipi",          "&#",    "\\emph{\\pi}"},
    { MS_I_LRHO,          TGREEK,   TG::Character,    "char irho",           "irho",         "&#",    "\\emph{\\rho}"},
    { MS_I_LSIGMA,        TGREEK,   TG::Character,    "char isigma",         "isigma",       "&#",    "\\emph{\\sigma}"},
    { MS_I_LTAU,          TGREEK,   TG::Character,    "char itau",           "itau",         "&#",    "\\emph{\\tau}"},
    { MS_I_LUPSILON,      TGREEK,   TG::Character,    "char iupsilon",       "iupsilon",     "&#",    "\\emph{\\upsilon}"},
    { MS_I_LUPSILON,      TGREEK,   TG::Character,    "char iypsilon",       "iypsilon",     "&#",    "\\emph{\\upsilon}"},
    { MS_I_LPHI,          TGREEK,   TG::Character,    "char iphi",           "iphi",         "&#",    "\\emph{\\phi}"},
    { MS_I_LPSI,          TGREEK,   TG::Character,    "char ipsi",           "ipsi",         "&#",    "\\emph{\\psi}"},
    { MS_I_LCHI,          TGREEK,   TG::Character,    "char ichi",           "ichi",         "&#",    "\\emph{\\chi}"},
    { MS_I_LOMEGA,        TGREEK,   TG::Character,    "char iomega",         "iomega",       "&#",    "\\emph{\\omega}"},
    //varletter
    { MS_I_UTHETAS,       TGREEK,   TG::Character,    "char iVartheta",      "iVartheta",    "&#;",   "\\emph{\\Vartheta}"},
    { MS_I_EPSILONLUNATE, TGREEK,   TG::Character,    "char ivarepsilon",    "ivarepsilon",  "&#;",   "\\emph{\\varepsilon}"},
    { MS_I_LTHETAS,       TGREEK,   TG::Character,    "char ivartheta",      "ivartheta",    "&#;",   "\\emph{\\vartheta}"},
    { MS_I_LKAPPAS,       TGREEK,   TG::Character,    "char ivarkappa",      "ivarkappa",    "&#;",   "\\emph{\\varkappa}"},
    { MS_I_LPHIS,         TGREEK,   TG::Character,    "char ivarphi",        "ivarphi",      "&#;",   "\\emph{\\varphi}"},
    { MS_I_LRHOS,         TGREEK,   TG::Character,    "char ivarrho",        "ivarrho",      "&#;",   "\\emph{\\varrho}"},
    { MS_I_LPIS,          TGREEK,   TG::Character,    "char ivarpi",         "ivarpi",       "&#;",   "\\emph{\\varpi}"}
};

// Greek boldital
const MathType2 aGreekBoldItalMathTypeTable[] =
{
    // Capboldital
    { MS_BI_UALPHA,       TGREEK,   TG::Character,   "char biAlpha",      "biAlpha",       "&#",    "\\emph{\\boldsymbol{\\Alpha}}"},
    { MS_BI_UBETA,        TGREEK,   TG::Character,   "char biBeta",       "biBeta",        "&#",    "\\emph{\\boldsymbol{\\Beta}}"},
    { MS_BI_UGAMMA,       TGREEK,   TG::Character,   "char biGamma",      "biGamma",       "&#",    "\\emph{\\boldsymbol{\\Gamma}}"},
    { MS_BI_UDELTA,       TGREEK,   TG::Character,   "char biDelta",      "biDelta",       "&#",    "\\emph{\\boldsymbol{\\Delta}}"},
    { MS_BI_UEPSILON,     TGREEK,   TG::Character,   "char biEpsilon",    "biEpsilon",     "&#",    "\\emph{\\boldsymbol{\\Epsilon}}"},
    { MS_BI_UZETA,        TGREEK,   TG::Character,   "char biZeta",       "biZeta",        "&#",    "\\emph{\\boldsymbol{\\Zeta}}"},
    { MS_BI_UETA,         TGREEK,   TG::Character,   "char biEta",        "biEta",         "&#",    "\\emph{\\boldsymbol{\\Eta}}"},
    { MS_BI_UTHETA,       TGREEK,   TG::Character,   "char biTheta",      "biTheta",       "&#",    "\\emph{\\boldsymbol{\\Theta}}"},
    { MS_BI_UIOTA,        TGREEK,   TG::Character,   "char biIota",       "biIota",        "&#",    "\\emph{\\boldsymbol{\\Iota}}"},
    { MS_BI_UKAPPA,       TGREEK,   TG::Character,   "char biKappa",      "biKappa",       "&#",    "\\emph{\\boldsymbol{\\Kappa}}"},
    { MS_BI_ULAMBDA,      TGREEK,   TG::Character,   "char biLambda",     "biLambda",      "&#",    "\\emph{\\boldsymbol{\\Lambda}}"},
    { MS_BI_UMU,          TGREEK,   TG::Character,   "char biMu",         "biMu",          "&#",    "\\emph{\\boldsymbol{\\Mu}}"},
    { MS_BI_UMU,          TGREEK,   TG::Character,   "char biMy",         "biMy",          "&#",    "\\emph{\\boldsymbol{\\Mu}}"},
    { MS_BI_UNU,          TGREEK,   TG::Character,   "char biNu",         "biNu",          "&#",    "\\emph{\\boldsymbol{\\Nu}}"},
    { MS_BI_UXI,          TGREEK,   TG::Character,   "char biXi",         "biXi",          "&#",    "\\emph{\\boldsymbol{\\Xi}}"},
    { MS_BI_UOMICRON,     TGREEK,   TG::Character,   "char biOmicron",    "biOmicron",     "&#",    "\\emph{\\boldsymbol{\\Omicron}}"},
    { MS_BI_UPI,          TGREEK,   TG::Character,   "char biPi",         "biPi",          "&#",    "\\emph{\\boldsymbol{\\Pi}}"},
    { MS_BI_URHO,         TGREEK,   TG::Character,   "char biRho",        "biRho",         "&#",    "\\emph{\\boldsymbol{\\Rho}}"},
    { MS_BI_USIGMA,       TGREEK,   TG::Character,   "char biSigma",      "biSigma",       "&#",    "\\emph{\\boldsymbol{\\Sigma}}"},
    { MS_BI_UTAU,         TGREEK,   TG::Character,   "char biTau",        "biTau",         "&#",    "\\emph{\\boldsymbol{\\Tau}}"},
    { MS_BI_UUPSILON,     TGREEK,   TG::Character,   "char biUpsion",     "biUpsilon",     "&#",    "\\emph{\\boldsymbol{\\Upsilon}}"},
    { MS_BI_UUPSILON,     TGREEK,   TG::Character,   "char biYpsilon",    "biYpsilon",     "&#",    "\\emph{\\boldsymbol{\\Upsilon}}"},
    { MS_BI_UPHI,         TGREEK,   TG::Character,   "char biPhi",        "biPhi",         "&#",    "\\emph{\\boldsymbol{\\Phi}}"},
    { MS_BI_UPSI,         TGREEK,   TG::Character,   "char biPsi",        "biPsi",         "&#",    "\\emph{\\boldsymbol{\\Psi}}"},
    { MS_BI_UCHI,         TGREEK,   TG::Character,   "char biChi",        "biChi",         "&#",    "\\emph{\\boldsymbol{\\Chi}}"},
    { MS_BI_UOMEGA,       TGREEK,   TG::Character,   "char biOmega",      "biOmega",       "&#",    "\\emph{\\boldsymbol{\\Omega}}"},
    // Lower case
    { MS_BI_LALPHA,       TGREEK,   TG::Character,   "char bialpha",         "bialpha",       "&#",   "\\emph{\\boldsymbol{\\alpha}}"},
    { MS_BI_LBETA,        TGREEK,   TG::Character,   "char bibeta",          "bibeta",        "&#",   "\\emph{\\boldsymbol{\\beta}}"},
    { MS_BI_LGAMMA,       TGREEK,   TG::Character,   "char bigamma",         "bigamma",       "&#",   "\\emph{\\boldsymbol{\\gamma}}"},
    { MS_BI_LDELTA,       TGREEK,   TG::Character,   "char bidelta",         "bidelta",       "&#",   "\\emph{\\boldsymbol{\\delta}}"},
    { MS_BI_LEPSILON,     TGREEK,   TG::Character,   "char biepsilon"  ,     "biepsilon",     "&#",   "\\emph{\\boldsymbol{\\epsilon}}"},
    { MS_BI_LZETA,        TGREEK,   TG::Character,   "char bizeta",          "bizeta",        "&#",   "\\emph{\\boldsymbol{\\zeta}}"},
    { MS_BI_LETA,         TGREEK,   TG::Character,   "char bieta",           "bieta",         "&#",   "\\emph{\\boldsymbol{\\eta}}"},
    { MS_BI_LTHETA,       TGREEK,   TG::Character,   "char bitheta",         "bitheta",       "&#",   "\\emph{\\boldsymbol{\\theta}}"},
    { MS_BI_LIOTA,        TGREEK,   TG::Character,   "char biiota",          "biiota",        "&#",   "\\emph{\\boldsymbol{\\iota}}"},
    { MS_BI_LKAPPA,       TGREEK,   TG::Character,   "char bikappa",         "bikappa",       "&#",   "\\emph{\\boldsymbol{\\kappa}}"},
    { MS_BI_LLAMBDA,      TGREEK,   TG::Character,   "char bilambda",        "bilambda",      "&#",   "\\emph{\\boldsymbol{\\lambda}}"},
    { MS_BI_LMU,          TGREEK,   TG::Character,   "char bimu",            "bimu",          "&#",   "\\emph{\\boldsymbol{\\mu}}"},
    { MS_BI_LMU,          TGREEK,   TG::Character,   "char bimy",            "bimy",          "&#",   "\\emph{\\boldsymbol{\\mu}}"},
    { MS_BI_LNU,          TGREEK,   TG::Character,   "char binu",            "binu",          "&#",   "\\emph{\\boldsymbol{\\nu}}"},
    { MS_BI_LXI,          TGREEK,   TG::Character,   "char bixi",            "bixi",          "&#",   "\\emph{\\boldsymbol{\\xi}}"},
    { MS_BI_LOMICRON,     TGREEK,   TG::Character,   "char biomicron",       "biomicron",     "&#",   "\\emph{\\boldsymbol{\\omicron}}"},
    { MS_BI_LPI,          TGREEK,   TG::Character,   "char bipi",            "bipi",          "&#",   "\\emph{\\boldsymbol{\\pi}}"},
    { MS_BI_LRHO,         TGREEK,   TG::Character,   "char birho",           "birho",         "&#",   "\\emph{\\boldsymbol{\\rho}}"},
    { MS_BI_LSIGMA,       TGREEK,   TG::Character,   "char bisigma",         "bisigma",       "&#",   "\\emph{\\boldsymbol{\\sigma}}"},
    { MS_BI_LTAU,         TGREEK,   TG::Character,   "char bitau",           "bitau",         "&#",   "\\emph{\\boldsymbol{\\tau}}"},
    { MS_BI_LUPSILON,     TGREEK,   TG::Character,   "char biupsilon",       "biupsilon",     "&#",   "\\emph{\\boldsymbol{\\upsilon}}"},
    { MS_BI_LUPSILON,     TGREEK,   TG::Character,   "char biypsilon",       "biypsilon",     "&#",   "\\emph{\\boldsymbol{\\upsilon}}"},
    { MS_BI_LPHI,         TGREEK,   TG::Character,   "char biphi",           "biphi",         "&#",   "\\emph{\\boldsymbol{\\phi}}"},
    { MS_BI_LPSI,         TGREEK,   TG::Character,   "char bipsi",           "bipsi",         "&#",   "\\emph{\\boldsymbol{\\psi}}"},
    { MS_BI_LCHI,         TGREEK,   TG::Character,   "char bichi",           "bichi",         "&#",   "\\emph{\\boldsymbol{\\chi}}"},
    { MS_BI_LOMEGA,       TGREEK,   TG::Character,   "char biomega",         "biomega",       "&#",   "\\emph{\\boldsymbol{\\omega}"},
    //varletter
    { MS_BI_UTHETAS,       TGREEK,  TG::Character,   "char biVartheta",      "biVartheta",    "&#;",  "\\emph{\\boldsymbol{\\Vartheta}}"},
    { MS_BI_EPSILONLUNATE, TGREEK,  TG::Character,   "char bivarepsilon",    "bivarepsilon",  "&#;",  "\\emph{\\boldsymbol{\\varepsilon}}"},
    { MS_BI_LTHETAS,       TGREEK,  TG::Character,   "char bivartheta",      "bivartheta",    "&#;",  "\\emph{\\boldsymbol{\\vartheta}}"},
    { MS_BI_LKAPPAS,       TGREEK,  TG::Character,   "char bivarkappa",      "bivarkappa",    "&#;",  "\\emph{\\boldsymbol{\\varkappa}}"},
    { MS_BI_LPHIS,         TGREEK,  TG::Character,   "char bivarphi",        "bivarphi",      "&#;",  "\\emph{\\boldsymbol{\\varphi}}"},
    { MS_BI_LRHOS,         TGREEK,  TG::Character,   "char bivarrho",        "bivarrho",      "&#;",  "\\emph{\\boldsymbol{\\varrho}}"},
    { MS_BI_LPIS,          TGREEK,  TG::Character,   "char bivarpi",         "bivarpi",       "&#;",  "\\emph{\\boldsymbol{\\varpi}}"}
};

// Greek double stroke
const MathType2 aGreekDoubleStrokeMathTypeTable[] =
{
    { MS_DS_UGAMMA,    TGREEK,    TG::Character,    "char setGamma",    "setGamma",    "&#213E;",     nullptr}, //TODO latex
    { MS_DS_LGAMMA,    TGREEK,    TG::Character,    "char setgamma",    "setgamma",    "&#213D;",     nullptr}, //TODO latex
    { MS_DS_UPI,       TGREEK,    TG::Character,    "char setPi",       "setPi",       "&#213F;",     nullptr}, //TODO latex
    { MS_DS_LPI,       TGREEK,    TG::Character,    "char setpi",       "setpi",       "&#213C;",     nullptr}, //TODO latex
    { MS_DS_USIGMA,    TGREEK,    TG::Character,    "char setSigma",    "setSigma",    "&#2140;",     nullptr}  //TODO latex
};

/**
  * Numeric
  */

// Number
const MathType2 aDigitsMathTypeTable[] =
{
    { MS_ZERO,      TDIGIT,     TG::Character,     "0",     "zero",        "0",     "0"},
    { MS_ONE,       TDIGIT,     TG::Character,     "1",     "one",         "1",     "1"},
    { MS_TWO,       TDIGIT,     TG::Character,     "2",     "two",         "2",     "2"},
    { MS_THREE,     TDIGIT,     TG::Character,     "3",     "three",       "3",     "3"},
    { MS_FOUR,      TDIGIT,     TG::Character,     "4",     "four",        "4",     "4"},
    { MS_FIVE,      TDIGIT,     TG::Character,     "5",     "five",        "5",     "5"},
    { MS_SIX,       TDIGIT,     TG::Character,     "6",     "six",         "6",     "6"},
    { MS_SEVEN,     TDIGIT,     TG::Character,     "7",     "seven",       "7",     "7"},
    { MS_EIGHT,     TDIGIT,     TG::Character,     "8",     "eight",       "8",     "8"},
    { MS_NINE,      TDIGIT,     TG::Character,     "9",     "nine",        "9",     "9"},
    { MS_ZERO,      TDIGIT,     TG::Character,     "0",     "0",           "0",     "0"},
    { MS_ONE,       TDIGIT,     TG::Character,     "1",     "1",           "1",     "1"},
    { MS_TWO,       TDIGIT,     TG::Character,     "2",     "2",           "2",     "2"},
    { MS_THREE,     TDIGIT,     TG::Character,     "3",     "3",           "3",     "3"},
    { MS_FOUR,      TDIGIT,     TG::Character,     "4",     "4",           "4",     "4"},
    { MS_FIVE,      TDIGIT,     TG::Character,     "5",     "5",           "5",     "5"},
    { MS_SIX,       TDIGIT,     TG::Character,     "6",     "6",           "6",     "6"},
    { MS_SEVEN,     TDIGIT,     TG::Character,     "7",     "7",           "7",     "7"},
    { MS_EIGHT,     TDIGIT,     TG::Character,     "8",     "8",           "8",     "8"},
    { MS_NINE,      TDIGIT,     TG::Character,     "9",     "9",           "9",     "9"}
};

// Number bold
const MathType2 aDigitsBoldMathTypeTable[] =
{
    { MS_ZERO,      TDIGIT,     TG::Character,     "char bZero",      "bZero",      "&#",     "\\boldsymbol{0}"},
    { MS_ONE,       TDIGIT,     TG::Character,     "char bOne",       "bOne",       "&#",     "\\boldsymbol{1}"},
    { MS_TWO,       TDIGIT,     TG::Character,     "char bTw0",       "BTwo",       "&#",     "\\boldsymbol{2}"},
    { MS_THREE,     TDIGIT,     TG::Character,     "char bThree",     "bThree",     "&#",     "\\boldsymbol{3}"},
    { MS_FOUR,      TDIGIT,     TG::Character,     "char bFour",      "bFour",      "&#",     "\\boldsymbol{4}"},
    { MS_FIVE,      TDIGIT,     TG::Character,     "char bFive",      "bFive",      "&#",     "\\boldsymbol{5}"},
    { MS_SIX,       TDIGIT,     TG::Character,     "char bSix",       "bSix",       "&#",     "\\boldsymbol{6}"},
    { MS_SEVEN,     TDIGIT,     TG::Character,     "char bSeven",     "bSeven",     "&#",     "\\boldsymbol{7}"},
    { MS_EIGHT,     TDIGIT,     TG::Character,     "char bEight",     "bEight",     "&#",     "\\boldsymbol{8}"},
    { MS_NINE,      TDIGIT,     TG::Character,     "char bNine",      "bNine",      "&#",     "\\boldsymbol{9}"},
    { MS_ZERO,      TDIGIT,     TG::Character,     "char b0",         "b0",         "&#",     "\\boldsymbol{0}"},
    { MS_ONE,       TDIGIT,     TG::Character,     "char b1",         "b1",         "&#",     "\\boldsymbol{1}"},
    { MS_TWO,       TDIGIT,     TG::Character,     "char b2",         "b2",         "&#",     "\\boldsymbol{2}"},
    { MS_THREE,     TDIGIT,     TG::Character,     "char b3",         "b3",         "&#",     "\\boldsymbol{3}"},
    { MS_FOUR,      TDIGIT,     TG::Character,     "char b4",         "b4",         "&#",     "\\boldsymbol{4}"},
    { MS_FIVE,      TDIGIT,     TG::Character,     "char b5",         "b5",         "&#",     "\\boldsymbol{5}"},
    { MS_SIX,       TDIGIT,     TG::Character,     "char b6",         "b6",         "&#",     "\\boldsymbol{6}"},
    { MS_SEVEN,     TDIGIT,     TG::Character,     "char b7",         "b7",         "&#",     "\\boldsymbol{7}"},
    { MS_EIGHT,     TDIGIT,     TG::Character,     "char b8",         "b8",         "&#",     "\\boldsymbol{8}"},
    { MS_NINE,      TDIGIT,     TG::Character,     "char b9",         "b9",         "&#",     "\\boldsymbol{9}"}
};

// Number set
const MathType2 aDigitsDoubleStrokeMathTypeTable[] =
{
    { MS_DS_ZERO,      TSET,     TG::Character,     "char set0",     "set0",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_ONE,       TSET,     TG::Character,     "char set1",     "set1",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_TWO,       TSET,     TG::Character,     "char set2",     "set2",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_THREE,     TSET,     TG::Character,     "char set3",     "set3",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_FOUR,      TSET,     TG::Character,     "char set4",     "set4",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_FIVE,      TSET,     TG::Character,     "char set5",     "set5",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_SIX,       TSET,     TG::Character,     "char set6",     "set6",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_SEVEN,     TSET,     TG::Character,     "char set7",     "set7",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_EIGHT,     TSET,     TG::Character,     "char set8",     "set8",     "&#",     nullptr}, //TODO ml latex
    { MS_DS_NINE,      TSET,     TG::Character,     "char set9",     "set9",     "&#",     nullptr}  //TODO ml latex
};

// Counting rod
const MathType2 aDigitsCountingRodMathTypeTable[] =
{
    { MS_CHROD0,      THCROD,     TG::Character,     "char chrod0",     "chrod0",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD1,      THCROD,     TG::Character,     "char chrod1",     "chrod1",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD2,      THCROD,     TG::Character,     "char chrod2",     "chrod2",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD3,      THCROD,     TG::Character,     "char chrod3",     "chrod3",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD4,      THCROD,     TG::Character,     "char chrod4",     "chrod4",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD5,      THCROD,     TG::Character,     "char chrod5",     "chrod5",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD6,      THCROD,     TG::Character,     "char chrod6",     "chrod6",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD7,      THCROD,     TG::Character,     "char chrod7",     "chrod7",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD8,      THCROD,     TG::Character,     "char chrod8",     "chrod8",     "&#",     nullptr}, //TODO ml latex
    { MS_CHROD9,      THCROD,     TG::Character,     "char chrod9",     "chrod9",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD0,      TVCROD,     TG::Character,     "char cvrod0",     "cvrod0",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD1,      TVCROD,     TG::Character,     "char cvrod1",     "cvrod1",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD2,      TVCROD,     TG::Character,     "char cvrod2",     "cvrod2",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD3,      TVCROD,     TG::Character,     "char cvrod3",     "cvrod3",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD4,      TVCROD,     TG::Character,     "char cvrod4",     "cvrod4",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD5,      TVCROD,     TG::Character,     "char cvrod5",     "cvrod5",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD6,      TVCROD,     TG::Character,     "char cvrod6",     "cvrod6",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD7,      TVCROD,     TG::Character,     "char cvrod7",     "cvrod7",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD8,      TVCROD,     TG::Character,     "char cvrod8",     "cvrod8",     "&#",     nullptr}, //TODO ml latex
    { MS_CVROD9,      TVCROD,     TG::Character,     "char cvrod9",     "cvrod9",     "&#",     nullptr}  //TODO ml latex
};

// Roman
const MathType2 aDigitsRomanMathTypeTable[] =
{
    { MS_ROM1,        TROM,     TG::Character,     "char rom1",        "rom1",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM2,        TROM,     TG::Character,     "char rom2",        "rom2",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM3,        TROM,     TG::Character,     "char rom3",        "rom3",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM4,        TROM,     TG::Character,     "char rom4",        "rom4",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM5,        TROM,     TG::Character,     "char rom5",        "rom5",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM6,        TROM,     TG::Character,     "char rom6",        "rom6",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM7,        TROM,     TG::Character,     "char rom7",        "rom7",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM8,        TROM,     TG::Character,     "char rom8",        "rom8",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM9,        TROM,     TG::Character,     "char rom9",        "rom9",        "&#",     nullptr}, //TODO ml latex
    { MS_ROM10,       TROM,     TG::Character,     "char rom10",       "rom10",       "&#",     nullptr}, //TODO ml latex
    { MS_ROM11,       TROM,     TG::Character,     "char rom11",       "rom11",       "&#",     nullptr}, //TODO ml latex
    { MS_ROM12,       TROM,     TG::Character,     "char rom12",       "rom12",       "&#",     nullptr}, //TODO ml latex
    { MS_ROM50,       TROM,     TG::Character,     "char rom50",       "rom50",       "&#",     nullptr}, //TODO ml latex
    { MS_ROM100,      TROM,     TG::Character,     "char rom100",      "rom100",      "&#",     nullptr}, //TODO ml latex
    { MS_ROM500,      TROM,     TG::Character,     "char rom500",      "rom500",      "&#",     nullptr}, //TODO ml latex
    { MS_ROM1000,     TROM,     TG::Character,     "char rom1000",     "rom1000",     "&#",     nullptr}, //TODO ml latex
    { MS_ROMD1000,    TROM,     TG::Character,     "char romd1000",    "romd1000",    "&#",     nullptr}, //TODO ml latex
    { MS_ROMD5000,    TROM,     TG::Character,     "char romd5000",    "romd5000",    "&#",     nullptr}, //TODO ml latex
    { MS_ROMD10000,   TROM,     TG::Character,     "char romd10000",   "romd10000",   "&#",     nullptr}, //TODO ml latex
    { MS_ROMD50000,   TROM,     TG::Character,     "char romd50000",   "romd50000",   "&#",     nullptr}, //TODO ml latex
    { MS_ROMD100000,  TROM,     TG::Character,     "char romd100000",  "romd100000",  "&#",     nullptr}  //TODO ml latex
};

// Phoenician
const MathType2 aDigitsPhoenicianMathTypeTable[] =
{
    { MS_PONE,         TPHOENICIAN,    TG::Character,   "char p1",     "p1",     "&#",    nullptr}, //TODO ml latex
    { MS_PTWO,         TPHOENICIAN,    TG::Character,   "char p2",     "p2",     "&#",    nullptr}, //TODO ml latex
    { MS_PTHREE,       TPHOENICIAN,    TG::Character,   "char p3",     "p3",     "&#",    nullptr}, //TODO ml latex
    { MS_PTEN,         TPHOENICIAN,    TG::Character,   "char p10",    "p10",    "&#",    nullptr}, //TODO ml latex
    { MS_PTWENTY,      TPHOENICIAN,    TG::Character,   "char p20",    "p20",    "&#",    nullptr}, //TODO ml latex
    { MS_PHUNDRED,     TPHOENICIAN,    TG::Character,   "char p100",   "p100",   "&#",    nullptr }  //TODO ml latex
};

/**
  * Latin letters
  */

// Latin letters
const MathType2 aLatinMathTypeTable[] =
{
    //Capital
    { MS_L_UA,    TLATINCHAR,    TG::Character,    "A",    "A",    "A",    "A"},
    { MS_L_UB,    TLATINCHAR,    TG::Character,    "B",    "B",    "B",    "B"},
    { MS_L_UC,    TLATINCHAR,    TG::Character,    "C",    "C",    "C",    "C"},
    { MS_L_UD,    TLATINCHAR,    TG::Character,    "D",    "D",    "D",    "D"},
    { MS_L_UE,    TLATINCHAR,    TG::Character,    "E",    "E",    "E",    "E"},
    { MS_L_UF,    TLATINCHAR,    TG::Character,    "F",    "F",    "F",    "F"},
    { MS_L_UG,    TLATINCHAR,    TG::Character,    "G",    "G",    "G",    "G"},
    { MS_L_UH,    TLATINCHAR,    TG::Character,    "H",    "H",    "H",    "H"},
    { MS_L_UI,    TLATINCHAR,    TG::Character,    "I",    "I",    "I",    "I"},
    { MS_L_UJ,    TLATINCHAR,    TG::Character,    "J",    "J",    "J",    "J"},
    { MS_L_UK,    TLATINCHAR,    TG::Character,    "K",    "K",    "K",    "K"},
    { MS_L_UL,    TLATINCHAR,    TG::Character,    "L",    "L",    "L",    "L"},
    { MS_L_UM,    TLATINCHAR,    TG::Character,    "M",    "M",    "M",    "M"},
    { MS_L_UN,    TLATINCHAR,    TG::Character,    "N",    "N",    "N",    "N"},
    { MS_L_UO,    TLATINCHAR,    TG::Character,    "O",    "O",    "O",    "O"},
    { MS_L_UP,    TLATINCHAR,    TG::Character,    "P",    "P",    "P",    "P"},
    { MS_L_UQ,    TLATINCHAR,    TG::Character,    "Q",    "Q",    "Q",    "Q"},
    { MS_L_UR,    TLATINCHAR,    TG::Character,    "R",    "R",    "R",    "R"},
    { MS_L_US,    TLATINCHAR,    TG::Character,    "S",    "S",    "S",    "S"},
    { MS_L_UT,    TLATINCHAR,    TG::Character,    "T",    "T",    "T",    "T"},
    { MS_L_UU,    TLATINCHAR,    TG::Character,    "U",    "U",    "U",    "U"},
    { MS_L_UV,    TLATINCHAR,    TG::Character,    "V",    "V",    "V",    "V"},
    { MS_L_UW,    TLATINCHAR,    TG::Character,    "W",    "W",    "W",    "W"},
    { MS_L_UX,    TLATINCHAR,    TG::Character,    "X",    "X",    "X",    "X"},
    { MS_L_UY,    TLATINCHAR,    TG::Character,    "Y",    "Y",    "Y",    "Y"},
    { MS_L_UZ,    TLATINCHAR,    TG::Character,    "Z",    "Z",    "Z",    "Z"},
    // Lower
    { MS_L_LA,    TLATINCHAR,    TG::Character,    "a",    "a",    "a",    "a"},
    { MS_L_LB,    TLATINCHAR,    TG::Character,    "b",    "b",    "b",    "b"},
    { MS_L_LC,    TLATINCHAR,    TG::Character,    "c",    "c",    "c",    "c"},
    { MS_L_LD,    TLATINCHAR,    TG::Character,    "d",    "d",    "d",    "d"},
    { MS_L_LE,    TLATINCHAR,    TG::Character,    "e",    "e",    "e",    "e"},
    { MS_L_LF,    TLATINCHAR,    TG::Character,    "f",    "f",    "f",    "f"},
    { MS_L_LG,    TLATINCHAR,    TG::Character,    "g",    "g",    "g",    "g"},
    { MS_L_LH,    TLATINCHAR,    TG::Character,    "h",    "h",    "h",    "h"},
    { MS_L_LI,    TLATINCHAR,    TG::Character,    "i",    "i",    "i",    "i"},
    { MS_L_LJ,    TLATINCHAR,    TG::Character,    "j",    "j",    "j",    "j"},
    { MS_L_LK,    TLATINCHAR,    TG::Character,    "k",    "k",    "k",    "k"},
    { MS_L_LL,    TLATINCHAR,    TG::Character,    "l",    "l",    "l",    "l"},
    { MS_L_LM,    TLATINCHAR,    TG::Character,    "m",    "m",    "m",    "m"},
    { MS_L_LN,    TLATINCHAR,    TG::Character,    "n",    "n",    "n",    "n"},
    { MS_L_LO,    TLATINCHAR,    TG::Character,    "o",    "o",    "o",    "o"},
    { MS_L_LP,    TLATINCHAR,    TG::Character,    "p",    "p",    "p",    "p"},
    { MS_L_LQ,    TLATINCHAR,    TG::Character,    "q",    "q",    "q",    "q"},
    { MS_L_LR,    TLATINCHAR,    TG::Character,    "r",    "r",    "r",    "r"},
    { MS_L_LS,    TLATINCHAR,    TG::Character,    "s",    "s",    "s",    "s"},
    { MS_L_LT,    TLATINCHAR,    TG::Character,    "t",    "t",    "t",    "t"},
    { MS_L_LU,    TLATINCHAR,    TG::Character,    "y",    "u",    "u",    "u"},
    { MS_L_LV,    TLATINCHAR,    TG::Character,    "v",    "v",    "v",    "v"},
    { MS_L_LW,    TLATINCHAR,    TG::Character,    "w",    "w",    "w",    "w"},
    { MS_L_LX,    TLATINCHAR,    TG::Character,    "x",    "x",    "x",    "x"},
    { MS_L_LY,    TLATINCHAR,    TG::Character,    "y",    "y",    "y",    "y"},
    { MS_L_LZ,    TLATINCHAR,    TG::Character,    "z",    "z",    "z",    "z"}
};

// Latin letters Bold
const MathType2 aLatinBoldMathTypeTable[] =
{
    //Capital
    { MS_B_UA,    TLATINCHAR,    TG::Character,    "char bA",    "bA",    "&#",    "\\textbf{A}"},
    { MS_B_UB,    TLATINCHAR,    TG::Character,    "char bB",    "bB",    "&#",    "\\textbf{B}"},
    { MS_B_UC,    TLATINCHAR,    TG::Character,    "char bC",    "bC",    "&#",    "\\textbf{C}"},
    { MS_B_UD,    TLATINCHAR,    TG::Character,    "char bD",    "bD",    "&#",    "\\textbf{D}"},
    { MS_B_UE,    TLATINCHAR,    TG::Character,    "char bE",    "bE",    "&#",    "\\textbf{E}"},
    { MS_B_UF,    TLATINCHAR,    TG::Character,    "char bF",    "bF",    "&#",    "\\textbf{F}"},
    { MS_B_UG,    TLATINCHAR,    TG::Character,    "char bG",    "bG",    "&#",    "\\textbf{G}"},
    { MS_B_UH,    TLATINCHAR,    TG::Character,    "char bH",    "bH",    "&#",    "\\textbf{H}"},
    { MS_B_UI,    TLATINCHAR,    TG::Character,    "char bI",    "bI",    "&#",    "\\textbf{I}"},
    { MS_B_UJ,    TLATINCHAR,    TG::Character,    "char bJ",    "bJ",    "&#",    "\\textbf{J}"},
    { MS_B_UK,    TLATINCHAR,    TG::Character,    "char bK",    "bK",    "&#",    "\\textbf{K}"},
    { MS_B_UL,    TLATINCHAR,    TG::Character,    "char bL",    "bL",    "&#",    "\\textbf{L}"},
    { MS_B_UM,    TLATINCHAR,    TG::Character,    "char bM",    "bM",    "&#",    "\\textbf{M}"},
    { MS_B_UN,    TLATINCHAR,    TG::Character,    "char bN",    "bN",    "&#",    "\\textbf{N}"},
    { MS_B_UO,    TLATINCHAR,    TG::Character,    "char bO",    "bO",    "&#",    "\\textbf{O}"},
    { MS_B_UP,    TLATINCHAR,    TG::Character,    "char bP",    "bP",    "&#",    "\\textbf{P}"},
    { MS_B_UQ,    TLATINCHAR,    TG::Character,    "char bQ",    "bQ",    "&#",    "\\textbf{Q}"},
    { MS_B_UR,    TLATINCHAR,    TG::Character,    "char bR",    "bR",    "&#",    "\\textbf{R}"},
    { MS_B_US,    TLATINCHAR,    TG::Character,    "char bS",    "bS",    "&#",    "\\textbf{S}"},
    { MS_B_UT,    TLATINCHAR,    TG::Character,    "char bT",    "bT",    "&#",    "\\textbf{T}"},
    { MS_B_UU,    TLATINCHAR,    TG::Character,    "char bU",    "bU",    "&#",    "\\textbf{U}"},
    { MS_B_UV,    TLATINCHAR,    TG::Character,    "char bV",    "bV",    "&#",    "\\textbf{V}"},
    { MS_B_UW,    TLATINCHAR,    TG::Character,    "char bW",    "bW",    "&#",    "\\textbf{W}"},
    { MS_B_UX,    TLATINCHAR,    TG::Character,    "char bX",    "bX",    "&#",    "\\textbf{X}"},
    { MS_B_UY,    TLATINCHAR,    TG::Character,    "char bY",    "bY",    "&#",    "\\textbf{Y}"},
    { MS_B_UZ,    TLATINCHAR,    TG::Character,    "char bZ",    "bZ",    "&#",    "\\textbf{Z}"},
    // Lower
    { MS_B_LA,    TLATINCHAR,    TG::Character,    "char ba",    "ba",    "&#",    "\\textbf{a}"},
    { MS_B_LB,    TLATINCHAR,    TG::Character,    "char bb",    "bb",    "&#",    "\\textbf{b}"},
    { MS_B_LC,    TLATINCHAR,    TG::Character,    "char bc",    "bc",    "&#",    "\\textbf{c}"},
    { MS_B_LD,    TLATINCHAR,    TG::Character,    "char bd",    "bd",    "&#",    "\\textbf{d}"},
    { MS_B_LE,    TLATINCHAR,    TG::Character,    "char be",    "be",    "&#",    "\\textbf{e}"},
    { MS_B_LF,    TLATINCHAR,    TG::Character,    "char bf",    "bf",    "&#",    "\\textbf{f}"},
    { MS_B_LG,    TLATINCHAR,    TG::Character,    "char bg",    "bg",    "&#",    "\\textbf{g}"},
    { MS_B_LH,    TLATINCHAR,    TG::Character,    "char bh",    "bh",    "&#",    "\\textbf{h}"},
    { MS_B_LI,    TLATINCHAR,    TG::Character,    "char bi",    "bi",    "&#",    "\\textbf{i}"},
    { MS_B_LJ,    TLATINCHAR,    TG::Character,    "char bj",    "bj",    "&#",    "\\textbf{j}"},
    { MS_B_LK,    TLATINCHAR,    TG::Character,    "char bk",    "bk",    "&#",    "\\textbf{k}"},
    { MS_B_LL,    TLATINCHAR,    TG::Character,    "char bl",    "bl",    "&#",    "\\textbf{l}"},
    { MS_B_LM,    TLATINCHAR,    TG::Character,    "char bm",    "bm",    "&#",    "\\textbf{m}"},
    { MS_B_LN,    TLATINCHAR,    TG::Character,    "char bn",    "bn",    "&#",    "\\textbf{n}"},
    { MS_B_LO,    TLATINCHAR,    TG::Character,    "char bo",    "bo",    "&#",    "\\textbf{o}"},
    { MS_B_LP,    TLATINCHAR,    TG::Character,    "char bp",    "bp",    "&#",    "\\textbf{o}"},
    { MS_B_LQ,    TLATINCHAR,    TG::Character,    "char bq",    "bq",    "&#",    "\\textbf{q}"},
    { MS_B_LR,    TLATINCHAR,    TG::Character,    "char br",    "br",    "&#",    "\\textbf{r}"},
    { MS_B_LS,    TLATINCHAR,    TG::Character,    "char bs",    "bs",    "&#",    "\\textbf{s}"},
    { MS_B_LT,    TLATINCHAR,    TG::Character,    "char bt",    "bt",    "&#",    "\\textbf{t}"},
    { MS_B_LU,    TLATINCHAR,    TG::Character,    "char by",    "bu",    "&#",    "\\textbf{u}"},
    { MS_B_LV,    TLATINCHAR,    TG::Character,    "char bv",    "bv",    "&#",    "\\textbf{v}"},
    { MS_B_LW,    TLATINCHAR,    TG::Character,    "char bw",    "bw",    "&#",    "\\textbf{w}"},
    { MS_B_LX,    TLATINCHAR,    TG::Character,    "char bx",    "bx",    "&#",    "\\textbf{x}"},
    { MS_B_LY,    TLATINCHAR,    TG::Character,    "char by",    "by",    "&#",    "\\textbf{y}"},
    { MS_B_LZ,    TLATINCHAR,    TG::Character,    "char bz",    "bz",    "&#",    "\\textbf{z}"}
};

// Latin letters Italic
const MathType2 aLatinItalicMathTypeTable[] =
{
    //Capital
    { MS_I_UA,    TLATINCHAR,    TG::Character,    "char iA",    "iA",    "&#",    "\\emph{A}"},
    { MS_I_UB,    TLATINCHAR,    TG::Character,    "char iB",    "iB",    "&#",    "\\emph{B}"},
    { MS_I_UC,    TLATINCHAR,    TG::Character,    "char iC",    "iC",    "&#",    "\\emph{C}"},
    { MS_I_UD,    TLATINCHAR,    TG::Character,    "char iD",    "iD",    "&#",    "\\emph{D}"},
    { MS_I_UE,    TLATINCHAR,    TG::Character,    "char iE",    "iE",    "&#",    "\\emph{E}"},
    { MS_I_UF,    TLATINCHAR,    TG::Character,    "char iF",    "iF",    "&#",    "\\emph{F}"},
    { MS_I_UG,    TLATINCHAR,    TG::Character,    "char iG",    "iG",    "&#",    "\\emph{G}"},
    { MS_I_UH,    TLATINCHAR,    TG::Character,    "char iH",    "iH",    "&#",    "\\emph{H}"},
    { MS_I_UI,    TLATINCHAR,    TG::Character,    "char iI",    "iI",    "&#",    "\\emph{I}"},
    { MS_I_UJ,    TLATINCHAR,    TG::Character,    "char iJ",    "iJ",    "&#",    "\\emph{J}"},
    { MS_I_UK,    TLATINCHAR,    TG::Character,    "char iK",    "iK",    "&#",    "\\emph{K}"},
    { MS_I_UL,    TLATINCHAR,    TG::Character,    "char iL",    "iL",    "&#",    "\\emph{L}"},
    { MS_I_UM,    TLATINCHAR,    TG::Character,    "char iM",    "iM",    "&#",    "\\emph{M}"},
    { MS_I_UN,    TLATINCHAR,    TG::Character,    "char iN",    "iN",    "&#",    "\\emph{N}"},
    { MS_I_UO,    TLATINCHAR,    TG::Character,    "char iO",    "iO",    "&#",    "\\emph{O}"},
    { MS_I_UP,    TLATINCHAR,    TG::Character,    "char iP",    "iP",    "&#",    "\\emph{P}"},
    { MS_I_UQ,    TLATINCHAR,    TG::Character,    "char iQ",    "iQ",    "&#",    "\\emph{Q}"},
    { MS_I_UR,    TLATINCHAR,    TG::Character,    "char iR",    "iR",    "&#",    "\\emph{R}"},
    { MS_I_US,    TLATINCHAR,    TG::Character,    "char iS",    "iS",    "&#",    "\\emph{S}"},
    { MS_I_UT,    TLATINCHAR,    TG::Character,    "char iT",    "iT",    "&#",    "\\emph{T}"},
    { MS_I_UU,    TLATINCHAR,    TG::Character,    "char iU",    "iU",    "&#",    "\\emph{U}"},
    { MS_I_UV,    TLATINCHAR,    TG::Character,    "char iV",    "iV",    "&#",    "\\emph{V}"},
    { MS_I_UW,    TLATINCHAR,    TG::Character,    "char iW",    "iW",    "&#",    "\\emph{W}"},
    { MS_I_UX,    TLATINCHAR,    TG::Character,    "char iX",    "iX",    "&#",    "\\emph{X}"},
    { MS_I_UY,    TLATINCHAR,    TG::Character,    "char iY",    "iY",    "&#",    "\\emph{Y}"},
    { MS_I_UZ,    TLATINCHAR,    TG::Character,    "char iZ",    "iZ",    "&#",    "\\emph{Z}"},
    // Lower
    { MS_I_LA,    TLATINCHAR,    TG::Character,    "char ia",    "ia",    "&#",    "\\emph{a}"},
    { MS_I_LB,    TLATINCHAR,    TG::Character,    "char ib",    "ib",    "&#",    "\\emph{b}"},
    { MS_I_LC,    TLATINCHAR,    TG::Character,    "char ic",    "ic",    "&#",    "\\emph{c}"},
    { MS_I_LD,    TLATINCHAR,    TG::Character,    "char id",    "id",    "&#",    "\\emph{d}"},
    { MS_I_LE,    TLATINCHAR,    TG::Character,    "char ie",    "ie",    "&#",    "\\emph{e}"},
    { MS_I_LF,    TLATINCHAR,    TG::Character,    "char if",    "if",    "&#",    "\\emph{f}"},
    { MS_I_LG,    TLATINCHAR,    TG::Character,    "char ig",    "ig",    "&#",    "\\emph{g}"},
    { MS_I_LH,    TLATINCHAR,    TG::Character,    "char ih",    "ih",    "&#",    "\\emph{h}"},
    { MS_I_LI,    TLATINCHAR,    TG::Character,    "char ii",    "ii",    "&#",    "\\emph{i}"},
    { MS_I_LJ,    TLATINCHAR,    TG::Character,    "char ij",    "ij",    "&#",    "\\emph{j}"},
    { MS_I_LK,    TLATINCHAR,    TG::Character,    "char ik",    "ik",    "&#",    "\\emph{k}"},
    { MS_I_LL,    TLATINCHAR,    TG::Character,    "char il",    "il",    "&#",    "\\emph{l}"},
    { MS_I_LM,    TLATINCHAR,    TG::Character,    "char im",    "im",    "&#",    "\\emph{m}"},
    { MS_I_LN,    TLATINCHAR,    TG::Character,    "char in",    "in",    "&#",    "\\emph{n}"},
    { MS_I_LO,    TLATINCHAR,    TG::Character,    "char io",    "io",    "&#",    "\\emph{o}"},
    { MS_I_LP,    TLATINCHAR,    TG::Character,    "char ip",    "ip",    "&#",    "\\emph{o}"},
    { MS_I_LQ,    TLATINCHAR,    TG::Character,    "char iq",    "iq",    "&#",    "\\emph{q}"},
    { MS_I_LR,    TLATINCHAR,    TG::Character,    "char ir",    "ir",    "&#",    "\\emph{r}"},
    { MS_I_LS,    TLATINCHAR,    TG::Character,    "char is",    "is",    "&#",    "\\emph{s}"},
    { MS_I_LT,    TLATINCHAR,    TG::Character,    "char it",    "it",    "&#",    "\\emph{t}"},
    { MS_I_LU,    TLATINCHAR,    TG::Character,    "char iy",    "iu",    "&#",    "\\emph{u}"},
    { MS_I_LV,    TLATINCHAR,    TG::Character,    "char iv",    "iv",    "&#",    "\\emph{v}"},
    { MS_I_LW,    TLATINCHAR,    TG::Character,    "char iw",    "iw",    "&#",    "\\emph{w}"},
    { MS_I_LX,    TLATINCHAR,    TG::Character,    "char ix",    "ix",    "&#",    "\\emph{x}"},
    { MS_I_LY,    TLATINCHAR,    TG::Character,    "char iy",    "iy",    "&#",    "\\emph{y}"},
    { MS_I_LZ,    TLATINCHAR,    TG::Character,    "char iz",    "iz",    "&#",    "\\emph{z}"}
};

// Latin letters bItalic
const MathType2 aLatinBoldItalicMathTypeTable[] =
{
    //Capital
    { MS_BI_UA,    TLATINCHAR,    TG::Character,    "char biA",    "biA",    "&#",    "\\textbf{\\emph{A}}"},
    { MS_BI_UB,    TLATINCHAR,    TG::Character,    "char biB",    "biB",    "&#",    "\\textbf{\\emph{B}}"},
    { MS_BI_UC,    TLATINCHAR,    TG::Character,    "char biC",    "biC",    "&#",    "\\textbf{\\emph{C}}"},
    { MS_BI_UD,    TLATINCHAR,    TG::Character,    "char biD",    "biD",    "&#",    "\\textbf{\\emph{D}}"},
    { MS_BI_UE,    TLATINCHAR,    TG::Character,    "char biE",    "biE",    "&#",    "\\textbf{\\emph{E}}"},
    { MS_BI_UF,    TLATINCHAR,    TG::Character,    "char biF",    "biF",    "&#",    "\\textbf{\\emph{F}}"},
    { MS_BI_UG,    TLATINCHAR,    TG::Character,    "char biG",    "biG",    "&#",    "\\textbf{\\emph{G}}"},
    { MS_BI_UH,    TLATINCHAR,    TG::Character,    "char biH",    "biH",    "&#",    "\\textbf{\\emph{H}}"},
    { MS_BI_UI,    TLATINCHAR,    TG::Character,    "char biI",    "biI",    "&#",    "\\textbf{\\emph{I}}"},
    { MS_BI_UJ,    TLATINCHAR,    TG::Character,    "char biJ",    "biJ",    "&#",    "\\textbf{\\emph{J}}"},
    { MS_BI_UK,    TLATINCHAR,    TG::Character,    "char biK",    "biK",    "&#",    "\\textbf{\\emph{K}}"},
    { MS_BI_UL,    TLATINCHAR,    TG::Character,    "char biL",    "biL",    "&#",    "\\textbf{\\emph{L}}"},
    { MS_BI_UM,    TLATINCHAR,    TG::Character,    "char biM",    "biM",    "&#",    "\\textbf{\\emph{M}}"},
    { MS_BI_UN,    TLATINCHAR,    TG::Character,    "char biN",    "biN",    "&#",    "\\textbf{\\emph{N}}"},
    { MS_BI_UO,    TLATINCHAR,    TG::Character,    "char biO",    "biO",    "&#",    "\\textbf{\\emph{O}}"},
    { MS_BI_UP,    TLATINCHAR,    TG::Character,    "char biP",    "biP",    "&#",    "\\textbf{\\emph{P}}"},
    { MS_BI_UQ,    TLATINCHAR,    TG::Character,    "char biQ",    "biQ",    "&#",    "\\textbf{\\emph{Q}}"},
    { MS_BI_UR,    TLATINCHAR,    TG::Character,    "char biR",    "biR",    "&#",    "\\textbf{\\emph{R}}"},
    { MS_BI_US,    TLATINCHAR,    TG::Character,    "char biS",    "biS",    "&#",    "\\textbf{\\emph{S}}"},
    { MS_BI_UT,    TLATINCHAR,    TG::Character,    "char biT",    "biT",    "&#",    "\\textbf{\\emph{T}}"},
    { MS_BI_UU,    TLATINCHAR,    TG::Character,    "char biU",    "biU",    "&#",    "\\textbf{\\emph{U}}"},
    { MS_BI_UV,    TLATINCHAR,    TG::Character,    "char biV",    "biV",    "&#",    "\\textbf{\\emph{V}}"},
    { MS_BI_UW,    TLATINCHAR,    TG::Character,    "char biW",    "biW",    "&#",    "\\textbf{\\emph{W}}"},
    { MS_BI_UX,    TLATINCHAR,    TG::Character,    "char biX",    "biX",    "&#",    "\\textbf{\\emph{X}}"},
    { MS_BI_UY,    TLATINCHAR,    TG::Character,    "char biY",    "biY",    "&#",    "\\textbf{\\emph{Y}}"},
    { MS_BI_UZ,    TLATINCHAR,    TG::Character,    "char biZ",    "biZ",    "&#",    "\\textbf{\\emph{Z}}"},
    //Lower
    { MS_BI_LA,    TLATINCHAR,    TG::Character,    "char bia",    "bia",    "&#",    "\\textbf{\\emph{a}}"},
    { MS_BI_LB,    TLATINCHAR,    TG::Character,    "char bib",    "bib",    "&#",    "\\textbf{\\emph{b}}"},
    { MS_BI_LC,    TLATINCHAR,    TG::Character,    "char bic",    "bic",    "&#",    "\\textbf{\\emph{c}}"},
    { MS_BI_LD,    TLATINCHAR,    TG::Character,    "char bid",    "bid",    "&#",    "\\textbf{\\emph{d}}"},
    { MS_BI_LE,    TLATINCHAR,    TG::Character,    "char bie",    "bie",    "&#",    "\\textbf{\\emph{e}}"},
    { MS_BI_LF,    TLATINCHAR,    TG::Character,    "char bif",    "bif",    "&#",    "\\textbf{\\emph{f}}"},
    { MS_BI_LG,    TLATINCHAR,    TG::Character,    "char big",    "big",    "&#",    "\\textbf{\\emph{g}}"},
    { MS_BI_LH,    TLATINCHAR,    TG::Character,    "char bih",    "bih",    "&#",    "\\textbf{\\emph{h}}"},
    { MS_BI_LI,    TLATINCHAR,    TG::Character,    "char bii",    "bii",    "&#",    "\\textbf{\\emph{i}}"},
    { MS_BI_LJ,    TLATINCHAR,    TG::Character,    "char bij",    "bij",    "&#",    "\\textbf{\\emph{j}}"},
    { MS_BI_LK,    TLATINCHAR,    TG::Character,    "char bik",    "bik",    "&#",    "\\textbf{\\emph{k}}"},
    { MS_BI_LL,    TLATINCHAR,    TG::Character,    "char bil",    "bil",    "&#",    "\\textbf{\\emph{l}}"},
    { MS_BI_LM,    TLATINCHAR,    TG::Character,    "char bim",    "bim",    "&#",    "\\textbf{\\emph{m}}"},
    { MS_BI_LN,    TLATINCHAR,    TG::Character,    "char bin",    "bin",    "&#",    "\\textbf{\\emph{n}}"},
    { MS_BI_LO,    TLATINCHAR,    TG::Character,    "char bio",    "bio",    "&#",    "\\textbf{\\emph{o}}"},
    { MS_BI_LP,    TLATINCHAR,    TG::Character,    "char bip",    "bip",    "&#",    "\\textbf{\\emph{o}}"},
    { MS_BI_LQ,    TLATINCHAR,    TG::Character,    "char biq",    "biq",    "&#",    "\\textbf{\\emph{q}}"},
    { MS_BI_LR,    TLATINCHAR,    TG::Character,    "char bir",    "bir",    "&#",    "\\textbf{\\emph{r}}"},
    { MS_BI_LS,    TLATINCHAR,    TG::Character,    "char bis",    "bis",    "&#",    "\\textbf{\\emph{s}}"},
    { MS_BI_LT,    TLATINCHAR,    TG::Character,    "char bit",    "bit",    "&#",    "\\textbf{\\emph{t}}"},
    { MS_BI_LU,    TLATINCHAR,    TG::Character,    "char biy",    "biu",    "&#",    "\\textbf{\\emph{u}}"},
    { MS_BI_LV,    TLATINCHAR,    TG::Character,    "char biv",    "biv",    "&#",    "\\textbf{\\emph{v}}"},
    { MS_BI_LW,    TLATINCHAR,    TG::Character,    "char biw",    "biw",    "&#",    "\\textbf{\\emph{w}}"},
    { MS_BI_LX,    TLATINCHAR,    TG::Character,    "char bix",    "bix",    "&#",    "\\textbf{\\emph{x}}"},
    { MS_BI_LY,    TLATINCHAR,    TG::Character,    "char biy",    "biy",    "&#",    "\\textbf{\\emph{y}}"},
    { MS_BI_LZ,    TLATINCHAR,    TG::Character,    "char biz",    "biz",    "&#",    "\\textbf{\\emph{z}}"}
};

// Double stroke Latin letters
const MathType2 aLatinDoubleStrokeMathTypeTable[] =
{
    //Capital
    { MS_DS_UA,     TSET,     TG::Character,     "char setA",    "setA",     "&#",     "\\mathbb{A}"},
    { MS_DS_UB,     TSET,     TG::Character,     "char setB",    "setB",     "&#",     "\\mathbb{B}"},
    { MS_DS_UC,     TSET,     TG::Character,     "setC",         "setC",     "&#",     "\\mathbb{C}"},
    { MS_DS_UD,     TSET,     TG::Character,     "char setD",    "setD",     "&#",     "\\mathbb{D}"},
    { MS_DS_UE,     TSET,     TG::Character,     "char setE",    "setE",     "&#",     "\\mathbb{E}"},
    { MS_DS_UF,     TSET,     TG::Character,     "char setF",    "setF",     "&#",     "\\mathbb{F}"},
    { MS_DS_UG,     TSET,     TG::Character,     "char setG",    "setG",     "&#",     "\\mathbb{G}"},
    { MS_DS_UH,     TSET,     TG::Character,     "char setH",    "setH",     "&#",     "\\mathbb{H}"},
    { MS_DS_UI,     TSET,     TG::Character,     "char setI",    "setI",     "&#",     "\\mathbb{I}"},
    { MS_DS_UJ,     TSET,     TG::Character,     "char setJ",    "setJ",     "&#",     "\\mathbb{J}"},
    { MS_DS_UK,     TSET,     TG::Character,     "char setK",    "setK",     "&#",     "\\mathbb{K}"},
    { MS_DS_UL,     TSET,     TG::Character,     "char setL",    "setL",     "&#",     "\\mathbb{L}"},
    { MS_DS_UM,     TSET,     TG::Character,     "char setM",    "setM",     "&#",     "\\mathbb{M}"},
    { MS_DS_UN,     TSET,     TG::Character,     "setN",         "setN",     "&#",     "\\mathbb{N}"},
    { MS_DS_UO,     TSET,     TG::Character,     "char setO",    "setO",     "&#",     "\\mathbb{O}"},
    { MS_DS_UP,     TSET,     TG::Character,     "char setP",    "setP",     "&#",     "\\mathbb{P}"},
    { MS_DS_UQ,     TSET,     TG::Character,     "setQ",         "setQ",     "&#",     "\\mathbb{Q}"},
    { MS_DS_UR,     TSET,     TG::Character,     "setR",         "setR",     "&#",     "\\mathbb{R}"},
    { MS_DS_US,     TSET,     TG::Character,     "char setS",    "setS",     "&#",     "\\mathbb{S}"},
    { MS_DS_UT,     TSET,     TG::Character,     "char setT",    "setT",     "&#",     "\\mathbb{T}"},
    { MS_DS_UU,     TSET,     TG::Character,     "char setU",    "setU",     "&#",     "\\mathbb{U}"},
    { MS_DS_UV,     TSET,     TG::Character,     "char setV",    "setV",     "&#",     "\\mathbb{V}"},
    { MS_DS_UW,     TSET,     TG::Character,     "char setW",    "setW",     "&#",     "\\mathbb{W}"},
    { MS_DS_UX,     TSET,     TG::Character,     "char setX",    "setX",     "&#",     "\\mathbb{X}"},
    { MS_DS_UY,     TSET,     TG::Character,     "char setY",    "setY",     "&#",     "\\mathbb{Y}"},
    { MS_DS_UZ,     TSET,     TG::Character,     "setZ",         "setZ",     "&#",     "\\mathbb{Z}"},
    // Lower
    { MS_DS_LA,     TSET,     TG::Character,     "char seta",    "seta",     "&#",     "\\mathbb{a}"},
    { MS_DS_LB,     TSET,     TG::Character,     "char setb",    "setb",     "&#",     "\\mathbb{b}"},
    { MS_DS_LC,     TSET,     TG::Character,     "char setc",    "setc",     "&#",     "\\mathbb{c}"},
    { MS_DS_LD,     TSET,     TG::Character,     "char setd",    "setd",     "&#",     "\\mathbb{d}"},
    { MS_DS_LE,     TSET,     TG::Character,     "char sete",    "sete",     "&#",     "\\mathbb{e}"},
    { MS_DS_LF,     TSET,     TG::Character,     "char setf",    "setf",     "&#",     "\\mathbb{f}"},
    { MS_DS_LG,     TSET,     TG::Character,     "char setg",    "setg",     "&#",     "\\mathbb{g}"},
    { MS_DS_LH,     TSET,     TG::Character,     "char seth",    "seth",     "&#",     "\\mathbb{h}"},
    { MS_DS_LI,     TSET,     TG::Character,     "char seti",    "seti",     "&#",     "\\mathbb{i}"},
    { MS_DS_LJ,     TSET,     TG::Character,     "char setj",    "setj",     "&#",     "\\mathbb{j}"},
    { MS_DS_LK,     TSET,     TG::Character,     "char setk",    "setk",     "&#",     "\\mathbb{k}"},
    { MS_DS_LL,     TSET,     TG::Character,     "char setl",    "setl",     "&#",     "\\mathbb{l}"},
    { MS_DS_LM,     TSET,     TG::Character,     "char setm",    "setm",     "&#",     "\\mathbb{m}"},
    { MS_DS_LN,     TSET,     TG::Character,     "char setn",    "setn",     "&#",     "\\mathbb{n}"},
    { MS_DS_LO,     TSET,     TG::Character,     "char seto",    "seto",     "&#",     "\\mathbb{o}"},
    { MS_DS_LP,     TSET,     TG::Character,     "char setp",    "setp",     "&#",     "\\mathbb{o}"},
    { MS_DS_LQ,     TSET,     TG::Character,     "char setq",    "setq",     "&#",     "\\mathbb{q}"},
    { MS_DS_LR,     TSET,     TG::Character,     "char setr",    "setr",     "&#",     "\\mathbb{r}"},
    { MS_DS_LS,     TSET,     TG::Character,     "char sets",    "sets",     "&#",     "\\mathbb{s}"},
    { MS_DS_LT,     TSET,     TG::Character,     "char sett",    "sett",     "&#",     "\\mathbb{t}"},
    { MS_DS_LU,     TSET,     TG::Character,     "char sety",    "setu",     "&#",     "\\mathbb{u}"},
    { MS_DS_LV,     TSET,     TG::Character,     "char setv",    "setv",     "&#",     "\\mathbb{v}"},
    { MS_DS_LW,     TSET,     TG::Character,     "char setw",    "setw",     "&#",     "\\mathbb{w}"},
    { MS_DS_LX,     TSET,     TG::Character,     "char setx",    "setx",     "&#",     "\\mathbb{x}"},
    { MS_DS_LY,     TSET,     TG::Character,     "char sety",    "sety",     "&#",     "\\mathbb{y}"},
    { MS_DS_LZ,     TSET,     TG::Character,     "char setz",    "setz",     "&#",     "\\mathbb{z}"}
};

// Fractur Latin letters
const MathType2 aLatinFrakturMathTypeTable[] =
{
    //Capital
    { MS_FR_UA,     TFRAK,     TG::Character,     "char frakA",    "frakA",     "&#",     "\\mathfrak{A}"},
    { MS_FR_UB,     TFRAK,     TG::Character,     "char frakB",    "frakB",     "&#",     "\\mathfrak{B}"},
    { MS_FR_UC,     TFRAK,     TG::Character,     "char frakC",    "frakC",     "&#",     "\\mathfrak{C}"},
    { MS_FR_UD,     TFRAK,     TG::Character,     "char frakD",    "frakD",     "&#",     "\\mathfrak{D}"},
    { MS_FR_UE,     TFRAK,     TG::Character,     "char frakE",    "frakE",     "&#",     "\\mathfrak{E}"},
    { MS_FR_UF,     TFRAK,     TG::Character,     "char frakF",    "frakF",     "&#",     "\\mathfrak{F}"},
    { MS_FR_UG,     TFRAK,     TG::Character,     "char frakG",    "frakG",     "&#",     "\\mathfrak{G}"},
    { MS_FR_UH,     TFRAK,     TG::Character,     "char frakH",    "frakH",     "&#",     "\\mathfrak{H}"},
    { MS_FR_UI,     TFRAK,     TG::Character,     "char frakI",    "frakI",     "&#",     "\\mathfrak{I}"},
    { MS_FR_UJ,     TFRAK,     TG::Character,     "char frakJ",    "frakJ",     "&#",     "\\mathfrak{J}"},
    { MS_FR_UK,     TFRAK,     TG::Character,     "char frakK",    "frakK",     "&#",     "\\mathfrak{K}"},
    { MS_FR_UL,     TFRAK,     TG::Character,     "char frakL",    "frakL",     "&#",     "\\mathfrak{L}"},
    { MS_FR_UM,     TFRAK,     TG::Character,     "char frakM",    "frakM",     "&#",     "\\mathfrak{M}"},
    { MS_FR_UN,     TFRAK,     TG::Character,     "char frakN",    "frakN",     "&#",     "\\mathfrak{N}"},
    { MS_FR_UO,     TFRAK,     TG::Character,     "char frakO",    "frakO",     "&#",     "\\mathfrak{O}"},
    { MS_FR_UP,     TFRAK,     TG::Character,     "char frakP",    "frakP",     "&#",     "\\mathfrak{P}"},
    { MS_FR_UQ,     TFRAK,     TG::Character,     "char frakQ",    "frakQ",     "&#",     "\\mathfrak{Q}"},
    { MS_FR_UR,     TFRAK,     TG::Character,     "char frakR",    "frakR",     "&#",     "\\mathfrak{R}"},
    { MS_FR_US,     TFRAK,     TG::Character,     "char frakS",    "frakS",     "&#",     "\\mathfrak{S}"},
    { MS_FR_UT,     TFRAK,     TG::Character,     "char frakT",    "frakT",     "&#",     "\\mathfrak{T}"},
    { MS_FR_UU,     TFRAK,     TG::Character,     "char frakU",    "frakU",     "&#",     "\\mathfrak{U}"},
    { MS_FR_UV,     TFRAK,     TG::Character,     "char frakV",    "frakV",     "&#",     "\\mathfrak{V}"},
    { MS_FR_UW,     TFRAK,     TG::Character,     "char frakW",    "frakW",     "&#",     "\\mathfrak{W}"},
    { MS_FR_UX,     TFRAK,     TG::Character,     "char frakX",    "frakX",     "&#",     "\\mathfrak{X}"},
    { MS_FR_UY,     TFRAK,     TG::Character,     "char frakY",    "frakY",     "&#",     "\\mathfrak{Y}"},
    { MS_FR_UZ,     TFRAK,     TG::Character,     "char frakZ",    "frakZ",     "&#",     "\\mathfrak{Z}"},
    // Lower
    { MS_FR_LA,     TFRAK,     TG::Character,     "char fraka",    "fraka",     "&#",     "\\mathfrak{a}"},
    { MS_FR_LB,     TFRAK,     TG::Character,     "char frakb",    "frakb",     "&#",     "\\mathfrak{b}"},
    { MS_FR_LC,     TFRAK,     TG::Character,     "char frakc",    "frakc",     "&#",     "\\mathfrak{c}"},
    { MS_FR_LD,     TFRAK,     TG::Character,     "char frakd",    "frakd",     "&#",     "\\mathfrak{d}"},
    { MS_FR_LE,     TFRAK,     TG::Character,     "char frake",    "frake",     "&#",     "\\mathfrak{e}"},
    { MS_FR_LF,     TFRAK,     TG::Character,     "char frakf",    "frakf",     "&#",     "\\mathfrak{f}"},
    { MS_FR_LG,     TFRAK,     TG::Character,     "char frakg",    "frakg",     "&#",     "\\mathfrak{g}"},
    { MS_FR_LH,     TFRAK,     TG::Character,     "char frakh",    "frakh",     "&#",     "\\mathfrak{h}"},
    { MS_FR_LI,     TFRAK,     TG::Character,     "char fraki",    "fraki",     "&#",     "\\mathfrak{i}"},
    { MS_FR_LJ,     TFRAK,     TG::Character,     "char frakj",    "frakj",     "&#",     "\\mathfrak{j}"},
    { MS_FR_LK,     TFRAK,     TG::Character,     "char frakk",    "frakk",     "&#",     "\\mathfrak{k}"},
    { MS_FR_LL,     TFRAK,     TG::Character,     "char frakl",    "frakl",     "&#",     "\\mathfrak{l}"},
    { MS_FR_LM,     TFRAK,     TG::Character,     "char frakm",    "frakm",     "&#",     "\\mathfrak{m}"},
    { MS_FR_LN,     TFRAK,     TG::Character,     "char frakn",    "frakn",     "&#",     "\\mathfrak{n}"},
    { MS_FR_LO,     TFRAK,     TG::Character,     "char frako",    "frako",     "&#",     "\\mathfrak{o}"},
    { MS_FR_LP,     TFRAK,     TG::Character,     "char frakp",    "frakp",     "&#",     "\\mathfrak{o}"},
    { MS_FR_LQ,     TFRAK,     TG::Character,     "char frakq",    "frakq",     "&#",     "\\mathfrak{q}"},
    { MS_FR_LR,     TFRAK,     TG::Character,     "char frakr",    "frakr",     "&#",     "\\mathfrak{r}"},
    { MS_FR_LS,     TFRAK,     TG::Character,     "char fraks",    "fraks",     "&#",     "\\mathfrak{s}"},
    { MS_FR_LT,     TFRAK,     TG::Character,     "char frakt",    "frakt",     "&#",     "\\mathfrak{t}"},
    { MS_FR_LU,     TFRAK,     TG::Character,     "char fraky",    "fraku",     "&#",     "\\mathfrak{u}"},
    { MS_FR_LV,     TFRAK,     TG::Character,     "char frakv",    "frakv",     "&#",     "\\mathfrak{v}"},
    { MS_FR_LW,     TFRAK,     TG::Character,     "char frakw",    "frakw",     "&#",     "\\mathfrak{w}"},
    { MS_FR_LX,     TFRAK,     TG::Character,     "char frakx",    "frakx",     "&#",     "\\mathfrak{x}"},
    { MS_FR_LY,     TFRAK,     TG::Character,     "char fraky",    "fraky",     "&#",     "\\mathfrak{y}"},
    { MS_FR_LZ,     TFRAK,     TG::Character,     "char frakz",    "frakz",     "&#",     "\\mathfrak{z}"}
};

// Fractur Bold Latin letters
const MathType2 aLatinFrakturBoldMathTypeTable[] =
{
    //Capital
    { MS_BFR_UA,     TFRAK,     TG::Character,     "char bfrakA",    "bfrakA",     "&#",     "\\mathfrak{\\boldsymbol{A}}"},
    { MS_BFR_UB,     TFRAK,     TG::Character,     "char bfrakB",    "bfrakB",     "&#",     "\\mathfrak{\\boldsymbol{B}}"},
    { MS_BFR_UC,     TFRAK,     TG::Character,     "char bfrakC",    "bfrakC",     "&#",     "\\mathfrak{\\boldsymbol{C}}"},
    { MS_BFR_UD,     TFRAK,     TG::Character,     "char bfrakD",    "bfrakD",     "&#",     "\\mathfrak{\\boldsymbol{D}}"},
    { MS_BFR_UE,     TFRAK,     TG::Character,     "char bfrakE",    "bfrakE",     "&#",     "\\mathfrak{\\boldsymbol{E}}"},
    { MS_BFR_UF,     TFRAK,     TG::Character,     "char bfrakF",    "bfrakF",     "&#",     "\\mathfrak{\\boldsymbol{F}}"},
    { MS_BFR_UG,     TFRAK,     TG::Character,     "char bfrakG",    "bfrakG",     "&#",     "\\mathfrak{\\boldsymbol{G}}"},
    { MS_BFR_UH,     TFRAK,     TG::Character,     "char bfrakH",    "bfrakH",     "&#",     "\\mathfrak{\\boldsymbol{H}}"},
    { MS_BFR_UI,     TFRAK,     TG::Character,     "char bfrakI",    "bfrakI",     "&#",     "\\mathfrak{\\boldsymbol{I}}"},
    { MS_BFR_UJ,     TFRAK,     TG::Character,     "char bfrakJ",    "bfrakJ",     "&#",     "\\mathfrak{\\boldsymbol{J}}"},
    { MS_BFR_UK,     TFRAK,     TG::Character,     "char bfrakK",    "bfrakK",     "&#",     "\\mathfrak{\\boldsymbol{K}}"},
    { MS_BFR_UL,     TFRAK,     TG::Character,     "char bfrakL",    "bfrakL",     "&#",     "\\mathfrak{\\boldsymbol{L}}"},
    { MS_BFR_UM,     TFRAK,     TG::Character,     "char bfrakM",    "bfrakM",     "&#",     "\\mathfrak{\\boldsymbol{M}}"},
    { MS_BFR_UN,     TFRAK,     TG::Character,     "char bfrakN",    "bfrakN",     "&#",     "\\mathfrak{\\boldsymbol{N}}"},
    { MS_BFR_UO,     TFRAK,     TG::Character,     "char bfrakO",    "bfrakO",     "&#",     "\\mathfrak{\\boldsymbol{O}}"},
    { MS_BFR_UP,     TFRAK,     TG::Character,     "char bfrakP",    "bfrakP",     "&#",     "\\mathfrak{\\boldsymbol{P}}"},
    { MS_BFR_UQ,     TFRAK,     TG::Character,     "char bfrakQ",    "bfrakQ",     "&#",     "\\mathfrak{\\boldsymbol{Q}}"},
    { MS_BFR_UR,     TFRAK,     TG::Character,     "char bfrakR",    "bfrakR",     "&#",     "\\mathfrak{\\boldsymbol{R}}"},
    { MS_BFR_US,     TFRAK,     TG::Character,     "char bfrakS",    "bfrakS",     "&#",     "\\mathfrak{\\boldsymbol{S}}"},
    { MS_BFR_UT,     TFRAK,     TG::Character,     "char bfrakT",    "bfrakT",     "&#",     "\\mathfrak{\\boldsymbol{T}}"},
    { MS_BFR_UU,     TFRAK,     TG::Character,     "char bfrakU",    "bfrakU",     "&#",     "\\mathfrak{\\boldsymbol{U}}"},
    { MS_BFR_UV,     TFRAK,     TG::Character,     "char bfrakV",    "bfrakV",     "&#",     "\\mathfrak{\\boldsymbol{V}}"},
    { MS_BFR_UW,     TFRAK,     TG::Character,     "char bfrakW",    "bfrakW",     "&#",     "\\mathfrak{\\boldsymbol{W}}"},
    { MS_BFR_UX,     TFRAK,     TG::Character,     "char bfrakX",    "bfrakX",     "&#",     "\\mathfrak{\\boldsymbol{X}}"},
    { MS_BFR_UY,     TFRAK,     TG::Character,     "char bfrakY",    "bfrakY",     "&#",     "\\mathfrak{\\boldsymbol{Y}}"},
    { MS_BFR_UZ,     TFRAK,     TG::Character,     "char bfrakZ",    "bfrakZ",     "&#",     "\\mathfrak{\\boldsymbol{Z}}"},
    // Lower
    { MS_BFR_LA,     TFRAK,     TG::Character,     "char bfraka",    "bfraka",     "&#",     "\\mathfrak{\\boldsymbol{a}}"},
    { MS_BFR_LB,     TFRAK,     TG::Character,     "char bfrakb",    "bfrakb",     "&#",     "\\mathfrak{\\boldsymbol{b}}"},
    { MS_BFR_LC,     TFRAK,     TG::Character,     "char bfrakc",    "bfrakc",     "&#",     "\\mathfrak{\\boldsymbol{c}}"},
    { MS_BFR_LD,     TFRAK,     TG::Character,     "char bfrakd",    "bfrakd",     "&#",     "\\mathfrak{\\boldsymbol{d}}"},
    { MS_BFR_LE,     TFRAK,     TG::Character,     "char bfrake",    "bfrake",     "&#",     "\\mathfrak{\\boldsymbol{e}}"},
    { MS_BFR_LF,     TFRAK,     TG::Character,     "char bfrakf",    "bfrakf",     "&#",     "\\mathfrak{\\boldsymbol{f}}"},
    { MS_BFR_LG,     TFRAK,     TG::Character,     "char bfrakg",    "bfrakg",     "&#",     "\\mathfrak{\\boldsymbol{g}}"},
    { MS_BFR_LH,     TFRAK,     TG::Character,     "char bfrakh",    "bfrakh",     "&#",     "\\mathfrak{\\boldsymbol{h}}"},
    { MS_BFR_LI,     TFRAK,     TG::Character,     "char bfraki",    "bfraki",     "&#",     "\\mathfrak{\\boldsymbol{i}}"},
    { MS_BFR_LJ,     TFRAK,     TG::Character,     "char bfrakj",    "bfrakj",     "&#",     "\\mathfrak{\\boldsymbol{j}}"},
    { MS_BFR_LK,     TFRAK,     TG::Character,     "char bfrakk",    "bfrakk",     "&#",     "\\mathfrak{\\boldsymbol{k}}"},
    { MS_BFR_LL,     TFRAK,     TG::Character,     "char bfrakl",    "bfrakl",     "&#",     "\\mathfrak{\\boldsymbol{l}}"},
    { MS_BFR_LM,     TFRAK,     TG::Character,     "char bfrakm",    "bfrakm",     "&#",     "\\mathfrak{\\boldsymbol{m}}"},
    { MS_BFR_LN,     TFRAK,     TG::Character,     "char bfrakn",    "bfrakn",     "&#",     "\\mathfrak{\\boldsymbol{n}}"},
    { MS_BFR_LO,     TFRAK,     TG::Character,     "char bfrako",    "bfrako",     "&#",     "\\mathfrak{\\boldsymbol{o}}"},
    { MS_BFR_LP,     TFRAK,     TG::Character,     "char bfrakp",    "bfrakp",     "&#",     "\\mathfrak{\\boldsymbol{o}}"},
    { MS_BFR_LQ,     TFRAK,     TG::Character,     "char bfrakq",    "bfrakq",     "&#",     "\\mathfrak{\\boldsymbol{q}}"},
    { MS_BFR_LR,     TFRAK,     TG::Character,     "char bfrakr",    "bfrakr",     "&#",     "\\mathfrak{\\boldsymbol{r}}"},
    { MS_BFR_LS,     TFRAK,     TG::Character,     "char bfraks",    "bfraks",     "&#",     "\\mathfrak{\\boldsymbol{s}}"},
    { MS_BFR_LT,     TFRAK,     TG::Character,     "char bfrakt",    "bfrakt",     "&#",     "\\mathfrak{\\boldsymbol{t}}"},
    { MS_BFR_LU,     TFRAK,     TG::Character,     "char bfraky",    "bfraku",     "&#",     "\\mathfrak{\\boldsymbol{u}}"},
    { MS_BFR_LV,     TFRAK,     TG::Character,     "char bfrakv",    "bfrakv",     "&#",     "\\mathfrak{\\boldsymbol{v}}"},
    { MS_BFR_LW,     TFRAK,     TG::Character,     "char bfrakw",    "bfrakw",     "&#",     "\\mathfrak{\\boldsymbol{w}}"},
    { MS_BFR_LX,     TFRAK,     TG::Character,     "char bfrakx",    "bfrakx",     "&#",     "\\mathfrak{\\boldsymbol{x}}"},
    { MS_BFR_LY,     TFRAK,     TG::Character,     "char bfraky",    "bfraky",     "&#",     "\\mathfrak{\\boldsymbol{y}}"},
    { MS_BFR_LZ,     TFRAK,     TG::Character,     "char bfrakz",    "bfrakz",     "&#",     "\\mathfrak{\\boldsymbol{z}}"}
};

// Script Latin letters
const MathType2 aLatinScriptMathTypeTable[] =
{
    //Capital
    { MS_SC_UA,     TSCRIPT,     TG::Character,     "char scrA",    "scrA",     "&#",     "\\mathscr{A}"},
    { MS_SC_UB,     TSCRIPT,     TG::Character,     "char scrB",    "scrB",     "&#",     "\\mathscr{B}"},
    { MS_SC_UC,     TSCRIPT,     TG::Character,     "char scrC",    "scrC",     "&#",     "\\mathscr{C}"},
    { MS_SC_UD,     TSCRIPT,     TG::Character,     "char scrD",    "scrD",     "&#",     "\\mathscr{D}"},
    { MS_SC_UE,     TSCRIPT,     TG::Character,     "char scrE",    "scrE",     "&#",     "\\mathscr{E}"},
    { MS_SC_UF,     TSCRIPT,     TG::Character,     "char scrF",    "scrF",     "&#",     "\\mathscr{F}"},
    { MS_SC_UG,     TSCRIPT,     TG::Character,     "char scrG",    "scrG",     "&#",     "\\mathscr{G}"},
    { MS_SC_UH,     TSCRIPT,     TG::Character,     "char scrH",    "scrH",     "&#",     "\\mathscr{H}"},
    { MS_SC_UI,     TSCRIPT,     TG::Character,     "char scrI",    "scrI",     "&#",     "\\mathscr{I}"},
    { MS_SC_UJ,     TSCRIPT,     TG::Character,     "char scrJ",    "scrJ",     "&#",     "\\mathscr{J}"},
    { MS_SC_UK,     TSCRIPT,     TG::Character,     "char scrK",    "scrK",     "&#",     "\\mathscr{K}"},
    { MS_SC_UL,     TSCRIPT,     TG::Character,     "char scrL",    "scrL",     "&#",     "\\mathscr{L}"},
    { MS_SC_UM,     TSCRIPT,     TG::Character,     "char scrM",    "scrM",     "&#",     "\\mathscr{M}"},
    { MS_SC_UN,     TSCRIPT,     TG::Character,     "char scrN",    "scrN",     "&#",     "\\mathscr{N}"},
    { MS_SC_UO,     TSCRIPT,     TG::Character,     "char scrO",    "scrO",     "&#",     "\\mathscr{O}"},
    { MS_SC_UP,     TSCRIPT,     TG::Character,     "char scrP",    "scrP",     "&#",     "\\mathscr{P}"},
    { MS_SC_UQ,     TSCRIPT,     TG::Character,     "char scrQ",    "scrQ",     "&#",     "\\mathscr{Q}"},
    { MS_SC_UR,     TSCRIPT,     TG::Character,     "char scrR",    "scrR",     "&#",     "\\mathscr{R}"},
    { MS_SC_US,     TSCRIPT,     TG::Character,     "char scrS",    "scrS",     "&#",     "\\mathscr{S}"},
    { MS_SC_UT,     TSCRIPT,     TG::Character,     "char scrT",    "scrT",     "&#",     "\\mathscr{T}"},
    { MS_SC_UU,     TSCRIPT,     TG::Character,     "char scrU",    "scrU",     "&#",     "\\mathscr{U}"},
    { MS_SC_UV,     TSCRIPT,     TG::Character,     "char scrV",    "scrV",     "&#",     "\\mathscr{V}"},
    { MS_SC_UW,     TSCRIPT,     TG::Character,     "char scrW",    "scrW",     "&#",     "\\mathscr{W}"},
    { MS_SC_UX,     TSCRIPT,     TG::Character,     "char scrX",    "scrX",     "&#",     "\\mathscr{X}"},
    { MS_SC_UY,     TSCRIPT,     TG::Character,     "char scrY",    "scrY",     "&#",     "\\mathscr{Y}"},
    { MS_SC_UZ,     TSCRIPT,     TG::Character,     "char scrZ",    "scrZ",     "&#",     "\\mathscr{Z}"},
    // Lower
    { MS_SC_LA,     TSCRIPT,     TG::Character,     "char scra",    "scra",     "&#",     "\\mathscr{a}"},
    { MS_SC_LB,     TSCRIPT,     TG::Character,     "char scrb",    "scrb",     "&#",     "\\mathscr{b}"},
    { MS_SC_LC,     TSCRIPT,     TG::Character,     "char scrc",    "scrc",     "&#",     "\\mathscr{c}"},
    { MS_SC_LD,     TSCRIPT,     TG::Character,     "char scrd",    "scrd",     "&#",     "\\mathscr{d}"},
    { MS_SC_LE,     TSCRIPT,     TG::Character,     "char scre",    "scre",     "&#",     "\\mathscr{e}"},
    { MS_SC_LF,     TSCRIPT,     TG::Character,     "char scrf",    "scrf",     "&#",     "\\mathscr{f}"},
    { MS_SC_LG,     TSCRIPT,     TG::Character,     "char scrg",    "scrg",     "&#",     "\\mathscr{g}"},
    { MS_SC_LH,     TSCRIPT,     TG::Character,     "char scrh",    "scrh",     "&#",     "\\mathscr{h}"},
    { MS_SC_LI,     TSCRIPT,     TG::Character,     "char scri",    "scri",     "&#",     "\\mathscr{i}"},
    { MS_SC_LJ,     TSCRIPT,     TG::Character,     "char scrj",    "scrj",     "&#",     "\\mathscr{j}"},
    { MS_SC_LK,     TSCRIPT,     TG::Character,     "char scrk",    "scrk",     "&#",     "\\mathscr{k}"},
    { MS_SC_LL,     TSCRIPT,     TG::Character,     "char scrl",    "scrl",     "&#",     "\\mathscr{l}"},
    { MS_SC_LM,     TSCRIPT,     TG::Character,     "char scrm",    "scrm",     "&#",     "\\mathscr{m}"},
    { MS_SC_LN,     TSCRIPT,     TG::Character,     "char scrn",    "scrn",     "&#",     "\\mathscr{n}"},
    { MS_SC_LO,     TSCRIPT,     TG::Character,     "char scro",    "scro",     "&#",     "\\mathscr{o}"},
    { MS_SC_LP,     TSCRIPT,     TG::Character,     "char scrp",    "scrp",     "&#",     "\\mathscr{o}"},
    { MS_SC_LQ,     TSCRIPT,     TG::Character,     "char scrq",    "scrq",     "&#",     "\\mathscr{q}"},
    { MS_SC_LR,     TSCRIPT,     TG::Character,     "char scrr",    "scrr",     "&#",     "\\mathscr{r}"},
    { MS_SC_LS,     TSCRIPT,     TG::Character,     "char scrs",    "scrs",     "&#",     "\\mathscr{s}"},
    { MS_SC_LT,     TSCRIPT,     TG::Character,     "char scrt",    "scrt",     "&#",     "\\mathscr{t}"},
    { MS_SC_LU,     TSCRIPT,     TG::Character,     "char scry",    "scru",     "&#",     "\\mathscr{u}"},
    { MS_SC_LV,     TSCRIPT,     TG::Character,     "char scrv",    "scrv",     "&#",     "\\mathscr{v}"},
    { MS_SC_LW,     TSCRIPT,     TG::Character,     "char scrw",    "scrw",     "&#",     "\\mathscr{w}"},
    { MS_SC_LX,     TSCRIPT,     TG::Character,     "char scrx",    "scrx",     "&#",     "\\mathscr{x}"},
    { MS_SC_LY,     TSCRIPT,     TG::Character,     "char scry",    "scry",     "&#",     "\\mathscr{y}"},
    { MS_SC_LZ,     TSCRIPT,     TG::Character,     "char scrz",    "scrz",     "&#",     "\\mathscr{z}"}
};

// Script Bold Latin letters
const MathType2 aLatinScriptBoldMathTypeTable[] =
{
    //Capital
    { MS_BSC_UA,     TSCRIPT,     TG::Character,     "char bscrA",    "bscrA",     "&#",     "\\mathscr{\\boldsymbol{{A}}"},
    { MS_BSC_UB,     TSCRIPT,     TG::Character,     "char bscrB",    "bscrB",     "&#",     "\\mathscr{\\boldsymbol{{B}}"},
    { MS_BSC_UC,     TSCRIPT,     TG::Character,     "char bscrC",    "bscrC",     "&#",     "\\mathscr{\\boldsymbol{{C}}"},
    { MS_BSC_UD,     TSCRIPT,     TG::Character,     "char bscrD",    "bscrD",     "&#",     "\\mathscr{\\boldsymbol{{D}}"},
    { MS_BSC_UE,     TSCRIPT,     TG::Character,     "char bscrE",    "bscrE",     "&#",     "\\mathscr{\\boldsymbol{{E}}"},
    { MS_BSC_UF,     TSCRIPT,     TG::Character,     "char bscrF",    "bscrF",     "&#",     "\\mathscr{\\boldsymbol{{F}}"},
    { MS_BSC_UG,     TSCRIPT,     TG::Character,     "char bscrG",    "bscrG",     "&#",     "\\mathscr{\\boldsymbol{{G}}"},
    { MS_BSC_UH,     TSCRIPT,     TG::Character,     "char bscrH",    "bscrH",     "&#",     "\\mathscr{\\boldsymbol{{H}}"},
    { MS_BSC_UI,     TSCRIPT,     TG::Character,     "char bscrI",    "bscrI",     "&#",     "\\mathscr{\\boldsymbol{{I}}"},
    { MS_BSC_UJ,     TSCRIPT,     TG::Character,     "char bscrJ",    "bscrJ",     "&#",     "\\mathscr{\\boldsymbol{{J}}"},
    { MS_BSC_UK,     TSCRIPT,     TG::Character,     "char bscrK",    "bscrK",     "&#",     "\\mathscr{\\boldsymbol{{K}}"},
    { MS_BSC_UL,     TSCRIPT,     TG::Character,     "char bscrL",    "bscrL",     "&#",     "\\mathscr{\\boldsymbol{{L}}"},
    { MS_BSC_UM,     TSCRIPT,     TG::Character,     "char bscrM",    "bscrM",     "&#",     "\\mathscr{\\boldsymbol{{M}}"},
    { MS_BSC_UN,     TSCRIPT,     TG::Character,     "char bscrN",    "bscrN",     "&#",     "\\mathscr{\\boldsymbol{{N}}"},
    { MS_BSC_UO,     TSCRIPT,     TG::Character,     "char bscrO",    "bscrO",     "&#",     "\\mathscr{\\boldsymbol{{O}}"},
    { MS_BSC_UP,     TSCRIPT,     TG::Character,     "char bscrP",    "bscrP",     "&#",     "\\mathscr{\\boldsymbol{{P}}"},
    { MS_BSC_UQ,     TSCRIPT,     TG::Character,     "char bscrQ",    "bscrQ",     "&#",     "\\mathscr{\\boldsymbol{{Q}}"},
    { MS_BSC_UR,     TSCRIPT,     TG::Character,     "char bscrR",    "bscrR",     "&#",     "\\mathscr{\\boldsymbol{{R}}"},
    { MS_BSC_US,     TSCRIPT,     TG::Character,     "char bscrS",    "bscrS",     "&#",     "\\mathscr{\\boldsymbol{{S}}"},
    { MS_BSC_UT,     TSCRIPT,     TG::Character,     "char bscrT",    "bscrT",     "&#",     "\\mathscr{\\boldsymbol{{T}}"},
    { MS_BSC_UU,     TSCRIPT,     TG::Character,     "char bscrU",    "bscrU",     "&#",     "\\mathscr{\\boldsymbol{{U}}"},
    { MS_BSC_UV,     TSCRIPT,     TG::Character,     "char bscrV",    "bscrV",     "&#",     "\\mathscr{\\boldsymbol{{V}}"},
    { MS_BSC_UW,     TSCRIPT,     TG::Character,     "char bscrW",    "bscrW",     "&#",     "\\mathscr{\\boldsymbol{{W}}"},
    { MS_BSC_UX,     TSCRIPT,     TG::Character,     "char bscrX",    "bscrX",     "&#",     "\\mathscr{\\boldsymbol{{X}}"},
    { MS_BSC_UY,     TSCRIPT,     TG::Character,     "char bscrY",    "bscrY",     "&#",     "\\mathscr{\\boldsymbol{{Y}}"},
    { MS_BSC_UZ,     TSCRIPT,     TG::Character,     "char bscrZ",    "bscrZ",     "&#",     "\\mathscr{\\boldsymbol{{Z}}"},
    // Lower
    { MS_BSC_LA,     TSCRIPT,     TG::Character,     "char bscra",    "bscra",     "&#",     "\\mathscr{\\boldsymbol{{a}}"},
    { MS_BSC_LB,     TSCRIPT,     TG::Character,     "char bscrb",    "bscrb",     "&#",     "\\mathscr{\\boldsymbol{{b}}"},
    { MS_BSC_LC,     TSCRIPT,     TG::Character,     "char bscrc",    "bscrc",     "&#",     "\\mathscr{\\boldsymbol{{c}}"},
    { MS_BSC_LD,     TSCRIPT,     TG::Character,     "char bscrd",    "bscrd",     "&#",     "\\mathscr{\\boldsymbol{{d}}"},
    { MS_BSC_LE,     TSCRIPT,     TG::Character,     "char bscre",    "bscre",     "&#",     "\\mathscr{\\boldsymbol{{e}}"},
    { MS_BSC_LF,     TSCRIPT,     TG::Character,     "char bscrf",    "bscrf",     "&#",     "\\mathscr{\\boldsymbol{{f}}"},
    { MS_BSC_LG,     TSCRIPT,     TG::Character,     "char bscrg",    "bscrg",     "&#",     "\\mathscr{\\boldsymbol{{g}}"},
    { MS_BSC_LH,     TSCRIPT,     TG::Character,     "char bscrh",    "bscrh",     "&#",     "\\mathscr{\\boldsymbol{{h}}"},
    { MS_BSC_LI,     TSCRIPT,     TG::Character,     "char bscri",    "bscri",     "&#",     "\\mathscr{\\boldsymbol{{i}}"},
    { MS_BSC_LJ,     TSCRIPT,     TG::Character,     "char bscrj",    "bscrj",     "&#",     "\\mathscr{\\boldsymbol{{j}}"},
    { MS_BSC_LK,     TSCRIPT,     TG::Character,     "char bscrk",    "bscrk",     "&#",     "\\mathscr{\\boldsymbol{{k}}"},
    { MS_BSC_LL,     TSCRIPT,     TG::Character,     "char bscrl",    "bscrl",     "&#",     "\\mathscr{\\boldsymbol{{l}}"},
    { MS_BSC_LM,     TSCRIPT,     TG::Character,     "char bscrm",    "bscrm",     "&#",     "\\mathscr{\\boldsymbol{{m}}"},
    { MS_BSC_LN,     TSCRIPT,     TG::Character,     "char bscrn",    "bscrn",     "&#",     "\\mathscr{\\boldsymbol{{n}}"},
    { MS_BSC_LO,     TSCRIPT,     TG::Character,     "char bscro",    "bscro",     "&#",     "\\mathscr{\\boldsymbol{{o}}"},
    { MS_BSC_LP,     TSCRIPT,     TG::Character,     "char bscrp",    "bscrp",     "&#",     "\\mathscr{\\boldsymbol{{o}}"},
    { MS_BSC_LQ,     TSCRIPT,     TG::Character,     "char bscrq",    "bscrq",     "&#",     "\\mathscr{\\boldsymbol{{q}}"},
    { MS_BSC_LR,     TSCRIPT,     TG::Character,     "char bscrr",    "bscrr",     "&#",     "\\mathscr{\\boldsymbol{{r}}"},
    { MS_BSC_LS,     TSCRIPT,     TG::Character,     "char bscrs",    "bscrs",     "&#",     "\\mathscr{\\boldsymbol{{s}}"},
    { MS_BSC_LT,     TSCRIPT,     TG::Character,     "char bscrt",    "bscrt",     "&#",     "\\mathscr{\\boldsymbol{{t}}"},
    { MS_BSC_LU,     TSCRIPT,     TG::Character,     "char bscry",    "bscru",     "&#",     "\\mathscr{\\boldsymbol{{u}}"},
    { MS_BSC_LV,     TSCRIPT,     TG::Character,     "char bscrv",    "bscrv",     "&#",     "\\mathscr{\\boldsymbol{{v}}"},
    { MS_BSC_LW,     TSCRIPT,     TG::Character,     "char bscrw",    "bscrw",     "&#",     "\\mathscr{\\boldsymbol{{w}}"},
    { MS_BSC_LX,     TSCRIPT,     TG::Character,     "char bscrx",    "bscrx",     "&#",     "\\mathscr{\\boldsymbol{{x}}"},
    { MS_BSC_LY,     TSCRIPT,     TG::Character,     "char bscry",    "bscry",     "&#",     "\\mathscr{\\boldsymbol{{y}}"},
    { MS_BSC_LZ,     TSCRIPT,     TG::Character,     "char bscrz",    "bscrz",     "&#",     "\\mathscr{\\boldsymbol{{z}}"}
};

/**
  * Hebrew
  */

const MathType2 aHebrewTypeTable[] =
{
    // Hebrew
    // Punctuation
    { MS_MAQAF,          THEBREW,    TG::Character,    "char maqaf",         "maqaf",         "&#",    nullptr},
    { MS_PASEQ,          THEBREW,    TG::Character,    "char paseq",         "paseq",         "&#",    nullptr},
    { MS_SOLFPASUQ,      THEBREW,    TG::Character,    "char solfpasq",      "solfpasq",      "&#",    nullptr},
    { MS_NUNHAFUKHA,     THEBREW,    TG::Character,    "char nunhafukha",    "nunhafukha",    "&#",    nullptr},
    // Letters
    { MS_SMALEPH,        TALEPH,     TG::Standalone,   "aleph",              "smaleph",       "&#",    nullptr},
    { MS_ALEPH,          THEBREW,    TG::Character,    "char aleph",         "aleph",         "&#",    nullptr},
    { MS_BET,            TBETH,      TG::Character,    "beth",               "beth",          "&#",    nullptr},
    { MS_GIMEL,          TGIMEL,     TG::Character,    "gimel",              "gimel",         "&#",    nullptr},
    { MS_DALET,          THEBREW,    TG::Character,    "char dalet",         "dalet",         "&#",    nullptr},
    { MS_HE,             THEBREW,    TG::Character,    "char he",            "he",            "&#",    nullptr},
    { MS_VAV,            THEBREW,    TG::Character,    "char vav",           "vav",           "&#",    nullptr},
    { MS_ZAYIN,          THEBREW,    TG::Character,    "char zayin",         "zayin",         "&#",    nullptr},
    { MS_HET,            THEBREW,    TG::Character,    "char het",           "het",           "&#",    nullptr},
    { MS_TET,            THEBREW,    TG::Character,    "char tet",           "tet",           "&#",    nullptr},
    { MS_YOD,            THEBREW,    TG::Character,    "char yod",           "yod",           "&#",    nullptr},
    { MS_FINALKAF,       THEBREW,    TG::Character,    "char fkaf",          "fkaf",          "&#",    nullptr},
    { MS_KAF,            THEBREW,    TG::Character,    "char kaf",           "kaf",           "&#",    nullptr},
    { MS_LAMED,          THEBREW,    TG::Character,    "char lamed",         "lamed",         "&#",    nullptr},
    { MS_FINALMEM,       THEBREW,    TG::Character,    "char fmem",          "fmem",          "&#",    nullptr},
    { MS_MEM,            THEBREW,    TG::Character,    "char mem",           "mem",           "&#",    nullptr},
    { MS_FINALNUN,       THEBREW,    TG::Character,    "char fnun",          "fnun",          "&#",    nullptr},
    { MS_NUN,            THEBREW,    TG::Character,    "char nun",           "nun",           "&#",    nullptr},
    { MS_SAMEKH,         THEBREW,    TG::Character,    "char samekh",        "samekh",        "&#",    nullptr},
    { MS_AYIN,           THEBREW,    TG::Character,    "char ayin",          "ayin",          "&#",    nullptr},
    { MS_FINELPE,        THEBREW,    TG::Character,    "char fpe",           "fpe",           "&#",    nullptr},
    { MS_PE,             THEBREW,    TG::Character,    "char pe",            "pe",            "&#",    nullptr},
    { MS_FINALTSADI,     THEBREW,    TG::Character,    "char ftsadi",        "ftsadi",        "&#",    nullptr},
    { MS_TSADI,          THEBREW,    TG::Character,    "char tsadi",         "tsadi",         "&#",    nullptr},
    { MS_QOF,            THEBREW,    TG::Character,    "char quf",           "quf",           "&#",    nullptr},
    { MS_RESH,           THEBREW,    TG::Character,    "char resh",          "resh",          "&#",    nullptr},
    { MS_SHIN,           THEBREW,    TG::Character,    "char shin",          "shin",          "&#",    nullptr},
    { MS_TAV,            THEBREW,    TG::Character,    "char tav",           "tav",           "&#",    nullptr},
    // others
    { MS_YDDOUBLEVAV,    THEBREW,    TG::Character,    "char ydvav",         "ydvav",         "&#",    nullptr},
    { MS_YVAVYOD,        THEBREW,    TG::Character,    "char yvavyod",       "yvavyod",       "&#",    nullptr},
    { MS_YDYOD,          THEBREW,    TG::Character,    "char ydyod",         "ydyod",         "&#",    nullptr},
    { MS_GERESH,         THEBREW,    TG::Character,    "char geresh",        "geresh",        "&#",    nullptr},
    { MS_GERSHAYIM,      THEBREW,    TG::Character,    "char gereshayim",    "gereshayim",    "&#",    nullptr }
};


/**
  * Phoenician
  */

const MathType2 aPhoenicianTypeTable[] =
{
    { MS_ALF,         TPHOENICIAN,    TG::Character,    "char alf",         "alf",         "&#",    nullptr},
    { MS_PBET,        TPHOENICIAN,    TG::Character,    "char pbet",        "pbet",        "&#",    nullptr},
    { MS_GAML,        TPHOENICIAN,    TG::Character,    "char gaml",        "gaml",        "&#",    nullptr},
    { MS_DELT,        TPHOENICIAN,    TG::Character,    "char delt",        "delt",        "&#",    nullptr},
    { MS_PHE,         TPHOENICIAN,    TG::Character,    "char phe",         "phe",         "&#",    nullptr},
    { MS_WAU,         TPHOENICIAN,    TG::Character,    "char wau",         "wau",         "&#",    nullptr},
    { MS_ZAI,         TPHOENICIAN,    TG::Character,    "char zai",         "zai",         "&#",    nullptr},
    { MS_PHET,        TPHOENICIAN,    TG::Character,    "char phet",        "phet",        "&#",    nullptr},
    { MS_PTET,        TPHOENICIAN,    TG::Character,    "char ptet",        "ptet",        "&#",    nullptr},
    { MS_PYOD,        TPHOENICIAN,    TG::Character,    "char pyod",        "pyod",        "&#",    nullptr},
    { MS_PKAF,        TPHOENICIAN,    TG::Character,    "char pkaf",        "pkaf",        "&#",    nullptr},
    { MS_LAMD,        TPHOENICIAN,    TG::Character,    "char lamd",        "lamd",        "&#",    nullptr},
    { MS_PMEM,        TPHOENICIAN,    TG::Character,    "char pmem",        "pmem",        "&#",    nullptr},
    { MS_PNUN,        TPHOENICIAN,    TG::Character,    "char snun",        "snun",        "&#",    nullptr},
    { MS_SEMK,        TPHOENICIAN,    TG::Character,    "char seml",        "seml",        "&#",    nullptr},
    { MS_AIN,         TPHOENICIAN,    TG::Character,    "char ain",         "ain",         "&#",    nullptr},
    { MS_PPE,         TPHOENICIAN,    TG::Character,    "char ppe",         "ppe",         "&#",    nullptr},
    { MS_SADE,        TPHOENICIAN,    TG::Character,    "char sade",        "sade",        "&#",    nullptr},
    { MS_PQOF,        TPHOENICIAN,    TG::Character,    "char pqof",        "pqof",        "&#",    nullptr},
    { MS_ROSH,        TPHOENICIAN,    TG::Character,    "char rosh",        "rosh",        "&#",    nullptr},
    { MS_PSHIN,       TPHOENICIAN,    TG::Character,    "char pshin",       "pshin",       "&#",    nullptr},
    { MS_TAU,         TPHOENICIAN,    TG::Character,    "char ptau",        "ptau",        "&#",    nullptr},
    { MS_PWORDSEP,    TPHOENICIAN,    TG::Character,    "char pwordsep",    "pwordsep",    "&#",    nullptr}
};

/**
 * Gothic
 */

const MathType2 aGothicTypeTable[] =
{
// Gothic
    { MS_AHSA,          TGOTHIC,    TG::Character,    "char ahsa",          "ahsa",           "ahsa",           nullptr},
    { MS_BAIRKAN,       TGOTHIC,    TG::Character,    "char baikran",       "baikran",        "baikran",        nullptr},
    { MS_GIBA,          TGOTHIC,    TG::Character,    "char giba",          "giba",           "giba",           nullptr},
    { MS_DAGS,          TGOTHIC,    TG::Character,    "char dags",          "dags",           "dags",           nullptr},
    { MS_AIHVUS,        TGOTHIC,    TG::Character,    "char aihvus",        "aihvus",         "aihvus",         nullptr},
    { MS_QAIRTHRA,      TGOTHIC,    TG::Character,    "char qairthra",      "qairthra",       "qairthra",       nullptr},
    { MS_IUJA,          TGOTHIC,    TG::Character,    "char iuja",          "iuja",           "iuja",           nullptr},
    { MS_HAGL,          TGOTHIC,    TG::Character,    "char hagl",          "hagl",           "hagl",           nullptr},
    { MS_THIUTH,        TGOTHIC,    TG::Character,    "char thiuth",        "thiuth",         "thiuth",         nullptr},
    { MS_EIS,           TGOTHIC,    TG::Character,    "char eis",           "eis",            "eis",            nullptr},
    { MS_KUSMA,         TGOTHIC,    TG::Character,    "char kusma",         "kusma",          "kusma",          nullptr},
    { MS_LAGUS,         TGOTHIC,    TG::Character,    "char lagus",         "lagus",          "lagus",          nullptr},
    { MS_MANNA,         TGOTHIC,    TG::Character,    "char manna",         "manna",          "manna",          nullptr},
    { MS_NAUTHS,        TGOTHIC,    TG::Character,    "char nauths",        "nauths",         "nauths",         nullptr},
    { MS_JER,           TGOTHIC,    TG::Character,    "char jer",           "jer",            "jer",            nullptr},
    { MS_URUS,          TGOTHIC,    TG::Character,    "char urus",          "urus",           "urus",           nullptr},
    { MS_PAIRTHRA,      TGOTHIC,    TG::Character,    "char pairthra",      "pairthra",       "pairthra",       nullptr},
    { MS_NINETY,        TGOTHIC,    TG::Character,    "char ninety",        "ninety",         "ninety",         nullptr},
    { MS_RAIDA,         TGOTHIC,    TG::Character,    "char raida",         "raida",          "raida",          nullptr},
    { MS_SAUIL,         TGOTHIC,    TG::Character,    "char sauil",         "sauil",          "sauil",          nullptr},
    { MS_TEIWS,         TGOTHIC,    TG::Character,    "char teiws",         "teiws",          "teiws",          nullptr},
    { MS_WINJA,         TGOTHIC,    TG::Character,    "char winja",         "winja",          "winja",          nullptr},
    { MS_FAIHU,         TGOTHIC,    TG::Character,    "char faihu",         "faihu",          "faihu",          nullptr},
    { MS_IGGWS,         TGOTHIC,    TG::Character,    "char iggws",         "iggws",          "iggws",          nullptr},
    { MS_HWAIR,         TGOTHIC,    TG::Character,    "char hwair",         "hwair",          "hwair",          nullptr},
    { MS_OTHAL,         TGOTHIC,    TG::Character,    "char othal",         "othal",          "othal",          nullptr},
    { MS_NINEHUNDRED,   TGOTHIC,    TG::Character,    "char ninehundred",   "ninehundred",    "ninehundred",    nullptr}
};

/**
  * International symbols
  */

// Latin letters International
const MathType2 aInternationalMathTypeTable[] =
{
    { MS_UAE,        TINTERNATIONAL,    TG::Character,    "char AE",           "AE",           "&#",    nullptr}, // TODO latex
    { MS_UAO,        TINTERNATIONAL,    TG::Character,    "char AO",           "AO",           "&#",    nullptr}, // TODO latex
    { MS_UOE,        TINTERNATIONAL,    TG::Character,    "char OE",           "OE",           "&#",    nullptr}, // TODO latex
    { MS_UOO,        TINTERNATIONAL,    TG::Character,    "char OO",           "OO",           "&#",    nullptr}, // TODO latex
    { MS_UAA,        TINTERNATIONAL,    TG::Character,    "char AA",           "AA",           "&#",    nullptr}, // TODO latex
    { MS_UCCEDILLE,  TINTERNATIONAL,    TG::Character,    "char Ccedille",     "Ccedille",     "&#",    nullptr}, // TODO latex
    { MS_UGCCEDILLE, TINTERNATIONAL,    TG::Character,    "char gCcedille",    "gCcedille",    "&#",    nullptr}, // TODO latex
    { MS_UTILDEN,    TINTERNATIONAL,    TG::Character,    "char tildeN",       "tildeN",       "&#",    nullptr}, // TODO latex
    { MS_LAE,        TINTERNATIONAL,    TG::Character,    "char ae",           "ae",           "&#",    nullptr}, // TODO latex
    { MS_LOE,        TINTERNATIONAL,    TG::Character,    "char oe",           "oe",           "&#",    nullptr}, // TODO latex
    { MS_LAO,        TINTERNATIONAL,    TG::Character,    "char ao",           "ao",           "&#",    nullptr}, // TODO latex
    { MS_LOO,        TINTERNATIONAL,    TG::Character,    "char oo",           "oo",           "&#",    nullptr}, // TODO latex
    { MS_LAA,        TINTERNATIONAL,    TG::Character,    "char aa",           "aa",           "&#",    nullptr}, // TODO latex
    { MS_LQP,        TINTERNATIONAL,    TG::Character,    "char qp",           "qp",           "&#",    nullptr}, // TODO latex
    { MS_LDB,        TINTERNATIONAL,    TG::Character,    "char db",           "db",           "&#",    nullptr}, // TODO latex
    { MS_LCCEDILLE,  TINTERNATIONAL,    TG::Character,    "char ccedille",     "ccedille",     "&#",    nullptr}, // TODO latex
    { MS_LGCCEDILLE, TINTERNATIONAL,    TG::Character,    "char gccedille",    "gccedille",    "&#",    nullptr}, // TODO latex
    { MS_LTILDEN,    TINTERNATIONAL,    TG::Character,    "char tilden",       "tilden",       "&#",    nullptr}  // TODO latex
};

/**
  * Currency
  */

// Currency
const MathType2 aCurrencyMathTypeTable[] =
{
    { MS_CURRENCY,    TCURRENCY,    TG::Character,    "char currency",    "currency",    "&#",    nullptr},  // TODO latex
    { MS_COLON,       TCURRENCY,    TG::Character,    "char colon",       "colon",       "&#",    nullptr},  // TODO latex
    { MS_CRUZEIRO,    TCURRENCY,    TG::Character,    "char cruzeiro",    "cruzeiro",    "&#",    nullptr},  // TODO latex
    { MS_FRANC,       TCURRENCY,    TG::Character,    "char franc",       "franc",       "&#",    nullptr},  // TODO latex
    { MS_LIRA,        TCURRENCY,    TG::Character,    "char lira",        "lira",        "&#",    nullptr},  // TODO latex
    { MS_RUPEE,       TCURRENCY,    TG::Character,    "char rupee",       "rupee",       "&#",    nullptr},  // TODO latex
    { MS_WON,         TCURRENCY,    TG::Character,    "char won",         "won",         "&#",    nullptr},  // TODO latex
    { MS_DONG,        TCURRENCY,    TG::Character,    "char dong",        "dong",        "&#",    nullptr},  // TODO latex
    { MS_YEN,         TCURRENCY,    TG::Character,    "char yen",         "yen",         "&#",    nullptr},  // TODO latex
    { MS_EURO,        TCURRENCY,    TG::Character,    "char euro",        "euro",        "&#",    nullptr},  // TODO latex
    { MS_CENT,        TCURRENCY,    TG::Character,    "char cent",        "cent",        "&#",    nullptr},  // TODO latex
    { MS_DOLLAR,      TCURRENCY,    TG::Character,    "char dollar",      "dollar",      "&#",    nullptr},  // TODO latex
    { MS_DOLLAR,      TCURRENCY,    TG::Character,    "$",                "$",           "&#",    nullptr},  // TODO latex
    { MS_CDRAM,       TCURRENCY,    TG::Character,    "char cdram",       "cdram",       "&#",    nullptr}   // TODO latex
};

/**
  * Math
  */

const MathType2 aConstantsAndOthersTypeTable [] =
{
    { MS_PLANK,          TPLANK,         TG::Standalone,     "plank",            "plank",        "&#",     nullptr},
    { MS_PLANKBAR,       THBAR,          TG::Standalone,     "hbar",             "hbar",         "&#",     nullptr},
    { MS_EULER,          TEULER,         TG::Standalone,     "euler",            "euler",        "&#",     nullptr},
    { MS_LAMBDABAR,      TLAMBDABAR,     TG::Standalone,     "lambdabar",        "lambdabar",    "&#",     nullptr},
    { MS_NATURALEXP,     TNATURALEXP,    TG::Standalone,     "nexp",             "nexp",         "&#",     nullptr},
    { MS_IM,             TIM,            TG::Standalone,     "im",               "cim",          "&#",     nullptr},
    { MS_RE,             TRE,            TG::Standalone,     "re",               "cre",          "&#",     nullptr},
    { MS_WP,             TWP,            TG::Standalone,     "wp",               "wp",           "&#",     nullptr},
    { MS_LETH,           TETH,           TG::Standalone,     "leth",             "eth",          "&#",     nullptr},
    { MS_UETH,           TETH,           TG::Standalone,     "ueth",             "Eth",          "&#",     nullptr},
    { MS_EULERCTE,       TEULERCTE,      TG::Standalone,     "eulercte",         "eulercte",     "&#",     nullptr},
    { MS_ICOMPLEX,       TICOMPLEX,      TG::Standalone,     "icomp",            "icomp",        "&#",     nullptr},
    { MS_JCOMPLEX,       TJCOMPLEX,      TG::Standalone,     "jcomp",            "jcomp",        "&#",     nullptr},
    { MS_INFINITY,       TINFINITY,      TG::Standalone,     "infinity",         "inf",          "&#",     nullptr},
    { MS_UDIGAMMA,       TDIGAMMA,       TG::Standalone,     "udigamma",         "Digamma",      "&#",     nullptr},
    { MS_LDIGAMMA,       TDIGAMMA,       TG::Standalone,     "ldigamma",         "digamma",      "&#",     nullptr},
    { MS_LI_NODOT,       TINODOT,        TG::Standalone,     "inodot",           "inodot",       "&#",     nullptr},
    { MS_LJ_NODOT,       TJNODOT,        TG::Standalone,     "jnodot",           "jnodot",       "&#",     nullptr},
    { MS_TOMBSTONE,      TTOMBSTONE,     TG::Standalone,     "tombstone",        "tombstone",    "&#",     nullptr},
    { MS_TOMBSTONE,      TTOMBSTONE,     TG::Standalone,     "tombstone",        "CQFD",         "&#",     nullptr},
    { MS_B_UDIGAMMA,     TDIGAMMA,       TG::Standalone,     "char bDigamma",    "bDigamma",     "&#",     nullptr},
    { MS_B_LDIGAMMA,     TDIGAMMA,       TG::Standalone,     "char bdigamma",    "bdigamma",     "&#",     nullptr},
    { MS_I_LI_NODOT,     TINODOT,        TG::Standalone,     "char iinodot",     "iinodot",      "&#",     nullptr},
    { MS_I_LJ_NODOT,     TJNODOT,        TG::Standalone,     "char ijnodot",     "ijnodot",      "&#",     nullptr},
    { MS_MICRO,          TMICRO,         TG::Standalone,     "micro",            "micro",        "&#",     nullptr},
    { MS_LITRE,          TLITRE,         TG::Standalone,     "litre",            "litre",        "&#",     nullptr},
    { MS_AMSTRONG,       TAMSTRONG,      TG::Standalone,     "amstrong",         "amstrong",     "&#",     nullptr},
    { MS_OUNCE,          TOUNCE,         TG::Standalone,     "ounce",            "ounce",        "&#",     nullptr},
    { MS_SIEMENS,        TSIEMENS,       TG::Standalone,     "siemens",          "siemens",      "&#",     nullptr},
    { MS_MORDINALI,      TMORDINALI,     TG::Standalone,     "char mordinali",   "mordinali",    "&#",     nullptr},
    { MS_FORDINALI,      TFORDINALI,     TG::Standalone,     "char fordinali",   "fordinali",    "&#",     nullptr},
    { MS_NUMBER,         TNUMSIGN,       TG::Standalone,     "num",              "num",          "&#",     nullptr},
    { MS_AMPERSAND,      TAMPERSAND,     TG::Standalone,     "amper",            "amper",        "&#",     nullptr},
    { MS_DEGREE,         TDEGREE,        TG::Standalone,     "degree",           "degree",       "&#",     nullptr}
};

const MathType2 aSimpleOperTypeTable [] =
{
    { MS_NEQ,            TNEQ,             TG::Relation,           "neq",         "neq",           "&#",     nullptr},
    { MS_PLUS,           TPLUS,        TG::UnOper | TG::Sum,       "+",           "plus",          "&#",     nullptr},
    { MS_MINUS,          TMINUS,       TG::UnOper | TG::Sum,       "-",           "minus",         "&#",     nullptr},
    { MS_MULTIPLY,       TMULTIPLY,        TG::Product,            "*",           "multiply",      "&#",     nullptr},
    { MS_PLUSMINUS,      TPLUSMINUS,   TG::UnOper | TG::Sum,       "+-",          "plusminus",     "&#",     nullptr},
    { MS_MINUSPLUS,      TMINUSPLUS,   TG::UnOper | TG::Sum,       "-+",          "minusplus",     "&#",     nullptr},
    { MS_PLUSDOT,        TPLUSDOT,     TG::UnOper | TG::Sum,       ".+",          "plusdot",       "&#",     nullptr},
    { MS_SLASH,          TDIVIDEBY,        TG::Product,            "/",           "divideby",      "&#",     nullptr},
    { MS_TIMES,          TTIMES,           TG::Product,            "times",       "times",         "&#",     nullptr},
    { MS_CDOT,           TCDOT,            TG::Product,            "cdot",        "cdot",          "&#",     nullptr},
    { MS_DIV,            TDIV,             TG::Product,            "div",         "div",           "&#",     nullptr},
    { MS_COMPOSITION,    TCOMPOSITION,     TG::Sum,                "comp",        "comp",          "&#",     nullptr},
    { MS_COMPOSITION,    TCIRCLE,          TG::Standalone,         "circ",        "circ",          "&#",     nullptr},
    { MS_BULLET,         TBULLET,          TG::Standalone,         "bullet",      "bullet",        "&#",     nullptr},
    { MS_INCREMENT,      TINCREMENT,       TG::Standalone,         "increment",   "increment",     "&#",     nullptr},
    { MS_PARTIAL,        TPARTIAL,         TG::Standalone,         "partial",     "partial",       "&#",     nullptr},
    { MS_PRIME,          TPRIME,           TG::Standalone,         "prime",       "prime",         "&#",     nullptr},
    { MS_DPRIME,         TDPRIME,          TG::Standalone,         "dprime",      "dprime",        "&#",     nullptr},
    { MS_TPRIME,         TTPRIME,          TG::Standalone,         "tprime",      "tprime",        "&#",     nullptr},
    { MS_NABLA,          TNABLA,           TG::Standalone,         "nabla",       "nabla",         "&#",     nullptr},
    { MS_UDIFF,          TUDIFF,           TG::Standalone,         "Diff",        "Diff",          "&#",     nullptr},
    { MS_LDIFF,          TLDIFF,           TG::Standalone,         "diff",        "diff",          "&#",     nullptr},
    { MS_LAPLACE,        TLAPLACE,         TG::Standalone,         "laplace",     "laplace",       "&#",     nullptr},
    { MS_FOURIER,        TFOURIER,         TG::Standalone,         "fourrier",    "fourrier",      "&#",     nullptr}
};

const MathType2 aOpperatorsTypeTable [] =
{
    { MS_PROD,                   TPROD,             TG::Oper,     "prod",        "prod",         "&#",     nullptr},
    { MS_COPROD,                 TCOPROD,           TG::Oper,     "coprod",      "coprod",       "&#",     nullptr},
    { MS_SUM,                    TSUM,              TG::Oper,     "sum",         "sum",          "&#",     nullptr},
    { MS_OAND,                   TOAND,             TG::Oper,     "oand",        "oand",         "&#",     nullptr},
    { MS_OOR,                    TOOR,              TG::Oper,     "oor",         "oor",          "&#",     nullptr},
    { MS_INT,                    TINT,              TG::Oper,     "int",         "int",          "&#",     nullptr},
    { MS_IINT,                   TIINT,             TG::Oper,     "iint",        "iint",         "&#",     nullptr},
    { MS_IIINT,                  TIIINT,            TG::Oper,     "iiint",       "iiint",        "&#",     nullptr},
    { MS_IIIINT,                 TIIIINT,           TG::Oper,     "iiiint",      "iiiint",       "&#",     nullptr},
    { MS_LINT,                   TLINT,             TG::Oper,     "lint",        "lint",         "&#",     nullptr},
    { MS_LLINT,                  TLLINT,            TG::Oper,     "llint",       "llint",        "&#",     nullptr},
    { MS_LLLINT,                 TLLLINT,           TG::Oper,     "lllint",      "lllint",       "&#",     nullptr},
    { MS_FINITEPARTINT,          TFILINT,           TG::Oper,     "filint",      "filint",       "&#",     nullptr},
    { MS_DBINT,                  TDBINT,            TG::Oper,     "dbint",       "dbint",        "&#",     nullptr},
    { MS_AVERAGEINT,             TAVERAGE,          TG::Oper,     "average",     "average",      "&#",     nullptr},
    { MS_CIRCULATIONFUNCINT,     TCLINT,            TG::Oper,     "clint",       "clint",        "&#",     nullptr},
    { MS_ANTICLCKWISEINT,        TACLINT,           TG::Oper,     "aclint",      "aclint",       "&#",     nullptr},
    { MS_RECTARROUNDPOLEINT,     TSQPINT,           TG::Oper,     "sqpint",      "sqpint",       "&#",     nullptr},
    { MS_CIRCARROUNDPOLEINT,     TCIPINT,           TG::Oper,     "cipint",      "cipint",       "&#",     nullptr},
    { MS_WITHPOLEINT,            TWPINT,            TG::Oper,     "wpint",       "wpint",        "&#",     nullptr},
    { MS_ARROUNDPOINT,           TAPLINT,           TG::Oper,     "aplint",      "aplint",       "&#",     nullptr},
    { MS_QUATERNIONINT,          TQUAINT,           TG::Oper,     "quaint",      "quaint",       "&#",     nullptr},
    { MS_LARROWINT,              TLARROWINT,        TG::Oper,     "larrowint",   "larrowint",    "&#",     nullptr},
    { MS_TIMESINT,               TTIMESINT,         TG::Oper,     "timesint",    "timesint",     "&#",     nullptr},
    { MS_UNIONINT,               TUNIONINT,         TG::Oper,     "unionint",    "unionint",     "&#",     nullptr},
    { MS_INTERSECTINT,           TINTERSECTINT,     TG::Oper,     "interint",    "interint",     "&#",     nullptr},
    { MS_OVERBARINT,             TOBINT,            TG::Oper,     "obint",       "obint",        "&#",     nullptr},
    { MS_UNDERBARINT,            TUBINT,            TG::Oper,     "ubint" ,      "ubint",        "&#",     nullptr},
    { MS_SUMINT,                 TSUMINT,           TG::Oper,     "sumint",      "sumint",       "&#",     nullptr},
    { MS_OINTERSECTION,          TOINTERSECT,       TG::Oper,     "ointersect",  "ointersect",   "&#",     nullptr},
    { MS_OUNION,                 TOUNION,           TG::Oper,     "ounion",      "ounion",       "&#",     nullptr},
    { MS_OOPLUS,                 TOOPLUS,           TG::Oper,     "ooplus",      "ooplus",       "&#",     nullptr},
    { MS_OODOT,                  TOODOT,            TG::Oper,     "oodot",       "oodot",        "&#",     nullptr},
    { MS_OOTIMES,                TOOTIMES,          TG::Oper,     "ootimes",     "ootimes",      "&#",     nullptr}
};

const MathType2 aMathSetsTypeTable [] =
{
    { MS_FORALL,              TFORALL,         TG::Standalone,    "forall",         "forall",         "&#",     nullptr},
    { MS_EXISTS,              TEXISTS,         TG::Standalone,    "exists",         "exists",         "&#",     nullptr},
    { MS_NOTEXISTS,           TNOTEXISTS,      TG::Standalone,    "nexists",        "nexists",        "&#",     nullptr},
    { MS_COMPLEMENT,          TCOMPLEMENT,     TG::Standalone,    "complement",     "complement",     "&#",     nullptr},
    { MS_IN,                  TIN,             TG::Relation,      "in",             "in",             "&#",     nullptr},
    { MS_NOTIN,               TNOTIN,          TG::Relation,      "notin",          "notin",          "&#",     nullptr},
    { MS_OWNS,                TOWNS,           TG::Relation,      "owns",           "owns",           "&#",     nullptr},
    { MS_NOTOWNS,             TNOTOWNS,        TG::Relation,      "nowns",          "nowns",          "&#",     nullptr},
    { MS_ELEMENTOF,           TELEMENTOF,      TG::Relation,      "elementof",      "elementof",      "&#",     nullptr},
    { MS_CONTAINSASMEMBER,    TCONTASMEMBER,   TG::Relation,      "contains",       "contains",       "&#",     nullptr},
    { MS_INTERSECT,           TINTERSECT,      TG::Product,       "intersect",      "intersect",      "&#",     nullptr},
    { MS_SQINTERSECT,         TSQINTERSECT,    TG::Product,       "sqintersect",    "sqintersect",    "&#",     nullptr},
    { MS_UNION,               TUNION,          TG::Sum,           "union",          "union",          "&#",     nullptr},
    { MS_SQUNION,             TSQUNION,        TG::Sum,           "squnion",        "squnion",        "&#",     nullptr},
    { MS_SUBSET,              TSUBSET,         TG::Relation,      "subset",         "subset",         "&#",     nullptr},
    { MS_SUBSETEQ,            TSUBSETEQ,       TG::Relation,      "subseteq",       "subseteq",       "&#",     nullptr},
    { MS_SQSUBSET,            TSQSUBSET,       TG::Relation,      "sqsubset",       "sqsubset",       "&#",     nullptr},
    { MS_SQSUBSETEQ,          TSQSUBSETEQ,     TG::Relation,      "sqsubseteq",     "sqsubseteq",     "&#",     nullptr},
    { MS_NSUBSET,             TNSUBSET,        TG::Relation,      "nsubset",        "nsubset",        "&#",     nullptr},
    { MS_NSUBSETEQ,           TNSUBSETEQ,      TG::Relation,      "nsubseteq",      "nsubseteq",      "&#",     nullptr},
    { MS_SUPSET,              TSUPSET,         TG::Relation,      "supset",         "supset",         "&#",     nullptr},
    { MS_SUPSETEQ,            TSUPSETEQ,       TG::Relation,      "supseteq",       "supseteq",       "&#",     nullptr},
    { MS_SQSUPSET,            TSQSUPSET,       TG::Relation,      "sqsupset",       "sqsupset",       "&#",     nullptr},
    { MS_SQSUPSETEQ,          TSQSUPSETEQ,     TG::Relation,      "sqsupseteq",     "sqsupseteq",     "&#",     nullptr},
    { MS_NSUPSET,             TNSUBSET,        TG::Relation,      "nsubset",        "nsubset",        "&#",     nullptr},
    { MS_NSUPSETEQ,           TNSUPSETEQ,      TG::Relation,      "nsubseteq",      "nsubseteq ",     "&#",     nullptr}
};

const MathType2 aCompareTypeTable [] =
{
    { MS_LT,               TLT,              TG::Relation,     "<",            "lt",           "&#",     nullptr},
    { MS_GT,               TGT,              TG::Relation,     ">",            "gt",           "&#",     nullptr},
    { MS_LE,               TLE,              TG::Relation,     "<=",           "le",           "&#",     nullptr},
    { MS_GE,               TGE,              TG::Relation,     ">=",           "ge",           "&#",     nullptr},
    { MS_LESLANT,          TLESLANT,         TG::Relation,     "leslant",      "leslant",      "&#",     nullptr},
    { MS_GESLANT,          TGESLANT,         TG::Relation,     "geslant",      "geslant",      "&#",     nullptr},
    { MS_LL,               TLL,              TG::Relation,     "<<",           "ll",           "&#",     nullptr},
    { MS_GG,               TGG,              TG::Relation,     ">>",           "gg",           "&#",     nullptr},
    { MS_LLL,              TLLL,             TG::Relation,     "<<<",          "lll",          "&#",     nullptr},
    { MS_GGG,              TGGG,             TG::Relation,     ">>>",          "ggg",          "&#",     nullptr},
    { MS_SIM,              TSIM,             TG::Relation,     "sim",          "sim",          "&#",     nullptr},
    { MS_SIMEQ,            TSIMEQ,           TG::Relation,     "simeq",        "simeq",        "&#",     nullptr},
    { MS_APPROX,           TAPPROX,          TG::Relation,     "approx",       "approx",       "&#",     nullptr},
    { MS_DEF,              TDEF,             TG::Relation,     "def",          "def",          "&#",     nullptr},
    { MS_EQUIV,            TEQUIV,           TG::Relation,     "equiv",        "equiv",        "&#",     nullptr},
    { MS_PROP,             TPROP,            TG::Relation,     "prop",         "prop",         "&#",     nullptr},
    { MS_EXESS,            TEXESS,           TG::Relation,     "exess",        "exess",        "&#",     nullptr},
    { MS_DIVIDES,          TDIVIDES,         TG::Relation,     "divides",      "divides",      "&#",     nullptr},
    { MS_NDIVIDES,         TNDIVIDES,        TG::Relation,     "ndivides",     "ndivides",     "&#",     nullptr},
    { MS_NDIVIDESWITH,     TNDIVIDESWITH,    TG::Relation,     "ndivswith",    "ndivswith",    "&#",     nullptr}
};

// Order
const MathType2 aOrderTypeTable [] =
{
    { MS_PRECEDES,          TPRECEDES,          TG::Relation,     "prec",           "prec",           "&#",     nullptr},
    { MS_PRECEDESEQUAL,     TPRECEDESEQUAL,     TG::Relation,     "preccurlyeq",    "preccurlyeq",    "&#",     nullptr},
    { MS_PRECEDESEQUIV,     TPRECEDESEQUIV,     TG::Relation,     "precsim",        "precsim",        "&#",     nullptr},
    { MS_SUCCEEDS,          TSUCCEEDS,          TG::Relation,     "succ",           "succ",           "&#",     nullptr},
    { MS_SUCCEEDSEQUAL,     TSUCCEEDSEQUAL,     TG::Relation,     "succcurlyeq",    "succcurlyeq",    "&#",     nullptr},
    { MS_SUCCEEDSEQUIV,     TSUCCEEDSEQUIV,     TG::Relation,     "succsim",        "succsim",        "&#",     nullptr},
    { MS_NOTPRECEDES,       TNOTPRECEDES,       TG::Relation,     "nprec",          "nprec",          "&#",     nullptr},
    { MS_NOTSUCCEEDS,       TNOTSUCCEEDS,       TG::Relation,     "nsucc",          "nsucc",          "&#",     nullptr}
};

const MathType2 aLogicTypeTable [] =
{
    { MS_AND,           TAND,           TG::Product,        "and",           "and",          "&#",     nullptr},
    { MS_OR,            TOR,            TG::Sum,            "or",            "or",           "&#",     nullptr},
    { MS_NEG,           TNEG,           TG::UnOper,         "neg",           "neg",          "&#",     nullptr},
    { MS_NEQ,           TNEQ,           TG::Relation,       "neq",           "neq",          "&#",     nullptr},
    { MS_KNOW,          TKNOW,          TG::Standalone,     "know",          "known",        "&#",     nullptr},
    { MS_ADJOINT,       TADJOINT,       TG::Standalone,     "adjoint",       "adjoint",      "&#",     nullptr},
    { MS_TOPTEE,        TTOPTEE,        TG::Standalone,     "topelment",     "topelment",    "&#",     nullptr},
    { MS_ASSERTION,     TASSERTION,     TG::Standalone,     "assertion",     "assertion",    "&#",     nullptr},
    { MS_NOTPROOVE,     TNOTPROOVE,     TG::Standalone,     "notproove",     "notproove",    "&#",     nullptr},
    { MS_MODELS,        TMODELS,        TG::Standalone,     "models",        "models",       "&#",     nullptr},
    { MS_TRUE,          TTRUE,          TG::Standalone,     "true",          "true",         "&#",     nullptr},
    { MS_NOTTRUE,       TNOTTRUE,       TG::Standalone,     "flase",         "false",        "&#",     nullptr},
    { MS_FORCES,        TFORCES,        TG::Standalone,     "forces",        "forces",       "&#",     nullptr},
    { MS_NOTFORCES,     TNOTFORCES,     TG::Standalone,     "notforces",     "notforces",    "&#",     nullptr},
    { MS_THEREFORE,     TTHEREFORE,     TG::Standalone,     "therefore",     "therefore",    "&#",     nullptr},
    { MS_RATIO,         TRATIO,         TG::Standalone,     "ratio",         "ratio",        "&#",     nullptr},
    { MS_PROPORTION,    TPROPORTION,    TG::Standalone,     "proportion",    "proportion",   "&#",     nullptr}
};

const MathType2 aTensorsTypeTable [] =
{
    { MS_VECTORPROD,      TGVECTORPROD,    TG::Product,       "vecprod",      "vecprod",     "&#",     nullptr},
    { MS_OPLUS,           TOPLUS,          TG::Sum,           "oplus",        "oplus",       "&#",     nullptr},
    { MS_OMINUS,          TOMINUS,         TG::Sum,           "ominus",       "ominus",      "&#",     nullptr},
    { MS_OTIMES,          TOTIMES,         TG::NONE,          "otimes",       "otimes",      "&#",     nullptr},
    { MS_ODIVIDE,         TODIVIDE,        TG::Product,       "odivide",      "odivide",     "&#",     nullptr},
    { MS_ODOT,            TODOT,           TG::Product,       "odot",         "odot",        "&#",     nullptr},
    { MS_OCOMPOSE,        TOCOMPOSE,       TG::Sum,           "ocomp",        "ocomp",       "&#",     nullptr},
    { MS_OASTERISK,       TOMULTIPLY,      TG::Product,       "oastk",        "oastk",       "&#",     nullptr},
    { MS_OEQUALS,         TOEQUAL,         TG::Relation,      "oequals",      "oequals",     "&#",     nullptr},
    { MS_ODASH,           TODASH,          TG::Product,       "oproduct",     "oproduct",    "&#",     nullptr},
    { MS_HERMITANCONJ,    THERMITANCONJ,   TG::Standalone,    "hermitic",     "hermitic",    "&#",     nullptr},
    { MS_MONUS,           TMONUS,          TG::Sum,           "monus",        "monus",       "&#",     nullptr},
    { MS_HOMOTHETY,       THOMOTHETY,      TG::Standalone,    "homothety",    "homothety",   "&#",     nullptr},
    { MS_SQPLUS,          TSQPLUS,         TG::Sum,           "sqplus",       "sqplus",      "&#",     nullptr},
    { MS_SQMINUS,         TSQMINUS,        TG::Sum,           "sqminus",      "sqminus",     "&#",     nullptr},
    { MS_SQTIMES,         TSQTIMES,        TG::Product,       "sqtimes",      "sqtimes",     "&#",     nullptr},
    { MS_SQDOT,           TSQDOT,          TG::Product,       "sqdot",        "sqdot",       "&#",     nullptr}
};

const MathType2 aGeometryTypeTable [] =
{
    { MS_SINEWAVE,             TWAVE,                  TG::Standalone,    "char wave",           "wave",           "&#",     nullptr},
    { MS_SPHERICALTRIGO,       TSPHTRIGO,              TG::Standalone,    "char sphtrigo",       "sphtrigo",       "&#",     nullptr},
    { MS_PROJECTIVE,           TPROJECTIVE,            TG::Standalone,    "char projective",     "projective",     "&#",     nullptr},
    { MS_PERSPECTIVE,          TPERSPECTIVE,           TG::Standalone,    "char perspective",    "perspective",    "&#",     nullptr},
    { MS_TRANSVERSAL,          TTRANSVERSAL,           TG::Standalone,    "char transversal",    "transversal",    "&#",     nullptr},
    { MS_ORTHO,                TORTHO,                 TG::Relation,      "ortho",               "ortho",          "&#",     nullptr},
    { MS_PARALLEL,             TPARALLEL,              TG::Relation,      "parallel",            "parallel",       "&#",     nullptr},
    { MS_NOTPARALLEL,          TNOTPARALLEL,           TG::Relation,      "notparallel",         "notparallel",    "&#",     nullptr},
    { MS_PERPENDICULAR,        TPERPENDICULAR,         TG::Relation,      "perp",                "perp",           "&#",     nullptr},
    { MS_PERPENDICULARWITHS,   TPERPENDICULARWITHS,    TG::Relation,      "perpwith",            "perpwith",       "&#",     nullptr},
    // Angles
    { MS_ANGLE90,              TANGLE,     TG::Standalone,     "char angle90",             "angle90",             "&#",     nullptr},
    { MS_ANGLE,                TANGLE,     TG::Standalone,     "char angle",               "angle",               "&#",     nullptr},
    { MS_ANGLEUNDERBAR,        TANGLE,     TG::Standalone,     "char angleunderbar",       "angleunderbar",       "&#",     nullptr},
    { MS_ANGLEWITHS,           TANGLE,     TG::Standalone,     "char anglewiths",          "anglewiths",          "&#",     nullptr},
    { MS_ACUTEANGLE,           TANGLE,     TG::Standalone,     "char angleacute",          "angleacute",          "&#",     nullptr},
    { MS_OBLIQUEANGLE,         TANGLE,     TG::Standalone,     "char angleoblique",        "angleoblique",        "&#",     nullptr},
    { MS_OBLIQUEANGLEDOWN,     TANGLE,     TG::Standalone,     "char angleobliqued",       "angleobliqued",       "&#",     nullptr},
    { MS_TURNEDANGLE,          TANGLE,     TG::Standalone,     "char angleturned",         "angleturned",         "&#",     nullptr},
    { MS_RESERVEDANGLE,        TANGLE,     TG::Standalone,     "char anglereservedown",    "anglereservedown",    "&#",     nullptr},
    { MS_RESERVEDANGLEUBAR,    TANGLE,     TG::Standalone,     "char anglereservedubar",   "anglereservedubar",   "&#",     nullptr},
    { MS_MEASUREDANGLE,        TANGLE,     TG::Standalone,     "char anglemeasure",        "anglemeasure",        "&#",     nullptr},
    { MS_LMEASUREDANGLE,       TANGLE,     TG::Standalone,     "char langlemeasure",       "langlemeasure",       "&#",     nullptr},
    { MS_SPHERICALANGLE,       TANGLE,     TG::Standalone,     "char sphanglemeasure",     "sphanglemeasure",     "&#",     nullptr},
    { MS_LSPHERICALANGLE,      TANGLE,     TG::Standalone,     "char sphlangle",           "sphlangle",           "&#",     nullptr},
    { MS_DSPHERICALANGLE,      TANGLE,     TG::Standalone,     "char sphrangle",           "sphrangle",           "&#",     nullptr},
    { MS_RIGHTANGLE,           TANGLE,     TG::Standalone,     "char angleright",          "angleright",          "&#",     nullptr},
    { MS_RIGHTANGLEARC,        TANGLE,     TG::Standalone,     "char anglerightarc",       "anglerightarc",       "&#",     nullptr},
    { MS_RIGHTANGLEDOT,        TANGLE,     TG::Standalone,     "char anglerightdot",       "anglerightdot",       "&#",     nullptr}
};

const MathType2 aShapesTypeTable [] =
{
    { MS_RIGHTTRIANGLE,     TGEOFIG,     TG::Standalone,     "char rtriangle",    "rtriangle",     "&#",     nullptr}
};

const MathType2 aArrowsTypeTable [] =
{
    { MS_DRARROW,        TDRARROW,        TG::Standalone,     "drarrow",      "drarrow",     "&#",     nullptr},
    { MS_DLARROW,        TDLARROW,        TG::Standalone,     "dlarrow",      "dlarrow",     "&#",     nullptr},
    { MS_DLRARROW,       TDLRARROW,       TG::Standalone,     "dlrarrow",     "dlrarrow",    "&#",     nullptr},
    { MS_LEFTARROW,      TLEFTARROW,      TG::Standalone,     "leftarrow",    "leftarrow",   "&#",     nullptr},
    { MS_UPARROW,        TUPARROW,        TG::Standalone,     "uparrow",      "uparrow",     "&#",     nullptr},
    { MS_RIGHTARROW,     TRIGHTARROW,     TG::Standalone,     "rightarrow",   "rightarrow",  "&#",     nullptr},
    { MS_DOWNARROW,      TDOWNARROW,      TG::Standalone,     "downarrow",    "downarrow",   "&#",     nullptr}
};

const MathType2 aMiscellaneousTypeTable [] =
{
    { MS_FACT,               TFACT,              TG::UnOper,        "fact",             "fact",         "&#",     nullptr},
    { MS_PERCENT,            TPERCENT,           TG::Standalone,    "char per100",      "per100",       "&#",     nullptr},
    { MS_PERTHOUSAND,        TPERTHOUSAND,       TG::Standalone,    "char per1000",     "per1000",      "&#",     nullptr},
    { MS_PERTENTHOUSAND,     TPERTENTHOUSAND,    TG::Standalone,    "char per10000",    "per10000",     "&#",     nullptr},
    { MS_ASSIGN,             TASSIGN,            TG::Relation,      ":=",               "assign",       "&#",     nullptr},
    { MS_EQUAL,              TEQUAL,   TG::UnOper | TG::Relation,   "=",                "equal",        "&#",     nullptr},
    { MS_ERROR,              TERROR,             TG::NONE,          "char error",       "error",        "&#",     nullptr},
    { MS_BACKSLASH,          TBACKSLASH,         TG::Standalone,    "backslash",        "backslash",    "&#",     nullptr},
    { MS_SQRT,               TSQRT,              TG::Function,      "sqrt",             "sqrt",         "&#",     nullptr},
    { MS_PLACE,              TPLACE,             TG::NONE,          "place",            "place",        "&#",     nullptr}
};

const MathType2 aEasterEggsTypeTable [] =
{
    { MS_PILCROW             ,     TEASTEREGG,     TG::Standalone,     "char pilcrow",       "pilcrow",       "&#",     nullptr},
    { MS_TM                  ,     TEASTEREGG,     TG::Standalone,     "char tm",            "tm",            "&#",     nullptr},
    { MS_COPYRIGHT           ,     TEASTEREGG,     TG::Standalone,     "char copyright",     "copyright",     "&#",     nullptr},
    { MS_REGISTERED          ,     TEASTEREGG,     TG::Standalone,     "char registered",    "registered",    "&#",     nullptr},
    { MS_DEATH               ,     TEASTEREGG,     TG::Standalone,     "char death",         "death",         "&#",     nullptr},
    { MS_SECTION             ,     TEASTEREGG,     TG::Standalone,     "char section",       "section",       "&#",     nullptr},
    { MS_SERVICE             ,     TEASTEREGG,     TG::Standalone,     "char service",       "service",       "&#",     nullptr}
};

// Do we really need this one?
// For now gonna left unimplemented and implement in case of need

const MathType2 aAtributesTypeTable [] =
{
    { MS_TILDE,    TTEXT,    TG::NONE,     "\"~\"",    "tilde",     "&#",     nullptr}
};

/*
const MathType2 aAtributesTypeTable [] =
{
    { MS_ACUTE,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_BAR,              TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_BREVE,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_CHECK,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_CIRCLE,           TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_VEC,              TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOT,              TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DDOT,             TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DDDOT,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DDDDOT,           TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_GRAVE,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_HARPOON,          TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_HAT,              TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_TILDE,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_LINE,             TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_VERTLINE,         TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DLINE,            TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DVERTLINE,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOTSLOW,          TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOTSAXIS,         TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOTSVERT,         TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOTSUP,           TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_DOTSDOWN,         TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_TRANSR,           TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_TRANSL,           TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBGRAVE,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBACUTE,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBHAT,          TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBTILDE,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBBAR,          TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBOVERLINE,     TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBBREVE,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBCIRCLE,       TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBCHECK,        TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBDOT,          TNONE,     TG::NONE,     "",    "",     "&#",     nullptr},
    { MS_COMBDDOT,         TNONE,     TG::NONE,     "",    "",     "&#",     nullptr}
};*/

const MathType2 aBracketsTypeTable [] =
{
    { MS_LPARENT,        TLPARENT,        TG::LBrace,     "(",             "(",              "&#",     "\\("},
    { MS_RPARENT,        TRPARENT,        TG::RBrace,     ")",             ")",              "&#",     "\\)"},
    { MS_LSQBRACKET,     TLBRACE,         TG::LBrace,     "[",             "lsqbracket",     "&#",     "\\["},
    { MS_RSQBRACKET,     TRBRACE,         TG::RBrace,     "]",             "rrqbracket",     "&#",     "\\}"},
    { MS_LBRACKET,       TLBRACKET,       TG::LBrace,     "lbracket",      "lbracket",       "&#",     nullptr},
    { MS_RBRACKET,       TRBRACKET,       TG::RBrace,     "rbracket",      "rbracket",       "&#",     nullptr},
    { MS_LBRACE,         TLBRACE,         TG::LBrace,     "lbrace",        "lbrace",         "&#",     nullptr},
    { MS_RBRACE,         TRBRACE,         TG::RBrace,     "rbrace",        "rbrace",         "&#",     nullptr},
    { MS_LCEIL,          TLCEIL,          TG::LBrace,     "lceil",         "lceil",          "&#",     nullptr},
    { MS_RCEIL,          TRCEIL,          TG::RBrace,     "rceil",         "rceil",          "&#",     nullptr},
    { MS_LFLOOR,         TLFLOOR,         TG::LBrace,     "lfloor",        "lfloor",         "&#",     nullptr},
    { MS_RFLOOR,         TRFLOOR,         TG::RBrace,     "rfloor",        "rfloor",         "&#",     nullptr},
    { MS_LANGLE,         TLANGLE,         TG::LBrace,     "langle",        "langle",         "&#",     nullptr},
    { MS_RDANGLE,        TRDANGLE,        TG::RBrace,     "rdangle",       "drangle",        "&#",     nullptr},
    { MS_LDANGLE,        TLDANGLE,        TG::LBrace,     "ldangle",       "dlangle",        "&#",     nullptr},
    { MS_RANGLE,         TRANGLE,         TG::RBrace,     "rangle",        "rangle",         "&#",     nullptr},
    { MS_LDBRACKET,      TLDBRACKET,      TG::LBrace,     "ldbracket",     "ldbracket",      "&#",     nullptr},
    { MS_RDBRACKET,      TRDBRACKET,      TG::RBrace,     "rdbracket",     "rdbracket",      "&#",     nullptr},
    { MS_OVERBRACE,      TOVERBRACE,      TG::Product,    "overbrace",     "overbrace",      "&#",     nullptr},
    { MS_UNDERBRACE,     TUNDERBRACE,     TG::Product,    "underbrace",    "underbrace",     "&#",     nullptr}
};

static const MathType2* AlphabetsTypeTable [] =
{
    const_cast<const MathType2*>(aGreekMathTypeTable),
    const_cast<const MathType2*>(aGreekBoldMathTypeTable),
    const_cast<const MathType2*>(aGreekItalMathTypeTable),
    const_cast<const MathType2*>(aGreekBoldItalMathTypeTable),
    const_cast<const MathType2*>(aGreekDoubleStrokeMathTypeTable),
    const_cast<const MathType2*>(aDigitsMathTypeTable),
    const_cast<const MathType2*>(aDigitsBoldMathTypeTable),
    const_cast<const MathType2*>(aDigitsDoubleStrokeMathTypeTable),
    const_cast<const MathType2*>(aDigitsCountingRodMathTypeTable),
    const_cast<const MathType2*>(aDigitsRomanMathTypeTable),
    const_cast<const MathType2*>(aDigitsPhoenicianMathTypeTable),
    const_cast<const MathType2*>(aLatinMathTypeTable),
    const_cast<const MathType2*>(aLatinBoldMathTypeTable),
    const_cast<const MathType2*>(aLatinItalicMathTypeTable),
    const_cast<const MathType2*>(aLatinBoldItalicMathTypeTable),
    const_cast<const MathType2*>(aLatinDoubleStrokeMathTypeTable),
    const_cast<const MathType2*>(aLatinFrakturMathTypeTable),
    const_cast<const MathType2*>(aLatinFrakturBoldMathTypeTable),
    const_cast<const MathType2*>(aLatinScriptMathTypeTable),
    const_cast<const MathType2*>(aLatinScriptBoldMathTypeTable),
    const_cast<const MathType2*>(aHebrewTypeTable),
    const_cast<const MathType2*>(aPhoenicianTypeTable),
    const_cast<const MathType2*>(aGothicTypeTable),
    const_cast<const MathType2*>(aInternationalMathTypeTable),
    const_cast<const MathType2*>(aCurrencyMathTypeTable),
    const_cast<const MathType2*>(aConstantsAndOthersTypeTable),
    const_cast<const MathType2*>(aSimpleOperTypeTable),
    const_cast<const MathType2*>(aOpperatorsTypeTable),
    const_cast<const MathType2*>(aMathSetsTypeTable),
    const_cast<const MathType2*>(aCompareTypeTable),
    const_cast<const MathType2*>(aOrderTypeTable),
    const_cast<const MathType2*>(aLogicTypeTable),
    const_cast<const MathType2*>(aTensorsTypeTable),
    const_cast<const MathType2*>(aGeometryTypeTable),
    const_cast<const MathType2*>(aShapesTypeTable),
    const_cast<const MathType2*>(aArrowsTypeTable),
    const_cast<const MathType2*>(aMiscellaneousTypeTable),
    const_cast<const MathType2*>(aEasterEggsTypeTable),
    const_cast<const MathType2*>(aAtributesTypeTable),
    const_cast<const MathType2*>(aBracketsTypeTable)
};

const sal_Int16 AlphabetsSizesMathType [] =
{
    std::end(aGreekMathTypeTable)              - std::begin(aGreekMathTypeTable),
    std::end(aGreekBoldMathTypeTable)          - std::begin(aGreekBoldMathTypeTable),
    std::end(aGreekItalMathTypeTable)          - std::begin(aGreekItalMathTypeTable),
    std::end(aGreekBoldItalMathTypeTable)      - std::begin(aGreekBoldItalMathTypeTable),
    std::end(aGreekDoubleStrokeMathTypeTable)  - std::begin(aGreekDoubleStrokeMathTypeTable),
    std::end(aDigitsMathTypeTable)             - std::begin(aDigitsMathTypeTable),
    std::end(aDigitsBoldMathTypeTable)         - std::begin(aDigitsBoldMathTypeTable),
    std::end(aDigitsDoubleStrokeMathTypeTable) - std::begin(aDigitsDoubleStrokeMathTypeTable),
    std::end(aDigitsCountingRodMathTypeTable)  - std::begin(aDigitsCountingRodMathTypeTable),
    std::end(aDigitsRomanMathTypeTable)        - std::begin(aDigitsRomanMathTypeTable),
    std::end(aDigitsPhoenicianMathTypeTable)   - std::begin(aDigitsPhoenicianMathTypeTable),
    std::end(aLatinMathTypeTable)              - std::begin(aLatinMathTypeTable),
    std::end(aLatinBoldMathTypeTable)          - std::begin(aLatinBoldMathTypeTable),
    std::end(aLatinItalicMathTypeTable)        - std::begin(aLatinItalicMathTypeTable),
    std::end(aLatinBoldItalicMathTypeTable)    - std::begin(aLatinBoldItalicMathTypeTable),
    std::end(aLatinDoubleStrokeMathTypeTable)  - std::begin(aLatinDoubleStrokeMathTypeTable),
    std::end(aLatinFrakturMathTypeTable)       - std::begin(aLatinFrakturMathTypeTable),
    std::end(aLatinFrakturBoldMathTypeTable)   - std::begin(aLatinFrakturBoldMathTypeTable),
    std::end(aLatinScriptMathTypeTable)        - std::begin(aLatinScriptMathTypeTable),
    std::end(aLatinScriptBoldMathTypeTable)    - std::begin(aLatinScriptBoldMathTypeTable),
    std::end(aHebrewTypeTable)                 - std::begin(aHebrewTypeTable),
    std::end(aPhoenicianTypeTable)             - std::begin(aPhoenicianTypeTable),
    std::end(aGothicTypeTable)                 - std::begin(aGothicTypeTable),
    std::end(aInternationalMathTypeTable)      - std::begin(aInternationalMathTypeTable),
    std::end(aCurrencyMathTypeTable)           - std::begin(aCurrencyMathTypeTable),
    std::end(aConstantsAndOthersTypeTable)     - std::begin(aConstantsAndOthersTypeTable),
    std::end(aSimpleOperTypeTable)             - std::begin(aSimpleOperTypeTable),
    std::end(aOpperatorsTypeTable)             - std::begin(aOpperatorsTypeTable),
    std::end(aMathSetsTypeTable)               - std::begin(aMathSetsTypeTable),
    std::end(aCompareTypeTable)                - std::begin(aCompareTypeTable),
    std::end(aOrderTypeTable)                  - std::begin(aOrderTypeTable),
    std::end(aLogicTypeTable)                  - std::begin(aLogicTypeTable),
    std::end(aTensorsTypeTable)                - std::begin(aTensorsTypeTable),
    std::end(aGeometryTypeTable)               - std::begin(aGeometryTypeTable),
    std::end(aShapesTypeTable)                 - std::begin(aShapesTypeTable),
    std::end(aArrowsTypeTable)                 - std::begin(aArrowsTypeTable),
    std::end(aMiscellaneousTypeTable)          - std::begin(aMiscellaneousTypeTable),
    std::end(aEasterEggsTypeTable)             - std::begin(aEasterEggsTypeTable),
    std::end(aAtributesTypeTable)              - std::begin(aAtributesTypeTable),
    std::end(aBracketsTypeTable)               - std::begin(aBracketsTypeTable)
};

static char mathtype2CompareStrings( const char * c1, const char * c2 )
{
    sal_uInt16 l1, l2;
    for( l1 = 0; c1[l1] != '\0'; ++l1 );
    for( l2 = 0; c2[l2] != '\0'; ++l2 );
    if ( l1 < l2 )
    {
        for ( l2 = 0; l2 < l1; ++l2 )
        {
            if( c1[l2] < c2[l2] ) return -1;
            else if( c1[l2] > c2[l2] ) return 1;
        }
        return -1;
    }
    else if ( l1 > l2 )
    {
        for ( l1 = 0; l1 < l2; ++l1 )
        {
            if( c1[l1] < c2[l1] ) return -1;
            else if( c1[l1] > c2[l1] ) return 1;
        }
        return 1;
    }
    else
    {
        for ( l1 = 0; l1 < l2; ++l1 )
        {
            if( c1[l1] < c2[l1] ) return -1;
            else if( c1[l1] > c2[l1] ) return 1;
        }
        return 0;
    }
}

static char mathtype2CompareOUStrings( const OUString &c1, const char* c2 )
{
    sal_uInt16 l1, l2;
    l1 = c1.getLength();
    for( l2 = 0; c2[l2] != '\0'; ++l2 );
    if ( l1 < l2 )
    {
        for ( l2 = 0; l2 < l1; ++l2 )
        {
            if( static_cast<char>(c1[l2]) < c2[l2] ) return -1;
            else if( static_cast<char>(c1[l2]) > c2[l2] ) return 1;
        }
        return -1;
    }
    else if ( l1 > l2 )
    {
        for ( l1 = 0; l1 < l2; ++l1 )
        {
            if( static_cast<char>(c1[l1]) < c2[l1] ) return -1;
            else if( static_cast<char>(c1[l1]) > c2[l1] ) return 1;
        }
        return 1;
    }
    else
    {
        for ( l1 = 0; l1 < l2; ++l1 )
        {
            if( static_cast<char>(c1[l1]) < c2[l1] ) return -1;
            else if( static_cast<char>(c1[l1]) > c2[l1] ) return 1;
        }
        return 0;
    }
}

static sal_Int32 mathtype2PartitionHoareChars
       ( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    const MathType2 * pivot = data[(hi + lo) / 2];
    const MathType2 * swap;
    sal_Int32 i = lo - 1;
    sal_Int32 j = hi + 1;
    while(true)
    {
        do { ++i; }
        while ( data[i]->nSymbol < pivot->nSymbol );
        do { --j; }
        while ( data[j]->nSymbol > pivot->nSymbol );
        if ( i >= j ) return j;
        swap = data[i];
        data[i] = data[j];
        data[j] = swap;
    }
}

static sal_Int32 mathtype2PartitionHoareCommands
       ( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    const MathType2 * pivot = data[(hi + lo) / 2];
    const MathType2 * swap;
    sal_Int32 i = lo - 1;
    sal_Int32 j = hi + 1;
    while(true)
    {
        do { ++i; }
        while ( mathtype2CompareStrings( data[i]->pCommand, pivot->pCommand ) < 0 );
        do { --j; }
        while ( mathtype2CompareStrings( data[j]->pCommand, pivot->pCommand ) > 0 );
        if ( i >= j ) return j;
        swap = data[i];
        data[i] = data[j];
        data[j] = swap;
    }
}

static sal_Int32 mathtype2PartitionHoareCharNames
       ( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    const MathType2 * pivot = data[(hi + lo) / 2];
    const MathType2 * swap;
    sal_Int32 i = lo - 1;
    sal_Int32 j = hi + 1;
    while(true)
    {
        do { ++i; }
        while ( mathtype2CompareStrings( data[i]->pName, pivot->pName ) < 0 );
        do { --j; }
        while ( mathtype2CompareStrings( data[j]->pName, pivot->pName ) > 0 );
        if ( i >= j ) return j;
        swap = data[i];
        data[i] = data[j];
        data[j] = swap;
    }
}

static sal_Int32 mathtype2PartitionHoareMathml
       ( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    const MathType2 * pivot = data[(hi + lo) / 2];
    const MathType2 * swap;
    sal_Int32 i = lo - 1;
    sal_Int32 j = hi + 1;
    while(true)
    {
        do { ++i; }
        while ( mathtype2CompareStrings( data[i]->pMlName, pivot->pMlName ) < 0 );
        do { --j; }
        while ( mathtype2CompareStrings( data[j]->pMlName, pivot->pMlName ) > 0 );
        if ( i >= j ) return j;
        swap = data[i];
        data[i] = data[j];
        data[j] = swap;
    }
}

/*
static sal_Int32 mathtype2PartitionHoareLatex
       ( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    const MathType2 * pivot = data[(hi + lo) / 2];
    const MathType2 * swap;
    sal_Int32 i = lo - 1;
    sal_Int32 j = hi + 1;
    while(true)
    {
        do { ++i; }
        while ( mathtype2CompareStrings( data[i]->plCommand, pivot->plCommand ) < 0 );
        do { --j; }
        while ( mathtype2CompareStrings( data[j]->plCommand, pivot->plCommand ) > 0 );
        if ( i >= j ) return j;
        swap = data[i];
        data[i] = data[j];
        data[j] = swap;
    }
}*/

static void mathtype2QuicksortChars( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    if ( lo < hi )
    {
        sal_Int32 p = mathtype2PartitionHoareChars( data, lo, hi );
        mathtype2QuicksortChars( data, lo, p );
        mathtype2QuicksortChars( data, p + 1, hi );
    }
}

static void mathtype2QuicksortCommands( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    if ( lo < hi )
    {
        sal_Int32 p = mathtype2PartitionHoareCommands( data, lo, hi );
        mathtype2QuicksortCommands( data, lo, p );
        mathtype2QuicksortCommands( data, p + 1, hi );
    }
}

static void mathtype2QuicksortCharNames( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    if ( lo < hi )
    {
        sal_Int32 p = mathtype2PartitionHoareCharNames( data, lo, hi );
        mathtype2QuicksortCharNames( data, lo, p );
        mathtype2QuicksortCharNames( data, p + 1, hi );
    }
}

static void mathtype2QuicksortMathml( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    if ( lo < hi )
    {
        sal_Int32 p = mathtype2PartitionHoareMathml( data, lo, hi );
        mathtype2QuicksortMathml( data, lo, p );
        mathtype2QuicksortMathml( data, p + 1, hi );
    }
}

/*
static void mathtype2QuicksortLatex( MathType2 const ** data, sal_Int32 lo, sal_Int32 hi )
{
    if ( lo < hi )
    {
        sal_Int32 p = mathtype2PartitionHoareLatex( data, lo, hi );
        mathtype2QuicksortLatex( data, lo, p - 1 );
        mathtype2QuicksortLatex( data, p + 1, hi );
    }
}*/

static sal_Int32 ltot = 0;
static sal_uInt16 ncount = 0;
static MathType2 const ** TypesSortedChars = nullptr;
static MathType2 const ** TypesSortedCommands = nullptr;
static MathType2 const ** TypesSortedCharNames = nullptr;
static MathType2 const ** TypesSortedMathml = nullptr;
//static MathType2 const ** TypesSortedLatex = nullptr;
static MathType2* TypesSortedCharNone = new MathType2( MS_PLACE, TCHAR, TG::Character, "char error", "error", "&#", nullptr );

void MathType2Namespace::initMathtype2()
{
    // This has been conceived to be the easyest possible.
    // It will result easy to add new chars without breaking it.
    // And keeps it almost automatic.
    // We are using memory in order to reduce processing costs afterwards.
    // Priority: execution speed, memory, code complexity
    // This shouuld need some milliseconds to load.

    sal_Int32 i, j, k = 0;
    // First step, count wathever we have
    ncount = std::end(AlphabetsTypeTable) - std::begin(AlphabetsTypeTable);
    for( i = 0; i < ncount; ++i ) ltot += AlphabetsSizesMathType[i];
    // Second step make place for it
    TypesSortedChars = new const MathType2 * [ltot];
    TypesSortedCommands = new const MathType2 * [ltot];
    TypesSortedCharNames = new const MathType2 * [ltot];
    TypesSortedMathml = new const MathType2 * [ltot];
    //TypesSortedLatex = new const MathType2 * [ltot];
    //TypesSortedChars = static_cast<MathType2 const **>(malloc(ltot*sizeof(const MathType2*)));
    //TypesSortedCommands = static_cast<MathType2 const **>(malloc(ltot*sizeof(const MathType2*)));
    //TypesSortedCharNames = static_cast<MathType2 const **>(malloc(ltot*sizeof(const MathType2*)));
    //TypesSortedMathml = static_cast<MathType2 const **>(malloc(ltot*sizeof(const MathType2*)));
    //TypesSortedLatex = static_cast<MathType2 const **>(malloc(ltot*sizeof(const MathType2*)));

    // Copy data
    for( i = 0; i < ncount; ++i )
    {
        for( j = 0; j < AlphabetsSizesMathType[i]; ++j )
        {
            TypesSortedChars[k] = &AlphabetsTypeTable[i][j];
            TypesSortedCommands[k] = &AlphabetsTypeTable[i][j];
            TypesSortedCharNames[k] = &AlphabetsTypeTable[i][j];
            TypesSortedMathml[k] = &AlphabetsTypeTable[i][j];
            //TypesSortedLatex[k] = &AlphabetsTypeTable[i][j];
            ++k;
        }
    }
    // Sort data
    mathtype2QuicksortChars( TypesSortedChars, 0, ltot - 1);
    mathtype2QuicksortCommands( TypesSortedCommands, 0, ltot - 1);
    mathtype2QuicksortCharNames( TypesSortedCharNames, 0, ltot - 1);
    mathtype2QuicksortMathml( TypesSortedMathml, 0, ltot - 1);
    //mathtype2QuicksortLatex( TypesSortedMathml, 0, ltot - 1, compareTypesMathml);

}

void MathType2Namespace::endMathtype2()
{
    // We give back what we've taken.
    // Note that pointer content has not to be freed.
    delete[] TypesSortedChars;
    delete[] TypesSortedCommands;
    delete[] TypesSortedCharNames;
    delete[] TypesSortedMathml;
    //delete[] TypesSortedLatex;
}

OUString MathType2Namespace::identifyCharCommand(sal_Unicode32 cChar)
{
    if(!ltot) initMathtype2();
    if( cChar == MS_NONE ) return OUString::createFromAscii("none");
    sal_Int32 i;
    for( i = 0; i < ltot; i++ ) if( cChar >= TypesSortedChars[i]->nSymbol ) break;
    if ( i < ltot )
    {
        if( cChar == TypesSortedChars[i]->nSymbol )
            return OUString::createFromAscii(TypesSortedChars[i]->pCommand);
        else return OUString::unicode32(cChar);
    }
    else return OUString::unicode32(cChar);
}

OUString MathType2Namespace::identifyCharName(sal_Unicode32 cChar)
{
    if(!ltot) initMathtype2();
    if( cChar == MS_NONE ) return OUString::createFromAscii("none");
    sal_Int32 i;
    for( i = 0; i < ltot; i++ ) if( cChar >= TypesSortedChars[i]->nSymbol ) break;
    if ( i < ltot )
    {
        if( cChar == TypesSortedChars[i]->nSymbol )
            return OUString::createFromAscii(TypesSortedChars[i]->pName);
        else return OUString::unicode32(cChar);
    }
    else return OUString::unicode32(cChar);
}

OUString MathType2Namespace::identifyCharMlname(sal_Unicode32 cChar)
{
    if(!ltot) initMathtype2();
    if( cChar == MS_NONE ) return OUString::createFromAscii("none");
    sal_Int32 i;
    for( i = 0; i < ltot; i++ ) if( cChar >= TypesSortedChars[i]->nSymbol ) break;
    if ( i < ltot )
    {
        if( cChar == TypesSortedChars[i]->nSymbol )
        {
            OUString res = OUString::createFromAscii(TypesSortedChars[i]->pMlName);
            if( res.compareToAscii("&#") == 0 )
                res = res.concat( OUString::number(static_cast<sal_uInt32>(cChar)) ).concat(";");
            return res;
        }
        else return OUString::unicode32(cChar);
    }
    else return OUString::unicode32(cChar);
}

sal_Unicode32 MathType2Namespace::identifyCharFromCommand(const OUString& cmd)
{
    if(!ltot) initMathtype2();
    if( cmd.getLength() == 0 ) return MS_NONE;
    sal_Int32 i;
    for( i = 0; i < ltot; ++i )
        if( mathtype2CompareOUStrings( cmd, TypesSortedCommands[i]->pCommand ) <= 0 ) break;
    if ( i < ltot )
    {
        if( cmd.compareToAscii(TypesSortedCommands[i]->pCommand) == 0 )
            return TypesSortedCommands[i]->nSymbol;
        else return MS_NONE;
    }
    else return MS_NONE;
}

sal_Unicode32 MathType2Namespace::identifyCharFromCharName(const OUString& charname)
{
    if(!ltot) initMathtype2();
    if( charname.getLength() == 0 ) return MS_NONE;
    sal_Int32 i;
    for( i = 0; i < ltot; ++i )
        if( mathtype2CompareOUStrings( charname, TypesSortedCharNames[i]->pName ) ) break;
    if ( i < ltot )
    {
        if( charname.compareToAscii(TypesSortedCharNames[i]->pName) == 0 )
            return TypesSortedCharNames[i]->nSymbol;
        else return MS_NONE;
    }
    else return MS_NONE;
}

sal_Unicode32 MathType2Namespace::identifyCharFromMathml(const OUString& MlName)
{
    if(!ltot) initMathtype2();
    if( MlName.getLength() == 0 ) return MS_NONE;
    if( MlName.indexOf('#') != -1 )
    {
        assert( MlName.getLength() - 3 > 0);
        OUString str2 = MlName.copy( 2, MlName.getLength() - 3 );
        return str2.toUInt32(16);
    }
    else
    {
        sal_Int32 i;
        for( i = 0; i < ltot; ++i )
            if( mathtype2CompareOUStrings( MlName, TypesSortedMathml[i]->pName ) <= 0 ) break;
        if ( i < ltot )
        {
            if( MlName.compareToAscii( TypesSortedMathml[i]->pMlName ) == 0 )
                return TypesSortedMathml[i]->nSymbol;
            else return MS_NONE;
        }
        else return MS_NONE;
    }
}

const MathType2* MathType2Namespace::identifyAndDataCharFromChar(sal_Unicode32 cChar)
{
    if(!ltot) initMathtype2();
    sal_Int32 i;
    for( i = 0; i < ltot; ++i )
        if( cChar >= TypesSortedChars[i]->nSymbol ) break;
    if( i < ltot )
    {
        if( cChar == TypesSortedChars[i]->nSymbol ) return TypesSortedCommands[i];
        else return TypesSortedCharNone;
        //TODO find a way to return hex without creating a list of 2^32 index.
    }
    else return TypesSortedCharNone;
}

const MathType2* MathType2Namespace::identifyAndDataCharFromCommand(const OUString& cmd)
{
    if(!ltot) initMathtype2();
    if( cmd.getLength() == 0 ) return TypesSortedCharNone;
    sal_Int32 i;
    for( i = 0; i < ltot; ++i )
        if( mathtype2CompareOUStrings( cmd, TypesSortedCommands[i]->pCommand ) <= 0 ) break;
    if( i < ltot )
    {
        if( cmd.compareToAscii(TypesSortedCommands[i]->pCommand) == 0 )
            return TypesSortedCommands[i];
        else return TypesSortedCharNone;
    }
    else return TypesSortedCharNone;
}

const MathType2* MathType2Namespace::identifyAndDataCharFromName(const OUString& name)
{
    if(!ltot) initMathtype2();
    if( name.getLength() == 0 ) return TypesSortedCharNone;
    sal_Int32 i;
    for( i = 0; i < ltot; ++i )
        if( mathtype2CompareOUStrings( name, TypesSortedCharNames[i]->pName ) <= 0 ) break;
    if( i < ltot )
    {
        if( name.compareToAscii(TypesSortedCharNames[i]->pName) == 0 )
            return TypesSortedCharNames[i];
        else return TypesSortedCharNone;
    }
    else return TypesSortedCharNone;
}

const MathType2* MathType2Namespace::identifyAndDataCharFromMathml(const OUString& MlName)
{
    if(!ltot) initMathtype2();
    if( MlName.getLength() == 0 ) return TypesSortedCharNone;
    if( MlName.indexOf('#') != -1 )
    {
       assert( MlName.getLength() - 3 > 0);
       OUString str2 = MlName.copy( 2, MlName.getLength() - 3 );
       return identifyAndDataCharFromChar( str2.toUInt32(16) );
    }
    else
        {
        sal_Int32 i;
        for( i = 0; i < ltot; ++i )
            if( mathtype2CompareOUStrings( MlName, TypesSortedMathml[i]->pMlName ) <= 0 ) break;
        if( i < ltot )
        {
            if( MlName.compareToAscii(TypesSortedMathml[i]->pMlName) == 0 )
                return TypesSortedMathml[i];
            else return TypesSortedCharNone;
        }
        else return TypesSortedCharNone;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
