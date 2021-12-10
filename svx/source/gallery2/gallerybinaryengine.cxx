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
#include <svx/galleryobjectcollection.hxx>
#include <svx/gallery1.hxx>
#include <galleryobjectbinarystorage.hxx>
#include <osl/thread.hxx>
#include "codec.hxx"
#include "gallerydrawmodel.hxx"
#include <vcl/cvtgrf.hxx>
#include <vcl/filter/SvmWriter.hxx>

#include <sal/log.hxx>

#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <comphelper/fileformat.h>
#include <comphelper/graphicmimetype.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/content.hxx>
#include <tools/vcompat.hxx>

using namespace ::com::sun::star;

GalleryBinaryEngine::GalleryBinaryEngine(
    const GalleryBinaryStorageLocations& rGalleryBinaryStorageLocations,
    GalleryObjectCollection& rGalleryObjectCollection, bool bReadOnly)
    : maGalleryStorageLocations(rGalleryBinaryStorageLocations)
    , mrGalleryObjectCollection(rGalleryObjectCollection)
    , mbReadOnly(bReadOnly)
    , m_bDestDirRelative(false)
{
    ImplCreateSvDrawStorage();
}

GalleryBinaryEngine::~GalleryBinaryEngine() { clearSotStorage(); }

void GalleryBinaryEngine::setDestDir(const OUString& rDestDir, bool bRelative)
{
    m_aDestDir = rDestDir;
    m_bDestDirRelative = bRelative;
}

void GalleryBinaryEngine::clearSotStorage() { m_aSvDrawStorageRef.clear(); }

void GalleryBinaryEngine::ImplCreateSvDrawStorage()
{
    try
    {
        m_aSvDrawStorageRef
            = new SotStorage(false, GetSdvURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
                             mbReadOnly ? StreamMode::READ : StreamMode::STD_READWRITE);
        // #i50423# ReadOnly may not been set though the file can't be written (because of security reasons)
        if ((m_aSvDrawStorageRef->GetError() != ERRCODE_NONE) && !mbReadOnly)
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

bool GalleryBinaryEngine::implWrite(const GalleryTheme& rTheme, const GalleryThemeEntry* pThm)
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
            writeGalleryTheme(*pOStm, rTheme, pThm);
            pOStm.reset();
            return true;
        }

        return false;
    }
    return true;
}

void GalleryBinaryEngine::insertObject(const SgaObject& rObj, GalleryObject* pFoundEntry,
                                       sal_uInt32 nInsertPos)
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

        implWriteSgaObject(rObj, nInsertPos, &aNewEntry);
        pFoundEntry->nOffset = aNewEntry.nOffset;
    }
    else
        implWriteSgaObject(rObj, nInsertPos, nullptr);
}

void GalleryBinaryEngine::removeObject(const std::unique_ptr<GalleryObject>& pEntry)
{
    if (mrGalleryObjectCollection.getObjectList().empty())
        KillFile(GetSdgURL());

    if (SgaObjKind::SvDraw == pEntry->eObjKind)
        GetSvDrawStorage()->Remove(
            pEntry->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE));
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
                    pSgaObj->ImplUpdateURL(pEntry->getURL());
                }
            }
        }
    }

    return pSgaObj;
}

bool GalleryBinaryEngine::implWriteSgaObject(const SgaObject& rObj, sal_uInt32 nPos,
                                             GalleryObject* pExistentEntry)
{
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
        GetSdgURL().GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE));
    bool bRet = false;

    if (pOStm)
    {
        const sal_uInt32 nOffset = pOStm->Seek(STREAM_SEEK_TO_END);

        rObj.WriteData(*pOStm, m_aDestDir);

        if (!pOStm->GetError())
        {
            GalleryObject* pEntry;

            if (!pExistentEntry)
            {
                pEntry = new GalleryObject;
                if (nPos < mrGalleryObjectCollection.size())
                {
                    mrGalleryObjectCollection.getObjectList().emplace(
                        mrGalleryObjectCollection.getObjectList().begin() + nPos, pEntry);
                }
                else
                    mrGalleryObjectCollection.getObjectList().emplace_back(pEntry);
            }
            else
                pEntry = pExistentEntry;

            pEntry->m_pGalleryObjectStorage = std::make_unique<GalleryObjectBinaryStorage>();
            pEntry->m_pGalleryObjectStorage->setURL(rObj.GetURL());

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

SgaObjectSvDraw GalleryBinaryEngine::insertModel(const FmFormModel& rModel,
                                                 const INetURLObject& rUserURL)
{
    INetURLObject aURL(implCreateUniqueURL(SgaObjKind::SvDraw, rUserURL));
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());
    bool bRet = false;

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(aURL));
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
    if (bRet)
    {
        SgaObjectSvDraw aObjSvDraw(rModel, aURL);
        return aObjSvDraw;
    }
    return SgaObjectSvDraw();
}

bool GalleryBinaryEngine::readModelStream(const GalleryObject* pObject,
                                          tools::SvRef<SotTempStream> const& rxModelStream)
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

                            SvxDrawingLayerExport(aModel.GetModel(), xDocOut);
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
GalleryBinaryEngine::insertModelStream(const tools::SvRef<SotTempStream>& rxModelStream,
                                       const INetURLObject& rUserURL)
{
    INetURLObject aURL(implCreateUniqueURL(SgaObjKind::SvDraw, rUserURL));
    tools::SvRef<SotStorage> xSotStorage(GetSvDrawStorage());

    if (xSotStorage.is())
    {
        const OUString aStreamName(GetSvDrawStreamNameFromURL(aURL));
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
                SgaObjectSvDraw aObjSvDraw(*xOutputStream, aURL);
                return aObjSvDraw;
            }
        }
    }
    return SgaObjectSvDraw();
}

INetURLObject GalleryBinaryEngine::implCreateUniqueURL(SgaObjKind eObjKind,
                                                       const INetURLObject& rUserURL,
                                                       ConvertDataFormat nFormat)
{
    INetURLObject aDir(rUserURL);
    INetURLObject aInfoFileURL(rUserURL);
    INetURLObject aNewURL;
    sal_uInt32 nNextNumber = 1999;
    char const* pExt = nullptr;
    bool bExists;

    aDir.Append(u"dragdrop");
    CreateDir(aDir);

    aInfoFileURL.Append(u"sdddndx1");

    // read next possible number
    if (FileExists(aInfoFileURL))
    {
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
            aInfoFileURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));

        if (pIStm)
        {
            pIStm->ReadUInt32(nNextNumber);
        }
    }

    pExt = comphelper::GraphicMimeTypeHelper::GetExtensionForConvertDataFormat(nFormat);

    do
    {
        // get URL
        if (SgaObjKind::SvDraw == eObjKind)
        {
            OUString aFileName = "gallery/svdraw/dd" + OUString::number(++nNextNumber % 99999999);
            aNewURL = INetURLObject(aFileName, INetProtocol::PrivSoffice);

            bExists = false;

            for (auto const& pObject : mrGalleryObjectCollection.getObjectList())
            {
                if (pObject->getURL() == aNewURL)
                {
                    bExists = true;
                    break;
                }
            }
        }
        else
        {
            OUString aFileName = "dd" + OUString::number(++nNextNumber % 999999);

            if (pExt)
                aFileName += OUString(pExt, strlen(pExt), RTL_TEXTENCODING_ASCII_US);

            aNewURL = aDir;
            aNewURL.Append(aFileName);

            bExists = FileExists(aNewURL);
        }
    } while (bExists);

    // write updated number
    std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream(
        aInfoFileURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE));

    if (pOStm)
    {
        pOStm->WriteUInt32(nNextNumber);
    }

    return aNewURL;
}

SgaObjectBmp GalleryBinaryEngine::insertGraphic(const Graphic& rGraphic, const GfxLink& aGfxLink,
                                                const ConvertDataFormat& nExportFormat,
                                                const INetURLObject& rUserURL)
{
    const INetURLObject aURL(implCreateUniqueURL(SgaObjKind::Bitmap, rUserURL, nExportFormat));
    std::unique_ptr<SvStream> pOStm(
        ::utl::UcbStreamHelper::CreateStream(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                                             StreamMode::WRITE | StreamMode::TRUNC));
    bool bRet = false;

    if (pOStm)
    {
        pOStm->SetVersion(SOFFICE_FILEFORMAT_50);

        if (ConvertDataFormat::SVM == nExportFormat)
        {
            GDIMetaFile aMtf(rGraphic.GetGDIMetaFile());

            SvmWriter aWriter(*pOStm);
            aWriter.Write(aMtf);
            bRet = (pOStm->GetError() == ERRCODE_NONE);
        }
        else
        {
            if (aGfxLink.GetDataSize() && aGfxLink.GetData())
            {
                pOStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                bRet = (pOStm->GetError() == ERRCODE_NONE);
            }
            else
                bRet = (GraphicConverter::Export(*pOStm, rGraphic, nExportFormat) == ERRCODE_NONE);
        }

        pOStm.reset();
    }
    if (bRet)
    {
        const SgaObjectBmp aObjBmp(aURL);
        return aObjBmp;
    }
    return SgaObjectBmp();
}

SgaObjectSvDraw GalleryBinaryEngine::updateSvDrawObject(const GalleryObject* pEntry)
{
    if (GetSvDrawStorage().is())
    {
        const OUString aStmName(GetSvDrawStreamNameFromURL(pEntry->getURL()));
        tools::SvRef<SotStorageStream> pIStm
            = GetSvDrawStorage()->OpenSotStream(aStmName, StreamMode::READ);

        if (pIStm.is() && !pIStm->GetError())
        {
            pIStm->SetBufferSize(16384);

            SgaObjectSvDraw aNewObj(*pIStm, pEntry->getURL());

            pIStm->SetBufferSize(0);

            return aNewObj;
        }
    }
    return SgaObjectSvDraw();
}

void GalleryBinaryEngine::updateTheme()
{
    ::utl::TempFile aTmp;
    INetURLObject aInURL(GetSdgURL());
    INetURLObject aTmpURL(aTmp.GetURL());

    DBG_ASSERT(aInURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");
    DBG_ASSERT(aTmpURL.GetProtocol() != INetProtocol::NotValid, "invalid URL");

    std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream(
        aInURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ));
    std::unique_ptr<SvStream> pTmpStm(::utl::UcbStreamHelper::CreateStream(
        aTmpURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
        StreamMode::WRITE | StreamMode::TRUNC));

    if (pIStm && pTmpStm)
    {
        for (const auto& i : mrGalleryObjectCollection.getObjectList())
        {
            GalleryObject* pEntry = i.get();
            std::unique_ptr<SgaObject> pObj;

            switch (pEntry->eObjKind)
            {
                case SgaObjKind::Bitmap:
                    pObj.reset(new SgaObjectBmp());
                    break;
                case SgaObjKind::Animation:
                    pObj.reset(new SgaObjectAnim());
                    break;
                case SgaObjKind::Inet:
                    pObj.reset(new SgaObjectINet());
                    break;
                case SgaObjKind::SvDraw:
                    pObj.reset(new SgaObjectSvDraw());
                    break;
                case SgaObjKind::Sound:
                    pObj.reset(new SgaObjectSound());
                    break;

                default:
                    break;
            }

            if (pObj)
            {
                pIStm->Seek(pEntry->nOffset);
                ReadSgaObject(*pIStm, *pObj);
                pEntry->nOffset = pTmpStm->Tell();
                WriteSgaObject(*pTmpStm, *pObj);
            }
        }
    }
    else
    {
        OSL_FAIL("File(s) could not be opened");
    }

    pIStm.reset();
    pTmpStm.reset();

    CopyFile(aTmpURL, aInURL);
    KillFile(aTmpURL);

    ErrCode nStorErr = ERRCODE_NONE;

    try
    {
        tools::SvRef<SotStorage> aTempStorageRef(
            new SotStorage(false, aTmpURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                           StreamMode::STD_READWRITE));
        GetSvDrawStorage()->CopyTo(aTempStorageRef.get());
        nStorErr = GetSvDrawStorage()->GetError();
    }
    catch (const css::ucb::ContentCreationException&)
    {
        TOOLS_WARN_EXCEPTION("svx", "failed to open: "
                                        << aTmpURL.GetMainURL(INetURLObject::DecodeMechanism::NONE)
                                        << "due to");
        nStorErr = ERRCODE_IO_GENERAL;
    }

    if (nStorErr == ERRCODE_NONE)
    {
        clearSotStorage();
        CopyFile(aTmpURL, GetSdvURL());
        ImplCreateSvDrawStorage();
    }

    KillFile(aTmpURL);
}

void GalleryBinaryEngine::insertFileOrDirURL(const INetURLObject& rFileOrDirURL,
                                             std::vector<INetURLObject>& rURLVector)
{
    INetURLObject aURL;
    try
    {
        ::ucbhelper::Content aCnt(rFileOrDirURL.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                                  uno::Reference<ucb::XCommandEnvironment>(),
                                  comphelper::getProcessComponentContext());
        bool bFolder = false;

        aCnt.getPropertyValue("IsFolder") >>= bFolder;

        if (bFolder)
        {
            uno::Sequence<OUString> aProps{ "Url" };
            uno::Reference<sdbc::XResultSet> xResultSet(
                aCnt.createCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY));
            uno::Reference<ucb::XContentAccess> xContentAccess(xResultSet, uno::UNO_QUERY);
            if (xContentAccess.is())
            {
                while (xResultSet->next())
                {
                    aURL.SetSmartURL(xContentAccess->queryContentIdentifierString());
                    rURLVector.push_back(aURL);
                }
            }
        }
        else
            rURLVector.push_back(rFileOrDirURL);
    }
    catch (const ucb::ContentCreationException&)
    {
    }
    catch (const uno::RuntimeException&)
    {
    }
    catch (const uno::Exception&)
    {
    }
}

SvStream& GalleryBinaryEngine::writeGalleryTheme(SvStream& rOStm, const GalleryTheme& rTheme,
                                                 const GalleryThemeEntry* pThm)
{
    const INetURLObject rRelURL1 = rTheme.GetParent()->GetRelativeURL();
    const INetURLObject rRelURL2 = rTheme.GetParent()->GetUserURL();
    const sal_uInt32 rId = rTheme.GetId();
    sal_uInt32 nCount = mrGalleryObjectCollection.size();
    bool bRel;

    rOStm.WriteUInt16(0x0004);
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, pThm->GetThemeName(),
                                                 RTL_TEXTENCODING_UTF8);
    rOStm.WriteUInt32(nCount).WriteUInt16(osl_getThreadTextEncoding());

    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        const GalleryObject* pObj = mrGalleryObjectCollection.getForPosition(i);
        OUString aPath;

        if (SgaObjKind::SvDraw == pObj->eObjKind)
        {
            aPath = GetSvDrawStreamNameFromURL(pObj->getURL());
            bRel = false;
        }
        else
        {
            aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
            aPath = aPath.copy(
                0, std::min(rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                            aPath.getLength()));
            bRel = aPath == rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE);

            if (bRel
                && (pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength()
                    > (rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength() + 1)))
            {
                aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
                aPath = aPath.copy(
                    std::min(rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                             aPath.getLength()));
            }
            else
            {
                aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
                aPath = aPath.copy(
                    0,
                    std::min(rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                             aPath.getLength()));
                bRel = aPath == rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE);

                if (bRel
                    && (pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength()
                        > (rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength()
                           + 1)))
                {
                    aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
                    aPath = aPath.copy(std::min(
                        rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                        aPath.getLength()));
                }
                else
                    aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
            }
        }

        if (!m_aDestDir.isEmpty())
        {
            bool aFound = aPath.indexOf(m_aDestDir) != -1;
            aPath = aPath.replaceFirst(m_aDestDir, "");
            if (aFound)
                bRel = m_bDestDirRelative;
            else
                SAL_WARN("svx", "failed to replace destdir of '" << m_aDestDir << "' in '" << aPath
                                                                 << "'");
        }

        rOStm.WriteBool(bRel);
        write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aPath, RTL_TEXTENCODING_UTF8);
        rOStm.WriteUInt32(pObj->nOffset).WriteUInt16(static_cast<sal_uInt16>(pObj->eObjKind));
    }

    // more recently, a 512-byte reserve buffer is written,
    // to recognize them two sal_uInt32-Ids will be written.
    rOStm.WriteUInt32(COMPAT_FORMAT('G', 'A', 'L', 'R'))
        .WriteUInt32(COMPAT_FORMAT('E', 'S', 'R', 'V'));

    const sal_uInt64 nReservePos = rOStm.Tell();
    std::unique_ptr<VersionCompatWrite> pCompat(new VersionCompatWrite(rOStm, 2));

    rOStm.WriteUInt32(rId).WriteBool(pThm->IsNameFromResource()); // From version 2 and up

    pCompat.reset();

    // Fill the rest of the buffer.
    const tools::Long nRest
        = std::max(tools::Long(512 - (rOStm.Tell() - nReservePos)), tools::Long(0));

    if (nRest)
    {
        std::unique_ptr<char[]> pReserve(new char[nRest]);
        memset(pReserve.get(), 0, nRest);
        rOStm.WriteBytes(pReserve.get(), nRest);
    }

    return rOStm;
}

DateTime GalleryBinaryEngine::getModificationDate() const
{
    ::ucbhelper::Content aCnt(GetThmURL().GetMainURL(INetURLObject::DecodeMechanism::NONE),
                              uno::Reference<ucb::XCommandEnvironment>(),
                              comphelper::getProcessComponentContext());
    util::DateTime aDateTimeModified;
    DateTime aDateTime(DateTime::EMPTY);

    aCnt.getPropertyValue("DateModified") >>= aDateTimeModified;
    ::utl::typeConvert(aDateTimeModified, aDateTime);

    return aDateTime;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
