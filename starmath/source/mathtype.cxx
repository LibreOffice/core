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

#include <filter/msfilter/classids.hxx>
#include <osl/diagnose.h>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>

#include "eqnolefilehdr.hxx"
#include "node.hxx"

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
bool MathType::LookupChar(sal_Unicode nChar,OUString &rRet,sal_uInt8 nVersion,
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
                rRet += OUStringLiteral1( nChar );
                bRet=true;
            }
            break;
        case 0x00fb:
            if ((nVersion < 3) && (nTypeFace == 0x81))
                nChar = 0xDF;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'a':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b1;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'b':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3b2;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'l':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bb;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'n':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3bd;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'r':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x3c1;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 'D':
            if ((nVersion < 3) && (nTypeFace == 0x84))
                nChar = 0x394;
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 0xa9:
            if ((nVersion < 3) && (nTypeFace == 0x82))
                nChar = '\'';
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
        case 0x00f1:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " \\rangle ";
            else
            {
                rRet += OUStringLiteral1( nChar );
                bRet=true;
            }
            break;
        case 0x00a3:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " <= ";
            else
            {
                rRet += OUStringLiteral1( nChar );
                bRet=true;
            }
            break;
        case 0x00de:
            if ((nVersion < 3) && (nTypeFace == 0x86))
                pC = " drarrow ";
            else
            {
                rRet += OUStringLiteral1( nChar );
                bRet=true;
            }
            break;
        case 0x0057:
            if ((nVersion < 3) && (nTypeFace == 0x85))
                pC = " %OMEGA ";
            else
            {
                rRet += OUStringLiteral1( nChar );
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
        case 0x22b2:
        case 0x22b3:
            rRet += " " + OUStringLiteral1( nChar ) + " ";
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
            rRet += "+";
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
            rRet += "`";
            break;
        case 0xeb05:    //large space
            rRet += "~";
            break;
        case 0x3a9:
            pC = " %OMEGA ";
            break;
        default:
            rRet += OUStringLiteral1( nChar );
            bRet=true;
            break;
    }
    if (pC)
        rRet += OUString::createFromAscii( pC );
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
    pS = xSrc.get();
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
    rRet += "{}";

#ifdef CAOLAN
    //sanity check

    //sigh, theres no point! MathType (in some bizarre subvarient) pads
    //the end of the formula with ENDs (0)'s
    auto nEnd = pS->Tell();
    SAL_WARN_IF(nEnd == pS->Seek(STREAM_SEEK_TO_END), "starmath", "Possibly unfully parsed formula");
#endif
    return bRet;
}

static void lcl_PrependDummyTerm(OUString &rRet, sal_Int32 &rTextStart)
{
    if ((rTextStart < rRet.getLength()) &&
        (rRet[rTextStart] == '=') &&
        ((rTextStart == 0) || (rRet[ rTextStart-1 ] == '{'))
       )
    {
        rRet = rRet.replaceAt(rTextStart,0," {}");
        rTextStart+=3;
    }
}

static void lcl_AppendDummyTerm(OUString &rRet)
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
        rRet += " {}";
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
                rRet = rRet.replaceAt(nTextStart,0,aStr);
                rRet += "\"";
            }
            else if (nRecord == END && !rRet.isEmpty())
            {
                sal_Unicode cChar = 0;
                sal_Int32 nI = rRet.getLength()-1;
                while (nI && ((cChar = rRet[nI]) == ' '))
                    --nI;
                if ((cChar == '=') || (cChar == '+') || (cChar == '-'))
                    rRet += "{}";
            }
        }

        switch(nRecord)
        {
            case LINE:
                {
                    if (xfLMOVE(nTag))
                        HandleNudge();

                    if (newline>0)
                        rRet += "\nnewline\n";
                    if (!(xfNULL(nTag)))
                    {
                        switch (nSelector)
                        {
                        case 0x0:
                            if (nVariation==0)
                                rRet += " langle ";
                            else if (nVariation==1)
                                rRet += " \\langle ";
                            break;
                        case 0x1:
                            if (nVariation==0)
                                rRet += " left (";
                            else if (nVariation==1)
                                rRet += "\\(";
                            break;
                        case 0x2:
                            if ((nVariation==0) || (nVariation==1))
                                rRet += " left lbrace ";
                            else
                                rRet += " left none ";
                            break;
                        case 0x3:
                            if (nVariation==0)
                                rRet += " left [";
                            else if (nVariation==1)
                                rRet += "\\[";
                            break;
                        case 0x8:
                        case 0xb:
                            rRet += " \\[";
                            break;
                        case 0x4:
                            if (nVariation==0)
                                rRet += " lline ";
                            else if (nVariation==1)
                                rRet += " \\lline ";
                            break;
                        case 0x5:
                            if (nVariation==0)
                                rRet += " ldline ";
                            else if (nVariation==1)
                                rRet += " \\ldline ";
                            break;
                        case 0x6:
                            if (nVariation == 0 || nVariation == 1)
                                rRet += " left lfloor ";
                            else if (nVariation==1)
                                rRet += " left none ";
                            break;
                        case 0x7:
                            if (nVariation==0)
                                rRet += " lceil ";
                            else if (nVariation==1)
                                rRet += " \\lceil ";
                            break;
                        case 0x9:
                        case 0xa:
                            rRet += " \\]";
                            break;
                        case 0xc:
                            rRet += " \\(";
                            break;
                        case 0xd:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " sqrt";
                                else
                                {
                                    rRet += " nroot";
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            rRet += " {";
                            break;
                        case 0xe:
                            if (nPart == 0)
                                rRet += " { ";


                            if (nPart == 1)
                                rRet += " over ";
                            rRet += " {";
                            break;
                        case 0xf:
                            nSubSupStartPos = rRet.getLength();
                            if ((nVariation == 0) ||
                                    ((nVariation == 2) && (nPart==1)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                rRet += " rSup";
                            }
                            else if ((nVariation == 1) ||
                                    ((nVariation == 2) && (nPart==0)))
                            {
                                lcl_AppendDummyTerm(rRet);
                                rRet += " rSub";
                            }
                            rRet += " {";
                            break;
                        case 0x10:
                            if (nVariation == 0)
                                rRet += " {underline ";
                            else if (nVariation == 1)
                                rRet += " {underline underline ";
                            rRet += " {";
                            break;
                        case 0x11:
                            if (nVariation == 0)
                                rRet += " {overline ";
                            else if (nVariation == 1)
                                rRet += " {overline overline ";
                            rRet += " {";
                            break;
                        case 0x12:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " widevec ";//left arrow above
                                else if (nVariation == 1)
                                    rRet += " widevec ";//left arrow below
                                rRet += " {";
                            }
                            break;
                        case 0x13:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " widevec ";//right arrow above
                                else if (nVariation == 1)
                                    rRet += " widevec ";//right arrow below
                                rRet += " {";
                            }
                            break;
                        case 0x14:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " widevec ";//double arrow above
                                else if (nVariation == 1)
                                    rRet += " widevec ";//double arrow below
                                rRet += " {";
                            }
                            break;
                        case 0x15:
                            if (nPart == 0)
                            {
                                if ((nVariation == 3) || (nVariation == 4))
                                    rRet += " lInt";
                                else
                                    rRet += " Int";
                                if ( (nVariation != 0) && (nVariation != 3))
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 4)) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 2) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 2) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x16:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    rRet += " llInt";
                                else
                                    rRet += " iInt";
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x17:
                            if (nPart == 0)
                            {
                                if ((nVariation == 2) || (nVariation == 3))
                                    rRet += " lllInt";
                                else
                                    rRet += " iiInt";
                                if ( (nVariation != 0) && (nVariation != 2))
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 3)) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x18:
                            if (nPart == 0)
                            {
                                if (nVariation == 2)
                                    rRet += " lInt";
                                else
                                    rRet += " Int";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if (((nVariation == 1) ||
                                    (nVariation == 2)) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 0) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x19:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " llInt";
                                else
                                    rRet += " iInt";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if (nPart==1)
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x1a:
                            if (nPart == 0)
                            {
                                if (nVariation == 0)
                                    rRet += " lllInt";
                                else
                                    rRet += " iiInt";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if (nPart==1)
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x1b:
                        case 0x1c:
                            rRet += " {";
                            break;
                        case 0x1d:
                            if (nPart == 0)
                            {
                                rRet += " Sum";
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x1e:
                            if (nPart == 0)
                            {
                                rRet += " Sum";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x1f:
                            if (nPart == 0)
                            {
                                rRet += " Prod";
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x20:
                            if (nPart == 0)
                            {
                                rRet += " Prod";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x21:
                            if (nPart == 0)
                            {
                                rRet += " coProd";
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x22:
                            if (nPart == 0)
                            {
                                rRet += " coProd";
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x23:
                            if (nPart == 0)
                            {
                                rRet += " union"; //union
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x24:
                            if (nPart == 0)
                            {
                                rRet += " union"; //union
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x25:
                            if (nPart == 0)
                            {
                                rRet += " intersect"; //intersect
                                if (nVariation != 2)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x26:
                            if (nPart == 0)
                            {
                                rRet += " intersect"; //intersect
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " rSub";
                            else if ((nVariation == 1) && (nPart==2))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x27:
                            if ((nVariation == 0) && (nPart==1))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 2) && (nPart==1))
                                rRet += " cSub";
                            else if ((nVariation == 2) && (nPart==2))
                                rRet += " cSup";
                            rRet += " {";
                            break;
                        case 0x28:
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sPush = rRet;
                                    rRet.clear();
                                }
                            }
                            rRet += " {";
                            if (nVariation == 0)
                            {
                                if (nPart == 1)
                                    rRet += "alignr ";
                            }
                            if (nPart == 0)
                                rRet += "\\lline ";
                            if (nVariation == 1)
                                rRet += "overline ";
                            break;
                        case 0x29:
                            rRet += " {";
                            break;
                        case 0x2a:
                            if (nPart == 0)
                            {
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                rRet += " rSup";
                            else if ((nVariation == 2) && (nPart==1))
                                rRet += " rSup";
                            else if ((nVariation == 1) && (nPart==0))
                                rRet += " rSub";
                            else if ((nVariation == 2) && (nPart==0))
                                rRet += " rSub";
                            rRet += " {";
                            break;
                        case 0x2b:
                            if (nPart == 0)
                            {
                                sPush = rRet;
                                rRet.clear();
                            }
                            if ((nVariation == 0) && (nPart==0))
                                rRet += " cSup";
                            else if ((nVariation == 2) && (nPart==1))
                                rRet += " cSup";
                            else if ((nVariation == 1) && (nPart==0))
                                rRet += " cSub";
                            else if ((nVariation == 2) && (nPart==0))
                                rRet += " cSub";
                            rRet += " {";
                            break;
                        case 0x2c:
                            if (nPart == 0)
                                rRet += "\"\"";
                            if ((nVariation == 0)
                                    || ((nVariation == 2) && (nPart==1)))
                                rRet += " lSup";
                            else if ((nVariation == 1)
                                    || ((nVariation == 2) && (nPart==0)))
                                rRet += " lSub";
                            rRet += " {";
                            break;
                        case 0x2d:
                            if (nVariation==0)
                            {
                                if (nPart == 0)
                                    rRet += " langle ";
                            }
                            else if (nVariation==1)
                            {
                                rRet += " \\langle ";
                                newline--;
                            }
                            else if (nVariation==2)
                            {
                                rRet += " \\lline ";
                                newline--;
                            }
                            break;
                        case 0x2e:
                            if (nVariation == 0)
                                rRet += " widevec ";//left below
                            else if (nVariation == 1)
                                rRet += " widevec ";//right below
                            else if (nVariation == 2)
                                rRet += " widevec ";//double headed below
                            rRet += " {";
                            break;
                        case 0x2f:
                            if (nVariation == 0)
                                rRet += " widevec ";//left above
                            else if (nVariation == 1)
                                rRet += " widevec ";//right above
                            else if (nVariation == 2)
                                rRet += " widevec ";//double headed above
                            rRet += " {";
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
                                rRet += "} ";
                            else
                                rRet = rRet.replaceAt( nSizeStartPos, rRet.getLength(), "" );
                            nSetSize--;
                            nCurSize=nOldCurSize;
                        }


                        HandleMatrixSeparator(nMatrixRows,nMatrixCols,
                            nCurCol,nCurRow);

                        switch (nSelector)
                        {
                        case 0x0:
                            if (nVariation==0)
                                rRet += " rangle ";
                            else if (nVariation==2)
                                rRet += " \\rangle ";
                            break;
                        case 0x1:
                            if (nVariation==0)
                                rRet += " right )";
                            else if (nVariation==2)
                                rRet += "\\)";
                            break;
                        case 0x2:
                            if ((nVariation==0) || (nVariation==2))
                                rRet += " right rbrace ";
                            else
                                rRet += " right none ";
                            break;
                        case 0x3:
                            if (nVariation==0)
                                rRet += " right ]";
                            else if (nVariation==2)
                                rRet += "\\]";
                            break;
                        case 0x4:
                            if (nVariation==0)
                                rRet += " rline ";
                            else if (nVariation==2)
                                rRet += " \\rline ";
                            break;
                        case 0x5:
                            if (nVariation==0)
                                rRet += " rdline ";
                            else if (nVariation==2)
                                rRet += " \\rdline ";
                            break;
                        case 0x6:
                            if (nVariation == 0 || nVariation == 2)
                                rRet += " right rfloor ";
                            else if (nVariation==2)
                                rRet += " right none ";
                            break;
                        case 0x7:
                            if (nVariation==0)
                                rRet += " rceil ";
                            else if (nVariation==2)
                                rRet += " \\rceil ";
                            break;
                        case 0x8:
                        case 0xa:
                            rRet += "\\[";
                            break;
                        case 0x9:
                        case 0xc:
                            rRet += "\\]";
                            break;
                        case 0xd:
                            rRet += "} ";
                            if (nVariation == 1)
                            {
                                if (nPart == 0)
                                {
                                    newline--;
                                    sMainTerm = rRet;
                                    rRet.clear();
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
                            rRet += "\\)";
                            break;
                        case 0xe:
                            rRet += "} ";
                            if (nPart == 0)
                                newline--;
                            else
                                rRet += "} ";
                            nPart++;
                            break;
                        case 0xf:
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
                                rRet += "} ";
                            else
                                rRet = rRet.replaceAt(nSubSupStartPos, rRet.getLength(), "");
                            nPart++;
                            }
                            break;
                        case 0x2c:
                            if ((nPart == 0) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            rRet += "} ";
                            nPart++;
                            break;
                        case 0x2e:
                        case 0x2f:
                            rRet += "} ";
                            break;
                        case 0x10:
                        case 0x11:
                            rRet += "}} ";
                            break;
                        case 0x12:
                        case 0x13:
                        case 0x14:
                            if (nPart == 0)
                            {
                                newline--;
                                rRet += "} ";
                            }
                            nPart++;
                            break;
                        case 0x1b:
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                newline--;
                                rRet += "overbrace";
                            }
                            nPart++;
                            break;
                        case 0x1c:
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                newline--;
                                rRet += "underbrace";
                            }
                            nPart++;
                            break;
                        case 0x27:
                            if (nPart==0)
                                newline--;
                            else if ((nPart==1) &&
                                    ((nVariation == 2) || (nVariation == 1)))
                                newline--;
                            rRet += "} ";
                            nPart++;
                            break;
                        case 0x28:
                            rRet += "} ";
                            if (nVariation == 0)
                            {
                                if (nPart == 0)
                                {
                                    sMainTerm = rRet;
                                    rRet.clear();
                                }
                                else
                                {
                                    sPush += rRet;
                                    rRet = sPush;
                                    rRet += " over ";
                                    rRet += sMainTerm;
                                }
                            }
                            if (nPart == 0)
                                newline--;
                            nPart++;
                            break;
                        case 0x29:
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                newline--;
                                switch (nVariation)
                                {
                                case 1:
                                    rRet += "slash";
                                    break;
                                default:
                                    rRet += "wideslash";
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
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                if (nVariation != 2)
                                {
                                    sMainTerm = rRet;
                                    rRet.clear();
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
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 3))
                                {
                                    sMainTerm = rRet;
                                    rRet.clear();
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
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                if ((nVariation != 0) && (nVariation != 2))
                                {
                                    sMainTerm = rRet;
                                    rRet.clear();
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
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                sMainTerm = rRet;
                                rRet.clear();
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
                            rRet += "} ";
                            if (nPart == 0)
                            {
                                sMainTerm = rRet;
                                rRet.clear();
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
                            rRet += "} ";

                            if ((nPart == 0) &&
                                    ((nVariation == 0) || (nVariation == 1)))
                            {
                                sMainTerm = rRet;
                                rRet.clear();
                                newline--;
                            }
                            else if ((nPart == 0) && (nVariation == 2))
                                newline--;
                            else if ((nPart == 1) && (nVariation == 2))
                            {
                                sMainTerm = rRet;
                                rRet.clear();
                                newline--;
                            }
                            else if ((nPart == 2) ||
                                     ((nPart == 1) && (nVariation == 0)) ||
                                     (nVariation == 1))
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
                                    rRet += " mline ";
                                }
                                else
                                    rRet += " rangle ";
                            }
                            else if (nVariation==1)
                                rRet += " \\lline ";
                            else if (nVariation==2)
                                rRet += " \\rangle ";
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
                    std::vector<sal_Char> aSeq;
                    while(true)
                    {
                        sal_Char nChar8(0);
                        pS->ReadChar( nChar8 );
                        if (nChar8 == 0)
                            break;
                        aSeq.push_back(nChar8);
                    }
                    // Do nothing to the font name now in aSeq!?
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
    while (nRecord != END && !pS->IsEof());
    while (nSetSize)
    {
        rRet += "}";
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
    if (nMatrixRows!=0)
    {
        if (rCurCol == nMatrixCols-1)
        {
            if (rCurRow != nMatrixRows-1)
                rRet += " {} ##\n";
            if (nMatrixRows!=-1)
            {
                rCurCol=0;
                rCurRow++;
            }
        }
        else
        {
            rRet += " {} # ";
            if (nMatrixRows!=-1)
                rCurCol++;
            else
                rRet += "\n";
        }
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
        rRet += "alignl {";
        break;
    case 2:
        rRet += "alignc {";
        break;
    case 3:
        rRet += "alignr {";
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
                rRet += "}";
                bRet=true;
            }
            if (-nLstSize/32 != nLastSize)
            {
                nLastSize = nCurSize;
                rRet += " size ";
                rRet += OUString::number(-nLstSize/32);
                rRet += "{";
                bRet=true;
                rSetSize++;
            }
            nCurSize = -nLstSize/32;
        }
    }
    else
    {
        /*sizetable should theoreticaly be filled with the default sizes
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
                rRet += "}";
                bRet=true;
            }
            if (nLstSize != nLastSize)
            {
                nLastSize = nCurSize;
                rRet += " size ";
                rRet += OUString::number(nLstSize);
                rRet += "{";
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
            sal_uInt16  nSize = pNode->GetNumSubNodes();
            for (sal_uInt16 i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
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
            sal_uInt16  nSize = pNode->GetNumSubNodes();
            for (sal_uInt16 i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END );
            }
            break;
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
            sal_uInt16  nSize = pNode->GetNumSubNodes();
            for (sal_uInt16 i = 0; i < nSize; i++)
                if (SmNode *pTemp = pNode->GetSubNode(i))
                    HandleNodes(pTemp,nLevel+1);
            }
            break;
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
    //theres just no way we can now handle any character
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
    sal_uInt16  nSize = pMatrix->GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (SmNode *pTemp = pMatrix->GetSubNode(i))
        {
            pS->WriteUChar( LINE ); //line
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END ); //end line
        }
    pS->WriteUChar( END );
}


//Root Node, PILE equivalent, i.e. vertical stack
void MathType::HandleTable(SmNode *pNode,int nLevel)
{
    sal_uInt16  nSize = pNode->GetNumSubNodes();
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

    for (sal_uInt16 i = 0; i < nSize; i++)
        if (SmNode *pTemp = pNode->GetSubNode(i))
        {
            pS->WriteUChar( LINE );
            HandleNodes(pTemp,nLevel+1);
            pS->WriteUChar( END );
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
        SAL_FALLTHROUGH;
    case TIINT:
    case TLLINT:
        pS->WriteUChar( CHAR );
        pS->WriteUChar( 0x86 );
        pS->WriteUInt16( 0x222B );
        SAL_FALLTHROUGH;
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

    rRet += " stack {\n";
    bool bRet = HandleRecords( nLevel+1, nSelector, nVariation, -1, -1 );
    rRet = rRet.replaceAt(rRet.getLength()-3,2,"");
    rRet += "} ";

    while (rSetAlign)
    {
        rRet += "} ";
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
    rRet += " matrix {\n";
    bool bRet = HandleRecords( nLevel+1, nSelector, nVariation, nRows, nCols );

    sal_Int32 nI = rRet.lastIndexOf('#');
    if (nI > 0)
        if (rRet[nI-1] != '#')  //missing column
            rRet += "{}";

    rRet += "\n} ";
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
        rRet = rRet.replaceAt(rLastTemplateBracket,1,"");
        rRet += "} ";
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
        switch (nEmbel)
        {
        case 0x02:
            rRet += " dot ";
            break;
        case 0x03:
            rRet += " ddot ";
            break;
        case 0x04:
            rRet += " dddot ";
            break;
        case 0x05:
            if (!nPostSup)
            {
                sPost += " sup {}";
                nPostSup = sPost.getLength();
            }
            sPost = sPost.replaceAt(nPostSup-1,0," ' ");
            nPostSup += 3;
            break;
        case 0x06:
            if (!nPostSup)
            {
                sPost += " sup {}";
                nPostSup = sPost.getLength();
            }
            sPost = sPost.replaceAt(nPostSup-1,0," '' ");
            nPostSup += 4;
            break;
        case 0x07:
            if (!nPostlSup)
            {
                sPost += " lsup {}";
                nPostlSup = sPost.getLength();
            }
            sPost = sPost.replaceAt(nPostlSup-1,0," ' ");
            nPostlSup += 3;
            break;
        case 0x08:
            rRet += " tilde ";
            break;
        case 0x09:
            rRet += " hat ";
            break;
        case 0x0b:
            rRet += " vec ";
            break;
        case 0x10:
            rRet += " overstrike ";
            break;
        case 0x11:
            rRet += " bar ";
            break;
        case 0x12:
            if (!nPostSup)
            {
                sPost += " sup {}";
                nPostSup = sPost.getLength();
            }
            sPost = sPost.replaceAt(nPostSup-1,0," ''' ");
            nPostSup += 5;
            break;
        case 0x14:
            rRet += " breve ";
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
    sal_Unicode nChar;
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
        sal_uInt8 nChar8;
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
        //mathtype can all be listed after eachother, in
        //starmath some must go before the character and some
        //must go after. In addition some of the emblishments
        //may repeated and in starmath some of these groups
        //must be gathered together. sPost is the portion that
        //follows the char and nPostSup and nPostlSup are the
        //indexes at which this class of emblishment is
        //collated together
        sPost.clear();
        nPostSup = nPostlSup = 0;
        int nOriglen=rRet.getLength()-rTextStart;
        rRet += " {";  // #i24340# make what would be "vec {A}_n" become "{vec {A}}_n"
        if ((!bSilent) && (nOriglen > 1))
            rRet += "\"";
        bRet = HandleRecords( nLevel+1, nSelector, nVariation );
        if (!bSilent)
        {
            if (nOriglen > 1)
            {
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet = rRet.replaceAt(rTextStart,0,aStr);

                aStr.clear();
                TypeFaceToString(aStr,nTypeFace);
                rRet += aStr + "{";
            }
            else
                rRet += " {";
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
                rRet = rRet.replaceAt(nOldLen, 0, "\"");
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet = rRet.replaceAt(rTextStart,0,aStr);
            }
            rTextStart = rRet.getLength();
        }
        nOldLen = rRet.getLength();
        if (!LookupChar(nChar,rRet,nVersion,nTypeFace))
        {
            if (nOldLen - rTextStart > 1)
            {
                rRet = rRet.replaceAt(nOldLen,0,"\"");
                OUString aStr;
                TypeFaceToString(aStr,nOldTypeFace);
                aStr += "\"";
                rRet = rRet.replaceAt(rTextStart, 0, aStr);
            }
            rTextStart = rRet.getLength();
        }
        lcl_PrependDummyTerm(rRet, rTextStart);
    }

    if ((xfEMBELL(nTag)) && (!bSilent))
    {
        rRet += "}}" + sPost;  // #i24340# make what would be "vec {A}_n" become "{vec {A}}_n"
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
    sal_uInt16  nSize = pNode->GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (SmNode *pTemp = pNode->GetSubNode(i))
            HandleNodes(pTemp,nLevel+1);
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
        else if ((nPendingAttributes) &&
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
            //theres just no way we can now handle any character
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
        if ((nPendingAttributes) &&
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
        if ((nPendingAttributes) &&
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
