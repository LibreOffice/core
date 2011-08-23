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
#ifndef MATHTYPE_HXX
#define MATHTYPE_HXX

#include "node.hxx"

#include <bf_so3/svstor.hxx>

#include <set>
namespace binfilter {

#define EQNOLEFILEHDR_SIZE 28
class EQNOLEFILEHDR 
{
public:
    EQNOLEFILEHDR() {}
    EQNOLEFILEHDR(sal_uInt32 nLenMTEF) : nCBHdr(0x1c),nVersion(0x20000),
        nCf(0xc1c6),nCBObject(nLenMTEF),nReserved1(0),nReserved2(0x0014F690),
        nReserved3(0x0014EBB4), nReserved4(0) {}
    sal_uInt16   nCBHdr;     // length of header, sizeof(EQNOLEFILEHDR) = 28 
    sal_uInt32   nVersion;   // hiword = 2, loword = 0
    sal_uInt16   nCf;        // clipboard format ("MathType EF")
    sal_uInt32   nCBObject;  // length of MTEF data following this header 
    sal_uInt32   nReserved1; // not used
    sal_uInt32   nReserved2; // not used
    sal_uInt32   nReserved3; // not used
    sal_uInt32   nReserved4; // not used
    
};

class MathTypeFont
{
public:
    sal_uInt8 nTface;
    sal_uInt8 nStyle;
    MathTypeFont() : nTface(0),nStyle(0) {}
    MathTypeFont(sal_uInt8 nFace) : nTface(nFace),nStyle(0) {}
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
    MathType(String &rIn) : bSilent(sal_False),nDefaultSize(12),nLSize(0),
        nDSize(0),nCurSize(0),nLastSize(0),nVAlign(0),nHAlign(0),rRet(rIn)
    {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 		Init();
    }
    MathType(String &rIn,SmNode *pIn) : bSilent(sal_False),nDefaultSize(12),
        nLSize(0),nDSize(0),nCurSize(0),nLastSize(0),nVAlign(0),nHAlign(2),
        pTree(pIn),rRet(rIn),nInsertion(0),nSpec(0)
    {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001Init();
    }
private:
/*Ver 2 Header*/
    sal_uInt8 nVersion;
    sal_uInt8 nPlatform;
    sal_uInt8 nProduct;
    sal_uInt8 nProdVersion;
    sal_uInt8 nProdSubVersion;

    SvStorageStream *pS;


    int xfLMOVE(sal_uInt8 nTest) {return nTest&0x80;}
    int xfAUTO(sal_uInt8 nTest) {return nTest&0x10;}
    int xfEMBELL(sal_uInt8 nTest) {return nTest&0x20;}
    int xfNULL(sal_uInt8 nTest) {return nTest&0x10;}
    int xfLSPACE(sal_uInt8 nTest) {return nTest&0x40;}
    int xfRULER(sal_uInt8 nTest) {return nTest&0x20;}


    String &rRet;
    SmNode *pTree;

    sal_uInt8 nHAlign;
    sal_uInt8 nVAlign;

    int nPendingAttributes;
    ULONG nInsertion;

    sal_Int16 aSizeTable[7];
    sal_Int16 nDefaultSize;
    sal_Int16 nLSize;
    sal_Int16 nDSize;
    sal_Int16 nCurSize;
    sal_Int16 nLastSize;
    sal_uInt8 nSpec;
    sal_Bool bSilent,bReInterpBrace;
    String sPost;
    xub_StrLen nPostSup;
    xub_StrLen nPostlSup; 
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
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
