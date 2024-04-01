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

#include <algorithm>

#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <tools/datetime.hxx>
#include <sot/storage.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <avmedia/mediawindow.hxx>
#include <svx/svdograf.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/galmisc.hxx>
#include <svx/galtheme.hxx>
#include <svx/svdpage.hxx>
#include <svx/galleryobjectcollection.hxx>
#include <galleryfilestorage.hxx>
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
    mpGalleryStorageEngine = pThm->createGalleryStorageEngine(maGalleryObjectCollection);
}

GalleryTheme::~GalleryTheme()
{
    if(pThm->IsModified())
        if(!mpGalleryStorageEngine->implWrite(*this, pThm))
            ImplSetModified(false);

    for (auto & pEntry : maGalleryObjectCollection.getObjectList())
    {
        Broadcast( GalleryHint( GalleryHintType::CLOSE_OBJECT, GetName(), pEntry.get() ) );
        pEntry.reset();
    }
    maGalleryObjectCollection.clear();
    mpGalleryStorageEngine->clearSotStorage();
}

void GalleryTheme::SetDestDir(const OUString& rDestDir, bool bRelative)
{
    mpGalleryStorageEngine->setDestDir(rDestDir, bRelative);
}

void GalleryTheme::ImplBroadcast(sal_uInt32 nUpdatePos)
{
    if( !IsBroadcasterLocked() )
    {
        if( GetObjectCount() && ( nUpdatePos >= GetObjectCount() ) )
            nUpdatePos = GetObjectCount() - 1;

        Broadcast( GalleryHint( GalleryHintType::THEME_UPDATEVIEW, GetName(), reinterpret_cast<void*>(static_cast<sal_uIntPtr>(nUpdatePos)) ) );
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
        if (*maGalleryObjectCollection.get(iFoundPos)->m_oStorageUrl == rObj.GetURL())
        {
            pFoundEntry = maGalleryObjectCollection.get(iFoundPos).get();
            break;
        }
    }

    mpGalleryStorageEngine->insertObject(rObj, pFoundEntry, nInsertPos);

    ImplSetModified(true);
    ImplBroadcast(pFoundEntry? iFoundPos: nInsertPos);

    return true;
}

std::unique_ptr<SgaObject> GalleryTheme::AcquireObject(sal_uInt32 nPos)
{
    return mpGalleryStorageEngine->implReadSgaObject(maGalleryObjectCollection.getForPosition(nPos));
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

    mpGalleryStorageEngine->removeObject(pEntry);

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

        const INetURLObject aURL( *pEntry->m_oStorageUrl );

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
            if ( mpGalleryStorageEngine->GetSvDrawStorage().is() )
            {
                SgaObjectSvDraw aNewObj = mpGalleryStorageEngine->updateSvDrawObject(pEntry);
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
    mpGalleryStorageEngine->updateTheme();
    ImplSetModified( true );
    if (pThm->IsModified())
        if (!mpGalleryStorageEngine->implWrite(*this, pThm))
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
                            aView.ShowSdrPage(aView.GetModel().GetPage(0));
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
                case GfxLinkType::NativeWebp: nExportFormat = ConvertDataFormat::WEBP; break;
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

        const SgaObjectBmp aObjBmp = mpGalleryStorageEngine->insertGraphic(rGraphic, aGfxLink, nExportFormat, GetParent()->GetUserURL());

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
        bRet = mpGalleryStorageEngine->readModel(pObject, rModel);
    }

    return bRet;
}

bool GalleryTheme::InsertModel(const FmFormModel& rModel, sal_uInt32 nInsertPos)
{
    bool bRet = false;
    SgaObjectSvDraw aObjSvDraw = mpGalleryStorageEngine->insertModel(rModel, GetParent()->GetUserURL());
    if(aObjSvDraw.IsValid())
        bRet = InsertObject( aObjSvDraw, nInsertPos );
    return bRet;
}

bool GalleryTheme::GetModelStream(sal_uInt32 nPos, SvStream& rModelStream)
{
    const GalleryObject*    pObject = maGalleryObjectCollection.getForPosition( nPos );
    bool                    bRet = false;

    if( pObject && ( SgaObjKind::SvDraw == pObject->eObjKind ) )
    {
        bRet = mpGalleryStorageEngine->readModelStream(pObject, rModelStream);
    }

    return bRet;
}

bool GalleryTheme::InsertModelStream(SvStream& rModelStream, sal_uInt32 nInsertPos)
{
    bool            bRet = false;

    const SgaObjectSvDraw aObjSvDraw = mpGalleryStorageEngine->insertModelStream(rModelStream, GetParent()->GetUserURL());
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
    GalleryFileStorage::insertFileOrDirURL(rFileOrDirURL, aURLVector);

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
        std::optional<Graphic> oGraphic;

        if( aDataHelper.HasFormat( SotClipboardFormatId::DRAWING ) )
        {
            std::unique_ptr<SvStream> xModelStm;

            if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::DRAWING, xModelStm ) )
                bRet = InsertModelStream( *xModelStm, nInsertPos );
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
                oGraphic.emplace( aGraphic );
        }

        if( oGraphic )
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
                        rtl::Reference<SdrGrafObj> pGrafObj = new SdrGrafObj(*aModel.GetModel(), *oGraphic );

                        pGrafObj->AppendUserData( std::unique_ptr<SdrObjUserData>(new SgaIMapInfo( aImageMap )) );
                        pPage->InsertObject( pGrafObj.get() );
                        bRet = InsertModel( *aModel.GetModel(), nInsertPos );
                    }
                }
            }

            if( !bRet )
                bRet = InsertGraphic( *oGraphic, nInsertPos );
        }
    }

    return bRet;
}

void GalleryTheme::CopyToClipboard(const weld::Widget& rWidget, sal_uInt32 nPos)
{
    rtl::Reference<GalleryTransferable> pTransferable = new GalleryTransferable( this, nPos, false );
    pTransferable->CopyToClipboard(rWidget.get_clipboard());
}

DateTime GalleryTheme::getModificationDate() const
{
    return mpGalleryStorageEngine->getModificationDate();
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
            sal_uInt16  nTemp;

            rIStm.ReadCharAsBool( bRel );
            OString aTempFileName = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
            rIStm.ReadUInt32( pObj->nOffset );
            rIStm.ReadUInt16( nTemp ); pObj->eObjKind = static_cast<SgaObjKind>(nTemp);

            aFileName = OStringToOUString(aTempFileName, osl_getThreadTextEncoding());

            pObj->m_oStorageUrl.emplace();

            if( bRel )
            {
                aFileName = aFileName.replaceAll( "\\", "/" );
                OUString aPath = aRelURL1.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                if( aFileName[ 0 ] != '/' )
                        aPath += "/";

                aPath += aFileName;

                pObj->m_oStorageUrl = INetURLObject(aPath);

                if (!FileExists(*pObj->m_oStorageUrl))
                {
                    aPath = aRelURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                    if( aFileName[0] != '/' )
                        aPath += "/";

                    aPath += aFileName;

                    // assign this URL, even in the case it is not valid (#94482)
                    pObj->m_oStorageUrl = INetURLObject(aPath);
                }
            }
            else
            {
                if( SgaObjKind::SvDraw == pObj->eObjKind )
                {
                    OUString aDummyURL = "gallery/svdraw/" + aFileName;
                    pObj->m_oStorageUrl = INetURLObject(aDummyURL, INetProtocol::PrivSoffice);
                }
                else
                {
                    OUString aLocalURL;

                    pObj->m_oStorageUrl = INetURLObject(aFileName);

                    if( ( pObj->m_oStorageUrl->GetProtocol() == INetProtocol::NotValid ) &&
                        osl::FileBase::getFileURLFromSystemPath( aFileName, aLocalURL ) == osl::FileBase::E_None )
                    {
                        pObj->m_oStorageUrl = INetURLObject(aLocalURL);
                    }
                }
            }
            maGalleryObjectCollection.getObjectList().push_back( std::move(pObj) );
        }

        rIStm.ReadUInt32( nId1 ).ReadUInt32( nId2 );

        // In newer versions a 512 byte reserve buffer is located at the end,
        // the data is located at the beginning of this buffer and are clamped
        // by a VersionCompatRead.
        if( !rIStm.eof() &&
            nId1 == COMPAT_FORMAT( 'G', 'A', 'L', 'R' ) &&
            nId2 == COMPAT_FORMAT( 'E', 'S', 'R', 'V' ) )
        {
            VersionCompatRead aCompat(rIStm);
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

void GalleryTheme::ImplSetModified( bool bModified )
{
    pThm->SetModified(bModified);
}

sal_uInt32 GalleryTheme::GetId() const { return pThm->GetId(); }
void GalleryTheme::SetId( sal_uInt32 nNewId, bool bResetThemeName ) { pThm->SetId( nNewId, bResetThemeName ); }
bool GalleryTheme::IsReadOnly() const { return pThm->IsReadOnly(); }
bool GalleryTheme::IsDefault() const { return pThm->IsDefault(); }

const OUString& GalleryTheme::GetName() const { return pThm->GetThemeName(); }
const INetURLObject& GalleryTheme::getThemeURL() const { return mpGalleryStorageEngine->getThemeURL(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
