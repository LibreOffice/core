/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iodetect.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:52:42 $
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

#ifndef _IODETECT_CXX
#define _IODETECT_CXX

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/parhtml.hxx>

#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // for ASSERT
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifdef _DLL_
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#endif

using namespace com::sun::star;

class Reader;
USHORT AutoDetec(const String& FileName, USHORT & rVersion);
bool IsDocShellRegistered();
void GetWW8Writer( const String&, const String&, WriterRef& );

typedef void (*FnGetWriter)(const String&, const String& rBaseURL, WriterRef&);

struct SwIoDetect
{
// eigentlich privat, aber der Compiler kann sonst die
// Tabelle nicht initialisieren
    const sal_Char* pName;
    USHORT nLen;

    inline int IsFilter( const String& rNm )
    {
        return pName && rNm.EqualsAscii( pName, 0, nLen );
    }

#ifdef _DLL_
    Reader* pReader;
    FnGetWriter fnGetWriter;
    BOOL bDelReader;

    inline Reader* GetReader() const { return pReader; }
    inline void GetWriter( const String& rNm, const String& rBaseURL, WriterRef& xWrt ) const
        { if( fnGetWriter ) (*fnGetWriter)(rNm,rBaseURL,xWrt); else xWrt = WriterRef(0); }
#endif

    const sal_Char* IsReader(const sal_Char* pHeader, ULONG nLen_,
        const String &rFileName, const String& rUserData ) const;
};


#ifdef _DLL_
#define SwIoEntry(sNm, cCharLen, pWrt, bDel)    { sNm, cCharLen, 0, pWrt, bDel }
#else
#define SwIoEntry(sNm, cCharLen, pWrt, bDel)    { sNm, cCharLen }
#endif

#ifdef DEBUG_SH

#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel) , SwIoEntry(sNm, cCharLen, pWrt, bDel)
#else
#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel)
#endif

const USHORT MAXFILTER =
#ifdef DEBUG_SH
        1 +
#endif
        11;

const sal_Char __FAR_DATA FILTER_BAS[]  = "BAS";
const sal_Char __FAR_DATA FILTER_RTF[]  = "RTF";
const sal_Char __FAR_DATA FILTER_SWW4V[]    = "CSW4VWEB";
const sal_Char __FAR_DATA FILTER_SWW5V[]    = "CSW5VWEB";
const sal_Char __FAR_DATA sRtfWH[]      = "WH_RTF";
const sal_Char __FAR_DATA sHTML[]       = "HTML";
const sal_Char __FAR_DATA sWW1[]            = "WW1";
const sal_Char __FAR_DATA sWW5[]            = "WW6";
const sal_Char __FAR_DATA sWW6[]            = "CWW6";
const sal_Char __FAR_DATA FILTER_WW8[]  = "CWW8";
const sal_Char __FAR_DATA FILTER_TEXT_DLG[] = "TEXT_DLG";
const sal_Char __FAR_DATA FILTER_TEXT[]     = "TEXT";
const sal_Char __FAR_DATA sDebug[]      = "DEBUG";
const sal_Char __FAR_DATA sUndo[]       = "UNDO";
const sal_Char __FAR_DATA FILTER_XML[]  = "CXML";
const sal_Char __FAR_DATA FILTER_XMLV[]     = "CXMLV";
const sal_Char __FAR_DATA FILTER_XMLVW[]    = "CXMLVWEB";

#ifdef _DLL_
const sal_Char* GetFILTER_XML()
{
    return FILTER_XML;
}
const sal_Char* GetFILTER_WW8()
{
    return FILTER_WW8;
}
#endif

SwIoDetect aReaderWriter[ MAXFILTER ] =
{
///*  0*/ SwIoEntry(FILTER_SW5,       4,          &::GetSw3Writer,    TRUE),
///*  1*/ SwIoEntry(FILTER_SW4,       4,          &::GetSw3Writer,    FALSE),
///*  2*/ SwIoEntry(FILTER_SW3,       4,          &::GetSw3Writer,    FALSE),
///*  3*/ SwIoEntry(FILTER_SWG,       STRING_LEN, 0,                  TRUE),
///*  4*/ SwIoEntry(FILTER_SWGV,      4,          0,                  FALSE),
/*  5*/ SwIoEntry(FILTER_RTF,       STRING_LEN, &::GetRTFWriter,    TRUE),
///*  6*/ SwIoEntry(sSwDos,           STRING_LEN,   0,                  TRUE),
/*  7*/ SwIoEntry(FILTER_BAS,       STRING_LEN, &::GetASCWriter,    FALSE),
/*  8*/ SwIoEntry(sWW6,             STRING_LEN, &::GetWW8Writer,    TRUE),
/*  9*/ SwIoEntry(FILTER_WW8,       STRING_LEN, &::GetWW8Writer,    FALSE),
///* 10*/ SwIoEntry(FILTER_W4W,         3,          &::GetW4WWriter,    TRUE),
/* 11*/ SwIoEntry(sRtfWH,           STRING_LEN, &::GetRTFWriter,    FALSE),
///* 12*/ SwIoEntry(sCExcel,            5,          0,                  TRUE),
///* 13*/ SwIoEntry(sExcel,         4,          0,                  FALSE),
///* 14*/ SwIoEntry(sLotusD,            5,          0,                  TRUE),
/* 15*/ SwIoEntry(sHTML,            4,          &::GetHTMLWriter,   TRUE),
/* 16*/ SwIoEntry(sWW1,             STRING_LEN, 0,                  TRUE),
/* 17*/ SwIoEntry(sWW5,             STRING_LEN, 0,                  FALSE),
///* 18*/ SwIoEntry(sSwg1,            4,          0,                 FALSE),
/* 19*/ SwIoEntry(FILTER_XML,       4,          &::GetXMLWriter,    TRUE),
/*20*/ SwIoEntry(FILTER_TEXT_DLG, 8,          &::GetASCWriter,    TRUE),
/*last*/ SwIoEntry(FILTER_TEXT,     4,          &::GetASCWriter,    TRUE)
};

const char* pSw = "swriter";
const char* pSwWeb = "swriter/web";

// Filter erkennung
struct W1_FIB
{
    SVBT16 wIdent;      // 0x0 int magic number
    SVBT16 nFib;        // 0x2 FIB version written
    SVBT16 nProduct;    // 0x4 product version written by
    SVBT16 nlocale;     // 0x6 language stamp---localized version;
    SVBT16 pnNext;      // 0x8
    SVBT16 fFlags;

    USHORT nFibGet()    { return SVBT16ToShort(nFib); }
    USHORT wIdentGet()  { return SVBT16ToShort(wIdent); }
    USHORT fFlagsGet()  { return SVBT16ToShort(fFlags); }
    // SVBT16 fComplex :1;//        0004 when 1, file is in complex, fast-saved format.
    BOOL fComplexGet() { return static_cast< BOOL >((fFlagsGet() >> 2) & 1); }
};

const sal_Char* SwIoDetect::IsReader(const sal_Char* pHeader, ULONG nLen_,
    const String & /*rFileName*/, const String& /*rUserData*/) const
{
    int bRet = FALSE;
    if( sHTML == pName )
        bRet = HTMLParser::IsHTMLFormat( pHeader, TRUE, RTL_TEXTENCODING_DONTKNOW );
    else if( FILTER_RTF == pName )
        bRet = 0 == strncmp( "{\\rtf", pHeader, 5 );
    else if( sWW5 == pName )
    {
        W1_FIB *pW1Header = (W1_FIB*)pHeader;
        if (pW1Header->wIdentGet() == 0xA5DC && pW1Header->nFibGet() == 0x65)
            bRet = true; /*WW5*/
        else if (pW1Header->wIdentGet() == 0xA5DB && pW1Header->nFibGet() == 0x2D)
            bRet = true; /*WW2*/
    }
    else if( sWW1 == pName )
    {
        bRet = (( ((W1_FIB*)pHeader)->wIdentGet() == 0xA59C
                 && ((W1_FIB*)pHeader)->nFibGet() == 0x21)
                && ((W1_FIB*)pHeader)->fComplexGet() == 0);
    }
    else if (FILTER_TEXT == pName)
        bRet = SwIoSystem::IsDetectableText(pHeader, nLen_);
    else if( FILTER_TEXT_DLG == pName)
        bRet = SwIoSystem::IsDetectableText( pHeader, nLen_, 0, 0, 0, true);
    return bRet ? pName : 0;
}

const String SwIoSystem::GetSubStorageName( const SfxFilter& rFltr )
{
    /* bei den StorageFiltern noch den SubStorageNamen setzen */
    const String& rUserData = rFltr.GetUserData();
    if( rUserData.EqualsAscii(FILTER_XML) ||
        rUserData.EqualsAscii(FILTER_XMLV) ||
        rUserData.EqualsAscii(FILTER_XMLVW) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "content.xml" ));
    if( rUserData.EqualsAscii(sWW6) || rUserData.EqualsAscii(FILTER_WW8) )
        return String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( "WordDocument" ));
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "" ));
}
const SfxFilter* SwIoSystem::GetFilterOfFormat(const String& rFmtNm,
    const SfxFilterContainer* pCnt)
{
    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer* pFltCnt = pCnt ? pCnt : ( IsDocShellRegistered() ? &aCntSw : &aCntSwWeb );

    do {
        if( pFltCnt )
        {
            SfxFilterMatcher aMatcher( pFltCnt->GetName() );
            SfxFilterMatcherIter aIter( &aMatcher );
            const SfxFilter* pFilter = aIter.First();
            while ( pFilter )
            {
                if( pFilter->GetUserData() == rFmtNm )
                    return pFilter;
                pFilter = aIter.Next();
            }
        }
        if( pCnt || pFltCnt == &aCntSwWeb )
            break;
        pFltCnt = &aCntSwWeb;
    } while( TRUE );
    return 0;
}

BOOL SwIoSystem::IsValidStgFilter( const uno::Reference < embed::XStorage >& rStg, const SfxFilter& rFilter)
{
    BOOL bRet = FALSE;
    try
    {
        ULONG nStgFmtId = SotStorage::GetFormatID( rStg );
        bRet = rStg->isStreamElement( ::rtl::OUString::createFromAscii("content.xml") );
        if ( bRet )
            bRet = ( nStgFmtId && ( rFilter.GetFormat() == nStgFmtId ) );
    }
    catch ( uno::Exception& )
    {
    }

    return bRet;
}

BOOL SwIoSystem::IsValidStgFilter(SotStorage& rStg, const SfxFilter& rFilter)
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
                ( rStg.IsContained( SwIoSystem::GetSubStorageName( rFilter )) );
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
//      else if( !rFilter.GetUserData().EqualsAscii(sCExcel) )
//          bRet = rFilter.GetFormat() == nStgFmtId;
    }
    return bRet;
}

void TerminateBuffer(sal_Char *pBuffer, ULONG nBytesRead, ULONG nBufferLen)
{
    ASSERT(nBytesRead <= nBufferLen - 2,
        "what you read must be less than the max + null termination");
    ASSERT(!(nBufferLen & 0x00000001), "nMaxReadBuf must be an even number");
    if (nBytesRead <= nBufferLen - 2)
    {
        pBuffer[nBytesRead] = '\0';
        pBuffer[nBytesRead+1] = '\0';
        if (nBytesRead & 0x00000001)
            pBuffer[nBytesRead+2] = '\0';
    }
}

    /* Feststellen ob das File in dem entsprechenden Format vorliegt. */
    /* Z.z werden nur unsere eigene Filter unterstuetzt               */
BOOL SwIoSystem::IsFileFilter( SfxMedium& rMedium, const String& rFmtName,
                                    const SfxFilter** ppFilter )
{
    BOOL bRet = FALSE;

    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer& rFltContainer = IsDocShellRegistered() ? aCntSw : aCntSwWeb;

    uno::Reference < embed::XStorage > xStor;
    SotStorageRef xStg;
    if (rMedium.IsStorage())
        xStor = rMedium.GetStorage();
    else
    {
        SvStream* pStream = rMedium.GetInStream();
        if ( pStream && SotStorage::IsStorageFile(pStream) )
            xStg = new SotStorage( pStream, FALSE );
    }

    SfxFilterMatcher aMatcher( rFltContainer.GetName() );
    SfxFilterMatcherIter aIter( &aMatcher );
    const SfxFilter* pFltr = aIter.First();
    while ( pFltr )
    {
        if( pFltr->GetUserData() == rFmtName )
        {
            const String& rUserData = pFltr->GetUserData();
            if( 'C' == *rUserData.GetBuffer() )
            {
                if ( xStor.is() )
                    bRet = IsValidStgFilter( xStor, *pFltr );
                else if ( xStg.Is() )
                    bRet = xStg.Is() && IsValidStgFilter( *xStg, *pFltr );
                bRet = bRet && (pFltr->GetUserData() == rFmtName);
            }
            else if( !xStg.Is() && !xStor.is() )
            {
                SvStream* pStrm = rMedium.GetInStream();
                if( pStrm && !pStrm->GetError() )
                {
                    sal_Char aBuffer[4098];
                    const ULONG nMaxRead = sizeof(aBuffer) - 2;
                    ULONG nBytesRead = pStrm->Read(aBuffer, nMaxRead);
                    pStrm->Seek(STREAM_SEEK_TO_BEGIN);
                    TerminateBuffer(aBuffer, nBytesRead, sizeof(aBuffer));
                    for (USHORT i = 0; i < MAXFILTER; ++i)
                    {
                        if (aReaderWriter[i].IsFilter(rFmtName))
                        {
                            bRet = 0 != aReaderWriter[i].IsReader( aBuffer, nBytesRead,
                                                    rMedium.GetPhysicalName(), rUserData );
                            break;
                        }
                    }
                }
            }

            if( bRet && ppFilter  )
                *ppFilter = pFltr;
        }

        pFltr = aIter.Next();
    }

    return bRet;
}

/* die Methode stellt fest, von welchem Typ der stream (File) ist.        */
/* Es wird versucht, eine dem Filter entsprechende Byte-Folge zu finden.  */
/* Wird kein entsprechender gefunden, wird zur Zeit der ASCII-Reader      */
/* returnt !! Der Returnwert ist der interne Filtername!                  */
/* rPrefFltName ist der interne Name des Filters, den der Benutzer im     */
/* Open-Dialog eingestellt hat.                                           */
const SfxFilter* SwIoSystem::GetFileFilter(const String& rFileName,
    const String& rPrefFltName, SfxMedium* pMedium)
{
    SfxFilterContainer aCntSw( String::CreateFromAscii( pSw ) );
    SfxFilterContainer aCntSwWeb( String::CreateFromAscii( pSwWeb ) );
    const SfxFilterContainer* pFCntnr = IsDocShellRegistered() ? &aCntSw : &aCntSwWeb;

    if( !pFCntnr )
        return 0;

    SfxFilterMatcher aMatcher( pFCntnr->GetName() );
    SfxFilterMatcherIter aIter( &aMatcher );
    const SfxFilter* pFilter = aIter.First();
    if ( !pFilter )
        return 0;

    if( pMedium ? ( pMedium->IsStorage() || SotStorage::IsStorageFile( pMedium->GetInStream() ) ) : SotStorage::IsStorageFile( rFileName ) )
    {
        // package storage or OLEStorage based format
        SotStorageRef xStg;
        BOOL bDeleteMedium = FALSE;
        if (!pMedium )
        {
            INetURLObject aObj;
            aObj.SetSmartProtocol( INET_PROT_FILE );
            aObj.SetSmartURL( rFileName );
            pMedium = new SfxMedium( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READ, FALSE );
            bDeleteMedium = TRUE;
        }

        // templates should not get precedence over "normal" filters (#i35508, #i33168)
        const SfxFilter* pTemplateFilter = 0;
        const SfxFilter* pOldFilter = pFCntnr->GetFilter4FilterName( rPrefFltName );
        BOOL bLookForTemplate = pOldFilter && pOldFilter->IsOwnTemplateFormat();
        if ( pMedium->IsStorage() )
        {
            uno::Reference < embed::XStorage > xStor = pMedium->GetStorage();
            if ( xStor.is() )
            {
                while ( pFilter )
                {
                    if( 'C' == *pFilter->GetUserData().GetBuffer() && IsValidStgFilter( xStor, *pFilter ) )
                    {
                        if ( pFilter->IsOwnTemplateFormat() && !bLookForTemplate )
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        else
                            return pFilter;
                    }

                    pFilter = aIter.Next();
                }

                // there's only a template filter that could be found
                if ( pTemplateFilter )
                    pFilter = pTemplateFilter;
            }
        }
        else
        {
            SvStream* pStream = pMedium->GetInStream();
            if ( pStream && SotStorage::IsStorageFile(pStream) )
                xStg = new SotStorage( pStream, FALSE );

            if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )
            {
                while ( pFilter )
                {
                    if( 'C' == *pFilter->GetUserData().GetBuffer() && IsValidStgFilter( *xStg, *pFilter ) )
                    {
                        if ( pFilter->IsOwnTemplateFormat() && !bLookForTemplate )
                            // found template filter; maybe there's a "normal" one also
                            pTemplateFilter = pFilter;
                        else
                            return pFilter;
                    }

                    pFilter = aIter.Next();
                }

                // there's only a template filter that could be found
                if ( pTemplateFilter )
                    pFilter = pTemplateFilter;

            }
        }

        return pFilter;
    }

    sal_Char aBuffer[4098];
    const ULONG nMaxRead = sizeof(aBuffer) - 2;
    ULONG nBytesRead = 0;
    if (pMedium)
    {
        SvStream* pIStrm = pMedium->GetInStream();
        if( !pIStrm || SVSTREAM_OK != pIStrm->GetError() )
            return 0;
        ULONG nCurrPos = pIStrm->Tell();
        nBytesRead = pIStrm->Read(aBuffer, nMaxRead);
        pIStrm->Seek( nCurrPos );
    }
    /*
    else
    {
        SvFileStream aStrm( rFileName, STREAM_READ );

        // ohne FileName oder ohne Stream gibts nur den ANSI-Filter
        if( !rFileName.Len() || SVSTREAM_OK != aStrm.GetError() )
            return 0;

        nBytesRead = aStrm.Read(aBuffer, nMaxRead);
        aStrm.Close();
    }*/

    TerminateBuffer(aBuffer, nBytesRead, sizeof(aBuffer));


    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    /* suche nach dem bestimmten Filter, falls kein entsprechender        */
    /* gefunden wird, so wird der ASCII-Filter returnt.                   */
    /* Gibt es Filter ohne einen Identifizierungs-String, so werden diese */
    /* nie erkannt und es wird auch der ASCII-Filter returnt.             */
    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
    {
        const SfxFilter* pFilterTmp = 0;
        const sal_Char* pNm;
        for( USHORT n = 0; n < MAXFILTER; ++n )
        {
            String sEmptyUserData;
            pNm = aReaderWriter[n].IsReader(aBuffer, nBytesRead, rFileName, sEmptyUserData);
            pFilterTmp = pNm ? SwIoSystem::GetFilterOfFormat(String::CreateFromAscii(pNm), pFCntnr) : 0;
            if (pNm && pFilterTmp)
            {
                return pFilterTmp;
            }
        }
    }

    /* Ok, bis jetzt kein Filter gefunden, also befrage mal die */
    /* "WORD 4 WORD" Filter                                     */
    if( rFileName.Len() )
    {
        if( pMedium )
            pMedium->CloseInStream();

    }
    return SwIoSystem::GetTextFilter( aBuffer, nBytesRead);
}

bool SwIoSystem::IsDetectableText(const sal_Char* pBuf, ULONG &rLen,
    CharSet *pCharSet, bool *pSwap, LineEnd *pLineEnd, bool bEncodedFilter)
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
        sal_Unicode *pNewBuf = sWork.AllocBuffer( static_cast< xub_StrLen >(rLen));
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
#ifdef OSL_LITENDIAN
            bool bNativeLE = true;
#else
            bool bNativeLE = false;
#endif
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

        sWork.ReleaseBufferAccess( static_cast< xub_StrLen >(nNewLen) );
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

    return bEncodedFilter || (!bIsBareUnicode && eSysLE == eLineEnd);
}

const SfxFilter* SwIoSystem::GetTextFilter( const sal_Char* pBuf, ULONG nLen)
{
    bool bAuto = IsDetectableText(pBuf, nLen);
    const sal_Char* pNm = bAuto ? FILTER_TEXT : FILTER_TEXT_DLG;
    return SwIoSystem::GetFilterOfFormat( String::CreateFromAscii(pNm), 0 );
}


#endif
