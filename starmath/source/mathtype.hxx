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

#pragma once

#include <node.hxx>

class SfxMedium;
class SotStorage;
class SvStream;

class MathTypeFont
{
public:
    sal_uInt8 nTface;
    sal_uInt8 nStyle;
    MathTypeFont() : nTface(0),nStyle(0) {}
    explicit MathTypeFont(sal_uInt8 nFace) : nTface(nFace),nStyle(0) {}
    void AppendStyleToText(OUString &rS);
};

struct LessMathTypeFont
{
    bool operator() (const MathTypeFont &rValue1,
        const MathTypeFont &rValue2) const
    {
        return rValue1.nTface < rValue2.nTface;
    }
};

typedef ::std::set< MathTypeFont, LessMathTypeFont > MathTypeFontSet;

class MathType
{
public:
    explicit MathType(OUStringBuffer &rIn)
        : nVersion(0)
        , pS(nullptr)
        , rRet(rIn)
        , pTree(nullptr)
        , nHAlign(0)
        , nPendingAttributes(0)
        , nInsertion(0)
        , nLSize(0)
        , nDSize(0)
        , nCurSize(0)
        , nLastSize(0)
        , nSpec(0)
        , bIsReInterpBrace(false)
        , nPostSup(0)
        , nPostlSup(0)
        , nTypeFace(0)
    {
        Init();
    }

    MathType(OUStringBuffer &rIn,SmNode *pIn)
        : nVersion(0)
        , pS(nullptr)
        , rRet(rIn)
        , pTree(pIn)
        , nHAlign(2)
        , nPendingAttributes(0)
        , nInsertion(0)
        , nLSize(0)
        , nDSize(0)
        , nCurSize(0)
        , nLastSize(0)
        , nSpec(0)
        , bIsReInterpBrace(false)
        , nPostSup(0)
        , nPostlSup(0)
        , nTypeFace(0)
    {
        Init();
    }

    bool Parse(SotStorage* pStor);
    bool Parse(SvStream* pStream);
    bool ConvertFromStarMath( SfxMedium& rMedium );

private:
/*Ver 2 Header*/
    sal_uInt8 nVersion;

    SvStream* pS;

    void Init();

    bool HandleRecords(int nLevel, sal_uInt8 nSelector =0xFF,
        sal_uInt8 nVariation =0xFF, int nRows =0, int nCols =0);
    bool HandleSize(sal_Int16 nLSize, sal_Int16 nDSize, int &rSetSize);
    void HandleAlign(sal_uInt8 nHAlign, int &rSetAlign);
    bool HandlePile(int &rSetAlign, int nLevel, sal_uInt8 nSelector, sal_uInt8 nVariation);
    bool HandleMatrix(int nLevel, sal_uInt8 nSelector, sal_uInt8 nVariarion);
    void HandleMatrixSeparator(int nMatrixRows, int nMatrixCols, int &rCurCol, int &rCurRow);
    bool HandleTemplate(int nLevel, sal_uInt8 &rSelector, sal_uInt8 &rVariation,
        sal_Int32 &rLastTemplateBracket);
    void HandleEmblishments();
    void HandleSetSize();
    bool HandleChar(sal_Int32 &rTextStart, int &rSetSize, int nLevel,
        sal_uInt8 nTag, sal_uInt8 nSelector, sal_uInt8 nVariation, bool bSilent);
    void HandleNudge();

    static int xfLMOVE(sal_uInt8 nTest) {return nTest&0x80;}
    static int xfAUTO(sal_uInt8 nTest) {return nTest&0x10;}
    static int xfEMBELL(sal_uInt8 nTest) {return nTest&0x20;}
    static int xfNULL(sal_uInt8 nTest) {return nTest&0x10;}

    void HandleNodes(SmNode *pNode,int nLevel);
    int StartTemplate(sal_uInt16 nSelector,sal_uInt16 nVariation=0);
    void EndTemplate(int nOldPendingAttributes);
    void HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel);
    void HandleTable(SmNode *pNode,int nLevel);
    void HandleRoot(SmNode *pNode,int nLevel);
    void HandleSubSupScript(SmNode *pNode,int nLevel);
    sal_uInt8 HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,
        sal_uInt64 *pPos=nullptr,bool bTest=true);
    void HandleFractions(SmNode *pNode,int nLevel);
    void HandleBrace(SmNode *pNode,int nLevel);
    void HandleVerticalBrace(SmNode *pNode,int nLevel);
    void HandleOperator(SmNode *pNode,int nLevel);
    bool HandleLim(SmNode *pNode,int nLevel);
    void HandleMAlign(SmNode *pNode,int nLevel);
    void HandleMath(SmNode *pNode);
    void HandleText(SmNode *pNode);
    void HandleAttributes(SmNode *pNode,int nLevel);
    void TypeFaceToString(OUString &rRet,sal_uInt8 nFace);

    OUStringBuffer &rRet;
    SmNode *pTree;

    sal_uInt8 nHAlign;

    int nPendingAttributes;
    sal_uInt64 nInsertion;

    std::vector<sal_Int16> aSizeTable;
    sal_Int16 nLSize;
    sal_Int16 nDSize;
    sal_Int16 nCurSize;
    sal_Int16 nLastSize;
    sal_uInt8 nSpec;
    bool  bIsReInterpBrace;
    OUStringBuffer sPost;
    sal_Int32 nPostSup;
    sal_Int32 nPostlSup;
    sal_uInt8 nTypeFace;
    MathTypeFontSet aUserStyles;

    enum MTOKENS {END,LINE,CHAR,TMPL,PILE,MATRIX,EMBEL,RULER,FONT,SIZE};
    enum MTEMPLATES
    {
        tmANGLE,tmPAREN,tmBRACE,tmBRACK,tmBAR,tmDBAR,tmFLOOR,tmCEILING,
        tmLBLB,tmRBRB,tmRBLB,tmLBRP,tmLPRB,tmROOT,tmFRACT,tmSCRIPT,tmUBAR,
        tmOBAR,tmLARROW,tmRARROW,tmBARROW,tmSINT,tmDINT,tmTINT,tmSSINT,
        tmDSINT,tmTSINT,tmUHBRACE,tmLHBRACE,tmSUM,tmISUM,tmPROD,tmIPROD,
        tmCOPROD,tmICOPROD,tmUNION,tmIUNION,tmINTER,tmIINTER,tmLIM,tmLDIV,
        tmSLFRACT,tmINTOP,tmSUMOP,tmLSCRIPT,tmDIRAC,tmUARROW,tmOARROW,
        tmOARC
    };
public:
    static bool LookupChar(sal_Unicode nChar,OUStringBuffer &rRet,
        sal_uInt8 nVersion,sal_uInt8 nTypeFace=0);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
