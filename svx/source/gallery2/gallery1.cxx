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

#include <config_features.h>

#if defined(MACOSX) && HAVE_FEATURE_READONLY_INSTALLSET
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>
#endif

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/thread.h>
#include <tools/vcompat.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include "svx/gallery.hxx"
#include "gallery.hrc"
#include "strings.hxx"
#include "svx/galmisc.hxx"
#include "svx/galtheme.hxx"
#include "svx/gallery1.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <memory>


using namespace ::com::sun::star;


static bool FileExists( const INetURLObject &rURL, const rtl::OUString &rExt )
{
    INetURLObject aURL( rURL );
    aURL.setExtension( rExt );
    return FileExists( aURL );
}

GalleryThemeEntry::GalleryThemeEntry( bool bCreateUniqueURL,
                                      const INetURLObject& rBaseURL, const OUString& rName,
                                      bool _bReadOnly, bool _bNewFile,
                                      sal_uInt32 _nId, bool _bThemeNameFromResource ) :
        nId                     ( _nId ),
        bReadOnly               ( _bReadOnly ),
        bThemeNameFromResource  ( _bThemeNameFromResource )
{
    INetURLObject aURL( rBaseURL );
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if (bCreateUniqueURL)
    {
        INetURLObject aBaseNoCase( ImplGetURLIgnoreCase( rBaseURL ) );
        aURL = aBaseNoCase;
        static sal_Int32 nIdx = 0;
        while( FileExists( aURL, "thm" ) )
        { // create new URLs
            nIdx++;
            aURL = aBaseNoCase;
            aURL.setName( aURL.getName() + OUString::number(nIdx));
        }
    }

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
    {
        //some of these are supposed to *not* be localized
        //so catch them before looking up the resource
        const std::pair<sal_uInt16, const char*> aUnlocalized[] =
        {
            { GALLERY_THEME_HOMEPAGE, RID_GALLERYSTR_THEME_HTMLBUTTONS },
            { GALLERY_THEME_POWERPOINT, RID_GALLERYSTR_THEME_POWERPOINT },
            { GALLERY_THEME_USERSOUNDS, RID_GALLERYSTR_THEME_USERSOUNDS },
            { GALLERY_THEME_DUMMY5, RID_GALLERYSTR_THEME_DUMMY5 },
            { GALLERY_THEME_FONTWORK, RID_GALLERYSTR_THEME_FONTWORK },
            { GALLERY_THEME_FONTWORK_VERTICAL, RID_GALLERYSTR_THEME_FONTWORK_VERTICAL }
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aUnlocalized); ++i)
        {
            if (aUnlocalized[i].first == nId)
            {
                aName = OUString::createFromAscii(aUnlocalized[i].second);
                break;
            }
        }
        //look up the rest of the ids in string resources
        if (aName.isEmpty())
            aName = GAL_RESSTR(RID_GALLERYSTR_THEME_START + (sal_uInt16) nId);
    }

    if( aName.isEmpty() )
        aName = rName;
}

INetURLObject GalleryThemeEntry::ImplGetURLIgnoreCase( const INetURLObject& rURL )
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
        SetModified( true );
        bThemeNameFromResource = false;
    }
}

void GalleryThemeEntry::SetId( sal_uInt32 nNewId, bool bResetThemeName )
{
    nId = nNewId;
    SetModified( true );
    bThemeNameFromResource = ( nId && bResetThemeName );
}


class GalleryThemeCacheEntry
{
private:

    const GalleryThemeEntry*        mpThemeEntry;
    std::unique_ptr<GalleryTheme>   mpTheme;

public:

                                GalleryThemeCacheEntry( const GalleryThemeEntry* pThemeEntry, GalleryTheme* pTheme ) :
                                    mpThemeEntry( pThemeEntry ), mpTheme( pTheme ) {}

    const GalleryThemeEntry*    GetThemeEntry() const { return mpThemeEntry; }
    GalleryTheme*               GetTheme() const { return mpTheme.get(); }
};


Gallery::Gallery( const OUString& rMultiPath )
:       bMultiPath          ( false )
{
    ImplLoad( rMultiPath );
}

Gallery::~Gallery()
{
    // erase theme list
    for (GalleryThemeEntry* p : aThemeList)
        delete p;
    aThemeList.clear();
}

Gallery* Gallery::GetGalleryInstance()
{
    static Gallery* s_pGallery = nullptr;

    if (!s_pGallery)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (!s_pGallery && !utl::ConfigManager::IsAvoidConfig())
        {
            s_pGallery = new Gallery( SvtPathOptions().GetGalleryPath() );
        }
    }

    return s_pGallery;
}

void Gallery::ImplLoad( const OUString& rMultiPath )
{
    const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rMultiPath, ';');
    bool            bIsReadOnlyDir;

    bMultiPath = ( nTokenCount > 0 );

    INetURLObject aCurURL(SvtPathOptions().GetConfigPath());
    ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

    if( !bIsReadOnlyDir )
        aUserURL = aCurURL;

    if( bMultiPath )
    {
        aRelURL = INetURLObject( rMultiPath.getToken(0, ';') );

        for( sal_Int32 i = 0; i < nTokenCount; ++i )
        {
            aCurURL = INetURLObject(rMultiPath.getToken(i, ';'));

            ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

            if( !bIsReadOnlyDir )
                aUserURL = aCurURL;
        }
    }
    else
        aRelURL = INetURLObject( rMultiPath );

    DBG_ASSERT( aUserURL.GetProtocol() != INetProtocol::NotValid, "no writable Gallery user directory available" );
    DBG_ASSERT( aRelURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
}

void Gallery::ImplLoadSubDirs( const INetURLObject& rBaseURL, bool& rbDirIsReadOnly )
{
    rbDirIsReadOnly = false;

    try
    {
        uno::Reference< ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content                       aCnt( rBaseURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext() );

        uno::Sequence<OUString> aProps { "Url" };

        uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

#if defined(MACOSX) && HAVE_FEATURE_READONLY_INSTALLSET
        if( rBaseURL.GetProtocol() == INetProtocol::File )
        {
            const char *appBundle = [[[NSBundle mainBundle] bundlePath] UTF8String];
            OUString path = rBaseURL.GetURLPath();
            if( path.startsWith( OUString( appBundle, strlen( appBundle ), RTL_TEXTENCODING_UTF8 ) + "/" ) )
                rbDirIsReadOnly = true;
        }
#else
        try
        {
            // check readonlyness the very hard way
            INetURLObject   aTestURL( rBaseURL );
            OUString        aTestFile( "cdefghij.klm" );

            aTestURL.Append( aTestFile );
            std::unique_ptr<SvStream> pTestStm(::utl::UcbStreamHelper::CreateStream( aTestURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE ));

            if( pTestStm )
            {
                pTestStm->WriteInt32( sal_Int32(1) );

                if( pTestStm->GetError() )
                    rbDirIsReadOnly = true;

                pTestStm.reset();
                KillFile( aTestURL );
            }
            else
                rbDirIsReadOnly = true;
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
#endif
        if( xResultSet.is() )
        {
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            if( xContentAccess.is() )
            {
                static const char s_sTitle[] = "Title";
                static const char s_sIsReadOnly[] = "IsReadOnly";

                while( xResultSet->next() )
                {
                    INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

                    if(aThmURL.GetExtension().equalsIgnoreAsciiCase("thm"))
                    {
                        INetURLObject   aSdgURL( aThmURL); aSdgURL.SetExtension( "sdg" );
                        INetURLObject   aSdvURL( aThmURL ); aSdvURL.SetExtension( "sdv" );
                        OUString        aTitle;

                        try
                        {
                            ::ucbhelper::Content aThmCnt( aThmURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext() );
                            ::ucbhelper::Content aSdgCnt( aSdgURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext() );
                            ::ucbhelper::Content aSdvCnt( aSdvURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), xEnv, comphelper::getProcessComponentContext() );

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
                                bool bReadOnly = false;

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
                                    catch( const css::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const css::uno::Exception& )
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
                                    catch( const css::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const css::uno::Exception& )
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
    GalleryThemeEntry* pFound = nullptr;

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
    GalleryThemeEntry* pFound = nullptr;

    for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
    {
        GalleryThemeEntry* pEntry = aThemeList[ i ];
        if( nThemeId == pEntry->GetId() )
            pFound = pEntry;
    }

    // try fallback, if no entry was found
    if( !pFound )
    {
        OUString aFallback;

        switch( nThemeId )
        {
            case GALLERY_THEME_3D:
                aFallback = GAL_RESSTR(RID_GALLERYSTR_THEME_3D);
                break;
            case GALLERY_THEME_BULLETS:
                aFallback = GAL_RESSTR(RID_GALLERYSTR_THEME_BULLETS);
                break;
            case GALLERY_THEME_HOMEPAGE:
                aFallback = GAL_RESSTR(RID_GALLERYSTR_THEME_HOMEPAGE);
                break;
            case GALLERY_THEME_POWERPOINT:
                aFallback = RID_GALLERYSTR_THEME_POWERPOINT;
                break;
            case GALLERY_THEME_FONTWORK:
                aFallback = RID_GALLERYSTR_THEME_FONTWORK;
                break;
            case GALLERY_THEME_FONTWORK_VERTICAL:
                aFallback = RID_GALLERYSTR_THEME_FONTWORK_VERTICAL;
                break;
            case GALLERY_THEME_SOUNDS:
                aFallback = GAL_RESSTR(RID_GALLERYSTR_THEME_SOUNDS);
                break;
            case RID_GALLERYSTR_THEME_ARROWS:
            case RID_GALLERYSTR_THEME_COMPUTERS:
            case RID_GALLERYSTR_THEME_DIAGRAMS:
            case RID_GALLERYSTR_THEME_EDUCATION:
            case RID_GALLERYSTR_THEME_ENVIRONMENT:
            case RID_GALLERYSTR_THEME_FINANCE:
            case RID_GALLERYSTR_THEME_PEOPLE:
            case RID_GALLERYSTR_THEME_SYMBOLS:
            case RID_GALLERYSTR_THEME_TRANSPORT:
            case RID_GALLERYSTR_THEME_TXTSHAPES:
                aFallback = GAL_RESSTR(static_cast<sal_uInt32>(nThemeId));
                break;
            default:
                break;
        }

        pFound = const_cast<Gallery*>(this)->ImplGetThemeEntry(aFallback);
    }

    return( pFound ? pFound->GetThemeName() : OUString() );
}

bool Gallery::HasTheme( const OUString& rThemeName )
{
    return( ImplGetThemeEntry( rThemeName ) != nullptr );
}

bool Gallery::CreateTheme( const OUString& rThemeName )
{
    bool bRet = false;

    if( !HasTheme( rThemeName ) && ( GetUserURL().GetProtocol() != INetProtocol::NotValid ) )
    {
        INetURLObject aURL( GetUserURL() );
        aURL.Append( rThemeName );
        GalleryThemeEntry* pNewEntry = new GalleryThemeEntry(
                true, aURL, rThemeName,
                false, true, 0, false );

        aThemeList.push_back( pNewEntry );
        delete( new GalleryTheme( this, pNewEntry ) );
        Broadcast( GalleryHint( GalleryHintType::THEME_CREATED, rThemeName ) );
        bRet = true;
    }

    return bRet;
}

void Gallery::RenameTheme( const OUString& rOldName, const OUString& rNewName )
{
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );

    // check if the new theme name is already present
    if( pThemeEntry && !HasTheme( rNewName ) && !pThemeEntry->IsReadOnly() )
    {
        SfxListener   aListener;
        GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

        if( pThm )
        {
            pThemeEntry->SetName( rNewName );
            pThm->ImplWrite();

            Broadcast( GalleryHint( GalleryHintType::THEME_RENAMED, rOldName, pThm->GetName() ) );
            ReleaseTheme( pThm, aListener );
        }
    }
}

bool Gallery::RemoveTheme( const OUString& rThemeName )
{
    GalleryThemeEntry*  pThemeEntry = ImplGetThemeEntry( rThemeName );
    bool                bRet = false;

    if( pThemeEntry && !pThemeEntry->IsReadOnly() )
    {
        Broadcast( GalleryHint( GalleryHintType::CLOSE_THEME, rThemeName ) );

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

        GalleryThemeList::const_iterator aEnd = aThemeList.end();
        for ( GalleryThemeList::iterator it = aThemeList.begin(); it != aEnd; ++it )
        {
            if ( pThemeEntry == *it ) {
                delete pThemeEntry;
                aThemeList.erase( it );
                break;
            }
        }

        Broadcast( GalleryHint( GalleryHintType::THEME_REMOVED, rThemeName ) );

        bRet = true;
    }

    return bRet;
}

GalleryTheme* Gallery::ImplGetCachedTheme(const GalleryThemeEntry* pThemeEntry)
{
    GalleryTheme* pTheme = nullptr;

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

            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

            if( FileExists( aURL ) )
            {
                std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

                if( pIStm )
                {
                    try
                    {
                        pTheme = new GalleryTheme( this, const_cast<GalleryThemeEntry*>(pThemeEntry) );
                        ReadGalleryTheme( *pIStm, *pTheme );

                        if( pIStm->GetError() )
                        {
                            delete pTheme;
                            pTheme = nullptr;
                        }
                    }
                    catch (const css::ucb::ContentCreationException&)
                    {
                    }
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
    GalleryCacheThemeList::const_iterator aEnd = aThemeCache.end();
    for (GalleryCacheThemeList::iterator it = aThemeCache.begin(); it != aEnd; ++it)
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
    GalleryTheme*           pTheme = nullptr;
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

    if( pThemeEntry && ( ( pTheme = ImplGetCachedTheme( pThemeEntry ) ) != nullptr ) )
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

bool GalleryThemeEntry::IsDefault() const
{
    return ( nId > 0 ) && ( nId != ( RID_GALLERYSTR_THEME_MYTHEME - RID_GALLERYSTR_THEME_START ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
