/*************************************************************************
 *
 *  $RCSfile: ddemldeb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:05 $
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

#include "ddemlimp.hxx"

#ifdef OV_DEBUG
#include <svgen.hxx>
#endif

#if defined(OV_DEBUG)

void ImpAddHSZ( HSZ hszString, String& rStr )
{
    char aBuf[ 128 ];
    ImpDdeMgr::DdeQueryString( hszString,aBuf,sizeof(aBuf),850);
    rStr += " (\""; rStr += aBuf; rStr += "\",";
    HATOMTBL hAtomTable = WinQuerySystemAtomTable();
    ULONG nRefCount = 0;
    if( hszString )
        nRefCount = WinQueryAtomUsage(hAtomTable, (ATOM)hszString );
    rStr += nRefCount; rStr += ')';
}


void ImpWriteDdeStatus(char* aFilename, char* pAppContext)
{
    char aBuf[ 128 ];
    USHORT nCtr;
    HWND* pAppPtr;
    ImpHCONV* pConvPtr;
    Transaction* pTransPtr;

    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if( !pData )
        return;
    SvFileStream aStrm(aFilename, STREAM_READWRITE );
    String aLine;
    aStrm.Seek( STREAM_SEEK_TO_END );
    aStrm << endl;
    aStrm.WriteLine("********************** DDEML-Log ***********************");
    aStrm << endl;
    if( pAppContext )
    {
        aLine = Application::GetAppName();
        aLine += ':';
        aLine += "App-Context:"; aLine += pAppContext;
        aStrm.WriteLine( aLine ); aStrm << endl;
    }
    aStrm.WriteLine("----------------- ImpDdeMgrData -------------------");
    aStrm << endl;
    aLine= "TotalSize      :"; aLine+= pData->nTotalSize; aStrm.WriteLine(aLine);
    aLine= "nOffsAppTable  :"; aLine+= pData->nOffsAppTable; aStrm.WriteLine(aLine);
    aLine= "nOffsConvTable :"; aLine+= pData->nOffsConvTable; aStrm.WriteLine(aLine);
    aLine= "nOffsTransTable:"; aLine+= pData->nOffsTransTable; aStrm.WriteLine(aLine);
    aLine= "nMaxAppCount   :"; aLine+= pData->nMaxAppCount; aStrm.WriteLine(aLine);
    aLine= "nMaxConvCount  :"; aLine+= pData->nMaxConvCount; aStrm.WriteLine(aLine);
    aLine= "nMaxTransCount :"; aLine+= pData->nMaxTransCount; aStrm.WriteLine(aLine);
    aLine= "nLastErr       :"; aLine+= pData->nLastErr; aStrm.WriteLine(aLine);
    aLine= "nCurConvCount  :"; aLine+= pData->nCurConvCount; aStrm.WriteLine(aLine);
    aLine= "nCurTransCount :"; aLine+= pData->nCurTransCount; aStrm.WriteLine(aLine);
    aStrm << endl;
    aStrm.WriteLine("---------- Registered DDEML-Applications -----------");
    aStrm << endl;
    pAppPtr = ImpDdeMgr::GetAppTable( pData );
    for( nCtr = 0; nCtr < pData->nMaxAppCount; nCtr++, pAppPtr++ )
    {
        if( *pAppPtr )
        {
            aLine = "App."; aLine += nCtr; aLine += " HWND:";
            aLine += (ULONG)*pAppPtr; aStrm.WriteLine(aLine);
        }
    }

    aStrm << endl;
    aStrm.WriteLine("-------------- Conversation handles ----------------");
    aStrm << endl;

    USHORT nCurCount = pData->nCurConvCount;

    if( nCurCount )
    {
    pConvPtr = ImpDdeMgr::GetConvTable( pData );
    for( nCtr = 0; nCtr < pData->nMaxConvCount; nCtr++, pConvPtr++ )
    {
        if( pConvPtr->hWndThis )
        {
            aLine = "HCONV:";  aLine += nCtr;
            aLine += " HCONVpartner: "; aLine += (USHORT)pConvPtr->hConvPartner;
            if( !pConvPtr->hConvPartner ) aLine += "(Non-DDEML-App)";
            aLine += " hszPartner: "; aLine += (USHORT)pConvPtr->hszPartner;
            ImpAddHSZ( pConvPtr->hszPartner, aLine );
            aStrm.WriteLine( aLine );

            aLine = "hszService: "; aLine += (USHORT)pConvPtr->hszServiceReq;
            ImpAddHSZ( pConvPtr->hszServiceReq, aLine );
            aLine += " hszTopic: "; aLine += (USHORT)pConvPtr->hszTopic;
            ImpAddHSZ( pConvPtr->hszTopic, aLine );
            aStrm.WriteLine( aLine );

            aLine= "Status: "; aLine+= pConvPtr->nStatus;
            if( pConvPtr->nStatus & ST_CLIENT ) aLine += " (Client)";
            if( pConvPtr->nStatus & ST_INLIST ) aLine += " (Inlist)";
            aStrm.WriteLine(aLine);

            aLine = "pidOwner: "; aLine += (ULONG)pConvPtr->pidOwner;
            aStrm.WriteLine( aLine );
            aLine = "hWndThis: "; aLine += (ULONG)pConvPtr->hWndThis;
            aStrm.WriteLine( aLine );
            aLine = "hWndPartner: "; aLine += (ULONG)pConvPtr->hWndPartner;
            aStrm.WriteLine( aLine );

            aLine = "hConvList: "; aLine += (ULONG)pConvPtr->hConvList;
            aLine += " Prev: "; aLine += pConvPtr->nPrevHCONV;
            aLine += " Next: "; aLine += pConvPtr->nNextHCONV;
            aStrm.WriteLine( aLine );
            aStrm.WriteLine("----------------------------------------------------");

            nCurCount--;
            if( !nCurCount )
                break;
        }
    }
    }

    aStrm.WriteLine("----------------- Transaction Ids ------------------");

    nCurCount = pData->nCurTransCount;
    if( nCurCount )
    {
    pTransPtr = ImpDdeMgr::GetTransTable( pData );
    for( nCtr = 0; nCtr < pData->nMaxTransCount; nCtr++, pTransPtr++ )
    {

        if( pTransPtr->hConvOwner )
        {
            aLine = "TransactionId:"; aLine += nCtr;
            aLine += " hConvOwner: "; aLine += (USHORT)pTransPtr->hConvOwner;
            aStrm.WriteLine( aLine );
            aLine = "Item: "; aLine += (USHORT)pTransPtr->hszItem;
            ImpAddHSZ( pTransPtr->hszItem, aLine );
            aLine += " Format: "; aLine += pTransPtr->nFormat;
            aStrm.WriteLine( aLine );
            aLine = "TransactionType: "; aLine += pTransPtr->nType;
            aLine += " Convst: "; aLine += pTransPtr->nConvst;
            aLine += " LastErr: "; aLine += pTransPtr->nLastError;
            aLine += " Userhandle: "; aLine += pTransPtr->nUser;
            aStrm.WriteLine( aLine );
            aStrm.WriteLine("--------------------------------------------------");

            nCurCount--;
            if( !nCurCount )
                break;
        }
    }
    }
    aStrm << endl;
    aStrm.WriteLine("******************* End of DDEML-Log *******************");
}

void ImpWriteDdeData(char* aFilename, DDESTRUCT* pData)
{
    char aBuf[ 128 ];
    USHORT nCtr;
    SvFileStream aStrm(aFilename, STREAM_READWRITE );
    aStrm.Seek( STREAM_SEEK_TO_END );
    String aLine;
    aStrm << endl;
    aLine = "cbData:"; aLine += pData->cbData; aStrm.WriteLine( aLine );
    aLine = "fsStatus:"; aLine += pData->fsStatus; aStrm.WriteLine( aLine );
    aLine = "usFormat:"; aLine += pData->usFormat; aStrm.WriteLine( aLine );
    aLine = "ItemName:"; aLine += (char*)((char*)pData+pData->offszItemName);
    aStrm.WriteLine( aLine );
    aLine = "offabData:"; aLine += pData->offabData; aStrm.WriteLine(aLine);
    char* pBuf = (char*)pData+pData->offabData;
    USHORT nLen = pData->cbData; // - pData->offabData;
    while( nLen )
    {
        aStrm << *pBuf;
        nLen--;
        pBuf++;
    }
    aStrm << endl;
}

void ImpWriteLogFile(char* pFilename, char* pStr)
{
    SvFileStream aStrm(pFilename, STREAM_READWRITE );
    aStrm.Seek( STREAM_SEEK_TO_END );
    String aStr( Application::GetAppName() );
    aStr += ':'; aStr += pStr;
    aStrm.WriteLine( (char*)aStr.GetStr() );
}

#else

void ImpWriteDdeStatus(char*, char* ) {}
void ImpWriteDdeData(char*, DDESTRUCT*) {}
void ImpWriteLogFile(char*, char*) {}

#endif

APIRET MyDosAllocSharedMem(void** ppBaseAddress, char* pszName, unsigned long ulObjectSize,
    unsigned long ulFlags, char* pContextStr )
{
    APIRET nRet = DosAllocSharedMem(ppBaseAddress,pszName,ulObjectSize,ulFlags );
#if 0 && defined(OV_DEBUG) && defined(LOGFILE)
    String aStr("DosAllocSharedMem:");
    aStr += pContextStr;
    aStr += ": ";
    aStr += ulObjectSize;
    aStr += " (";
    aStr += (ULONG)*((char**)ppBaseAddress);
    aStr += ')';
    ImpWriteLogFile("\\ddeml.mem", (char*)aStr.GetStr() );
#endif
    return nRet;
}

APIRET MyDosAllocMem(void** ppBaseAddress, unsigned long ulObjectSize,
    unsigned long ulFlags, char* pContextStr )
{
    APIRET nRet = DosAllocMem(ppBaseAddress, ulObjectSize,ulFlags );
#if 0 && defined(OV_DEBUG) && defined(LOGFILE)
    String aStr("DosAllocMem:");
    aStr += pContextStr;
    aStr += ": ";
    aStr += ulObjectSize;
    aStr += " (";
    aStr += (ULONG)*((char**)ppBaseAddress);
    aStr += ')';
    ImpWriteLogFile("\\ddeml.mem", (char*)aStr.GetStr() );
#endif
    return nRet;
}


APIRET MyDosFreeMem( void* pBaseAddress, char* pContextStr )
{
    APIRET nRet = DosFreeMem( pBaseAddress );
#if 0 && defined(OV_DEBUG) && defined(LOGFILE)
    String aStr("DosFreeMem:");
    aStr += pContextStr;
    aStr += ": ";
    aStr += (ULONG)pBaseAddress;
    ImpWriteLogFile("\\ddeml.mem", (char*)aStr.GetStr());
#endif
    return nRet;
}





