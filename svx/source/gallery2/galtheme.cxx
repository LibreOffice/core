/*************************************************************************
 *
 *  $RCSfile: galtheme.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-16 12:32:00 $
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

#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <tools/new.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <so3/svstor.hxx>
#include <sot/formats.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/docfile.hxx>
#include "svdograf.hxx"
#include "fmpage.hxx"
#include "codec.hxx"
#include "fmmodel.hxx"
#include "fmview.hxx"
#include "galmisc.hxx"
#include "galtheme.hxx"

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;

// ------------
// - SgaTheme -
// ------------

GalleryTheme::GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry ) :
        pParent     ( pGallery ),
        pThm        ( pThemeEntry ),
        mnLockCount ( 0 ),
        nDragPos    ( 0 ),
        bDragging   ( FALSE )
{
    ImplCreateSvDrawStorage();

    if( pThm->IsImported() )
        aImportName = pThm->GetThemeName();
}

// ------------------------------------------------------------------------

GalleryTheme::~GalleryTheme()
{
    ImplWrite();

    for( GalleryObject* pEntry = aObjectList.First(); pEntry; pEntry = aObjectList.Next() )
        delete pEntry;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplCreateSvDrawStorage()
{
    if( !pThm->IsImported() )
        aSvDrawStorageRef = new SvStorage( GetSdvURL().GetMainURL(), pThm->IsReadOnly() ? STREAM_READ : STREAM_STD_READWRITE );
    else
        aSvDrawStorageRef = new SvStorage();
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ImplWriteSgaObject( const SgaObject& rObj, ULONG nPos, GalleryObject* pExistentEntry )
{
    SvStream*   pOStm = ::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL(), STREAM_WRITE );
    BOOL        bRet = FALSE;

    if( pOStm )
    {
        const sal_uInt32 nOffset = pOStm->Seek( STREAM_SEEK_TO_END );

        *pOStm << rObj;

        if( !pOStm->GetError() )
        {
            GalleryObject* pEntry;

            if( !pExistentEntry )
            {
                pEntry = new GalleryObject;
                aObjectList.Insert( pEntry, nPos );
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
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL(), STREAM_READ );

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
    SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL(), STREAM_READ );

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
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL(), STREAM_WRITE | STREAM_COPY_ON_SYMLINK | STREAM_TRUNC );
#else
            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL(), STREAM_WRITE | STREAM_TRUNC );
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

void GalleryTheme::UnlockBroadcaster( ULONG nUpdatePos )
{
    DBG_ASSERT( mnLockCount, "Broadcaster is not locked" );

    if( mnLockCount && !--mnLockCount )
        ImplBroadcast( nUpdatePos );
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::InsertObject( const SgaObject& rObj, ULONG nInsertPos )
{
    BOOL bRet = FALSE;

    if( rObj.IsValid() )
    {
        GalleryObject   aNewEntry;
        GalleryObject*  pEntry = aObjectList.First();
        GalleryObject*  pFoundEntry = NULL;
        ULONG           nUpdatePos = LIST_APPEND;

        for( ; pEntry && !pFoundEntry; pEntry = aObjectList.Next() )
            if( pEntry->aURL == rObj.GetURL() )
                pFoundEntry = pEntry;

        if( pFoundEntry && ImplWriteSgaObject( rObj, nInsertPos, &aNewEntry ) )
            pFoundEntry->nOffset = aNewEntry.nOffset;
        else
            ImplWriteSgaObject( rObj, nInsertPos, NULL );

        ImplSetModified( bRet = TRUE );
        ImplBroadcast( pFoundEntry ? aObjectList.GetPos( pFoundEntry ) : nInsertPos );
    }

    return bRet;
}

// ------------------------------------------------------------------------

SgaObject* GalleryTheme::AcquireObject( ULONG nPos )
{
    return ImplReadSgaObject( aObjectList.GetObject( nPos ) );
}

// ------------------------------------------------------------------------

void GalleryTheme::ReleaseObject( SgaObject* pObject )
{
    delete pObject;
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::RemoveObject( ULONG nPos )
{
    GalleryObject* pEntry = aObjectList.Remove( nPos );

    if( !aObjectList.Count() )
        KillFile( GetSdgURL() );

    if( pEntry )
    {
        if( SGA_OBJ_SVDRAW == pEntry->eObjKind )
            aSvDrawStorageRef->Remove( pEntry->aURL.GetMainURL() );

        delete pEntry;
        ImplSetModified( TRUE );
        ImplBroadcast( nPos );
    }

    return( pEntry != NULL );
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ChangeObjectPos( ULONG nOldPos, ULONG nNewPos )
{
    BOOL bRet = FALSE;

    if( nOldPos != nNewPos )
    {
        GalleryObject* pEntry = aObjectList.GetObject( nOldPos );

        if( pEntry )
        {
            aObjectList.Insert( pEntry, nNewPos );

            if( nNewPos < nOldPos )
                nOldPos++;

            aObjectList.Remove( nOldPos );
            ImplSetModified( bRet = TRUE );
            ImplBroadcast( ( nNewPos < nOldPos ) ? nNewPos : ( nNewPos - 1 ) );
        }
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
        const ULONG     nCount = aObjectList.Count();
        ULONG           i;

        LockBroadcaster();
        bAbortActualize = FALSE;

        // LoeschFlag zuruecksetzen
        for ( i = 0; i < nCount; i++ )
            aObjectList.GetObject( i )->bDummy = FALSE;

        for( i = 0; ( i < nCount ) && !bAbortActualize; i++ )
        {
            if( pProgress )
                pProgress->Update( i, nCount - 1 );

            pEntry = aObjectList.GetObject( i );
            aActualizeURL = pEntry->aURL;
            rActualizeLink.Call( this );

            // SvDraw-Objekte werden spaeter aktualisiert
            if( pEntry->eObjKind != SGA_OBJ_SVDRAW )
            {
                // Hier muss noch etwas eingebaut werden,
                // das Files auf den ensprechenden Eintrag matched
                // Grafiken als Grafik-Objekte in die Gallery aufnehmen
#ifndef MAC
                if( pEntry->aURL.GetExtension().CompareIgnoreCaseToAscii( "wav" ) != COMPARE_EQUAL )
#else
                if( TRUE )
#endif
                {
                    // vermeiden, dass ein native Link angelegt wird,
                    // da dies an dieser Stelle nicht noetig ist
                    aGraphic.Clear();
                    aGraphic.SetLink( GfxLink() );

                    if ( SGAImport( aActualizeURL, aGraphic, aFormat ) )
                    {
                        SgaObject* pNewObj;

                        if ( SGA_OBJ_INET == pEntry->eObjKind )
                            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, aActualizeURL, aFormat );
                        else if ( aGraphic.IsAnimated() )
                            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, aActualizeURL, aFormat );
                        else
                            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, aActualizeURL, aFormat );

                        if( !InsertObject( *pNewObj ) )
                            pEntry->bDummy = TRUE;

                        delete pNewObj;
                    }
                    else
                        pEntry->bDummy = TRUE; // Loesch-Flag setzen
                }
                // restliche Sachen als Sound-Objekte aufnehmen
                else
                {
                    SgaObjectSound aObjSound( aActualizeURL );
                    if( !InsertObject( aObjSound ) )
                        pEntry->bDummy = TRUE;
                }
            }
            else
            {
                INetURLObject aURL( pEntry->aURL );

                if ( aSvDrawStorageRef.Is() )
                {
                    const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
                    SvStorageStreamRef  pIStm = aSvDrawStorageRef->OpenStream( aStmName, STREAM_READ );

                    if ( pIStm && !pIStm->GetError() )
                    {
                        SvMemoryStream  aMemStm;

                        pIStm->SetBufferSize( 16384 );

                        if ( !RLECodec::IsRLECoded( *pIStm ) )
                        {
                            const ULONG nSize = pIStm->Seek( STREAM_SEEK_TO_END );
                            void*       pBuffer = SvMemAlloc( nSize );

                            pIStm->Seek( STREAM_SEEK_TO_BEGIN );

                            if ( pBuffer && nSize )
                            {
                                pIStm->Read( pBuffer, nSize );

                                pIStm->SetBufferSize( 0 );
                                pIStm.Clear();
                                pIStm = aSvDrawStorageRef->OpenStream( aStmName, STREAM_WRITE | STREAM_TRUNC );
                                pIStm->SetBufferSize( 16384 );
                                pIStm->Seek( STREAM_SEEK_TO_BEGIN );

                                RLECodec aCodec( *pIStm );

                                aMemStm.SetBuffer( (char*) pBuffer, nSize, FALSE, nSize );
                                aCodec.Write( aMemStm );

                                SgaObjectSvDraw aNewObj( aMemStm, aURL );
                                if( !InsertObject( aNewObj ) )
                                    pEntry->bDummy = TRUE;
                            }

                            if( pBuffer )
                                SvMemFree( pBuffer );
                        }
                        else
                        {
                            SgaObjectSvDraw aNewObj( *pIStm, aURL );
                            if( !InsertObject( aNewObj ) )
                                pEntry->bDummy = TRUE;
                        }

                        pIStm->SetBufferSize( 0L );
                    }
                }
            }
        }

        // remove all entries with set flag
        pEntry = aObjectList.First();
        while( pEntry )
        {
            if( pEntry->bDummy )
            {
                delete aObjectList.Remove( pEntry );
                pEntry = aObjectList.GetCurObject();
            }
            else
                pEntry = aObjectList.Next();
        }

        // update theme
        ::utl::TempFile aTmp;
        INetURLObject   aInURL( GetSdgURL() );
        INetURLObject   aTmpURL( aTmp.GetURL() );

        DBG_ASSERT( aInURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        DBG_ASSERT( aTmpURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aInURL.GetMainURL(), STREAM_READ );
        SvStream* pTmpStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC );

        if( pIStm && pTmpStm )
        {
            pEntry = aObjectList.First();

            while( pEntry )
            {
                SgaObject* pObj;

                switch( pEntry->eObjKind )
                {
                    case( SGA_OBJ_BMP ):    pObj = new SgaObjectBmp(); break;
                    case( SGA_OBJ_ANIM ):   pObj = new SgaObjectAnim(); break;
                    case( SGA_OBJ_INET ):   pObj = new SgaObjectINet(); break;
                    case( SGA_OBJ_SVDRAW ): pObj = new SgaObjectSvDraw(); break;
                    case (SGA_OBJ_SOUND):   pObj = new SgaObjectSound(); break;

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

                pEntry = aObjectList.Next();
            }
        }
        else
        {
            DBG_ERROR( "File(s) could not be opened" );
        }

        delete pIStm;
        delete pTmpStm;

        CopyFile( aTmpURL, aInURL );
        KillFile( aTmpURL );

        ULONG nStorErr = 0;

        {
            SvStorageRef aTempStorageRef( new SvStorage( aTmpURL.GetMainURL() ) );
            aSvDrawStorageRef->CopyTo( aTempStorageRef );
            nStorErr = aSvDrawStorageRef->GetError();
        }

        if( !nStorErr )
        {
            aSvDrawStorageRef = new SvStorage();
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
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( rURL.GetMainURL(), STREAM_READ );

        if( pIStm )
        {
            String          aThemeName;
            sal_uInt32      nThemeId = 0;
            sal_uInt16      nVersion;
            BOOL            bThemeNameFromResource = FALSE;

            *pIStm >> nVersion;

            if( nVersion <= 0x00ff )
            {
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
                        ULONG nId1, nId2;

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
                                              rURL.GetBase().Copy( 2, 6 ).ToInt32(),
                                              bReadOnly, FALSE, FALSE, nThemeId,
                                              bThemeNameFromResource );
            }

            delete pIStm;
        }
    }

    return pRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetThumb( ULONG nPos, Bitmap& rBmp, BOOL bProgress )
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
                bRet = ( SGAImport( aURL, rGraphic, aFilterDummy, bProgress ) != SGA_IMPORT_NONE );
            }
            break;

            case( SGA_OBJ_SVDRAW ):
            {
                FmFormModel aModel;

                aModel.GetItemPool().FreezeIdRanges();

                if( GetModel( nPos, aModel, bProgress ) )
                {
                    ImageMap aIMap;

                    if( CreateIMapGraphic( aModel, rGraphic, aIMap ) )
                        bRet = TRUE;
                    else
                    {
                        VirtualDevice aVDev;
                        aVDev.SetMapMode( MapMode( MAP_100TH_MM ) );
                        FmFormView aView( &aModel, &aVDev );

                        aView.SetMarkHdlHidden( TRUE );
                        aView.ShowPagePgNum( 0, Point() );
                        aView.MarkAll();
                        rGraphic = aView.GetAllMarkedMetaFile();
                        bRet = TRUE;
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
        INetURLObject aURL( CreateUniqueURL( GetParent(), SGA_OBJ_BMP ) );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC );

        if( pOStm )
        {
            if( rGraphic.GetType() == GRAPHIC_BITMAP )
            {
                ULONG nExportFormat;

                if( rGraphic.IsAnimated() )
                {
                    aURL.SetExtension( String( RTL_CONSTASCII_USTRINGPARAM( "gif" ) ) );
                    nExportFormat = CVT_GIF;
                }
                else
                {
                    aURL.SetExtension( String( RTL_CONSTASCII_USTRINGPARAM( "png" ) ) );
                    nExportFormat = CVT_PNG;
                }

                pOStm->SetVersion( SOFFICE_FILEFORMAT_NOW );
                bRet = ( GraphicConverter::Export( *pOStm, rGraphic, nExportFormat ) == ERRCODE_NONE );
            }
            else
            {
                aURL.SetExtension( String( RTL_CONSTASCII_USTRINGPARAM( "svm" ) ) );

                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );

                pOStm->SetVersion( SOFFICE_FILEFORMAT_NOW );
                aMtf.Write( *pOStm );
                bRet = ( pOStm->GetError() == ERRCODE_NONE );
            }

            delete pOStm;
        }

        if( bRet )
        {
            const SgaObjectBmp aObjBmp( aURL.GetMainURL() );
            InsertObject( aObjBmp, nInsertPos );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetModel( ULONG nPos, FmFormModel& rModel, BOOL bProgress )
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
            SvStorageStreamRef  xIStm( xStor->OpenStream( aStmName, STREAM_READ ) );

            if( xIStm.Is() && !xIStm->GetError() )
            {
                xIStm->SetBufferSize( STREAMBUF_SIZE );
                bRet = SGASvDrawImport( *xIStm, rModel );
                xIStm->SetBufferSize( 0L );
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertModel( const FmFormModel& rModel, ULONG nInsertPos )
{
    INetURLObject   aURL( CreateUniqueURL( GetParent(), SGA_OBJ_SVDRAW ) );
    SvStorageRef    xStor( GetSvDrawStorage() );
    BOOL            bRet = FALSE;

    if( xStor.Is() )
    {
        const String        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
        SvStorageStreamRef  xOStm( xStor->OpenStream( aStmName, STREAM_WRITE | STREAM_TRUNC ) );

        if( xOStm.Is() && !xOStm->GetError() )
        {
            SvMemoryStream  aMemStm( 65535, 65535 );
            RLECodec        aCodec( *xOStm );

            aMemStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
            ( (FmFormModel&) rModel ).SetStreamingSdrModel( TRUE );
            ( (FmFormModel&) rModel ).PreSave();
            rModel.GetItemPool().Store( aMemStm );
            aMemStm << rModel;
            ( (FmFormModel&) rModel ).PostSave();
            ( (FmFormModel&) rModel ).SetStreamingSdrModel( FALSE );
            aMemStm.Seek( 0L );

            xOStm->SetBufferSize( STREAMBUF_SIZE );
            aCodec.Write( aMemStm );

            if( !xOStm->GetError() )
            {
                SgaObjectSvDraw aObjSvDraw( rModel, aURL );
                InsertObject( aObjSvDraw, nInsertPos );
                bRet = TRUE;
            }

            xOStm->SetBufferSize( 0L );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetURL( ULONG nPos, INetURLObject& rURL, BOOL bProgress )
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
    const USHORT    nImportRet = SGAImport( rURL, aGraphic, aFormat );
    BOOL            bRet = FALSE;

    aGraphic.SetLink( GfxLink() );

    if( nImportRet != SGA_IMPORT_NONE )
    {
        if ( SGA_IMPORT_INET == nImportRet )
            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, rURL, aFormat );
        else if ( aGraphic.IsAnimated() )
            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, rURL, aFormat );
        else
            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, rURL, aFormat );
    }
    else if( SGAIsSoundFile( rURL ) )
        pNewObj = (SgaObject*) new SgaObjectSound( rURL );

    if( pNewObj && InsertObject( *pNewObj, nInsertPos ) )
        bRet = TRUE;

    delete pNewObj;

    return bRet;
}

// -----------------------------------------------------------------------------

SvDataTypeList GalleryTheme::GetDataXChgTypeList( const SvDataTypeList& rTypeList, ULONG nPos )
{
    SvDataTypeList          aTypeList( rTypeList );
    const GalleryObject*    pObj = ImplGetGalleryObject( nPos );

    if( pObj )
    {
        if( pObj->eObjKind == SGA_OBJ_SVDRAW )
        {
            FmFormModel aModel;
            Graphic     aGraphic;
            ImageMap    aIMap;

            aModel.GetItemPool().FreezeIdRanges();

            if( GetModel( nPos, aModel ) && CreateIMapGraphic( aModel, aGraphic, aIMap ) )
            {
                aTypeList.Insert( SvDataType( SOT_FORMATSTR_ID_SVXB, MEDIUM_STREAM | MEDIUM_MEMORY ), LIST_APPEND );
                aTypeList.Insert( SvDataType( SOT_FORMATSTR_ID_SVIM, MEDIUM_STREAM | MEDIUM_MEMORY ), LIST_APPEND );
            }
            else
                aTypeList.Insert( SvDataType( SOT_FORMATSTR_ID_DRAWING ), LIST_APPEND );
        }
        else
        {
            aTypeList.Insert( SvDataType( FORMAT_FILE, MEDIUM_MEMORY ), LIST_APPEND );

            if( ( pObj->eObjKind == SGA_OBJ_BMP ) || ( pObj->eObjKind == SGA_OBJ_ANIM ) )
            {
                aTypeList.Insert( SvDataType( SOT_FORMATSTR_ID_SVXB, MEDIUM_STREAM | MEDIUM_MEMORY ), LIST_APPEND );
                aTypeList.Insert( SvDataType( FORMAT_GDIMETAFILE, MEDIUM_MEMORY ), LIST_APPEND );
                aTypeList.Insert( SvDataType( FORMAT_BITMAP, MEDIUM_MEMORY ), LIST_APPEND );
            }
        }
    }

    return aTypeList;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::GetDataXChgData( SvData* pData, ULONG nFormat, ULONG nPos )
{
    BOOL bRet = FALSE;

    if( pData )
    {
        if( nFormat == SOT_FORMATSTR_ID_SVXB )
        {
            Graphic aGraphic;
            bRet = GetGraphic( nPos, aGraphic );

            if( bRet )
                pData->SetData( (SvDataCopyStream*) &aGraphic );
        }
        else if( nFormat == SOT_FORMATSTR_ID_SVIM )
        {
            FmFormModel aModel;
            Graphic     aGraphic;
            ImageMap    aIMap;

            aModel.GetItemPool().FreezeIdRanges();
            bRet = ( GetModel( nPos, aModel ) && CreateIMapGraphic( aModel, aGraphic, aIMap ) );

            if( bRet )
                pData->SetData( (SvDataCopyStream*) &aIMap );
        }
        else if( nFormat == SOT_FORMATSTR_ID_DRAWING )
        {
            SvStorageRef xStor( GetSvDrawStorage() );

            if( xStor.Is() )
            {
                const String        aStmName( GetSvDrawStreamNameFromURL( GetObjectURL( nPos ) ) );
                SvStorageStreamRef  xIStm( xStor->OpenStream( aStmName, STREAM_READ ) );

                if( xIStm.Is() && !xIStm->GetError() )
                {
                    xIStm->SetBufferSize( 16384 );

                    if( RLECodec::IsRLECoded( *xIStm ) )
                    {
                        SvMemoryStream  aMemStm;
                        RLECodec        aCodec( *xIStm );
                        ULONG           nSize;

                        nSize = aCodec.Read( aMemStm );
                        pData->SetData( (void*) aMemStm.GetData(), nSize, TRANSFER_COPY );
                        bRet = TRUE;
                    }
                    else
                    {
                        const ULONG nSize = xIStm->Seek( STREAM_SEEK_TO_END );
                        void*       pBuffer = SvMemAlloc( nSize );

                        if ( pBuffer && nSize )
                        {
                            xIStm->Seek( STREAM_SEEK_TO_BEGIN );
                            xIStm->Read( pBuffer, nSize );
                            pData->SetData( pBuffer, nSize, TRANSFER_COPY );
                            bRet = TRUE;
                        }

                        if ( pBuffer )
                            SvMemFree( pBuffer );
                    }

                    xIStm->SetBufferSize( 0L );
                }
            }
        }
        else
        {
            switch( nFormat )
            {
                case( FORMAT_FILE ):
                {
                    pData->SetData( GetObjectURL( nPos ).GetMainURL() );
                    bRet = TRUE;
                }
                break;


                case( FORMAT_BITMAP ):
                {
                    Graphic aGraphic;

                    bRet = GetGraphic( nPos, aGraphic );

                    if( bRet )
                    {
                        Bitmap aBmp( aGraphic.GetBitmap() );
                        pData->SetData( &aBmp );
                    }
                }
                break;


                case( FORMAT_GDIMETAFILE ):
                {
                    Graphic aGraphic;

                    bRet = GetGraphic( nPos, aGraphic );

                    if( bRet )
                    {
                        GDIMetaFile aMtf( aGraphic.GetGDIMetaFile() );
                        pData->SetData( &aMtf );
                    }
                }
                break;

                default:
                break;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertDataXChgData( SvDataObjectRef& rxData, ULONG nInsertPos )
{
    const SvDataTypeList&   rTypeList = rxData->GetTypeList();
    Graphic*                pGraphic = NULL;
    BOOL                    bRet = FALSE;

    for( ULONG n = 0, nCount = rTypeList.Count(); ( n < nCount ) && !bRet ; n++ )
    {
        const ULONG nFormat = rTypeList.GetObject( n ).GetFormat();

        if( SOT_FORMATSTR_ID_DRAWING == nFormat )
        {
            SvData aData( SOT_FORMATSTR_ID_DRAWING );

            if( rxData->GetData( &aData ) )
            {
                void* pSvDrawData = NULL;

                if( aData.GetData( &pSvDrawData, TRANSFER_MOVE ) )
                {
                    FmFormModel     aModel;
                    SvMemoryStream  aMemStm;
                    const ULONG     nSvDrawDataSize = aData.GetMinMemorySize();

                    aModel.GetItemPool().FreezeIdRanges();
                    aMemStm.SetBuffer( (char*) pSvDrawData, nSvDrawDataSize, FALSE, nSvDrawDataSize );

                    if( SGASvDrawImport( aMemStm, aModel ) )
                        bRet = InsertModel( aModel, nInsertPos );
                }
            }
        }
        else if( FORMAT_FILE == nFormat )
        {
            SvData aData( FORMAT_FILE );

            if( rxData->GetData( &aData ) )
            {
                String aURLStr;

                if( aData.GetData( aURLStr ) )
                {
                    INetURLObject aURL( aURLStr );
                    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

                    try
                    {
                        Content     aCnt( aURL.GetMainURL(), uno::Reference< XCommandEnvironment >() );
                        sal_Bool    bFolder;

                        aCnt.getPropertyValue( OUString::createFromAscii( "IsFolder" ) ) >>= bFolder;

                        if( bFolder )
                        {
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
#if SUPD>611
                                        aURL.SetSmartURL( xContentAccess->queryContentIdentifierString() );
#else
                                        aURL.SetSmartURL( xContentAccess->queryContentIdentfierString() );
#endif
                                        bRet = bRet || InsertURL( aURL, nInsertPos );
                                    }
                                }
                            }
                        }
                        else
                            bRet = InsertURL( aURL, nInsertPos );
                    }
                    catch( ... )
                    {
                        DBG_ERROR( "GalleryTheme::InsertDataXChgData: ucb error" );
                    }
                }
            }
        }
        else if( SOT_FORMATSTR_ID_SVXB == nFormat )
        {
            SvData aData( SOT_FORMATSTR_ID_SVXB );

            if( rxData->GetData( &aData ) )
            {
                if( aData.GetData( (SvDataCopyStream**) &pGraphic, Graphic::StaticType(), TRANSFER_MOVE ) )
                    bRet = TRUE;
                else
                    delete pGraphic, pGraphic = NULL;
            }
        }
        else if( FORMAT_GDIMETAFILE == nFormat )
        {
            SvData aData( FORMAT_GDIMETAFILE );

            if( rxData->GetData( &aData ) )
            {
                GDIMetaFile* pMtf = NULL;

                if( aData.GetData( &pMtf, TRANSFER_MOVE ) && pMtf )
                {
                    pGraphic = new Graphic( *pMtf );
                    bRet = TRUE;
                }

                delete pMtf;
            }
        }
        else if ( FORMAT_BITMAP == nFormat )
        {
            SvData aData( FORMAT_BITMAP );

            if( rxData->GetData( &aData ) )
            {
                Bitmap* pBmp = NULL;

                if( aData.GetData( &pBmp, TRANSFER_MOVE ) && pBmp )
                {
                    pGraphic = new Graphic( *pBmp );
                    bRet = TRUE;
                }

                delete pBmp;
            }
        }
    }

    if( pGraphic )
    {
        bRet = FALSE;

        if( rTypeList.HasEqualType( SvDataType( SOT_FORMATSTR_ID_SVIM, MEDIUM_STREAM | MEDIUM_MEMORY ) ) )
        {
            SvData aData( SOT_FORMATSTR_ID_SVIM );

            if( rxData->GetData( &aData ) )
            {
                ImageMap* pIMap = NULL;

                if( aData.GetData( (SvDataCopyStream**) &pIMap, ImageMap::StaticType(), TRANSFER_MOVE ) && pIMap )
                {
                    FmFormModel         aModel;
                    SgaUserDataFactory  aFactory;

                    aModel.GetItemPool().FreezeIdRanges();

                    SdrPage*    pPage = aModel.AllocPage( FALSE );
                    SdrGrafObj* pGrafObj = new SdrGrafObj( *pGraphic );

                    pGrafObj->InsertUserData( new SgaIMapInfo( *pIMap ) );
                    pPage->InsertObject( pGrafObj );
                    aModel.SetPageNotValid( TRUE );
                    aModel.InsertPage( pPage );
                    bRet = InsertModel( aModel, nInsertPos );
                }

                delete pIMap;
            }
        }

        if( !bRet )
            bRet = InsertGraphic( *pGraphic, nInsertPos );

        delete pGraphic;
    }

    return bRet;
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
            aPath = pObj->aURL.GetMainURL();
            bRel = ( ( aPath.Erase( aRelURL1.GetMainURL().Len() ) ) == aRelURL1.GetMainURL() );

            if( bRel && ( pObj->aURL.GetMainURL().Len() > ( aRelURL1.GetMainURL().Len() + 1 ) ) )
            {
                aPath = pObj->aURL.GetMainURL();
                aPath = aPath.Erase( 0, aRelURL1.GetMainURL().Len() );
            }
            else
            {
                aPath = pObj->aURL.GetMainURL();

                bRel = ( ( aPath.Erase( aRelURL2.GetMainURL().Len() ) ) == aRelURL2.GetMainURL() );

                if( bRel && ( pObj->aURL.GetMainURL().Len() > ( aRelURL2.GetMainURL().Len() + 1 ) ) )
                {
                    aPath = pObj->aURL.GetMainURL();
                    aPath = aPath.Erase( 0, aRelURL2.GetMainURL().Len() );
                }
                else
                    aPath = pObj->aURL.GetMainURL();
            }
        }

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

        for( pObj = aObjectList.First(); pObj; pObj = aObjectList.Next() )
            delete pObj;

        aObjectList.Clear();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            pObj = new GalleryObject;

            ByteString  aTmpStr;
            String      aFileName;
            String      aPath;
            sal_uInt16  nTemp;

            rIStm >> bRel >> aTmpStr >> pObj->nOffset;
            rIStm >> nTemp; pObj->eObjKind = (SgaObjKind) nTemp;

            aFileName = String( aTmpStr.GetBuffer(), gsl_getSystemTextEncoding() );

            if( bRel )
            {
                aFileName.SearchAndReplaceAll( '\\', '/' );
                aPath = aRelURL1.GetMainURL();

                if( aFileName.GetChar( 0 ) != '/' )
                    aPath += '/';

                aPath += aFileName;

                pObj->aURL = INetURLObject( aPath );

                if( !FileExists( pObj->aURL ) )
                {
                    aPath = aRelURL2.GetMainURL();

                    if( aFileName.GetChar( 0 ) != '/' )
                        aPath += '/';

                    aPath += aFileName;

                    pObj->aURL = INetURLObject( aPath );

                    if( !FileExists( pObj->aURL ) )
                        pObj->aURL = INetURLObject();
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
                    pObj->aURL = INetURLObject( aFileName );

                    if( ( pObj->aURL.GetProtocol() == INET_PROT_NOT_VALID ) &&
                        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aFileName ) )
                    {
                        pObj->aURL = INetURLObject( aFileName );
                    }
                }
            }

            aObjectList.Insert( pObj, LIST_APPEND );
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
