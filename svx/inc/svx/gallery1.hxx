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
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <svl/brdcst.hxx>
#include "svx/svxdllapi.h"
//#include "gallery.hrc"

#include <cstdio>
#include <list>

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
    sal_uInt32                  nFileNumber;
    sal_uInt32                  nId;
    sal_Bool                    bReadOnly;
    sal_Bool                    bImported;
    sal_Bool                    bModified;
    sal_Bool                    bThemeNameFromResource;

                            GalleryThemeEntry();
    INetURLObject           ImplGetURLIgnoreCase( const INetURLObject& rURL ) const;

public:

                            GalleryThemeEntry( const INetURLObject& rBaseURL, const String& rName,
                                               sal_uInt32 nFileNumber, sal_Bool bReadOnly, sal_Bool bImported,
                                               sal_Bool bNewFile, sal_uInt32 nId, sal_Bool bThemeNameFromResource );
                            ~GalleryThemeEntry() {};

    const String&           GetThemeName() const { return aName; }
    sal_uInt32                  GetFileNumber() const { return nFileNumber; }

    const INetURLObject&    GetThmURL() const { return aThmURL; }
    const INetURLObject&    GetSdgURL() const { return aSdgURL; }
    const INetURLObject&    GetSdvURL() const { return aSdvURL; }

    sal_Bool                    IsImported() const { return bImported; }
    sal_Bool                    IsReadOnly() const { return bReadOnly; }
    sal_Bool                    IsDefault() const;

    sal_Bool                    IsHidden() const { return aName.SearchAscii( "private://gallery/hidden/" ) == 0; }

    sal_Bool                    IsModified() const { return bModified; }
    void                    SetModified( sal_Bool bSet ) { bModified = ( bSet && !IsImported() && !IsReadOnly() ); }

    void                    SetName( const String& rNewName );
    sal_Bool                    IsNameFromResource() const { return bThemeNameFromResource; }

    sal_uInt32                  GetId() const { return nId; }
    void                    SetId( sal_uInt32 nNewId, sal_Bool bResetThemeName );
};

DECLARE_LIST( GalleryThemeList, GalleryThemeEntry* )

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
DECLARE_LIST( GalleryImportThemeList, GalleryImportThemeEntry* )

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
    sal_uIntPtr                     nLastFileNumber;
    sal_Bool                        bMultiPath;

    void                        ImplLoad( const String& rMultiPath );
    void                        ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbIsReadOnly );
    void                        ImplLoadImports();
    void                        ImplWriteImportList();

    SVX_DLLPUBLIC GalleryThemeEntry*            ImplGetThemeEntry( const String& rThemeName );
    GalleryThemeEntry*          ImplGetThemeEntry( sal_uIntPtr nThemeId );
    GalleryImportThemeEntry*    ImplGetImportThemeEntry( const String& rImportName );

    GalleryTheme*               ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    void                        ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                SVX_DLLPUBLIC Gallery( const String& rMultiPath );
                                SVX_DLLPUBLIC ~Gallery();

public:

    SVX_DLLPUBLIC static Gallery* GetGalleryInstance();

    sal_uIntPtr                 GetThemeCount() const { return aThemeList.Count(); }
    const GalleryThemeEntry*    GetThemeInfo( sal_uIntPtr nPos ) { return aThemeList.GetObject( nPos ); }
    const GalleryThemeEntry*    GetThemeInfo( const String& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    SVX_DLLPUBLIC sal_Bool          HasTheme( const String& rThemeName );
    String                      GetThemeName( sal_uIntPtr nThemeId ) const;

    SVX_DLLPUBLIC sal_Bool          CreateTheme( const String& rThemeName, sal_uInt32 nNumFrom = 0 );
    sal_Bool                        CreateImportTheme( const INetURLObject& rURL, const String& rFileName );
    sal_Bool                        RenameTheme( const String& rOldName, const String& rNewName );
    SVX_DLLPUBLIC sal_Bool                      RemoveTheme( const String& rThemeName );

    SVX_DLLPUBLIC GalleryTheme* AcquireTheme( const String& rThemeName, SfxListener& rListener );
    SVX_DLLPUBLIC void          ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    INetURLObject               GetImportURL( const String& rThemeName );

    const INetURLObject&        GetUserURL() const { return aUserURL; }
    const INetURLObject&        GetRelativeURL() const { return aRelURL; }

    sal_Bool                        IsMultiPath() const { return bMultiPath; }
};

#endif // _SVX_GALLERY1_HXX_
