/*************************************************************************
 *
 *  $RCSfile: gallery1.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-16 12:17:44 $
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

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/vcompat.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include "gallery.hxx"
#include "galmisc.hxx"
#include "galtheme.hxx"
#include "gallery1.hxx"

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#define ENABLE_BYTESTRING_STREAM_OPERATORS

// --------------
// - Namespaces -
// --------------

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;

// ---------------------
// - GalleryCacheEntry -
// ---------------------

class GalleryCacheEntry
{
private:

    Gallery*                mpGallery;
    String                  maMultiPath;
    ULONG                   mnRefCount;

public:

                            GalleryCacheEntry( Gallery* pGallery, const String& rMultiPath ) :
                                mpGallery( pGallery ), maMultiPath( rMultiPath ) {}
                            ~GalleryCacheEntry() { delete mpGallery; }

    Gallery*                GetGallery() const { return mpGallery; }
    const String&           GetMultiPath() const { return maMultiPath; }

    ULONG                   GetRefCount() const { return mnRefCount; }
    void                    IncRefCount() { mnRefCount++; }
    void                    DecRefCount() { mnRefCount--; }
};

// ---------------------
// - GalleryThemeEntry -
// ---------------------

GalleryThemeEntry::GalleryThemeEntry( const INetURLObject& rBaseURL, const String& rName,
                                      UINT32 _nFileNumber, BOOL _bReadOnly, BOOL _bImported,
                                      BOOL _bNewFile, UINT32 _nId, BOOL _bThemeNameFromResource ) :
        nFileNumber                             ( _nFileNumber ),
        nId                                     ( _nId ),
        bReadOnly                               ( _bReadOnly || _bImported ),
        bImported                               ( _bImported ),
        bThemeNameFromResource  ( _bThemeNameFromResource )
{
    INetURLObject aURL( rBaseURL );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
    String aFileName( String( RTL_CONSTASCII_USTRINGPARAM( "sg" ) ) );

    aURL.Append( ( aFileName += String::CreateFromInt32( nFileNumber ) ) += String( RTL_CONSTASCII_USTRINGPARAM( ".thm" ) ) );
    aThmURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdg" ) ) );
    aSdgURL = ImplGetURLIgnoreCase( aURL );

    aURL.setExtension( String( RTL_CONSTASCII_USTRINGPARAM( "sdv" ) ) );
    aSdvURL = ImplGetURLIgnoreCase( aURL );

    SetModified( _bNewFile );

    if( nId && bThemeNameFromResource )
        aName = String( GAL_RESID( RID_GALLERYSTR_THEME_START + (USHORT) nId ) );

    if( !aName.Len() )
        aName = rName;
}

// -----------------------------------------------------------------------------

INetURLObject GalleryThemeEntry::ImplGetURLIgnoreCase( const INetURLObject& rURL ) const
{
    INetURLObject   aURL( rURL );
    String          aFileName;
    BOOL            bExists = FALSE;

    // check original file name
    if( FileExists( aURL ) )
        bExists = TRUE;
    else
    {
        // check upper case file name
        aURL.setName( aURL.getName().ToUpperAscii() );

        if( FileExists( aURL ) )
            bExists = TRUE;
        else
        {
            // check lower case file name
            aURL.setName( aURL.getName().ToLowerAscii() );

            if( FileExists( aURL ) )
                bExists = TRUE;
        }
    }

    return aURL;
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetName( const String& rNewName )
{
    if( aName != rNewName )
    {
        aName = rNewName;
        SetModified( TRUE );
        bThemeNameFromResource = FALSE;
    }
}

// -----------------------------------------------------------------------------

void GalleryThemeEntry::SetId( UINT32 nNewId, BOOL bResetThemeName )
{
    nId = nNewId;
    SetModified( TRUE );
    bThemeNameFromResource = ( nId && bResetThemeName );
}

// ---------------------------
// - GalleryImportThemeEntry -
// ---------------------------

SvStream& operator<<( SvStream& rOut, const GalleryImportThemeEntry& rEntry )
{
    ByteString aDummy;

    rOut << ByteString( rEntry.aThemeName, RTL_TEXTENCODING_UTF8 ) <<
            ByteString( rEntry.aUIName, RTL_TEXTENCODING_UTF8 ) <<
            ByteString( rEntry.aURL.GetMainURL(), RTL_TEXTENCODING_UTF8 ) <<
            ByteString( rEntry.aImportName, RTL_TEXTENCODING_UTF8 ) <<
            aDummy;

    return rOut;
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, GalleryImportThemeEntry& rEntry )
{
    ByteString aTmpStr;

    rIn >> aTmpStr; rEntry.aThemeName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
    rIn >> aTmpStr; rEntry.aUIName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
    rIn >> aTmpStr; rEntry.aURL = INetURLObject( String( aTmpStr, RTL_TEXTENCODING_UTF8 ) );
    rIn >> aTmpStr; rEntry.aImportName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
    rIn >> aTmpStr;

    return rIn;
}

// --------------------------
// - GalleryThemeCacheEntry -
// --------------------------

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

// -----------
// - Statics -
// -----------

List Gallery::aGalleryCache;

// -----------
// - Gallery -
// -----------

Gallery::Gallery( const String& rMultiPath ) :
        bMultiPath          ( FALSE ),
        nReadTextEncoding   ( gsl_getSystemTextEncoding() ),
        nLastFileNumber     ( 0 )
{
    ImplLoad( rMultiPath );
}

// ------------------------------------------------------------------------

Gallery::~Gallery()
{
    // Themen-Liste loeschen
    for( GalleryThemeEntry* pThemeEntry = aThemeList.First(); pThemeEntry; pThemeEntry = aThemeList.Next() )
        delete pThemeEntry;

    // Import-Liste loeschen
    for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
        delete pImportEntry;
}

// ------------------------------------------------------------------------

Gallery* Gallery::AcquireGallery( const String& rMultiPath )
{
    Gallery*                pGallery = NULL;
    GalleryCacheEntry*      pEntry;
    GalleryCacheEntry*      pFound;

    for( pEntry = (GalleryCacheEntry*) aGalleryCache.First(); pEntry && !pGallery; pEntry = (GalleryCacheEntry*) aGalleryCache.Next() )
        if( rMultiPath == pEntry->GetMultiPath() )
            pGallery = ( pFound = pEntry )->GetGallery();

    if( !pGallery )
        aGalleryCache.Insert( pFound = new GalleryCacheEntry( new Gallery( rMultiPath ), rMultiPath ), LIST_APPEND );

    pFound->IncRefCount();

    return( pFound->GetGallery() );
}

// ------------------------------------------------------------------------

void Gallery::ReleaseGallery( Gallery* pGallery )
{
    GalleryCacheEntry*      pFound = NULL;
    GalleryCacheEntry*      pEntry;

    for( pEntry = (GalleryCacheEntry*) aGalleryCache.First(); pEntry && !pFound; pEntry = (GalleryCacheEntry*) aGalleryCache.Next() )
        if( pGallery == pEntry->GetGallery() )
            pFound = pEntry;

    DBG_ASSERT( pFound, "Gallery::ReleaseGallery(...): Gallery entry not found" );
    pFound->DecRefCount();

    if( !pFound->GetRefCount() )
        delete (GalleryCacheEntry*) aGalleryCache.Remove( pFound );
}

// ------------------------------------------------------------------------

void Gallery::ImplLoad( const String& rMultiPath )
{
    const USHORT nTokenCount = rMultiPath.GetTokenCount( ';' );

    bMultiPath = ( nTokenCount > 0 );
    aUserURL = SvtPathOptions().GetConfigPath();
    ImplLoadSubDirs( aUserURL, bMultiPath );

    if( bMultiPath )
    {
        aRelURL = INetURLObject( rMultiPath.GetToken( 0, ';' ) );
        aUserURL = INetURLObject( rMultiPath.GetToken( nTokenCount - 1, ';' ) );

        for( USHORT i = 0UL; i < nTokenCount; i++ )
            ImplLoadSubDirs( INetURLObject( rMultiPath.GetToken( i, ';' ) ), i < ( nTokenCount - 1 ) );
    }
    else
        aRelURL = INetURLObject( rMultiPath );

    DBG_ASSERT( aUserURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
    DBG_ASSERT( aRelURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    ImplLoadImports();
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadSubDirs( const INetURLObject& rBaseURL, BOOL bReadOnly )
{
    try
    {
        uno::Reference< XCommandEnvironment > xEnv;
        Content                               aCnt( rBaseURL.GetMainURL(), xEnv );

        uno::Sequence< OUString > aProps( 1 );
        aProps.getArray()[ 0 ] == OUString::createFromAscii( "Url" );

        uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, INCLUDE_DOCUMENTS_ONLY ) );

        if( xResultSet.is() )
        {
            uno::Reference< XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            if( xContentAccess.is() )
            {
                while( xResultSet->next() )
                {
                    INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

                    if( aThmURL.GetExtension().CompareIgnoreCaseToAscii( "thm" ) == COMPARE_EQUAL )
                    {
                        INetURLObject   aSdgURL( aThmURL); aSdgURL.SetExtension( OUString::createFromAscii( "sdg" ) );
                        INetURLObject   aSdvURL( aThmURL ); aSdvURL.SetExtension( OUString::createFromAscii( "sdv" ) );
                        const OUString  aTitleProp( OUString::createFromAscii( "Title" ) );
                        const OUString  aReadOnlyProp( OUString::createFromAscii( "IsReadOnly" ) );
                        OUString        aTitle;
                        sal_Bool        bReadOnly = sal_False;

                        Content aThmCnt( aThmURL.GetMainURL(), xEnv );
                        Content aSdgCnt( aSdgURL.GetMainURL(), xEnv );
                        Content aSdvCnt( aSdvURL.GetMainURL(), xEnv );

                        aThmCnt.getPropertyValue( aTitleProp ) >>= aTitle;

                        if( aTitle.getLength() )
                        {
                            aThmCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;

                            if( !bReadOnly )
                            {
                                aSdgCnt.getPropertyValue( aTitleProp ) >>= aTitle;

                                if( aTitle.getLength() )
                                    aSdgCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;
                            }

                            if( !bReadOnly )
                            {
                                aSdvCnt.getPropertyValue( aTitleProp ) >>= aTitle;

                                if( aTitle.getLength() )
                                    aSdvCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;
                            }

                            GalleryThemeEntry* pEntry = GalleryTheme::CreateThemeEntry( aThmURL, bReadOnly );

                            if( pEntry )
                            {
                                const ULONG nFileNumber = (ULONG) aThmURL.GetBase().Erase( 0, 2 ).Erase( 6 ).ToInt32();

                                aThemeList.Insert( pEntry, LIST_APPEND );

                                if( nFileNumber > nLastFileNumber )
                                    nLastFileNumber = nFileNumber;
                            }
                        }
                    }
                }
            }
        }
    }
    catch( ... )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadImports()
{
    INetURLObject aURL( GetUserURL() );

    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "gallery.sdi" ) ) );

    if( FileExists( aURL ) )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_READ );

        if( pIStm )
        {
            GalleryThemeEntry*          pThemeEntry;
            GalleryImportThemeEntry*    pImportEntry;
            INetURLObject               aFile;
            UINT32                      nInventor;
            UINT32                      nCount;
            UINT16                      nId;
            UINT16                      i;
            UINT16                      nTempCharSet;

            for( pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
                delete pImportEntry;

            aImportList.Clear();
            *pIStm >> nInventor;

            if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
            {
                *pIStm >> nId >> nCount >> nTempCharSet;

                for( i = 0; i < nCount; i++ )
                {
                    pImportEntry = new GalleryImportThemeEntry;

                    *pIStm >> *pImportEntry;
                    aImportList.Insert( pImportEntry, LIST_APPEND );
                    aFile = INetURLObject( pImportEntry->aURL );
                    pThemeEntry = new GalleryThemeEntry( aFile,
                                                         pImportEntry->aUIName,
                                                         aFile.GetBase().Erase( 0, 2 ).Erase( 6 ).ToInt32(),
                                                         TRUE, TRUE, FALSE, 0, FALSE );

                    aThemeList.Insert( pThemeEntry, LIST_APPEND );
                }
            }

            delete pIStm;
        }
    }
}

// ------------------------------------------------------------------------

void Gallery::ImplWriteImportList()
{
    INetURLObject aURL( GetUserURL() );
    aURL.Append( ( String( "gallery.sdi", RTL_TEXTENCODING_UTF8 ) ) );
    SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC );

    if( pOStm )
    {
        const UINT32 nInventor = (UINT32) COMPAT_FORMAT( 'S', 'G', 'A', '3' );
        const UINT16 nId = 0x0004;

        *pOStm << nInventor << nId << (UINT32) aImportList.Count() << (UINT16) gsl_getSystemTextEncoding();

        for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry; pImportEntry = aImportList.Next() )
            *pOStm << *pImportEntry;

        if( pOStm->GetError() )
            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );

        delete pOStm;
    }
}

// ------------------------------------------------------------------------

GalleryThemeEntry* Gallery::ImplGetThemeEntry( const String& rThemeName )
{
    GalleryThemeEntry* pFound = NULL;

    if( rThemeName.Len() )
        for( GalleryThemeEntry* pEntry = aThemeList.First(); pEntry && !pFound; pEntry = aThemeList.Next() )
            if( rThemeName == pEntry->GetThemeName() )
                pFound = pEntry;

    return pFound;
}

// ------------------------------------------------------------------------

GalleryImportThemeEntry* Gallery::ImplGetImportThemeEntry( const String& rImportName )
{
    GalleryImportThemeEntry* pFound = NULL;

    for( GalleryImportThemeEntry* pImportEntry = aImportList.First(); pImportEntry && !pFound; pImportEntry = aImportList.Next() )
        if ( rImportName == pImportEntry->aUIName )
            pFound = pImportEntry;

    return pFound;
}

// ------------------------------------------------------------------------

String Gallery::GetThemeName( ULONG nThemeId ) const
{
    GalleryThemeEntry* pFound = NULL;

    for( ULONG n = 0, nCount = aThemeList.Count(); n < nCount; n++ )
    {
        GalleryThemeEntry* pEntry = aThemeList.GetObject( n );

        if( nThemeId == pEntry->GetId() )
            pFound = pEntry;
    }

    // try fallback, if no entry was found
    if( !pFound )
    {
        ByteString      aFallback;

        switch( nThemeId )
        {
            case( GALLERY_THEME_3D ): aFallback = "3D"; break;
            case( GALLERY_THEME_BULLETS ): aFallback = "Bullets"; break;
            case( GALLERY_THEME_HOMEPAGE ): aFallback = "Homepage"; break;
            case( GALLERY_THEME_HTMLBUTTONS ): aFallback = "private://gallery/hidden/HtmlExportButtons"; break;
            case( GALLERY_THEME_POWERPOINT ): aFallback = "private://gallery/hidden/imgppt"; break;
            case( GALLERY_THEME_RULERS ): aFallback = "Rulers"; break;
            case( GALLERY_THEME_SOUNDS ): aFallback = "Sounds"; break;

            default:
            break;
        }

        pFound = ( (Gallery*) this )->ImplGetThemeEntry( String::CreateFromAscii( aFallback.GetBuffer() ) );
    }

    return( pFound ? pFound->GetThemeName() : String() );
}

// ------------------------------------------------------------------------

BOOL Gallery::HasTheme( const String& rThemeName )
{
    return( ImplGetThemeEntry( rThemeName ) != NULL );
}

// ------------------------------------------------------------------------

BOOL Gallery::CreateTheme( const String& rThemeName )
{
    BOOL bRet = FALSE;

    if( !HasTheme( rThemeName ) )
    {
        GalleryThemeEntry* pNewEntry = new GalleryThemeEntry( GetUserURL(), rThemeName,
                                                              ++nLastFileNumber,
                                                              FALSE, FALSE, TRUE, 0, FALSE );

        aThemeList.Insert( pNewEntry, LIST_APPEND );
        delete( new GalleryTheme( this, pNewEntry ) );
        Broadcast( GalleryHint( GALLERY_HINT_THEME_CREATED, rThemeName ) );
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Gallery::CreateImportTheme( const INetURLObject& rURL, const String& rImportName )
{
    INetURLObject   aURL( rURL );
    BOOL            bRet = FALSE;

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( FileExists( aURL ) )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_READ );

        if( pIStm )
        {
            ULONG   nStmErr;
            UINT16  nId;

            *pIStm >> nId;

            if( nId > 0x0004 )
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
            else
            {
                ByteString              aTmpStr;
                String                  aThemeName; *pIStm >> aTmpStr; aThemeName = String( aTmpStr, RTL_TEXTENCODING_UTF8 );
                GalleryThemeEntry*      pThemeEntry = new GalleryThemeEntry( aURL, rImportName,
                                                                             aURL.GetBase().Erase( 0, 2 ).Erase( 6 ).ToInt32(),
                                                                             TRUE, TRUE, TRUE, 0, FALSE );
                GalleryTheme*           pImportTheme = new GalleryTheme( this, pThemeEntry );

                pIStm->Seek( STREAM_SEEK_TO_BEGIN );
                *pIStm >> *pImportTheme;
                nStmErr = pIStm->GetError();

                if( nStmErr )
                {
                    delete pThemeEntry;
                    ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
                }
                else
                {
                    String  aName( rImportName );
                    String  aNewName( aName );
                    ULONG   nCount = 0;

                    aName += ' ';

                    while ( HasTheme( aNewName ) && ( nCount++ < 16000 ) )
                    {
                        aNewName = aName;
                        aNewName += nCount;
                    }

                    pImportTheme->SetImportName( aNewName );
                    aThemeList.Insert( pThemeEntry, LIST_APPEND );

                    // Thema in Import-Liste eintragen und Import-Liste     speichern
                    GalleryImportThemeEntry* pImportEntry = new GalleryImportThemeEntry;
                    pImportEntry->aThemeName = pImportEntry->aUIName = aNewName;
                    pImportEntry->aURL = rURL;
                    pImportEntry->aImportName = rImportName;
                    aImportList.Insert( pImportEntry, LIST_APPEND );
                    ImplWriteImportList();
                    bRet = TRUE;
                }

                delete pImportTheme;
            }

            delete pIStm;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Gallery::RenameTheme( const String& rOldName, const String& rNewName )
{
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rOldName );
    BOOL                            bRet = FALSE;

    // Ueberpruefen, ob neuer Themenname schon vorhanden ist
    if( pThemeEntry && !HasTheme( rNewName ) && ( !pThemeEntry->IsReadOnly() || pThemeEntry->IsImported() ) )
    {
        SfxListener             aDummyListener;
        GalleryTheme*   pThm = AcquireTheme( rOldName, aDummyListener );

        if( pThm )
        {
            const String aOldName( rOldName );

            pThemeEntry->SetName( rNewName );
            pThm->ImplWrite();

            if( pThemeEntry->IsImported() )
            {
                pThm->SetImportName( rNewName );

                GalleryImportThemeEntry* pImportEntry = ImplGetImportThemeEntry( rOldName );

                if( pImportEntry )
                {
                    pImportEntry->aUIName = rNewName;
                    ImplWriteImportList();
                }
            }

            Broadcast( GalleryHint( GALLERY_HINT_THEME_RENAMED, aOldName, pThm->GetName() ) );
            ReleaseTheme( pThm, aDummyListener );
            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL Gallery::RemoveTheme( const String& rThemeName )
{
    GalleryThemeEntry*  pThemeEntry = ImplGetThemeEntry( rThemeName );
    BOOL                bRet = FALSE;

    if( pThemeEntry && ( !pThemeEntry->IsReadOnly() || pThemeEntry->IsImported() ) )
    {
        Broadcast( GalleryHint( GALLERY_HINT_CLOSE_THEME, rThemeName ) );

        if( pThemeEntry->IsImported() )
        {
            GalleryImportThemeEntry* pImportEntry = ImplGetImportThemeEntry( rThemeName );

            if( pImportEntry )
            {
                delete aImportList.Remove( pImportEntry );
                ImplWriteImportList();
            }
        }
        else
        {
            SfxListener     aDummyListener;
            GalleryTheme*   pThm = AcquireTheme( rThemeName, aDummyListener );
            INetURLObject   aThmURL( pThm->GetThmURL() );
            INetURLObject   aSdgURL( pThm->GetSdgURL() );
            INetURLObject   aSdvURL( pThm->GetSdvURL() );

            ReleaseTheme( pThm, aDummyListener );

            KillFile( aThmURL );
            KillFile( aSdgURL );
            KillFile( aSdvURL );
        }

        delete aThemeList.Remove( pThemeEntry );
        Broadcast( GalleryHint( GALLERY_HINT_THEME_REMOVED, rThemeName ) );

        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

INetURLObject Gallery::GetImportURL( const String& rThemeName )
{
    INetURLObject               aURL;
    GalleryImportThemeEntry*    pImportEntry = ImplGetImportThemeEntry( rThemeName );

    if( pImportEntry )
    {
        aURL = pImportEntry->aURL;
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
    }

    return aURL;
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::ImplGetCachedTheme( const GalleryThemeEntry* pThemeEntry )
{
    GalleryTheme* pTheme = NULL;

    if( pThemeEntry )
    {
        GalleryThemeCacheEntry* pEntry;

        for( pEntry = (GalleryThemeCacheEntry*) aThemeCache.First(); pEntry && !pTheme; pEntry = (GalleryThemeCacheEntry*) aThemeCache.Next() )
            if( pThemeEntry == pEntry->GetThemeEntry() )
                pTheme = pEntry->GetTheme();

        if( !pTheme )
        {
            INetURLObject aURL;

            if( !pThemeEntry->IsImported() )
                aURL = pThemeEntry->GetThmURL();
            else
                aURL = GetImportURL( pThemeEntry->GetThemeName() );

            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( FileExists( aURL ) )
            {
                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_READ );

                if( pIStm )
                {
                    pTheme = new GalleryTheme( this, (GalleryThemeEntry*) pThemeEntry );
                    *pIStm >> *pTheme;

                    if( pIStm->GetError() )
                        delete pTheme, pTheme = NULL;
                    else if( pThemeEntry->IsImported() )
                        pTheme->SetImportName( pThemeEntry->GetThemeName() );

                    delete pIStm;
                }
            }

            if( pTheme )
                aThemeCache.Insert( new GalleryThemeCacheEntry( pThemeEntry, pTheme ), LIST_APPEND );
        }
    }

    return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ImplDeleteCachedTheme( GalleryTheme* pTheme )
{
    GalleryThemeCacheEntry* pEntry;
    BOOL                                    bDone = FALSE;

    for( pEntry = (GalleryThemeCacheEntry*) aThemeCache.First(); pEntry && !bDone; pEntry = (GalleryThemeCacheEntry*) aThemeCache.Next() )
    {
        if( pTheme == pEntry->GetTheme() )
        {
            delete (GalleryThemeCacheEntry*) aThemeCache.Remove( pEntry );
            bDone = TRUE;
        }
    }
}

// ------------------------------------------------------------------------

GalleryTheme* Gallery::AcquireTheme( const String& rThemeName, SfxListener& rListener )
{
    GalleryTheme*           pTheme = NULL;
    GalleryThemeEntry*      pThemeEntry = ImplGetThemeEntry( rThemeName );

    if( pThemeEntry && ( ( pTheme = ImplGetCachedTheme( pThemeEntry ) ) != NULL ) )
        rListener.StartListening( *pTheme );

    return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ReleaseTheme( GalleryTheme* pTheme, SfxListener& rListener )
{
    if( pTheme )
    {
        rListener.EndListening( *pTheme );

        if( !pTheme->HasListeners() )
            ImplDeleteCachedTheme( pTheme );
    }
}
