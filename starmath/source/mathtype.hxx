/*************************************************************************
 *
 *  $RCSfile: mathtype.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:26 $
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
#ifndef MATHTYPE_HXX
#define MATHTYPE_HXX

#ifndef NODE_HXX
#include "node.hxx"
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#define EQNOLEFILEHDR_SIZE 28
class EQNOLEFILEHDR
{
public:
    EQNOLEFILEHDR() {}
    EQNOLEFILEHDR(UINT32 nLenMTEF): nCBHdr(0x1c), nVersion(0x20000),
    nCf(0xc1c6), nCBObject(nLenMTEF), nReserved1(0), nReserved2(0x0014F690),
    nReserved3(0x0014EBB4), nReserved4(0) {}
    UINT16   nCBHdr;     // length of header, sizeof(EQNOLEFILEHDR) = 28 bytes
    UINT32   nVersion;   // hiword = 2, loword = 0
    UINT16   nCf;        // clipboard format ("MathType EF")
    UINT32   nCBObject;  // length of MTEF data following this header in bytes
    UINT32   nReserved1; // not used
    UINT32   nReserved2; // not used
    UINT32   nReserved3; // not used
    UINT32   nReserved4; // not used

    void Read(SvStorageStream *pS);
    void Write(SvStorageStream *pS);
};

class MathType
{
public:
    MathType(String &rIn) : bSilent(FALSE),nDefaultSize(12),
        nLSize(0),nDSize(0),nCurSize(0),nLastSize(0),nVAlign(0),nHAlign(0),rRet(rIn)
    {
        aSizeTable[0]=12;
        aSizeTable[1]=8;
        aSizeTable[2]=6;
        aSizeTable[3]=24;
        aSizeTable[4]=10;
        aSizeTable[5]=12;
        aSizeTable[6]=12;
    }
    MathType(String &rIn,SmNode *pIn) : bSilent(FALSE),nDefaultSize(12),
        nLSize(0),nDSize(0),nCurSize(0),nLastSize(0),nVAlign(0),nHAlign(2),pTree(pIn),
        rRet(rIn),nInsertion(0),nSpec(0)
    {
        aSizeTable[0]=12;
        aSizeTable[1]=8;
        aSizeTable[2]=6;
        aSizeTable[3]=24;
        aSizeTable[4]=10;
        aSizeTable[5]=12;
        aSizeTable[6]=12;
    }
    int Parse(SvStorage *pS);
    int ConvertFromStarMath(SvStorage *pStor);
private:
/*Ver 2 Header*/
    BYTE nVersion;
    BYTE nPlatform;
    BYTE nProduct;
    BYTE nProdVersion;
    BYTE nProdSubVersion;

    SvStorageStream *pS;

    int HandleRecords(int nLevel=0,BYTE nSelector=-1,
        BYTE nVariation=-1,int nRows=0,int nCols=0);
    BOOL LookupChar(sal_Unicode nChar);
    BOOL HandleSize(INT16 nLSize,INT16 nDSize, int &rSetSize);
    void HandleAlign(BYTE nHAlign,BYTE nVAlign, int &rSetAlign);
    int HandlePile(int &rSetAlign,int nLevel,BYTE nSelector,BYTE nVariation);
    int HandleMatrix(int nLevel,BYTE nSelector,BYTE nVariarion);
    void HandleMatrixSeperator(int nMatrixRows,int nMatrixCols,
        int &rCurCol,int &rCurRow);
    int HandleTemplate(int nLevel,BYTE &rSelector,BYTE &rVariation);
    void HandleEmblishments();
    void HandleSetSize();
    int HandleChar(xub_StrLen &rTextStart,int &rSetSize,int nLevel,BYTE nTag,
        BYTE nSelector,BYTE nVariation,BOOL bSilent);
    int xfLMOVE(BYTE nTest) {return nTest&0x80;}
    int xfAUTO(BYTE nTest) {return nTest&0x10;}
    int xfEMBELL(BYTE nTest) {return nTest&0x20;}
    int xfNULL(BYTE nTest) {return nTest&0x10;}
    int xfLSPACE(BYTE nTest) {return nTest&0x40;}
    int xfRULER(BYTE nTest) {return nTest&0x20;}

    BYTE HandleNodes(SmNode *pNode,int nLevel=0);
    int StartTemplate(UINT16 nSelector,UINT16 nVariation=0);
    void EndTemplate(int nOldPendingAttributes);
    void HandleSmMatrix(SmMatrixNode *pMatrix,int nLevel);
    void HandleTable(SmNode *pNode,int nLevel);
    void HandleRoot(SmNode *pNode,int nLevel);
    void HandleSubSupScript(SmNode *pNode,int nLevel);
    BYTE HandleCScript(SmNode *pNode,SmNode *pContent,int nLevel,ULONG *pPos=NULL,BOOL bTest=TRUE);
    void HandleFractions(SmNode *pNode,int nLevel);
    void HandleBrace(SmNode *pNode,int nLevel);
    void HandleVerticalBrace(SmNode *pNode,int nLevel);
    void HandleOperator(SmNode *pNode,int nLevel);
    BOOL HandleLim(SmNode *pNode,int nLevel);
    void HandleMAlign(SmNode *pNode,int nLevel);
    void HandleMath(SmNode *pNode,int nLevel);
    void HandleText(SmNode *pNode,int nLevel);
    void HandleAttributes(SmNode *pNode,int nLevel);

    String &rRet;
    SmNode *pTree;

    BYTE nHAlign;
    BYTE nVAlign;

    int nPendingAttributes;
    ULONG nInsertion;

    INT16 aSizeTable[7];
    INT16 nDefaultSize;
    INT16 nLSize;
    INT16 nDSize;
    INT16 nCurSize;
    INT16 nLastSize;
    BYTE nSpec;
    BOOL bSilent,bReInterpBrace;
    String sPost;
    xub_StrLen nPostSup;
    xub_StrLen nPostlSup;
    BYTE nTypeFace;

    enum MTOKENS {END,LINE,CHAR,TMPL,PILE,MATRIX,EMBEL,RULER,FONT,SIZE};
    enum MTEMPLATES
    {
        tmANGLE,tmPAREN,tmBRACE,tmBRACK,tmBAR,tmDBAR,tmFLOOR,tmCEILING,
        tmLBLB,tmRBRB,tmRBLB,tmLBRP,tmLPRB,tmROOT,tmFRACT,tmSCRIPT,tmUBAR,
        tmOBAR,tmLARROW,tmRARROW,tmBARROW,tmSINT,tmDINT,tmTINT,tmSSINT,
        tmDSINT,tmTSINT,tmUHBRACE,tmLHBRACE,tmSUM
    };
};


#endif
