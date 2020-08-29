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

#include <sal/config.h>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/dialmgr.hxx>
#include <svx/gallery.hxx>
#include <svx/strings.hrc>
#include <strings.hxx>
#include <svx/galmisc.hxx>
#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>
#include <svx/gallerybinaryengineentry.hxx>
#include <galobj.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <memory>


using namespace ::com::sun::star;


const std::pair<sal_uInt16, const char*> aUnlocalized[] =
{
    { GALLERY_THEME_HOMEPAGE, RID_GALLERYSTR_THEME_HTMLBUTTONS },
    { GALLERY_THEME_POWERPOINT, RID_GALLERYSTR_THEME_POWERPOINT },
    { GALLERY_THEME_USERSOUNDS, RID_GALLERYSTR_THEME_USERSOUNDS },
    { GALLERY_THEME_DUMMY5, RID_GALLERYSTR_THEME_DUMMY5 },
    { GALLERY_THEME_RULERS, RID_GALLERYSTR_THEME_RULERS },
    { GALLERY_THEME_FONTWORK, RID_GALLERYSTR_THEME_FONTWORK },
    { GALLERY_THEME_FONTWORK_VERTICAL, RID_GALLERYSTR_THEME_FONTWORK_VERTICAL }
};

const std::pair<sal_uInt16, const char*> aLocalized[] =
{
    { RID_GALLERY_THEME_3D, RID_GALLERYSTR_THEME_3D },
    { RID_GALLERY_THEME_ANIMATIONS, RID_GALLERYSTR_THEME_ANIMATIONS },
    { RID_GALLERY_THEME_BULLETS, RID_GALLERYSTR_THEME_BULLETS },
    { RID_GALLERY_THEME_OFFICE, RID_GALLERYSTR_THEME_OFFICE },
    { RID_GALLERY_THEME_FLAGS, RID_GALLERYSTR_THEME_FLAGS },
    { RID_GALLERY_THEME_FLOWCHARTS, RID_GALLERYSTR_THEME_FLOWCHARTS },
    { RID_GALLERY_THEME_EMOTICONS, RID_GALLERYSTR_THEME_EMOTICONS },
    { RID_GALLERY_THEME_PHOTOS, RID_GALLERYSTR_THEME_PHOTOS },
    { RID_GALLERY_THEME_BACKGROUNDS, RID_GALLERYSTR_THEME_BACKGROUNDS },
    { RID_GALLERY_THEME_HOMEPAGE, RID_GALLERYSTR_THEME_HOMEPAGE },
    { RID_GALLERY_THEME_INTERACTION, RID_GALLERYSTR_THEME_INTERACTION },
    { RID_GALLERY_THEME_MAPS, RID_GALLERYSTR_THEME_MAPS },
    { RID_GALLERY_THEME_PEOPLE, RID_GALLERYSTR_THEME_PEOPLE },
    { RID_GALLERY_THEME_SURFACES, RID_GALLERYSTR_THEME_SURFACES },
    { RID_GALLERY_THEME_SOUNDS, RID_GALLERYSTR_THEME_SOUNDS },
    { RID_GALLERY_THEME_SYMBOLS, RID_GALLERYSTR_THEME_SYMBOLS },
    { RID_GALLERY_THEME_MYTHEME, RID_GALLERYSTR_THEME_MYTHEME },

    { RID_GALLERY_THEME_ARROWS, RID_GALLERYSTR_THEME_ARROWS },
    { RID_GALLERY_THEME_BALLOONS, RID_GALLERYSTR_THEME_BALLOONS },
    { RID_GALLERY_THEME_KEYBOARD, RID_GALLERYSTR_THEME_KEYBOARD },
    { RID_GALLERY_THEME_TIME, RID_GALLERYSTR_THEME_TIME },
    { RID_GALLERY_THEME_PRESENTATION, RID_GALLERYSTR_THEME_PRESENTATION },
    { RID_GALLERY_THEME_CALENDAR, RID_GALLERYSTR_THEME_CALENDAR },
    { RID_GALLERY_THEME_NAVIGATION, RID_GALLERYSTR_THEME_NAVIGATION },
    { RID_GALLERY_THEME_COMMUNICATION, RID_GALLERYSTR_THEME_COMMUNICATION },
    { RID_GALLERY_THEME_FINANCES, RID_GALLERYSTR_THEME_FINANCES },
    { RID_GALLERY_THEME_COMPUTER, RID_GALLERYSTR_THEME_COMPUTER },

    { RID_GALLERY_THEME_CLIMA, RID_GALLERYSTR_THEME_CLIMA },
    { RID_GALLERY_THEME_EDUCATION, RID_GALLERYSTR_THEME_EDUCATION },
    { RID_GALLERY_THEME_TROUBLE, RID_GALLERYSTR_THEME_TROUBLE },
    { RID_GALLERY_THEME_SCREENBEANS, RID_GALLERYSTR_THEME_SCREENBEANS },

    { RID_GALLERY_THEME_COMPUTERS, RID_GALLERYSTR_THEME_COMPUTERS },
    { RID_GALLERY_THEME_DIAGRAMS, RID_GALLERYSTR_THEME_DIAGRAMS },
    { RID_GALLERY_THEME_ENVIRONMENT, RID_GALLERYSTR_THEME_ENVIRONMENT },
    { RID_GALLERY_THEME_FINANCE, RID_GALLERYSTR_THEME_FINANCE },
    { RID_GALLERY_THEME_TRANSPORT, RID_GALLERYSTR_THEME_TRANSPORT },
    { RID_GALLERY_THEME_TXTSHAPES, RID_GALLERYSTR_THEME_TXTSHAPES }
};

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
        GalleryBinaryEngineEntry::CreateUniqueURL(rBaseURL,aURL);
    }

    mpGalleryStorageEngineEntry = std::make_unique<GalleryBinaryEngineEntry>();
    setStorageLocations(aURL);

    SetModified( _bNewFile );

    aName = mpGalleryStorageEngineEntry->ReadStrFromIni( "name" );

    // This is awful - we shouldn't use these resources if we
    // possibly can avoid them
    if( aName.isEmpty() && nId && bThemeNameFromResource )
    {
        //some of these are supposed to *not* be localized
        //so catch them before looking up the resource
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
        {
            for (size_t i = 0; i < SAL_N_ELEMENTS(aLocalized); ++i)
            {
                if (aLocalized[i].first == nId)
                {
                    aName = SvxResId(aLocalized[i].second);
                    break;
                }
            }
        }
    }

    if( aName.isEmpty() )
        aName = rName;
}

void GalleryThemeEntry::setStorageLocations(INetURLObject& rURL)
{
    mpGalleryStorageEngineEntry->setStorageLocations(rURL);
}

void GalleryTheme::InsertAllThemes(weld::ComboBox& rListBox)
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(aUnlocalized); ++i)
        rListBox.append_text(OUString::createFromAscii(aUnlocalized[i].second));

    for (size_t i = 0; i < SAL_N_ELEMENTS(aLocalized); ++i)
        rListBox.append_text(SvxResId(aLocalized[i].second));
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

void GalleryThemeEntry::removeTheme()
{
    mpGalleryStorageEngineEntry->removeTheme();
}

class GalleryThemeCacheEntry
{
private:

    const GalleryThemeEntry*        mpThemeEntry;
    std::unique_ptr<GalleryTheme>   mpTheme;

public:

                                GalleryThemeCacheEntry( const GalleryThemeEntry* pThemeEntry, std::unique_ptr<GalleryTheme> pTheme ) :
                                    mpThemeEntry( pThemeEntry ), mpTheme( std::move(pTheme) ) {}

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
}

Gallery* Gallery::GetGalleryInstance()
{
    // note: this would deadlock if it used osl::Mutex::getGlobalMutex()
    static Gallery *const s_pGallery(
        utl::ConfigManager::IsFuzzing() ? nullptr :
            new Gallery(SvtPathOptions().GetGalleryPath()));

    return s_pGallery;
}

void Gallery::ImplLoad( const OUString& rMultiPath )
{
    bool bIsReadOnlyDir {false};

    bMultiPath = !rMultiPath.isEmpty();

    INetURLObject aCurURL(SvtPathOptions().GetConfigPath());
    ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

    if( !bIsReadOnlyDir )
        aUserURL = aCurURL;

    if( bMultiPath )
    {
        bool bIsRelURL {true};
        sal_Int32 nIdx {0};
        do
        {
            aCurURL = INetURLObject(rMultiPath.getToken(0, ';', nIdx));
            if (bIsRelURL)
            {
                aRelURL = aCurURL;
                bIsRelURL = false;
            }

            ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

            if( !bIsReadOnlyDir )
                aUserURL = aCurURL;
        }
        while (nIdx>0);
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

            aTestURL.Append( "cdefghij.klm" );
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

                    if (aThmURL.GetFileExtension().equalsIgnoreAsciiCase("thm"))
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

                                GalleryThemeEntry* pEntry = GalleryBinaryEngineEntry::CreateThemeEntry( aThmURL, rbDirIsReadOnly || bReadOnly );

                                if( pEntry )
                                    aThemeList.emplace_back( pEntry );
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
    if( !rThemeName.isEmpty() )
    {
        for ( size_t i = 0, n = aThemeList.size(); i < n; ++i )
            if( rThemeName == aThemeList[ i ]->GetThemeName() )
                return aThemeList[ i ].get();
    }

    return nullptr;
}

OUString Gallery::GetThemeName( sal_uInt32 nThemeId ) const
{
    GalleryThemeEntry* pFound = nullptr;

    for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
    {
        GalleryThemeEntry* pEntry = aThemeList[ i ].get();
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
                aFallback = SvxResId(RID_GALLERYSTR_THEME_3D);
                break;
            case GALLERY_THEME_BULLETS:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_BULLETS);
                break;
            case GALLERY_THEME_HOMEPAGE:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_HOMEPAGE);
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
                aFallback = SvxResId(RID_GALLERYSTR_THEME_SOUNDS);
                break;
            case RID_GALLERY_THEME_ARROWS:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_ARROWS);
                break;
            case RID_GALLERY_THEME_COMPUTERS:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_COMPUTERS);
                break;
            case RID_GALLERY_THEME_DIAGRAMS:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_DIAGRAMS);
                break;
            case RID_GALLERY_THEME_EDUCATION:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_EDUCATION);
                break;
            case RID_GALLERY_THEME_ENVIRONMENT:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_ENVIRONMENT);
                break;
            case RID_GALLERY_THEME_FINANCE:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_FINANCE);
                break;
            case RID_GALLERY_THEME_PEOPLE:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_PEOPLE);
                break;
            case RID_GALLERY_THEME_SYMBOLS:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_SYMBOLS);
                break;
            case RID_GALLERY_THEME_TRANSPORT:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_TRANSPORT);
                break;
            case RID_GALLERY_THEME_TXTSHAPES:
                aFallback = SvxResId(RID_GALLERYSTR_THEME_TXTSHAPES);
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

        aThemeList.emplace_back( pNewEntry );
        delete new GalleryTheme( this, pNewEntry );
        Broadcast( GalleryHint( GalleryHintType::THEME_CREATED, rThemeName ) );
        bRet = true;
    }

    return bRet;
}

void Gallery::RenameTheme( const OUString& rOldName, const OUString& rNewName )
{
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );

    // check if the new theme name is already present
    if( !pThemeEntry || HasTheme( rNewName ) || pThemeEntry->IsReadOnly() )
        return;

    SfxListener   aListener;
    GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

    if( pThm )
    {
        pThemeEntry->SetName( rNewName );
        if (pThm->pThm->IsModified())
            if (!pThm->mpGalleryStorageEngine->implWrite(*pThm, pThm->pThm))
                pThm->ImplSetModified(false);

        Broadcast( GalleryHint( GalleryHintType::THEME_RENAMED, rOldName, pThm->GetName() ) );
        ReleaseTheme( pThm, aListener );
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
            ReleaseTheme(pThm, aListener);
            pThemeEntry->removeTheme();
        }

        auto it = std::find_if(aThemeList.begin(), aThemeList.end(),
            [&pThemeEntry](const std::unique_ptr<GalleryThemeEntry>& rpEntry) { return pThemeEntry == rpEntry.get(); });
        if (it != aThemeList.end())
            aThemeList.erase( it );

        Broadcast( GalleryHint( GalleryHintType::THEME_REMOVED, rThemeName ) );

        bRet = true;
    }

    return bRet;
}

std::unique_ptr<GalleryTheme> GalleryThemeEntry::getCachedTheme(Gallery* pGallery) const
{
    std::unique_ptr<GalleryTheme> pNewTheme;
    pNewTheme.reset(new GalleryTheme(pGallery, const_cast<GalleryThemeEntry*>(this)));
    mpGalleryStorageEngineEntry->getCachedTheme(pNewTheme);
    return pNewTheme;
}

GalleryTheme* Gallery::ImplGetCachedTheme(const GalleryThemeEntry* pThemeEntry)
{
    GalleryTheme* pTheme = nullptr;

    if( pThemeEntry )
    {
        auto it = std::find_if(aThemeCache.begin(), aThemeCache.end(),
            [&pThemeEntry](const GalleryThemeCacheEntry* pEntry) { return pThemeEntry == pEntry->GetThemeEntry(); });
        if (it != aThemeCache.end())
            pTheme = (*it)->GetTheme();

        if( !pTheme )
        {
            std::unique_ptr<GalleryTheme> pNewTheme = pThemeEntry->getCachedTheme(this);
            if (pNewTheme)
            {
                aThemeCache.push_back( new GalleryThemeCacheEntry( pThemeEntry, std::move(pNewTheme) ));
                pTheme = aThemeCache.back()->GetTheme();
            }
        }
    }

    return pTheme;
}

void Gallery::ImplDeleteCachedTheme( GalleryTheme const * pTheme )
{
    auto it = std::find_if(aThemeCache.begin(), aThemeCache.end(),
        [&pTheme](const GalleryThemeCacheEntry* pEntry) { return pTheme == pEntry->GetTheme(); });
    if (it != aThemeCache.end())
    {
        delete *it;
        aThemeCache.erase(it);
    }
}

GalleryTheme* Gallery::AcquireTheme( const OUString& rThemeName, SfxListener& rListener )
{
    GalleryTheme*           pTheme = nullptr;
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

    if( pThemeEntry )
    {
        pTheme = ImplGetCachedTheme( pThemeEntry );
        if (pTheme)
            rListener.StartListening(*pTheme, DuplicateHandling::Prevent);
    }
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
    return nId > 0 && nId != GALLERY_THEME_MYTHEME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
