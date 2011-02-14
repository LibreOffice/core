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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <cstdio>
#include <csignal>
#include <vector>
#include <set>
#include <map>

#include <rtl/crc.h>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/pngread.hxx>

#include "svl/solar.hrc"

#define EXIT_NOERROR        0x00000000
#define EXIT_INVALIDFILE    0x00000001
#define EXIT_COMMONERROR    0x80000000

// ----------
// - BmpSum -
// ----------

class BmpSum
{
private:

    sal_uInt32      cExitCode;

    sal_Bool            GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rSwitchParam );
    sal_Bool            GetCommandOptions( const ::std::vector< String >& rArgs, const String& rSwitch, ::std::vector< String >& rSwitchParams );

    void            SetExitCode( sal_uInt8 cExit )
                    {
                        if( ( EXIT_NOERROR == cExitCode ) || ( cExit != EXIT_NOERROR ) )
                            cExitCode = cExit;
                    }
    void            ShowUsage();
    void            Message( const String& rText, sal_uInt8 cExitCode );

    sal_uInt64      GetCRC( const BitmapEx& rBmpEx );

    void            ProcessFile( const String& rBmpFileName );
    void            ProcessFileList( const String& rInFileList, const String& rOutFileList, const String& rOutPath );

public:
//
                    BmpSum();
                    ~BmpSum();

    int             Start( const ::std::vector< String >& rArgs );
};

// -----------------------------------------------------------------------------

BmpSum::BmpSum()
{
}

// -----------------------------------------------------------------------------

BmpSum::~BmpSum()
{
}

// -----------------------------------------------------------------------

sal_Bool BmpSum::GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rParam )
{
    sal_Bool bRet = sal_False;

    for( int i = 0, nCount = rArgs.size(); ( i < nCount ) && !bRet; i++ )
    {
        String  aTestStr( '-' );

        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.CompareIgnoreCaseToAscii( rArgs[ i ] ) == COMPARE_EQUAL )
            {
                bRet = sal_True;

                if( i < ( nCount - 1 ) )
                    rParam = rArgs[ i + 1 ];
                else
                    rParam = String();
            }

            if( 0 == n )
                aTestStr = '/';
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool BmpSum::GetCommandOptions( const ::std::vector< String >& rArgs, const String& rSwitch, ::std::vector< String >& rParams )
{
    sal_Bool bRet = sal_False;

    for( int i = 0, nCount = rArgs.size(); ( i < nCount ); i++ )
    {
        String  aTestStr( '-' );

        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.CompareIgnoreCaseToAscii( rArgs[ i ] ) == COMPARE_EQUAL )
            {
                if( i < ( nCount - 1 ) )
                    rParams.push_back( rArgs[ i + 1 ] );
                else
                    rParams.push_back( String() );

                break;
            }

            if( 0 == n )
                aTestStr = '/';
        }
    }

    return( rParams.size() > 0 );
}

// -----------------------------------------------------------------------

void BmpSum::Message( const String& rText, sal_uInt8 nExitCode )
{
    if( EXIT_NOERROR != nExitCode )
        SetExitCode( nExitCode );

    ByteString aText( rText, RTL_TEXTENCODING_UTF8 );
    aText.Append( "\r\n" );
    fprintf( stderr, aText.GetBuffer() );
}

// -----------------------------------------------------------------------------

void BmpSum::ShowUsage()
{
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Usage:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmpsum bmp_inputfile" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmpsum -i input_filelist -o output_filelist [-p path_for_copied_bitmaps]" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Options:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Examples:" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmpsum /home/test.bmp" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmpsum -i /home/inlist.txt -o /home/outlist.txt" ) ), EXIT_NOERROR );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    bmpsum -i /home/inlist.txt -o /home/outlist.txt -p /home/outpath" ) ), EXIT_NOERROR );
}

// -----------------------------------------------------------------------------

int BmpSum::Start( const ::std::vector< String >& rArgs )
{
    cExitCode = EXIT_NOERROR;

    if( rArgs.size() >= 1 )
    {
        String  aInFileList, aOutFileList, aOutPath;

        if( GetCommandOption( rArgs, 'i', aInFileList ) &&
            GetCommandOption( rArgs, 'o', aOutFileList ) )
        {
            GetCommandOption( rArgs, 'p', aOutPath );
            ProcessFileList( aInFileList, aOutFileList, aOutPath );
        }
        else
        {
            ProcessFile( rArgs[ 0 ] );
        }
    }
    else
    {
        ShowUsage();
        cExitCode = EXIT_COMMONERROR;
    }

    return cExitCode;
}

// -----------------------------------------------------------------------------

sal_uInt64 BmpSum::GetCRC( const BitmapEx& rBmpEx )
{
    Bitmap              aBmp( rBmpEx.GetBitmap() );
    BitmapReadAccess*   pRAcc = aBmp.AcquireReadAccess();
    AlphaMask           aAlpha;
    BitmapReadAccess*   pAAcc = NULL;
    sal_uInt64          nRet = 0;
    sal_uInt32          nCrc = 0;

    if( rBmpEx.IsTransparent() )
    {
        aAlpha = rBmpEx.GetAlpha();
        pAAcc = aAlpha.AcquireReadAccess();
    }

    if( pRAcc && pRAcc->Width() && pRAcc->Height() )
    {
        SVBT32 aBT32;

        for( long nY = 0; nY < pRAcc->Height(); ++nY )
        {
            for( long nX = 0; nX < pRAcc->Width(); ++nX )
            {
                const BitmapColor aCol( pRAcc->GetColor( nY, nX ) );

                UInt32ToSVBT32( aCol.GetRed(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( aCol.GetGreen(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                UInt32ToSVBT32( aCol.GetBlue(), aBT32 );
                nCrc = rtl_crc32( nCrc, aBT32, 4 );

                if( pAAcc )
                {
                    const BitmapColor aMaskCol( pAAcc->GetColor( nY, nX ) );

                    UInt32ToSVBT32( aMaskCol.GetRed(), aBT32 );
                    nCrc = rtl_crc32( nCrc, aBT32, 4 );

                    UInt32ToSVBT32( aMaskCol.GetGreen(), aBT32 );
                    nCrc = rtl_crc32( nCrc, aBT32, 4 );

                    UInt32ToSVBT32( aMaskCol.GetBlue(), aBT32 );
                    nCrc = rtl_crc32( nCrc, aBT32, 4 );
                }
            }
        }

        nRet = ( ( (sal_uInt64) pRAcc->Width() ) << 48 ) |
               ( ( (sal_uInt64) pRAcc->Height() ) << 32 ) |
               ( (sal_uInt64) nCrc );
    }

    if( pAAcc )
        aAlpha.ReleaseAccess( pAAcc);

    aBmp.ReleaseAccess( pRAcc );

    return nRet;
}

// -----------------------------------------------------------------------------

void BmpSum::ProcessFile( const String& rBmpFileName )
{
    SvFileStream aIStm( rBmpFileName, STREAM_READ );

    if( aIStm.IsOpen() )
    {
        BitmapEx aBmpEx;

        aIStm >> aBmpEx;

        if( !aBmpEx.IsEmpty() )
        {
            fprintf( stdout, "%" SAL_PRIuUINT64 "\r\n", GetCRC( aBmpEx ) );
        }
        else
        {
            aIStm.ResetError();
            aIStm.Seek( 0 );

               ::vcl::PNGReader aPngReader( aIStm );

            aBmpEx = aPngReader.Read();

              if( !aBmpEx.IsEmpty() )
            {
                fprintf( stdout, "%" SAL_PRIuUINT64 "\r\n", GetCRC( aBmpEx ) );
            }
            else
                Message( String( RTL_CONSTASCII_USTRINGPARAM( "file not valid" ) ), EXIT_INVALIDFILE );
        }
    }
}

// -----------------------------------------------------------------------------

void BmpSum::ProcessFileList( const String& rInFileList,
                              const String& rOutFileList,
                              const String& rOutPath )
{
    SvFileStream    aIStm( rInFileList, STREAM_READ );
    SvFileStream    aOStm( rOutFileList, STREAM_WRITE | STREAM_TRUNC );
    const DirEntry  aBaseDir( rOutPath );

    if( rOutPath.Len() )
        aBaseDir.MakeDir();

    if( aIStm.IsOpen() && aOStm.IsOpen() )
    {
        ByteString                  aReadLine;
        ::std::set< ByteString >    aFileNameSet;

        while( aIStm.ReadLine( aReadLine ) )
        {
            if( aReadLine.Len() )
                aFileNameSet.insert( aReadLine );

            if( aReadLine.Search( "enus" ) != STRING_NOTFOUND )
            {
                static const char* aLanguages[] =
                {
                    "chinsim",
                    "chintrad",
                    "dtch",
                    "enus",
                    "fren",
                    "hebrew"
                    "ital",
                    "japn",
                    "korean",
                    "pol",
                    "poln",
                    "port",
                    "russ",
                    "span",
                    "turk"
                };

                for( sal_uInt32 n = 0; n < 14; ++n )
                {
                    ByteString aLangPath( aReadLine );

                    aLangPath.SearchAndReplace( "enus", aLanguages[ n ] );

                    DirEntry aTestFile( aLangPath );

                    if( aTestFile.Exists() )
                        aFileNameSet.insert( aLangPath );
                }
            }

            aReadLine.Erase();
        }

        aIStm.Close();

        ::std::set< ByteString >::iterator aIter( aFileNameSet.begin() );
        ::std::map< sal_uInt64, ::std::vector< ByteString > > aFileNameMap;

        while( aIter != aFileNameSet.end() )
        {
            ByteString      aStr( *aIter++ );
            SvFileStream    aBmpStm( String( aStr.GetBuffer(), RTL_TEXTENCODING_ASCII_US ), STREAM_READ );
            sal_uInt64      nCRC = 0;

            if( aBmpStm.IsOpen() )
            {
                BitmapEx aBmpEx;

                aBmpStm >> aBmpEx;

                if( !aBmpEx.IsEmpty() )
                    nCRC = GetCRC( aBmpEx );
                else
                {
                    aBmpStm.ResetError();
                    aBmpStm.Seek( 0 );

                       ::vcl::PNGReader aPngReader( aBmpStm );

                    aBmpEx = aPngReader.Read();

                      if( !aBmpEx.IsEmpty() )
                           nCRC = GetCRC( aBmpEx );

                    else
                        fprintf( stderr, "%s could not be opened\n", aStr.GetBuffer() );
               }

                aBmpStm.Close();
            }

            if( nCRC )
            {
                ::std::map< sal_uInt64, ::std::vector< ByteString > >::iterator aFound( aFileNameMap.find( nCRC ) );

                if( aFound != aFileNameMap.end() )
                    (*aFound).second.push_back( aStr );
                else
                {
                    ::std::vector< ByteString > aVector( 1, aStr );
                    aFileNameMap[ nCRC ]  = aVector;
                }

            }
            else
            {
                ::std::vector< ByteString > aVector( 1, aStr );
                aFileNameMap[ nCRC ]  = aVector;
            }
        }

        ::std::map< sal_uInt64, ::std::vector< ByteString > >::iterator aMapIter( aFileNameMap.begin() );
        sal_uInt32 nFileCount = 0;

        while( aMapIter != aFileNameMap.end() )
        {
            ::std::pair< const sal_uInt64, ::std::vector< ByteString > > aPair( *aMapIter++ );
            ::std::vector< ByteString > aFileNameVector( aPair.second );

            // write new entries
            for( sal_uInt32 i = 0; i < aFileNameVector.size(); ++i )
            {
                ByteString  aStr( ByteString::CreateFromInt64( aPair.first ) );
                ByteString  aFileName( aFileNameVector[ i ] );
                DirEntry    aSrcFile( aFileName );

                aStr += '\t';
                aStr += aFileName;

                aOStm.WriteLine( aStr );

                // copy bitmap
                if( rOutPath.Len() )
                {
                    if( aFileName.Search( ":\\" ) != STRING_NOTFOUND )
                        aFileName.Erase( 0, aFileName.Search( ":\\" ) + 2 );

                    aFileName.SearchAndReplaceAll( '\\', '/' );

                    sal_uInt16  nTokenCount = aFileName.GetTokenCount( '/' );
                    DirEntry    aNewDir( aBaseDir );

                    for( sal_uInt16 n = 0; ( n < nTokenCount - 1 ); n++ )
                    {
                        aNewDir += DirEntry( aFileName.GetToken( n, '/' ) );
                        aNewDir.MakeDir();
                    }

                    aNewDir += DirEntry( aFileName.GetToken( nTokenCount - 1, '/' ) );
                    aSrcFile.CopyTo( aNewDir, FSYS_ACTION_COPYFILE );
                }
            }

            ++nFileCount;
        }

        fprintf(
            stdout, "unique file count: %lu",
            sal::static_int_cast< unsigned long >(nFileCount) );
    }
}

// --------
// - Main -
// --------

int main( int nArgCount, char* ppArgs[] )
{
#ifdef UNX
    static char aDisplayVar[ 1024 ];

    strcpy( aDisplayVar, "DISPLAY=" );
    putenv( aDisplayVar );
#endif

    ::std::vector< String > aArgs;
    BmpSum                  aBmpSum;

    InitVCL( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >() );

    for( int i = 1; i < nArgCount; i++ )
        aArgs.push_back( String( ppArgs[ i ], RTL_TEXTENCODING_ASCII_US ) );

    return aBmpSum.Start( aArgs );
}
