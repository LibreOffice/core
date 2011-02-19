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

#ifndef _SVX_GALLERY1_HXX_
#define _SVX_GALLERY1_HXX_

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <svl/brdcst.hxx>
#include "svx/svxdllapi.h"

#include <cstdio>
#include <list>
#include <vector>

// ---------------------
// - GalleryThemeEntry -
// ---------------------

class SvStream;

class GalleryThemeEntry
{
private:

    String                  aName;
    INetURLObject           aThmURL;
    INetURLObject           aSdgURL;
    INetURLObject           aSdvURL;
    UINT32                  nFileNumber;
    UINT32                  nId;
    BOOL                    bReadOnly;
    BOOL                    bImported;
    BOOL                    bModified;
    BOOL                    bThemeNameFromResource;

                            GalleryThemeEntry();
    INetURLObject           ImplGetURLIgnoreCase( const INetURLObject& rURL ) const;

public:

                            GalleryThemeEntry( const INetURLObject& rBaseURL, const String& rName,
                                               UINT32 nFileNumber, BOOL bReadOnly, BOOL bImported,
                                               BOOL bNewFile, UINT32 nId, BOOL bThemeNameFromResource );
                            ~GalleryThemeEntry() {};

    const String&           GetThemeName() const { return aName; }
    UINT32                  GetFileNumber() const { return nFileNumber; }

    const INetURLObject&    GetThmURL() const { return aThmURL; }
    const INetURLObject&    GetSdgURL() const { return aSdgURL; }
    const INetURLObject&    GetSdvURL() const { return aSdvURL; }

    BOOL                    IsImported() const { return bImported; }
    BOOL                    IsReadOnly() const { return bReadOnly; }
    BOOL                    IsDefault() const;

    BOOL                    IsHidden() const { return aName.SearchAscii( "private://gallery/hidden/" ) == 0; }

    BOOL                    IsModified() const { return bModified; }
    void                    SetModified( BOOL bSet ) { bModified = ( bSet && !IsImported() && !IsReadOnly() ); }

    void                    SetName( const String& rNewName );
    BOOL                    IsNameFromResource() const { return bThemeNameFromResource; }

    UINT32                  GetId() const { return nId; }
    void                    SetId( UINT32 nNewId, BOOL bResetThemeName );
};

typedef ::std::vector< GalleryThemeEntry* > GalleryThemeList;

// ---------------------------
// - GalleryImportThemeEntry -
// ---------------------------

struct GalleryImportThemeEntry
{
    String          aThemeName;
    String          aUIName;
    INetURLObject   aURL;
    String          aImportName;
};

typedef ::std::vector< GalleryImportThemeEntry* > GalleryImportThemeList;

// -----------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOut, const GalleryImportThemeEntry& rEntry );
SvStream& operator>>( SvStream& rIn, GalleryImportThemeEntry& rEntry );

// -----------
// - Gallery -
// -----------

class SfxListener;
class GalleryTheme;

class Gallery : public SfxBroadcaster
{
    // only for gengal utility!
    friend Gallery* createGallery( const rtl::OUString& );
    friend void disposeGallery( Gallery* );

private:

    GalleryThemeList            aThemeList;
    GalleryImportThemeList      aImportList;
    List                        aThemeCache;
    INetURLObject               aRelURL;
    INetURLObject               aUserURL;
    rtl_TextEncoding            nReadTextEncoding;
    ULONG                       nLastFileNumber;
    BOOL                        bMultiPath;

    void                        ImplLoad( const String& rMultiPath );
    void                        ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbIsReadOnly );
    void                        ImplLoadImports();
    void                        ImplWriteImportList();

    SVX_DLLPUBLIC GalleryThemeEntry*            ImplGetThemeEntry( const String& rThemeName );
    GalleryThemeEntry*          ImplGetThemeEntry( ULONG nThemeId );
    GalleryImportThemeEntry*    ImplGetImportThemeEntry( const String& rImportName );

    GalleryTheme*               ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    void                        ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                SVX_DLLPUBLIC Gallery( const String& rMultiPath );
                                SVX_DLLPUBLIC ~Gallery();

public:

    SVX_DLLPUBLIC static Gallery* GetGalleryInstance();

    size_t                      GetThemeCount() const { return aThemeList.size(); }
    const GalleryThemeEntry*    GetThemeInfo( size_t nPos )
                                { return nPos < aThemeList.size() ? aThemeList[ nPos ] : NULL; }
    const GalleryThemeEntry*    GetThemeInfo( const String& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    SVX_DLLPUBLIC BOOL          HasTheme( const String& rThemeName );
    String                      GetThemeName( ULONG nThemeId ) const;

    SVX_DLLPUBLIC BOOL          CreateTheme( const String& rThemeName, UINT32 nNumFrom = 0 );
    BOOL                        CreateImportTheme( const INetURLObject& rURL, const String& rFileName );
    BOOL                        RenameTheme( const String& rOldName, const String& rNewName );
    SVX_DLLPUBLIC BOOL                      RemoveTheme( const String& rThemeName );

    SVX_DLLPUBLIC GalleryTheme* AcquireTheme( const String& rThemeName, SfxListener& rListener );
    SVX_DLLPUBLIC void          ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    INetURLObject               GetImportURL( const String& rThemeName );

    const INetURLObject&        GetUserURL() const { return aUserURL; }
    const INetURLObject&        GetRelativeURL() const { return aRelURL; }

    BOOL                        IsMultiPath() const { return bMultiPath; }
};

#endif // _SVX_GALLERY1_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
