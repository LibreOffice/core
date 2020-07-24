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

#include <svx/unomodel.hxx>
#include <svx/fmmodel.hxx>
#include <galobj.hxx>
#include <svx/gallerybinaryengine.hxx>
#include "codec.hxx"
#include "gallerydrawmodel.hxx"
#include <vcl/cvtgrf.hxx>

#include <sal/log.hxx>

#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/vcompat.hxx>

using namespace ::com::sun::star;

GalleryBinaryEngine::GalleryBinaryEngine(const GalleryStorageLocations& rGalleryStorageLocations)
    : maGalleryStorageLocations(rGalleryStorageLocations)
{
}

void GalleryBinaryEngine::clearSotStorage() { m_aSvDrawStorageRef.clear(); }

void GalleryBinaryEngine::ImplCreateSvDrawStorage(bool bReadOnly)
{
    try
    {
        m_aSvDrawStorageRef
            = new SotStorage(false, GetSdvURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
                             bReadOnly ? StreamMode::READ : StreamMode::STD_READWRITE);
        // #i50423# ReadOnly may not been set though the file can't be written (because of security reasons)
        if ((m_aSvDrawStorageRef->GetError() != ERRCODE_NONE) && !bReadOnly)
            m_aSvDrawStorageRef = new SotStorage(
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
    return m_aSvDrawStorageRef;
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

SvStream& WriteGalleryTheme(SvStream& rOut, const GalleryTheme& rTheme)
{
    return rTheme.WriteData(rOut);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
