/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impimagetree.cxx,v $
 * $Revision: 1.18 $
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
#include "precompiled_vcl.hxx"

#include <cstdio>
#include <ctype.h>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/pngread.hxx>
#include <unotools/ucbstreamhelper.hxx>
#ifndef _UNTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#include <unotools/datetime.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/packages/zip/XZipFileAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

#include "vcl/impimagetree.hxx"

#include <vector>
#include <hash_map>

#define DEFAULT_PROJECTNAME         "res"
#define IMAGES_ZIPFILENAME_PREFIX   "images"
#define IMAGES_ZIPFILENAME_BRAND    "_brand"
#define IMAGES_ZIPFILENAME_SUFFIX   ".zip"
#define IMAGES_CACHEDIR             "imagecache"

using namespace ::com::sun::star;

ImplZipAccessor::ImplZipAccessor()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    mxFileAccess.set( xFactory->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
        uno::UNO_QUERY );
}

// -----------------------------------------------------------------------

ImplZipAccessor::~ImplZipAccessor()
{
    Clear();
}

// -----------------------------------------------------------------------

void ImplZipAccessor::Update( const ::rtl::OUString& rSymbolsStyle )
{
#ifdef DEBUG
    fprintf( stderr, "\nUpdating symbols for style: %s\n", ByteString( String( rSymbolsStyle ), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif

    Clear();

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    const ::rtl::OUString               aZipFileName( ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_PREFIX ) );
    rtl::OUString                       aBrandPath( RTL_CONSTASCII_USTRINGPARAM( "$BRAND_BASE_DIR/share/config" ) );
    rtl::OUString                       aOOOPath( RTL_CONSTASCII_USTRINGPARAM( "$OOO_BASE_DIR/share/config" ) );
    ::std::vector< ::rtl::OUString >    aTmpURLVector;

    rtl::Bootstrap::expandMacros( aBrandPath );
    rtl::Bootstrap::expandMacros( aOOOPath );

    // Theme Branding Zip
    if( rSymbolsStyle.getLength() )
    {
        INetURLObject   aThemeBrandURL( aBrandPath );
        ::rtl::OUString aThemeBrandZip( aZipFileName );

        ( aThemeBrandZip += ::rtl::OUString::createFromAscii( "_" ) ) += rSymbolsStyle;
        aThemeBrandZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_BRAND );
        aThemeBrandZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_SUFFIX );
        aThemeBrandURL.Append( aThemeBrandZip );

        if( mxFileAccess->exists( aThemeBrandURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            aTmpURLVector.push_back( aThemeBrandURL.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    // Branding Zip
    INetURLObject   aBrandURL( aBrandPath );
    ::rtl::OUString aBrandZip( aZipFileName );

    aBrandZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_BRAND );
    aBrandZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_SUFFIX );
    aBrandURL.Append( aBrandZip );

    if( mxFileAccess->exists( aBrandURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
        aTmpURLVector.push_back( aBrandURL.GetMainURL( INetURLObject::NO_DECODE ) );

    // Theme Zip
    if( rSymbolsStyle.getLength() )
    {
        INetURLObject   aThemeURL( aOOOPath );
        ::rtl::OUString aThemeZip( aZipFileName );

        ( aThemeZip += ::rtl::OUString::createFromAscii( "_" ) ) += rSymbolsStyle;
        aThemeZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_SUFFIX );
        aThemeURL.Append( aThemeZip );

        if( mxFileAccess->exists( aThemeURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            aTmpURLVector.push_back( aThemeURL.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    // Default Zip
    if( rSymbolsStyle.getLength() )
    {
        INetURLObject   aDefaultURL( aOOOPath );
        ::rtl::OUString aDefaultZip( aZipFileName );

        aDefaultZip += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_SUFFIX );
        aDefaultURL.Append( aDefaultZip );

        if( mxFileAccess->exists( aDefaultURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            aTmpURLVector.push_back( aDefaultURL.GetMainURL( INetURLObject::NO_DECODE ) );
    }

    if( xFactory.is() )
    {
        for( unsigned int i = 0; i < aTmpURLVector.size(); ++i )
        {
            try
            {
                uno::Reference< packages::zip::XZipFileAccess > xZipAcc( xFactory->createInstance(
                    ::rtl::OUString::createFromAscii( "com.sun.star.packages.zip.ZipFileAccess" ) ),
                    uno::UNO_QUERY );

                if( xZipAcc.is() )
                {
                    uno::Reference< lang::XInitialization > xInit( xZipAcc, uno::UNO_QUERY );

                    if( xInit.is() )
                    {
                        uno::Sequence< uno::Any >   aInitSeq( 1 );
                        const ::rtl::OUString&      rZipURL = aTmpURLVector[ i ];

                        if( rZipURL.getLength() )
                        {
                            uno::Reference< container::XNameAccess > xNameAcc;

                            aInitSeq[ 0 ] <<= rZipURL;
                            xInit->initialize( aInitSeq );
                            xNameAcc.set( xZipAcc, uno::UNO_QUERY );

                            if( xNameAcc.is() && xNameAcc->getElementNames().getLength() )
                            {
                                maURLVector.push_back( rZipURL );
                                maZipAccVector.push_back( xZipAcc );
                                maNameAccVector.push_back( xNameAcc );
#ifdef DEBUG
                                fprintf( stderr, "Current set has symbols from archive: %s\n", ByteString( String( rZipURL ), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif
                            }
                        }
                    }
                }
            }
            catch( const uno::Exception& )
            {
            }
        }
    }
}

// -----------------------------------------------------------------------

void ImplZipAccessor::Clear()
{
    maURLVector.clear();
    maZipAccVector.clear();
    maNameAccVector.clear();
}

// -----------------------------------------------------------------------

bool ImplZipAccessor::HasEntries() const
{
    return( maNameAccVector.size() > 0 );
}

// -----------------------------------------------------------------------

uno::Reference< io::XInputStream > ImplZipAccessor::GetByName( const ::rtl::OUString& rName ) const
{
    uno::Reference< io::XInputStream > xRet;

#ifdef DEBUG
    fprintf( stderr, "Looking for file: %s\n", ByteString( String( rName ), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif

    for( unsigned int i = 0; ( i < maNameAccVector.size() ) && !xRet.is(); ++i )
    {
        if( maNameAccVector[ i ]->hasByName( rName ) )
        {
            try
            {
                if( maNameAccVector[ i ]->getByName( rName ) >>= xRet )
                {
#ifdef DEBUG
                    fprintf( stderr, "Found in archive: %s\n\n", ByteString( String( maURLVector[ i ] ), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
#endif


                    break;
                }
            }
            catch( const uno::Exception & )
            {
            }
        }
    }

    return( xRet );
}


// -----------------
// - ImplImageTree -
// -----------------

typedef ::std::hash_map< ::rtl::OUString, BitmapEx, ::rtl::OUStringHash > BmpExHashMap;
static BmpExHashMap aBmpExHashMap;

// -----------------------------------------------------------------------

ImplImageTree::ImplImageTree() :
    mbInit( false ),
    maSymbolsStyle( Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName() )
{
}

// -----------------------------------------------------------------------

ImplImageTree::~ImplImageTree()
{
}

// -----------------------------------------------------------------------

void ImplImageTree::cleanup()
{
    ImplImageTreeSingletonRef aCleaner;

    aCleaner->maZipAcc.Clear();
    aCleaner->mxFileAccess.clear();
    aCleaner->mxPathSettings.clear();

    BmpExHashMap aTmp;
    aBmpExHashMap.swap( aTmp );
}

// -----------------------------------------------------------------------

bool ImplImageTree::implInit()
{
    if( !mbInit )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

        if( xFactory.is() )
        {
            // #137795# protect against exceptions in service instantiation
            try
            {
                mxPathSettings.set( xFactory->createInstance(
                    ::rtl::OUString::createFromAscii( "com.sun.star.util.PathSettings" ) ),
                    uno::UNO_QUERY );
                mxFileAccess.set( xFactory->createInstance(
                    ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

                if( mxPathSettings.is() && mxFileAccess.is() )
                {
                    maZipAcc.Update( maSymbolsStyle );
                    // implCheckUserCache();
                }
            }
            catch( const uno::Exception& )
            {
                maZipAcc.Clear();
                mxPathSettings.clear();
                mxFileAccess.clear();
            }
        }
    }

    return( mbInit = maZipAcc.HasEntries() );
}

// -----------------------------------------------------------------------

const ::rtl::OUString& ImplImageTree::implGetUserDirURL() const
{
    static ::rtl::OUString aRet;

    if( !aRet.getLength() && mxPathSettings.is() && mxFileAccess.is() )
    {
        const ::rtl::OUString   aImagesCacheDir( ::rtl::OUString::createFromAscii( IMAGES_CACHEDIR ) );
        uno::Any                aAny( mxPathSettings->getPropertyValue( ::rtl::OUString::createFromAscii( "UserConfig" ) ) );

        if( ( aAny >>= aRet ) && aRet.getLength() )
        {
            INetURLObject aCacheURL( aRet );

            aCacheURL.Append( aImagesCacheDir );

            try
            {
                mxFileAccess->createFolder( aRet = aCacheURL.GetMainURL( INetURLObject::NO_DECODE ) );
            }
            catch( const ucb::CommandAbortedException& )
            {
            }
            catch( const uno::Exception& )
            {
            }
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------

::rtl::OUString ImplImageTree::implGetUserFileURL( const ::rtl::OUString& rName ) const
{
    INetURLObject aFileURL( implGetUserDirURL() );

    aFileURL.Append( rName );

    return( aFileURL.GetMainURL( INetURLObject::NO_DECODE ) );
}

// -----------------------------------------------------------------------

void ImplImageTree::implCheckUserCache()
{
/*
    const ::rtl::OUString& rZipURL = implGetZipFileURL();
    const ::rtl::OUString& rUserDirURL = implGetUserDirURL();

    if( rZipURL.getLength() && rUserDirURL.getLength() )
    {
        try
        {
            ::DateTime aZipDateTime, aCacheFileDateTime;
            const uno::Sequence< ::rtl::OUString > aCacheFiles( mxFileAccess->getFolderContents( rUserDirURL, false ) );

            ::utl::typeConvert( mxFileAccess->getDateTimeModified( rZipURL ), aZipDateTime );

            for( sal_Int32 i = 0; i < aCacheFiles.getLength(); ++i )
            {
                const ::rtl::OUString aCacheFile( aCacheFiles[ i ] );

                try
                {
                    ::utl::typeConvert( mxFileAccess->getDateTimeModified( aCacheFile ), aCacheFileDateTime );

                    if( aZipDateTime > aCacheFileDateTime )
                        mxFileAccess->kill( aCacheFile );
                }
                catch( const ucb::CommandAbortedException& )
                {
                }
                catch( const uno::Exception& )
                {
                }
            }
        }
        catch( const ucb::CommandAbortedException& )
        {
        }
        catch( const uno::Exception& )
        {
        }
    }
*/
}

// ------------------------------------------------------------------------------

bool ImplImageTree::implLoadFromStream( SvStream& rIStm,
                                        const ::rtl::OUString& rFileName,
                                        BitmapEx& rReturn )
{
    static const ::rtl::OUString    aPNGExtension( ::rtl::OUString::createFromAscii( "png" ) );
    const sal_Int32                 nDotPos = rFileName.lastIndexOf( '.' );
    const sal_uInt32                nStmPos = rIStm.Tell();

    if( ( -1 != nDotPos ) && ( rFileName.lastIndexOf( aPNGExtension ) == ( nDotPos + 1 ) ) )
    {
        ::vcl::PNGReader aPNGReader( rIStm );
        rReturn = aPNGReader.Read();
    }

    if( rReturn.IsEmpty() )
    {
        rIStm.Seek( nStmPos );
        rIStm.ResetError();
        rIStm >> rReturn;
    }

    return( !rReturn.IsEmpty() );
}

// ------------------------------------------------------------------------------

::std::auto_ptr< SvStream > ImplImageTree::implGetStream( const uno::Reference< io::XInputStream >& rxIStm ) const
{
    ::std::auto_ptr< SvStream > apRet;

    // create a seekable memory stream from the non-seekable XInputStream
    if( rxIStm.is() )
    {
        const sal_Int32             nBufferSize = 32768;
        sal_Int32                   nRead;
        uno::Sequence < sal_Int8 >  aReadSeq( nBufferSize );

        apRet.reset( new SvMemoryStream( nBufferSize, nBufferSize ) );

        do
        {
            nRead = rxIStm->readBytes ( aReadSeq, nBufferSize );
            apRet->Write( aReadSeq.getConstArray(), nRead );
        }
        while ( nRead == nBufferSize );

        if( apRet->Tell() > 0 )
            apRet->Seek( 0 );
        else
            apRet.reset();
    }

    return apRet;
}

// ------------------------------------------------------------------------------

void ImplImageTree::implUpdateSymbolsStyle( const ::rtl::OUString& rSymbolsStyle )
{
    if ( rSymbolsStyle != maSymbolsStyle )
    {
        maSymbolsStyle = rSymbolsStyle;
        if ( mbInit )
        {
            maZipAcc.Clear();
            mxFileAccess.clear();
            mxPathSettings.clear();

            BmpExHashMap aTmp;
            aBmpExHashMap.swap( aTmp );

            mbInit = false;
        }
    }
}

// ------------------------------------------------------------------------------

bool ImplImageTree::loadImage( const ::rtl::OUString& rName,
        const ::rtl::OUString& rSymbolsStyle,
        BitmapEx& rReturn,
        bool bSearchLanguageDependent )
{
    implUpdateSymbolsStyle( rSymbolsStyle );

    const BmpExHashMap::const_iterator  aBmpExFindIter( aBmpExHashMap.find( rName ) );

    if( aBmpExFindIter != aBmpExHashMap.end() )
        rReturn = (*aBmpExFindIter).second;
    else
    {
        if( !rReturn.IsEmpty() )
            rReturn.SetEmpty();

        if( maZipAcc.HasEntries() || ( implInit() && maZipAcc.HasEntries() ) )
        {
            if( bSearchLanguageDependent )
            {
                // try to get image from local subdirectory
                const ::rtl::OUString   aDash( ::rtl::OUString::createFromAscii( "-" ) );
                const lang::Locale&     rLocale = Application::GetSettings().GetUILocale();
                ::rtl::OUString         aSubDir[3]; // array must be expanded if more locale variants are checked!
                int                     nSubDirs=0;
                ::rtl::OUString         aLocaleStr( rLocale.Language );

                aSubDir[nSubDirs++] = aLocaleStr;
                if( rLocale.Country.getLength() )
                {
                    ( aLocaleStr += aDash ) += rLocale.Country;
                    aSubDir[nSubDirs++] = aLocaleStr;

                    if( rLocale.Variant.getLength() )
                    {
                        ( aLocaleStr += aDash ) += rLocale.Variant;
                        aSubDir[nSubDirs++] = aLocaleStr;
                    }
                }

                for( --nSubDirs; nSubDirs >= 0; nSubDirs-- )
                {
                    // check all locale variants, starting with the most detailed one
                    if( aSubDir[nSubDirs].getLength() )
                    {
                        const sal_Int32 nPos = rName.lastIndexOf( '/' );

                        if( -1 != nPos )
                        {
                            ::rtl::OUString aLocaleName( rName.copy( 0, nPos + 1 ) );

                            aLocaleName += aSubDir[nSubDirs];
                            aLocaleName += ::rtl::OUString::createFromAscii( "/" );
                            aLocaleName += rName.copy( nPos + 1 );

                            try
                            {
                                uno::Reference< io::XInputStream > xIStm( maZipAcc.GetByName( aLocaleName ) );

                                if( xIStm.is() )
                                {
                                    ::std::auto_ptr< SvStream > apRet( implGetStream( xIStm ) );

                                    if( apRet.get() )
                                        implLoadFromStream( *apRet, aLocaleName, rReturn );
                                }
                            }
                            catch( const uno::Exception & )
                            {
                            }
                        }
                    }
                }
            }

            if( rReturn.IsEmpty() )
            {
                try
                {
                    uno::Reference< io::XInputStream > xIStm( maZipAcc.GetByName( rName ) );

                    if( xIStm.is() )
                    {
                        ::std::auto_ptr< SvStream > apRet( implGetStream( xIStm ) );

                        if( apRet.get() )
                            implLoadFromStream( *apRet, rName, rReturn );
                    }
                }
                catch( const uno::Exception & )
                {
                }
            }

            if( rReturn.IsEmpty() )
            {
                ::std::auto_ptr< SvStream > apIStm( ::utl::UcbStreamHelper::CreateStream( implGetUserFileURL( rName ), STREAM_READ ));

                if( apIStm.get() )
                    *apIStm >> rReturn;
            }
        }
        else
        {
            // HACK for old setup!!! search in filesystem relative to application root
            ::rtl::OUString aAppDir( Application::GetAppFileName() );
            sal_Int32       nPos = aAppDir.lastIndexOf( '/' );

            if( -1 == nPos )
                nPos = aAppDir.lastIndexOf( '\\' );

            if( -1 != nPos )
            {
                String aURLStr;

                aAppDir = aAppDir.copy( 0, nPos  );

                if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aAppDir, aURLStr ) )
                {
                    INetURLObject   aURL( aURLStr );
                    sal_Int32       nIndex = 0;

                    do
                    {
                        aURL.Append( rName.getToken( 0, '/', nIndex ) );
                    }
                    while( nIndex >= 0 );

                    aURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );

                    ::std::auto_ptr< SvStream > apIStm( ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_READ ) );

                    if( apIStm.get() )
                    {
                        implLoadFromStream( *apIStm, aURLStr, rReturn );
                        apIStm.reset();
                    }

                    if( rReturn.IsEmpty() )
                    {
                        // HACK for old setup!!! try to look in ../share/config
                        nPos = aAppDir.lastIndexOf( '/' );

                        if( -1 == nPos )
                            nPos = aAppDir.lastIndexOf( '\\' );

                        aAppDir = aAppDir.copy( 0, nPos );

                        if( ( -1 != nPos ) && ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aAppDir, aURLStr ) )
                        {
                            aURL = INetURLObject( aURLStr );
                            aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "share" ) ) );
                            aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "config" ) ) );
                            nIndex = 0;

                            do
                            {
                                aURL.Append( rName.getToken( 0, '/', nIndex ) );
                            }
                            while( nIndex >= 0 );

                            aURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );

                            apIStm.reset( ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_READ ) );

                            if( apIStm.get() )
                                implLoadFromStream( *apIStm, aURLStr, rReturn );
                        }
                    }
                }
            }
        }
    }

    if( !rReturn.IsEmpty() )
        aBmpExHashMap[ rName.intern() ] = rReturn;

    return( !rReturn.IsEmpty() );
}

// ------------------------------------------------------------------------------

void ImplImageTree::addUserImage( const ::rtl::OUString& rName, const BitmapEx& rBmpEx )
{
    const ::rtl::OUString aFileName( implGetUserFileURL( rName ) );

    if( aFileName.getLength() )
    {
        ::std::auto_ptr< SvStream > apOStm( ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC ) );

        if( apOStm.get() )
            *apOStm << rBmpEx;
    }
}
