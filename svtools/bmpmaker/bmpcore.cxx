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


#include <comphelper/string.hxx>
#include <tools/color.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/alpha.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include "bmpcore.hxx"

#include <vector>
#include <algorithm>

#undef WRITE_ALPHA_PNG

// -------------------------
// - ImplGetSystemFileName -
// -------------------------

static String ImplGetSystemFileName( const String& rFileName )
{
    String              aRet( rFileName );
    const sal_Unicode   aReplace = DirEntry::GetAccessDelimiter( FSYS_STYLE_HOST ).GetChar( 0 );

    aRet.SearchAndReplaceAll( '/', aReplace );
    aRet.SearchAndReplaceAll( '\\', aReplace );

    return aRet;
}

// --------------
// - BmpCreator -
// --------------

BmpCreator::BmpCreator():pSRS(NULL)
{
}

// -----------------------------------------------------------------------------

BmpCreator::~BmpCreator()
{
}

// -----------------------------------------------------------------------------

void BmpCreator::Message( const String&, sal_uInt8 )
{
}

// -----------------------------------------------------------------------

void BmpCreator::ImplCreate( const ::std::vector< DirEntry >& rInDirs,
                             const DirEntry& rOut,
                             const String& rName,
                             const LangInfo& rLang )
{
    const sal_uInt32    nOldPos = pSRS->Tell();
       const char*         pCollectFile = getenv( "BMP_COLLECT_FILE" );
       SvFileStream*       pCollectStm = pCollectFile ? new SvFileStream( String( pCollectFile, RTL_TEXTENCODING_ASCII_US ),
                                                                          STREAM_WRITE ) : NULL;

    if( pCollectStm )
        pCollectStm->Seek( STREAM_SEEK_TO_END );

    if( !rInDirs.empty() )
    {
        rtl::OString                aLine;
        String                      aInfo, aPrefix, aName( rName ), aString;
        SvFileStream                aOutStream;
        BitmapEx                    aTotalBmpEx;
        DirEntry                    aOutFile( rOut );
        ::std::vector< DirEntry >   aInFiles( rInDirs );
        ::std::vector< String >     aNameVector;
        sal_uInt32                  i;

        for( i = 0; i < aInFiles.size(); i++ )
            aInFiles[ i ] += DirEntry( String( RTL_CONSTASCII_USTRINGPARAM( "xxx.xxx" ) ) );

        // get prefix for files
        if( ( aName.Len() >= 3 ) && ( aName.GetChar( 2 ) != '_' ) )
            aPrefix = String( aName, 0, 3 );
        else
            aPrefix = String( aName, 0, 2 );

            String aNumStr(rtl::OUString::createFromAscii(rLang.maLangDir));

            if( aNumStr.Len() == 1 )
                aNumStr.Insert( '0', 0 );

            aName = DirEntry( aName ).GetBase();
            aName += String( RTL_CONSTASCII_USTRINGPARAM( ".bmp" ) );

        // create output file name
        aOutFile += DirEntry( aName );

        // get number of bitmaps
        while( aLine.indexOf('}') == -1 )
        {
            if( !pSRS->ReadLine( aLine ) )
                break;

            aLine = comphelper::string::stripStart(aLine, ' ');
            aLine = comphelper::string::stripStart(aLine, '\t');
            aLine = comphelper::string::remove(aLine, ';');

            if (comphelper::string::isdigitAsciiString(aLine))
            {
                aString = aPrefix;

                if( atoi( aLine.getStr() ) < 10000 )
                    aString += OUString::number( 0 );

                // search for pngs by default
                String aPngString( aString += rtl::OStringToOUString(aLine, RTL_TEXTENCODING_UTF8) );
                aNameVector.push_back( aPngString += String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) ) );
           }
        }

        if( !aNameVector.size() )
            Message( String( RTL_CONSTASCII_USTRINGPARAM( "WARNING: No imagelist resource found: " ) ).Append( aString ), EXIT_MISSING_RESOURCE );
        else
        {
            // write info
            aInfo = String( RTL_CONSTASCII_USTRINGPARAM( "CREATING ImageList for language: " ) );
            aInfo += String( ::rtl::OUString::createFromAscii( rLang.maLangDir ) );
            aInfo += String( RTL_CONSTASCII_USTRINGPARAM( " [ " ) );

            for( i = 0; i < rInDirs.size(); i++ )
                ( aInfo += rInDirs[ i ].GetFull() ) += String( RTL_CONSTASCII_USTRINGPARAM( "; " ) );

            aInfo += String( RTL_CONSTASCII_USTRINGPARAM( " ]" ) );
            Message( aInfo );

            // create bit vector to hold flags for valid bitmaps
            ::std::vector<bool, std::allocator<bool> >   aValidBmpBitVector( aNameVector.size(), false );
            BitmapEx            aBmpEx;

            for( sal_uInt32 n = 0; n < aNameVector.size(); n++ )
            {
                aBmpEx.SetEmpty();

                for( i = 0; i < aInFiles.size() && aBmpEx.IsEmpty(); i++ )
                {
                    DirEntry aInFile( aInFiles[ i ] );

                    aInFile.SetName( aString = aNameVector[ n ] );
                    bool bPNG = aInFile.Exists();

                    if( !bPNG )
            {
                        aInFile.SetExtension( String( RTL_CONSTASCII_USTRINGPARAM( "bmp" ) ) );
            aString = aInFile.GetName();
            }

                    if( aInFile.Exists() )
                    {
                        const String    aFileName( aInFile.GetFull() );
                        SvFileStream    aIStm( aFileName, STREAM_READ );

                        if( bPNG )
                        {
                            ::vcl::PNGReader aPNGReader( aIStm );
                            aBmpEx = aPNGReader.Read();
                        }
                        else
                            aIStm >> aBmpEx;

                        if( pCollectStm && !aBmpEx.IsEmpty() )
                        {
                            const rtl::OString aCollectString(rtl::OUStringToOString(
                                aFileName, RTL_TEXTENCODING_ASCII_US));
                            pCollectStm->WriteLine( aCollectString );
                        }
                    }
                }

                const Size aSize( aBmpEx.GetSizePixel() );

                if( aBmpEx.IsEmpty() )
                {
                    Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: graphic is missing: " ) ).Append( aString ), EXIT_MISSING_BITMAP );
                }
                else
                {
                    if( aTotalBmpEx.IsEmpty() )
                    {
                        // first bitmap determines metrics of total bitmap
                        Size aTotalSize( aOneSize = aSize );

                        aTotalSize.Width() *= aNameVector.size();
                        aTotalBmpEx = Bitmap( aTotalSize, aBmpEx.GetBitmap().GetBitCount() );
                    }

                    if( ( aSize.Width() > aOneSize.Width() ) || ( aSize.Height() > aOneSize.Height() ) )
                         Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Different dimensions in file: " ) ).Append( aString ), EXIT_DIMENSIONERROR );
                    else
                    {
                        Point           aPoint;
                        const Rectangle aDst( Point( aOneSize.Width() * n, 0L ), aSize );
                        const Rectangle aSrc( aPoint, aSize );

                        if( !aTotalBmpEx.IsEmpty() && !aBmpEx.IsEmpty() && !aDst.IsEmpty() && !aSrc.IsEmpty() )
                        {
                            if( !aTotalBmpEx.IsTransparent() && aBmpEx.IsTransparent() )
                            {
                                const Bitmap aTmpBmp( aTotalBmpEx.GetBitmap() );
                                aTotalBmpEx = BitmapEx( aTmpBmp, AlphaMask( aTmpBmp.CreateMask( COL_LIGHTMAGENTA ) ) );
                            }
                            else if( aTotalBmpEx.IsTransparent() && !aBmpEx.IsTransparent() )
                            {
                                const Bitmap aTmpBmp( aBmpEx.GetBitmap() );
                                aBmpEx = BitmapEx( aTmpBmp, AlphaMask( aTmpBmp.CreateMask( COL_LIGHTMAGENTA ) ) );
                            }

                            aTotalBmpEx.CopyPixel( aDst, aSrc, &aBmpEx );
                            aValidBmpBitVector[ n ] = true;
                        }
                    }
                }
            }

            if( !aTotalBmpEx.IsEmpty() )
            {
                // do we have invalid bitmaps?
                if( ::std::find( aValidBmpBitVector.begin(), aValidBmpBitVector.end(), false ) != aValidBmpBitVector.end() )
                {
                    Bitmap              aTmpBmp( aTotalBmpEx.GetBitmap() );
                    BitmapWriteAccess*  pAcc = aTmpBmp.AcquireWriteAccess();

                    if( pAcc )
                    {
                        pAcc->SetLineColor( Color( COL_LIGHTGREEN ) );

                        for( sal_uInt32 n = 0; n < aValidBmpBitVector.size(); n++ )
                        {
                            if( !aValidBmpBitVector[ n ] )
                            {
                                const Rectangle aDst( Point( aOneSize.Width() * n, 0L ), aOneSize );

                                pAcc->DrawRect( aDst );
                                pAcc->DrawLine( aDst.TopLeft(), aDst.BottomRight() );
                                pAcc->DrawLine( aDst.TopRight(), aDst.BottomLeft() );
                            }
                        }

                        aTmpBmp.ReleaseAccess( pAcc );

                        if( aTotalBmpEx.IsAlpha() )
                            aTotalBmpEx = BitmapEx( aTmpBmp, aTotalBmpEx.GetAlpha() );
                        else if( aTotalBmpEx.IsTransparent() )
                            aTotalBmpEx = BitmapEx( aTmpBmp, aTotalBmpEx.GetMask() );
                        else
                            aTotalBmpEx = aTmpBmp;
                    }
                }

                // write output file
                const String aOutFileName( aOutFile.GetFull() );

                aOutStream.Open( aOutFileName, STREAM_WRITE | STREAM_TRUNC );

                if( !aOutStream.IsOpen() )
                    Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not open output file: " ) ).Append( aOutFileName ), EXIT_IOERROR );
                else
                {
                    if( aOutFileName.Search( String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) ) ) != STRING_NOTFOUND )
                    {
                        ::vcl::PNGWriter aPNGWriter( aTotalBmpEx );
                        aPNGWriter.Write( aOutStream );
                    }
                    else
                        aOutStream << aTotalBmpEx;

                    if( aOutStream.GetError()  )
                        Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not write to output file: " ) ).Append( aOutFileName ), EXIT_IOERROR );
                    else
                        Message( String( RTL_CONSTASCII_USTRINGPARAM( "Successfully generated ImageList " ) ).Append( aOutFileName ) );

                    aOutStream.Close();
                }
            }
            else
                Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Could not generate  " ) ).Append( aOutFile.GetFull() ), EXIT_COMMONERROR );

            Message( rtl::OUString(' ') );
        }
    }
    else
        Message( rtl::OUString("ERROR: SOLARSRC environment variable not set!"), EXIT_MISSING_SOLARSRC_ENV );

    pSRS->Seek( nOldPos );
    delete pCollectStm;
}

// -----------------------------------------------------------------------------

void BmpCreator::Create( const String& rSRSName,
                         const ::std::vector< String >& rInDirs,
                         const String& rOutName,
                         const LangInfo& rLang )
{
    DirEntry                    aFileName( ImplGetSystemFileName( rSRSName ) ), aOutDir( ImplGetSystemFileName( rOutName ) );
    ::std::vector< DirEntry >   aInDirs;
    sal_Bool                        bDone = sal_False;

    aFileName.ToAbs();
    aOutDir.ToAbs();

    // create vector of all valid input directories,
    // including language subdirectories
    for( sal_uInt32 i = 0; i < rInDirs.size(); i++ )
    {
        DirEntry aInDir( ImplGetSystemFileName( rInDirs[ i ] ) );

        aInDir.ToAbs();

        if( aInDir.Exists() )
        {
            DirEntry aLangInDir( aInDir );

            if( ( aLangInDir += DirEntry( ::rtl::OUString::createFromAscii( rLang.maLangDir ) ) ).Exists() )
                aInDirs.push_back( aLangInDir );

            aInDirs.push_back( aInDir );
        }
    }

    pSRS = new SvFileStream ( aFileName.GetFull(), STREAM_STD_READ );

    if( pSRS->GetError() )
        Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: Kein SRS file!" ) ), EXIT_NOSRSFILE );
    else
    {
        String      aText;
        rtl::OString aByteText;
        sal_Bool        bLangDep = sal_False;

        do
        {
            do
            {
                if (!pSRS->ReadLine(aByteText))
                    break;
            }
            while ( aByteText.indexOfL(RTL_CONSTASCII_STRINGPARAM("ImageList")) == -1 );

            do
            {
                if (!pSRS->ReadLine( aByteText ) )
                    break;
            }
            while ( aByteText.indexOfL(RTL_CONSTASCII_STRINGPARAM("File")) == -1 );
            aText = rtl::OStringToOUString(aByteText, RTL_TEXTENCODING_ASCII_US);

            const String aName( aText.GetToken( 1, '"' ) );

            do
            {
                if( !bLangDep &&
                    aByteText.indexOfL(RTL_CONSTASCII_STRINGPARAM("File")) != -1 &&
                    aByteText.indexOf('[') != -1 &&
                    aByteText.indexOf(']') != -1 )
                {
                    bLangDep = sal_True;
                }

                if (!pSRS->ReadLine(aByteText))
                    break;
            }
            while (aByteText.indexOfL(RTL_CONSTASCII_STRINGPARAM("IdList")) == -1);
            aText = rtl::OStringToOUString(aByteText, RTL_TEXTENCODING_ASCII_US);

            // if image list is not language dependent, don't do anything for languages except german
            if( aText.Len() )
            {
                bDone = sal_True;
                ImplCreate( aInDirs, aOutDir, aName, rLang );
            }
/*          else if( ( rLang.mnLangNum != 49 ) && !bLangDep )
            {
                Message( String( RTL_CONSTASCII_USTRINGPARAM( "INFO: ImageList is not language dependent! Nothing to do for this language." ) ) );
                bDone = sal_True;
            }*/
        }
        while ( aText.Len() );
    }

    if( !bDone )
        Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: No ImageList found in SRS file!" ) ), EXIT_NOIMGLIST );

    delete pSRS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
