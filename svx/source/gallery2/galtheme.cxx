/*************************************************************************
 *
 *  $RCSfile: galtheme.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:03:34 $
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
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <so3/svstor.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/docfile.hxx>
#include <avmedia/mediawindow.hxx>
#include "svdograf.hxx"
#include "fmpage.hxx"
#include "codec.hxx"
#include "unomodel.hxx"
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
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
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
        aSvDrawStorageRef = new SvStorage( FALSE, GetSdvURL().GetMainURL( INetURLObject::NO_DECODE ), pThm->IsReadOnly() ? STREAM_READ : STREAM_STD_READWRITE );
    else
        aSvDrawStorageRef.Clear();
}

// ------------------------------------------------------------------------

BOOL GalleryTheme::ImplWriteSgaObject( const SgaObject& rObj, ULONG nPos, GalleryObject* pExistentEntry )
{
    SvStream*   pOStm = ::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );
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
    sal_uInt32      nNextNumber;
    sal_Char*       pExt;
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
    else
        nNextNumber = 1999;

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

            for( GalleryObject* pEntry = aObjectList.First(); pEntry && !bExists; pEntry = aObjectList.Next() )
                if( pEntry->aURL == aNewURL )
                    bExists = TRUE;
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
        GalleryObject*  pEntry = aObjectList.First();
        GalleryObject*  pFoundEntry = NULL;
        ULONG           nUpdatePos = LIST_APPEND;

        for( ; pEntry && !pFoundEntry; pEntry = aObjectList.Next() )
            if( pEntry->aURL == rObj.GetURL() )
                pFoundEntry = pEntry;

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
            aSvDrawStorageRef->Remove( pEntry->aURL.GetMainURL( INetURLObject::NO_DECODE ) );

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
                    SvStorageStreamRef  pIStm = aSvDrawStorageRef->OpenStream( aStmName, STREAM_READ );

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

        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aInURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
        SvStream* pTmpStm = ::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );

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
                bRet = ( GalleryGraphicImport( aURL, rGraphic, aFilterDummy, bProgress ) != SGA_IMPORT_NONE );
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
                        rGraphic = aView.GetAllMarkedGraphic();
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
                if( aGfxLink.GetDataSize() )
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
        SvStorageStreamRef  xOStm( xStor->OpenStream( aStmName, STREAM_WRITE | STREAM_TRUNC ) );

        if( xOStm.Is() && !xOStm->GetError() )
        {
            SvMemoryStream  aMemStm( 65535, 65535 );
            FmFormModel*    pFormModel = (FmFormModel*) &rModel;

            pFormModel->BurnInStyleSheetAttributes();
            pFormModel->SetStreamingSdrModel( TRUE );
            pFormModel->RemoveNotPersistentObjects( TRUE );

            {
                com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xDocOut( new utl::OOutputStreamWrapper( aMemStm ) );

                if( xDocOut.is() )
                    SvxDrawingLayerExport( pFormModel, xDocOut );
            }

            pFormModel->SetStreamingSdrModel( FALSE );
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

BOOL GalleryTheme::GetModelStream( ULONG nPos, SotStorageStreamRef& rxModelStream, BOOL bProgress )
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
                UINT32 nVersion = 0;

                xIStm->SetBufferSize( 16348 );

                if( GalleryCodec::IsCoded( *xIStm, nVersion ) )
                {
                    if( 1 == nVersion )
                    {
                        FmFormModel aModel;

                        aModel.GetItemPool().FreezeIdRanges();

                        if( GallerySvDrawImport( *xIStm, aModel ) )
                        {
                            aModel.BurnInStyleSheetAttributes();
                            aModel.SetStreamingSdrModel( TRUE );
                            aModel.RemoveNotPersistentObjects( TRUE );

                            {
                                com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxModelStream ) );

                                if( SvxDrawingLayerExport( &aModel, xDocOut ) )
                                    rxModelStream->Commit();
                            }

                            aModel.SetStreamingSdrModel( FALSE );
                        }
                    }
                    else if( 2 == nVersion )
                    {
                        GalleryCodec aCodec( *xIStm );
                        aCodec.Read( *rxModelStream );
                    }

                    bRet = ( rxModelStream->GetError() == ERRCODE_NONE );
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
        SvStorageStreamRef  xOStm( xStor->OpenStream( aStmName, STREAM_WRITE | STREAM_TRUNC ) );

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
        Content         aCnt( rFileOrDirURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< XCommandEnvironment >() );
        sal_Bool        bFolder;

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
                        aURL.SetSmartURL( xContentAccess->queryContentIdentifierString() );
                        aURLVector.push_back( aURL );
                    }
                }
            }
        }
        else
            aURLVector.push_back( rFileOrDirURL );
    }
    catch( const ContentCreationException& )
    {
    }
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    ::std::vector< INetURLObject >::const_iterator aIter( aURLVector.begin() ), aEnd( aURLVector.end() );

    while( aIter != aEnd )
        bRet = bRet || InsertURL( *aIter++, nInsertPos );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryTheme::InsertTransferable( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable, ULONG nInsertPos )
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

                if( aDataHelper.GetImageMap( SOT_FORMATSTR_ID_SVIM, aImageMap ) )
                {
                    FmFormModel         aModel;
                    SgaUserDataFactory  aFactory;

                    aModel.GetItemPool().FreezeIdRanges();

                    SdrPage*    pPage = aModel.AllocPage( FALSE );
                    SdrGrafObj* pGrafObj = new SdrGrafObj( *pGraphic );

                    pGrafObj->InsertUserData( new SgaIMapInfo( aImageMap ) );
                    pPage->InsertObject( pGrafObj );
                    aModel.SetPageNotValid( TRUE );
                    aModel.InsertPage( pPage );
                    bRet = InsertModel( aModel, nInsertPos );
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
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos );
    pTransferable->CopyToClipboard( pWindow );
}

// -----------------------------------------------------------------------------

void GalleryTheme::StartDrag( Window* pWindow, ULONG nPos )
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos );
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
            bRel = ( ( aPath.Erase( aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).Len() ) ) == aRelURL1.GetMainURL( INetURLObject::NO_DECODE ) );

            if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::NO_DECODE ).Len() > ( aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).Len() + 1 ) ) )
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
                aPath = aPath.Erase( 0, aRelURL1.GetMainURL( INetURLObject::NO_DECODE ).Len() );
            }
            else
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );

                bRel = ( ( aPath.Erase( aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).Len() ) ) == aRelURL2.GetMainURL( INetURLObject::NO_DECODE ) );

                if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::NO_DECODE ).Len() > ( aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).Len() + 1 ) ) )
                {
                    aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
                    aPath = aPath.Erase( 0, aRelURL2.GetMainURL( INetURLObject::NO_DECODE ).Len() );
                }
                else
                    aPath = pObj->aURL.GetMainURL( INetURLObject::NO_DECODE );
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
