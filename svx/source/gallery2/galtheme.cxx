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

#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <svl/itempool.hxx>
#include <sfx2/docfile.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/svdograf.hxx>
#include <svx/fmpage.hxx>
#include "codec.hxx"
#include <svx/unomodel.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include "svx/galmisc.hxx"
#include "galtheme.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include "galobj.hxx"
#include <svx/gallery1.hxx>
#include "galtheme.hrc"
#include <vcl/lstbox.hxx>
#include "gallerydrawmodel.hxx"

// --------------
// - Namespaces -
// --------------

using namespace ::rtl;
using namespace ::com::sun::star;

// ------------
// - SgaTheme -
// ------------

GalleryTheme::GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry ) :
        pParent               ( pGallery ),
        pThm                  ( pThemeEntry ),
        mnThemeLockCount      ( 0 ),
        mnBroadcasterLockCount( 0 ),
        nDragPos              ( 0 ),
        bDragging             ( FALSE )
{
    ImplCreateSvDrawStorage();

    if( pThm->IsImported() )
        aImportName = pThm->GetThemeName();
}

// ------------------------------------------------------------------------

GalleryTheme::~GalleryTheme()
{
    ImplWrite();

    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
    {
        GalleryObject* pEntry = aObjectList[ i ];
        Broadcast( GalleryHint( GALLERY_HINT_CLOSE_OBJECT, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );
        delete pEntry;
        Broadcast( GalleryHint( GALLERY_HINT_OBJECT_REMOVED, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );
    }
    aObjectList.clear();
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplCreateSvDrawStorage()
{
    if( !pThm->IsImported() )
    {
        aSvDrawStorageRef = new SvStorage( FALSE, GetSdvURL().GetMainURL( INetURLObject::NO_DECODE ), pThm->IsReadOnly() ? STREAM_READ : STREAM_STD_READWRITE );
        // #i50423# ReadOnly may not been set though the file can't be written (because of security reasons)
        if ( ( aSvDrawStorageRef->GetError() != ERRCODE_NONE ) && !pThm->IsReadOnly() )
            aSvDrawStorageRef = new SvStorage( FALSE, GetSdvURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
    }
    else
        aSvDrawStorageRef.Clear();
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ImplWriteSgaObject( const SgaObject& rObj, size_t nPos, GalleryObject* pExistentEntry )
{
    SvStream*   pOStm = ::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );
    BOOL        bRet = FALSE;

    if( pOStm )
    {
        const sal_uInt32 nOffset = pOStm->Seek( STREAM_SEEK_TO_END );

        rObj.WriteData( *pOStm, m_aDestDir );

        if( !pOStm->GetError() )
        {
            GalleryObject* pEntry;

            if( !pExistentEntry )
            {
                pEntry = new GalleryObject;
                if ( nPos < aObjectList.size() )
                {
                    GalleryObjectList::iterator it = aObjectList.begin();
                    ::std::advance( it, nPos );
                    aObjectList.insert( it, pEntry );
                }
                else
                    aObjectList.push_back( pEntry );
            }
            else
                pEntry = pExistentEntry;

            pEntry->aURL = rObj.GetURL();
            pEntry->nOffset = nOffset;
            pEntry->eObjKind = rObj.GetObjKind();
            bRet = TRUE;
        }

        delete pOStm;
    }

    return bRet;
}

// ------------------------------------------------------------------------

SgaObject* GalleryTheme::ImplReadSgaObject( GalleryObject* pEntry )
{
    SgaObject* pSgaObj = NULL;

    if( pEntry )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

        if( pIStm )
        {
            sal_uInt32 nInventor;

            // Ueberpruefen, ob das File ein gueltiges SGA-File ist
            pIStm->Seek( pEntry->nOffset );
            *pIStm >> nInventor;

            if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
            {
                pIStm->Seek( pEntry->nOffset );

                switch( pEntry->eObjKind )
                {
                    case( SGA_OBJ_BMP ):    pSgaObj = new SgaObjectBmp(); break;
                    case( SGA_OBJ_ANIM ):   pSgaObj = new SgaObjectAnim(); break;
                    case( SGA_OBJ_INET ):   pSgaObj = new SgaObjectINet(); break;
                    case( SGA_OBJ_SVDRAW ): pSgaObj = new SgaObjectSvDraw(); break;
                    case( SGA_OBJ_SOUND ):  pSgaObj = new SgaObjectSound(); break;

                    default:
                    break;
                }

                if( pSgaObj )
                {
                    *pIStm >> *pSgaObj;
                    pSgaObj->ImplUpdateURL( pEntry->aURL );
                }
            }

            delete pIStm;
        }
    }

    return pSgaObj;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplRead()
{
    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

    if( pIStm )
    {
        *pIStm >> *this;
        delete pIStm;
    }
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplWrite()
{
    if( IsModified() )
    {
        INetURLObject aPathURL( GetThmURL() );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        DBG_ASSERT( aPathURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

        if( FileExists( aPathURL ) || CreateDir( aPathURL ) )
        {
#ifdef UNX
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_COPY_ON_SYMLINK | STREAM_TRUNC );
#else
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );
#endif

            if( pOStm )
            {
                *pOStm << *this;
                delete pOStm;
            }

            ImplSetModified( FALSE );
        }
    }
}

// ------------------------------------------------------------------------

const GalleryObject* GalleryTheme::ImplGetGalleryObject( const INetURLObject& rURL )
{
    for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
        if ( aObjectList[ i ]->aURL == rURL )
            return aObjectList[ i ];
    return NULL;
}

// ------------------------------------------------------------------------

INetURLObject GalleryTheme::ImplGetURL( const GalleryObject* pObject ) const
{
    INetURLObject aURL;

    if( pObject )
    {
        if( IsImported() )
        {
            INetURLObject aPathURL( GetParent()->GetImportURL( GetName() ) );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();
            aPathURL.Append( pObject->aURL.GetName() );
            aURL = aPathURL;
        }
        else
            aURL = pObject->aURL;
    }

    return aURL;
}

// ------------------------------------------------------------------------

INetURLObject GalleryTheme::ImplCreateUniqueURL( SgaObjKind eObjKind, ULONG nFormat )
{
    INetURLObject   aDir( GetParent()->GetUserURL() );
    INetURLObject   aInfoFileURL( GetParent()->GetUserURL() );
    INetURLObject   aNewURL;
    sal_uInt32      nNextNumber = 1999;
    sal_Char const* pExt = NULL;
    BOOL            bExists;

    aDir.Append( String( RTL_CONSTASCII_USTRINGPARAM( "dragdrop" ) ) );
    CreateDir( aDir );

    aInfoFileURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( "sdddndx1" ) ) );

    // read next possible number
    if( FileExists( aInfoFileURL ) )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aInfoFileURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

        if( pIStm )
        {
            *pIStm >> nNextNumber;
            delete pIStm;
        }
    }

    // create extension
    if( nFormat )
    {
        switch( nFormat )
        {
            case( CVT_BMP ): pExt = ".bmp"; break;
            case( CVT_GIF ): pExt = ".gif"; break;
            case( CVT_JPG ): pExt = ".jpg"; break;
            case( CVT_MET ): pExt = ".met"; break;
            case( CVT_PCT ): pExt = ".pct"; break;
            case( CVT_PNG ): pExt = ".png"; break;
            case( CVT_SVM ): pExt = ".svm"; break;
            case( CVT_TIF ): pExt = ".tif"; break;
            case( CVT_WMF ): pExt = ".wmf"; break;
            case( CVT_EMF ): pExt = ".emf"; break;

            default:
                pExt = ".grf";
            break;
        }
    }

    do
    {
        // get URL
        if( SGA_OBJ_SVDRAW == eObjKind )
        {
            String aFileName( RTL_CONSTASCII_USTRINGPARAM( "gallery/svdraw/dd" ) );
            aNewURL = INetURLObject( aFileName += String::CreateFromInt32( ++nNextNumber % 99999999 ), INET_PROT_PRIV_SOFFICE );

            bExists = FALSE;

            for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
                if ( aObjectList[ i ]->aURL == aNewURL )
                {
                    bExists = TRUE;
                    break;
                }
        }
        else
        {
            String aFileName( RTL_CONSTASCII_USTRINGPARAM( "dd" ) );

            aFileName += String::CreateFromInt32( ++nNextNumber % 999999 );
            aFileName += String( pExt, RTL_TEXTENCODING_ASCII_US );

            aNewURL = aDir;
            aNewURL.Append( aFileName );

            bExists = FileExists( aNewURL );
        }
    }
    while( bExists );

    // write updated number
    SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aInfoFileURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );

    if( pOStm )
    {
        *pOStm << nNextNumber;
        delete pOStm;
    }

    return aNewURL;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplBroadcast( ULONG nUpdatePos )
{
    if( !IsBroadcasterLocked() )
    {
        if( GetObjectCount() && ( nUpdatePos >= GetObjectCount() ) )
            nUpdatePos = GetObjectCount() - 1;

        Broadcast( GalleryHint( GALLERY_HINT_THEME_UPDATEVIEW, GetName(), nUpdatePos ) );
    }
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::UnlockTheme()
{
    DBG_ASSERT( mnThemeLockCount, "Theme is not locked" );

    BOOL bRet = FALSE;

    if( mnThemeLockCount )
    {
        --mnThemeLockCount;
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GalleryTheme::UnlockBroadcaster( ULONG nUpdatePos )
{
    DBG_ASSERT( mnBroadcasterLockCount, "Broadcaster is not locked" );

    if( mnBroadcasterLockCount && !--mnBroadcasterLockCount )
        ImplBroadcast( nUpdatePos );
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::InsertObject( const SgaObject& rObj, ULONG nInsertPos )
{
    BOOL bRet = FALSE;

    if( rObj.IsValid() )
    {
        GalleryObject*  pFoundEntry = NULL;
        size_t iFoundPos = 0;
        for ( size_t n = aObjectList.size(); iFoundPos < n; ++iFoundPos )
        {
            if ( aObjectList[ iFoundPos ]->aURL == rObj.GetURL() )
            {
                pFoundEntry = aObjectList[ iFoundPos ];
                break;
            }
        }

        if( pFoundEntry )
        {
            GalleryObject aNewEntry;

            // update title of new object if neccessary
            if( !rObj.GetTitle().Len() )
            {
                SgaObject* pOldObj = ImplReadSgaObject( pFoundEntry );

                if( pOldObj )
                {
                    ( (SgaObject&) rObj ).SetTitle( pOldObj->GetTitle() );
                    delete pOldObj;
                }
            }
            else if( rObj.GetTitle() == String( RTL_CONSTASCII_USTRINGPARAM( "__<empty>__" ) ) )
                ( (SgaObject&) rObj ).SetTitle( String() );

            ImplWriteSgaObject( rObj, nInsertPos, &aNewEntry );
            pFoundEntry->nOffset = aNewEntry.nOffset;
        }
        else
            ImplWriteSgaObject( rObj, nInsertPos, NULL );

        ImplSetModified( bRet = TRUE );
        ImplBroadcast( pFoundEntry ? iFoundPos : nInsertPos );
    }

    return bRet;
}

// ------------------------------------------------------------------------

SgaObject* GalleryTheme::AcquireObject( size_t nPos )
{
    return ImplReadSgaObject( aObjectList[ nPos ] );
}

// ------------------------------------------------------------------------

void GalleryTheme::ReleaseObject( SgaObject* pObject )
{
    delete pObject;
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::RemoveObject( size_t nPos )
{
    GalleryObject* pEntry = NULL;
    if ( nPos < aObjectList.size() )
    {
        GalleryObjectList::iterator it = aObjectList.begin();
        ::std::advance( it, nPos );
        pEntry = *it;
        aObjectList.erase( it );
    }

    if( aObjectList.empty() )
        KillFile( GetSdgURL() );

    if( pEntry )
    {
        if( SGA_OBJ_SVDRAW == pEntry->eObjKind )
            aSvDrawStorageRef->Remove( pEntry->aURL.GetMainURL( INetURLObject::NO_DECODE ) );

        Broadcast( GalleryHint( GALLERY_HINT_CLOSE_OBJECT, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );
        delete pEntry;
        Broadcast( GalleryHint( GALLERY_HINT_OBJECT_REMOVED, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );

        ImplSetModified( TRUE );
        ImplBroadcast( nPos );
    }

    return( pEntry != NULL );
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ChangeObjectPos( size_t nOldPos, size_t nNewPos )
{
    BOOL bRet = FALSE;

    if(  nOldPos != nNewPos
      && nOldPos < aObjectList.size()
      )
    {
        GalleryObject* pEntry = aObjectList[ nOldPos ];

        GalleryObjectList::iterator it = aObjectList.begin();
        ::std::advance( it, nNewPos );
        aObjectList.insert( it, pEntry );

        if( nNewPos < nOldPos ) nOldPos++;

        it = aObjectList.begin();
        ::std::advance( it, nOldPos );
        aObjectList.erase( it );

        ImplSetModified( bRet = TRUE );
        ImplBroadcast( ( nNewPos < nOldPos ) ? nNewPos : ( nNewPos - 1 ) );
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GalleryTheme::Actualize( const Link& rActualizeLink, GalleryProgress* pProgress )
{
    if( !IsReadOnly() && !IsImported() )
    {
        Graphic         aGraphic;
        String          aFormat;
        GalleryObject*  pEntry;
        const size_t    nCount = aObjectList.size();

        LockBroadcaster();
        bAbortActualize = FALSE;

        // LoeschFlag zuruecksetzen
        for (size_t i = 0; i < nCount; i++)
            aObjectList[ i ]->bDummy = FALSE;

        for(size_t i = 0; ( i < nCount ) && !bAbortActualize; i++)
        {
            if( pProgress )
                pProgress->Update( i, nCount - 1 );

            pEntry = aObjectList[ i ];

            const INetURLObject aURL( pEntry->aURL );

            rActualizeLink.Call( (void*) &aURL );

            // SvDraw-Objekte werden spaeter aktualisiert
            if( pEntry->eObjKind != SGA_OBJ_SVDRAW )
            {
                // Hier muss noch etwas eingebaut werden,
                // das Files auf den ensprechenden Eintrag matched
                // Grafiken als Grafik-Objekte in die Gallery aufnehmen
                if( pEntry->eObjKind == SGA_OBJ_SOUND )
                {
                    SgaObjectSound aObjSound( aURL );
                    if( !InsertObject( aObjSound ) )
                        pEntry->bDummy = TRUE;
                }
                else
                {
                    aGraphic.Clear();

                    if ( GalleryGraphicImport( aURL, aGraphic, aFormat ) )
                    {
                        SgaObject* pNewObj;

                        if ( SGA_OBJ_INET == pEntry->eObjKind )
                            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, aURL, aFormat );
                        else if ( aGraphic.IsAnimated() )
                            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, aURL, aFormat );
                        else
                            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, aURL, aFormat );

                        if( !InsertObject( *pNewObj ) )
                            pEntry->bDummy = TRUE;

                        delete pNewObj;
                    }
                    else
                        pEntry->bDummy = TRUE; // Loesch-Flag setzen
                }
            }
            else
            {
                if ( aSvDrawStorageRef.Is() )
                {
                    const String        aStmName( GetSvDrawStreamNameFromURL( pEntry->aURL ) );
                    SvStorageStreamRef  pIStm = aSvDrawStorageRef->OpenSotStream( aStmName, STREAM_READ );

                    if( pIStm && !pIStm->GetError() )
                    {
                        pIStm->SetBufferSize( 16384 );

                        SgaObjectSvDraw aNewObj( *pIStm, pEntry->aURL );

                        if( !InsertObject( aNewObj ) )
                            pEntry->bDummy = TRUE;

                        pIStm->SetBufferSize( 0L );
                    }
                }
            }
        }

        // remove all entries with set flag
        for ( size_t i = 0; i < aObjectList.size(); )
        {
            pEntry = aObjectList[ i ];
            if( pEntry->bDummy )
            {
                Broadcast( GalleryHint( GALLERY_HINT_CLOSE_OBJECT, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );
                GalleryObjectList::iterator it = aObjectList.begin();
                ::std::advance( it, i );
                aObjectList.erase( it );
                delete pEntry;
                Broadcast( GalleryHint( GALLERY_HINT_OBJECT_REMOVED, GetName(), reinterpret_cast< ULONG >( pEntry ) ) );
            }
            else ++i;
        }

        // update theme
        ::utl::TempFile aTmp;
        INetURLObject   aInURL( GetSdgURL() );
        INetURLObject   aTmpURL( aTmp.GetURL() );

        DBG_ASSERT( aInURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        DBG_ASSERT( aTmpURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aInURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
        SvStream* pTmpStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

        if( pIStm && pTmpStm )
        {
            for ( size_t i = 0, n = aObjectList.size(); i < n; ++i )
            {
                pEntry = aObjectList[ i ];
                SgaObject* pObj;

                switch( pEntry->eObjKind )
                {
                    case( SGA_OBJ_BMP ):    pObj = new SgaObjectBmp();      break;
                    case( SGA_OBJ_ANIM ):   pObj = new SgaObjectAnim();     break;
                    case( SGA_OBJ_INET ):   pObj = new SgaObjectINet();     break;
                    case( SGA_OBJ_SVDRAW ): pObj = new SgaObjectSvDraw();   break;
                    case (SGA_OBJ_SOUND):   pObj = new SgaObjectSound();    break;

                    default:
                        pObj = NULL;
                    break;
                }

                if( pObj )
                {
                    pIStm->Seek( pEntry->nOffset );
                    *pIStm >> *pObj;
                    pEntry->nOffset = pTmpStm->Tell();
                    *pTmpStm << *pObj;
                    delete pObj;
                }
            }
        }
        else
        {
            OSL_FAIL( "File(s) could not be opened" );
        }

        delete pIStm;
        delete pTmpStm;

        CopyFile( aTmpURL, aInURL );
        KillFile( aTmpURL );

        ULONG nStorErr = 0;

        {
            SvStorageRef aTempStorageRef( new SvStorage( FALSE, aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_STD_READWRITE ) );
            aSvDrawStorageRef->CopyTo( aTempStorageRef );
            nStorErr = aSvDrawStorageRef->GetError();
        }

        if( !nStorErr )
        {
            aSvDrawStorageRef.Clear();
            CopyFile( aTmpURL, GetSdvURL() );
            ImplCreateSvDrawStorage();
        }

        KillFile( aTmpURL );
        ImplSetModified( TRUE );
        ImplWrite();
        UnlockBroadcaster();
    }
}

// ------------------------------------------------------------------------

GalleryThemeEntry* GalleryTheme::CreateThemeEntry( const INetURLObject& rURL, BOOL bReadOnly )
{
    DBG_ASSERT( rURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    GalleryThemeEntry*  pRet = NULL;

    if( FileExists( rURL ) )
    {
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( rURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

        if( pIStm )
        {
            String          aThemeName;
            sal_uInt16      nVersion;
            BOOL            bThemeNameFromResource = FALSE;

            *pIStm >> nVersion;

            if( nVersion <= 0x00ff )
            {
                sal_uInt32      nThemeId = 0;
                ByteString aTmpStr;

                *pIStm >> aTmpStr; aThemeName = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );

                // Charakterkonvertierung durchfuehren
                if( nVersion >= 0x0004 )
                {
                    sal_uInt32  nCount;
                    sal_uInt16  nTemp16;

                    *pIStm >> nCount >> nTemp16;
                    pIStm->Seek( STREAM_SEEK_TO_END );

                    // pruefen, ob es sich um eine neuere Version handelt;
                    // daher um 520Bytes (8Bytes Kennung + 512Bytes Reserverpuffer ) zurueckspringen,
                    // falls dies ueberhaupt moeglich ist
                    if( pIStm->Tell() >= 520 )
                    {
                        sal_uInt32 nId1, nId2;

                        pIStm->SeekRel( -520 );
                        *pIStm >> nId1 >> nId2;

                        if( nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
                            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
                        {
                            VersionCompat* pCompat = new VersionCompat( *pIStm, STREAM_READ );

                            *pIStm >> nThemeId;

                            if( pCompat->GetVersion() >= 2 )
                            {
                                *pIStm >> bThemeNameFromResource;
                            }

                            delete pCompat;
                        }
                    }
                }

                INetURLObject aPathURL( rURL );

                aPathURL.removeSegment();
                aPathURL.removeFinalSlash();
                pRet = new GalleryThemeEntry( aPathURL, aThemeName,
                                              String(rURL.GetBase()).Copy( 2, 6 ).ToInt32(),
                                              bReadOnly, FALSE, FALSE, nThemeId,
                                              bThemeNameFromResource );
            }

            delete pIStm;
        }
    }

    return pRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetThumb( ULONG nPos, Bitmap& rBmp, BOOL )
{
    SgaObject*  pObj = AcquireObject( nPos );
    BOOL        bRet = FALSE;

    if( pObj )
    {
        rBmp = pObj->GetThumbBmp();
        ReleaseObject( pObj );
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetGraphic( ULONG nPos, Graphic& rGraphic, BOOL bProgress )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    BOOL                    bRet = FALSE;

    if( pObject )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );

        switch( pObject->eObjKind )
        {
            case( SGA_OBJ_BMP ):
            case( SGA_OBJ_ANIM ):
            case( SGA_OBJ_INET ):
            {
                String aFilterDummy;
                bRet = ( GalleryGraphicImport( aURL, rGraphic, aFilterDummy, bProgress ) != SGA_IMPORT_NONE );
            }
            break;

            case( SGA_OBJ_SVDRAW ):
            {
                SvxGalleryDrawModel aModel;

                if( aModel.GetModel() )
                {
                    if( GetModel( nPos, *aModel.GetModel(), bProgress ) )
                    {
                        ImageMap aIMap;

                        if( CreateIMapGraphic( *aModel.GetModel(), rGraphic, aIMap ) )
                            bRet = TRUE;
                        else
                        {
                            VirtualDevice aVDev;
                            aVDev.SetMapMode( MapMode( MAP_100TH_MM ) );
                            FmFormView aView( aModel.GetModel(), &aVDev );

                            aView.hideMarkHandles();
                            aView.ShowSdrPage(aView.GetModel()->GetPage(0));
                            aView.MarkAll();
                            rGraphic = aView.GetAllMarkedGraphic();
                            bRet = TRUE;
                        }
                    }
                }
            }
            break;

            case( SGA_OBJ_SOUND ):
            {
                SgaObject* pObj = AcquireObject( nPos );

                if( pObj )
                {
                    Bitmap aBmp( pObj->GetThumbBmp() );
                    aBmp.Replace( COL_LIGHTMAGENTA, COL_WHITE );
                    rGraphic = aBmp;
                    ReleaseObject( pObj );
                    bRet = TRUE;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertGraphic( const Graphic& rGraphic, ULONG nInsertPos )
{
    BOOL bRet = FALSE;

    if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        ULONG           nExportFormat = CVT_UNKNOWN;
        const GfxLink   aGfxLink( ( (Graphic&) rGraphic ).GetLink() );

        if( aGfxLink.GetDataSize() )
        {
            switch( aGfxLink.GetType() )
            {
                case( GFX_LINK_TYPE_EPS_BUFFER ): nExportFormat = CVT_SVM; break;
                case( GFX_LINK_TYPE_NATIVE_GIF ): nExportFormat = CVT_GIF; break;
                case( GFX_LINK_TYPE_NATIVE_JPG ): nExportFormat = CVT_JPG; break;
                case( GFX_LINK_TYPE_NATIVE_PNG ): nExportFormat = CVT_PNG; break;
                case( GFX_LINK_TYPE_NATIVE_TIF ): nExportFormat = CVT_TIF; break;
                case( GFX_LINK_TYPE_NATIVE_WMF ): nExportFormat = CVT_WMF; break;
                case( GFX_LINK_TYPE_NATIVE_MET ): nExportFormat = CVT_MET; break;
                case( GFX_LINK_TYPE_NATIVE_PCT ): nExportFormat = CVT_PCT; break;
                default:
                    break;
            }
        }
        else
        {
            if( rGraphic.GetType() == GRAPHIC_BITMAP )
            {
                if( rGraphic.IsAnimated() )
                    nExportFormat = CVT_GIF;
                else
                    nExportFormat = CVT_PNG;
            }
            else
                nExportFormat = CVT_SVM;
        }

        const INetURLObject aURL( ImplCreateUniqueURL( SGA_OBJ_BMP, nExportFormat ) );
        SvStream*           pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

        if( pOStm )
        {
            pOStm->SetVersion( SOFFICE_FILEFORMAT_50 );

            if( CVT_SVM == nExportFormat )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );

                aMtf.Write( *pOStm );
                bRet = ( pOStm->GetError() == ERRCODE_NONE );
            }
            else
            {
                if( aGfxLink.GetDataSize() && aGfxLink.GetData() )
                {
                    pOStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
                    bRet = ( pOStm->GetError() == ERRCODE_NONE );
                }
                else
                    bRet = ( GraphicConverter::Export( *pOStm, rGraphic, nExportFormat ) == ERRCODE_NONE );
            }

            delete pOStm;
        }

        if( bRet )
        {
            const SgaObjectBmp aObjBmp( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
            InsertObject( aObjBmp, nInsertPos );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetModel( ULONG nPos, SdrModel& rModel, BOOL )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    BOOL                    bRet = FALSE;

    if( pObject && ( SGA_OBJ_SVDRAW == pObject->eObjKind ) )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );
        SvStorageRef        xStor( GetSvDrawStorage() );

        if( xStor.Is() )
        {
            const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
            SvStorageStreamRef  xIStm( xStor->OpenSotStream( aStmName, STREAM_READ ) );

            if( xIStm.Is() && !xIStm->GetError() )
            {
                xIStm->SetBufferSize( STREAMBUF_SIZE );
                bRet = GallerySvDrawImport( *xIStm, rModel );
                xIStm->SetBufferSize( 0L );
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertModel( const FmFormModel& rModel, ULONG nInsertPos )
{
    INetURLObject   aURL( ImplCreateUniqueURL( SGA_OBJ_SVDRAW ) );
    SvStorageRef    xStor( GetSvDrawStorage() );
    BOOL            bRet = FALSE;

    if( xStor.Is() )
    {
        const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
        SvStorageStreamRef  xOStm( xStor->OpenSotStream( aStmName, STREAM_WRITE | STREAM_TRUNC ) );

        if( xOStm.Is() && !xOStm->GetError() )
        {
            SvMemoryStream  aMemStm( 65535, 65535 );
            FmFormModel*    pFormModel = (FmFormModel*) &rModel;

            pFormModel->BurnInStyleSheetAttributes();

            {
                uno::Reference< io::XOutputStream > xDocOut( new utl::OOutputStreamWrapper( aMemStm ) );

                if( xDocOut.is() )
                    SvxDrawingLayerExport( pFormModel, xDocOut );
            }

            aMemStm.Seek( 0 );

            xOStm->SetBufferSize( 16348 );
            GalleryCodec aCodec( *xOStm );
            aCodec.Write( aMemStm );

            if( !xOStm->GetError() )
            {
                SgaObjectSvDraw aObjSvDraw( rModel, aURL );
                bRet = InsertObject( aObjSvDraw, nInsertPos );
            }

            xOStm->SetBufferSize( 0L );
            xOStm->Commit();
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetModelStream( ULONG nPos, SotStorageStreamRef& rxModelStream, BOOL )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    BOOL                    bRet = FALSE;

    if( pObject && ( SGA_OBJ_SVDRAW == pObject->eObjKind ) )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );
        SvStorageRef        xStor( GetSvDrawStorage() );

        if( xStor.Is() )
        {
            const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
            SvStorageStreamRef  xIStm( xStor->OpenSotStream( aStmName, STREAM_READ ) );

            if( xIStm.Is() && !xIStm->GetError() )
            {
                UINT32 nVersion = 0;

                xIStm->SetBufferSize( 16348 );

                if( GalleryCodec::IsCoded( *xIStm, nVersion ) )
                {
                    SvxGalleryDrawModel aModel;

                    if( aModel.GetModel() )
                    {
                        if( GallerySvDrawImport( *xIStm, *aModel.GetModel() ) )
                        {
                            aModel.GetModel()->BurnInStyleSheetAttributes();

                            {
                                uno::Reference< io::XOutputStream > xDocOut( new utl::OOutputStreamWrapper( *rxModelStream ) );

                                if( SvxDrawingLayerExport( aModel.GetModel(), xDocOut ) )
                                    rxModelStream->Commit();
                            }
                        }

                        bRet = ( rxModelStream->GetError() == ERRCODE_NONE );
                    }
                }

                xIStm->SetBufferSize( 0 );
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertModelStream( const SotStorageStreamRef& rxModelStream, ULONG nInsertPos )
{
    INetURLObject   aURL( ImplCreateUniqueURL( SGA_OBJ_SVDRAW ) );
    SvStorageRef    xStor( GetSvDrawStorage() );
    BOOL            bRet = FALSE;

    if( xStor.Is() )
    {
        const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
        SvStorageStreamRef  xOStm( xStor->OpenSotStream( aStmName, STREAM_WRITE | STREAM_TRUNC ) );

        if( xOStm.Is() && !xOStm->GetError() )
        {
            GalleryCodec    aCodec( *xOStm );
            SvMemoryStream  aMemStm( 65535, 65535 );

            xOStm->SetBufferSize( 16348 );
            aCodec.Write( *rxModelStream );

            if( !xOStm->GetError() )
            {
                xOStm->Seek( 0 );
                SgaObjectSvDraw aObjSvDraw( *xOStm, aURL );
                bRet = InsertObject( aObjSvDraw, nInsertPos );
            }

            xOStm->SetBufferSize( 0L );
            xOStm->Commit();
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetURL( ULONG nPos, INetURLObject& rURL, BOOL )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    BOOL                    bRet = FALSE;

    if( pObject )
    {
        rURL = INetURLObject( ImplGetURL( pObject ) );
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertURL( const INetURLObject& rURL, ULONG nInsertPos )
{
    Graphic         aGraphic;
    String          aFormat;
    SgaObject*      pNewObj = NULL;
    const USHORT    nImportRet = GalleryGraphicImport( rURL, aGraphic, aFormat );
    BOOL            bRet = FALSE;

    if( nImportRet != SGA_IMPORT_NONE )
    {
        if ( SGA_IMPORT_INET == nImportRet )
            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, rURL, aFormat );
        else if ( aGraphic.IsAnimated() )
            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, rURL, aFormat );
        else
            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, rURL, aFormat );
    }
    else if( ::avmedia::MediaWindow::isMediaURL( rURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) ) )
        pNewObj = (SgaObject*) new SgaObjectSound( rURL );

    if( pNewObj && InsertObject( *pNewObj, nInsertPos ) )
        bRet = TRUE;

    delete pNewObj;

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, ULONG nInsertPos )
{
    INetURLObject                   aURL;
    ::std::vector< INetURLObject >  aURLVector;
    BOOL                            bRet = FALSE;

    try
    {
        ::ucbhelper::Content         aCnt( rFileOrDirURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >() );
        sal_Bool        bFolder = false;

        aCnt.getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder")) ) >>= bFolder;

        if( bFolder )
        {
            uno::Sequence< OUString > aProps( 1 );
            aProps.getArray()[ 0 ] = OUString(RTL_CONSTASCII_USTRINGPARAM("Url"));
            uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

            if( xResultSet.is() )
            {
                uno::Reference< ucb::XContentAccess > xContentAccess( xResultSet, uno::UNO_QUERY );

                if( xContentAccess.is() )
                {
                    while( xResultSet->next() )
                    {
                        aURL.SetSmartURL( xContentAccess->queryContentIdentifierString() );
                        aURLVector.push_back( aURL );
                    }
                }
            }
        }
        else
            aURLVector.push_back( rFileOrDirURL );
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

    ::std::vector< INetURLObject >::const_iterator aIter( aURLVector.begin() ), aEnd( aURLVector.end() );

    while( aIter != aEnd )
        bRet = bRet || InsertURL( *aIter++, nInsertPos );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertTransferable( const uno::Reference< datatransfer::XTransferable >& rxTransferable, ULONG nInsertPos )
{
    BOOL bRet = FALSE;

    if( rxTransferable.is() )
    {
        TransferableDataHelper  aDataHelper( rxTransferable );
        Graphic*                pGraphic = NULL;

        if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_DRAWING ) )
        {
            SotStorageStreamRef xModelStm;

            if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xModelStm ) )
                bRet = InsertModelStream( xModelStm, nInsertPos );
        }
        else if( aDataHelper.HasFormat( SOT_FORMAT_FILE_LIST ) ||
                 aDataHelper.HasFormat( FORMAT_FILE ) )
        {
            FileList aFileList;

            if( aDataHelper.HasFormat( SOT_FORMAT_FILE_LIST ) )
                aDataHelper.GetFileList( SOT_FORMAT_FILE_LIST, aFileList );
            else
            {
                String aFile;

                aDataHelper.GetString( FORMAT_FILE, aFile );

                if( aFile.Len() )
                    aFileList.AppendFile( aFile );
            }

            for( sal_uInt32 i = 0, nCount = aFileList.Count(); i < nCount; ++i )
            {
                const String    aFile( aFileList.GetFile( i ) );
                INetURLObject   aURL( aFile );

                if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
                {
                    String aLocalURL;

                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFile, aLocalURL ) )
                        aURL = INetURLObject( aLocalURL );
                }

                if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                    bRet = InsertFileOrDirURL( aURL, nInsertPos );
            }
        }
        else
        {
            Graphic aGraphic;
            ULONG   nFormat = 0;

            if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVXB ) )
                nFormat = SOT_FORMATSTR_ID_SVXB;
            else if( aDataHelper.HasFormat( FORMAT_GDIMETAFILE ) )
                nFormat = FORMAT_GDIMETAFILE;
            else if( aDataHelper.HasFormat( FORMAT_BITMAP ) )
                nFormat = FORMAT_BITMAP;

            if( nFormat && aDataHelper.GetGraphic( nFormat, aGraphic ) )
                pGraphic = new Graphic( aGraphic );
        }

        if( pGraphic )
        {
            bRet = FALSE;

            if( aDataHelper.HasFormat( SOT_FORMATSTR_ID_SVIM ) )
            {

                ImageMap aImageMap;

                // according to KA we don't need a BaseURL here
                if( aDataHelper.GetImageMap( SOT_FORMATSTR_ID_SVIM, aImageMap ) )
                {
                    SvxGalleryDrawModel aModel;

                    if( aModel.GetModel() )
                    {
                        SgaUserDataFactory  aFactory;

                        SdrPage*    pPage = aModel.GetModel()->GetPage(0);
                        SdrGrafObj* pGrafObj = new SdrGrafObj( *pGraphic );

                        pGrafObj->InsertUserData( new SgaIMapInfo( aImageMap ) );
                        pPage->InsertObject( pGrafObj );
                        bRet = InsertModel( *aModel.GetModel(), nInsertPos );
                    }
                }
            }

            if( !bRet )
                bRet = InsertGraphic( *pGraphic, nInsertPos );

            delete pGraphic;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void GalleryTheme::CopyToClipboard( Window* pWindow, ULONG nPos )
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos, false );
    pTransferable->CopyToClipboard( pWindow );
}

// -----------------------------------------------------------------------------

void GalleryTheme::StartDrag( Window* pWindow, ULONG nPos )
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos, true );
    pTransferable->StartDrag( pWindow, DND_ACTION_COPY | DND_ACTION_LINK );
}

// -----------------------------------------------------------------------------

SvStream& GalleryTheme::WriteData( SvStream& rOStm ) const
{
    const INetURLObject aRelURL1( GetParent()->GetRelativeURL() );
    const INetURLObject aRelURL2( GetParent()->GetUserURL() );
    INetURLObject       aNewURL, aTempURL;
    sal_uInt32          nCount = GetObjectCount();
    BOOL                bRel;

    rOStm << (sal_uInt16) 0x0004;
    rOStm << ByteString( GetRealName(), RTL_TEXTENCODING_UTF8 );
    rOStm << nCount << (sal_uInt16) gsl_getSystemTextEncoding();

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        const GalleryObject* pObj = ImplGetGalleryObject( i );
        String               aPath;

        if( SGA_OBJ_SVDRAW == pObj->eObjKind )
        {
            aPath = GetSvDrawStreamNameFromURL( pObj->aURL );
            bRel = FALSE;
        }
        else
        {
            aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
            bRel = ( ( aPath.Erase( sal::static_int_cast< xub_StrLen >( aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).getLength() ) ) ) == String(aRelURL1.GetMainURL( INetURLObject::NO_DECODE ) ));

            if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::NO_DECODE ).getLength() > ( aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).getLength() + 1 ) ) )
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
                aPath = aPath.Erase( 0, sal::static_int_cast< xub_StrLen >( aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).getLength() ) );
            }
            else
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
                bRel = ( ( aPath.Erase( sal::static_int_cast< xub_StrLen >( aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).getLength() ) ) ) == String(aRelURL2.GetMainURL( INetURLObject::NO_DECODE ) ));

                if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::NO_DECODE ).getLength() > ( aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).getLength() + 1 ) ) )
                {
                    aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
                    aPath = aPath.Erase( 0, sal::static_int_cast< xub_StrLen >( aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).getLength() ) );
                }
                else
                    aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
            }
        }

        aPath.SearchAndReplace(m_aDestDir, String());
        rOStm << bRel << ByteString( aPath, RTL_TEXTENCODING_UTF8 ) << pObj->nOffset << (sal_uInt16) pObj->eObjKind;
    }

    // neuerdings wird ein 512-Byte-Reservepuffer gechrieben;
    // um diesen zu erkennen werden zwei ULONG-Ids geschrieben
    rOStm << COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) << COMPAT_FORMAT( 'E', 'S', 'R', 'V' );

    const long      nReservePos = rOStm.Tell();
    VersionCompat*  pCompat = new VersionCompat( rOStm, STREAM_WRITE, 2 );

    rOStm << (sal_uInt32) GetId() << IsThemeNameFromResource(); // ab Version 2

    delete pCompat;

    // Rest des Puffers auffuellen
    const long  nRest = Max( 512L - ( (long) rOStm.Tell() - nReservePos ), 0L );

    if( nRest )
    {
        char* pReserve = new char[ nRest ];
        memset( pReserve, 0, nRest );
        rOStm.Write( pReserve, nRest );
        delete[] pReserve;
    }

    return rOStm;
}

// ------------------------------------------------------------------------

SvStream& GalleryTheme::ReadData( SvStream& rIStm )
{
    sal_uInt32          nCount;
    sal_uInt16          nVersion;
    ByteString          aTmpStr;
    String              aThemeName;
    rtl_TextEncoding    nTextEncoding;

    aImportName = String();
    rIStm >> nVersion >> aTmpStr >> nCount;

    if( nVersion >= 0x0004 )
    {
        UINT16 nTmp16;
        rIStm >> nTmp16;
        nTextEncoding = (rtl_TextEncoding) nTmp16;
    }
    else
        nTextEncoding = RTL_TEXTENCODING_UTF8;

    aThemeName = String( aTmpStr.GetBuffer(), nTextEncoding );

    if( nCount <= ( 1L << 14 ) )
    {
        GalleryObject*  pObj;
        INetURLObject   aRelURL1( GetParent()->GetRelativeURL() );
        INetURLObject   aRelURL2( GetParent()->GetUserURL() );
        sal_uInt32      nId1, nId2;
        BOOL            bRel;

        for( size_t i = 0, n = aObjectList.size(); i < n; ++i )
        {
            pObj = aObjectList[ i ];
            Broadcast( GalleryHint( GALLERY_HINT_CLOSE_OBJECT, GetName(), reinterpret_cast< ULONG >( pObj ) ) );
            delete pObj;
            Broadcast( GalleryHint( GALLERY_HINT_OBJECT_REMOVED, GetName(), reinterpret_cast< ULONG >( pObj ) ) );
        }
        aObjectList.clear();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            pObj = new GalleryObject;

            ByteString  aTempFileName;
            String      aFileName;
            String      aPath;
            sal_uInt16  nTemp;

            rIStm >> bRel >> aTempFileName >> pObj->nOffset;
            rIStm >> nTemp; pObj->eObjKind = (SgaObjKind) nTemp;

            aFileName = String( aTempFileName.GetBuffer(), gsl_getSystemTextEncoding() );

            if( bRel )
            {
                aFileName.SearchAndReplaceAll( '\\', '/' );
                aPath = aRelURL1.GetMainURL( INetURLObject::NO_DECODE );

                if( aFileName.GetChar( 0 ) != '/' )
                        aPath += '/';

                aPath += aFileName;

                pObj->aURL = INetURLObject( aPath );

                if( !FileExists( pObj->aURL ) )
                {
                    aPath = aRelURL2.GetMainURL( INetURLObject::NO_DECODE );

                    if( aFileName.GetChar( 0 ) != '/' )
                        aPath += '/';

                    aPath += aFileName;

                    // assign this URL, even in the case it is not valid (#94482)
                    pObj->aURL = INetURLObject( aPath );
                }
            }
            else
            {
                if( SGA_OBJ_SVDRAW == pObj->eObjKind )
                {
                    const static String aBaseURLStr( RTL_CONSTASCII_USTRINGPARAM( "gallery/svdraw/" ) );

                    String aDummyURL( aBaseURLStr );
                    pObj->aURL = INetURLObject( aDummyURL += aFileName, INET_PROT_PRIV_SOFFICE );
                }
                else
                {
                    String aLocalURL;

                    pObj->aURL = INetURLObject( aFileName );

                    if( ( pObj->aURL.GetProtocol() == INET_PROT_NOT_VALID ) &&
                        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aLocalURL ) )
                    {
                        pObj->aURL = INetURLObject( aLocalURL );
                    }
                }
            }

            aObjectList.push_back( pObj );
        }

        rIStm >> nId1 >> nId2;

        // in neueren Versionen befindet sich am Ende ein 512-Byte-Reservepuffer;
        // die Daten befinden sich am Anfang dieses Puffers und
        // sind durch eine VersionCompat geklammert
        if( !rIStm.IsEof() &&
            nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
        {
            VersionCompat*  pCompat = new VersionCompat( rIStm, STREAM_READ );
            sal_uInt32      nTemp32;
            BOOL            bThemeNameFromResource = FALSE;

            rIStm >> nTemp32;

            if( pCompat->GetVersion() >= 2 )
            {
                rIStm >> bThemeNameFromResource;
            }

            SetId( nTemp32, bThemeNameFromResource );
            delete pCompat;
        }
    }
    else
        rIStm.SetError( SVSTREAM_READ_ERROR );

    ImplSetModified( FALSE );

    return rIStm;
}

// ------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOut, const GalleryTheme& rTheme )
{
    return rTheme.WriteData( rOut );
}

// ------------------------------------------------------------------------

SvStream& operator>>( SvStream& rIn, GalleryTheme& rTheme )
{
    return rTheme.ReadData( rIn );
}

void GalleryTheme::ImplSetModified( BOOL bModified )
{ pThm->SetModified( bModified ); }

const String& GalleryTheme::GetRealName() const { return pThm->GetThemeName(); }
const INetURLObject& GalleryTheme::GetThmURL() const { return pThm->GetThmURL(); }
const INetURLObject& GalleryTheme::GetSdgURL() const { return pThm->GetSdgURL(); }
const INetURLObject& GalleryTheme::GetSdvURL() const { return pThm->GetSdvURL(); }
UINT32 GalleryTheme::GetId() const { return pThm->GetId(); }
void GalleryTheme::SetId( UINT32 nNewId, BOOL bResetThemeName ) { pThm->SetId( nNewId, bResetThemeName ); }
BOOL GalleryTheme::IsThemeNameFromResource() const { return pThm->IsNameFromResource(); }
BOOL GalleryTheme::IsImported() const { return pThm->IsImported(); }
BOOL GalleryTheme::IsReadOnly() const { return pThm->IsReadOnly(); }
BOOL GalleryTheme::IsDefault() const { return pThm->IsDefault(); }
BOOL GalleryTheme::IsModified() const { return pThm->IsModified(); }
const String& GalleryTheme::GetName() const { return IsImported() ? aImportName : pThm->GetThemeName(); }

void GalleryTheme::InsertAllThemes( ListBox& rListBox )
{
    for( USHORT i = RID_GALLERYSTR_THEME_FIRST; i <= RID_GALLERYSTR_THEME_LAST; i++ )
        rListBox.InsertEntry( String( GAL_RESID( i ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
