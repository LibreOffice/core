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

#include <svx/gallerybinaryengine.hxx>
#include <svx/galmisc.hxx>
#include <svx/unomodel.hxx>
#include <svx/fmmodel.hxx>
#include <svx/gallery1.hxx>
#include <galobj.hxx>
#include "codec.hxx"
#include "gallerydrawmodel.hxx"

#include <sal/log.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

static bool FileExists(const INetURLObject& rURL, const OUString& rExt)
{
    INetURLObject aURL(rURL);
    aURL.setExtension(rExt);
    return FileExists(aURL);
}

void GalleryBinaryEngine::galleryThemeInit(bool bReadOnly)
{
    SAL_WARN_IF(aSvDrawStorageRef.is(), "svx", "SotStorage is already initialized");
    ImplCreateSvDrawStorage(bReadOnly);
}

void GalleryBinaryEngine::galleryThemeDestroy() { aSvDrawStorageRef.clear(); }

INetURLObject GalleryBinaryEngine::ImplGetURLIgnoreCase(const INetURLObject& rURL)
{
    INetURLObject aURL(rURL);

    // check original file name
    if (!FileExists(aURL))
    {
        // check upper case file name
        aURL.setName(aURL.getName().toAsciiUpperCase());

        if (!FileExists(aURL))
        {
            // check lower case file name
            aURL.setName(aURL.getName().toAsciiLowerCase());
        }
    }

    return aURL;
}

void GalleryBinaryEngine::CreateUniqueURL(const INetURLObject& rBaseURL, INetURLObject& aURL)
{
    INetURLObject aBaseNoCase(ImplGetURLIgnoreCase(rBaseURL));
    aURL = aBaseNoCase;
    static sal_Int32 nIdx = 0;
    while (FileExists(aURL, "thm"))
    { // create new URLs
        nIdx++;
        aURL = aBaseNoCase;
        aURL.setName(aURL.getName() + OUString::number(nIdx));
    }
}

void GalleryBinaryEngine::SetThmExtension(INetURLObject aURL)
{
    aURL.setExtension("thm");
    aThmURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetSdgExtension(INetURLObject aURL)
{
    aURL.setExtension("sdg");
    aSdgURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetSdvExtension(INetURLObject aURL)
{
    aURL.setExtension("sdv");
    aSdvURL = ImplGetURLIgnoreCase(aURL);
}

void GalleryBinaryEngine::SetStrExtension(INetURLObject aURL)
{
    aURL.setExtension("str");
    aStrURL = ImplGetURLIgnoreCase(aURL);
}

std::unique_ptr<SgaObject> GalleryBinaryEngine::implReadSgaObject(GalleryObject const* pEntry)
{
    std::unique_ptr<SgaObject> pSgaObj;

    if (pEntry)
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
            GetSdgURL().GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

        if (pIStm)
        {
            sal_uInt32 nInventor;

            // Check to ensure that the file is a valid SGA file
            pIStm->Seek(pEntry->nOffset);
            pIStm->ReadUInt32(nInventor);

            if (nInventor == COMPAT_FORMAT('S', 'G', 'A', '3'))
            {
                pIStm->Seek(pEntry->nOffset);

                switch (pEntry->eObjKind)
                {
                    case SgaObjKind::Bitmap:
                        pSgaObj.reset(new SgaObjectBmp());
                        break;
                    case SgaObjKind::Animation:
                        pSgaObj.reset(new SgaObjectAnim());
                        break;
                    case SgaObjKind::Inet:
                        pSgaObj.reset(new SgaObjectINet());
                        break;
                    case SgaObjKind::SvDraw:
                        pSgaObj.reset(new SgaObjectSvDraw());
                        break;
                    case SgaObjKind::Sound:
                        pSgaObj.reset(new SgaObjectSound());
                        break;

                    default:
                        break;
                }

                if (pSgaObj)
                {
                    ReadSgaObject(*pIStm, *pSgaObj);
                    pSgaObj->ImplUpdateURL(pEntry->aURL);
                }
            }
        }
    }

    return pSgaObj;
}

bool GalleryBinaryEngine::implWriteSgaObject(
    const SgaObject& rObj, sal_uInt32 nPos, GalleryObject* pExistentEntry, OUString& aDestDir,
    ::std::vector<std::unique_ptr<GalleryObject>>& rObjectList)
{
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
        GetSdgURL().GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE));
    bool bRet = false;

    if (pOStm)
    {
        const sal_uInt32 nOffset = pOStm->Seek(STREAM_SEEK_TO_END);

        rObj.WriteData(*pOStm, aDestDir);

        if (!pOStm->GetError())
        {
            GalleryObject* pEntry;

            if (!pExistentEntry)
            {
                pEntry = new GalleryObject;
                if (nPos < rObjectList.size())
                {
                    rObjectList.emplace(rObjectList.begin() + nPos, pEntry);
                }
                else
                    rObjectList.emplace_back(pEntry);
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

bool GalleryBinaryEngine::implWrite(const GalleryTheme& rTheme)
{
    INetURLObject aPathURL(GetThmURL());

    aPathURL.removeSegment();
    aPathURL.removeFinalSlash();

    DBG_ASSERT(aPathURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    if (FileExists(aPathURL) || CreateDir(aPathURL))
    {
#ifdef UNX
        std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
            GetThmURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
            StreamMode::WRITE | StreamMode::COPY_ON_SYMLINK | StreamMode::TRUNC));
#else
        std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
            GetThmURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
            StreamMode::WRITE | StreamMode::TRUNC));
#endif

        if (pOStm)
        {
            WriteGalleryTheme(*pOStm, rTheme);
            pOStm.reset();
            return true;
        }

        return false;
    }
    return true;
}

void GalleryBinaryEngine::ImplCreateSvDrawStorage(bool bReadOnly)
{
    try
    {
        aSvDrawStorageRef
            = new SotStorage(false, GetSdvURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
                             bReadOnly ? StreamMode::READ : StreamMode::STD_READWRITE);
        // #i50423# ReadOnly may not been set though the file can't be written (because of security reasons)
        if ((aSvDrawStorageRef->GetError() != ERRCODE_NONE) && !bReadOnly)
            aSvDrawStorageRef = new SotStorage(
                false, GetSdvURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
                StreamMode::READ);
    }
    catch (const css::ucb::ContentCreationException&)
    {
        TOOLS_WARN_EXCEPTION("svx", "failed to open: " << GetSdvURL().GetMainURL(
                                                              INetURLObject::DecodeMechanism::NONE)
                                                       << "due to");
    }
}

const tools::SvRef<SotStorage>& GalleryBinaryEngine::GetSvDrawStorage() const
{
    return aSvDrawStorageRef;
}

bool GalleryBinaryEngine::readModel(const GalleryObject* pObject, SdrModel& rModel)
{
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());
    bool bRet = false;
    const INetURLObject aURL(ImplGetURL(pObject));

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(aURL));
        tools::SvRef<SotStorageStream> xInputStream(
            xSotStorage->OpenSotStream(aStreamName, StreamMode::READ));

        if (xInputStream.is() && !xInputStream->GetError())
        {
            xInputStream->SetBufferSize(STREAMBUF_SIZE);
            bRet = GallerySvDrawImport(*xInputStream, rModel);
            xInputStream->SetBufferSize(0);
        }
    }
    return bRet;
}

bool GalleryBinaryEngine::insertModel(const FmFormModel& rModel, INetURLObject& rURL)
{
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());
    bool bRet = false;

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(rURL));
        tools::SvRef<SotStorageStream> xOutputStream(
            xSotStorage->OpenSotStream(aStreamName, StreamMode::WRITE | StreamMode::TRUNC));

        if (xOutputStream.is() && !xOutputStream->GetError())
        {
            SvMemoryStream aMemoryStream(65535, 65535);
            FmFormModel* pFormModel = const_cast<FmFormModel*>(&rModel);

            pFormModel->BurnInStyleSheetAttributes();

            {
                uno::Reference<io::XOutputStream> xDocOut(
                    new utl::OOutputStreamWrapper(aMemoryStream));

                if (xDocOut.is())
                    (void)SvxDrawingLayerExport(pFormModel, xDocOut);
            }

            aMemoryStream.Seek(0);

            xOutputStream->SetBufferSize(16348);
            GalleryCodec aCodec(*xOutputStream);
            aCodec.Write(aMemoryStream);

            xOutputStream->SetBufferSize(0);
            xOutputStream->Commit();
            bRet = !xOutputStream->GetError();
        }
    }
    return bRet;
}

bool GalleryBinaryEngine::readModelStream(const GalleryObject* pObject,
                                          tools::SvRef<SotStorageStream> const& rxModelStream)
{
    const INetURLObject aURL(ImplGetURL(pObject));
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());
    bool bRet = false;

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(aURL));
        tools::SvRef<SotStorageStream> xInputStream(
            xSotStorage->OpenSotStream(aStreamName, StreamMode::READ));

        if (xInputStream.is() && !xInputStream->GetError())
        {
            sal_uInt32 nVersion = 0;

            xInputStream->SetBufferSize(16348);

            if (GalleryCodec::IsCoded(*xInputStream, nVersion))
            {
                SvxGalleryDrawModel aModel;

                if (aModel.GetModel())
                {
                    if (GallerySvDrawImport(*xInputStream, *aModel.GetModel()))
                    {
                        aModel.GetModel()->BurnInStyleSheetAttributes();

                        {
                            uno::Reference<io::XOutputStream> xDocOut(
                                new utl::OOutputStreamWrapper(*rxModelStream));

                            if (SvxDrawingLayerExport(aModel.GetModel(), xDocOut))
                                rxModelStream->Commit();
                        }
                    }

                    bRet = (rxModelStream->GetError() == ERRCODE_NONE);
                }
            }

            xInputStream->SetBufferSize(0);
        }
    }
    return bRet;
}

SgaObjectSvDraw
GalleryBinaryEngine::insertModelStream(const tools::SvRef<SotStorageStream>& rxModelStream,
                                       INetURLObject& rURL)
{
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(rURL));
        tools::SvRef<SotStorageStream> xOutputStream(
            xSotStorage->OpenSotStream(aStreamName, StreamMode::WRITE | StreamMode::TRUNC));

        if (xOutputStream.is() && !xOutputStream->GetError())
        {
            GalleryCodec aCodec(*xOutputStream);

            xOutputStream->SetBufferSize(16348);
            aCodec.Write(*rxModelStream);

            if (!xOutputStream->GetError())
            {
                xOutputStream->Seek(0);
                SgaObjectSvDraw aObjSvDraw(*xOutputStream, rURL);
                return aObjSvDraw;
            }
        }
    }
    return SgaObjectSvDraw();
}

void GalleryBinaryEngine::insertObject(const SgaObject& rObj, GalleryObject* pFoundEntry,
                                       OUString& rDestDir,
                                       ::std::vector<std::unique_ptr<GalleryObject>>& rObjectList,
                                       sal_uInt32& rInsertPos)
{
    if (pFoundEntry)
    {
        GalleryObject aNewEntry;

        // update title of new object if necessary
        if (rObj.GetTitle().isEmpty())
        {
            std::unique_ptr<SgaObject> pOldObj(implReadSgaObject(pFoundEntry));

            if (pOldObj)
            {
                const_cast<SgaObject&>(rObj).SetTitle(pOldObj->GetTitle());
            }
        }
        else if (rObj.GetTitle() == "__<empty>__")
            const_cast<SgaObject&>(rObj).SetTitle("");

        implWriteSgaObject(rObj, rInsertPos, &aNewEntry, rDestDir, rObjectList);
        pFoundEntry->nOffset = aNewEntry.nOffset;
    }
    else
        implWriteSgaObject(rObj, rInsertPos, nullptr, rDestDir, rObjectList);
}

GalleryThemeEntry* GalleryBinaryEngine::CreateThemeEntry(const INetURLObject& rURL, bool bReadOnly)
{
    DBG_ASSERT(rURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    GalleryThemeEntry* pRet = nullptr;

    if (FileExists(rURL))
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
            rURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

        if (pIStm)
        {
            OUString aThemeName;
            sal_uInt16 nVersion;

            pIStm->ReadUInt16(nVersion);

            if (nVersion <= 0x00ff)
            {
                bool bThemeNameFromResource = false;
                sal_uInt32 nThemeId = 0;

                OString aTmpStr = read_uInt16_lenPrefixed_uInt8s_ToOString(*pIStm);
                aThemeName = OStringToOUString(aTmpStr, RTL_TEXTENCODING_UTF8);

                // execute a character conversion
                if (nVersion >= 0x0004)
                {
                    sal_uInt32 nCount;
                    sal_uInt16 nTemp16;

                    pIStm->ReadUInt32(nCount).ReadUInt16(nTemp16);
                    pIStm->Seek(STREAM_SEEK_TO_END);

                    // check whether there is a newer version;
                    // therefore jump back by 520Bytes (8 bytes ID + 512Bytes reserve buffer)
                    // if this is at all possible.
                    if (pIStm->Tell() >= 520)
                    {
                        sal_uInt32 nId1, nId2;

                        pIStm->SeekRel(-520);
                        pIStm->ReadUInt32(nId1).ReadUInt32(nId2);

                        if (nId1 == COMPAT_FORMAT('G', 'A', 'L', 'R')
                            && nId2 == COMPAT_FORMAT('E', 'S', 'R', 'V'))
                        {
                            VersionCompat aCompat(*pIStm, StreamMode::READ);

                            pIStm->ReadUInt32(nThemeId);

                            if (aCompat.GetVersion() >= 2)
                            {
                                pIStm->ReadCharAsBool(bThemeNameFromResource);
                            }
                        }
                    }
                }

                pRet = new GalleryThemeEntry(false, rURL, aThemeName, bReadOnly, false, nThemeId,
                                             bThemeNameFromResource);
            }
        }
    }

    return pRet;
}

SvStream& WriteGalleryTheme(SvStream& rOut, const GalleryTheme& rTheme)
{
    return rTheme.WriteData(rOut);
}
