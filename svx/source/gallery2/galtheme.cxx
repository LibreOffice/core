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

#include <config_features.h>

#include "sal/config.h"

#include <algorithm>

#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
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
#include "svx/galtheme.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include "galobj.hxx"
#include <svx/gallery1.hxx>
#include "galtheme.hrc"
#include <vcl/lstbox.hxx>
#include "gallerydrawmodel.hxx"
#include <memory>

using namespace ::com::sun::star;


GalleryTheme::GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry )
    : m_bDestDirRelative(false)
    , pParent(pGallery)
    , pThm(pThemeEntry)
    , mnThemeLockCount(0)
    , mnBroadcasterLockCount(0)
    , nDragPos(0)
    , bDragging(false)
    , bAbortActualize(false)
{
    ImplCreateSvDrawStorage();
}

GalleryTheme::~GalleryTheme()
{
    ImplWrite();

    for (GalleryObject* pEntry : aObjectList)
    {
        Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), reinterpret_cast< sal_uIntPtr >( pEntry ) ) );
        Broadcast( GalleryHint( GalleryHintType::OBJECT_REMOVED, GetName(), reinterpret_cast< sal_uIntPtr >( pEntry ) ) );
        delete pEntry;
    }
    aObjectList.clear();

}

void GalleryTheme::ImplCreateSvDrawStorage()
{
    try
    {
        aSvDrawStorageRef = new SotStorage( false, GetSdvURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), pThm->IsReadOnly() ? StreamMode::READ : StreamMode::STD_READWRITE );
        // #i50423# ReadOnly may not been set though the file can't be written (because of security reasons)
        if ( ( aSvDrawStorageRef->GetError() != ERRCODE_NONE ) && !pThm->IsReadOnly() )
            aSvDrawStorageRef = new SotStorage( false, GetSdvURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ );
    }
    catch (const css::ucb::ContentCreationException& e)
    {
        SAL_WARN("svx", "failed to open: "
                  << GetSdvURL().GetMainURL(INetURLObject::DecodeMechanism::NONE)
                  << "due to : " << e.Message);
    }
}

bool GalleryTheme::ImplWriteSgaObject( const SgaObject& rObj, size_t nPos, GalleryObject* pExistentEntry )
{
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE ));
    bool        bRet = false;

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
            bRet = true;
        }
    }

    return bRet;
}

SgaObject* GalleryTheme::ImplReadSgaObject( GalleryObject* pEntry )
{
    SgaObject* pSgaObj = nullptr;

    if( pEntry )
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( GetSdgURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

        if( pIStm )
        {
            sal_uInt32 nInventor;

            // Check to ensure that the file is a valid SGA file
            pIStm->Seek( pEntry->nOffset );
            pIStm->ReadUInt32( nInventor );

            if( nInventor == COMPAT_FORMAT( 'S', 'G', 'A', '3' ) )
            {
                pIStm->Seek( pEntry->nOffset );

                switch( pEntry->eObjKind )
                {
                    case SgaObjKind::Bitmap:    pSgaObj = new SgaObjectBmp(); break;
                    case SgaObjKind::Animation:   pSgaObj = new SgaObjectAnim(); break;
                    case SgaObjKind::Inet:   pSgaObj = new SgaObjectINet(); break;
                    case SgaObjKind::SvDraw: pSgaObj = new SgaObjectSvDraw(); break;
                    case SgaObjKind::Sound:  pSgaObj = new SgaObjectSound(); break;

                    default:
                    break;
                }

                if( pSgaObj )
                {
                    ReadSgaObject( *pIStm, *pSgaObj );
                    pSgaObj->ImplUpdateURL( pEntry->aURL );
                }
            }
        }
    }

    return pSgaObj;
}

void GalleryTheme::ImplWrite()
{
    if( pThm->IsModified() )
    {
        INetURLObject aPathURL( GetThmURL() );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        DBG_ASSERT( aPathURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

        if( FileExists( aPathURL ) || CreateDir( aPathURL ) )
        {
#ifdef UNX
            std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::COPY_ON_SYMLINK | StreamMode::TRUNC ));
#else
            std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( GetThmURL().GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC ));
#endif

            if( pOStm )
            {
                WriteGalleryTheme( *pOStm, *this );
                pOStm.reset();
            }

            ImplSetModified( false );
        }
    }
}

const GalleryObject* GalleryTheme::ImplGetGalleryObject( const INetURLObject& rURL )
{
    for (GalleryObject* i : aObjectList)
        if ( i->aURL == rURL )
            return i;
    return nullptr;
}

INetURLObject GalleryTheme::ImplGetURL( const GalleryObject* pObject )
{
    INetURLObject aURL;

    if( pObject )
        aURL = pObject->aURL;

    return aURL;
}

INetURLObject GalleryTheme::ImplCreateUniqueURL( SgaObjKind eObjKind, ConvertDataFormat nFormat )
{
    INetURLObject   aDir( GetParent()->GetUserURL() );
    INetURLObject   aInfoFileURL( GetParent()->GetUserURL() );
    INetURLObject   aNewURL;
    sal_uInt32      nNextNumber = 1999;
    sal_Char const* pExt = nullptr;
    bool            bExists;

    aDir.Append( "dragdrop" );
    CreateDir( aDir );

    aInfoFileURL.Append( "sdddndx1" );

    // read next possible number
    if( FileExists( aInfoFileURL ) )
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aInfoFileURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

        if( pIStm )
        {
            pIStm->ReadUInt32( nNextNumber );
        }
    }

    // create extension
    if( nFormat != ConvertDataFormat::Unknown )
    {
        switch( nFormat )
        {
            case ConvertDataFormat::BMP: pExt = ".bmp"; break;
            case ConvertDataFormat::GIF: pExt = ".gif"; break;
            case ConvertDataFormat::JPG: pExt = ".jpg"; break;
            case ConvertDataFormat::MET: pExt = ".met"; break;
            case ConvertDataFormat::PCT: pExt = ".pct"; break;
            case ConvertDataFormat::PNG: pExt = ".png"; break;
            case ConvertDataFormat::SVM: pExt = ".svm"; break;
            case ConvertDataFormat::TIF: pExt = ".tif"; break;
            case ConvertDataFormat::WMF: pExt = ".wmf"; break;
            case ConvertDataFormat::EMF: pExt = ".emf"; break;

            default:
                pExt = ".grf";
            break;
        }
    }

    do
    {
        // get URL
        if( SgaObjKind::SvDraw == eObjKind )
        {
            OUString aFileName( "gallery/svdraw/dd" );
            aNewURL = INetURLObject( aFileName += OUString::number( ++nNextNumber % 99999999 ), INetProtocol::PrivSoffice );

            bExists = false;

            for (GalleryObject* p : aObjectList)
                if ( p->aURL == aNewURL )
                {
                    bExists = true;
                    break;
                }
        }
        else
        {
            OUString aFileName( "dd" );

            aFileName += OUString::number( ++nNextNumber % 999999 );

            if (pExt)
                aFileName += OUString( pExt, strlen(pExt), RTL_TEXTENCODING_ASCII_US );

            aNewURL = aDir;
            aNewURL.Append( aFileName );

            bExists = FileExists( aNewURL );
        }
    }
    while( bExists );

    // write updated number
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( aInfoFileURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE ));

    if( pOStm )
    {
        pOStm->WriteUInt32( nNextNumber );
    }

    return aNewURL;
}

void GalleryTheme::ImplBroadcast( sal_uIntPtr nUpdatePos )
{
    if( !IsBroadcasterLocked() )
    {
        if( GetObjectCount() && ( nUpdatePos >= GetObjectCount() ) )
            nUpdatePos = GetObjectCount() - 1;

        Broadcast( GalleryHint( GalleryHintType::THEME_UPDATEVIEW, GetName(), nUpdatePos ) );
    }
}

bool GalleryTheme::UnlockTheme()
{
    DBG_ASSERT( mnThemeLockCount, "Theme is not locked" );

    bool bRet = false;

    if( mnThemeLockCount )
    {
        --mnThemeLockCount;
        bRet = true;
    }

    return bRet;
}

void GalleryTheme::UnlockBroadcaster()
{
    DBG_ASSERT( mnBroadcasterLockCount, "Broadcaster is not locked" );

    if( mnBroadcasterLockCount && !--mnBroadcasterLockCount )
        ImplBroadcast( 0 );
}

bool GalleryTheme::InsertObject( const SgaObject& rObj, sal_uIntPtr nInsertPos )
{
    if (!rObj.IsValid())
        return false;

    GalleryObject* pFoundEntry = nullptr;
    size_t iFoundPos = 0;
    for (size_t n = aObjectList.size(); iFoundPos < n; ++iFoundPos)
    {
        if (aObjectList[ iFoundPos ]->aURL == rObj.GetURL())
        {
            pFoundEntry = aObjectList[ iFoundPos ];
            break;
        }
    }

    if (pFoundEntry)
    {
        GalleryObject aNewEntry;

        // update title of new object if necessary
        if (rObj.GetTitle().isEmpty())
        {
            std::unique_ptr<SgaObject> pOldObj(ImplReadSgaObject(pFoundEntry));

            if (pOldObj)
            {
                ((SgaObject&) rObj).SetTitle( pOldObj->GetTitle() );
            }
        }
        else if (rObj.GetTitle() == "__<empty>__")
            ((SgaObject&) rObj).SetTitle("");

        ImplWriteSgaObject(rObj, nInsertPos, &aNewEntry);
        pFoundEntry->nOffset = aNewEntry.nOffset;
    }
    else
        ImplWriteSgaObject(rObj, nInsertPos, nullptr);

    ImplSetModified(true);
    ImplBroadcast(pFoundEntry? iFoundPos: nInsertPos);

    return true;
}

SgaObject* GalleryTheme::AcquireObject( size_t nPos )
{
    return ImplReadSgaObject( aObjectList[ nPos ] );
}

void GalleryTheme::GetPreviewBitmapExAndStrings(sal_uIntPtr nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath) const
{
    const GalleryObject* pGalleryObject = nPos < aObjectList.size() ? aObjectList[ nPos ] : nullptr;

    if(pGalleryObject)
    {
        rBitmapEx = pGalleryObject->maPreviewBitmapEx;
        rSize = pGalleryObject->maPreparedSize;
        rTitle = pGalleryObject->maTitle;
        rPath = pGalleryObject->maPath;
    }
    else
    {
        OSL_ENSURE(false, "OOps, no GalleryObject at this index (!)");
    }
}

void GalleryTheme::SetPreviewBitmapExAndStrings(sal_uIntPtr nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath)
{
    GalleryObject* pGalleryObject = nPos < aObjectList.size() ? aObjectList[ nPos ] : nullptr;

    if(pGalleryObject)
    {
        pGalleryObject->maPreviewBitmapEx = rBitmapEx;
        pGalleryObject->maPreparedSize = rSize;
        pGalleryObject->maTitle = rTitle;
        pGalleryObject->maPath = rPath;
    }
    else
    {
        OSL_ENSURE(false, "OOps, no GalleryObject at this index (!)");
    }
}

void GalleryTheme::ReleaseObject( SgaObject* pObject )
{
    delete pObject;
}

bool GalleryTheme::RemoveObject( size_t nPos )
{
    GalleryObject* pEntry = nullptr;
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
        if( SgaObjKind::SvDraw == pEntry->eObjKind )
            aSvDrawStorageRef->Remove( pEntry->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), reinterpret_cast< sal_uIntPtr >( pEntry ) ) );
        Broadcast( GalleryHint( GalleryHintType::OBJECT_REMOVED, GetName(), reinterpret_cast< sal_uIntPtr >( pEntry ) ) );
        delete pEntry;
        pEntry = nullptr;

        ImplSetModified( true );
        ImplBroadcast( nPos );
    }

    return( pEntry != nullptr );
}

bool GalleryTheme::ChangeObjectPos( size_t nOldPos, size_t nNewPos )
{
    if (nOldPos == nNewPos || nOldPos >= aObjectList.size())
        return false;

    GalleryObject* pEntry = aObjectList[nOldPos];

    GalleryObjectList::iterator it = aObjectList.begin();
    ::std::advance(it, nNewPos);
    aObjectList.insert(it, pEntry);

    if (nNewPos < nOldPos)
        nOldPos++;

    it = aObjectList.begin();
    ::std::advance(it, nOldPos);
    aObjectList.erase(it);

    ImplSetModified(true);
    ImplBroadcast((nNewPos < nOldPos)? nNewPos: (nNewPos - 1));

    return true;
}

void GalleryTheme::Actualize( const Link<const INetURLObject&, void>& rActualizeLink, GalleryProgress* pProgress )
{
    if( !IsReadOnly() )
    {
        Graphic         aGraphic;
        OUString        aFormat;
        GalleryObject*  pEntry;
        const size_t    nCount = aObjectList.size();

        LockBroadcaster();
        bAbortActualize = false;

        // reset delete flag
        for (size_t i = 0; i < nCount; i++)
            aObjectList[ i ]->mbDelete = false;

        for(size_t i = 0; ( i < nCount ) && !bAbortActualize; i++)
        {
            if( pProgress )
                pProgress->Update( i, nCount - 1 );

            pEntry = aObjectList[ i ];

            const INetURLObject aURL( pEntry->aURL );

            rActualizeLink.Call( aURL );

            // SvDraw objects will be updated later
            if( pEntry->eObjKind != SgaObjKind::SvDraw )
            {
                // Still a function should be implemented,
                // which assigns files to the relevant entry.
                // insert graphics as graphic objects into the gallery
                if( pEntry->eObjKind == SgaObjKind::Sound )
                {
                    SgaObjectSound aObjSound( aURL );
                    if( !InsertObject( aObjSound ) )
                        pEntry->mbDelete = true;
                }
                else
                {
                    aGraphic.Clear();

                    if ( GalleryGraphicImport( aURL, aGraphic, aFormat ) != GalleryGraphicImportRet::IMPORT_NONE )
                    {
                        std::unique_ptr<SgaObject> pNewObj;

                        if ( SgaObjKind::Inet == pEntry->eObjKind )
                            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectINet( aGraphic, aURL, aFormat )));
                        else if ( aGraphic.IsAnimated() )
                            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectAnim( aGraphic, aURL, aFormat )));
                        else
                            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectBmp( aGraphic, aURL, aFormat )));

                        if( !InsertObject( *pNewObj ) )
                            pEntry->mbDelete = true;
                    }
                    else
                        pEntry->mbDelete = true; // set delete flag
                }
            }
            else
            {
                if ( aSvDrawStorageRef.is() )
                {
                    const OUString        aStmName( GetSvDrawStreamNameFromURL( pEntry->aURL ) );
                    tools::SvRef<SotStorageStream>  pIStm = aSvDrawStorageRef->OpenSotStream( aStmName, StreamMode::READ );

                    if( pIStm.is() && !pIStm->GetError() )
                    {
                        pIStm->SetBufferSize( 16384 );

                        SgaObjectSvDraw aNewObj( *pIStm, pEntry->aURL );

                        if( !InsertObject( aNewObj ) )
                            pEntry->mbDelete = true;

                        pIStm->SetBufferSize( 0L );
                    }
                }
            }
        }

        // remove all entries with set flag
        for ( GalleryObjectList::iterator it = aObjectList.begin(); it != aObjectList.end(); /* increment is in the body of loop */)
        {
            if( (*it)->mbDelete )
            {
                Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), reinterpret_cast< sal_uIntPtr >( *it ) ) );
                Broadcast( GalleryHint( GalleryHintType::OBJECT_REMOVED, GetName(), reinterpret_cast< sal_uLong >( *it ) ) );
                delete *it;
                it = aObjectList.erase( it );
            }
            else ++it;
        }

        // update theme
        ::utl::TempFile aTmp;
        INetURLObject   aInURL( GetSdgURL() );
        INetURLObject   aTmpURL( aTmp.GetURL() );

        DBG_ASSERT( aInURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        DBG_ASSERT( aTmpURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aInURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));
        std::unique_ptr<SvStream> pTmpStm(::utl::UcbStreamHelper::CreateStream( aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC ));

        if( pIStm && pTmpStm )
        {
            for (GalleryObject* i : aObjectList)
            {
                pEntry = i;
                std::unique_ptr<SgaObject> pObj;

                switch( pEntry->eObjKind )
                {
                case SgaObjKind::Bitmap:    pObj.reset(new SgaObjectBmp());      break;
                case SgaObjKind::Animation:   pObj.reset(new SgaObjectAnim());     break;
                case SgaObjKind::Inet:   pObj.reset(new SgaObjectINet());     break;
                case SgaObjKind::SvDraw: pObj.reset(new SgaObjectSvDraw());   break;
                case SgaObjKind::Sound:   pObj.reset(new SgaObjectSound());    break;

                    default:
                    break;
                }

                if( pObj )
                {
                    pIStm->Seek( pEntry->nOffset );
                    ReadSgaObject( *pIStm, *pObj);
                    pEntry->nOffset = pTmpStm->Tell();
                    WriteSgaObject( *pTmpStm, *pObj );
                }
            }
        }
        else
        {
            OSL_FAIL( "File(s) could not be opened" );
        }

        pIStm.reset();
        pTmpStm.reset();

        CopyFile( aTmpURL, aInURL );
        KillFile( aTmpURL );

        ErrCode nStorErr = ERRCODE_NONE;

        try
        {
            tools::SvRef<SotStorage> aTempStorageRef( new SotStorage( false, aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::STD_READWRITE ) );
            aSvDrawStorageRef->CopyTo( aTempStorageRef.get() );
            nStorErr = aSvDrawStorageRef->GetError();
        }
        catch (const css::ucb::ContentCreationException& e)
        {
            SAL_WARN("svx", "failed to open: "
                      << aTmpURL.GetMainURL(INetURLObject::DecodeMechanism::NONE)
                      << "due to : " << e.Message);
            nStorErr = ERRCODE_IO_GENERAL;
        }

        if( nStorErr == ERRCODE_NONE )
        {
            aSvDrawStorageRef.clear();
            CopyFile( aTmpURL, GetSdvURL() );
            ImplCreateSvDrawStorage();
        }

        KillFile( aTmpURL );
        ImplSetModified( true );
        ImplWrite();
        UnlockBroadcaster();
    }
}

GalleryThemeEntry* GalleryTheme::CreateThemeEntry( const INetURLObject& rURL, bool bReadOnly )
{
    DBG_ASSERT( rURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    GalleryThemeEntry*  pRet = nullptr;

    if( FileExists( rURL ) )
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( rURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

        if( pIStm )
        {
            OUString        aThemeName;
            sal_uInt16      nVersion;

            pIStm->ReadUInt16( nVersion );

            if( nVersion <= 0x00ff )
            {
                bool bThemeNameFromResource = false;
                sal_uInt32      nThemeId = 0;

                OString aTmpStr = read_uInt16_lenPrefixed_uInt8s_ToOString(*pIStm);
                aThemeName = OStringToOUString(aTmpStr, RTL_TEXTENCODING_UTF8);

                // execute a character conversion
                if( nVersion >= 0x0004 )
                {
                    sal_uInt32  nCount;
                    sal_uInt16  nTemp16;

                    pIStm->ReadUInt32( nCount ).ReadUInt16( nTemp16 );
                    pIStm->Seek( STREAM_SEEK_TO_END );

                    // check whether there is a newer version;
                    // therefore jump back by 520Bytes (8 bytes ID + 512Bytes reserve buffer)
                    // if this is at all possible.
                    if( pIStm->Tell() >= 520 )
                    {
                        sal_uInt32 nId1, nId2;

                        pIStm->SeekRel( -520 );
                        pIStm->ReadUInt32( nId1 ).ReadUInt32( nId2 );

                        if( nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
                            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
                        {
                            std::unique_ptr<VersionCompat> pCompat(new VersionCompat( *pIStm, StreamMode::READ ));

                            pIStm->ReadUInt32( nThemeId );

                            if( pCompat->GetVersion() >= 2 )
                            {
                                pIStm->ReadCharAsBool( bThemeNameFromResource );
                            }
                        }
                    }
                }

                pRet = new GalleryThemeEntry( false, rURL, aThemeName,
                                              bReadOnly, false, nThemeId,
                                              bThemeNameFromResource );
            }
        }
    }

    return pRet;
}

bool GalleryTheme::GetThumb( sal_uIntPtr nPos, BitmapEx& rBmp, bool )
{
    SgaObject*  pObj = AcquireObject( nPos );
    bool        bRet = false;

    if( pObj )
    {
        rBmp = pObj->GetThumbBmp();
        ReleaseObject( pObj );
        bRet = true;
    }

    return bRet;
}

bool GalleryTheme::GetGraphic( sal_uIntPtr nPos, Graphic& rGraphic, bool bProgress )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    bool                    bRet = false;

    if( pObject )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );

        switch( pObject->eObjKind )
        {
            case SgaObjKind::Bitmap:
            case SgaObjKind::Animation:
            case SgaObjKind::Inet:
            {
                OUString aFilterDummy;
                bRet = ( GalleryGraphicImport( aURL, rGraphic, aFilterDummy, bProgress ) != GalleryGraphicImportRet::IMPORT_NONE );
            }
            break;

            case SgaObjKind::SvDraw:
            {
                SvxGalleryDrawModel aModel;

                if( aModel.GetModel() )
                {
                    if( GetModel( nPos, *aModel.GetModel(), bProgress ) )
                    {
                        ImageMap aIMap;

                        if( CreateIMapGraphic( *aModel.GetModel(), rGraphic, aIMap ) )
                            bRet = true;
                        else
                        {
                            ScopedVclPtrInstance< VirtualDevice > pVDev;
                            pVDev->SetMapMode( MapMode( MapUnit::Map100thMM ) );
                            FmFormView aView( aModel.GetModel(), pVDev );

                            aView.hideMarkHandles();
                            aView.ShowSdrPage(aView.GetModel()->GetPage(0));
                            aView.MarkAll();
                            rGraphic = aView.GetAllMarkedGraphic();
                            bRet = true;
                        }
                    }
                }
            }
            break;

            case SgaObjKind::Sound:
            {
                SgaObject* pObj = AcquireObject( nPos );

                if( pObj )
                {
                    rGraphic = pObj->GetThumbBmp();
                    //Bitmap aBmp( pObj->GetThumbBmp() );
                    //aBmp.Replace( COL_LIGHTMAGENTA, COL_WHITE );
                    //rGraphic = aBmp;
                    ReleaseObject( pObj );
                    bRet = true;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

bool GalleryTheme::InsertGraphic( const Graphic& rGraphic, sal_uIntPtr nInsertPos )
{
    bool bRet = false;

    if( rGraphic.GetType() != GraphicType::NONE )
    {
        ConvertDataFormat nExportFormat = ConvertDataFormat::Unknown;
        const GfxLink     aGfxLink( ( (Graphic&) rGraphic ).GetLink() );

        if( aGfxLink.GetDataSize() )
        {
            switch( aGfxLink.GetType() )
            {
                case GfxLinkType::EpsBuffer: nExportFormat = ConvertDataFormat::SVM; break;
                case GfxLinkType::NativeGif: nExportFormat = ConvertDataFormat::GIF; break;

                // #i15508# added BMP type
                // could not find/trigger a call to this, but should do no harm
                case GfxLinkType::NativeBmp: nExportFormat = ConvertDataFormat::BMP; break;

                case GfxLinkType::NativeJpg: nExportFormat = ConvertDataFormat::JPG; break;
                case GfxLinkType::NativePng: nExportFormat = ConvertDataFormat::PNG; break;
                case GfxLinkType::NativeTif: nExportFormat = ConvertDataFormat::TIF; break;
                case GfxLinkType::NativeWmf: nExportFormat = ConvertDataFormat::WMF; break;
                case GfxLinkType::NativeMet: nExportFormat = ConvertDataFormat::MET; break;
                case GfxLinkType::NativePct: nExportFormat = ConvertDataFormat::PCT; break;
                case GfxLinkType::NativeSvg: nExportFormat = ConvertDataFormat::SVG; break;
                default:
                    break;
            }
        }
        else
        {
            if( rGraphic.GetType() == GraphicType::Bitmap )
            {
                if( rGraphic.IsAnimated() )
                    nExportFormat = ConvertDataFormat::GIF;
                else
                    nExportFormat = ConvertDataFormat::PNG;
            }
            else
                nExportFormat = ConvertDataFormat::SVM;
        }

        const INetURLObject aURL( ImplCreateUniqueURL( SgaObjKind::Bitmap, nExportFormat ) );
        std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC ));

        if( pOStm )
        {
            pOStm->SetVersion( SOFFICE_FILEFORMAT_50 );

            if( ConvertDataFormat::SVM == nExportFormat )
            {
                GDIMetaFile aMtf( rGraphic.GetGDIMetaFile() );

                aMtf.Write( *pOStm );
                bRet = ( pOStm->GetError() == ERRCODE_NONE );
            }
            else
            {
                if( aGfxLink.GetDataSize() && aGfxLink.GetData() )
                {
                    pOStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                    bRet = ( pOStm->GetError() == ERRCODE_NONE );
                }
                else
                    bRet = ( GraphicConverter::Export( *pOStm, rGraphic, nExportFormat ) == ERRCODE_NONE );
            }

            pOStm.reset();
        }

        if( bRet )
        {
            const SgaObjectBmp aObjBmp( aURL );
            InsertObject( aObjBmp, nInsertPos );
        }
    }

    return bRet;
}

bool GalleryTheme::GetModel( sal_uIntPtr nPos, SdrModel& rModel, bool )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    bool                    bRet = false;

    if( pObject && ( SgaObjKind::SvDraw == pObject->eObjKind ) )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );
        tools::SvRef<SotStorage>        xStor( GetSvDrawStorage() );

        if( xStor.is() )
        {
            const OUString        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
            tools::SvRef<SotStorageStream>  xIStm( xStor->OpenSotStream( aStmName, StreamMode::READ ) );

            if( xIStm.is() && !xIStm->GetError() )
            {
                xIStm->SetBufferSize( STREAMBUF_SIZE );
                bRet = GallerySvDrawImport( *xIStm, rModel );
                xIStm->SetBufferSize( 0L );
            }
        }
    }

    return bRet;
}

bool GalleryTheme::InsertModel( const FmFormModel& rModel, sal_uIntPtr nInsertPos )
{
    INetURLObject   aURL( ImplCreateUniqueURL( SgaObjKind::SvDraw ) );
    tools::SvRef<SotStorage>    xStor( GetSvDrawStorage() );
    bool            bRet = false;

    if( xStor.is() )
    {
        const OUString        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
        tools::SvRef<SotStorageStream>  xOStm( xStor->OpenSotStream( aStmName, StreamMode::WRITE | StreamMode::TRUNC ) );

        if( xOStm.is() && !xOStm->GetError() )
        {
            SvMemoryStream  aMemStm( 65535, 65535 );
            FmFormModel*    pFormModel = const_cast<FmFormModel*>(&rModel);

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

bool GalleryTheme::GetModelStream( sal_uIntPtr nPos, tools::SvRef<SotStorageStream>& rxModelStream )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    bool                    bRet = false;

    if( pObject && ( SgaObjKind::SvDraw == pObject->eObjKind ) )
    {
        const INetURLObject aURL( ImplGetURL( pObject ) );
        tools::SvRef<SotStorage>        xStor( GetSvDrawStorage() );

        if( xStor.is() )
        {
            const OUString        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
            tools::SvRef<SotStorageStream>  xIStm( xStor->OpenSotStream( aStmName, StreamMode::READ ) );

            if( xIStm.is() && !xIStm->GetError() )
            {
                sal_uInt32 nVersion = 0;

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

bool GalleryTheme::InsertModelStream( const tools::SvRef<SotStorageStream>& rxModelStream, sal_uIntPtr nInsertPos )
{
    INetURLObject   aURL( ImplCreateUniqueURL( SgaObjKind::SvDraw ) );
    tools::SvRef<SotStorage>    xStor( GetSvDrawStorage() );
    bool            bRet = false;

    if( xStor.is() )
    {
        const OUString        aStmName( GetSvDrawStreamNameFromURL( aURL ) );
        tools::SvRef<SotStorageStream>  xOStm( xStor->OpenSotStream( aStmName, StreamMode::WRITE | StreamMode::TRUNC ) );

        if( xOStm.is() && !xOStm->GetError() )
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

bool GalleryTheme::GetURL( sal_uIntPtr nPos, INetURLObject& rURL )
{
    const GalleryObject*    pObject = ImplGetGalleryObject( nPos );
    bool                    bRet = false;

    if( pObject )
    {
        rURL = INetURLObject( ImplGetURL( pObject ) );
        bRet = true;
    }

    return bRet;
}

bool GalleryTheme::InsertURL( const INetURLObject& rURL, sal_uIntPtr nInsertPos )
{
    Graphic         aGraphic;
    OUString        aFormat;
    std::unique_ptr<SgaObject> pNewObj;
    const GalleryGraphicImportRet nImportRet = GalleryGraphicImport( rURL, aGraphic, aFormat );
    bool            bRet = false;

    if( nImportRet != GalleryGraphicImportRet::IMPORT_NONE )
    {
        if ( GalleryGraphicImportRet::IMPORT_INET == nImportRet )
            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectINet( aGraphic, rURL, aFormat )));
        else if ( aGraphic.IsAnimated() )
            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectAnim( aGraphic, rURL, aFormat )));
        else
            pNewObj.reset(static_cast<SgaObject*>(new SgaObjectBmp( aGraphic, rURL, aFormat )));
    }
#if HAVE_FEATURE_AVMEDIA
    else if( ::avmedia::MediaWindow::isMediaURL( rURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), ""/*TODO?*/ ) )
        pNewObj.reset(static_cast<SgaObject*>(new SgaObjectSound( rURL )));
#endif
    if( pNewObj && InsertObject( *pNewObj, nInsertPos ) )
        bRet = true;

    return bRet;
}

bool GalleryTheme::InsertFileOrDirURL( const INetURLObject& rFileOrDirURL, sal_uIntPtr nInsertPos )
{
    INetURLObject                   aURL;
    ::std::vector< INetURLObject >  aURLVector;
    bool                            bRet = false;

    try
    {
        ::ucbhelper::Content         aCnt( rFileOrDirURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        bool        bFolder = false;

        aCnt.getPropertyValue("IsFolder") >>= bFolder;

        if( bFolder )
        {
            uno::Sequence<OUString> aProps { "Url" };
            uno::Reference< sdbc::XResultSet > xResultSet( aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );
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

bool GalleryTheme::InsertTransferable( const uno::Reference< datatransfer::XTransferable >& rxTransferable, sal_uIntPtr nInsertPos )
{
    bool bRet = false;

    if( rxTransferable.is() )
    {
        TransferableDataHelper  aDataHelper( rxTransferable );
        std::unique_ptr<Graphic> pGraphic;

        if( aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ) )
        {
            tools::SvRef<SotStorageStream> xModelStm;

            if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::DRAWING, xModelStm ) )
                bRet = InsertModelStream( xModelStm, nInsertPos );
        }
        else if( aDataHelper.HasFormat( SotClipboardFormatId::FILE_LIST ) ||
                 aDataHelper.HasFormat( SotClipboardFormatId::SIMPLE_FILE ) )
        {
            FileList aFileList;

            if( aDataHelper.HasFormat( SotClipboardFormatId::FILE_LIST ) )
                aDataHelper.GetFileList( SotClipboardFormatId::FILE_LIST, aFileList );
            else
            {
                OUString aFile;
                if (aDataHelper.GetString(SotClipboardFormatId::SIMPLE_FILE, aFile) && !aFile.isEmpty())
                    aFileList.AppendFile( aFile );
            }

            for( sal_uInt32 i = 0, nCount = aFileList.Count(); i < nCount; ++i )
            {
                const OUString  aFile( aFileList.GetFile( i ) );
                INetURLObject   aURL( aFile );

                if( aURL.GetProtocol() == INetProtocol::NotValid )
                {
                    OUString aLocalURL;

                    if( osl::FileBase::getFileURLFromSystemPath( aFile, aLocalURL ) == osl::FileBase::E_None )
                        aURL = INetURLObject( aLocalURL );
                }

                if( aURL.GetProtocol() != INetProtocol::NotValid )
                    bRet = InsertFileOrDirURL( aURL, nInsertPos );
            }
        }
        else
        {
            Graphic aGraphic;
            SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;

            if( aDataHelper.HasFormat( SotClipboardFormatId::SVXB ) )
                nFormat = SotClipboardFormatId::SVXB;
            else if( aDataHelper.HasFormat( SotClipboardFormatId::GDIMETAFILE ) )
                nFormat = SotClipboardFormatId::GDIMETAFILE;
            else if( aDataHelper.HasFormat( SotClipboardFormatId::BITMAP ) )
                nFormat = SotClipboardFormatId::BITMAP;

            if( nFormat != SotClipboardFormatId::NONE && aDataHelper.GetGraphic( nFormat, aGraphic ) )
                pGraphic.reset(new Graphic( aGraphic ));
        }

        if( pGraphic )
        {
            bRet = false;

            if( aDataHelper.HasFormat( SotClipboardFormatId::SVIM ) )
            {

                ImageMap aImageMap;

                // according to KA we don't need a BaseURL here
                if( aDataHelper.GetImageMap( SotClipboardFormatId::SVIM, aImageMap ) )
                {
                    SvxGalleryDrawModel aModel;

                    if( aModel.GetModel() )
                    {
                        SgaUserDataFactory  aFactory;

                        SdrPage*    pPage = aModel.GetModel()->GetPage(0);
                        SdrGrafObj* pGrafObj = new SdrGrafObj( *pGraphic );

                        pGrafObj->AppendUserData( new SgaIMapInfo( aImageMap ) );
                        pPage->InsertObject( pGrafObj );
                        bRet = InsertModel( *aModel.GetModel(), nInsertPos );
                    }
                }
            }

            if( !bRet )
                bRet = InsertGraphic( *pGraphic, nInsertPos );
        }
    }

    return bRet;
}

void GalleryTheme::CopyToClipboard( vcl::Window* pWindow, sal_uIntPtr nPos )
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos, false );
    pTransferable->CopyToClipboard( pWindow );
}

void GalleryTheme::StartDrag( vcl::Window* pWindow, sal_uIntPtr nPos )
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos, true );
    pTransferable->StartDrag( pWindow, DND_ACTION_COPY | DND_ACTION_LINK );
}

SvStream& GalleryTheme::WriteData( SvStream& rOStm ) const
{
    const INetURLObject aRelURL1( GetParent()->GetRelativeURL() );
    const INetURLObject aRelURL2( GetParent()->GetUserURL() );
    sal_uInt32          nCount = GetObjectCount();
    bool                bRel;

    rOStm.WriteUInt16( 0x0004 );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, pThm->GetThemeName(), RTL_TEXTENCODING_UTF8);
    rOStm.WriteUInt32( nCount ).WriteUInt16( osl_getThreadTextEncoding() );

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        const GalleryObject* pObj = ImplGetGalleryObject( i );
        OUString               aPath;

        if( SgaObjKind::SvDraw == pObj->eObjKind )
        {
            aPath = GetSvDrawStreamNameFromURL( pObj->aURL );
            bRel = false;
        }
        else
        {
            aPath = pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            aPath = aPath.copy( 0, std::min(aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength(), aPath.getLength()) );
            bRel = aPath == aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength() > ( aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength() + 1 ) ) )
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                aPath = aPath.copy( std::min(aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength(), aPath.getLength()) );
            }
            else
            {
                aPath = pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                aPath = aPath.copy( 0, std::min(aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength(), aPath.getLength()) );
                bRel = aPath == aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                if( bRel && ( pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength() > ( aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength() + 1 ) ) )
                {
                    aPath = pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    aPath = aPath.copy( std::min(aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE ).getLength(), aPath.getLength()) );
                }
                else
                    aPath = pObj->aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            }
        }

        if ( !m_aDestDir.isEmpty() )
        {
            bool aFound = aPath.indexOf(m_aDestDir) != -1;
            aPath = aPath.replaceFirst(m_aDestDir, "");
            if ( aFound )
                bRel = m_bDestDirRelative;
            else
                SAL_WARN( "svx", "failed to replace destdir of '"
                          << m_aDestDir << "' in '" << aPath << "'");
        }

        rOStm.WriteBool( bRel );
        write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aPath, RTL_TEXTENCODING_UTF8);
        rOStm.WriteUInt32( pObj->nOffset ).WriteUInt16( (sal_uInt16)pObj->eObjKind );
    }

    // more recently, a 512-byte reserve buffer is written,
    // to recognize them two sal_uIntPtr-Ids will be written.
    rOStm.WriteUInt32( COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) ).WriteUInt32( COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) );

    const long      nReservePos = rOStm.Tell();
    std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rOStm, StreamMode::WRITE, 2 ));

    rOStm.WriteUInt32( GetId() ).WriteBool( IsThemeNameFromResource() ); // From version 2 and up

    pCompat.reset();

    // Fill the rest of the buffer.
    const long  nRest = std::max( 512L - ( (long) rOStm.Tell() - nReservePos ), 0L );

    if( nRest )
    {
        std::unique_ptr<char[]> pReserve(new char[ nRest ]);
        memset( pReserve.get(), 0, nRest );
        rOStm.WriteBytes(pReserve.get(), nRest);
    }

    return rOStm;
}

SvStream& GalleryTheme::ReadData( SvStream& rIStm )
{
    sal_uInt32          nCount;
    sal_uInt16          nVersion;
    OUString            aThemeName;
    rtl_TextEncoding    nTextEncoding;

    rIStm.ReadUInt16( nVersion );
    OString aTmpStr = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
    rIStm.ReadUInt32( nCount );

    if( nVersion >= 0x0004 )
    {
        sal_uInt16 nTmp16;
        rIStm.ReadUInt16( nTmp16 );
        nTextEncoding = (rtl_TextEncoding) nTmp16;
    }
    else
        nTextEncoding = RTL_TEXTENCODING_UTF8;

    aThemeName = OStringToOUString(aTmpStr, nTextEncoding);

    if( nCount <= ( 1L << 14 ) )
    {
        GalleryObject*  pObj;
        INetURLObject   aRelURL1( GetParent()->GetRelativeURL() );
        INetURLObject   aRelURL2( GetParent()->GetUserURL() );
        sal_uInt32      nId1, nId2;
        bool            bRel;

        for(GalleryObject* i : aObjectList)
        {
            pObj = i;
            Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), reinterpret_cast< sal_uIntPtr >( pObj ) ) );
            Broadcast( GalleryHint( GalleryHintType::OBJECT_REMOVED, GetName(), reinterpret_cast< sal_uIntPtr >( pObj ) ) );
            delete pObj;
        }
        aObjectList.clear();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            pObj = new GalleryObject;

            OUString    aFileName;
            OUString    aPath;
            sal_uInt16  nTemp;

            rIStm.ReadCharAsBool( bRel );
            OString aTempFileName = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
            rIStm.ReadUInt32( pObj->nOffset );
            rIStm.ReadUInt16( nTemp ); pObj->eObjKind = (SgaObjKind) nTemp;

            aFileName = OStringToOUString(aTempFileName, osl_getThreadTextEncoding());

            if( bRel )
            {
                aFileName = aFileName.replaceAll( "\\", "/" );
                aPath = aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                if( aFileName[ 0 ] != '/' )
                        aPath += "/";

                aPath += aFileName;

                pObj->aURL = INetURLObject( aPath );

                if( !FileExists( pObj->aURL ) )
                {
                    aPath = aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                    if( aFileName[0] != '/' )
                        aPath += "/";

                    aPath += aFileName;

                    // assign this URL, even in the case it is not valid (#94482)
                    pObj->aURL = INetURLObject( aPath );
                }
            }
            else
            {
                if( SgaObjKind::SvDraw == pObj->eObjKind )
                {
                    OUString aDummyURL( "gallery/svdraw/" );
                    pObj->aURL = INetURLObject( aDummyURL += aFileName, INetProtocol::PrivSoffice );
                }
                else
                {
                    OUString aLocalURL;

                    pObj->aURL = INetURLObject( aFileName );

                    if( ( pObj->aURL.GetProtocol() == INetProtocol::NotValid ) &&
                        osl::FileBase::getFileURLFromSystemPath( aFileName, aLocalURL ) == osl::FileBase::E_None )
                    {
                        pObj->aURL = INetURLObject( aLocalURL );
                    }
                }
            }
            aObjectList.push_back( pObj );
        }

        rIStm.ReadUInt32( nId1 ).ReadUInt32( nId2 );

        // In newer versions a 512 byte reserve buffer is located at the end,
        // the data is located at the beginning of this buffer and are clamped
        // by a VersionCompat.
        if( !rIStm.IsEof() &&
            nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
        {
            std::unique_ptr<VersionCompat> pCompat(new VersionCompat( rIStm, StreamMode::READ ));
            sal_uInt32      nTemp32;
            bool            bThemeNameFromResource = false;

            rIStm.ReadUInt32( nTemp32 );

            if( pCompat->GetVersion() >= 2 )
            {
                rIStm.ReadCharAsBool( bThemeNameFromResource );
            }

            SetId( nTemp32, bThemeNameFromResource );
        }
    }
    else
        rIStm.SetError( SVSTREAM_READ_ERROR );

    ImplSetModified( false );

    return rIStm;
}

SvStream& WriteGalleryTheme( SvStream& rOut, const GalleryTheme& rTheme )
{
    return rTheme.WriteData( rOut );
}

SvStream& ReadGalleryTheme( SvStream& rIn, GalleryTheme& rTheme )
{
    return rTheme.ReadData( rIn );
}

void GalleryTheme::ImplSetModified( bool bModified )
{
    pThm->SetModified(bModified);
}

const INetURLObject& GalleryTheme::GetThmURL() const { return pThm->GetThmURL(); }
const INetURLObject& GalleryTheme::GetSdgURL() const { return pThm->GetSdgURL(); }
const INetURLObject& GalleryTheme::GetSdvURL() const { return pThm->GetSdvURL(); }
sal_uInt32 GalleryTheme::GetId() const { return pThm->GetId(); }
void GalleryTheme::SetId( sal_uInt32 nNewId, bool bResetThemeName ) { pThm->SetId( nNewId, bResetThemeName ); }
bool GalleryTheme::IsThemeNameFromResource() const { return pThm->IsNameFromResource(); }
bool GalleryTheme::IsReadOnly() const { return pThm->IsReadOnly(); }
bool GalleryTheme::IsDefault() const { return pThm->IsDefault(); }

const tools::SvRef<SotStorage>& GalleryTheme::GetSvDrawStorage() const
{
    return aSvDrawStorageRef;
}

const OUString& GalleryTheme::GetName() const { return pThm->GetThemeName(); }

void GalleryTheme::InsertAllThemes( ListBox& rListBox )
{
    for( sal_uInt16 i = RID_GALLERYSTR_THEME_FIRST; i <= RID_GALLERYSTR_THEME_LAST; i++ )
        rListBox.InsertEntry(GAL_RESSTR(i));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
