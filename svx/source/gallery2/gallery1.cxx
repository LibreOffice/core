/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/vcompat.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include "svx/gallery.hxx"
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "svx/galtheme.hxx"
#include "svx/gallery1.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

// --------------
// - Namespaces -
// --------------

using namespace ::rtl;
using namespace ::com::sun::star;

// ---------------------
// - GalleryThemeEntry -
// ---------------------

GalleryThemeEntry::GalleryThemeEntry( const INetURLObject& rBaseURL, const String& rName,
                                      sal_uInt32 _nFileNumber, sal_Bool _bReadOnly,
                                      sal_Bool _bNewFile, sal_uInt32 _nId, sal_Bool _bThemeNameFromResource ) :
        nFileNumber                             ( _nFileNumber ),
        nId                                     ( _nId ),
        bReadOnly                               ( _bReadOnly ),
        bThemeNameFromResource  ( _bThemeNameFromResource )
{
    INetURLObject aURL( rBaseURL );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
    String aFileName( String( "sg"  ) );

    aURL.Append( ( aFileName += String::CreateFromInt32( nFileNumber ) ) += String( RTL_CONSTASCII_USTRINGPARAM( ".thm" ) ) );
    aThmURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdg" ) ) );
    aSdgURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdv" ) ) );
    aSdvURL = ImplGetURLIgnoreCase( aURL );

    SetModified( _bNewFile );

    if( nId && bThemeNameFromResource )
        aName = GAL_RESSTR( RID_GALLERYSTR_THEME_START + (sal_uInt16) nId );

    if( aName.isEmpty() )
        aName = rName;
}

// -----------------------------------------------------------------------------

INetURLObject GalleryThemeEntry::ImplGetURLIgnoreCase( const INetURLObject& rURL ) const
{
    INetURLObject   aURL( rURL );
    String          aFileName;

    // check original file name
    if( !FileExists( aURL ) )
    {
        // check upper case file name
        aURL.setName( aURL.getName().toAsciiUpperCase() );

        if(!FileExists( aURL ) )
        {
            // check lower case file name
            aURL.setName( aURL.getName().toAsciiLowerCase() );
        }
    }

    return aURL;
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetName( const rtl::OUString& rNewName )
{
    if( aName != rNewName )
    {
        aName = rNewName;
        SetModified( sal_True );
        bThemeNameFromResource = sal_False;
    }
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName )
{
    nId = nNewId;
    SetModified( sal_True );
    bThemeNameFromResource = ( nId && bResetThemeName );
}

// ---------------------------
// - GalleryImportThemeEntry -
// ---------------------------

SvStream& operator<<( SvStream& rOut, const GalleryImportThemeEntry& rEntry )
{
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rEntry.aThemeName, RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rEntry.aUIName, RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, (rEntry.aURL.GetMainURL( INetURLObject::NO_DECODE )), RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOut, rtl::OUString(), RTL_TEXTENCODING_UTF8); //aImportName

    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOut, rtl::OString());
    return rOut;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, GalleryImportThemeEntry& rEntry )
{
    rEntry.aThemeName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
    rEntry.aUIName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
    rEntry.aURL = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8);
    read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIn, RTL_TEXTENCODING_UTF8); //aImportName
    read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIn);
    return rIn;
}

// --------------------------
// - GalleryThemeCacheEntry -
// --------------------------

class GalleryThemeCacheEntry
{
private:

    const GalleryThemeEntry*        mpThemeEntry;
    GalleryTheme*                           mpTheme;

public:

                                GalleryThemeCacheEntry( const GalleryThemeEntry* pThemeEntry, GalleryTheme* pTheme ) :
                                    mpThemeEntry( pThemeEntry ), mpTheme( pTheme ) {}
                                ~GalleryThemeCacheEntry() { delete mpTheme; }

    const GalleryThemeEntry*        GetThemeEntry() const { return mpThemeEntry; }
    GalleryTheme*                           GetTheme() const { return mpTheme; }
};

// -----------
// - Gallery -
// -----------

Gallery::Gallery( const rtl::OUString& rMultiPath )
:       nReadTextEncoding   ( osl_getThreadTextEncoding() )
,       nLastFileNumber     ( 0 )
,       bMultiPath          ( sal_False )
{
    ImplLoad( rMultiPath );
}

// ------------------------------------------------------------------------

Gallery::~Gallery()
{
    // Themen-Liste loeschen
    for ( size_t i = 0, n = aThemeList.size(); i < n; ++i )
        delete aThemeList[ i ];
    aThemeList.clear();

    // Import-Liste loeschen
    for ( size_t i = 0, n = aImportList.size(); i < n; ++i )
        delete aImportList[ i ];
    aImportList.clear();
}

// ------------------------------------------------------------------------

Gallery* Gallery::GetGalleryInstance()
{
    static Gallery* pGallery = NULL;

    if( !pGallery )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pGallery )
        {
            pGallery = new Gallery( SvtPathOptions().GetGalleryPath() );
        }
    }

    return pGallery;
}

// ------------------------------------------------------------------------

void Gallery::ImplLoad( const rtl::OUString& rMultiPath )
{
    const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rMultiPath, ';');
    sal_Bool        bIsReadOnlyDir;

    bMultiPath = ( nTokenCount > 0 );

    INetURLObject aCurURL(SvtPathOptions().GetConfigPath());
    ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

    if( !bIsReadOnlyDir )
        aUserURL = aCurURL;

    if( bMultiPath )
    {
        aRelURL = INetURLObject( comphelper::string::getToken(rMultiPath, 0, ';') );

        for( sal_Int32 i = 0; i < nTokenCount; ++i )
        {
            aCurURL = INetURLObject(comphelper::string::getToken(rMultiPath, i, ';'));

            ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

            if( !bIsReadOnlyDir )
                aUserURL = aCurURL;
        }
    }
    else
        aRelURL = INetURLObject( rMultiPath );

    DBG_ASSERT( aUserURL.GetProtocol() != INET_PROT_NOT_VALID, "no writable Gallery user directory available" );
    DBG_ASSERT( aRelURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbDirIsReadOnly )
{
    rbDirIsReadOnly = sal_False;

    try
    {
        uno::Reference< ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content                       aCnt( rBaseURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext() );

        uno::Sequence< OUString > aProps( 1 );
        aProps.getArray()[ 0 ] = OUString("Url");

        uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

        try
        {
            // check readonlyness the very hard way
            INetURLObject   aTestURL( rBaseURL );
            String          aTestFile( RTL_CONSTASCII_USTRINGPARAM( "cdefghij.klm" ) );

            aTestURL.Append( aTestFile );
            SvStream* pTestStm = ::utl::UcbStreamHelper::CreateStream( aTestURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );

            if( pTestStm )
            {
                *pTestStm << sal_Int32(1);

                if( pTestStm->GetError() )
                    rbDirIsReadOnly = sal_True;

                delete pTestStm;
                KillFile( aTestURL );
            }
            else
                rbDirIsReadOnly = sal_True;
        }
        catch( const ucb::ContentCreationException& )
        {
        }
        catch( const uno::RuntimeException& )
        {
        }
        catch( const uno::Exception& )
        {
        }

        if( xResultSet.is() )
        {
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            if( xContentAccess.is() )
            {
                static const ::rtl::OUString s_sTitle("Title");
                static const ::rtl::OUString s_sIsReadOnly("IsReadOnly");
                static const ::rtl::OUString s_sSDG_EXT("sdg");
                static const ::rtl::OUString s_sSDV_EXT("sdv");

                while( xResultSet->next() )
                {
                    INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

                    if(aThmURL.GetExtension().equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("thm")))
                    {
                        INetURLObject   aSdgURL( aThmURL); aSdgURL.SetExtension( s_sSDG_EXT );
                        INetURLObject   aSdvURL( aThmURL ); aSdvURL.SetExtension( s_sSDV_EXT );
                        OUString        aTitle;
                        sal_Bool        bReadOnly = sal_False;

                        try
                        {
                            ::ucbhelper::Content aThmCnt( aThmURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext() );
                            ::ucbhelper::Content aSdgCnt( aSdgURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext() );
                            ::ucbhelper::Content aSdvCnt( aSdvURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv, comphelper::getProcessComponentContext() );

                            try
                            {
                                aThmCnt.getPropertyValue( s_sTitle ) >>= aTitle;
                            }
                            catch( const uno::RuntimeException& )
                            {
                            }
                            catch( const uno::Exception& )
                            {
                            }

                            if( !aTitle.isEmpty() )
                            {
                                try
                                {
                                    aThmCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
                                }
                                catch( const uno::RuntimeException& )
                                {
                                }
                                catch( const uno::Exception& )
                                {
                                }

                                if( !bReadOnly )
                                {
                                    try
                                    {
                                        aSdgCnt.getPropertyValue( s_sTitle ) >>= aTitle;
                                    }
                                    catch( const ::com::sun::star::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const ::com::sun::star::uno::Exception& )
                                    {
                                    }

                                    if( !aTitle.isEmpty() )
                                    {
                                        try
                                        {
                                            aSdgCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
                                        }
                                        catch( const uno::RuntimeException& )
                                        {
                                        }
                                        catch( const uno::Exception& )
                                        {
                                        }
                                    }
                                }

                                if( !bReadOnly )
                                {
                                    try
                                    {
                                        aSdvCnt.getPropertyValue( s_sTitle ) >>= aTitle;
                                    }
                                    catch( const ::com::sun::star::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const ::com::sun::star::uno::Exception& )
                                    {
                                    }

                                    if( !aTitle.isEmpty() )
                                    {
                                        try
                                        {
                                            aSdvCnt.getPropertyValue( s_sIsReadOnly ) >>= bReadOnly;
                                        }
                                        catch( const uno::RuntimeException& )
                                        {
                                        }
                                        catch( const uno::Exception& )
                                        {
                                        }
                                    }
                                }

                                GalleryThemeEntry* pEntry = GalleryTheme::CreateThemeEntry( aThmURL, rbDirIsReadOnly || bReadOnly );

                                if( pEntry )
                                {
                                    const sal_uIntPtr nFileNumber = (sal_uIntPtr) String(aThmURL.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32();

                                    aThemeList.push_back( pEntry );

                                    if( nFileNumber > nLastFileNumber )
                                        nLastFileNumber = nFileNumber;
                                }
                            }
                        }
                        catch( const ucb::ContentCreationException& )
                        {
                        }
                        catch( const uno::RuntimeException& )
                        {
                        }
                        catch( const uno::Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
    catch( const ucb::ContentCreationException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    catch( const uno::Exception& )
    {
    }
}

// ------------------------------------------------------------------------

GalleryThemeEntry* Gallery::ImplGetThemeEntry( const rtl::OUString& rThemeName )
{
    GalleryThemeEntry* pFound = NULL;

    if( !rThemeName.isEmpty() )
    {
        for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
            if( rThemeName == aThemeList[ i ]->GetThemeName() )
                pFound = aThemeList[ i ];
    }

    return pFound;
}

// ------------------------------------------------------------------------

rtl::OUString Gallery::GetThemeName( sal_uIntPtr nThemeId ) const
{
    GalleryThemeEntry* pFound = NULL;

    for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
    {
        GalleryThemeEntry* pEntry = aThemeList[ i ];
        if( nThemeId == pEntry->GetId() )
            pFound = pEntry;
    }

    // try fallback, if no entry was found
    if( !pFound )
    {
        rtl::OString aFallback;

        switch( nThemeId )
        {
            case( GALLERY_THEME_3D ):
                aFallback = "3D";
                break;
            case( GALLERY_THEME_BULLETS ):
                aFallback = "Bullets";
                break;
            case( GALLERY_THEME_HOMEPAGE ):
                aFallback = "Homepage";
                break;
            case( GALLERY_THEME_HTMLBUTTONS ):
                aFallback = "private://gallery/hidden/HtmlExportButtons";
                break;
            case( GALLERY_THEME_POWERPOINT ):
                aFallback = "private://gallery/hidden/imgppt";
                break;
            case( GALLERY_THEME_FONTWORK ):
                aFallback = "private://gallery/hidden/fontwork";
                break;
            case( GALLERY_THEME_FONTWORK_VERTICAL ):
                aFallback = "private://gallery/hidden/fontworkvertical";
                break;
            case( GALLERY_THEME_RULERS ):
                aFallback = "Rulers";
                break;
            case( GALLERY_THEME_SOUNDS ):
                aFallback = "Sounds";
                break;
            default:
                break;
        }

        pFound = const_cast<Gallery*>(this)->ImplGetThemeEntry(rtl::OStringToOUString(aFallback, RTL_TEXTENCODING_ASCII_US));
    }

    return( pFound ? pFound->GetThemeName() : rtl::OUString() );
}

// ------------------------------------------------------------------------

sal_Bool Gallery::HasTheme( const String& rThemeName )
{
    return( ImplGetThemeEntry( rThemeName ) != NULL );
}

// ------------------------------------------------------------------------

sal_Bool Gallery::CreateTheme( const String& rThemeName, sal_uInt32 nNumFrom )
{
    sal_Bool bRet = sal_False;

    if( !HasTheme( rThemeName ) && ( GetUserURL().GetProtocol() != INET_PROT_NOT_VALID ) )
    {
        nLastFileNumber = nNumFrom > nLastFileNumber ? nNumFrom : nLastFileNumber + 1;
        GalleryThemeEntry* pNewEntry = new GalleryThemeEntry( GetUserURL(), rThemeName,
                                                              nLastFileNumber,
                                                              sal_False, sal_True, 0, sal_False );

        aThemeList.push_back( pNewEntry );
        delete( new GalleryTheme( this, pNewEntry ) );
        Broadcast( GalleryHint( GALLERY_HINT_THEME_CREATED, rThemeName ) );
        bRet = sal_True;
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Gallery::RenameTheme( const String& rOldName, const String& rNewName )
{
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );
    sal_Bool                            bRet = sal_False;

    // Ueberpruefen, ob neuer Themenname schon vorhanden ist
    if( pThemeEntry && !HasTheme( rNewName ) && !pThemeEntry->IsReadOnly() )
    {
        SfxListener   aListener;
        GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

        if( pThm )
        {
            const String aOldName( rOldName );

            pThemeEntry->SetName( rNewName );
            pThm->ImplWrite();

            Broadcast( GalleryHint( GALLERY_HINT_THEME_RENAMED, aOldName, pThm->GetName() ) );
            ReleaseTheme( pThm, aListener );
            bRet = sal_True;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Gallery::RemoveTheme( const String& rThemeName )
{
    GalleryThemeEntry*  pThemeEntry = ImplGetThemeEntry( rThemeName );
    sal_Bool                bRet = sal_False;

    if( pThemeEntry && !pThemeEntry->IsReadOnly() )
    {
        Broadcast( GalleryHint( GALLERY_HINT_CLOSE_THEME, rThemeName ) );

        SfxListener     aListener;
        GalleryTheme*   pThm = AcquireTheme( rThemeName, aListener );

        if( pThm )
        {
            INetURLObject   aThmURL( pThm->GetThmURL() );
            INetURLObject   aSdgURL( pThm->GetSdgURL() );
            INetURLObject   aSdvURL( pThm->GetSdvURL() );

            ReleaseTheme( pThm, aListener );

            KillFile( aThmURL );
            KillFile( aSdgURL );
            KillFile( aSdvURL );
        }

        for ( GalleryThemeList::iterator it = aThemeList.begin(); it != aThemeList.end(); ++it )
        {
            if ( pThemeEntry == *it ) {
                delete pThemeEntry;
                aThemeList.erase( it );
                break;
            }
        }

        Broadcast( GalleryHint( GALLERY_HINT_THEME_REMOVED, rThemeName ) );

        bRet = sal_True;
    }

    return bRet;
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry )
{
    GalleryTheme* pTheme = NULL;

    if( pThemeEntry )
    {
        for (GalleryCacheThemeList::const_iterator it = aThemeCache.begin(); it != aThemeCache.end(); ++it)
        {
            if (pThemeEntry == (*it)->GetThemeEntry())
            {
                pTheme = (*it)->GetTheme();
                break;
            }
        }

        if( !pTheme )
        {
            INetURLObject aURL = pThemeEntry->GetThmURL();

            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( FileExists( aURL ) )
            {
                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

                if( pIStm )
                {
                    pTheme = new GalleryTheme( this, (GalleryThemeEntry*) pThemeEntry );
                    *pIStm >> *pTheme;

                    if( pIStm->GetError() )
                    {
                        delete pTheme, pTheme = NULL;
                    }

                    delete pIStm;
                }
            }

            if( pTheme )
                aThemeCache.push_back( new GalleryThemeCacheEntry( pThemeEntry, pTheme ));
        }
    }

    return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ImplDeleteCachedTheme( GalleryTheme* pTheme )
{
    for (GalleryCacheThemeList::iterator it = aThemeCache.begin(); it != aThemeCache.end(); ++it)
    {
        if (pTheme == (*it)->GetTheme())
        {
            delete *it;
            aThemeCache.erase(it);
            break;
        }
    }
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::AcquireTheme( const String& rThemeName, SfxListener& rListener )
{
    GalleryTheme*           pTheme = NULL;
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

    if( pThemeEntry && ( ( pTheme = ImplGetCachedTheme( pThemeEntry ) ) != NULL ) )
        rListener.StartListening( *pTheme );

    return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener )
{
    if( pTheme )
    {
        rListener.EndListening( *pTheme );

        if( !pTheme->HasListeners() )
            ImplDeleteCachedTheme( pTheme );
    }
}

sal_Bool GalleryThemeEntry::IsDefault() const
{ return( ( nId > 0 ) && ( nId != ( RID_GALLERYSTR_THEME_MYTHEME - RID_GALLERYSTR_THEME_START ) ) ); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
