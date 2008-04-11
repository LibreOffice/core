/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FmtFilter.cxx,v $
 * $Revision: 1.16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"
#include "FmtFilter.hxx"
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <Shobjidl.h>
#include <shlguid.h>
#include <ObjIdl.h>
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include <systools/win32/comtools.hxx>

using namespace com::sun::star::uno;
using rtl::OString;

#pragma pack(2)
struct METAFILEHEADER
{
    DWORD       key;
    short       hmf;
    SMALL_RECT  bbox;
    WORD        inch;
    DWORD       reserved;
    WORD        checksum;
};
#pragma pack()

//------------------------------------------------------------------------
// convert a windows metafile picture to a openoffice metafile picture
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinMFPictToOOMFPict( Sequence< sal_Int8 >& aMetaFilePict )
{
    OSL_ASSERT( aMetaFilePict.getLength( ) == sizeof( METAFILEPICT ) );

    Sequence< sal_Int8 > mfpictStream;
    METAFILEPICT* pMFPict = reinterpret_cast< METAFILEPICT* >( aMetaFilePict.getArray( ) );
    HMETAFILE hMf = pMFPict->hMF;
    sal_uInt32 nCount = GetMetaFileBitsEx( hMf, 0, NULL );

    if ( nCount > 0 )
    {
        mfpictStream.realloc( nCount + sizeof( METAFILEHEADER ) );

        METAFILEHEADER* pMFHeader = reinterpret_cast< METAFILEHEADER* >( mfpictStream.getArray( ) );
        SMALL_RECT aRect = { 0,
                             0,
                             static_cast< short >( pMFPict->xExt ),
                             static_cast< short >( pMFPict->yExt ) };
        USHORT nInch;

        switch( pMFPict->mm )
        {
        case MM_TEXT:
            nInch = 72;
            break;

        case MM_LOMETRIC:
            nInch = 100;
            break;

        case MM_HIMETRIC:
            nInch = 1000;
            break;

        case MM_LOENGLISH:
            nInch = 254;
            break;

        case MM_HIENGLISH:
        case MM_ISOTROPIC:
        case MM_ANISOTROPIC:
            nInch = 2540;
            break;

        case MM_TWIPS:
            nInch = 1440;
            break;

        default:
            nInch = 576;
        }

        pMFHeader->key      = 0x9AC6CDD7L;
        pMFHeader->hmf      = 0;
        pMFHeader->bbox     = aRect;
        pMFHeader->inch     = nInch;
        pMFHeader->reserved = 0;
        pMFHeader->checksum = 0;

        char* pMFBuff = reinterpret_cast< char* >( mfpictStream.getArray( ) );

        nCount = GetMetaFileBitsEx( pMFPict->hMF, nCount, pMFBuff + sizeof( METAFILEHEADER ) );
        OSL_ASSERT( nCount > 0 );
    }

    return mfpictStream;
}

//-------------------------------------------------------------
// convert a windows enhanced metafile to a openoffice metafile
//-------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinENHMFPictToOOMFPict( HENHMETAFILE hEnhMetaFile )
{
    Sequence< sal_Int8 >    aRet;
    UINT                    nSize = 0;

    if( hEnhMetaFile &&
        ( ( nSize = GetEnhMetaFileBits( hEnhMetaFile, 0, NULL ) ) != 0 ) )
    {
        aRet.realloc( nSize );

        if( GetEnhMetaFileBits( hEnhMetaFile, nSize, (sal_uChar*) aRet.getArray() ) != nSize )
            aRet.realloc( 0 );
    }

    return aRet;
}

//------------------------------------------------------------------------
// convert a openoffice metafile picture to a windows metafile picture
//------------------------------------------------------------------------

HMETAFILEPICT SAL_CALL OOMFPictToWinMFPict( Sequence< sal_Int8 >& aOOMetaFilePict )
{
    OSL_ASSERT( aOOMetaFilePict.getLength() > 22 );

    HMETAFILEPICT   hPict = NULL;
    HMETAFILE       hMtf = SetMetaFileBitsEx( aOOMetaFilePict.getLength() - 22, (sal_uChar*) aOOMetaFilePict.getConstArray() + 22 );

    if( hMtf )
    {
        METAFILEPICT* pPict = (METAFILEPICT*) GlobalLock( hPict = GlobalAlloc( GHND, sizeof( METAFILEPICT ) ) );

        pPict->mm = 8;
        pPict->xExt = 0;
        pPict->yExt = 0;
        pPict->hMF = hMtf;

        GlobalUnlock( hPict );
    }

    return hPict;
}

//-----------------------------------------------------------------------------
// convert a openoffice metafile picture to a windows enhanced metafile picture
//-----------------------------------------------------------------------------

HENHMETAFILE SAL_CALL OOMFPictToWinENHMFPict( Sequence< sal_Int8 >& aOOMetaFilePict )
{
    HENHMETAFILE hEnhMtf = SetEnhMetaFileBits( aOOMetaFilePict.getLength(), (sal_uChar*) aOOMetaFilePict.getConstArray() );

    return hEnhMtf;
}

//------------------------------------------------------------------------
// convert a windows device independent bitmap into a openoffice bitmap
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL WinDIBToOOBMP( const Sequence< sal_Int8 >& aWinDIB )
{
    OSL_ASSERT( aWinDIB.getLength( ) > sizeof( BITMAPINFOHEADER ) );

    Sequence< sal_Int8 > ooBmpStream;

    ooBmpStream.realloc( aWinDIB.getLength( ) + sizeof(BITMAPFILEHEADER) );

    const BITMAPINFOHEADER  *pBmpInfoHdr = (const BITMAPINFOHEADER*)aWinDIB.getConstArray();
    BITMAPFILEHEADER        *pBmpFileHdr = reinterpret_cast< BITMAPFILEHEADER* >( ooBmpStream.getArray() );
    DWORD                   nOffset      = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );

    rtl_copyMemory( pBmpFileHdr + 1, pBmpInfoHdr, aWinDIB.getLength( ) );

    if( pBmpInfoHdr->biBitCount <= 8 )
        nOffset += ( pBmpInfoHdr->biClrUsed ? pBmpInfoHdr->biClrUsed : ( 1 << pBmpInfoHdr->biBitCount ) ) << 2;
    else if( ( BI_BITFIELDS == pBmpInfoHdr->biCompression ) && ( ( 16 == pBmpInfoHdr->biBitCount ) || ( 32 == pBmpInfoHdr->biBitCount ) ) )
        nOffset += 12;

    pBmpFileHdr->bfType      = 'MB';
    pBmpFileHdr->bfSize      = 0; // maybe: nMemSize + sizeof(BITMAPFILEHEADER)
    pBmpFileHdr->bfReserved1 = 0;
    pBmpFileHdr->bfReserved2 = 0;
    pBmpFileHdr->bfOffBits   = nOffset;

    return ooBmpStream;
}

//------------------------------------------------------------------------
// convert a openoffice bitmap into a windows device independent bitmap
//------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OOBmpToWinDIB( Sequence< sal_Int8 >& aOOBmp )
{
    OSL_ASSERT( aOOBmp.getLength( ) >
                ( sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ) );

    Sequence< sal_Int8 > winDIBStream( aOOBmp.getLength( ) - sizeof( BITMAPFILEHEADER ) );

    rtl_copyMemory( winDIBStream.getArray( ),
                    aOOBmp.getArray( )  + sizeof( BITMAPFILEHEADER ),
                    aOOBmp.getLength( ) - sizeof( BITMAPFILEHEADER ) );

    return winDIBStream;
}

//------------------------------------------------------------------------------
// converts the openoffice text/html clipboard format to the HTML Format
// well known under MS Windows
// the MS HTML Format has a header before the real html data
//
// Version:1.0      Version number of the clipboard. Staring is 0.9
// StartHTML:       Byte count from the beginning of the clipboard to the start
//                  of the context, or -1 if no context
// EndHTML:         Byte count from the beginning of the clipboard to the end
//                  of the context, or -1 if no context
// StartFragment:   Byte count from the beginning of the clipboard to the
//                  start of the fragment
// EndFragment:     Byte count from the beginning of the clipboard to the
//                  end of the fragment
// StartSelection:  Byte count from the beginning of the clipboard to the
//                  start of the selection
// EndSelection:    Byte count from the beginning of the clipboard to the
//                  end of the selection
//
// StartSelection and EndSelection are optional
// The fragment should be preceded and followed by the HTML comments
// <!--StartFragment--> and <!--EndFragment--> (no space between !-- and the
// text
//------------------------------------------------------------------------------
/*
Sequence< sal_Int8 > SAL_CALL TextHtmlToHTMLFormat( Sequence< sal_Int8 >& aTextHtml )
{
    OSL_ASSERT( aTextHtml.getLength( ) > 0 );

    // check parameter
    if ( !(aTextHtml.getLength( ) > 0) )
        return Sequence< sal_Int8 >( );

    // we create a buffer with the approximated size of
    // the HTML Format header
    char aHTMLFmtHdr[120];

    rtl_zeroMemory( aHTMLFmtHdr, sizeof( aHTMLFmtHdr ) );

    // fill the buffer with dummy values to calc the
    // exact length

    wsprintf(
        aHTMLFmtHdr,
        "Version:1.0\nStartHTML:%010d\r\nnEndHTML:%010d\r\nStartFragment:%010\r\nnEndFragment:%010d\r\n", 0, 0, 0, 0 );

    sal_uInt32 lHTMLFmtHdr = rtl_str_getLength( aHTMLFmtHdr );

    // the office allways writes the start
    // and end html tag in upper cases and
    // without spaces
    // both tags don't allow parameters
    OString startHtmlTag( "<HTML>" );
    OString endHtmlTag(   "</HTML>" );

    // we don't include '>' into the search
    // because the body tag allows parameters
    // e.g. <BODY param>
    // #92840#
    OString startBodyTag( "<BODY" );
    OString endBodyTag(   "</BODY" );

    OString textHtml(
        reinterpret_cast< const sal_Char* >( aTextHtml.getConstArray( ) ),
        aTextHtml.getLength( ) );

    sal_Int32 nStartHtml  = textHtml.indexOf( startHtmlTag );
    sal_Int32 nEndHtml    = textHtml.indexOf( endHtmlTag );
    sal_Int32 nStartFrgmt = textHtml.indexOf( startBodyTag );
    sal_Int32 nEndFrgmt   = textHtml.indexOf( endBodyTag );

    OSL_ASSERT( (nStartHtml >= 0) && (nEndHtml > nStartHtml) && (nStartFrgmt > nStartHtml) && (nEndFrgmt > nStartFrgmt) );

    Sequence< sal_Int8 > aHTMLFmtSequence;

    if ( (nStartHtml > -1) && (nEndHtml > -1) && (nStartFrgmt > -1) && (nEndFrgmt > -1) )
    {
        nStartHtml  = nStartHtml  + lHTMLFmtHdr - 1; // we start one before <HTML> Word 2000 does also so
        nEndHtml    = nEndHtml    + lHTMLFmtHdr + endHtmlTag.getLength( ) + 1; // our SOffice 5.2 wants 2 behind </HTML>?
        nStartFrgmt = nStartFrgmt + startBodyTag.getLength( ) + lHTMLFmtHdr; // after the <BODY> tag
        nEndFrgmt   = nEndFrgmt   + lHTMLFmtHdr;

        // fill the html header
        rtl_zeroMemory( aHTMLFmtHdr, sizeof( aHTMLFmtHdr ) );

        wsprintf(
            aHTMLFmtHdr,
            "Version:1.0\nStartHTML:%010d\r\nEndHTML:%010d\r\nStartFragment:%010d\r\nEndFragment:%010d\r\n",
            nStartHtml, nEndHtml, nStartFrgmt, nEndFrgmt );

        // we add space for a trailing \0
        aHTMLFmtSequence.realloc( lHTMLFmtHdr + aTextHtml.getLength( ) + 1 );
        rtl_zeroMemory( aHTMLFmtSequence.getArray( ), aHTMLFmtSequence.getLength( ) );

        // copy the HTML Format header
        rtl_copyMemory(
            static_cast< LPVOID >( aHTMLFmtSequence.getArray( ) ),
            static_cast< LPVOID >( aHTMLFmtHdr ), lHTMLFmtHdr );

        // concat the text/html
        rtl_copyMemory(
            static_cast< LPVOID >( aHTMLFmtSequence.getArray( ) + lHTMLFmtHdr ),
            static_cast< LPVOID >( aTextHtml.getArray( ) ),
            aTextHtml.getLength( ) );
    }

    return aHTMLFmtSequence;
}
*/

std::string GetHtmlFormatHeader(size_t startHtml, size_t endHtml, size_t startFragment, size_t endFragment)
{
    std::ostringstream htmlHeader;
    htmlHeader << "Version:1.0" << '\r' << '\n';
    htmlHeader << "StartHTML:" << std::setw(10) << std::setfill('0') << std::dec << startHtml << '\r' << '\n';
    htmlHeader << "EndHTML:" << std::setw(10) << std::setfill('0') << std::dec << endHtml << '\r' << '\n';
    htmlHeader << "StartFragment:" << std::setw(10) << std::setfill('0') << std::dec << startFragment << '\r' << '\n';
    htmlHeader << "EndFragment:" << std::setw(10) << std::setfill('0') << std::dec << endFragment << '\r' << '\n';
    return htmlHeader.str();
}

// the office allways writes the start and end html tag in upper cases and
// without spaces both tags don't allow parameters
const std::string TAG_HTML = std::string("<HTML>");
const std::string TAG_END_HTML = std::string("</HTML>");

// The body tag may have parameters so we need to search for the
// closing '>' manually e.g. <BODY param> #92840#
const std::string TAG_BODY = std::string("<BODY");
const std::string TAG_END_BODY = std::string("</BODY");

Sequence<sal_Int8> SAL_CALL TextHtmlToHTMLFormat(Sequence<sal_Int8>& aTextHtml)
{
    OSL_ASSERT(aTextHtml.getLength() > 0);

    if (!(aTextHtml.getLength() > 0))
        return Sequence<sal_Int8>();

    // fill the buffer with dummy values to calc the exact length
    std::string dummyHtmlHeader = GetHtmlFormatHeader(0, 0, 0, 0);
    size_t lHtmlFormatHeader = dummyHtmlHeader.length();

    std::string textHtml(
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()),
        reinterpret_cast<const sal_Char*>(aTextHtml.getConstArray()) + aTextHtml.getLength());

    std::string::size_type nStartHtml = textHtml.find(TAG_HTML) + lHtmlFormatHeader - 1; // we start one before '<HTML>' Word 2000 does also so
    std::string::size_type nEndHtml = textHtml.find(TAG_END_HTML) + lHtmlFormatHeader + TAG_END_HTML.length() + 1; // our SOffice 5.2 wants 2 behind </HTML>?

    // The body tag may have parameters so we need to search for the
    // closing '>' manually e.g. <BODY param> #92840#
    std::string::size_type nStartFragment = textHtml.find(">", textHtml.find(TAG_BODY)) + lHtmlFormatHeader + 1;
    std::string::size_type nEndFragment = textHtml.find(TAG_END_BODY) + lHtmlFormatHeader;

    std::string htmlFormat = GetHtmlFormatHeader(nStartHtml, nEndHtml, nStartFragment, nEndFragment);
    htmlFormat += textHtml;

    Sequence<sal_Int8> byteSequence(htmlFormat.length() + 1); // space the trailing '\0'
    rtl_zeroMemory(byteSequence.getArray(), byteSequence.getLength());

    rtl_copyMemory(
        static_cast<void*>(byteSequence.getArray()),
        static_cast<const void*>(htmlFormat.c_str()),
        htmlFormat.length());

    return byteSequence;
}

std::wstring getFileExtension(const std::wstring& aFilename)
{
    std::wstring::size_type idx = aFilename.rfind(L".");
    if ((idx != std::wstring::npos))
    {
        return std::wstring(aFilename, idx);
    }
    return std::wstring();
}

const std::wstring SHELL_LINK_FILE_EXTENSION = L".lnk";

bool isShellLink(const std::wstring& aFilename)
{
    std::wstring ext = getFileExtension(aFilename);
    return (_wcsicmp(ext.c_str(), SHELL_LINK_FILE_EXTENSION.c_str()) == 0);
}

/** Resolve a Windows Shell Link (lnk) file. If a resolution
    is not possible simply return the provided name of the
    lnk file. */
std::wstring getShellLinkTarget(const std::wstring& aLnkFile)
{
    OSL_ASSERT(isShellLink(aLnkFile));

    std::wstring target = aLnkFile;

    try
    {
        sal::systools::COMReference<IShellLinkA> pIShellLink;
        HRESULT hr = CoCreateInstance(
            CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID*>(&pIShellLink));
        if (FAILED(hr))
            return target;

        sal::systools::COMReference<IPersistFile> pIPersistFile =
            pIShellLink.QueryInterface<IPersistFile>(IID_IPersistFile);

        hr = pIPersistFile->Load(aLnkFile.c_str(), STGM_READ);
        if (FAILED(hr))
            return target;

        hr = pIShellLink->Resolve(NULL, SLR_UPDATE | SLR_NO_UI);
        if (FAILED(hr))
            return target;

        char pathA[MAX_PATH];
        WIN32_FIND_DATA wfd;
        hr = pIShellLink->GetPath(pathA, MAX_PATH, &wfd, SLGP_RAWPATH);
        if (FAILED(hr))
            return target;

        wchar_t pathW[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, pathA, -1, pathW, MAX_PATH);
        target = pathW;
    }
    catch(sal::systools::ComError& ex)
    {
        OSL_ENSURE(false, ex.what());
        ex = ex;
    }
    return target;
}

typedef std::vector<std::wstring> FileList_t;
typedef FileList_t::value_type FileList_ValueType_t;
typedef Sequence<sal_Int8> ByteSequence_t;

/* Calculate the size required for turning a string list into
   a double '\0' terminated string buffer */
size_t CalcSizeForStringListBuffer(const FileList_t& fileList)
{
    if (fileList.size() == 0)
        return 0;

    size_t size = 1; // one for the very final '\0'
    FileList_t::const_iterator iter_end = fileList.end();
    for (FileList_t::const_iterator iter = fileList.begin(); iter != iter_end; ++iter)
    {
        size += iter->length() + 1; // length including terminating '\0'
    }
    return (size * sizeof(FileList_ValueType_t::value_type));
}

ByteSequence_t FileListToByteSequence(const FileList_t& fileList)
{
    ByteSequence_t bseq;
    size_t size = CalcSizeForStringListBuffer(fileList);

    if (size > 0)
    {
        bseq.realloc(size);
        wchar_t* p = reinterpret_cast<wchar_t*>(bseq.getArray());
        ZeroMemory(p, size);

        FileList_t::const_iterator iter;
        FileList_t::const_iterator iter_end = fileList.end();
        for (iter = fileList.begin(); iter != iter_end; ++iter)
        {
            wcsncpy(p, iter->c_str(), iter->length());
            p += (iter->length() + 1);
        }
    }
    return bseq;
}

ByteSequence_t CF_HDROPToFileList(HGLOBAL hGlobal)
{
    UINT nFiles = DragQueryFileW((HDROP)hGlobal, 0xFFFFFFFF, NULL, 0);
    FileList_t files;

    for (UINT i = 0; i < nFiles; i++)
    {
        wchar_t buff[MAX_PATH];
        /*UINT size =*/ DragQueryFileW((HDROP)hGlobal, i, buff, MAX_PATH);
        std::wstring filename = buff;
        if (isShellLink(filename))
            filename = getShellLinkTarget(filename);
        files.push_back(filename);
    }
    return FileListToByteSequence(files);
}

