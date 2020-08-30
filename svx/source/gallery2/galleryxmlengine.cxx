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

#include <svx/galleryxmlengine.hxx>
#include <svx/galleryobjectcollection.hxx>
#include <svx/galtheme.hxx>
#include <svx/gallery1.hxx>
#include <svx/galleryxmlstoragelocations.hxx>
#include <galobj.hxx>

#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/XmlWriter.hxx>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>

#include <osl/thread.hxx>
#include <rtl/ustring.hxx>

#include <sal/log.hxx>

#include <tools/urlobj.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/XmlWriter.hxx>

#include <sal/log.hxx>

using namespace ::com::sun::star;

GalleryXMLEngine::GalleryXMLEngine(const GalleryXMLStorageLocations& rGalleryStorageLocations,
                                   GalleryObjectCollection& rGalleryObjectCollection,
                                   bool bReadOnly)
    : maGalleryStorageLocations(rGalleryStorageLocations)
    , mrGalleryObjectCollection(rGalleryObjectCollection)
    , mbReadOnly(bReadOnly)
{
}

void GalleryXMLEngine::writeMimeType(uno::Reference<embed::XStorage>& rxWriteableZipStore)
{
    // Write mimetype to an existing ZIP file
    css::uno::Reference<css::io::XOutputStream> xOutputStream;
    xOutputStream.set(
        rxWriteableZipStore->openStreamElement("mimetype", embed::ElementModes::READWRITE),
        uno::UNO_QUERY);
    const OString aMimeType("application/gallerytheme+zip");
    uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(aMimeType.getStr()),
                                  aMimeType.getLength());
    xOutputStream->writeBytes(aData);

    uno::Reference<embed::XTransactedObject> xTransact(rxWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();
    xTransact.set(rxWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();
}

uno::Reference<embed::XStorage> GalleryXMLEngine::writeZipFile(const OUString& rZipFile)
{
    // Write ZipFile
    std::unique_ptr<SvStream> pStream(
        utl::UcbStreamHelper::CreateStream(rZipFile, StreamMode::READ | StreamMode::WRITE));
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    uno::Reference<embed::XStorage> xWriteableZipStore;
    xWriteableZipStore = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
        ZIP_STORAGE_FORMAT_STRING, xStream);

    uno::Reference<embed::XTransactedObject> xTransact(xWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();
    xTransact.set(xWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();

    return xWriteableZipStore;
}

SvStream& GalleryXMLEngine::writeGalleryTheme(SvStream& rOStrm, const GalleryTheme& rTheme,
                                              const GalleryThemeEntry* pThm)
{
    const OUString sThemeName = pThm->GetThemeName();

    OUString aZipFile = "file:///C:/Users/adith/Desktop/zip testing/" + sThemeName + ".zip";
    uno::Reference<embed::XStorage> xWriteableZipStore = writeZipFile(aZipFile);
    writeMimeType(xWriteableZipStore);

    // Currently, Stream is unknown so for test purposes custom rOStm was used, will replace rOStrm with rOStm as soon as I know the correct rOStm
    rOStrm.GetVersion();

    const INetURLObject rRelURL1 = rTheme.GetParent()->GetRelativeURL();
    const INetURLObject rRelURL2 = rTheme.GetParent()->GetUserURL();
    const sal_uInt32 rId = rTheme.GetId();
    bool bRel;

    sal_uInt32 nCount = mrGalleryObjectCollection.size();

    std::unique_ptr<SvMemoryStream> pMemoryStream = std::make_unique<SvMemoryStream>();
    tools::XmlWriter aXmlWriter(pMemoryStream.get());

    if (aXmlWriter.startDocument())
    {
        aXmlWriter.startElement("Theme-Information");

        //  write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, pThm->GetThemeName(), RTL_TEXTENCODING_UTF8);
        aXmlWriter.startElement("Name");
        aXmlWriter.content(OUStringToOString(sThemeName, RTL_TEXTENCODING_UTF8));
        aXmlWriter.endElement();

        // rOStm.WriteUInt32(nCount).WriteUInt16(osl_getThreadTextEncoding());
        aXmlWriter.startElement("Object-Count");
        aXmlWriter.content(OString::number(nCount));
        aXmlWriter.endElement();

        aXmlWriter.startElement("Text-Encoding");
        aXmlWriter.content(OString(osl_getThreadTextEncoding()));
        aXmlWriter.endElement();

        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            OString CurrentObject("Object-" + OString::number(nCount));
            aXmlWriter.startElement(CurrentObject);

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
                    0,
                    std::min(rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                             aPath.getLength()));
                bRel = aPath == rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE);

                if (bRel
                    && (pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength()
                        > (rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength()
                           + 1)))
                {
                    aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
                    aPath = aPath.copy(std::min(
                        rRelURL1.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                        aPath.getLength()));
                }
                else
                {
                    aPath = pObj->getURL().GetMainURL(INetURLObject::DecodeMechanism::NONE);
                    aPath = aPath.copy(
                        0,
                        std::min(
                            rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE).getLength(),
                            aPath.getLength()));
                    bRel = aPath == rRelURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE);

                    if (bRel
                        && (pObj->getURL()
                                .GetMainURL(INetURLObject::DecodeMechanism::NONE)
                                .getLength()
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

            /*if (!m_aDestDir.isEmpty())
            {
                bool aFound = aPath.indexOf(m_aDestDir) != -1;
                aPath = aPath.replaceFirst(m_aDestDir, "");
                if (aFound)
                    bRel = m_bDestDirRelative;
                else
                    SAL_WARN("svx", "failed to replace destdir of '" << m_aDestDir << "' in '" << aPath
                        << "'");
            }*/

            //rOStm.WriteBool(bRel);
            aXmlWriter.startElement("bRel");
            aXmlWriter.content(OString::boolean(bRel));
            aXmlWriter.endElement();

            //write_uInt16_lenPrefixed_uInt8s_FromOUString(rOStm, aPath, RTL_TEXTENCODING_UTF8);
            aXmlWriter.startElement("Path");
            aXmlWriter.content(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8));
            aXmlWriter.endElement();

            //rOStm.WriteUInt32(pObj->nOffset).WriteUInt16(static_cast<sal_uInt16>(pObj->eObjKind));
            aXmlWriter.startElement("Offset");
            aXmlWriter.content(OString::number(pObj->nOffset));
            aXmlWriter.endElement();

            aXmlWriter.startElement("Object-Kind");
            aXmlWriter.content(OString::number(static_cast<sal_uInt16>(pObj->eObjKind)));
            aXmlWriter.endElement();

            aXmlWriter.endElement();
        }

        //rOStm.WriteUInt32(rId).WriteBool(pThm->IsNameFromResource()); // From version 2 and up
        aXmlWriter.startElement("Id");
        aXmlWriter.content(OString::number(rId));
        aXmlWriter.endElement();

        aXmlWriter.startElement("IsNameFromResource");
        aXmlWriter.content(OString::boolean(pThm->IsNameFromResource()));
        aXmlWriter.endElement();

        aXmlWriter.endElement();

        aXmlWriter.endDocument();
    }
    else
        SAL_WARN("svx", "failed to start document");

    pMemoryStream->Seek(0);

    css::uno::Reference<css::io::XOutputStream> xOutputStream;
    xOutputStream.set(
        xWriteableZipStore->openStreamElement(sThemeName + ".xml", embed::ElementModes::READWRITE),
        uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xInputStream
        = new utl::OInputStreamWrapper(pMemoryStream.get());
    comphelper::OStorageHelper::CopyInputToOutput(xInputStream, xOutputStream);

    uno::Reference<embed::XTransactedObject> xTransact(xWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();
    xTransact.set(xWriteableZipStore, uno::UNO_QUERY_THROW);
    xTransact->commit();

    return *pMemoryStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
