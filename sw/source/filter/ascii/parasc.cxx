/*************************************************************************
 *
 *  $RCSfile: parasc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-09-19 17:04:14 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen wg. SvxLanguageItem
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif

#define ASC_BUFFLEN 4096


#ifdef ASYNCHRON

class _SvLockBytes
{
    AutoTimer aTimer;
    SvStream& rIn;

    ULONG nDataRead;
    Link aCallDataRead;

    DECL_STATIC_LINK( _SvLockBytes, DataRead, Timer* );

public:
    _SvLockBytes( SvStream& rInput, const Link& rCallBack );
    ~_SvLockBytes();

    ErrCode ReadAt( ULONG nPos, void* pArr, ULONG nCount, ULONG* pReadCnt );

    SvStream& GetStream() const         { return rIn; }
};


// client vom Standard SwPageDesc. Wird dieser geloescht, dann
// der Reader auch beendet werden!!
class SwAsynchLoader : public SvRefBase, public SwClient
{
protected:

    SwDoc* pDoc;
    SwPaM* pPam;

    BOOL bInCall;
    _SvLockBytes* pLoader;

    DECL_STATIC_LINK( SwAsynchLoader, NewData, _SvLockBytes* );

protected:
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );
    virtual ~SwAsynchLoader();

    virtual ULONG NextData() = 0;

public:
    SwAsynchLoader( SvStream& rIn, const SwPaM& rCrsr );

    ULONG CallParser();
};

SV_DECL_REF(SwAsynchLoader)
SV_IMPL_REF(SwAsynchLoader)


class SwASCIIParser : public SwAsynchLoader
{
    char* pArr, *pStt, *pEnd, *pLastStt;

    long nLineLen;
    ULONG nError, nReadCnt, nFileSize;

    const SwAsciiOptions& rOpt;
    char cLastCRLF;

    virtual ULONG NextData();

protected:
    virtual ~SwASCIIParser();

public:
    SwASCIIParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                                int bReadNewDoc, const SwAsciiOptions& rOpts);
};

SV_DECL_REF(SwASCIIParser)
SV_IMPL_REF(SwASCIIParser)


// Aufruf fuer die allg. Reader-Schnittstelle
ULONG AsciiReader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
{
    if( !pStrm )
    {
        ASSERT( !this, "ASCII-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
    //              Kapitelnummer. Darum hier explizit abschalten
    //              weil das Default jetzt wieder auf AN ist.
    if( !bInsertMode )
        Reader::SetNoOutlineNum( rDoc );

    SwASCIIParserRef xParser( new SwASCIIParser( &rDoc, rPam, *pStrm,
                            !bInsertMode, eCodeSet ));
    ULONG nRet = xParser->CallParser();
    // nach dem Lesen sofort wieder defaulten
    eCodeSet = GetSystemCharSet();
    return ERRCODE_IO_PENDING == nRet ? 0 : nRet;
}


/**/

SwASCIIParser::SwASCIIParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                                int bReadNewDoc, CharSet eSrc )
    : SwAsynchLoader( rIn, rCrsr ),
     eCodeSet( eSrc )
{
    pArr = new char [ ASC_BUFFLEN + 1 ];

    pStt = pEnd = pLastStt = pArr;

    cLastCRLF = 0;
    nLineLen = 0;
    nError = 0;
    nReadCnt = 0;

    rIn.Seek(STREAM_SEEK_TO_END);
    rIn.ResetError();

    nFileSize = rIn.Tell();
    rIn.Seek(STREAM_SEEK_TO_BEGIN);
    rIn.ResetError();

}

SwASCIIParser::~SwASCIIParser()
{
    delete pArr;
}


ULONG SwASCIIParser::NextData()
{
    SvStream& rInput = pLoader->GetStream();
    do {
        if( pStt >= pEnd )
        {
            if( pLastStt != pStt )
            {
                pDoc->Insert( *pPam, pLastStt, eCodeSet );
                pLastStt = pStt;
            }

            // lese einen neuen Block ein
            ULONG lGCount;

            if (SVSTREAM_OK != rInput.GetError() || 0 != ( nError = pLoader->
                ReadAt( nReadCnt, pArr, ASC_BUFFLEN, &lGCount )) ||
                ( rInput.IsEof() && !lGCount ))
            {
                if( ERRCODE_IO_PENDING != nError || !lGCount )
                    break;      // aus der WHILE-Schleife heraus
            }

            pEnd = pArr + lGCount;
            nReadCnt += lGCount;
            *pEnd = 0;
            pStt = pLastStt = pArr;

            ::SetProgressState( nReadCnt, pDoc->GetDocShell() );

            if( cLastCRLF )
            {
                if( ( 0x0a == *pStt && 0x0d == cLastCRLF ) ||
                    ( 0x0d == *pStt && 0x0a == cLastCRLF ))
                    pLastStt = ++pStt;
                cLastCRLF = 0;
                nLineLen = 0;
                // JP 03.04.96: das letze am Ende nehmen wir nicht
                if( !rInput.IsEof() || !(pEnd == pStt ||
                    ( !*pEnd && pEnd == pStt+1 ) ) )
                    pDoc->SplitNode( *pPam->GetPoint() );
            }
        }

        BOOL bIns = TRUE, bSplitNode = FALSE;
        switch( *pStt )
        {
        case 0:
                    pEnd = pStt;
                    bIns = FALSE;
                    break;

        case 0x0a:
                    bIns = FALSE;
                    *pStt = 0;
                    if( ++pStt == pEnd )
                        cLastCRLF = 0x0a;
                    else
                    {
                        if( 0x0d == *pStt )
                            pStt++;
                        // JP 03.04.96: das letze am Ende nehmen wir nicht
                        if( !rInput.IsEof() || !(pEnd == pStt ||
                            ( !*pEnd && pEnd == pStt+1 ) ) )
                            bSplitNode = TRUE;
                    }
                    break;

        case 0x0d:
                    bIns = FALSE;
                    *pStt = 0;
                    if( ++pStt == pEnd )
                        cLastCRLF = 0x0d;
                    else
                    {
                        if( 0x0a == *pStt )
                            pStt++;
                        // JP 03.04.96: das letze am Ende nehmen wir nicht
                        if( !rInput.IsEof() || !(pEnd == pStt ||
                            ( !*pEnd && pEnd == pStt+1 ) ) )
                            bSplitNode = TRUE;
                    }
                    break;

        case '\t':  break;

        case 0x0c:
                    {
                        // dann mal einen harten Seitenumbruch einfuegen
                        *pStt++ = 0;
                        if( nLineLen )
                            pDoc->Insert( *pPam, pLastStt, eCodeSet );
                        pDoc->SplitNode( *pPam->GetPoint() );
                        pDoc->Insert( *pPam, SvxFmtBreakItem(
                                    SVX_BREAK_PAGE_BEFORE ));
                        pLastStt = pStt;
                        nLineLen = 0;
                        bIns = FALSE;
                    }
                    break;

        case 0x1a:
                    if( nReadCnt == nFileSize && pStt+1 == pEnd )
                        *pStt = 0;
                    else
                        *pStt = '#';        // Ersatzdarstellung
                    break;

        default:

            if( (BYTE)' ' > (BYTE)*pStt )
                    // Ctrl-Zchn gefunden ersetze durch '#'
                *pStt = '#';
        }

        if( bIns )
        {
            if( ( nLineLen >= MAX_ASCII_PARA - 100 ) &&
                ( ( *pStt == ' ' ) || ( nLineLen >= MAX_ASCII_PARA - 1 ) ) )
            {
                char c = *pStt;
                *pStt = 0;
                pDoc->Insert( *pPam, pLastStt, eCodeSet );
                pDoc->SplitNode( *pPam->GetPoint() );
                pLastStt = pStt;
                nLineLen = 0;
                *pStt = c;
            }
            ++pStt;
            ++nLineLen;
        }
        else if( bSplitNode )
        {
            // es wurde ein CR/LF erkannt, also speichere den Text
            pDoc->Insert( *pPam, pLastStt, eCodeSet );
            pDoc->SplitNode( *pPam->GetPoint() );
            pLastStt = pStt;
            nLineLen = 0;
        }
    } while( TRUE );

    return nError;
}


/**/

SwAsynchLoader::SwAsynchLoader( SvStream& rIn, const SwPaM& rCrsr )
    : SwClient( 0 )
{
    bInCall = FALSE;
    pDoc = (SwDoc*)rCrsr.GetDoc();
    pPam = new SwPaM( pDoc, *rCrsr.GetPoint() );

    SwPageDesc& rDesc = pDoc->_GetPageDesc( 0 );
    rDesc.Add( this );

    pLoader = new _SvLockBytes( rIn,
            STATIC_LINK( this, SwAsynchLoader, NewData ) );
}

SwAsynchLoader::~SwAsynchLoader()
{
    delete pPam;
    delete pLoader;
}

void SwAsynchLoader::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
    {
    case RES_OBJECTDYING:
        if( ((SwPtrMsgPoolItem *)pOld)->pObject == pRegisteredIn )
        {
            // dann uns selbst beenden
            pRegisteredIn->Remove( this );
            ReleaseRef();                   // ansonsten sind wir fertig!
        }
        break;
    }
}

IMPL_STATIC_LINK( SwAsynchLoader, NewData, _SvLockBytes*, p )
{
    ULONG nRet = 0;
    if( !pThis->bInCall )               // kein mehrfaches Aufrufen
    {
        pThis->bInCall = TRUE;

        BOOL bModify = pThis->pDoc->IsModified();
        nRet = pThis->NextData();
        if( !bModify )
            pThis->pDoc->ResetModified();

        if( ERRCODE_IO_PENDING != nRet )
            pThis->ReleaseRef();                    // ansonsten sind wir fertig!

        pThis->bInCall = FALSE;
    }

    return nRet;
}


ULONG SwAsynchLoader::CallParser()
{
    bInCall = TRUE;
    AddRef();
    ULONG nRet = NextData();
    if( ERRCODE_IO_PENDING != nRet )
        ReleaseRef();                   // ansonsten sind wir fertig!

    // Sind wir im Pending-Status, wird ueber den Callback "geidelt" bis
    // alles gelesen ist oder ein Fehler aufgetreten ist!

    // Was passiert dann ???

    bInCall = FALSE;
    return nRet;
}

/**/

_SvLockBytes::_SvLockBytes( SvStream& rInput, const Link& rCallback )
    : rIn( rInput ), aCallDataRead( rCallback )
{
    nDataRead = 0;

    aTimer.SetTimeout( 1000 );      // jede Sekunde 100 Zeichen lesen
    aTimer.SetTimeoutHdl( STATIC_LINK( this, _SvLockBytes, DataRead ));
    aTimer.Start();
}

_SvLockBytes::~_SvLockBytes()
{
    aTimer.Stop();
}

ErrCode _SvLockBytes::ReadAt( ULONG nPos, void* pArr, ULONG nCount,
                                ULONG* pReadCnt )
{
    ErrCode nRet = 0;
    if( nPos + nCount > nDataRead )
    {
        nCount = nDataRead - nPos;
        nRet = ERRCODE_IO_PENDING;
    }

    if( nCount )
    {
        rIn.Seek( nPos );
        *pReadCnt = rIn.Read( pArr, nCount );
    }
    else
        *pReadCnt = 0;
    return rIn.GetError() ? rIn.GetError()
                          : ( rIn.IsEof() ? 0 : nRet );
}

IMPL_STATIC_LINK( _SvLockBytes, DataRead, Timer*, pTimer )
{
    pThis->nDataRead += 100;
    pThis->aCallDataRead.Call( pThis );

    return 0;
}

/**/

#else

class SwASCIIParser
{
    SwDoc* pDoc;
    SwPaM* pPam;
    SvStream& rInput;
    sal_Char* pArr;
    const SwAsciiOptions& rOpt;
    SfxItemSet* pItemSet;
    long nFileSize;
    BOOL bNewDoc;

    ULONG ReadChars();

public:
    SwASCIIParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                            int bReadNewDoc, const SwAsciiOptions& rOpts );
    ~SwASCIIParser();

    ULONG CallParser();
};


// Aufruf fuer die allg. Reader-Schnittstelle
ULONG AsciiReader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
{
    if( !pStrm )
    {
        ASSERT( !this, "ASCII-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
    //              Kapitelnummer. Darum hier explizit abschalten
    //              weil das Default jetzt wieder auf AN ist.
    if( !bInsertMode )
        Reader::SetNoOutlineNum( rDoc );

    SwASCIIParser* pParser = new SwASCIIParser( &rDoc, rPam, *pStrm,
                                        !bInsertMode, aOpt.GetASCIIOpts() );
    ULONG nRet = pParser->CallParser();

    delete pParser;
    // after Read reset the options
    aOpt.ResetASCIIOpts();
    return nRet;
}

void AsciiReader::SetFltName( const String& rFltNm )
{
    if( 5 <= rFltNm.Len() )
    {
        SwAsciiOptions aNewOpts;
        switch( rFltNm.GetChar( 4 ) )
        {
        case 'D':
#if !defined(PM2)
                    aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );
                    aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                    if( 5 < rFltNm.Len() )
                        switch( rFltNm.Copy( 5 ).ToInt32() )
                        {
                        case 437: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_437 );  break;
                        case 850: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_850 );  break;
                        case 860: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_860 );  break;
                        case 861: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_861 );  break;
                        case 863: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_863 );  break;
                        case 865: aNewOpts.SetCharSet( RTL_TEXTENCODING_IBM_865 );  break;
                        }
                    break;

        case 'A':
#if !defined(WIN) && !defined(WNT)
                    aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                    aNewOpts.SetParaFlags( LINEEND_CRLF );
#endif
                    break;
        case 'M':
#if !defined(MAC)
                    aNewOpts.SetCharSet( RTL_TEXTENCODING_APPLE_ROMAN );
                    aNewOpts.SetParaFlags( LINEEND_CR );
#endif
                    break;
        case 'X':
#if !defined(UNX)
                    aNewOpts.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                    aNewOpts.SetParaFlags( LINEEND_LF );
#endif
                    break;

        default:
            if( rFltNm.Copy( 4 ).EqualsAscii( "_DLG" ))
            {
                // use the options
                aNewOpts = aOpt.GetASCIIOpts();
            }
        }
        aOpt.SetASCIIOpts( aNewOpts );
    }
}


SwASCIIParser::SwASCIIParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                            int bReadNewDoc, const SwAsciiOptions& rOpts )
    : pDoc( pD ), rInput( rIn ), rOpt( rOpts ), bNewDoc( bReadNewDoc ),
    pItemSet( 0 )
{
    pPam = new SwPaM( *rCrsr.GetPoint() );
    pArr = new sal_Char [ ASC_BUFFLEN + 1 ];

    SfxItemSet aDfltSet( pDoc->GetAttrPool(),
                RES_CHRATR_FONT,        RES_CHRATR_LANGUAGE,
                RES_CHRATR_CJK_FONT,    RES_CHRATR_CJK_LANGUAGE,
                RES_CHRATR_CTL_FONT,    RES_CHRATR_CTL_LANGUAGE,
                0 );

    // set defaults from the options
    if( rOpt.GetLanguage() )
    {
        SvxLanguageItem aLang( (LanguageType)rOpt.GetLanguage(),
                                 RES_CHRATR_LANGUAGE );
        aDfltSet.Put( aLang );
//!!!!!!!!!!!!!!!
//JP 19.9.2001: must exit defines for CJK and CTL
//      aDfltSet.Put( aLang, RES_CHRATR_CJK_LANGUAGE );
//      aDfltSet.Put( aLang, RES_CHRATR_CTL_LANGUAGE );
    }
    if( rOpt.GetFontName().Len() )
    {
        BOOL bDelete = FALSE;
        const SfxFont* pFnt = 0;
        if( pDoc->GetPrt() )
            pFnt = pDoc->GetPrt()->GetFontByName( rOpt.GetFontName() );

        if( !pFnt )
        {
            pFnt = new SfxFont( FAMILY_DONTKNOW, rOpt.GetFontName() );
            bDelete = TRUE;
        }
        SvxFontItem aFont( pFnt->GetFamily(), pFnt->GetName(),
                        aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet() );
        aDfltSet.Put( aFont );
//!!!!!!!!!!!!!!!
//JP 19.9.2001: must exit defines for CJK and CTL
//      aDfltSet.Put( aFont, RES_CHRATR_CJK_FONT );
//      aDfltSet.Put( aFont, RES_CHRATR_CTL_FONT );


        if( bDelete )
            delete (SfxFont*)pFnt;
    }

    if( aDfltSet.Count() )
    {
        if( bNewDoc )
            pDoc->SetDefault( aDfltSet );
        else
            pItemSet = new SfxItemSet( aDfltSet );
    }
}

SwASCIIParser::~SwASCIIParser()
{
    delete pPam;
    delete [] pArr;
    delete pItemSet;
}


// Aufruf des Parsers
ULONG SwASCIIParser::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_END);
    rInput.ResetError();

    nFileSize = rInput.Tell();
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    ::StartProgress( STR_STATSTR_W4WREAD, 0, nFileSize, pDoc->GetDocShell() );

    SwPaM* pInsPam = 0;
    xub_StrLen nSttCntnt;
    if( !bNewDoc )
    {
        const SwNodeIndex& rTmp = pPam->GetPoint()->nNode;
        pInsPam = new SwPaM( rTmp, rTmp, 0, -1 );
        nSttCntnt = pPam->GetPoint()->nContent.GetIndex();
    }

    ULONG nError = ReadChars();

    if( pInsPam )
    {
        if( pItemSet )
        {
            // then set over the insert range the defined attributes
            *pInsPam->GetMark() = *pPam->GetPoint();
            pInsPam->GetPoint()->nNode++;
            SwNode* pNd = pInsPam->GetNode();
            if( pNd->IsCntntNode() )
                pInsPam->GetPoint()->nContent.Assign(
                                    (SwCntntNode*)pNd, nSttCntnt );
            else
                pInsPam->GetPoint()->nContent.Assign( 0, 0 );

            // !!!!!
            ASSERT( !this, "Have to change - hard attr. to para. style" );
            pDoc->Insert( *pInsPam, *pItemSet );

            delete pItemSet, pItemSet = 0;
        }
        delete pInsPam;
    }

    ::EndProgress( pDoc->GetDocShell() );
    return nError;
}

ULONG SwASCIIParser::ReadChars()
{
    sal_Unicode *pStt = 0, *pEnd = 0, *pLastStt = 0;
    long nReadCnt = 0, nLineLen = 0;
    sal_Unicode cLastCR = 0;

    BOOL bSwapUnicode;
    rtl_TextToUnicodeConverter hConverter;
    if( RTL_TEXTENCODING_UCS2 != rOpt.GetCharSet() )
    {
        hConverter = rtl_createTextToUnicodeConverter( rOpt.GetCharSet() );
        ASSERT( hConverter, "no string convert avaiable" );
        if( !hConverter )
            return ERR_W4W_DLL_ERROR | ERROR_SW_READ_BASE;
        bSwapUnicode = FALSE;
    }
    else
    {
        hConverter = 0;
        rInput.StartReadingUnicodeText();
        bSwapUnicode = rInput.IsEndianSwap();
    }

    String sWork;
    ULONG nArrOffset = 0;

    do {
        if( pStt >= pEnd )
        {
            if( pLastStt != pStt )
                pDoc->Insert( *pPam, String( pLastStt ));

            // lese einen neuen Block ein
            ULONG lGCount;
            if( SVSTREAM_OK != rInput.GetError() || 0 == (lGCount =
                        rInput.Read( pArr + nArrOffset,
                                     ASC_BUFFLEN - nArrOffset )))
                break;      // aus der WHILE-Schleife heraus

            if( hConverter )
            {
                sal_uInt32 nInfo;
                sal_Size nNewLen = lGCount, nCntBytes;
                sal_Unicode* pBuf = sWork.AllocBuffer( nNewLen );

                nNewLen = rtl_convertTextToUnicode( hConverter, 0,
                                pArr, lGCount, pBuf, nNewLen,
                                (
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                ),
                                &nInfo,
                                &nCntBytes );
                if( 0 != ( nArrOffset = lGCount - nCntBytes ) )
                    memmove( pArr, pArr + nCntBytes, nArrOffset );
                sWork.ReleaseBufferAccess( nNewLen );

                pStt = pLastStt = sWork.GetBufferAccess();
                pEnd = pStt + nNewLen;
            }
            else
            {
                pStt = pLastStt = (sal_Unicode*)pArr;
                pEnd = (sal_Unicode*)(pArr + lGCount);

                if( bSwapUnicode )
                {
                    sal_Char* pF = pArr, *pN = pArr + 1;
                    for( ULONG n = 0; n < lGCount; n += 2, pF += 2, pN += 2 )
                    {
                        sal_Char c = *pF;
                        *pF = *pN;
                        *pN = c;
                    }
                }
            }

            *pEnd = 0;
            nReadCnt += lGCount;

            ::SetProgressState( nReadCnt, pDoc->GetDocShell() );

            if( cLastCR )
            {
                if( 0x0a == *pStt && 0x0d == cLastCR )
                    pLastStt = ++pStt;
                cLastCR = 0;
                nLineLen = 0;
                // JP 03.04.96: das letze am Ende nehmen wir nicht
                if( !rInput.IsEof() || !(pEnd == pStt ||
                    ( !*pEnd && pEnd == pStt+1 ) ) )
                    pDoc->SplitNode( *pPam->GetPoint() );
            }
        }

        BOOL bIns = TRUE, bSplitNode = FALSE;
        switch( *pStt )
        {
        case 0:
                    pEnd = pStt;
                    bIns = FALSE;
                    break;

        case 0x0a:  if( LINEEND_LF == rOpt.GetParaFlags() )
                    {
                        bIns = FALSE;
                        *pStt = 0;
                        ++pStt;

                        // JP 03.04.96: das letze am Ende nehmen wir nicht
                        if( !rInput.IsEof() || !(pEnd == pStt ||
                            ( !*pEnd && pEnd == pStt+1 ) ) )
                            bSplitNode = TRUE;
                    }
                    break;

        case 0x0d:  if( LINEEND_LF != rOpt.GetParaFlags() )
                    {
                        bIns = FALSE;
                        *pStt = 0;
                        ++pStt;

                        BOOL bChkSplit = FALSE;
                        if( LINEEND_CRLF == rOpt.GetParaFlags() )
                        {
                            if( pStt == pEnd )
                                cLastCR = 0x0d;
                            else if( 0x0a == *pStt )
                            {
                                ++pStt;
                                bChkSplit = TRUE;
                            }
                        }
                        else
                            bChkSplit = TRUE;

                            // JP 03.04.96: das letze am Ende nehmen wir nicht
                        if( bChkSplit && ( !rInput.IsEof() ||
                            !(pEnd == pStt || ( !*pEnd && pEnd == pStt+1 ))))
                                bSplitNode = TRUE;
                    }
                    break;

        case 0x0c:
                    {
                        // dann mal einen harten Seitenumbruch einfuegen
                        *pStt++ = 0;
                        if( nLineLen )
                        {
                            // Change to charset system!!!!
                            //rOpt.GetCharSet();
                            pDoc->Insert( *pPam, String( pLastStt ));
                        }
                        pDoc->SplitNode( *pPam->GetPoint() );
                        pDoc->Insert( *pPam, SvxFmtBreakItem(
                                    SVX_BREAK_PAGE_BEFORE ));
                        pLastStt = pStt;
                        nLineLen = 0;
                        bIns = FALSE;
                    }
                    break;

        case 0x1a:
                    if( nReadCnt == nFileSize && pStt+1 == pEnd )
                        *pStt = 0;
                    else
                        *pStt = '#';        // Ersatzdarstellung
                    break;

        case '\t':  break;

        default:
            if( ' ' > *pStt )
                    // Ctrl-Zchn gefunden ersetze durch '#'
                *pStt = '#';
            break;
        }

        if( bIns )
        {
            if( ( nLineLen >= MAX_ASCII_PARA - 100 ) &&
                ( ( *pStt == ' ' ) || ( nLineLen >= MAX_ASCII_PARA - 1 ) ) )
            {
                sal_Unicode c = *pStt;
                *pStt = 0;
                pDoc->Insert( *pPam, String( pLastStt ));
                pDoc->SplitNode( *pPam->GetPoint() );
                pLastStt = pStt;
                nLineLen = 0;
                *pStt = c;
            }
            ++pStt;
            ++nLineLen;
        }
        else if( bSplitNode )
        {
            // es wurde ein CR/LF erkannt, also speichere den Text

            pDoc->Insert( *pPam, String( pLastStt ));
            pDoc->SplitNode( *pPam->GetPoint() );
            pLastStt = pStt;
            nLineLen = 0;
        }
    } while( TRUE );

    if( hConverter )
        rtl_destroyTextToUnicodeConverter( hConverter );

    return 0;
}


#endif

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ascii/parasc.cxx,v 1.3 2001-09-19 17:04:14 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2001/01/19 12:40:07  jp
      use new stream interface instead of one implementation; Bugfix: #76023#: Textfilter use system charset

      Revision 1.1.1.1  2000/09/18 17:14:53  hr
      initial import

      Revision 1.25  2000/09/18 16:04:39  willem.vandorp
      OpenOffice header added.

      Revision 1.24  2000/08/04 16:25:28  jp
      read/write unicode ascii files

      Revision 1.23  2000/05/08 17:37:02  jp
      Changes for Unicode

      Revision 1.22  2000/03/16 16:42:31  kz
      chg. include to sfx2/printer

      Revision 1.21  1999/10/25 19:26:58  jp
      Bug #69048#: dont change LF to #

      Revision 1.20  1999/10/08 16:31:54  jp
      Bug #69048#: dont change tab to #

      Revision 1.19  1999/08/26 18:34:10  JP
      load and save Text with more options (charset/language/lineend/font)


      Rev 1.18   26 Aug 1999 20:34:10   JP
   load and save Text with more options (charset/language/lineend/font)

      Rev 1.17   23 Jul 1998 11:11:14   JP
   Task #52654#: Einfuegen Doc nicht mit einer CrsrShell sondern mit einen PaM

      Rev 1.16   29 Jun 1998 13:01:16   JP
   SvxFmtBreakItem ohne Flag!

      Rev 1.15   29 Apr 1998 08:17:14   MIB
   MUSS-Aenderung: SvStream::operator!() faellt weg

      Rev 1.14   27 Feb 1998 09:19:56   JP
   ObjectDying-MessageItem umbenannt

      Rev 1.13   22 Jan 1998 20:00:10   JP
   CTOR des SwPaM umgestellt

      Rev 1.12   30 Oct 1997 17:13:52   AMA
   Chg: Kein AutoFlag mehr an Break bzw. PageDesc-Attributen

      Rev 1.11   10 Oct 1997 12:20:22   JP
   Warnings vom Ueberlauf entfernt

      Rev 1.10   09 Oct 1997 14:26:02   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.9   12 Aug 1997 10:06:46   OS
   Header-Umstellung

      Rev 1.8   07 Aug 1997 15:07:08   OM
   Headerfile-Umstellung

      Rev 1.7   06 Aug 1997 12:46:52   HJS
   includes

      Rev 1.6   13 Dec 1996 12:18:58   JP
   Bug #34482#: nach dem Lesen auf SystemCodeSet umschalten

      Rev 1.5   29 Oct 1996 12:10:24   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.4   09 Oct 1996 17:58:46   MA
   Progress

      Rev 1.3   28 Jun 1996 13:59:08   MA
   includes

      Rev 1.2   02 May 1996 11:48:46   JP
   zum Testen: asynchrones Laden erweitert

      Rev 1.1   30 Apr 1996 12:09:38   JP
   Vorbereitung fuer asynchrones laden

      Rev 1.0   29 Apr 1996 18:11:00   JP
   Initial revision.

*************************************************************************/

