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

#ifndef INCLUDED_SVX_GALLERY1_HXX
#define INCLUDED_SVX_GALLERY1_HXX

#include <rtl/ustring.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svx/svxdllapi.h>
#include <tools/urlobj.hxx>

#include <cstdio>
#include <memory>
#include <vector>

class Gallery;
class GalleryFileStorage;
class GalleryFileStorageEntry;
class GalleryObjectCollection;
class GalleryStorageLocations;
class GalleryTheme;

class SVXCORE_DLLPUBLIC GalleryThemeEntry
{
private:

    std::unique_ptr<GalleryFileStorageEntry> mpGalleryStorageEngineEntry;
    OUString                aName;
    sal_uInt32              nId;
    bool                    bReadOnly;
    bool                    bModified;
    bool                    bThemeNameFromResource;

public:
                            GalleryThemeEntry( bool bCreateUniqueURL,
                                               const INetURLObject& rBaseURL,
                                               const OUString& rName,
                                               bool bReadOnly, bool bNewFile,
                                               sal_uInt32 nId, bool bThemeNameFromResource );
    ~GalleryThemeEntry();
    
    GalleryStorageLocations& getGalleryStorageLocations() const;

    GalleryTheme* createGalleryTheme(Gallery* pGallery);

    std::unique_ptr<GalleryFileStorage> createGalleryStorageEngine(GalleryObjectCollection& mrGalleryObjectCollection);

    const OUString&         GetThemeName() const { return aName; }

    bool                    IsReadOnly() const { return bReadOnly; }
    bool                    IsDefault() const;

    bool                    IsHidden() const { return aName.match("private://gallery/hidden/"); }

    bool                    IsModified() const { return bModified; }
    void                    SetModified( bool bSet ) { bModified = ( bSet && !IsReadOnly() ); }

    void                    SetName( const OUString& rNewName );
    bool                    IsNameFromResource() const { return bThemeNameFromResource; }

    sal_uInt32              GetId() const { return nId; }
    void                    SetId( sal_uInt32 nNewId, bool bResetThemeName );


    void removeTheme();

    std::unique_ptr<GalleryTheme> getCachedTheme(Gallery* pGallery);

    void setStorageLocations(INetURLObject& rURL);
};

class SfxListener;
class GalleryThemeCacheEntry;


class SVXCORE_DLLPUBLIC Gallery final : public SfxBroadcaster
{
    typedef std::vector<GalleryThemeCacheEntry*> GalleryCacheThemeList;

private:

    std::vector< std::unique_ptr<GalleryThemeEntry> > m_aThemeList;
    GalleryCacheThemeList       m_aThemeCache;
    INetURLObject               m_aRelURL;
    INetURLObject               m_aUserURL;
    bool                        m_bMultiPath;

    SAL_DLLPRIVATE void         ImplLoad( std::u16string_view rMultiPath );
    SAL_DLLPRIVATE void         ImplLoadSubDirs( const INetURLObject& rBaseURL, bool& rbIsReadOnly );

    GalleryThemeEntry*          ImplGetThemeEntry( std::u16string_view rThemeName );

    SAL_DLLPRIVATE GalleryTheme* ImplGetCachedTheme( GalleryThemeEntry* pThemeEntry );
    SAL_DLLPRIVATE void         ImplDeleteCachedTheme( GalleryTheme const * pTheme );

    Gallery&                    operator=( Gallery const & ) = delete; // MSVC2015 workaround
                                Gallery( Gallery const & ) = delete; // MSVC2015 workaround

public:
                                // only for gengal utility!
                                Gallery( std::u16string_view rMultiPath );
                                virtual ~Gallery() override;

    static Gallery*             GetGalleryInstance();

    size_t                      GetThemeCount() const { return m_aThemeList.size(); }
    SAL_DLLPRIVATE const GalleryThemeEntry* GetThemeInfo( size_t nPos )
                                { return nPos < m_aThemeList.size() ? m_aThemeList[ nPos ].get() : nullptr; }
    const GalleryThemeEntry* GetThemeInfo( std::u16string_view rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    bool                        HasTheme( std::u16string_view rThemeName );
    SAL_DLLPRIVATE OUString     GetThemeName( sal_uInt32 nThemeId ) const;

    bool                        CreateTheme( const OUString& rThemeName );
    void                        RenameTheme( const OUString& rOldName, const OUString& rNewName );
    bool                        RemoveTheme( const OUString& rThemeName );

    SAL_RET_MAYBENULL GalleryTheme* AcquireTheme( std::u16string_view rThemeName, SfxListener& rListener );
    void                        ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    SAL_DLLPRIVATE const INetURLObject& GetUserURL() const { return m_aUserURL; }
    SAL_DLLPRIVATE const INetURLObject& GetRelativeURL() const { return m_aRelURL; }
};

#endif // INCLUDED_SVX_GALLERY1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
