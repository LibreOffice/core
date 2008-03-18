/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impimagetree.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 14:11:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <cstdio>
#include <ctype.h>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_PNGREAD_HXX
#include <vcl/pngread.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <rtl/bootstrap.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_XZIPFILEACCESS_HPP_
#include <com/sun/star/packages/zip/XZipFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#include "vcl/impimagetree.hxx"

#include <vector>
#include <hash_map>

#define DEFAULT_PROJECTNAME "res"
#define IMAGES_ZIPFILENAME_PREFIX   "images"
#define IMAGES_ZIPFILENAME_SUFFIX   ".zip"
#define IMAGES_CACHEDIR     "imagecache"

using namespace ::com::sun::star;

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

    aCleaner->mxNameAcc.clear();
    aCleaner->mxZipAcc.clear();
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
                mxZipAcc.set( xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.packages.zip.ZipFileAccess" ) ), uno::UNO_QUERY ) ;
                mxPathSettings.set( xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.util.PathSettings" ) ), uno::UNO_QUERY );
                mxFileAccess.set( xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), uno::UNO_QUERY );

                if( mxZipAcc.is() && mxPathSettings.is() && mxFileAccess.is() )
                {
                    uno::Reference< lang::XInitialization > xInit( mxZipAcc, uno::UNO_QUERY );

                    if( xInit.is() )
                    {
                        uno::Sequence< uno::Any > aInitSeq( 1 );

                        try
                        {
                            const ::rtl::OUString& rZipURL = implGetZipFileURL();

                            if( rZipURL.getLength() )
                            {
                                aInitSeq[ 0 ] <<= rZipURL;
                                xInit->initialize( aInitSeq );
                                mxNameAcc.set( mxZipAcc, uno::UNO_QUERY );
                                implCheckUserCache();
                            }
                            else
                                mxZipAcc.clear();
                        }
                        catch( const uno::Exception& )
                        {
                            mxZipAcc.clear();
                        }
                    }
                }
                else
                {
                    mxZipAcc.clear();
                }
            }
            catch( const uno::Exception& )
            {
                mxZipAcc.clear();
                mxPathSettings.clear();
                mxFileAccess.clear();
            }
        }
    }

    return( mbInit = mxNameAcc.is() );
}

// -----------------------------------------------------------------------

::rtl::OUString ImplImageTree::implGetZipFileURL( bool bWithStyle ) const
{
    ::rtl::OUString aRet;

    if( !aRet.getLength() && mxPathSettings.is() && mxFileAccess.is() )
    {
        ::rtl::OUString aZipFileName( ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_PREFIX ) );
        if ( bWithStyle && maSymbolsStyle.getLength() > 0 )
        {
            aZipFileName += ::rtl::OUString::createFromAscii( "_" );
            aZipFileName += maSymbolsStyle;
        }
        aZipFileName += ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME_SUFFIX );

        uno::Any                aUserAny( mxPathSettings->getPropertyValue( ::rtl::OUString::createFromAscii( "UserConfig" ) ) );
        INetURLObject           aZipURL;

        if( ( aUserAny >>= aRet ) && aRet.getLength() )
        {
            aZipURL = INetURLObject( aRet );
            aZipURL.Append( aZipFileName );

            if( !mxFileAccess->exists( aRet = aZipURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                rtl::OUString aPath(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "$BRAND_BASE_DIR/share/config" ) );
                rtl::Bootstrap::expandMacros( aPath );
                aZipURL = INetURLObject( aPath );
                aZipURL.Append( aZipFileName );

                if( !mxFileAccess->exists( aRet = aZipURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
                {
                    if ( bWithStyle && maSymbolsStyle.getLength() > 0 )
                        aRet = implGetZipFileURL( false ); // Try without style
                    else
                        aRet = ::rtl::OUString();
                }
            }
        }
    }

    return aRet;
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
            mxNameAcc.clear();
            mxZipAcc.clear();
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

        if( mxNameAcc.is() || ( implInit() && mxNameAcc.is() ) )
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

                            if( mxNameAcc->hasByName( aLocaleName ) )
                            {
                                try
                                {
                                    uno::Reference< io::XInputStream > xIStm;

                                    if( mxNameAcc->getByName( aLocaleName ) >>= xIStm )
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
            }

            if( rReturn.IsEmpty() && mxNameAcc->hasByName( rName ) )
            {
                try
                {
                    uno::Reference< io::XInputStream > xIStm;

                    if( mxNameAcc->getByName( rName ) >>= xIStm )
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
