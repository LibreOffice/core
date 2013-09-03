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

#ifndef _SVX_GALLERY1_HXX_
#define _SVX_GALLERY1_HXX_

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <svl/brdcst.hxx>
#include "svx/svxdllapi.h"

#include <cstdio>
#include <vector>

// ---------------------
// - GalleryThemeEntry -
// ---------------------

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
    sal_Bool                bReadOnly;
    sal_Bool                bModified;
    sal_Bool                bThemeNameFromResource;

                            GalleryThemeEntry();
    INetURLObject           ImplGetURLIgnoreCase( const INetURLObject& rURL ) const;

public:

                            GalleryThemeEntry( const INetURLObject& rBaseURL, const OUString& rName,
                                               sal_Bool bReadOnly, sal_Bool bNewFile,
                                               sal_uInt32 nId, sal_Bool bThemeNameFromResource );
                            ~GalleryThemeEntry() {};

    const OUString&         GetThemeName() const { return aName; }

    const INetURLObject&    GetThmURL() const { return aThmURL; }
    const INetURLObject&    GetSdgURL() const { return aSdgURL; }
    const INetURLObject&    GetSdvURL() const { return aSdvURL; }
    const INetURLObject&    GetStrURL() const { return aStrURL; }

    OUString                ReadStrFromIni(const OUString &aKeyName );

    sal_Bool                IsReadOnly() const { return bReadOnly; }
    sal_Bool                IsDefault() const;

    sal_Bool                IsHidden() const { return aName.match("private://gallery/hidden/"); }

    sal_Bool                IsModified() const { return bModified; }
    void                    SetModified( sal_Bool bSet ) { bModified = ( bSet && !IsReadOnly() ); }

    void                    SetName( const OUString& rNewName );
    sal_Bool                IsNameFromResource() const { return bThemeNameFromResource; }

    sal_uInt32              GetId() const { return nId; }
    void                    SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName );
};

typedef ::std::vector< GalleryThemeEntry* > GalleryThemeList;

// -----------
// - Gallery -
// -----------

class SfxListener;
class GalleryTheme;
class GalleryThemeCacheEntry;

class Gallery : public SfxBroadcaster
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
    sal_Bool                    bMultiPath;

    void                        ImplLoad( const OUString& rMultiPath );
    void                        ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbIsReadOnly );

    SVX_DLLPUBLIC GalleryThemeEntry*            ImplGetThemeEntry( const OUString& rThemeName );
    GalleryThemeEntry*          ImplGetThemeEntry( sal_uIntPtr nThemeId );

    GalleryTheme*               ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    void                        ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                SVX_DLLPUBLIC Gallery( const OUString& rMultiPath );
                                SVX_DLLPUBLIC ~Gallery();

public:

    SVX_DLLPUBLIC static Gallery* GetGalleryInstance();

    size_t                      GetThemeCount() const { return aThemeList.size(); }
    const GalleryThemeEntry*    GetThemeInfo( size_t nPos )
                                { return nPos < aThemeList.size() ? aThemeList[ nPos ] : NULL; }
    const GalleryThemeEntry*    GetThemeInfo( const OUString& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    SVX_DLLPUBLIC sal_Bool      HasTheme( const OUString& rThemeName );
    OUString                    GetThemeName( sal_uIntPtr nThemeId ) const;

    SVX_DLLPUBLIC sal_Bool      CreateTheme( const OUString& rThemeName );
    sal_Bool                    RenameTheme( const OUString& rOldName, const OUString& rNewName );
    SVX_DLLPUBLIC sal_Bool      RemoveTheme( const OUString& rThemeName );

    SVX_DLLPUBLIC GalleryTheme* AcquireTheme( const OUString& rThemeName, SfxListener& rListener );
    SVX_DLLPUBLIC void          ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    const INetURLObject&        GetUserURL() const { return aUserURL; }
    const INetURLObject&        GetRelativeURL() const { return aRelURL; }

    sal_Bool                    IsMultiPath() const { return bMultiPath; }
};

#endif // _SVX_GALLERY1_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
