/*************************************************************************
 *
 *  $RCSfile: gallery1.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 17:24:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_GALLERY1_HXX_
#define _SVX_GALLERY1_HXX_

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <svtools/brdcst.hxx>
#include "gallery.hrc"

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
    BOOL                    IsDefault() const { return( ( nId > 0 ) && ( nId != ( RID_GALLERYSTR_THEME_MYTHEME - RID_GALLERYSTR_THEME_START ) ) ); }

    BOOL                    IsHidden() const { return aName.Search( String( RTL_CONSTASCII_USTRINGPARAM( "private://gallery/hidden/" ) ) ) == 0; }

    BOOL                    IsModified() const { return bModified; }
    void                    SetModified( BOOL bSet ) { bModified = ( bSet && !IsImported() && !IsReadOnly() ); }

    void                    SetName( const String& rNewName );
    BOOL                    IsNameFromResource() const { return bThemeNameFromResource; }

    UINT32                  GetId() const { return nId; }
    void                    SetId( UINT32 nNewId, BOOL bResetThemeName );
};

DECLARE_LIST( GalleryThemeList, GalleryThemeEntry* );

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
DECLARE_LIST( GalleryImportThemeList, GalleryImportThemeEntry* );

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
    friend class GalleryCacheEntry;

private:

    static List                 aGalleryCache;

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

    GalleryThemeEntry*          ImplGetThemeEntry( const String& rThemeName );
    GalleryThemeEntry*          ImplGetThemeEntry( ULONG nThemeId );
    GalleryImportThemeEntry*    ImplGetImportThemeEntry( const String& rImportName );

    GalleryTheme*               ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry );
    void                        ImplDeleteCachedTheme( GalleryTheme* pTheme );

                                Gallery( const String& rMultiPath );
                                ~Gallery();

public:

    static Gallery*             AcquireGallery( const String& rMultiPath );
    static void                 ReleaseGallery( Gallery* pGallery );

    const ULONG                 GetThemeCount() const { return aThemeList.Count(); }
    const GalleryThemeEntry*    GetThemeInfo( ULONG nPos ) { return aThemeList.GetObject( nPos ); }
    const GalleryThemeEntry*    GetThemeInfo( const String& rThemeName ) { return ImplGetThemeEntry( rThemeName ); }

    BOOL                        HasTheme( const String& rThemeName );
    String                      GetThemeName( ULONG nThemeId ) const;

    BOOL                        CreateTheme( const String& rThemeName );
    BOOL                        CreateImportTheme( const INetURLObject& rURL, const String& rFileName );
    BOOL                        RenameTheme( const String& rOldName, const String& rNewName );
    BOOL                        RemoveTheme( const String& rThemeName );

    GalleryTheme*               AcquireTheme( const String& rThemeName, SfxListener& rListener );
    void                        ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener );

public:

    INetURLObject               GetImportURL( const String& rThemeName );

    const INetURLObject&        GetUserURL() const { return aUserURL; }
    const INetURLObject&        GetRelativeURL() const { return aRelURL; }

    BOOL                        IsMultiPath() const { return bMultiPath; }
};

#endif // _SVX_GALLERY1_HXX_
