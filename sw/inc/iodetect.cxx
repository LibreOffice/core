/*************************************************************************
 *
 *  $RCSfile: iodetect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:35:50 $
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

#ifndef _IODETECT_CXX
#define _IODETECT_CXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif


#ifdef _DLL_
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#endif

class Reader;
USHORT AutoDetec(const String& FileName, USHORT & rVersion);
bool IsDocShellRegistered();

typedef void (*FnGetWriter)(const String&, WriterRef&);

struct SwIoDetect
{
// eigentlich privat, aber der Compiler kann sonst die
// Tabelle nicht initialisieren
    const sal_Char* pName;
    USHORT nLen;

    inline int IsFilter( const String& rNm )
    {
        return rNm.EqualsAscii( pName, 0, nLen );
    }

#ifdef _DLL_
    Reader* pReader;
    FnGetWriter fnGetWriter;
    BOOL bDelReader;

    inline Reader* GetReader() const { return pReader; }
    inline void GetWriter( const String& rNm, WriterRef& xWrt ) const
        { if( fnGetWriter ) (*fnGetWriter)(rNm,xWrt); else xWrt = WriterRef(0); }
#endif

    const sal_Char* IsReader(const sal_Char* pHeader, ULONG nLen,
        const String &rFileName) const;
};


#ifdef _DLL_
#define SwIoEntry(sNm, cCharLen, pWrt, bDel)    sNm, cCharLen, 0, pWrt, bDel
#else
#define SwIoEntry(sNm, cCharLen, pWrt, bDel)    sNm, cCharLen
#endif

#ifdef DEBUG_SH

#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel) , SwIoEntry(sNm, cCharLen, pWrt, bDel)
#define W4W_CHECK_FOR_INTERNAL_FILTER                          \
    if( nW4WId == 3 )                                          \
        for( nCnt = 0; nCnt < nFltrCount; nCnt++ )             \
            if( 0 == ( pFilter = pFCntnr->GetFilter( nCnt ))-> \
                GetUserData().Search( sW4W_Int ))              \
                return pFilter;

#define W4W_INFOBOX InfoBox(0, String("Textformat wurde nicht erkannt.")).Execute();

#define W4W_FILTER_NOT_FOUND                                    \
        aW4WName = String::CreateFromAscii("W4W-Filter Nr. ");  \
        aW4WName += String::CreateFromInt32(nW4WId);            \
        aW4WName += '.';                                        \
        aW4WName += String::CreateFromInt32(nVersion);           \
        aW4WName.AppendAscii(" detected, ist aber nicht installiert");\
        InfoBox( 0, aW4WName ).Execute();

#else

#define DEB_SH_SwIoEntry(sNm, cCharLen, pWrt, bDel)
#define W4W_CHECK_FOR_INTERNAL_FILTER
#define W4W_INFOBOX
#define W4W_FILTER_NOT_FOUND

#endif

#if !( defined(PRODUCT) || defined(MAC) || defined(PM2))
#define DEB_DBWRT_SwIoEntry(sNm, cCharLen, pWrt, bDel)  , SwIoEntry(sNm, cCharLen, pWrt, bDel)
#else
#define DEB_DBWRT_SwIoEntry(sNm, cCharLen, pWrt, bDel)
#endif


const USHORT MAXFILTER =
#if !( defined(PRODUCT) || defined(MAC) || defined(PM2))
        2 +
#endif
#ifdef DEBUG_SH
        1 +
#endif
        21;

#define FORAMTNAME_SW4      "StarWriter 4.0"
#define FORAMTNAME_SW3      "StarWriter 3.0"
#define FORAMTNAME_SWGLOB   "StarWriter/Global 4.0"


sal_Char __FAR_DATA FILTER_SWG[]    = "SWG";
sal_Char __FAR_DATA FILTER_SW3[]    = "CSW3";
sal_Char __FAR_DATA FILTER_SW4[]    = "CSW4";
sal_Char __FAR_DATA FILTER_SW5[]    = "CSW5";
sal_Char __FAR_DATA FILTER_BAS[]    = "BAS";
sal_Char __FAR_DATA FILTER_RTF[]    = "RTF";
sal_Char __FAR_DATA FILTER_W4W[]    = "W4W";
sal_Char __FAR_DATA FILTER_SWGV[]   = "SWGV";
sal_Char __FAR_DATA FILTER_SW3V[]   = "CSW3V";
sal_Char __FAR_DATA FILTER_SW4V[]   = "CSW4V";
sal_Char __FAR_DATA FILTER_SW5V[]   = "CSW5V";
sal_Char __FAR_DATA FILTER_SWW4V[]  = "CSW4VWEB";
sal_Char __FAR_DATA FILTER_SWW5V[]  = "CSW5VWEB";
sal_Char __FAR_DATA sSwg1[]         = "SWG1";
sal_Char __FAR_DATA sRtfWH[]        = "WH_RTF";
sal_Char __FAR_DATA sCExcel[]       = "CEXCEL";
sal_Char __FAR_DATA sExcel[]        = "EXCEL";
sal_Char __FAR_DATA sLotusD[]       = "LOTUSD";
sal_Char __FAR_DATA sLotusW[]       = "LOTUSW";
sal_Char __FAR_DATA sHTML[]         = "HTML";
sal_Char __FAR_DATA sWW1[]          = "WW1";
sal_Char __FAR_DATA sWW5[]          = "WW6";
sal_Char __FAR_DATA sWW6[]          = "CWW6";
sal_Char __FAR_DATA FILTER_WW8[]    = "CWW8";
sal_Char __FAR_DATA FILTER_TEXT_DLG[] = "TEXT_DLG";
sal_Char __FAR_DATA FILTER_TEXT[]   = "TEXT";
sal_Char __FAR_DATA sW4W_Int[]      = "W4_INT";
sal_Char __FAR_DATA sDebug[]        = "DEBUG";
sal_Char __FAR_DATA sUndo[]         = "UNDO";
sal_Char __FAR_DATA FILTER_XML[]    = "CXML";
sal_Char __FAR_DATA FILTER_XMLV[]   = "CXMLV";
sal_Char __FAR_DATA FILTER_XMLVW[]  = "CXMLVWEB";
sal_Char __FAR_DATA sSwDos[]        = "SW6";


SwIoDetect aReaderWriter[ MAXFILTER ] = {
/*  0*/ SwIoEntry(FILTER_SW5,       4,          &::GetSw3Writer,    TRUE),
/*  1*/ SwIoEntry(FILTER_SW4,       4,          &::GetSw3Writer,    FALSE),
/*  2*/ SwIoEntry(FILTER_SW3,       4,          &::GetSw3Writer,    FALSE),
/*  3*/ SwIoEntry(FILTER_SWG,       STRING_LEN, 0,                  TRUE),
/*  4*/ SwIoEntry(FILTER_SWGV,      4,          0,                  FALSE),
/*  5*/ SwIoEntry(FILTER_RTF,       STRING_LEN, &::GetRTFWriter,    TRUE),
/*  6*/ SwIoEntry(sSwDos,           STRING_LEN, 0,                  TRUE),
/*  7*/ SwIoEntry(FILTER_BAS,       STRING_LEN, &::GetASCWriter,    FALSE),
/*  8*/ SwIoEntry(sWW6,             STRING_LEN, &::GetWW8Writer,    TRUE),
/*  9*/ SwIoEntry(FILTER_WW8,       STRING_LEN, &::GetWW8Writer,    FALSE),
/* 10*/ SwIoEntry(FILTER_W4W,       3,          &::GetW4WWriter,    TRUE),
/* 11*/ SwIoEntry(sRtfWH,           STRING_LEN, &::GetRTFWriter,    FALSE),
/* 12*/ SwIoEntry(sCExcel,          5,          0,                  TRUE),
/* 13*/ SwIoEntry(sExcel,           4,          0,                  FALSE),
/* 14*/ SwIoEntry(sLotusD,          5,          0,                  TRUE),
/* 15*/ SwIoEntry(sHTML,            4,          &::GetHTMLWriter,   TRUE),
/* 16*/ SwIoEntry(sWW1,             STRING_LEN, 0,                  TRUE),
/* 17*/ SwIoEntry(sWW5,             STRING_LEN, 0,                  FALSE),
/* 18*/ SwIoEntry(sSwg1,            4,          0,                  FALSE),
/* 19*/ SwIoEntry(FILTER_XML,       4,          &::GetXMLWriter,    TRUE)

/* opt*/ DEB_SH_SwIoEntry(sW4W_Int, STRING_LEN, 0,                  TRUE)
/* opt*/ DEB_DBWRT_SwIoEntry(sDebug,STRING_LEN, &::GetDebugWriter,  FALSE)
/* opt*/ DEB_DBWRT_SwIoEntry(sUndo, STRING_LEN, &::GetUndoWriter,   FALSE)
                                                                          ,
/*last*/ SwIoEntry(FILTER_TEXT,     4,          &::GetASCWriter,    TRUE)
};


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
    BOOL fComplexGet() { return ((fFlagsGet() >> 2) & 1); }
};
#if OSL_DEBUG_LEVEL > 1
#define WW2B || ((W1_FIB*)pHeader)->wIdentGet() == 0xA5DB &&  \
                ((W1_FIB*)pHeader)->nFibGet() == 0x2D
#else
#define WW2B
#endif

#if OSL_DEBUG_LEVEL > 1
#define WW3B || ((W1_FIB*)pHeader)->wIdentGet() == 0xA5DC &&  \
                ((W1_FIB*)pHeader)->nFibGet() == 0x65
#else
#define WW3B
#endif

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
        bRet = 0 == strncmp( "{\\rtf", pHeader, 5 );
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
const SfxFilter* SwIoSystem::GetFilterOfFormat(const String& rFmtNm,
    const SfxFactoryFilterContainer* pCnt)
{
    const SfxFactoryFilterContainer* pFltCnt = pCnt ? pCnt :
        ( IsDocShellRegistered()
            ? SwDocShell::Factory().GetFilterContainer()
            : SwWebDocShell::Factory().GetFilterContainer() );

    do {
        if( pFltCnt )
        {
            const SfxFilter* pFilter;
            USHORT nCount = pFltCnt->GetFilterCount();
            for( USHORT i = 0; i < nCount; ++i )
                if( ( pFilter = pFltCnt->GetFilter( i ))->GetUserData() == rFmtNm )
                    return pFilter;
        }
        if( pCnt || pFltCnt == SwWebDocShell::Factory().GetFilterContainer())
            break;
        pFltCnt = SwWebDocShell::Factory().GetFilterContainer();
    } while( TRUE );
    return 0;
}

FASTBOOL SwIoSystem::IsValidStgFilter(SvStorage& rStg, const SfxFilter& rFilter)
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
                SvStorageStreamRef xRef =
                    rStg.OpenStream(String::CreateFromAscii("WordDocument"),
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
    const SfxFactoryFilterContainer& rFltContainer = IsDocShellRegistered()
            ? *SwDocShell::Factory().GetFilterContainer()
            : *SwWebDocShell::Factory().GetFilterContainer();
    USHORT nFltCount = rFltContainer.GetFilterCount();
    SvStorageRef xStg;
    if (rMedium.IsStorage())
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
/* die Methode stellt fest, von welchem Typ der stream (File) ist.        */
/* Es wird versucht, eine dem Filter entsprechende Byte-Folge zu finden.  */
/* Wird kein entsprechender gefunden, wird zur Zeit der ASCII-Reader      */
/* returnt !! Der Returnwert ist der interne Filtername!                  */
/* rPrefFltName ist der interne Name des Filters, den der Benutzer im     */
/* Open-Dialog eingestellt hat.                                           */
const SfxFilter* SwIoSystem::GetFileFilter(const String& rFileName,
    const String& rPrefFltName, SfxMedium* pMedium)
{
    SfxFactoryFilterContainer* pFCntnr = IsDocShellRegistered()
            ? SwDocShell::Factory().GetFilterContainer()
            : SwWebDocShell::Factory().GetFilterContainer();

    USHORT nFltrCount;
    if( !pFCntnr || 0 == ( nFltrCount = pFCntnr->GetFilterCount() ) )
        return 0;

    const SfxFilter* pFilter;
    if( pMedium ? pMedium->IsStorage()
                : SvStorage::IsStorageFile( rFileName ) )
    {
        /* Storage: Suchen nach einem Sub-Storage, dessen Name  */
        /* dem in einem Filter stehenden DLL-Namen entspricht   */
        SvStorageRef xStg;
        if( pMedium )
            xStg = pMedium->GetStorage();
        else
            xStg = new SvStorage( rFileName, STREAM_STD_READ );

        if( xStg.Is() && ( xStg->GetError() == SVSTREAM_OK ) )
        {
            USHORT nCnt;
            for( nCnt = 0; nCnt < nFltrCount; ++nCnt )
                if( 'C' == *( pFilter = pFCntnr->GetFilter( nCnt ))->
                    GetUserData().GetBuffer() &&
                    IsValidStgFilter( *xStg, *pFilter ))
                    return pFilter;

            if( IsDocShellRegistered() && 0 != ( pFCntnr =
                SwWebDocShell::Factory().GetFilterContainer() ) &&
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
        {
            if(
               (pNm = aReaderWriter[n].IsReader(aBuffer, nBytesRead, rFileName))
                && (pFilter = SwIoSystem::GetFilterOfFormat(
                     String::CreateFromAscii(pNm), pFCntnr))
              )
              {
                return pFilter;
              }
        }
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
#ifdef __LITTLEENDIAN
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

#endif
