/*************************************************************************
 *
 *  $RCSfile: dxfgrprd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:15 $
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

#include <string.h>
#include <stdlib.h>
#include <tools/stream.hxx>
#include "dxfgrprd.hxx"

// ----------------------------------------------------------------------------

// we use an own ReadLine function, because Stream::ReadLine stops if
// a 0-sign occurs; this functions converts 0-signs to blanks and reads
// a complete line until a cr/lf is found

BOOL DXFReadLine( SvStream& rIStm, ByteString& rStr )
{
    char  buf[256 + 1];
    BOOL  bEnd = FALSE;
    ULONG nOldFilePos = rIStm.Tell();
    char  c = 0;

    rStr.Erase();

    while( !bEnd && !rIStm.GetError() )   // !!! nicht auf EOF testen,
                                          // !!! weil wir blockweise
                                          // !!! lesen
    {
        USHORT nLen = (USHORT)rIStm.Read( buf, sizeof(buf)-1 );
        if( !nLen )
            if( rStr.Len() == 0 )
                return FALSE;
            else
                break;

        for( USHORT n = 0; n < nLen ; n++ )
        {
            c = buf[n];
            if( c != '\n' && c != '\r' )
            {
                if( !c )
                    c = ' ';
                rStr += c;
            }
            else
            {
                bEnd = TRUE;
                break;
            }
        }
    }

    if( !bEnd && !rIStm.GetError() && rStr.Len() )
        bEnd = TRUE;

    nOldFilePos += rStr.Len();
    if( rIStm.Tell() > nOldFilePos )
        nOldFilePos++;
    rIStm.Seek( nOldFilePos );  // seeken wg. obigem BlockRead!

    if( bEnd && (c=='\r' || c=='\n'))  // Sonderbehandlung DOS-Dateien
    {
        char cTemp;
        rIStm.Read((char*)&cTemp , sizeof(cTemp) );
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            rIStm.Seek( nOldFilePos );
    }

    return bEnd;
}

// ------------------

DXFGroupReader::DXFGroupReader(SvStream & rIStream,
                PFilterCallback pcallback, void * pcallerdata,
                USHORT nminpercent, USHORT nmaxpercent) :
    rIS(rIStream)
{
    USHORT i;

    nIBuffPos=0;
    nIBuffSize=0;
    bStatus=TRUE;
    nLastG=0;
    nGCount=0;

    pCallback=pcallback;
    pCallerData=pcallerdata;
    nMinPercent=(ULONG)nminpercent;
    nMaxPercent=(ULONG)nmaxpercent;
    nLastPercent=nMinPercent;

    rIS.Seek(STREAM_SEEK_TO_END);
    nFileSize=rIS.Tell();
    rIS.Seek(0);

    for (i=0; i<10; i++) S0_9[i][0]=0;
    for (i=0; i<50; i++) F10_59[i]=0.0;
    for (i=0; i<20; i++) I60_79[i]=0;
    for (i=0; i< 8; i++) F140_147[i]=0.0;
    for (i=0; i< 6; i++) I170_175[i]=0;
    for (i=0; i<30; i++) F210_239[i]=0.0;
    for (i=0; i<11; i++) S999_1009[i][0]=0;
    for (i=0; i<50; i++) F1010_1059[i]=0.0;
    for (i=0; i<20; i++) I1060_1079[i]=0;

}


USHORT DXFGroupReader::Read()
{
    USHORT nG;

    if (bStatus==FALSE) goto LErr;

    nGCount++;

    nG=(USHORT)ReadI();

    if (bStatus==FALSE) goto LErr;

    if      (nG<  10) ReadS(S0_9[nG]);
    else if (nG<  60) F10_59[nG-10]=ReadF();
    else if (nG<  80) I60_79[nG-60]=ReadI();
    else if (nG< 140) goto LErr;
    else if (nG< 148) F140_147[nG-140]=ReadF();
    else if (nG< 170) goto LErr;
    else if (nG< 176) I170_175[nG-175]=ReadI();
    else if (nG< 180) ReadI();
    else if (nG< 210) goto LErr;
    else if (nG< 240) F210_239[nG-210]=ReadF();
    else if (nG< 999) goto LErr;
    else if (nG<1010) ReadS(S999_1009[nG-999]);
    else if (nG<1060) F1010_1059[nG-1010]=ReadF();
    else if (nG<1080) I1060_1079[nG-1060]=ReadI();
    else goto LErr;

    if (bStatus==FALSE) goto LErr;

    nLastG=nG;
    return nG;

LErr:
    bStatus=FALSE;
    nLastG=0;
    SetS(0,"EOF");

    if (nGCount!=0xffffffff) {
        // InfoBox(NULL,String("Fehler ab Gruppe Nr ")+String(nGCount)).Execute();
        nGCount=0xffffffff;
    }

    return 0;
}


long DXFGroupReader::GetI(USHORT nG)
{
    nG-=60;
    if (nG<20) return I60_79[nG];
    else {
        nG-=110;
        if (nG<6) return I170_175[nG];
        else {
            nG-=890;
            if (nG<20) return I1060_1079[nG];
            else return 0;
        }
    }
}

double DXFGroupReader::GetF(USHORT nG)
{
    nG-=10;
    if (nG<50) return F10_59[nG];
    else {
        nG-=130;
        if (nG<8) return F140_147[nG];
        else {
            nG-=70;
            if (nG<30) return F210_239[nG];
            else {
                nG-=800;
                if (nG<50) return F1010_1059[nG];
                else return 0;
            }
        }
    }
}

const char * DXFGroupReader::GetS(USHORT nG)
{
    if (nG<10) return S0_9[nG];
    else {
        nG-=999;
        if (nG<11) return S999_1009[nG];
        else return NULL;
    }
}

void DXFGroupReader::SetI(USHORT nG, long nI)
{
    nG-=60;
    if (nG<20) I60_79[nG]=nI;
    else {
        nG-=110;
        if (nG<6) I170_175[nG]=nI;
        else {
            nG-=890;
            if (nG<20) I1060_1079[nG]=nI;
        }
    }
}


void DXFGroupReader::SetF(USHORT nG, double fF)
{
    nG-=10;
    if (nG<50) F10_59[nG]=fF;
    else {
        nG-=130;
        if (nG<8) F140_147[nG]=fF;
        else {
            nG-=70;
            if (nG<30) F210_239[nG]=fF;
            else {
                nG-=800;
                if (nG<50) F1010_1059[nG]=fF;
            }
        }
    }
}


void DXFGroupReader::SetS(USHORT nG, const char * sS)
{
    if (nG<10) strcpy(S0_9[nG],sS);
    else {
        nG-=999;
        if (nG<11) strcpy(S999_1009[nG],sS);
    }
}


void DXFGroupReader::FillIBuff()
{
}


void DXFGroupReader::ReadLine(char * ptgt)
{
    ByteString  aStr;
    ULONG       nLen;

    DXFReadLine( rIS, aStr );
    memcpy( ptgt, aStr.GetBuffer(), nLen = aStr.Len() );
    ptgt[ nLen ] = 0x00;

    if ( pCallback )
    {
        const ULONG nPercent= nMinPercent + (nMaxPercent-nMinPercent)*rIS.Tell() / nFileSize;

        if ( nPercent >= nLastPercent + 4 )
        {
            nLastPercent=nPercent;
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE)
                bStatus=FALSE;
        }
    }
}


long DXFGroupReader::ReadI()
{
    char sl[DXF_MAX_STRING_LEN+1],*p;
    long res,nv;

    ReadLine(sl);

    p=sl;

    while(*p==0x20) p++;

    if ((*p<'0' || *p>'9') && *p!='-') {
        bStatus=FALSE;
        return 0;
    }

    if (*p=='-') {
        nv=-1;
        p++;
    }
    else nv=1;

    res=0;
    do {
        res=res*10+(long)(*p-'0');
        p++;
    } while (*p>='0' && *p<='9');

    while (*p==0x20) p++;
    if (*p!=0) {
        bStatus=FALSE;
        return 0;
    }

    return res*nv;
}


double DXFGroupReader::ReadF()
{
    char sl[DXF_MAX_STRING_LEN+1],*p;

    ReadLine(sl);
    p=sl;
    while(*p==0x20) p++;
    if ((*p<'0' || *p>'9') && *p!='.' && *p!='-') {
        bStatus=FALSE;
        return 0.0;
    }
    return atof(p);
}


void DXFGroupReader::ReadS(char * ptgt)
{
    ReadLine(ptgt);
}


