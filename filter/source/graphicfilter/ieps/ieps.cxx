/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdio.h>

//
#include <tools/solar.h>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.h>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/bmpacc.hxx>
#include <svtools/fltcall.hxx>
#include <tools/tempfile.hxx>
#include <osl/process.h>
#include <osl/file.hxx>

/*************************************************************************
|*
|*    ImpSearchEntry()
|*
|*    Beschreibung      Prueft ob im Speicherbereich pSource der nComp Bytes
|*                      gross ist eine Zeichenkette(pDest) mit der l�nge nSize
|*                      liegt. Geprueft wird NON-CASE-SENSITIVE und der Rueck-
|*                      gabewert ist die Adresse an der die Zeichekette gefunden
|*                      wurde oder NULL
|*
*************************************************************************/

static sal_uInt8* ImplSearchEntry( sal_uInt8* pSource, sal_uInt8* pDest, sal_uLong nComp, sal_uLong nSize )
{
    while ( nComp-- >= nSize )
    {
        sal_uLong i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}

//--------------------------------------------------------------------------
// SecurityCount is the buffersize of the buffer in which we will parse for a number
static long ImplGetNumber( sal_uInt8 **pBuf, int& nSecurityCount )
{
    sal_Bool    bValid = sal_True;
    sal_Bool    bNegative = sal_False;
    long    nRetValue = 0;
    while ( ( --nSecurityCount ) && ( ( **pBuf == ' ' ) || ( **pBuf == 0x9 ) ) )
        (*pBuf)++;
    sal_uInt8 nByte = **pBuf;
    while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
    {
        switch ( nByte )
        {
            case '.' :
                // we'll only use the integer format
                bValid = sal_False;
                break;
            case '-' :
                bNegative = sal_True;
                break;
            default :
                if ( ( nByte < '0' ) || ( nByte > '9' ) )
                    nSecurityCount = 1;         // error parsing the bounding box values
                else if ( bValid )
                {
                    nRetValue *= 10;
                    nRetValue += nByte - '0';
                }
                break;
        }
        nSecurityCount--;
        nByte = *(++(*pBuf));
    }
    if ( bNegative )
        nRetValue = -nRetValue;
    return nRetValue;
}

//--------------------------------------------------------------------------

static int ImplGetLen( sal_uInt8* pBuf, int nMax )
{
    int nLen = 0;
    while( nLen != nMax )
    {
        sal_uInt8 nDat = *pBuf++;
        if ( nDat == 0x0a || nDat == 0x25 )
            break;
        nLen++;
    }
    return nLen;
}

static void MakeAsMeta(Graphic &rGraphic)
{
    VirtualDevice   aVDev;
    GDIMetaFile     aMtf;
    Bitmap          aBmp( rGraphic.GetBitmap() );
    Size            aSize = aBmp.GetPrefSize();

    if( !aSize.Width() || !aSize.Height() )
        aSize = Application::GetDefaultDevice()->PixelToLogic(
            aBmp.GetSizePixel(), MAP_100TH_MM );
    else
        aSize = Application::GetDefaultDevice()->LogicToLogic( aSize,
            aBmp.GetPrefMapMode(), MAP_100TH_MM );

    aVDev.EnableOutput( sal_False );
    aMtf.Record( &aVDev );
    aVDev.DrawBitmap( Point(), aSize, rGraphic.GetBitmap() );
    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefMapMode( MAP_100TH_MM );
    aMtf.SetPrefSize( aSize );
    rGraphic = aMtf;
}

static oslProcessError runProcessWithPathSearch(const rtl::OUString &rProgName,
    rtl_uString* pArgs[], sal_uInt32 nArgs, oslProcess *pProcess,
    oslFileHandle *pIn, oslFileHandle *pOut, oslFileHandle *pErr)
{
#ifdef WNT
    /*
     * ooo#72096
     * On Window the underlying SearchPath searches in order of...
     * The directory from which the application loaded.
     * The current directory.
     * The Windows system directory.
     * The Windows directory.
     * The directories that are listed in the PATH environment variable.
     *
     * Because one of our programs is called "convert" and there is a convert
     * in the windows system directory, we want to explicitly search the PATH
     * to avoid picking up on that one if ImageMagick's convert preceeds it in
     * PATH.
     *
     */
    rtl::OUString url;
    rtl::OUString path(reinterpret_cast<const sal_Unicode*>(_wgetenv(L"PATH")));

    oslFileError err = osl_searchFileURL(rProgName.pData, path.pData, &url.pData);
    if (err != osl_File_E_None)
        return osl_Process_E_NotFound;
    return osl_executeProcess_WithRedirectedIO(url.pData,
    pArgs, nArgs, osl_Process_HIDDEN,
        osl_getCurrentSecurity(), 0, 0, 0, pProcess, pIn, pOut, pErr);
#else
    return osl_executeProcess_WithRedirectedIO(rProgName.pData,
        pArgs, nArgs, osl_Process_SEARCHPATH | osl_Process_HIDDEN,
        osl_getCurrentSecurity(), 0, 0, 0, pProcess, pIn, pOut, pErr);
#endif
}

#if defined(WNT)
#    define EXESUFFIX ".exe"
#else
#    define EXESUFFIX ""
#endif

static bool RenderAsEMF(const sal_uInt8* pBuf, sal_uInt32 nBytesRead, Graphic &rGraphic)
{
    TempFile aTemp;
    aTemp.EnableKillingFile();
    rtl::OUString fileName =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pstoedit" EXESUFFIX));
    rtl::OUString arg1 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-f"));
    rtl::OUString arg2 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("emf:-OO"));
    rtl::OUString arg3 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
    rtl::OUString output;
    osl::FileBase::getSystemPathFromFileURL(aTemp.GetName(), output);
    rtl_uString *args[] =
    {
        arg1.pData, arg2.pData, arg3.pData, output.pData
    };
    oslProcess aProcess;
    oslFileHandle pIn = NULL;
    oslFileHandle pOut = NULL;
    oslFileHandle pErr = NULL;
        oslProcessError eErr = runProcessWithPathSearch(fileName,
            args, sizeof(args)/sizeof(rtl_uString *),
            &aProcess, &pIn, &pOut, &pErr);

    if (eErr!=osl_Process_E_None)
        return false;

    bool bRet = false;
    sal_uInt64 nCount;
    osl_writeFile(pIn, pBuf, nBytesRead, &nCount);
    if (pIn) osl_closeFile(pIn);
    bool bEMFSupported=true;
    if (pOut)
    {
        rtl::ByteSequence seq;
                if (osl_File_E_None == osl_readLine(pOut, (sal_Sequence **)&seq))
        {
                        rtl::OString line( (const sal_Char *) seq.getConstArray(), seq.getLength() );
            if (line.indexOf(rtl::OString("Unsupported output format")) == 0)
                bEMFSupported=false;
        }
        osl_closeFile(pOut);
    }
    if (pErr) osl_closeFile(pErr);
    if (nCount == nBytesRead && bEMFSupported)
    {
        SvFileStream aFile(output, STREAM_READ);
        if (GraphicConverter::Import(aFile, rGraphic, CVT_EMF) == ERRCODE_NONE)
            bRet = true;
    }
    osl_joinProcess(aProcess);
    osl_freeProcessHandle(aProcess);
    return bRet;
}

static bool RenderAsPNGThroughHelper(const sal_uInt8* pBuf, sal_uInt32 nBytesRead,
    Graphic &rGraphic, rtl::OUString &rProgName, rtl_uString *pArgs[], size_t nArgs)
{
    oslProcess aProcess;
    oslFileHandle pIn = NULL;
    oslFileHandle pOut = NULL;
    oslFileHandle pErr = NULL;
        oslProcessError eErr = runProcessWithPathSearch(rProgName,
            pArgs, nArgs,
            &aProcess, &pIn, &pOut, &pErr);
    if (eErr!=osl_Process_E_None)
        return false;

    bool bRet = false;
    sal_uInt64 nCount;
    osl_writeFile(pIn, pBuf, nBytesRead, &nCount);
    if (pIn) osl_closeFile(pIn);
    if (nCount == nBytesRead)
    {
        SvMemoryStream aMemStm;
        sal_uInt8 aBuf[32000];
        oslFileError eFileErr = osl_readFile(pOut, aBuf, 32000, &nCount);
        while (eFileErr == osl_File_E_None && nCount)
        {
            aMemStm.Write(aBuf, sal::static_int_cast< sal_Size >(nCount));
            eFileErr = osl_readFile(pOut, aBuf, 32000, &nCount);
        }

        aMemStm.Seek(0);
        if (
            aMemStm.GetEndOfData() &&
            GraphicConverter::Import(aMemStm, rGraphic, CVT_PNG) == ERRCODE_NONE
           )
        {
            MakeAsMeta(rGraphic);
            bRet = true;
        }
    }
    if (pOut) osl_closeFile(pOut);
    if (pErr) osl_closeFile(pErr);
    osl_joinProcess(aProcess);
    osl_freeProcessHandle(aProcess);
    return bRet;
}

static bool RenderAsPNGThroughConvert(const sal_uInt8* pBuf, sal_uInt32 nBytesRead,
    Graphic &rGraphic)
{
    rtl::OUString fileName =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("convert" EXESUFFIX));
    // density in pixel/inch
    rtl::OUString arg1 = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-density"));
    // since the preview is also used for PDF-Export & printing on non-PS-printers,
    // use some better quality - 300x300 should allow some resizing as well
    rtl::OUString arg2 = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("300x300"));
    // read eps from STDIN
    rtl::OUString arg3 = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("eps:-"));
    // write png to STDOUT
    rtl::OUString arg4 = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("png:-"));
    rtl_uString *args[] =
    {
        arg1.pData, arg2.pData, arg3.pData, arg4.pData
    };
    return RenderAsPNGThroughHelper(pBuf, nBytesRead, rGraphic, fileName, args,
        sizeof(args)/sizeof(rtl_uString *));
}

static bool RenderAsPNGThroughGS(const sal_uInt8* pBuf, sal_uInt32 nBytesRead,
    Graphic &rGraphic)
{
#ifdef WNT
    rtl::OUString fileName =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("gswin32c" EXESUFFIX));
#else
    rtl::OUString fileName =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("gs" EXESUFFIX));
#endif
    rtl::OUString arg1 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-q"));
    rtl::OUString arg2 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dBATCH"));
    rtl::OUString arg3 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dNOPAUSE"));
    rtl::OUString arg4 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dPARANOIDSAFER"));
    rtl::OUString arg5 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dEPSCrop"));
    rtl::OUString arg6 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dTextAlphaBits=4"));
    rtl::OUString arg7 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-dGraphicsAlphaBits=4"));
    rtl::OUString arg8 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-r300x300"));
    rtl::OUString arg9 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-sDEVICE=png256"));
    rtl::OUString arg10 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-sOutputFile=-"));
    rtl::OUString arg11 =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
    rtl_uString *args[] =
    {
        arg1.pData, arg2.pData, arg3.pData, arg4.pData, arg5.pData,
        arg6.pData, arg7.pData, arg8.pData, arg9.pData, arg10.pData,
        arg11.pData
    };
    return RenderAsPNGThroughHelper(pBuf, nBytesRead, rGraphic, fileName, args,
        sizeof(args)/sizeof(rtl_uString *));
}

static bool RenderAsPNG(const sal_uInt8* pBuf, sal_uInt32 nBytesRead, Graphic &rGraphic)
{
    if (RenderAsPNGThroughConvert(pBuf, nBytesRead, rGraphic))
        return true;
    else
        return RenderAsPNGThroughGS(pBuf, nBytesRead, rGraphic);
}

// this method adds a replacement action containing the original wmf or tiff replacement,
// so the original eps can be written when storing to ODF.
void CreateMtfReplacementAction( GDIMetaFile& rMtf, SvStream& rStrm, sal_uInt32 nOrigPos, sal_uInt32 nPSSize,
                                sal_uInt32 nPosWMF, sal_uInt32 nSizeWMF, sal_uInt32 nPosTIFF, sal_uInt32 nSizeTIFF )
{
    rtl::OString aComment(RTL_CONSTASCII_STRINGPARAM("EPSReplacementGraphic"));
    if ( nSizeWMF || nSizeTIFF )
    {
        SvMemoryStream aReplacement( nSizeWMF + nSizeTIFF + 28 );
        sal_uInt32 nMagic = 0xc6d3d0c5;
        sal_uInt32 nPPos = 28 + nSizeWMF + nSizeTIFF;
        sal_uInt32 nWPos = nSizeWMF ? 28 : 0;
        sal_uInt32 nTPos = nSizeTIFF ? 28 + nSizeWMF : 0;

        aReplacement << nMagic << nPPos << nPSSize
                     << nWPos << nSizeWMF
                     << nTPos << nSizeTIFF;
        if ( nSizeWMF )
        {
            sal_uInt8* pBuf = new sal_uInt8[ nSizeWMF ];
            rStrm.Seek( nOrigPos + nPosWMF );
            rStrm.Read( pBuf, nSizeWMF );
            aReplacement.Write( pBuf, nSizeWMF );
            delete[] pBuf;
        }
        if ( nSizeTIFF )
        {
            sal_uInt8* pBuf = new sal_uInt8[ nSizeTIFF ];
            rStrm.Seek( nOrigPos + nPosTIFF );
            rStrm.Read( pBuf, nSizeTIFF );
            aReplacement.Write( pBuf, nSizeTIFF );
            delete[] pBuf;
        }
        rMtf.AddAction( (MetaAction*)( new MetaCommentAction( aComment, 0, (const sal_uInt8*)aReplacement.GetData(), aReplacement.Tell() ) ) );
    }
    else
        rMtf.AddAction( (MetaAction*)( new MetaCommentAction( aComment, 0, NULL, 0 ) ) );
}

//there is no preview -> make a red box
void MakePreview(sal_uInt8* pBuf, sal_uInt32 nBytesRead,
    long nWidth, long nHeight, Graphic &rGraphic)
{
    GDIMetaFile aMtf;
    VirtualDevice   aVDev;
    Font            aFont;

    aVDev.EnableOutput( sal_False );
    aMtf.Record( &aVDev );
    aVDev.SetLineColor( Color( COL_RED ) );
    aVDev.SetFillColor();

    aFont.SetColor( COL_LIGHTRED );
//  aFont.SetSize( Size( 0, 32 ) );

    aVDev.Push( PUSH_FONT );
    aVDev.SetFont( aFont );

    Rectangle aRect( Point( 1, 1 ), Size( nWidth - 2, nHeight - 2 ) );
    aVDev.DrawRect( aRect );

    String aString;
    int nLen;
    sal_uInt8* pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%Title:", nBytesRead - 32, 8 );
    if ( pDest )
    {
        pDest += 8;
        if ( *pDest == ' ' )
            pDest++;
        nLen = ImplGetLen( pDest, 32 );
        sal_uInt8 aOldValue(pDest[ nLen ]); pDest[ nLen ] = 0;
        if ( strcmp( (const char*)pDest, "none" ) != 0 )
        {
            aString.AppendAscii( " Title:" );
            aString.AppendAscii( (char*)pDest );
            aString.AppendAscii( "\n" );
        }
        pDest[ nLen ] = aOldValue;
    }
    pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%Creator:", nBytesRead - 32, 10 );
    if ( pDest )
    {
        pDest += 10;
        if ( *pDest == ' ' )
            pDest++;
        nLen = ImplGetLen( pDest, 32 );
        sal_uInt8 aOldValue(pDest[ nLen ]); pDest[ nLen ] = 0;
        aString.AppendAscii( " Creator:" );
        aString.AppendAscii( (char*)pDest );
        aString.AppendAscii( "\n" );
        pDest[ nLen ] = aOldValue;
    }
    pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%CreationDate:", nBytesRead - 32, 15 );
    if ( pDest )
    {
        pDest += 15;
        if ( *pDest == ' ' )
            pDest++;
        nLen = ImplGetLen( pDest, 32 );
        sal_uInt8 aOldValue(pDest[ nLen ]); pDest[ nLen ] = 0;
        if ( strcmp( (const char*)pDest, "none" ) != 0 )
        {
            aString.AppendAscii( " CreationDate:" );
            aString.AppendAscii( (char*)pDest );
            aString.AppendAscii( "\n" );
        }
        pDest[ nLen ] = aOldValue;
    }
    pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%LanguageLevel:", nBytesRead - 4, 16 );
    if ( pDest )
    {
        pDest += 16;
        int nCount = 4;
        long nNumber = ImplGetNumber( &pDest, nCount );
        if ( nCount && ( (sal_uInt32)nNumber < 10 ) )
        {
            aString.AppendAscii( " LanguageLevel:" );
            aString.Append( UniString::CreateFromInt32( nNumber ) );
        }
    }
    aVDev.DrawText( aRect, aString, TEXT_DRAW_CLIP | TEXT_DRAW_MULTILINE );
    aVDev.Pop();
    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefMapMode( MAP_POINT );
    aMtf.SetPrefSize( Size( nWidth, nHeight ) );
    rGraphic = aMtf;
}


//================== GraphicImport - die exportierte Funktion ================

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool __LOADONCALLAPI
GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    if ( rStream.GetError() )
        return sal_False;

    Graphic     aGraphic;
    sal_Bool    bRetValue = sal_False;
    sal_Bool    bHasPreview = sal_False;
    sal_Bool    bGraphicLinkCreated = sal_False;
    sal_uInt32  nSignature, nPSStreamPos, nPSSize;
    sal_uInt32  nSizeWMF = 0;
    sal_uInt32  nPosWMF = 0;
    sal_uInt32  nSizeTIFF = 0;
    sal_uInt32  nPosTIFF = 0;
    sal_uInt32  nOrigPos = nPSStreamPos = rStream.Tell();
    sal_uInt16  nOldFormat = rStream.GetNumberFormatInt();
    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStream >> nSignature;
    if ( nSignature == 0xc6d3d0c5 )
    {
        rStream >> nPSStreamPos >> nPSSize >> nPosWMF >> nSizeWMF;

        // first we try to get the metafile grafix

        if ( nSizeWMF )
        {
            if ( nPosWMF != 0 )
            {
                rStream.Seek( nOrigPos + nPosWMF );
                if ( GraphicConverter::Import( rStream, aGraphic, CVT_WMF ) == ERRCODE_NONE )
                    bHasPreview = bRetValue = sal_True;
            }
        }
        else
        {
            rStream >> nPosTIFF >> nSizeTIFF;

            // else we have to get the tiff grafix

            if ( nPosTIFF && nSizeTIFF )
            {
                rStream.Seek( nOrigPos + nPosTIFF );
                if ( GraphicConverter::Import( rStream, aGraphic, CVT_TIF ) == ERRCODE_NONE )
                {
                    MakeAsMeta(aGraphic);
                    rStream.Seek( nOrigPos + nPosTIFF );
                    bHasPreview = bRetValue = sal_True;
                }
            }
        }
    }
    else
    {
        nPSStreamPos = nOrigPos;            // no preview available _>so we must get the size manually
        nPSSize = rStream.Seek( STREAM_SEEK_TO_END ) - nOrigPos;
    }
    sal_uInt8* pHeader = new sal_uInt8[ 22 ];
    rStream.Seek( nPSStreamPos );
    rStream.Read( pHeader, 22 );    // check PostScript header
    if ( ImplSearchEntry( pHeader, (sal_uInt8*)"%!PS-Adobe", 10, 10 ) &&
        ImplSearchEntry( &pHeader[ 15 ], (sal_uInt8*)"EPS", 3, 3 ) )
    {
        rStream.Seek( nPSStreamPos );
        sal_uInt8* pBuf = new sal_uInt8[ nPSSize ];
        if ( pBuf )
        {
            sal_uInt32  nBufStartPos = rStream.Tell();
            sal_uInt32  nBytesRead = rStream.Read( pBuf, nPSSize );
            if ( nBytesRead == nPSSize )
            {
                int nSecurityCount = 32;
                if ( !bHasPreview )     // if there is no tiff/wmf preview, we will parse for an preview in the eps prolog
                {
                    sal_uInt8* pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%BeginPreview:", nBytesRead - 32, 15 );
                    if ( pDest  )
                    {
                        pDest += 15;
                        long nWidth = ImplGetNumber( &pDest, nSecurityCount );
                        long nHeight = ImplGetNumber( &pDest, nSecurityCount );
                        long nBitDepth = ImplGetNumber( &pDest, nSecurityCount );
                        long nScanLines = ImplGetNumber( &pDest, nSecurityCount );
                        pDest = ImplSearchEntry( pDest, (sal_uInt8*)"%", 16, 1 );       // go to the first Scanline
                        if ( nSecurityCount && pDest && nWidth && nHeight && ( ( nBitDepth == 1 ) || ( nBitDepth == 8 ) ) && nScanLines )
                        {
                            rStream.Seek( nBufStartPos + ( pDest - pBuf ) );

                            Bitmap aBitmap( Size( nWidth, nHeight ), 1 );
                            BitmapWriteAccess* pAcc = aBitmap.AcquireWriteAccess();
                            if ( pAcc )
                            {
                                int  nBitsLeft;
                                sal_Bool bIsValid = sal_True;
                                sal_uInt8 nDat = 0;
                                char nByte;
                                for ( long y = 0; bIsValid && ( y < nHeight ); y++ )
                                {
                                    nBitsLeft = 0;
                                    for ( long x = 0; x < nWidth; x++ )
                                    {
                                        if ( --nBitsLeft < 0 )
                                        {
                                            while ( bIsValid && ( nBitsLeft != 7 ) )
                                            {
                                                rStream >> nByte;
                                                switch ( nByte )
                                                {
                                                    case 0x0a :
                                                        if ( --nScanLines < 0 )
                                                            bIsValid = sal_False;
                                                    case 0x09 :
                                                    case 0x0d :
                                                    case 0x20 :
                                                    case 0x25 :
                                                    break;
                                                    default:
                                                    {
                                                        if ( nByte >= '0' )
                                                        {
                                                            if ( nByte > '9' )
                                                            {
                                                                nByte &=~0x20;  // case none sensitive for hexadezimal values
                                                                nByte -= ( 'A' - 10 );
                                                                if ( nByte > 15 )
                                                                    bIsValid = sal_False;
                                                            }
                                                            else
                                                                nByte -= '0';
                                                            nBitsLeft += 4;
                                                            nDat <<= 4;
                                                            nDat |= ( nByte ^ 0xf ); // in epsi a zero bit represents white color
                                                        }
                                                        else
                                                            bIsValid = sal_False;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                        if ( nBitDepth == 1 )
                                            pAcc->SetPixel( y, x, sal::static_int_cast< sal_uInt8 >(( nDat >> nBitsLeft ) & 1) );
                                        else
                                        {
                                            pAcc->SetPixel( y, x, ( nDat ) ? 1 : 0 );   // nBitDepth == 8
                                            nBitsLeft = 0;
                                        }
                                    }
                                }
                                if ( bIsValid )
                                {
                                    VirtualDevice   aVDev;
                                    GDIMetaFile     aMtf;
                                    Size            aSize;
                                    aVDev.EnableOutput( sal_False );
                                    aMtf.Record( &aVDev );
                                    aSize = aBitmap.GetPrefSize();
                                    if( !aSize.Width() || !aSize.Height() )
                                        aSize = Application::GetDefaultDevice()->PixelToLogic( aBitmap.GetSizePixel(), MAP_100TH_MM );
                                    else
                                        aSize = Application::GetDefaultDevice()->LogicToLogic( aSize, aBitmap.GetPrefMapMode(), MAP_100TH_MM );
                                    aVDev.DrawBitmap( Point(), aSize, aBitmap );
                                    aMtf.Stop();
                                    aMtf.WindStart();
                                    aMtf.SetPrefMapMode( MAP_100TH_MM );
                                    aMtf.SetPrefSize( aSize );
                                    aGraphic = aMtf;
                                    bHasPreview = bRetValue = sal_True;
                                }
                                aBitmap.ReleaseAccess( pAcc );
                            }
                        }
                    }
                }

                sal_uInt8* pDest = ImplSearchEntry( pBuf, (sal_uInt8*)"%%BoundingBox:", nBytesRead, 14 );
                if ( pDest )
                {
                    nSecurityCount = 100;
                    long nNumb[4];
                    nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
                    pDest += 14;
                    for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
                    {
                        nNumb[ i ] = ImplGetNumber( &pDest, nSecurityCount );
                    }
                    if ( nSecurityCount)
                    {
                        bGraphicLinkCreated = sal_True;
                        GfxLink     aGfxLink( pBuf, nPSSize, GFX_LINK_TYPE_EPS_BUFFER, sal_True ) ;
                        GDIMetaFile aMtf;

                        long nWidth =  nNumb[2] - nNumb[0] + 1;
                        long nHeight = nNumb[3] - nNumb[1] + 1;

                        // if there is no preview -> try with gs to make one
                        if( !bHasPreview )
                        {
                            bHasPreview = RenderAsEMF(pBuf, nBytesRead, aGraphic);
                            if (!bHasPreview)
                                bHasPreview = RenderAsPNG(pBuf, nBytesRead, aGraphic);
                        }

                        // if there is no preview -> make a red box
                        if( !bHasPreview )
                        {
                            MakePreview(pBuf, nBytesRead, nWidth, nHeight,
                                aGraphic);
                        }

                        aMtf.AddAction( (MetaAction*)( new MetaEPSAction( Point(), Size( nWidth, nHeight ),
                                                                          aGfxLink, aGraphic.GetGDIMetaFile() ) ) );
                        CreateMtfReplacementAction( aMtf, rStream, nOrigPos, nPSSize, nPosWMF, nSizeWMF, nPosTIFF, nSizeTIFF );
                        aMtf.WindStart();
                        aMtf.SetPrefMapMode( MAP_POINT );
                        aMtf.SetPrefSize( Size( nWidth, nHeight ) );
                        rGraphic = aMtf;
                        bRetValue = sal_True;
                    }
                }
            }
        }
        if ( !bGraphicLinkCreated )
            delete[] pBuf;
    }
    delete[] pHeader;
    rStream.SetNumberFormatInt(nOldFormat);
    rStream.Seek( nOrigPos );
    return ( bRetValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
