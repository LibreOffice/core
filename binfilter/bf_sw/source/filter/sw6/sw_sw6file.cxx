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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <string.h>
#include "errhdl.hxx"

#include "sw6file.hxx"
namespace binfilter {


// --- LAYDESC ---

void LAYDESC::DelLayout(BOOL GLay,BOOL Kopf,BOOL Fuss)
{
    short Idx;

    if (GLay)
    {
        for (Idx=0; Idx<NumAlay; Idx++)
            if (FileAlay[Idx])
                DELETEZ(FileAlay[Idx]);
        NumAlay=0;
        for (Idx=0; Idx<NumSlay; Idx++)
            if (FileSlay[Idx])
                DELETEZ(FileSlay[Idx]);
        NumSlay=0;
        for (Idx=0; Idx<NumBlay; Idx++)
            if (FileBlay[Idx])
                DELETEZ(FileBlay[Idx]);
        NumBlay=0;
    }
    if (Kopf)
    {
        for (Idx=0; Idx<NumKopf; Idx++)
            if (HartKopf[Idx])
                DELETEZ(HartKopf[Idx]);
        NumKopf=0;
    }
    if (Fuss)
    {
        for (Idx=0; Idx<NumFuss; Idx++)
            if (HartFuss[Idx])
                DELETEZ(HartFuss[Idx]);
        NumFuss=0;
    }
}

// --- Sw6Stack ---

void Sw6Stack::Clr(void)
// loescht oberstes Element
{
    DATA *pOld=pData;
    pData=pData->pNext;
    delete (sal_Char *)pOld;
}

BOOL Sw6Stack::Push(void *pSrce,size_t nMaxSize)
// Kopiert ein Element auf den Stack drauf
{
    if (pSrce && nMaxSize)
    {
        DATA *pOld=pData;
        if ((pData=(DATA *)new         // Hole neues Element und falls
            sal_Char[sizeof(*pData)+       // dies ok war, stelle auf Stack
            nMaxSize])!=NULL)
        {
            pData->pNext=pOld;
            pData->nSize=nMaxSize;
            register sal_Char *q=(sal_Char *)pSrce;
            register sal_Char *p=(sal_Char *)pData+sizeof(*pData);
            while (nMaxSize--) *p++=*q++;
            return TRUE;
        }
        else pData=pOld;
    }
    return FALSE;
}

BOOL Sw6Stack::Get(void *pDest,size_t nMaxSize,size_t *pnSize)
// Kopiert Element nach *pDest mit maximal nMaxSize
// Bytes. *pnSize enthaelt dann tatsaechliche Anzahl
{
    if (pDest && nMaxSize && pData)
    {
        if (pnSize) *pnSize=pData->nSize;
        if (nMaxSize>pData->nSize) nMaxSize=pData->nSize;
        register sal_Char *p=(sal_Char *)pDest;
        register sal_Char *q=(sal_Char *)pData+sizeof(*pData);
        while (nMaxSize--) *p++=*q++;
        return TRUE;
    }
    return FALSE;
}

BOOL Sw6Stack::Pop(void *pDest,size_t nMaxSize,size_t *pnSize)
// Holt Element nach *pDest und loescht es vom Stack
{
    if (Get(pDest,nMaxSize,pnSize))
    {
        Clr();
        return TRUE;
    }
    return FALSE;
}

void Sw6Stack::Clear(void)
// Loescht den Stack komplett
{
    while (pData) Clr();
}

// --- Sw6File ---

void Sw6File::PushStat(long nFPos)
// Merke des Status
{
    long nPos=nFPos;

    if (nPos>=0)
    {
        nPos=Tell();
        Seek(nFPos);
    }
    aStk.Push(&nPos,sizeof(nPos));
    aStk.Push(&aSta,sizeof(aSta));
}

void Sw6File::PopStat(void)
// Wiederherstellen Status
{
    long nPos;
    aStk.Pop(&aSta,sizeof(aSta));
    aStk.Pop(&nPos,sizeof(nPos));

    if (nPos>=0) Seek(nPos);
}

BOOL Sw6File::FlushPhys(short nAnz, BUFFR &rBuf)
// Holt immer nAnz Zeichen nach pStr[nIdx], egal wie der Puf-
// ferstand aus ist. Aufruf mit nAnz==0 dient zum Input-Flush
{
    if (rBuf.nIdx+nAnz>=rBuf.nMax)
    {
        ASSERT(rBuf.nIdx<=rBuf.nMax,"Sw6-Datei ist wohl defekt");

        if (rBuf.nMax<rBuf.nIdx)
        {
            rBuf.nMax=rBuf.nIdx;       // Datei war Schrott...
        }
        memmove(rBuf.pBuf,
            &rBuf.pBuf[rBuf.nIdx],     // Gewnschte Zeichen runter-
            nAnz=rBuf.nMax-rBuf.nIdx); // schieben und Rest dazulesen
        rBuf.nIdx=0;                   // Leseindex im Puffer auf 0

        //$ pInp->clear(pInp->rdstate()&~ios::eofbit); // Kein EOF
        pInp->ResetError();
        ULONG lGCount = pInp->Read(&rBuf.pBuf[nAnz],rBuf.nSiz-nAnz);
        rBuf.pBuf[rBuf.nMax=nAnz+lGCount]='\x1A';
        if (!FileOk()) return FALSE;
        if (rBuf.nMax<rBuf.nSiz-nAnz) // Nicht alles gelesen?
        {
            //$ pInp->clear(pInp->rdstate() & ~ios::failbit); // Kein Fail-Bit
            pInp->ResetError();
        }
    }
    return TRUE;
}

BOOL Sw6File::Flush(short nAnz)
{
    if (aPwd.pBuf)
    {
        if (aBuf.nIdx+nAnz>=aBuf.nMax)
        {
            memmove(aBuf.pBuf,
                &aBuf.pBuf[aBuf.nIdx],
                nAnz=aBuf.nMax-aBuf.nIdx);
            aBuf.nIdx=0;
            for (aBuf.nMax=0; aBuf.nMax<
                aBuf.nSiz-nAnz; aBuf.nMax++)
            {
                NODE *pNode=&aNode;
                while (pNode->bAst)
                {
                    pNode=(ReadBit(aPwd)==0)?
                        pNode->pLks:pNode->pRts;
                }
                aBuf.pBuf[nAnz+aBuf.nMax]=pNode->cZch;
                if (aPwd.nMax==0)
                {
                    break;             // Nothalt fuer EOF
                }
            }
        }
        return TRUE;
    }
    else return FlushPhys(nAnz,aBuf);
}

BOOL Sw6File::Eof(void) const
// Gibt TRUE zurck wenn End of File erreicht
{
    return aBuf.pBuf?aBuf.pBuf[aBuf.nIdx]=='\x1A':pInp->IsEof();
}

long Sw6File::Tell(void) const
// Gibt die Fileposition zurck
{
    return pInp->Tell()+aBuf.nIdx-aBuf.nMax;
}

void Sw6File::Seek(const long lWhere)
// Sucht zu einer Fileposition
{
    pInp->Seek(lWhere);
    aBuf.nIdx=aBuf.nMax=0;
    Flush(0);
}


BYTE Sw6File::ReadBit(BUFFR &rBuf)
// Lese genau 1 Bit ein
{
    BYTE nRet = (nBit&0x80) !=0 ;
    nBit=(nBit&0x7f)<<1;

    if (nBit==0)
    {
        FlushPhys(1,rBuf);
        nBit=rBuf.pBuf[rBuf.nIdx];
        nRet=(nBit&0x80)!=0;
        nBit=((nBit&0x7f)<<1)+1;
        rBuf.nIdx++;
    }
    return nRet;
}

void Sw6File::DelTree(NODE **ppNode)
{
    if (*ppNode)
    {
        DelTree(&(*ppNode)->pLks);
        DelTree(&(*ppNode)->pRts);
        delete *ppNode;
    }
}

void Sw6File::ReadTree(NODE *pNode)
{
    if (nLev<256 && nOvl==0)
    {
        nLev++;
        if (ReadBit(aBuf)==0)
        {
            pNode->pLks=new NODE;
            pNode->pRts=new NODE;
            pNode->bAst=TRUE;
            if (pNode->pLks && pNode->pRts)
            {
                ReadTree(pNode->pLks);
                ReadTree(pNode->pRts);
            }
            else nOvl=2;
        }
        else
        {
            pNode->pLks=NULL;
            pNode->pRts=NULL;
            pNode->bAst=FALSE;
            pNode->cZch=0;
            for (USHORT n=0; n<8; n++)
            {
                pNode->cZch=(pNode->
                    cZch<<1)+ReadBit(aBuf);
            }
        }
        nLev--;
    }
    else nOvl=1;
}

BOOL Sw6File::ActivatePass(void)
// Lese Codebaum und setze ReadLn um
{
    Flush(3);
    if (aBuf.pBuf[aBuf.nIdx+0]==NewPara &&
        aBuf.pBuf[aBuf.nIdx+1]==NewLine &&
        aBuf.pBuf[aBuf.nIdx+2]==NewText)
    {                                  // Es werden CR/LF/EOF erwartet
        aBuf.nIdx+=3;
        nBit=0;
        nLev=0;
        nOvl=0;
        ReadTree(&aNode);
        nBit=0;                        // Fuers Lesen nochmal Reset Bitlesen
        if (nOvl==0)
        {
            aPwd.nMax=aBuf.nMax;
            aPwd.nSiz=aBuf.nSiz;
            aPwd.nIdx=aBuf.nIdx;
            aPwd.pBuf=aBuf.pBuf;
            aBuf.nMax=0;
            aBuf.nIdx=0;
            if ((aBuf.pBuf=new sal_Char[aBuf.nSiz+1])==NULL)
            {
                aBuf.nMax=aPwd.nMax;
                aBuf.nSiz=aPwd.nSiz;
                aBuf.nIdx=aPwd.nIdx;
                aBuf.pBuf=aPwd.pBuf;
                aPwd.pBuf=NULL;
            }
            else return Flush(0);
        }
    }
    return FALSE;
}

BOOL Sw6File::TestCtrl(sal_Char cZch)
// Spezial:Test auf Ende Extratext
// ob cZch und ^M^J gleich kommen!
{
    Flush(3);                                    // Teste die naechsten 3 Zchn
    return aBuf.pBuf[aBuf.nIdx+0]==cZch &&
           aBuf.pBuf[aBuf.nIdx+1]==NewPara &&
           aBuf.pBuf[aBuf.nIdx+2]==NewLine;
}


BOOL Sw6File::ReadLn(String &rStr)
// Lese eine PC-Zeile ein (CR/LF)
{
    rStr.Erase();                       // String ist leer
    while (FileOk())                    // Solange kein Fehler auftrat
    {
        register BYTE *p = (BYTE *)&aBuf.pBuf[aBuf.nIdx], *pStt = p;
        while (*p>'#' || *p>=' ' && *p<'#')
            p++;

        xub_StrLen nAsc = p - pStt;
        if( nAsc )
        {
            rtl_TextEncoding eEnc = aSta.bTrs ? RTL_TEXTENCODING_IBM_850
                                              :	gsl_getSystemTextEncoding();
            rStr += String( (sal_Char*)pStt, nAsc, eEnc );

            aBuf.nIdx += nAsc;
        }
        switch (aBuf.pBuf[aBuf.nIdx])
        {
            case NewPara:
                aBuf.nIdx++;            // NewPara annehmen
                Flush(1);               // Sicherstellen das 1 Zchn da ist
                if (aBuf.pBuf[aBuf.nIdx]==NewLine)
                {
                    aBuf.nIdx++;
                    return TRUE;
                }
                break;
            case NewLine:
                aBuf.nIdx++;            // Zeichen annehmen
                Flush(0);               // Einfach ignorieren
                break;
            case NewText:
                if (aBuf.nIdx<aBuf.nMax)
                {                       // Eof bleibt im Puffer
                    return TRUE;        // erhalten und liefert
                }                       // ab hier Leerzeilen
                Flush(0);
                break;
            default:
                Flush(nCtr);            // Test ob es nCtr Zeichen gibt
                aBuf.nIdx+=PutRest(     // Zchn annehmen, Ignore Anzahl Zchn
                    rStr,&aBuf.pBuf[aBuf.nIdx]);
        } // switch
    } // while
    return FALSE;
}


Sw6File::Sw6File( SvStream &rInp, size_t nBufSize, size_t nCtrSize,
                    ULONG *pErrno )
// ctor, allokiert Puffer und merkt sich den
// offenen Stream, von dem der Input kommt.
{
    pInp=&rInp;
    pErr = pErrno;
    nCtr=nCtrSize;
    aSta.bErr=FALSE;
    aSta.bTrs=FALSE;
    aSta.bEof=pInp->IsEof();
    aBuf.nSiz=nBufSize;
    if (aBuf.nSiz && aBuf.nSiz>         // Puffergr”áe==0 unzul„ssig
        nCtr && (aBuf.pBuf=new          // nSiz<Ctrl-SSize ebenfalls
        sal_Char[aBuf.nSiz+1])!=NULL)
    {
        Flush(0);                       // Lese nun den Puffer voll
    }
    aNode.bAst=TRUE;
    aNode.pLks=NULL;
    aNode.pRts=NULL;
}

Sw6File::~Sw6File(void)
// dtor, gibt den Filepuffer wieder frei
{
    DelTree(&aNode.pLks);
    DelTree(&aNode.pRts);
}

}
