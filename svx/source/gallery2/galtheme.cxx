/*************************************************************************
 *
 *  $RCSfile: galtheme.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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
#include <tools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <so3/svstor.hxx>
#include <sot/formats.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <svtools/itempool.hxx>
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
    if( pThm->IsImported() )
    {
        aSvDrawStorageRef = new SvStorage();
        aImportName = pThm->GetThemeName();
    }
    else
        aSvDrawStorageRef = new SvStorage( GetSdvPath(), pThm->IsReadOnly() ? STREAM_READ : STREAM_STD_READWRITE );
}

// ------------------------------------------------------------------------

GalleryTheme::~GalleryTheme()
{
    ImplWrite();

    for( GalleryObject* pEntry = aObjectList.First(); pEntry; pEntry = aObjectList.Next() )
        delete pEntry;
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ImplWriteSgaObject( const SgaObject& rObj, ULONG nPos, GalleryObject* pExistentEntry )
{
    SvFileStream    aOStm;
    BOOL            bRet = FALSE;

    aOStm.Open( GetSdgPath(), STREAM_WRITE );

    if( aOStm.IsOpen() )
    {
        const sal_uInt32 nOffset = aOStm.Seek( STREAM_SEEK_TO_END );

        aOStm << rObj;

        if( !aOStm.GetError() )
        {
            GalleryObject* pEntry;

            if( !pExistentEntry )
            {
                pEntry = new GalleryObject;
                aObjectList.Insert( pEntry, nPos );
            }
            else
                pEntry = pExistentEntry;

            pEntry->aPath = rObj.GetPath();
            pEntry->nOffset = nOffset;
            pEntry->eObjKind = rObj.GetObjKind();
            bRet = TRUE;
        }

        aOStm.Close();
    }

    return bRet;
}

// ------------------------------------------------------------------------

SgaObject* GalleryTheme::ImplReadSgaObject( GalleryObject* pEntry )
{
    SgaObject* pSgaObj = NULL;

    if( pEntry )
    {
        SvFileStream aIStm( GetSdgPath(), STREAM_READ );

        if( aIStm.IsOpen() )
        {
            sal_uInt32 nInventor;

            // Ueberpruefen, ob das File ein gueltiges SGA-File ist
            aIStm.Seek( pEntry->nOffset );
            aIStm >> nInventor;

            if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
            {
                aIStm.Seek( pEntry->nOffset );

                switch( pEntry->eObjKind )
                {
                    case ( SGA_OBJ_BMP ) :
                    {
                        SgaObjectBmp* pObj = new SgaObjectBmp();
                        aIStm >> *pObj;
                        pSgaObj = (SgaObject*) pObj;
                    }
                    break;

                    case ( SGA_OBJ_ANIM ) :
                    {
                        SgaObjectAnim* pObj = new SgaObjectAnim();
                        aIStm >> *pObj;
                        pSgaObj = (SgaObject*) pObj;
                    }
                    break;

                    case ( SGA_OBJ_INET ) :
                    {
                        SgaObjectINet* pObj = new SgaObjectINet();
                        aIStm >> *pObj;
                        pSgaObj = (SgaObject*) pObj;
                    }
                    break;

                    case ( SGA_OBJ_SVDRAW ) :
                    {
                        SgaObjectSvDraw* pObj = new SgaObjectSvDraw();
                        aIStm >> *pObj;
                        pSgaObj = (SgaObject*) pObj;
                    }
                    break;

                    case ( SGA_OBJ_SOUND ) :
                    {
                        SgaObjectSound* pObj = new SgaObjectSound();
                        aIStm >> *pObj;
                        pSgaObj = (SgaObject*) pObj;
                    }
                    break;

                    default:
                    break;
                }
            }
        }
    }

    return pSgaObj;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplRead()
{
    SvFileStream aIStm( GetThmPath(), STREAM_READ );

    if( aIStm.IsOpen() )
        aIStm >> *this;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplWrite()
{
    if( IsModified() )
    {
        const INetURLObject aFileName( GetThmPath(), INET_PROT_FILE );
        const INetURLObject aFilePath( aFileName.GetPath(), INET_PROT_FILE );

        if( FileExists( aFilePath ) || CreateDir( aFilePath ) )
        {
#ifdef UNX
            SvFileStream aOStm( aFileName.PathToFileName(), STREAM_WRITE | STREAM_COPY_ON_SYMLINK | STREAM_TRUNC );
#else
            SvFileStream aOStm( aFileName.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );
#endif

            if( aOStm.IsOpen() )
                aOStm << *this;

            ImplSetModified( FALSE );
        }
    }
}

// ------------------------------------------------------------------------

String GalleryTheme::ImplGetPathToFile( const GalleryObject* pObject ) const
{
    String aFilePath;

    if( pObject )
    {
        if( IsImported() )
        {
            INetURLObject aPath( GetParent()->GetImportPath( GetName() ), INET_PROT_FILE );

            aPath.removeSegment();
            aPath.Append( INetURLObject( pObject->aPath, INET_PROT_FILE ).GetName() );
            aFilePath = aPath.PathToFileName();
        }
        else
            aFilePath = pObject->aPath;
    }

    return aFilePath;
}

// ------------------------------------------------------------------------

void GalleryTheme::ImplBroadcast( ULONG nUpdatePos )
{
    if( !IsBroadcasterLocked() )
        Broadcast( GalleryHint( GALLERY_HINT_THEME_UPDATEVIEW, GetName(), nUpdatePos ) );
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
            if( pEntry->aPath == rObj.GetPath() )
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
        KillFile( INetURLObject( GetSdgPath(), INET_PROT_FILE ) );

    if( pEntry )
    {
        if( SGA_OBJ_SVDRAW == pEntry->eObjKind )
            aSvDrawStorageRef->Remove( pEntry->aPath );

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
            ImplBroadcast( nNewPos );
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
            aActualizeFile = pEntry->aPath;
            rActualizeLink.Call( this );

            // SvDraw-Objekte werden spaeter aktualisiert
            if( pEntry->eObjKind != SGA_OBJ_SVDRAW )
            {
                // Hier muss noch etwas eingebaut werden,
                // das Files auf den ensprechenden Eintrag matched
                // Grafiken als Grafik-Objekte in die Gallery aufnehmen
#ifndef MAC
                if( INetURLObject( pEntry->aPath, INET_PROT_FILE ).GetExtension().CompareIgnoreCaseToAscii( "wav" ) != COMPARE_EQUAL )
#else
                if( TRUE )
#endif
                {
                    // vermeiden, dass ein native Link angelegt wird,
                    // da dies an dieser Stelle nicht noetig ist
                    aGraphic.Clear();
                    aGraphic.SetLink( GfxLink() );

                    if ( SGAImport( aActualizeFile, aGraphic, aFormat ) )
                    {
                        SgaObject* pNewObj;

                        if ( SGA_OBJ_INET == pEntry->eObjKind )
                            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, aActualizeFile, aFormat );
                        else if ( aGraphic.IsAnimated() )
                            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, aActualizeFile, aFormat );
                        else
                            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, aActualizeFile, aFormat );

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
                    SgaObjectSound aObjSound( aActualizeFile );
                    if( !InsertObject( aObjSound ) )
                        pEntry->bDummy = TRUE;
                }
            }
            else
            {
                INetURLObject aURL( pEntry->aPath, INET_PROT_FILE );

                if ( aSvDrawStorageRef.Is() )
                {
                    SvStorageStreamRef pIStm = aSvDrawStorageRef->OpenStream( aURL.GetBase(), STREAM_READ );

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
                                pIStm = aSvDrawStorageRef->OpenStream( aURL.GetBase(), STREAM_WRITE | STREAM_TRUNC );
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

        // alle Eintraege mit gesetztem LoeschFlag
        // aus der ObjektListe entfernen
        pEntry = aObjectList.First();
        while( pEntry )
        {
            if ( pEntry->bDummy )
            {
                delete aObjectList.Remove( pEntry );
                pEntry = aObjectList.GetCurObject();
            }
            else
                pEntry = aObjectList.Next();
        }

        // Thema komplett neu aufbauen
        INetURLObject   aTempFile( TempFile::CreateTempName(), INET_PROT_FILE );
        SvFileStream    aIStm( GetSdgPath(), STREAM_READ );
        SvFileStream    aOStm( aTempFile.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );

        pEntry = aObjectList.First();

        while( pEntry )
        {
            switch( pEntry->eObjKind )
            {
                case( SGA_OBJ_BMP ):
                {
                    SgaObjectBmp* pObj = new SgaObjectBmp();
                    aIStm.Seek( pEntry->nOffset );
                    aIStm >> *pObj;
                    pEntry->nOffset = aOStm.Tell();
                    aOStm << *pObj;
                    delete pObj;
                }
                break;

                case( SGA_OBJ_ANIM ):
                {
                    SgaObjectAnim* pObj = new SgaObjectAnim();
                    aIStm.Seek( pEntry->nOffset );
                    aIStm >> *pObj;
                    pEntry->nOffset = aOStm.Tell();
                    aOStm << *pObj;
                    delete pObj;
                }
                break;

                case( SGA_OBJ_INET ) :
                {
                    SgaObjectINet* pObj = new SgaObjectINet();
                    aIStm.Seek( pEntry->nOffset );
                    aIStm >> *pObj;
                    pEntry->nOffset = aOStm.Tell();
                    aOStm << *pObj;
                    delete pObj;
                }
                break;

                case( SGA_OBJ_SVDRAW ) :
                {
                    SgaObjectSvDraw* pObj = new SgaObjectSvDraw();
                    aIStm.Seek( pEntry->nOffset );
                    aIStm >> *pObj;
                    pEntry->nOffset = aOStm.Tell();
                    aOStm << *pObj;
                    delete pObj;
                }
                break;

                case (SGA_OBJ_SOUND) :
                {
                    SgaObjectSound* pObj = new SgaObjectSound();
                    aIStm.Seek( pEntry->nOffset );
                    aIStm >> *pObj;
                    pEntry->nOffset = aOStm.Tell();
                    aOStm << *pObj;
                    delete pObj;
                }
                break;

                default:
                break;
            }

            pEntry = aObjectList.Next();
        }

        aIStm.Close();
        aOStm.Close();

        CopyFile( aTempFile, INetURLObject( GetSdgPath(), INET_PROT_FILE ) );
        KillFile( aTempFile );

        // storage updaten (kopieren)
        SvStorageRef aTempStorageRef( new SvStorage( aTempFile.PathToFileName() ) );
        aSvDrawStorageRef->CopyTo( aTempStorageRef );

        if( !aSvDrawStorageRef->GetError() )
        {
            aSvDrawStorageRef.Clear();
            aTempStorageRef.Clear();
            CopyFile( aTempFile, INetURLObject( GetSdvPath(), INET_PROT_FILE ) );
            aSvDrawStorageRef = new SvStorage( GetSdvPath() );
        }

        KillFile( aTempFile );
        ImplSetModified( TRUE );
        ImplWrite();
        UnlockBroadcaster();
    }
}

// ------------------------------------------------------------------------

GalleryThemeEntry* GalleryTheme::CreateThemeEntry( const String& rFile, BOOL bReadOnly )
{
    GalleryThemeEntry* pRet = NULL;

    if( FileExists( INetURLObject( rFile, INET_PROT_FILE ) ) )
    {
        SvFileStream    aThmStm( rFile, STREAM_READ );
        String          aThemeName;
        sal_uInt32      nThemeId = 0;
        sal_uInt16      nVersion;
        BOOL            bThemeNameFromResource = FALSE;

        aThmStm >> nVersion;

        if( nVersion <= 0x00ff )
        {
            ByteString aTmpStr;

            aThmStm >> aTmpStr; aThemeName = String( aTmpStr.GetBuffer(), RTL_TEXTENCODING_UTF8 );

            // Charakterkonvertierung durchfuehren
            if( nVersion >= 0x0004 )
            {
                sal_uInt32  nCount;
                sal_uInt16  nTemp16;

                aThmStm >> nCount >> nTemp16;
                aThmStm.Seek( STREAM_SEEK_TO_END );

                // pruefen, ob es sich um eine neuere Version handelt;
                // daher um 520Bytes (8Bytes Kennung + 512Bytes Reserverpuffer ) zurueckspringen,
                // falls dies ueberhaupt moeglich ist
                if( aThmStm.Tell() >= 520 )
                {
                    ULONG nId1, nId2;

                    aThmStm.SeekRel( -520 );
                    aThmStm >> nId1 >> nId2;

                    if( nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
                        nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
                    {
                        VersionCompat* pCompat = new VersionCompat( aThmStm, STREAM_READ );

                        aThmStm >> nThemeId;

                        if( pCompat->GetVersion() >= 2 )
                        {
                            aThmStm >> bThemeNameFromResource;
                        }

                        delete pCompat;
                    }
                }
            }

            INetURLObject aFileObj( rFile, INET_PROT_FILE );
            pRet = new GalleryThemeEntry( aFileObj.GetPath(), aThemeName,
                                          aFileObj.GetBase().Copy( 2, 6 ).ToInt32(),
                                          bReadOnly, FALSE, FALSE, nThemeId,
                                          bThemeNameFromResource );
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
        const String aPath( ImplGetPathToFile( pObject ) );

        switch( pObject->eObjKind )
        {
            case( SGA_OBJ_BMP ):
            case( SGA_OBJ_ANIM ):
            case( SGA_OBJ_INET ):
            {
                String aFilterDummy;
                bRet = ( SGAImport( aPath, rGraphic, aFilterDummy, bProgress ) != SGA_IMPORT_NONE );
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
                    rGraphic = BitmapEx( pObj->GetThumbBmp(), COL_LIGHTMAGENTA );
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
        INetURLObject   aURL( CreateUniqueFileName( GetParent(), SGA_OBJ_BMP ) );
        SvFileStream    aOStm;

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

            aOStm.Open( aURL.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );

            if( aOStm.IsOpen() )
            {
                aOStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
                bRet = ( GraphicConverter::Export( aOStm, rGraphic, nExportFormat ) == ERRCODE_NONE );
                aOStm.Close();
            }
        }
        else
        {
            aURL.SetExtension( String( RTL_CONSTASCII_USTRINGPARAM( "svm" ) ) );
            aOStm.Open( aURL.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );

            if( aOStm.IsOpen() )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );
                aOStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
                aMtf.Write( aOStm );
                bRet = ( aOStm.GetError() == ERRCODE_NONE );
                aOStm.Close();
            }
        }

        if( bRet )
        {
            const SgaObjectBmp aObjBmp( aURL.PathToFileName() );
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
        const INetURLObject aFile( ImplGetPathToFile( pObject ), INET_PROT_FILE );
        SvStorageRef        xStor( GetSvDrawStorage() );

        if( xStor.Is() )
        {
            SvStorageStreamRef xIStm( xStor->OpenStream( aFile.GetBase(), STREAM_READ ) );

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
    INetURLObject   aURL( CreateUniqueFileName( GetParent(), SGA_OBJ_SVDRAW ) );
    SvStorageRef    xStor( GetSvDrawStorage() );
    BOOL            bRet = FALSE;

    if( xStor.Is() )
    {
        SvStorageStreamRef xOStm( xStor->OpenStream( aURL.GetBase(), STREAM_WRITE | STREAM_TRUNC ) );

        if( xOStm.Is() && !xOStm->GetError() )
        {
            SvMemoryStream  aMemStm( 65535, 65535 );
            RLECodec        aCodec( *xOStm );

            aMemStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
            ( (FmFormModel&) rModel ).SetStreamingSdrModel( TRUE );
            rModel.GetItemPool().Store( aMemStm );
            aMemStm << rModel;
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
        rURL = INetURLObject( ImplGetPathToFile( pObject ), INET_PROT_FILE );
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
    const String    aFileName( rURL.PathToFileName() );
    const USHORT    nImportRet = SGAImport( aFileName, aGraphic, aFormat );
    BOOL            bRet = FALSE;

    aGraphic.SetLink( GfxLink() );

    if( nImportRet != SGA_IMPORT_NONE )
    {
        if ( SGA_IMPORT_INET == nImportRet )
            pNewObj = (SgaObject*) new SgaObjectINet( aGraphic, aFileName, aFormat );
        else if ( aGraphic.IsAnimated() )
            pNewObj = (SgaObject*) new SgaObjectAnim( aGraphic, aFileName, aFormat );
        else
            pNewObj = (SgaObject*) new SgaObjectBmp( aGraphic, aFileName, aFormat );
    }
    else if( SGAIsSoundFile( aFileName ) )
        pNewObj = (SgaObject*) new SgaObjectSound( aFileName );

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
                SvStorageStreamRef xIStm( xStor->OpenStream( INetURLObject( GetObjectPath( nPos ), INET_PROT_FILE ).GetBase(), STREAM_READ ) );

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
                    pData->SetData( GetObjectPath( nPos ) );
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
                String aFile;

                if( aData.GetData( aFile ) )
                {
                    try
                    {
                        INetURLObject aURL( aFile, INET_PROT_FILE );

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
                                        aURL.SetSmartURL( xContentAccess->queryContentIdentfierString() );
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
    const String    aRelParent1( GetParent()->GetRelativePath() );
    const String    aRelParent2( GetParent()->GetUserPath() );
    String          aNewPath, aTemp;
    sal_uInt32      nCount = GetObjectCount();
    BOOL            bRel;

    rOStm << (sal_uInt16) 0x0004;
    rOStm << ByteString( GetRealName(), RTL_TEXTENCODING_UTF8 );
    rOStm << nCount << (sal_uInt16) gsl_getSystemTextEncoding();

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        const GalleryObject* pObj = ImplGetGalleryObject( i );

        aTemp = pObj->aPath;
        bRel = ( ( aTemp.Erase( aRelParent1.Len() ) ) == aRelParent1 );

        if( bRel && ( pObj->aPath.Len() > ( aRelParent1.Len() + 1 ) ) )
        {
            aTemp = pObj->aPath;
            aNewPath = aTemp.Erase( 0, aRelParent1.Len() + 1 );
        }
        else
        {
            aTemp = pObj->aPath;
            bRel = ( ( aTemp.Erase( aRelParent2.Len() ) ) == aRelParent2 );

            if( bRel && ( pObj->aPath.Len() > ( aRelParent2.Len() + 1 ) ) )
            {
                aTemp = pObj->aPath;
                aNewPath = aTemp.Erase( 0, aRelParent2.Len() + 1 );
            }
            else
                aNewPath = pObj->aPath;
        }

        rOStm << bRel << ByteString( aNewPath, RTL_TEXTENCODING_UTF8 );
        rOStm << pObj->nOffset << (sal_uInt16) pObj->eObjKind;
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
    static char         cSystemDelimiter = 0;

    if( !cSystemDelimiter )
    {
#ifdef MAC
        cSystemDelimiter = ':';
#elif defined WIN || defined WNT || defined OS2
        cSystemDelimiter = '\\';
#else
        cSystemDelimiter = '/';
#endif
    }

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
        String          aRelParent1( GetParent()->GetRelativePath() ), aRelParent2( GetParent()->GetUserPath() );
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
            aFileName.SearchAndReplaceAll( '\\', cSystemDelimiter );

            if( bRel )
            {
                aPath = aRelParent1;

                if( aFileName.GetChar( 0 ) != cSystemDelimiter )
                    aPath += cSystemDelimiter;

                aPath += aFileName;

                if( FileExists( INetURLObject( aPath, INET_PROT_FILE ) ) )
                    pObj->aPath = aPath;
                else
                {
                    aPath = aRelParent2;

                    if( aFileName.GetChar( 0 ) != cSystemDelimiter )
                        aPath += cSystemDelimiter;

                    aPath += aFileName;

                    if( FileExists( INetURLObject( aPath, INET_PROT_FILE ) ) )
                        pObj->aPath = aPath;
                }
            }
            else
                pObj->aPath = aFileName;

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
