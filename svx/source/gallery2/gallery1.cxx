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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <tools/vcompat.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/docfile.hxx>
#include "gallery.hxx"
#include "gallery.hrc"
#include "svx/galmisc.hxx"
#include "galtheme.hxx"
#include "svx/gallery1.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>

#define ENABLE_BYTESTRING_STREAM_OPERATORS

// --------------
// - Namespaces -
// --------------

using namespace ::rtl;
using namespace ::com::sun::star;

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
            ByteString( String(rEntry.aURL.GetMainURL( INetURLObject::NO_DECODE )), RTL_TEXTENCODING_UTF8 ) <<
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
// - Gallery -
// -----------

Gallery::Gallery( const String& rMultiPath )
:       nReadTextEncoding   ( gsl_getSystemTextEncoding() )
,       nLastFileNumber     ( 0 )
,       bMultiPath          ( FALSE )
{
    ImplLoad( rMultiPath );
}

// ------------------------------------------------------------------------

Gallery::~Gallery()
{
    // Themen-Liste loeschen
    for ( size_t i = 0, n = aThemeList.size(); i < n; ++i )
        delete aThemeList[ i ];
    aThemeList.clear();

    // Import-Liste loeschen
    for ( size_t i = 0, n = aImportList.size(); i < n; ++i )
        delete aImportList[ i ];
    aImportList.clear();
}

// ------------------------------------------------------------------------

Gallery* Gallery::GetGalleryInstance()
{
    static Gallery* pGallery = NULL;

    if( !pGallery )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pGallery )
        {
            pGallery = new Gallery( SvtPathOptions().GetGalleryPath() );
        }
    }

    return pGallery;
}

// ------------------------------------------------------------------------

void Gallery::ImplLoad( const String& rMultiPath )
{
    const USHORT    nTokenCount = rMultiPath.GetTokenCount( ';' );
    sal_Bool        bIsReadOnlyDir;

    bMultiPath = ( nTokenCount > 0 );

    INetURLObject aCurURL(SvtPathOptions().GetConfigPath());
    ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

    if( !bIsReadOnlyDir )
        aUserURL = aCurURL;

    if( bMultiPath )
    {
        aRelURL = INetURLObject( rMultiPath.GetToken( 0, ';' ) );

        for( USHORT i = 0UL; i < nTokenCount; i++ )
        {
            aCurURL = INetURLObject(rMultiPath.GetToken( i, ';' ));

            ImplLoadSubDirs( aCurURL, bIsReadOnlyDir );

            if( !bIsReadOnlyDir )
                aUserURL = aCurURL;
        }
    }
    else
        aRelURL = INetURLObject( rMultiPath );

    DBG_ASSERT( aUserURL.GetProtocol() != INET_PROT_NOT_VALID, "no writable Gallery user directory available" );
    DBG_ASSERT( aRelURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    ImplLoadImports();
}

// ------------------------------------------------------------------------

void Gallery::ImplLoadSubDirs( const INetURLObject& rBaseURL, sal_Bool& rbDirIsReadOnly )
{
    rbDirIsReadOnly = sal_False;

    try
    {
        uno::Reference< ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content                       aCnt( rBaseURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );

        uno::Sequence< OUString > aProps( 1 );
        aProps.getArray()[ 0 ] = OUString(RTL_CONSTASCII_USTRINGPARAM("Url"));

        uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

        try
        {
            // check readonlyness the very hard way
            INetURLObject   aTestURL( rBaseURL );
            String          aTestFile( RTL_CONSTASCII_USTRINGPARAM( "cdefghij.klm" ) );

            aTestURL.Append( aTestFile );
            SvStream* pTestStm = ::utl::UcbStreamHelper::CreateStream( aTestURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );

            if( pTestStm )
            {
                *pTestStm << 1;

                if( pTestStm->GetError() )
                    rbDirIsReadOnly = sal_True;

                delete pTestStm;
                KillFile( aTestURL );
            }
            else
                rbDirIsReadOnly = sal_True;
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

        if( xResultSet.is() )
        {
            uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

            if( xContentAccess.is() )
            {
                while( xResultSet->next() )
                {
                    INetURLObject aThmURL( xContentAccess->queryContentIdentifierString() );

                    if(aThmURL.GetExtension().equalsIgnoreAsciiCaseAscii("thm"))
                    {
                        INetURLObject   aSdgURL( aThmURL); aSdgURL.SetExtension( OUString(RTL_CONSTASCII_USTRINGPARAM("sdg")) );
                        INetURLObject   aSdvURL( aThmURL ); aSdvURL.SetExtension( OUString(RTL_CONSTASCII_USTRINGPARAM("sdv")) );
                        const OUString aTitleProp( RTL_CONSTASCII_USTRINGPARAM("Title") );
                        const OUString aReadOnlyProp( RTL_CONSTASCII_USTRINGPARAM("IsReadOnly") );
                        OUString        aTitle;
                        sal_Bool        bReadOnly = sal_False;

                        try
                        {
                            ::ucbhelper::Content aThmCnt( aThmURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );
                            ::ucbhelper::Content aSdgCnt( aSdgURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );
                            ::ucbhelper::Content aSdvCnt( aSdvURL.GetMainURL( INetURLObject::NO_DECODE ), xEnv );

                            try
                            {
                                aThmCnt.getPropertyValue( aTitleProp ) >>= aTitle;
                            }
                            catch( const uno::RuntimeException& )
                            {
                            }
                            catch( const uno::Exception& )
                            {
                            }

                            if( aTitle.getLength() )
                            {
                                try
                                {
                                    aThmCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;
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
                                        aSdgCnt.getPropertyValue( aTitleProp ) >>= aTitle;
                                    }
                                    catch( const ::com::sun::star::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const ::com::sun::star::uno::Exception& )
                                    {
                                    }

                                    if( aTitle.getLength() )
                                    {
                                        try
                                        {
                                            aSdgCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;
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
                                        aSdvCnt.getPropertyValue( aTitleProp ) >>= aTitle;
                                    }
                                    catch( const ::com::sun::star::uno::RuntimeException& )
                                    {
                                    }
                                    catch( const ::com::sun::star::uno::Exception& )
                                    {
                                    }

                                    if( aTitle.getLength() )
                                    {
                                        try
                                        {
                                            aSdvCnt.getPropertyValue( aReadOnlyProp ) >>= bReadOnly;
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
                                {
                                    const ULONG nFileNumber = (ULONG) String(aThmURL.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32();

                                    aThemeList.push_back( pEntry );

                                    if( nFileNumber > nLastFileNumber )
                                        nLastFileNumber = nFileNumber;
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

// ------------------------------------------------------------------------

void Gallery::ImplLoadImports()
{
    INetURLObject aURL( GetUserURL() );

    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "gallery.sdi" ) ) );

    if( FileExists( aURL ) )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

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

            for ( size_t j = 0, n = aImportList.size(); j < n; ++j )
                delete aImportList[ j ];
            aImportList.clear();

            *pIStm >> nInventor;

            if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
            {
                *pIStm >> nId >> nCount >> nTempCharSet;

                for( i = 0; i < nCount; i++ )
                {
                    pImportEntry = new GalleryImportThemeEntry;

                    *pIStm >> *pImportEntry;
                    aImportList.push_back( pImportEntry );
                    aFile = INetURLObject( pImportEntry->aURL );
                    pThemeEntry = new GalleryThemeEntry( aFile,
                                                         pImportEntry->aUIName,
                                                         String(aFile.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32(),
                                                         TRUE, TRUE, FALSE, 0, FALSE );

                    aThemeList.push_back( pThemeEntry );
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
    SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

    if( pOStm )
    {
        const UINT32 nInventor = (UINT32) COMPAT_FORMAT( 'S', 'G', 'A', '3' );
        const UINT16 nId = 0x0004;

        *pOStm << nInventor << nId << (UINT32) aImportList.size() << (UINT16) gsl_getSystemTextEncoding();

        for ( size_t i = 0, n = aImportList.size(); i < n; ++i )
            *pOStm << *aImportList[ i ];

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
        for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
            if( rThemeName == aThemeList[ i ]->GetThemeName() )
                pFound = aThemeList[ i ];

    return pFound;
}

// ------------------------------------------------------------------------

GalleryImportThemeEntry* Gallery::ImplGetImportThemeEntry( const String& rImportName )
{
    for ( size_t i = 0, n = aImportList.size(); i < n; ++i )
        if ( rImportName == aImportList[ i ]->aUIName )
            return aImportList[ i ];
    return NULL;
}

// ------------------------------------------------------------------------

String Gallery::GetThemeName( ULONG nThemeId ) const
{
    GalleryThemeEntry* pFound = NULL;

    for ( size_t i = 0, n = aThemeList.size(); i < n && !pFound; ++i )
    {
        GalleryThemeEntry* pEntry = aThemeList[ i ];
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
            case( GALLERY_THEME_FONTWORK ): aFallback = "private://gallery/hidden/fontwork"; break;
            case( GALLERY_THEME_FONTWORK_VERTICAL ): aFallback = "private://gallery/hidden/fontworkvertical"; break;
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

BOOL Gallery::CreateTheme( const String& rThemeName, UINT32 nNumFrom )
{
    BOOL bRet = FALSE;

    if( !HasTheme( rThemeName ) && ( GetUserURL().GetProtocol() != INET_PROT_NOT_VALID ) )
    {
        nLastFileNumber = nNumFrom > nLastFileNumber ? nNumFrom : nLastFileNumber + 1;
        GalleryThemeEntry* pNewEntry = new GalleryThemeEntry( GetUserURL(), rThemeName,
                                                              nLastFileNumber,
                                                              FALSE, FALSE, TRUE, 0, FALSE );

        aThemeList.push_back( pNewEntry );
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
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

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
                                                                             String(aURL.GetBase()).Erase( 0, 2 ).Erase( 6 ).ToInt32(),
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
                        aNewName += String::CreateFromInt32( nCount );
                    }

                    pImportTheme->SetImportName( aNewName );
                    aThemeList.push_back( pThemeEntry );

                    // Thema in Import-Liste eintragen und Import-Liste     speichern
                    GalleryImportThemeEntry* pImportEntry = new GalleryImportThemeEntry;
                    pImportEntry->aThemeName = pImportEntry->aUIName = aNewName;
                    pImportEntry->aURL = rURL;
                    pImportEntry->aImportName = rImportName;
                    aImportList.push_back( pImportEntry );
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
        SfxListener   aListener;
        GalleryTheme* pThm = AcquireTheme( rOldName, aListener );

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
            ReleaseTheme( pThm, aListener );
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
                for ( GalleryImportThemeList::iterator it = aImportList.begin(); it < aImportList.end(); ++it )
                {
                    if ( *it == pImportEntry )
                    {
                        delete pImportEntry;
                        aImportList.erase( it );
                        break;
                    }
                }
            }
        }
        else
        {
            SfxListener     aListener;
            GalleryTheme*   pThm = AcquireTheme( rThemeName, aListener );

            if( pThm )
            {
                INetURLObject   aThmURL( pThm->GetThmURL() );
                INetURLObject   aSdgURL( pThm->GetSdgURL() );
                INetURLObject   aSdvURL( pThm->GetSdvURL() );

                ReleaseTheme( pThm, aListener );

                KillFile( aThmURL );
                KillFile( aSdgURL );
                KillFile( aSdvURL );
            }
        }

        for ( GalleryThemeList::iterator it = aThemeList.begin(); it < aThemeList.end(); ++it )
        {
            if ( pThemeEntry == *it ) {
                delete pThemeEntry;
                aThemeList.erase( it );
                break;
            }
        }

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
            INetURLObject aURL;

            if( !pThemeEntry->IsImported() )
                aURL = pThemeEntry->GetThmURL();
            else
                aURL = GetImportURL( pThemeEntry->GetThemeName() );

            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( FileExists( aURL ) )
            {
                SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

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
                aThemeCache.push_back( new GalleryThemeCacheEntry( pThemeEntry, pTheme ));
        }
    }

    return pTheme;
}

// ------------------------------------------------------------------------

void Gallery::ImplDeleteCachedTheme( GalleryTheme* pTheme )
{
    for (GalleryCacheThemeList::iterator it = aThemeCache.begin(); it != aThemeCache.end(); ++it)
    {
        if (pTheme == (*it)->GetTheme())
        {
            delete *it;
            aThemeCache.erase(it);
            break;
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

BOOL GalleryThemeEntry::IsDefault() const
{ return( ( nId > 0 ) && ( nId != ( RID_GALLERYSTR_THEME_MYTHEME - RID_GALLERYSTR_THEME_START ) ) ); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
