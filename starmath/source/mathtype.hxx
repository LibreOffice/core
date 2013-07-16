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

#ifndef MATHTYPE_HXX
#define MATHTYPE_HXX

#include "node.hxx"
#include "eqnolefilehdr.hxx"

#include <sot/storage.hxx>

#include <set>

class SfxMedium;

class MathTypeFont
{
public:
    sal_uInt8 nTface;
    sal_uInt8 nStyle;
    MathTypeFont() : nTface(0),nStyle(0) {}
    MathTypeFont(sal_uInt8 nFace) : nTface(nFace),nStyle(0) {}
    void AppendStyleToText(OUString &rS);
};

struct LessMathTypeFont
{
    sal_Bool operator() (const MathTypeFont &rValue1,
        const MathTypeFont &rValue2) const
    {
        return rValue1.nTface < rValue2.nTface;
    }
};

typedef ::std::set< MathTypeFont, LessMathTypeFont > MathTypeFontSet;

class MathType
{
public:
    MathType(OUString &rIn) :
        rRet(rIn), nHAlign(0), nVAlign(0), nDefaultSize(12),
        nLSize(0), nDSize(0), nCurSize(0), nLastSize(0), bIsSilent(sal_False),
        nTypeFace(0)
    {
        Init();
    }

    MathType(OUString &rIn,SmNode *pIn) :
        rRet(rIn), pTree(pIn), nHAlign(2), nVAlign(0), nInsertion(0), nDefaultSize(12),
        nLSize(0), nDSize(0), nCurSize(0), nLastSize(0), nSpec(0), bIsSilent(sal_False),
        nTypeFace(0)
    {
        Init();
    }

    int Parse( SotStorage* pStor );
    int ConvertFromStarMath( SfxMedium& rMedium );

private:
/*Ver 2 Header*/
    sal_uInt8 nVersion;
    sal_uInt8 nPlatform;
    sal_uInt8 nProduct;
    sal_uInt8 nProdVersion;
    sal_uInt8 nProdSubVersion;

    SvStorageStream *pS;

    void Init();

    int HandleRecords(int nLevel=0,sal_uInt8 nSelector=0xFF,
        sal_uInt8 nVariation=0xFF,int nRows=0,int nCols=0);
    sal_Bool HandleSize(sal_Int16 nLSize,sal_Int16 nDSize, int &rSetSize);
    void HandleAlign(sal_uInt8 nHAlign,sal_uInt8 nVAlign, int &rSetAlign);
    int HandlePile(int &rSetAlign,int nLevel,sal_uInt8 nSelector,
        sal_uInt8 nVariation);
    int HandleMatrix(int nLevel,sal_uInt8 nSelector,sal_uInt8 nVariarion);
    void HandleMatrixSeparator(int nMatrixRows,int nMatrixCols,int &rCurCol,
        int &rCurRow);
    int HandleTemplate(int nLevel,sal_uInt8 &rSelector,sal_uInt8 &rVariation,
        sal_Int32 &rLastTemplateBracket);
    void HandleEmblishments();
    void HandleSetSize();
    int HandleChar(sal_Int32 &rTextStart,int &rSetSize,int nLevel,
        sal_uInt8 nTag,sal_uInt8 nSelector,sal_uInt8 nVariation,
        sal_Bool bSilent);
    void HandleNudge();
    int xfLMOVE(sal_uInt8 nTest) const {return nTest&0x80;}
    int xfAUTO(sal_uInt8 nTest) const {return nTest&0x10;}
    int xfEMBELL(sal_uInt8 nTest) const {return nTest&0x20;}
    int xfNULL(sal_uInt8 nTest) const {return nTest&0x10;}
    int xfLSPACE(sal_uInt8 nTest) const {return nTest&0x40;}
    int xfRULER(sal_uInt8 nTest) const {return nTest&0x20;}

    sal_uInt8 HandleNodes(SmNode *pNode,int nLevel=0);
    int StartTemplate(sal_uInt16 nSelector,sal_uInt16 nVariation=0);
    void EndTemplate(int nOldPendingAttributes);
    void HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel);
    void HandleTable(SmNode *pNode,int nLevel);
    void HandleRoot(SmNode *pNode,int nLevel);
    void HandleSubSupScript(SmNode *pNode,int nLevel);
    sal_uInt8 HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,
        sal_uLong *pPos=NULL,sal_Bool bTest=sal_True);
    void HandleFractions(SmNode *pNode,int nLevel);
    void HandleBrace(SmNode *pNode,int nLevel);
    void HandleVerticalBrace(SmNode *pNode,int nLevel);
    void HandleOperator(SmNode *pNode,int nLevel);
    sal_Bool HandleLim(SmNode *pNode,int nLevel);
    void HandleMAlign(SmNode *pNode,int nLevel);
    void HandleMath(SmNode *pNode,int nLevel);
    void HandleText(SmNode *pNode,int nLevel);
    void HandleAttributes(SmNode *pNode,int nLevel);
    void TypeFaceToString(OUString &rRet,sal_uInt8 nFace);

    OUString &rRet;
    SmNode *pTree;

    sal_uInt8 nHAlign;
    sal_uInt8 nVAlign;

    int nPendingAttributes;
    sal_uLong nInsertion;

    sal_Int16 aSizeTable[7];
    sal_Int16 nDefaultSize;
    sal_Int16 nLSize;
    sal_Int16 nDSize;
    sal_Int16 nCurSize;
    sal_Int16 nLastSize;
    sal_uInt8 nSpec;
    sal_Bool  bIsSilent, bIsReInterpBrace;
    OUString  sPost;
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
        tmDSINT,tmTSINT,tmUHBRACE,tmLHBRACE,tmSUM
    };
public:
    static sal_Bool LookupChar(sal_Unicode nChar,OUString &rRet,
        sal_uInt8 nVersion=3,sal_uInt8 nTypeFace=0);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
