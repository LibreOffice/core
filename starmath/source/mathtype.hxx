/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mathtype.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:10:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef MATHTYPE_HXX
#define MATHTYPE_HXX

#ifndef NODE_HXX
#include "node.hxx"
#endif
#ifndef __EQNOLEFILEHDR_HXX__
#include "eqnolefilehdr.hxx"
#endif

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
    void AppendStyleToText(String &rS);
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
        Init();
    }
    MathType(String &rIn,SmNode *pIn) : bSilent(sal_False),nDefaultSize(12),
        nLSize(0),nDSize(0),nCurSize(0),nLastSize(0),nVAlign(0),nHAlign(2),
        pTree(pIn),rRet(rIn),nInsertion(0),nSpec(0)
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

    int HandleRecords(int nLevel=0,sal_uInt8 nSelector=-1,
        sal_uInt8 nVariation=-1,int nRows=0,int nCols=0);
    sal_Bool HandleSize(sal_Int16 nLSize,sal_Int16 nDSize, int &rSetSize);
    void HandleAlign(sal_uInt8 nHAlign,sal_uInt8 nVAlign, int &rSetAlign);
    int HandlePile(int &rSetAlign,int nLevel,sal_uInt8 nSelector,
        sal_uInt8 nVariation);
    int HandleMatrix(int nLevel,sal_uInt8 nSelector,sal_uInt8 nVariarion);
    void HandleMatrixSeperator(int nMatrixRows,int nMatrixCols,int &rCurCol,
        int &rCurRow);
    int HandleTemplate(int nLevel,sal_uInt8 &rSelector,sal_uInt8 &rVariation,
        xub_StrLen &rLastTemplateBracket);
    void HandleEmblishments();
    void HandleSetSize();
    int HandleChar(xub_StrLen &rTextStart,int &rSetSize,int nLevel,
        sal_uInt8 nTag,sal_uInt8 nSelector,sal_uInt8 nVariation,
        sal_Bool bSilent);
    void HandleNudge();
    int xfLMOVE(sal_uInt8 nTest) {return nTest&0x80;}
    int xfAUTO(sal_uInt8 nTest) {return nTest&0x10;}
    int xfEMBELL(sal_uInt8 nTest) {return nTest&0x20;}
    int xfNULL(sal_uInt8 nTest) {return nTest&0x10;}
    int xfLSPACE(sal_uInt8 nTest) {return nTest&0x40;}
    int xfRULER(sal_uInt8 nTest) {return nTest&0x20;}

    sal_uInt8 HandleNodes(SmNode *pNode,int nLevel=0);
    int StartTemplate(sal_uInt16 nSelector,sal_uInt16 nVariation=0);
    void EndTemplate(int nOldPendingAttributes);
    void HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel);
    void HandleTable(SmNode *pNode,int nLevel);
    void HandleRoot(SmNode *pNode,int nLevel);
    void HandleSubSupScript(SmNode *pNode,int nLevel);
    sal_uInt8 HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,
        ULONG *pPos=NULL,sal_Bool bTest=TRUE);
    void HandleFractions(SmNode *pNode,int nLevel);
    void HandleBrace(SmNode *pNode,int nLevel);
    void HandleVerticalBrace(SmNode *pNode,int nLevel);
    void HandleOperator(SmNode *pNode,int nLevel);
    sal_Bool HandleLim(SmNode *pNode,int nLevel);
    void HandleMAlign(SmNode *pNode,int nLevel);
    void HandleMath(SmNode *pNode,int nLevel);
    void HandleText(SmNode *pNode,int nLevel);
    void HandleAttributes(SmNode *pNode,int nLevel);
    void TypeFaceToString(String &rRet,sal_uInt8 nFace);

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
    static sal_Bool LookupChar(sal_Unicode nChar,String &rRet,
        sal_uInt8 nVersion=3,sal_uInt8 nTypeFace=0);
};


#endif
