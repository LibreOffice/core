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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <vcl/virdev.hxx>
#include <vcl/cvtgrf.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/svdograf.hxx>
#include "codec.hxx"
#include <svx/unomodel.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/galmisc.hxx>
#include <svx/galtheme.hxx>
#include <svx/svdpage.hxx>
#include <svx/galleryobjectcollection.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <galobj.hxx>
#include <svx/gallery1.hxx>
#include "gallerydrawmodel.hxx"
#include <memory>

using namespace ::com::sun::star;

GalleryTheme::GalleryTheme( Gallery* pGallery, GalleryThemeEntry* pThemeEntry )
    : pParent(pGallery)
    , pThm(pThemeEntry)
    , mnThemeLockCount(0)
    , mnBroadcasterLockCount(0)
    , nDragPos(0)
    , bDragging(false)
    , bAbortActualize(false)
{
    mpGalleryBinaryEngine = createGalleryBinaryEngine(IsReadOnly());
}

GalleryTheme::~GalleryTheme()
{
    if(pThm->IsModified())
        if(!mpGalleryBinaryEngine->implWrite(*this, pThm))
            ImplSetModified(false);

    for (auto & pEntry : maGalleryObjectCollection.getObjectList())
    {
        Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), pEntry.get() ) );
        pEntry.reset();
    }
    maGalleryObjectCollection.clear();
    mpGalleryBinaryEngine->clearSotStorage();
}

void GalleryTheme::SetDestDir(const OUString& rDestDir, bool bRelative)
{
    mpGalleryBinaryEngine->setDestDir(rDestDir, bRelative);
}

std::unique_ptr<GalleryBinaryEngine> GalleryTheme::createGalleryBinaryEngine(bool bReadOnly)
{
    std::unique_ptr<GalleryBinaryEngine> pGalleryBinaryEngine = std::make_unique<GalleryBinaryEngine>(pThm->getGalleryStorageLocations(), maGalleryObjectCollection, bReadOnly);
    return pGalleryBinaryEngine;
}

void GalleryTheme::ImplBroadcast(sal_uInt32 nUpdatePos)
{
    if( !IsBroadcasterLocked() )
    {
        if( GetObjectCount() && ( nUpdatePos >= GetObjectCount() ) )
            nUpdatePos = GetObjectCount() - 1;

        Broadcast( GalleryHint( GalleryHintType::THEME_UPDATEVIEW, GetName(), reinterpret_cast<void*>(nUpdatePos) ) );
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

bool GalleryTheme::InsertObject(const SgaObject& rObj, sal_uInt32 nInsertPos)
{
    if (!rObj.IsValid())
        return false;

    GalleryObject* pFoundEntry = nullptr;
    sal_uInt32 iFoundPos = 0;
    for (sal_uInt32 n = maGalleryObjectCollection.size(); iFoundPos < n; ++iFoundPos)
    {
        if (maGalleryObjectCollection.get(iFoundPos)->aURL == rObj.GetURL())
        {
            pFoundEntry = maGalleryObjectCollection.get(iFoundPos).get();
            break;
        }
    }

    mpGalleryBinaryEngine->insertObject(rObj, pFoundEntry, nInsertPos);

    ImplSetModified(true);
    ImplBroadcast(pFoundEntry? iFoundPos: nInsertPos);

    return true;
}

std::unique_ptr<SgaObject> GalleryTheme::AcquireObject(sal_uInt32 nPos)
{
    return mpGalleryBinaryEngine->implReadSgaObject(maGalleryObjectCollection.getForPosition(nPos));
}

void GalleryTheme::GetPreviewBitmapExAndStrings(sal_uInt32 nPos, BitmapEx& rBitmapEx, Size& rSize, OUString& rTitle, OUString& rPath)
{
    const GalleryObject* pGalleryObject = maGalleryObjectCollection.get(nPos).get();

    rBitmapEx = pGalleryObject->maPreviewBitmapEx;
    rSize = pGalleryObject->maPreparedSize;
    rTitle = pGalleryObject->maTitle;
    rPath = pGalleryObject->maPath;
}

void GalleryTheme::SetPreviewBitmapExAndStrings(sal_uInt32 nPos, const BitmapEx& rBitmapEx, const Size& rSize, const OUString& rTitle, const OUString& rPath)
{
    GalleryObject* pGalleryObject = maGalleryObjectCollection.get(nPos).get();

    pGalleryObject->maPreviewBitmapEx = rBitmapEx;
    pGalleryObject->maPreparedSize = rSize;
    pGalleryObject->maTitle = rTitle;
    pGalleryObject->maPath = rPath;
}

void GalleryTheme::RemoveObject(sal_uInt32 nPos)
{
    auto it = maGalleryObjectCollection.getObjectList().begin() + nPos;
    std::unique_ptr<GalleryObject> pEntry = std::move(*it);
    maGalleryObjectCollection.getObjectList().erase( it );

    mpGalleryBinaryEngine->removeObject(pEntry);

    Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), pEntry.get() ) );
    pEntry.reset();

    ImplSetModified( true );
    ImplBroadcast( nPos );
}

bool GalleryTheme::ChangeObjectPos(sal_uInt32 nOldPos, sal_uInt32 nNewPos)
{
    if (nOldPos == nNewPos || nOldPos >= maGalleryObjectCollection.size())
        return false;

    std::unique_ptr<GalleryObject> pEntry = std::move(maGalleryObjectCollection.get(nOldPos));

    maGalleryObjectCollection.getObjectList().insert(maGalleryObjectCollection.getObjectList().begin() + nNewPos, std::move(pEntry));

    if (nNewPos < nOldPos)
        nOldPos++;

    auto it = maGalleryObjectCollection.getObjectList().begin() + nOldPos;
    maGalleryObjectCollection.getObjectList().erase(it);

    ImplSetModified(true);
    ImplBroadcast((nNewPos < nOldPos)? nNewPos: (nNewPos - 1));

    return true;
}

void GalleryTheme::Actualize( const Link<const INetURLObject&, void>& rActualizeLink, GalleryProgress* pProgress )
{
    if( IsReadOnly() )
        return;

    Graphic         aGraphic;
    OUString        aFormat;
    const sal_uInt32 nCount = maGalleryObjectCollection.size();

    LockBroadcaster();
    bAbortActualize = false;

    // reset delete flag
    for (sal_uInt32 i = 0; i < nCount; i++)
        maGalleryObjectCollection.get(i)->mbDelete = false;

    for (sal_uInt32 i = 0; ( i < nCount ) && !bAbortActualize; i++)
    {
        if( pProgress )
            pProgress->Update( i, nCount - 1 );

        GalleryObject* pEntry = maGalleryObjectCollection.get(i).get();

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
                        pNewObj.reset(new SgaObjectINet( aGraphic, aURL ));
                    else if ( aGraphic.IsAnimated() )
                        pNewObj.reset(new SgaObjectAnim( aGraphic, aURL ));
                    else
                        pNewObj.reset(new SgaObjectBmp( aGraphic, aURL ));

                    if( !InsertObject( *pNewObj ) )
                        pEntry->mbDelete = true;
                }
                else
                    pEntry->mbDelete = true; // set delete flag
            }
        }
        else
        {
            //update SvDraw object
            if ( mpGalleryBinaryEngine->GetSvDrawStorage().is() )
            {
                SgaObjectSvDraw aNewObj = mpGalleryBinaryEngine->updateSvDrawObject(pEntry);
                if (aNewObj.IsValid() && !InsertObject(aNewObj))
                    pEntry->mbDelete = true;
            }
        }
    }

    // remove all entries with set flag
    for ( auto it = maGalleryObjectCollection.getObjectList().begin(); it != maGalleryObjectCollection.getObjectList().end(); /* increment is in the body of loop */)
    {
        if( (*it)->mbDelete )
        {
            Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), it->get() ) );
            it = maGalleryObjectCollection.getObjectList().erase( it );
        }
        else
            ++it;
    }

    // update theme
    mpGalleryBinaryEngine->updateTheme();
    ImplSetModified( true );
    if (pThm->IsModified())
        if (!mpGalleryBinaryEngine->implWrite(*this, pThm))
            ImplSetModified(false);
    UnlockBroadcaster();
}

bool GalleryTheme::GetThumb(sal_uInt32 nPos, BitmapEx& rBmp)
{
    std::unique_ptr<SgaObject> pObj = AcquireObject( nPos );
    bool        bRet = false;

    if( pObj )
    {
        rBmp = pObj->GetThumbBmp();
        bRet = true;
    }

    return bRet;
}

bool GalleryTheme::GetGraphic(sal_uInt32 nPos, Graphic& rGraphic)
{
    const GalleryObject*    pObject = maGalleryObjectCollection.getForPosition( nPos );
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
                bRet = ( GalleryGraphicImport( aURL, rGraphic, aFilterDummy ) != GalleryGraphicImportRet::IMPORT_NONE );
            }
            break;

            case SgaObjKind::SvDraw:
            {
                SvxGalleryDrawModel aModel;

                if( aModel.GetModel() )
                {
                    if( GetModel( nPos, *aModel.GetModel() ) )
                    {
                        ImageMap aIMap;

                        if( CreateIMapGraphic( *aModel.GetModel(), rGraphic, aIMap ) )
                            bRet = true;
                        else
                        {
                            ScopedVclPtrInstance< VirtualDevice > pVDev;
                            pVDev->SetMapMode( MapMode( MapUnit::Map100thMM ) );
                            FmFormView aView(*aModel.GetModel(), pVDev);

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
                std::unique_ptr<SgaObject> pObj = AcquireObject( nPos );

                if( pObj )
                {
                    rGraphic = pObj->GetThumbBmp();
                    //Bitmap aBmp( pObj->GetThumbBmp() );
                    //aBmp.Replace( COL_LIGHTMAGENTA, COL_WHITE );
                    //rGraphic = aBmp;
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

bool GalleryTheme::InsertGraphic(const Graphic& rGraphic, sal_uInt32 nInsertPos)
{
    bool bRet = false;

    if( rGraphic.GetType() != GraphicType::NONE )
    {
        ConvertDataFormat nExportFormat = ConvertDataFormat::Unknown;
        const GfxLink     aGfxLink( rGraphic.GetGfxLink() );

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

        const SgaObjectBmp aObjBmp = mpGalleryBinaryEngine->insertGraphic(rGraphic, aGfxLink, nExportFormat, GetParent()->GetUserURL());

        if (aObjBmp.IsValid())
            bRet = InsertObject(aObjBmp, nInsertPos);
    }

    return bRet;
}

bool GalleryTheme::GetModel(sal_uInt32 nPos, SdrModel& rModel)
{
    const GalleryObject*    pObject = maGalleryObjectCollection.getForPosition( nPos );
    bool                    bRet = false;

    if( pObject && ( SgaObjKind::SvDraw == pObject->eObjKind ) )
    {
        bRet = mpGalleryBinaryEngine->readModel(pObject, rModel);
    }

    return bRet;
}

bool GalleryTheme::InsertModel(const FmFormModel& rModel, sal_uInt32 nInsertPos)
{
    bool bRet = false;
    SgaObjectSvDraw aObjSvDraw = mpGalleryBinaryEngine->insertModel(rModel, GetParent()->GetUserURL());
    if(aObjSvDraw.IsValid())
        bRet = InsertObject( aObjSvDraw, nInsertPos );
    return bRet;
}

bool GalleryTheme::GetModelStream(sal_uInt32 nPos, tools::SvRef<SotStorageStream> const & rxModelStream)
{
    const GalleryObject*    pObject = maGalleryObjectCollection.getForPosition( nPos );
    bool                    bRet = false;

    if( pObject && ( SgaObjKind::SvDraw == pObject->eObjKind ) )
    {
        bRet = mpGalleryBinaryEngine->readModelStream(pObject, rxModelStream);
    }

    return bRet;
}

bool GalleryTheme::InsertModelStream(const tools::SvRef<SotStorageStream>& rxModelStream, sal_uInt32 nInsertPos)
{
    bool            bRet = false;

    const SgaObjectSvDraw aObjSvDraw = mpGalleryBinaryEngine->insertModelStream(rxModelStream, GetParent()->GetUserURL());
    if(aObjSvDraw.IsValid())
        bRet = InsertObject( aObjSvDraw, nInsertPos );

    return bRet;
}

bool GalleryTheme::GetURL(sal_uInt32 nPos, INetURLObject& rURL)
{
    const GalleryObject*    pObject = maGalleryObjectCollection.getForPosition( nPos );
    bool                    bRet = false;

    if( pObject )
    {
        rURL = ImplGetURL( pObject );
        bRet = true;
    }

    return bRet;
}

bool GalleryTheme::InsertURL(const INetURLObject& rURL, sal_uInt32 nInsertPos)
{
    Graphic         aGraphic;
    OUString        aFormat;
    std::unique_ptr<SgaObject> pNewObj;
    const GalleryGraphicImportRet nImportRet = GalleryGraphicImport( rURL, aGraphic, aFormat );
    bool            bRet = false;

    if( nImportRet != GalleryGraphicImportRet::IMPORT_NONE )
    {
        if ( aGraphic.IsAnimated() )
            pNewObj.reset(new SgaObjectAnim( aGraphic, rURL ));
        else
            pNewObj.reset(new SgaObjectBmp( aGraphic, rURL ));
    }
#if HAVE_FEATURE_AVMEDIA
    else if( ::avmedia::MediaWindow::isMediaURL( rURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), ""/*TODO?*/ ) )
        pNewObj.reset(new SgaObjectSound( rURL ));
#endif
    if( pNewObj && InsertObject( *pNewObj, nInsertPos ) )
        bRet = true;

    return bRet;
}

bool GalleryTheme::InsertFileOrDirURL(const INetURLObject& rFileOrDirURL, sal_uInt32 nInsertPos)
{
    bool bRet = false;
    std::vector< INetURLObject > aURLVector;
    GalleryBinaryEngine::insertFileOrDirURL(rFileOrDirURL, aURLVector);

    for( const auto& rURL : aURLVector )
        bRet = bRet || InsertURL( rURL, nInsertPos );

    return bRet;
}

bool GalleryTheme::InsertTransferable(const uno::Reference< datatransfer::XTransferable >& rxTransferable, sal_uInt32 nInsertPos)
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
                        SdrPage*    pPage = aModel.GetModel()->GetPage(0);
                        SdrGrafObj* pGrafObj = new SdrGrafObj(*aModel.GetModel(), *pGraphic );

                        pGrafObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new SgaIMapInfo( aImageMap )) );
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

void GalleryTheme::CopyToClipboard(sal_uInt32 nPos)
{
    GalleryTransferable* pTransferable = new GalleryTransferable( this, nPos, false );
    pTransferable->CopyToClipboard(GetSystemClipboard());
}

SvStream& GalleryTheme::ReadData( SvStream& rIStm )
{
    sal_uInt32          nCount;
    sal_uInt16          nVersion;

    rIStm.ReadUInt16( nVersion );
    read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
    rIStm.ReadUInt32( nCount );

    if( nVersion >= 0x0004 )
    {
        sal_uInt16 nTmp16;
        rIStm.ReadUInt16( nTmp16 );
    }

    if( nCount <= ( 1 << 14 ) )
    {
        INetURLObject   aRelURL1( GetParent()->GetRelativeURL() );
        INetURLObject   aRelURL2( GetParent()->GetUserURL() );
        sal_uInt32      nId1, nId2;
        bool            bRel;

        for(auto & i : maGalleryObjectCollection.getObjectList())
        {
            GalleryObject* pObj = i.get();
            Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), pObj ) );
            i.reset();
        }
        maGalleryObjectCollection.clear();

        for( sal_uInt32 i = 0; i < nCount; i++ )
        {
            std::unique_ptr<GalleryObject> pObj(new GalleryObject);

            OUString    aFileName;
            OUString    aPath;
            sal_uInt16  nTemp;

            rIStm.ReadCharAsBool( bRel );
            OString aTempFileName = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
            rIStm.ReadUInt32( pObj->nOffset );
            rIStm.ReadUInt16( nTemp ); pObj->eObjKind = static_cast<SgaObjKind>(nTemp);

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
                    OUString aDummyURL = "gallery/svdraw/" + aFileName;
                    pObj->aURL = INetURLObject( aDummyURL, INetProtocol::PrivSoffice );
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
            maGalleryObjectCollection.getObjectList().push_back( std::move(pObj) );
        }

        rIStm.ReadUInt32( nId1 ).ReadUInt32( nId2 );

        // In newer versions a 512 byte reserve buffer is located at the end,
        // the data is located at the beginning of this buffer and are clamped
        // by a VersionCompat.
        if( !rIStm.eof() &&
            nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
        {
            VersionCompat   aCompat( rIStm, StreamMode::READ );
            sal_uInt32      nTemp32;
            bool            bThemeNameFromResource = false;

            rIStm.ReadUInt32( nTemp32 );

            if( aCompat.GetVersion() >= 2 )
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
const INetURLObject& GalleryTheme::GetStrURL() const { return pThm->GetStrURL(); }
sal_uInt32 GalleryTheme::GetId() const { return pThm->GetId(); }
void GalleryTheme::SetId( sal_uInt32 nNewId, bool bResetThemeName ) { pThm->SetId( nNewId, bResetThemeName ); }
bool GalleryTheme::IsReadOnly() const { return pThm->IsReadOnly(); }
bool GalleryTheme::IsDefault() const { return pThm->IsDefault(); }

const OUString& GalleryTheme::GetName() const { return pThm->GetThemeName(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
