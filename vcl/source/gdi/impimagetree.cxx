/*************************************************************************
 *
 *  $RCSfile: impimagetree.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:47:33 $
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
#include <svapp.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <bitmapex.hxx>
#endif
#ifndef _SV_PNGREAD_HXX
#include <pngread.hxx>
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

#include "impimagetree.hxx"

#include <vector>
#include <hash_map>

#define DEFAULT_PROJECTNAME "res"
#define IMAGES_ZIPFILENAME  "images.zip"
#define IMAGES_CACHEDIR     "imagecache"

using namespace ::com::sun::star;

// -----------------
// - ImplImageTree -
// -----------------

typedef ::std::hash_map< ::rtl::OUString, BitmapEx, ::rtl::OUStringHash > BmpExHashMap;
static BmpExHashMap aBmpExHashMap;

// -----------------------------------------------------------------------

ImplImageTree::ImplImageTree() :
    mbInit( false )
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
    }

    return( mbInit = mxNameAcc.is() );
}

// -----------------------------------------------------------------------

const ::rtl::OUString& ImplImageTree::implGetZipFileURL() const
{
    static ::rtl::OUString aRet;

    if( !aRet.getLength() && mxPathSettings.is() && mxFileAccess.is() )
    {
        const ::rtl::OUString   aZipFileName( ::rtl::OUString::createFromAscii( IMAGES_ZIPFILENAME ) );
        uno::Any                aAny( mxPathSettings->getPropertyValue( ::rtl::OUString::createFromAscii( "UserConfig" ) ) );
        INetURLObject           aZipURL;

        if( ( aAny >>= aRet ) && aRet.getLength() )
        {
            aZipURL = INetURLObject( aRet );
            aZipURL.Append( aZipFileName );

            if( !mxFileAccess->exists( aRet = aZipURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                uno::Any aAny( mxPathSettings->getPropertyValue( ::rtl::OUString::createFromAscii( "Config" ) ) );

                if( ( aAny >>= aRet ) && aRet.getLength() )
                {
                    aZipURL = INetURLObject( aRet );
                    aZipURL.Append( aZipFileName );

                    if( !mxFileAccess->exists( aRet = aZipURL.GetMainURL( INetURLObject::NO_DECODE ) ) )
                    {
                        aRet = ::rtl::OUString();
                    }
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

bool ImplImageTree::loadImage( const ::rtl::OUString& rName, BitmapEx& rReturn, bool bSearchLanguageDependent )
{
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
                        const sal_uInt32 nPos = rName.lastIndexOf( '/' );

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

                                    if( ( mxNameAcc->getByName( aLocaleName ) >>= xIStm ) && xIStm.is() )
                                    {
                                        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( xIStm );

                                        if( pIStm )
                                        {
                                            implLoadFromStream( *pIStm, aLocaleName, rReturn );
                                            delete pIStm;
                                        }
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

                    if( ( mxNameAcc->getByName( rName ) >>= xIStm ) && xIStm.is() )
                    {
                        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( xIStm );

                        if( pIStm )
                        {
                            implLoadFromStream( *pIStm, rName, rReturn );
                            delete pIStm;
                        }
                    }
                }
                catch( const uno::Exception & )
                {
                }
            }

            if( rReturn.IsEmpty() )
            {
                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( implGetUserFileURL( rName ), STREAM_READ );

                if( pIStm )
                {
                    *pIStm >> rReturn;
                    delete pIStm;
                }
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

                if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( ( aAppDir = aAppDir.copy( 0, nPos  ) ), aURLStr ) )
                {
                    INetURLObject   aURL( aURLStr );
                    sal_Int32       nIndex = 0;

                    do
                    {
                        aURL.Append( rName.getToken( 0, '/', nIndex ) );
                    }
                    while( nIndex >= 0 );

                    aURLStr = aURL.GetMainURL( INetURLObject::NO_DECODE );
                    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_READ );

                    if( pIStm )
                    {
                        implLoadFromStream( *pIStm, aURLStr, rReturn );
                        delete pIStm;
                    }

                    if( rReturn.IsEmpty() )
                    {
                        // HACK for old setup!!! try to look in ../share/config
                        nPos = aAppDir.lastIndexOf( '/' );

                        if( -1 == nPos )
                            nPos = aAppDir.lastIndexOf( '\\' );

                        if( ( -1 != nPos ) && ::utl::LocalFileHelper::ConvertPhysicalNameToURL( ( aAppDir = aAppDir.copy( 0, nPos  ) ), aURLStr ) )
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
                            SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_READ );

                            if( pIStm )
                            {
                                implLoadFromStream( *pIStm, aURLStr, rReturn );
                                delete pIStm;
                            }
                        }
                    }
                }
            }
        }
    }

    if( !rReturn.IsEmpty() )
        aBmpExHashMap[ rName ] = rReturn;

    return( !rReturn.IsEmpty() );
}

// ------------------------------------------------------------------------------

void ImplImageTree::addUserImage( const ::rtl::OUString& rName, const BitmapEx& rBmpEx )
{
    const ::rtl::OUString aFileName( implGetUserFileURL( rName ) );

    if( aFileName.getLength() )
    {
        SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC );

        if( pOStm )
        {
            *pOStm << rBmpEx;
            delete pOStm;
        }
    }
}
