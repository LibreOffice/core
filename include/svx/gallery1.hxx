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
#include <vector>


// - GalleryThemeEntry -


class SvStream;

class GalleryThemeEntry
{
private:

    OUString                aName;
    INetURLObject           aThmURL;
    INetURLObject           aSdgURL;
    INetURLObject           aSdvURL;
    INetURLObject           aStrURL;
    sal_uInt32              nId;
    bool                    bReadOnly;
    bool                    bModified;
    bool                    bThemeNameFromResource;

                            GalleryThemeEntry();
    static INetURLObject    ImplGetURLIgnoreCase( const INetURLObject& rURL );

public:

                            GalleryThemeEntry( bool bCreateUniqueURL,
                                               const INetURLObject& rBaseURL,
                                               const OUString& rName,
                                               bool bReadOnly, bool bNewFile,
                                               sal_uInt32 nId, bool bThemeNameFromResource );
                            ~GalleryThemeEntry() {};

    const OUString&         GetThemeName() const { return aName; }

    const INetURLObject&    GetThmURL() const { return aThmURL; }
    const INetURLObject&    GetSdgURL() const { return aSdgURL; }
    const INetURLObject&    GetSdvURL() const { return aSdvURL; }
    const INetURLObject&    GetStrURL() const { return aStrURL; }

    OUString                ReadStrFromIni(const OUString &aKeyName );

    bool                    IsReadOnly() const { return bReadOnly; }
    bool                    IsDefault() const;

    bool                    IsHidden() const { return aName.match("private://gallery/hidden/"); }

    bool                    IsModified() const { return bModified; }
    void                    SetModified( bool bSet ) { bModified = ( bSet && !IsReadOnly() ); }

    void                    SetName( const OUString& rNewName );
    bool                    IsNameFromResource() const { return bThemeNameFromResource; }

    sal_uInt32              GetId() const { return nId; }
    void                    SetId( sal_uInt32 nNewId, bool bResetThemeName );
};

typedef ::std::vector< GalleryThemeEntry* > GalleryThemeList;


// - Gallery -


class SfxListener;
class GalleryTheme;
class GalleryThemeCacheEntry;

class SVX_DLLPUBLIC Gallery : public SfxBroadcaster
{
    // only for gengal utility!
    friend Gallery* createGallery( const OUString& );
    friend void     disposeGallery( Gallery* );

    typedef std::vector<GalleryThemeCacheEntry*> GalleryCacheThemeList;

private:

    GalleryThemeList            aThemeList;
    GalleryCacheThemeList       aThemeCache;
    INetURLObject               aRelURL;
    INetURLObject               aUserURL;
    rtl_TextEncoding            nReadTextEncoding;
    bool                        bMultiPath;

    SAL_DLLPRIVATE void         ImplLoad( const OUString& rMultiPath );
    SAL_DLLPRIVATE void         ImplLoadSubDirs( const INetURLObject& rBaseURL, bool& rbIsReadOnly );

    GalleryThemeEntry*          ImplGetThemeEntry( const OUString& rThemeName );

    SAL_DLLPRIVATE GalleryTheme* ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    SAL_DLLPRIVATE void         ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                Gallery( const OUString& rMultiPath );
                                virtual ~Gallery();

public:

    static Gallery*             GetGalleryInstance();

    SAL_DLLPRIVATE size_t       GetThemeCount() const { return aThemeList.size(); }
    SAL_DLLPRIVATE const GalleryThemeEntry* GetThemeInfo( size_t nPos )
                                { return nPos < aThemeList.size() ? aThemeList[ nPos ] : nullptr; }
    SAL_DLLPRIVATE const GalleryThemeEntry* GetThemeInfo( const OUString& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    bool                        HasTheme( const OUString& rThemeName );
    SAL_DLLPRIVATE OUString     GetThemeName( sal_uIntPtr nThemeId ) const;

    bool                        CreateTheme( const OUString& rThemeName );
    SAL_DLLPRIVATE bool         RenameTheme( const OUString& rOldName, const OUString& rNewName );
    bool                        RemoveTheme( const OUString& rThemeName );

    GalleryTheme*               AcquireTheme( const OUString& rThemeName, SfxListener& rListener );
    void                        ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    SAL_DLLPRIVATE const INetURLObject& GetUserURL() const { return aUserURL; }
    SAL_DLLPRIVATE const INetURLObject& GetRelativeURL() const { return aRelURL; }
};

#endif // INCLUDED_SVX_GALLERY1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
