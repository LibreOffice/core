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
#include <svl/brdcst.hxx>
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
    INetURLObject           ImplGetURLIgnoreCase( const INetURLObject& rURL ) const;

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
    bool                        bMultiPath;

    void                        ImplLoad( const OUString& rMultiPath );
    void                        ImplLoadSubDirs( const INetURLObject& rBaseURL, bool& rbIsReadOnly );

    SVX_DLLPUBLIC GalleryThemeEntry*            ImplGetThemeEntry( const OUString& rThemeName );
    GalleryThemeEntry*          ImplGetThemeEntry( sal_uIntPtr nThemeId );

    GalleryTheme*               ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    void                        ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                SVX_DLLPUBLIC Gallery( const OUString& rMultiPath );
                                SVX_DLLPUBLIC virtual ~Gallery();

public:

    SVX_DLLPUBLIC static Gallery* GetGalleryInstance();

    size_t                      GetThemeCount() const { return aThemeList.size(); }
    const GalleryThemeEntry*    GetThemeInfo( size_t nPos )
                                { return nPos < aThemeList.size() ? aThemeList[ nPos ] : NULL; }
    const GalleryThemeEntry*    GetThemeInfo( const OUString& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    SVX_DLLPUBLIC bool          HasTheme( const OUString& rThemeName );
    OUString                    GetThemeName( sal_uIntPtr nThemeId ) const;

    SVX_DLLPUBLIC bool          CreateTheme( const OUString& rThemeName );
    bool                        RenameTheme( const OUString& rOldName, const OUString& rNewName );
    SVX_DLLPUBLIC bool          RemoveTheme( const OUString& rThemeName );

    SVX_DLLPUBLIC GalleryTheme* AcquireTheme( const OUString& rThemeName, SfxListener& rListener );
    SVX_DLLPUBLIC void          ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    const INetURLObject&        GetUserURL() const { return aUserURL; }
    const INetURLObject&        GetRelativeURL() const { return aRelURL; }

    bool                        IsMultiPath() const { return bMultiPath; }
};

#endif // INCLUDED_SVX_GALLERY1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
