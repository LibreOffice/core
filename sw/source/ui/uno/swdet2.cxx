/*************************************************************************
 *
 *  $RCSfile: swdet2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:48:25 $
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

#define _SWLIB_CXX

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PARHTML_HXX //autogen
#include <svtools/parhtml.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif

#ifndef _SHELLIO_HXX //autogen
#include <shellio.hxx>
#endif

#undef _DLL_

#include <swdetect.hxx>
#include "iodetect.hxx"

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>

#include <svtools/moduleoptions.hxx>

#define C2S(cChar) String::CreateFromAscii(cChar)

USHORT AutoDetec( const String& FileName, USHORT & rVersion );

inline BOOL IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

IO_DETECT_IMPL1
/*
IO_DETECT_IMPL2
IO_DETECT_IMPL3
IO_DETECT_IMPL4
 */

extern char __FAR_DATA sHTML[];

const sal_Char* SwIoDetect::IsReader(const sal_Char* pHeader, ULONG nLen,
    const String &rFileName) const
{
    int bRet = FALSE;
    if( sHTML == pName )
        bRet = HTMLParser::IsHTMLFormat( pHeader, TRUE, RTL_TEXTENCODING_DONTKNOW );
    else if( FILTER_SWG == pName )
        bRet = 0 == strncmp( FILTER_SWG, pHeader, 3 ) &&
                '1' != *(pHeader + 3);
    else if( sSwg1 == pName )
        bRet = 0 == strncmp( FILTER_SWG, pHeader, 3 ) &&
                '1' == *(pHeader + 3);
    else if( FILTER_RTF == pName )
        bRet = 0 == strncmp( "{rtf", pHeader, 5 );
    else if( sLotusD == pName )
        bRet = 0 == *pHeader++ && 0 == *pHeader++ &&
                2 == *pHeader++ && 0 == *pHeader++ &&
                ( 4 == *pHeader || 6 == *pHeader ) && 4 == *++pHeader;
    else if( sExcel == pName )
    {
        if( 0x09 == *pHeader++ )
        {
            if( 0x00 == *pHeader )
                bRet = 0x04 == *++pHeader && 0 == *++pHeader;
            else if( 0x02 == *pHeader || 0x04 == *pHeader )
                bRet = 0x06 == *++pHeader && 0 == *++pHeader;
        }
    }
    else if( sWW5 == pName )
    {
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA5DC
                 && ((W1_FIB*)pHeader)->nFibGet() == 0x65 )
                 /*&& ((W1_FIB*)pHeader)->fComplexGet() == 0*/);
    }
    else if( sWW1 == pName )
    {
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA59C
                 && ((W1_FIB*)pHeader)->nFibGet() == 0x21
                 WW2B ) && ((W1_FIB*)pHeader)->fComplexGet() == 0);
    }
    else if( sSwDos == pName )
    {
        sal_Char __READONLY_DATA sSw6_FormatStt[] =         ".\\\\\\ WRITER ";
        sal_Char __READONLY_DATA sSw6_FormatEnd[] =         " \\\\\\";

        bRet = 0 == strncmp( sSw6_FormatStt, pHeader, 12 ) &&
                  0 == strncmp( sSw6_FormatEnd, pHeader + 12 + 1, 4 );
    }
    else if (FILTER_TEXT == pName)
        bRet = SwIoSystem::IsDetectableText(pHeader, nLen);
    else if (FILTER_W4W == pName)
    bRet = SwIoSystem::IsDetectableW4W(rFileName);
    return bRet ? pName : 0;
}

const String SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )
{
    /* bei den StorageFiltern noch den SubStorageNamen setzen */
    const String& rUserData = rFltr.GetUserData();
    if( rUserData.EqualsAscii(FILTER_SW5) || rUserData.EqualsAscii(FILTER_SW5V) ||
        rUserData.EqualsAscii(FILTER_SWW5V) ||
        rUserData.EqualsAscii(FILTER_SW4 )|| rUserData.EqualsAscii(FILTER_SW4V) ||
        rUserData.EqualsAscii(FILTER_SWW4V) ||
        rUserData.EqualsAscii(FILTER_SW3 )|| rUserData.EqualsAscii(FILTER_SW3V) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "StarWriterDocument" ));
    if( rUserData.EqualsAscii(FILTER_XML) ||
        rUserData.EqualsAscii(FILTER_XMLV) ||
        rUserData.EqualsAscii(FILTER_XMLVW) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "content.xml" ));
    if( rUserData.EqualsAscii(sWW6) || rUserData.EqualsAscii(FILTER_WW8) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "WordDocument" ));
    if( rUserData.EqualsAscii(sExcel) || rUserData.EqualsAscii(sCExcel) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "Book" ));
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "" ));
}
const SfxFilter* SwIoSystem::GetFilterOfFormat( const String& rFmtNm,
                                const SfxFilterContainer* pCnt )
{
    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer* pFltCnt = IsDocShellRegistered ? &aCntSw : &aCntSwWeb;

    do {
        if( pFltCnt )
        {
            const SfxFilter* pFilter;
            USHORT nCount = pFltCnt->GetFilterCount();
            for( USHORT i = 0; i < nCount; ++i )
                if( ( pFilter = pFltCnt->GetFilter( i ))->GetUserData() == rFmtNm )
                    return pFilter;
        }
        if( pCnt || pFltCnt == &aCntSwWeb )
            break;
        pFltCnt = &aCntSwWeb;
    } while( TRUE );

    return 0;
}

FASTBOOL SwIoSystem::IsValidStgFilter( SotStorage& rStg, const SfxFilter& rFilter )
{
    ULONG nStgFmtId = rStg.GetFormat();
    /*#i8409# We cannot trust the clipboard id anymore :-(*/
    if( rFilter.GetUserData().EqualsAscii(FILTER_WW8) ||
        rFilter.GetUserData().EqualsAscii(sWW6) )
    {
        nStgFmtId = 0;
    }
    BOOL bRet = SVSTREAM_OK == rStg.GetError() &&
                ( !nStgFmtId || rFilter.GetFormat() == nStgFmtId ) &&
                ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) ||
                  (rFilter.GetUserData().EqualsAscii(FILTER_XML) &&
                   rStg.IsContained( String::CreateFromAscii("Content.xml") )) );
    if( bRet )
    {
        /* Bug 53445 - es gibt Excel Docs ohne ClipBoardId! */
        /* Bug 62703 - und auch WinWord Docs ohne ClipBoardId! */
        if( rFilter.GetUserData().EqualsAscii(FILTER_WW8) ||
            rFilter.GetUserData().EqualsAscii(sWW6) )
        {
            bRet = !((rStg.IsContained( String::CreateFromAscii("0Table" )) ||
                    rStg.IsContained( String::CreateFromAscii("1Table" ))) ^
                       rFilter.GetUserData().EqualsAscii(FILTER_WW8));
            if (bRet && !rFilter.IsAllowedAsTemplate())
            {
                SotStorageStreamRef xRef =
                    rStg.OpenSotStream(String::CreateFromAscii("WordDocument"),
                    STREAM_STD_READ | STREAM_NOCREATE );
                xRef->Seek(10);
                BYTE nByte;
                *xRef >> nByte;
                bRet = !(nByte & 1);
            }
        }
        else if( rFilter.GetUserData().EqualsAscii(FILTER_XML, 0, sizeof(FILTER_XML)-1) )
            bRet = !nStgFmtId || rFilter.GetFormat() == nStgFmtId;
        else if( !rFilter.GetUserData().EqualsAscii(sCExcel) )
            bRet = rFilter.GetFormat() == nStgFmtId;
    }
    return bRet;
}

    /* Feststellen ob das File in dem entsprechenden Format vorliegt. */
    /* Z.z werden nur unsere eigene Filter unterstuetzt               */
FASTBOOL SwIoSystem::IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
                                    const SfxFilter** ppFilter )
{
    FASTBOOL bRet = FALSE;
    const SfxFilter* pFltr;
    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer& rFltContainer = IsDocShellRegistered ? aCntSw : aCntSwWeb;
    USHORT nFltCount = rFltContainer.GetFilterCount();
    SotStorageRef xStg;
    if( rMedium.IsStorage() )
         xStg = rMedium.GetStorage();
    for( USHORT n = 0; n < nFltCount; ++n )
        if( ( pFltr = rFltContainer.GetFilter( n ))->GetUserData() == rFmtName )
        {
            if( 'C' == *pFltr->GetUserData().GetBuffer() )
            {
                bRet = xStg.Is() && IsValidStgFilter( *xStg, *pFltr );
            }
            else if( !xStg.Is() )
            {
                SvStream* pStrm = rMedium.GetInStream();
                if( pStrm && !pStrm->GetError() )
                {
                    sal_Char aBuffer[ 4097 ];
                    ULONG nBytesRead = pStrm->Read( aBuffer, 4096 );
                    pStrm->Seek( STREAM_SEEK_TO_BEGIN );
                    if( nBytesRead<=80 )
                    {
                        aBuffer[nBytesRead] = '\0';
                        aBuffer[nBytesRead+1] = '\0';
                        if( (nBytesRead & 0x00000001) != 0 )
                            aBuffer[nBytesRead+2] = '\0';
                    }

                    for( USHORT i = 0; i < MAXFILTER; ++i )
                        if( aReaderWriter[i].IsFilter( rFmtName ) )
                        {
                            bRet = 0 != aReaderWriter[i].IsReader( aBuffer, nBytesRead, rMedium.GetPhysicalName() );
                            break;
                        }
                }
            }

            if( bRet && ppFilter  )
                *ppFilter = pFltr;

            break;
        }

    return bRet;
}


#ifdef __LITTLEENDIAN
#   define ENDHACK bool bNativeLE = true;
#else
#   define ENDHACK bool bNativeLE = false;
#endif


/* die Methode stellt fest, von welchem Typ der stream (File) ist.        */
/* Es wird versucht, eine dem Filter entsprechende Byte-Folge zu finden.  */
/* Wird kein entsprechender gefunden, wird zur Zeit der ASCII-Reader      */
/* returnt !! Der Returnwert ist der interne Filtername!                  */
/* rPrefFltName ist der interne Name des Filters, den der Benutzer im     */
/* Open-Dialog eingestellt hat.                                           */
const SfxFilter* SwIoSystem::GetFileFilter( const String& rFileName,
                                             const String& rPrefFltName,
                                             SfxMedium* pMedium )
{
    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer* pFCntnr = IsDocShellRegistered ? &aCntSw : &aCntSwWeb;

    USHORT nFltrCount;
    if( !pFCntnr || 0 == ( nFltrCount = pFCntnr->GetFilterCount() ) )
        return 0;

    const SfxFilter* pFilter;
    if( pMedium ? pMedium->IsStorage()
                : SotStorage::IsStorageFile( rFileName ) )
    {
        /* Storage: Suchen nach einem Sub-Storage, dessen Name  */
        /* dem in einem Filter stehenden DLL-Namen entspricht   */
        SotStorageRef xStg;
        if( pMedium )
            xStg = pMedium->GetStorage();
        else
            xStg = new SotStorage( rFileName, STREAM_STD_READ );

        if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )
        {
            USHORT nCnt;
            for( nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( 'C' == *( pFilter = pFCntnr->GetFilter( nCnt ))->
                    GetUserData().GetBuffer() &&
                    IsValidStgFilter( *xStg, *pFilter ))
                    return pFilter;

            if( IsDocShellRegistered() && 0 != ( pFCntnr = &aCntSwWeb ) &&
                0 != ( nFltrCount = pFCntnr->GetFilterCount() ) )
                for( nCnt = 0; nCnt < nFltrCount; ++nCnt )
                    if( 'C' == *( pFilter = pFCntnr->GetFilter( nCnt ))->
                        GetUserData().GetBuffer() &&
                        IsValidStgFilter( *xStg, *pFilter ))
                        return pFilter;
        }
        return 0;
    }

    sal_Char aBuffer[ 4098 ];
    ULONG nBytesRead;
    if( pMedium )
    {
        SvStream* pIStrm = pMedium->GetInStream();
        if( !pIStrm || SVSTREAM_OK != pIStrm->GetError() )
            return 0;
        ULONG nCurrPos = pIStrm->Tell();
        nBytesRead = pIStrm->Read( aBuffer, 4096 );
        pIStrm->Seek( nCurrPos );
    }
    else
    {
        SvFileStream aStrm( rFileName, STREAM_READ );

        /* ohne FileName oder ohne Stream gibts nur den ANSI-Filter */
        if( !rFileName.Len() || SVSTREAM_OK != aStrm.GetError() )
            return 0;

        nBytesRead = aStrm.Read( aBuffer, 4096 );
        aStrm.Close();
    }
    DBG_ASSERT( nBytesRead<=4096, "zu viele Bytes gelesen?" );
    if( nBytesRead <= 4096 )
    {
        aBuffer[nBytesRead] = '\0';
        aBuffer[nBytesRead+1] = '\0';
        if( (nBytesRead & 0x00000001) != 0 )
            aBuffer[nBytesRead+2] = '\0';
    }

    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    /* suche nach dem bestimmten Filter, falls kein entsprechender        */
    /* gefunden wird, so wird der ASCII-Filter returnt.                   */
    /* Gibt es Filter ohne einen Identifizierungs-String, so werden diese */
    /* nie erkannt und es wird auch der ASCII-Filter returnt.             */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    {
        const SfxFilter* pFilter;
        const sal_Char* pNm;
        for( USHORT n = 0; n < MAXFILTER; ++n )
            if( 0 != ( pNm = aReaderWriter[n].IsReader(aBuffer, nBytesRead, rFileName)) &&
                0 != ( pFilter =  SwIoSystem::GetFilterOfFormat(
                                String::CreateFromAscii(pNm), pFCntnr )))
                return pFilter;
    }

    /* Ok, bis jetzt kein Filter gefunden, also befrage mal die */
    /* "WORD 4 WORD" Filter                                     */
    if( rFileName.Len() )
    {
        if( pMedium )
            pMedium->CloseInStream();
        USHORT nVersion, nW4WId = AutoDetec( rFileName, nVersion );

        if( 1 < nW4WId )
        {
            String aW4WName( String::CreateFromAscii(FILTER_W4W ));
            if( nW4WId < 10 )
                aW4WName += '0';
            aW4WName += String::CreateFromInt32(nW4WId);
            aW4WName += '_';
            aW4WName += String::CreateFromInt32(nVersion);

            for( USHORT nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( 0 == ( pFilter = pFCntnr->GetFilter( nCnt ))->
                    GetUserData().Search( aW4WName ))
                    return pFilter;

            W4W_CHECK_FOR_INTERNAL_FILTER
            W4W_FILTER_NOT_FOUND
            return 0;
        }
    }
    return SwIoSystem::GetTextFilter( aBuffer, nBytesRead);
}

bool SwIoSystem::IsDetectableText(const sal_Char* pBuf, ULONG &rLen,
    CharSet *pCharSet, bool *pSwap, LineEnd *pLineEnd)
{
    bool bSwap = false;
    CharSet eCharSet = RTL_TEXTENCODING_DONTKNOW;
    bool bLE = true;
    ULONG nHead=0;
    /*See if its a known unicode type*/
    if (rLen >= 2)
    {
        if (rLen > 2 && BYTE(pBuf[0]) == 0xEF && BYTE(pBuf[1]) == 0xBB &&
            BYTE(pBuf[2]) == 0xBF)
        {
            eCharSet = RTL_TEXTENCODING_UTF8;
            nHead = 3;
        }
        else if (BYTE(pBuf[0]) == 0xFE && BYTE(pBuf[1]) == 0xFF)
        {
            eCharSet = RTL_TEXTENCODING_UCS2;
            bLE = false;
            nHead = 2;
        }
        else if (BYTE(pBuf[1]) == 0xFE && BYTE(pBuf[0]) == 0xFF)
        {
            eCharSet = RTL_TEXTENCODING_UCS2;
            nHead = 2;
        }
        pBuf+=nHead;
        rLen-=nHead;
    }

    bool bCR = false, bLF = false, bNoNormalChar = false,
        bIsBareUnicode = false;

    if (eCharSet != RTL_TEXTENCODING_DONTKNOW)
    {
        String sWork;
        sal_Unicode *pNewBuf = sWork.AllocBuffer(rLen);
        sal_Size nNewLen;
        if (eCharSet != RTL_TEXTENCODING_UCS2)
        {
            nNewLen = rLen;
            rtl_TextToUnicodeConverter hConverter =
                rtl_createTextToUnicodeConverter(eCharSet);
            rtl_TextToUnicodeContext hContext =
                rtl_createTextToUnicodeContext(hConverter);

            sal_Size nCntBytes;
            sal_uInt32 nInfo;
            nNewLen = rtl_convertTextToUnicode( hConverter, hContext, pBuf,
                rLen, pNewBuf, nNewLen,
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                  RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                  RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT), &nInfo, &nCntBytes);

            rtl_destroyTextToUnicodeContext(hConverter, hContext);
            rtl_destroyTextToUnicodeConverter(hConverter);
        }
        else
        {
            nNewLen = rLen/2;
            memcpy(pNewBuf, pBuf, rLen);
            ENDHACK
            if (bLE != bNativeLE)
            {
                bSwap = true;
                sal_Char* pF = (sal_Char*)pNewBuf;
                sal_Char* pN = pF+1;
                for(xub_StrLen n = 0; n < nNewLen; ++n, pF+=2, pN+=2)
                {
                    sal_Char c = *pF;
                    *pF = *pN;
                    *pN = c;
                }
            }
        }

        sWork.ReleaseBufferAccess(nNewLen);
        pNewBuf = sWork.GetBufferAccess();

        for (ULONG nCnt = 0; nCnt < nNewLen; ++nCnt, ++pNewBuf)
        {
            switch (*pNewBuf)
            {
                case 0xA:
                    bLF = true;
                    break;
                case 0xD:
                    bCR = true;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        for( ULONG nCnt = 0; nCnt < rLen; ++nCnt, ++pBuf )
        {
            switch (*pBuf)
            {
                case 0x0:
                    if( nCnt + 1 < rLen && !*(pBuf+1) )
                        return 0;
                    bIsBareUnicode = true;
                    break;
                case 0xA:
                    bLF = true;
                    break;
                case 0xD:
                    bCR = true;
                    break;
                case 0xC:
                case 0x1A:
                case 0x9:
                    break;
                default:
                    if (0x20 > (BYTE)*pBuf)
                        bNoNormalChar = true;
                    break;
            }
        }
    }

    LineEnd eSysLE = GetSystemLineEnd();
    LineEnd eLineEnd;
    if (!bCR && !bLF)
        eLineEnd = eSysLE;
    else
        eLineEnd = bCR ? ( bLF ? LINEEND_CRLF : LINEEND_CR ) : LINEEND_LF;

    if (pCharSet)
        *pCharSet = eCharSet;
    if (pSwap)
        *pSwap = bSwap;
    if (pLineEnd)
        *pLineEnd = eLineEnd;

    return (!bIsBareUnicode && eSysLE == eLineEnd);
}


const SfxFilter* SwIoSystem::GetTextFilter( const sal_Char* pBuf, ULONG nLen)
{
    bool bAuto = IsDetectableText(pBuf, nLen);
    const sal_Char* pNm = bAuto ? FILTER_TEXT : FILTER_TEXT_DLG;
    return SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(pNm), 0 );
}


bool SwIoSystem::IsDetectableW4W(const String& rFileName)
{
    bool bRet(false);
    if (rFileName.Len())
    {
        USHORT nVersion, nW4WId = AutoDetec( rFileName, nVersion );
        if (nW4WId > 1)
            bRet = true;
    }
    return bRet;
}


//-------------------------------------------------------------------------

ULONG SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    SfxFilterFlags nMust = SFX_FILTER_IMPORT;
    SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;
    ULONG nRet = ERRCODE_NONE;
    const SfxFilter* pSavFilter = *ppFilter;

    do {
            // dann ueberpruefe mal ob der richtige ausgewaehlt wurde
        if( *ppFilter )
        {
            const String& rUData = (*ppFilter)->GetUserData();

            BOOL bTxtFilter = rUData.EqualsAscii( FILTER_TEXT, 0, 4 );

            if (SwIoSystem::IsFileFilter( rMedium, rUData ) && !bTxtFilter)
                break;

            //JP 08.06.98: Bugfix 50498
            if (bTxtFilter)
            {
                //JP 09.11.98: der SWDOS - Filter hat dieselbe Extension und
                // wird vom SFX vorgeschlagen. Das es auch eine Textdatei ist,
                // muss die hier ausgefilter werden!
                if (SwIoSystem::IsFileFilter( rMedium, C2S("SW6"), ppFilter))
                    break;
            }
        }

        if( SFX_FILTER_TEMPLATE & nMust )
        {
            // nur einen Vorlagen Filter
            BOOL bStorage = rMedium.IsStorage();
            if( bStorage && *ppFilter )
                break;
            else if( bStorage &&
                ( SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_XMLV), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW5V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW4V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SW3V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_XMLVW), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWW5V), ppFilter ) ||
                  SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWW4V), ppFilter ) ))
                break;
            else if( !bStorage &&
                SwIoSystem::IsFileFilter( rMedium, C2S(FILTER_SWGV), ppFilter ) )
                break;

            nRet = ERRCODE_ABORT;
            break;
        }


        String aPrefFlt;
        if( *ppFilter )
        {
            aPrefFlt = (*ppFilter)->GetUserData();
            if( SwIoSystem::IsFileFilter( rMedium, aPrefFlt ) )
            {
                nRet = ERRCODE_NONE;
                break;
            }

            // beim Browsen soll keine Filterbox kommen, wenn das Dokument nicht
            // in den ersten paar Bytes HTML-Tags hat (MA/ST/...). Solche Dok.
            // erzeugen z.B. SearchEngines
//JP 20.07.00: from now on we are not a browser
//          else if( aPrefFlt == C2S(sHTML) )
//          {
//              nRet = ERRCODE_NONE;
//              break;
//          }
        }

        const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(),
                                                            aPrefFlt, &rMedium );
        if( !pTmp )
            nRet = ERRCODE_ABORT;


        else if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( "W4W", 0, 3 )
                    && pTmp->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ) )
        {
            // Bug 95262 - if the user (or short  detect) select a
            //              Word 4 Word filter, but the autodect of mastersoft
            //              can't detect it, we normally return the ascii filter
            //              But the user may have a change to use the W4W filter,
            //              so the SFX must show now a dialog with the 2 filters
            nRet = ERRCODE_SFX_CONSULTUSER;
            *ppFilter = pTmp;
        }
        // sollte der voreingestellte Filter ASCII sein und wir haben
        // ASCII erkannt, dann ist das ein gultiger Filter, ansonsten ist das
        // ein Fehler und wir wollen die Filterbox sehen
        else if( pTmp->GetUserData().EqualsAscii( FILTER_TEXT ))
        {
            // Bug 28974: "Text" erkannt, aber "Text Dos" "Text ..." eingestellt
            //  -> keine FilterBox, sondern den eingestellten Filter benutzen
            if( *ppFilter && (*ppFilter)->GetUserData().EqualsAscii( FILTER_TEXT, 0, 4 ))
                ;
            else
//          if( !*ppFilter || COMPARE_EQUAL != pTmp->GetUserData().Compare(
//              (*ppFilter)->GetUserData(), 4 ))
            {
//              nRet = ERRCODE_ABORT;
                *ppFilter = pTmp;
            }
        }
        else
        {
            //Bug 41417: JP 09.07.97: HTML auf die WebDocShell defaulten
            if( pTmp->GetUserData() != C2S(sHTML) ||
                String::CreateFromAscii( "com.sun.star.text.WebDocument" ) ==
                String( pTmp->GetServiceName() ) ||
                0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( C2S(sHTML),
                     &SfxFilterContainer(String::CreateFromAscii("swriter/web") ) ) ) )
                *ppFilter = pTmp;
        }

    } while( FALSE );

    if( ERRCODE_NONE == nRet && (
        nMust != ( (*ppFilter)->GetFilterFlags() & nMust ) ||
        0 != ( (*ppFilter)->GetFilterFlags() & nDont )) )
    {
        nRet = ERRCODE_ABORT;
        *ppFilter = pSavFilter;
    }
    return nRet;
}

//-------------------------------------------------------------------------

ULONG SwFilterDetect::GlobDetectFilter( SfxMedium& rMedium, const SfxFilter **ppFilter )
{
    SfxFilterFlags nMust = SFX_FILTER_IMPORT;
    SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED;
    ULONG nRet = ERRCODE_ABORT;
    const SfxFilter* pSavFilter = *ppFilter;
    do {
            // dann ueberpruefe mal ob der richtige ausgewaehlt wurde
        if( rMedium.IsStorage() )
        {
            SvStorageRef aStg = rMedium.GetStorage();

            if( *ppFilter &&
                aStg.Is() && SVSTREAM_OK == aStg->GetError() &&
                SwIoSystem::IsValidStgFilter( *aStg, **ppFilter ))
            {
                nRet = ERRCODE_NONE;
                break;
            }

            if( SFX_FILTER_TEMPLATE & nMust )
                break;

            const SfxFilter* pFltr;
            const SfxFilterContainer aFltContainer( String::CreateFromAscii("swriter/global") );
            USHORT nFltrCount = aFltContainer.GetFilterCount();
            for( USHORT nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( (sal_Unicode)'C' == ( pFltr = aFltContainer.GetFilter( nCnt ))->GetUserData().GetChar(0) &&
                    aStg.Is() && SwIoSystem::IsValidStgFilter( *aStg, *pFltr ) )
                {
                    *ppFilter = pFltr;
                    nRet = ERRCODE_NONE;
                    break;
                }
        }

    } while( FALSE );

    if( ERRCODE_NONE == nRet && (
        nMust != ( (*ppFilter)->GetFilterFlags() & nMust ) ||
        0 != ( (*ppFilter)->GetFilterFlags() & nDont )) )
    {
        nRet = ERRCODE_ABORT;
        *ppFilter = pSavFilter;
    }

    return nRet;
}
