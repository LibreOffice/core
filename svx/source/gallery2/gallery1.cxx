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

// - Namespaces -

using namespace ::rtl;
using namespace ::com::sun::star;

// - GalleryThemeEntry -

GalleryThemeEntry::GalleryThemeEntry( const INetURLObject& rBaseURL, const OUString& rName,
                                      sal_Bool _bReadOnly, sal_Bool _bNewFile,
                                      sal_uInt32 _nId, sal_Bool _bThemeNameFromResource ) :
        nId                                     ( _nId ),
        bReadOnly                               ( _bReadOnly ),
        bThemeNameFromResource  ( _bThemeNameFromResource )
{
    INetURLObject aURL( rBaseURL );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    aURL.setExtension( "thm" );
    aThmURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( "sdg" );
    aSdgURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( "sdv" );
    aSdvURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( "str" );
    aStrURL = ImplGetURLIgnoreCase( aURL );

    SetModified( _bNewFile );

    aName = ReadStrFromIni( "name" );

    // This is awful - we shouldn't use these resources if we
    // possibly can avoid them
    if( aName.isEmpty() && nId && bThemeNameFromResource )
        aName = GAL_RESSTR( RID_GALLERYSTR_THEME_START + (sal_uInt16) nId );

    if( aName.isEmpty() )
        aName = rName;
}

INetURLObject GalleryThemeEntry::ImplGetURLIgnoreCase( const INetURLObject& rURL ) const
{
    INetURLObject   aURL( rURL );

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

void GalleryThemeEntry::SetName( const OUString& rNewName )
{
    if( aName != rNewName )
    {
        aName = rNewName;
        SetModified( sal_True );
        bThemeNameFromResource = sal_False;
    }
}

void GalleryThemeEntry::SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName )
{
    nId = nNewId;
    SetModified( sal_True );
    bThemeNameFromResource = ( nId && bResetThemeName );
}

// - GalleryThemeCacheEntry -

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

// - Gallery -

Gallery::Gallery( const OUString& rMultiPath )
:       nReadTextEncoding   ( osl_getThreadTextEncoding() )
,       bMultiPath          ( sal_False )
{
    ImplLoad( rMultiPath );
}

Gallery::~Gallery()
{
    // erase theme list
    for ( size_t i = 0, n = aThemeList.size(); i < n; ++i )
        delete aThemeList[ i ];
    aThemeList.clear();
}

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

void Gallery::ImplLoad( const OUString& rMultiPath )
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
            OUString        aTestFile( "cdefghij.klm" );

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
                static const OUString s_sTitle("Title");
                static const OUString s_sIsReadOnly("IsReadOnly");
                static const OUString s_sSDG_EXT("sdg");
                static const OUString s_sSDV_EXT("sdv");

                while( xResultSet->next() )
                {
                    INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

                    if(aThmURL.GetExtension().equalsIgnoreAsciiCase("thm"))
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
                                    aThemeList.push_back( pEntry );
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

GalleryThemeEntry* Gallery::ImplGetThemeEntry( const OUString& rThemeName )
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

OUString Gallery::GetThemeName( sal_uIntPtr nThemeId ) const
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
        OString aFallback;

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
            case( GALLERY_THEME_POWERPOINT ):
                aFallback = "private://gallery/hidden/imgppt";
                break;
            case( GALLERY_THEME_FONTWORK ):
                aFallback = "private://gallery/hidden/fontwork";
                break;
            case( GALLERY_THEME_FONTWORK_VERTICAL ):
                aFallback = "private://gallery/hidden/fontworkvertical";
                break;
            case( GALLERY_THEME_SOUNDS ):
                aFallback = "Sounds";
                break;
            case( RID_GALLERYSTR_THEME_ARROWS ):
                aFallback = "Arrows";
                break;
            case( RID_GALLERYSTR_THEME_COMPUTERS ):
                aFallback = "Computers";
                break;
            case( RID_GALLERYSTR_THEME_DIAGRAMS ):
                aFallback = "Diagrams";
                break;
            case( RID_GALLERYSTR_THEME_EDUCATION ):
                aFallback = "Education";
                break;
            case( RID_GALLERYSTR_THEME_ENVIRONMENT ):
                aFallback = "Environment";
                break;
            case( RID_GALLERYSTR_THEME_FINANCE ):
                aFallback = "Finance";
                break;
            case( RID_GALLERYSTR_THEME_PEOPLE ):
                aFallback = "People";
                break;
            case( RID_GALLERYSTR_THEME_SYMBOLS ):
                aFallback = "Symbols";
                break;
            case( RID_GALLERYSTR_THEME_TRANSPORT ):
                aFallback = "Transport";
                break;
            case( RID_GALLERYSTR_THEME_TXTSHAPES ):
                aFallback = "Textshapes";
                break;
            default:
                break;
        }

        pFound = const_cast<Gallery*>(this)->ImplGetThemeEntry(OStringToOUString(aFallback, RTL_TEXTENCODING_ASCII_US));
    }

    return( pFound ? pFound->GetThemeName() : OUString() );
}

sal_Bool Gallery::HasTheme( const OUString& rThemeName )
{
    return( ImplGetThemeEntry( rThemeName ) != NULL );
}

sal_Bool Gallery::CreateTheme( const OUString& rThemeName )
{
    sal_Bool bRet = sal_False;

    if( !HasTheme( rThemeName ) && ( GetUserURL().GetProtocol() != INET_PROT_NOT_VALID ) )
    {
        INetURLObject aURL( GetUserURL() );
        aURL.Append( rThemeName );
        GalleryThemeEntry* pNewEntry = new GalleryThemeEntry( aURL, rThemeName,
                                                              sal_False, sal_True, 0, sal_False );

        aThemeList.push_back( pNewEntry );
        delete( new GalleryTheme( this, pNewEntry ) );
        Broadcast( GalleryHint( GALLERY_HINT_THEME_CREATED, rThemeName ) );
        bRet = sal_True;
    }

    return bRet;
}

sal_Bool Gallery::RenameTheme( const OUString& rOldName, const OUString& rNewName )
{
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );
    sal_Bool                            bRet = sal_False;

    // check if the new theme name is already present
    if( pThemeEntry && !HasTheme( rNewName ) && !pThemeEntry->IsReadOnly() )
    {
        SfxListener   aListener;
        GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

        if( pThm )
        {
            const OUString aOldName( rOldName );

            pThemeEntry->SetName( rNewName );
            pThm->ImplWrite();

            Broadcast( GalleryHint( GALLERY_HINT_THEME_RENAMED, aOldName, pThm->GetName() ) );
            ReleaseTheme( pThm, aListener );
            bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool Gallery::RemoveTheme( const OUString& rThemeName )
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
            INetURLObject   aStrURL( pThm->GetSdvURL() );

            ReleaseTheme( pThm, aListener );

            KillFile( aThmURL );
            KillFile( aSdgURL );
            KillFile( aSdvURL );
            KillFile( aStrURL );
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

GalleryTheme* Gallery::AcquireTheme( const OUString& rThemeName, SfxListener& rListener )
{
    GalleryTheme*           pTheme = NULL;
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

    if( pThemeEntry && ( ( pTheme = ImplGetCachedTheme( pThemeEntry ) ) != NULL ) )
        rListener.StartListening( *pTheme );

    return pTheme;
}

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
