/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdf/objectcopier.hxx>

#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <vcl/filter/pdfobjectcontainer.hxx>

#include "pdfwriter_impl.hxx"

namespace vcl
{
PDFObjectCopier::PDFObjectCopier(PDFObjectContainer& rContainer)
    : m_rContainer(rContainer)
{
}

void PDFObjectCopier::copyRecursively(OStringBuffer& rLine, filter::PDFElement* pInputElement,
                                      SvMemoryStream& rDocBuffer,
                                      std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    if (auto pReference = dynamic_cast<filter::PDFReferenceElement*>(pInputElement))
    {
        filter::PDFObjectElement* pReferenced = pReference->LookupObject();
        if (pReferenced)
        {
            // Copy the referenced object.
            sal_Int32 nRef = copyExternalResource(rDocBuffer, *pReferenced, rCopiedResources);

            // Write the updated reference.
            rLine.append(nRef);
            rLine.append(" 0 R");
        }
    }
    else if (auto pInputArray = dynamic_cast<filter::PDFArrayElement*>(pInputElement))
    {
        rLine.append("[ ");
        for (auto const& pElement : pInputArray->GetElements())
        {
            copyRecursively(rLine, pElement, rDocBuffer, rCopiedResources);
            rLine.append(" ");
        }
        rLine.append("] ");
    }
    else if (auto pInputDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pInputElement))
    {
        rLine.append("<< ");
        for (auto const& pPair : pInputDictionary->GetItems())
        {
            rLine.append("/");
            rLine.append(pPair.first);
            rLine.append(" ");
            copyRecursively(rLine, pPair.second, rDocBuffer, rCopiedResources);
            rLine.append(" ");
        }
        rLine.append(">> ");
    }
    else
    {
        pInputElement->writeString(rLine);
    }
}

sal_Int32 PDFObjectCopier::copyExternalResource(SvMemoryStream& rDocBuffer,
                                                filter::PDFObjectElement& rObject,
                                                std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    auto it = rCopiedResources.find(rObject.GetObjectValue());
    if (it != rCopiedResources.end())
    {
        // This resource was already copied once, nothing to do.
        return it->second;
    }

    sal_Int32 nObject = m_rContainer.createObject();
    // Remember what is the ID of this object in our output.
    rCopiedResources[rObject.GetObjectValue()] = nObject;
    SAL_INFO("vcl.pdfwriter", "PDFObjectCopier::copyExternalResource: " << rObject.GetObjectValue()
                                                                        << " -> " << nObject);

    SvMemoryStream* pObjectStream = rObject.GetStreamBuffer();
    if (!pObjectStream)
    {
        pObjectStream = &rDocBuffer;
    }

    OStringBuffer aLine;
    aLine.append(nObject);
    aLine.append(" 0 obj\n");

    if (rObject.GetDictionary())
    {
        aLine.append("<< ");
        bool bFirst = true;
        for (auto const& rPair : rObject.GetDictionaryItems())
        {
            if (bFirst)
                bFirst = false;
            else
                aLine.append(" ");

            aLine.append("/");
            aLine.append(rPair.first);
            aLine.append(" ");
            copyRecursively(aLine, rPair.second, rDocBuffer, rCopiedResources);
        }

        aLine.append(" >>\n");
    }

    if (filter::PDFStreamElement* pStream = rObject.GetStream())
    {
        aLine.append("stream\n");
        SvMemoryStream& rStream = pStream->GetMemory();
        aLine.append(static_cast<const char*>(rStream.GetData()), rStream.GetSize());
        aLine.append("\nendstream\n");
    }

    if (filter::PDFArrayElement* pArray = rObject.GetArray())
    {
        aLine.append("[ ");

        const std::vector<filter::PDFElement*>& rElements = pArray->GetElements();

        bool bFirst = true;
        for (auto const& pElement : rElements)
        {
            if (bFirst)
                bFirst = false;
            else
                aLine.append(" ");
            copyRecursively(aLine, pElement, rDocBuffer, rCopiedResources);
        }
        aLine.append("]\n");
    }

    // If the object has a number element outside a dictionary or array, copy that.
    if (filter::PDFNumberElement* pNumber = rObject.GetNumberElement())
    {
        pNumber->writeString(aLine);
        aLine.append("\n");
    }

    aLine.append("endobj\n\n");

    // We have the whole object, now write it to the output.
    if (!m_rContainer.updateObject(nObject))
        return -1;
    if (!m_rContainer.writeBuffer(aLine.getStr(), aLine.getLength()))
        return -1;

    return nObject;
}

OString PDFObjectCopier::copyExternalResources(filter::PDFObjectElement& rPage,
                                               const OString& rKind,
                                               std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    // A name - object ID map, IDs as they appear in our output, not the
    // original ones.
    std::map<OString, sal_Int32> aRet;

    // Get the rKind subset of the resource dictionary.
    std::map<OString, filter::PDFElement*> aItems;
    if (auto pResources = dynamic_cast<filter::PDFDictionaryElement*>(rPage.Lookup("Resources")))
    {
        // Resources is a direct dictionary.
        filter::PDFElement* pLookup = pResources->LookupElement(rKind);
        if (auto pDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pLookup))
        {
            // rKind is an inline dictionary.
            aItems = pDictionary->GetItems();
        }
        else if (auto pReference = dynamic_cast<filter::PDFReferenceElement*>(pLookup))
        {
            // rKind refers to a dictionary.
            filter::PDFObjectElement* pReferenced = pReference->LookupObject();
            if (!pReferenced)
            {
                return OString();
            }

            aItems = pReferenced->GetDictionaryItems();
        }
    }
    else if (filter::PDFObjectElement* pPageResources = rPage.LookupObject("Resources"))
    {
        // Resources is an indirect object.
        filter::PDFElement* pValue = pPageResources->Lookup(rKind);
        if (auto pDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pValue))
            // Kind is a direct dictionary.
            aItems = pDictionary->GetItems();
        else if (filter::PDFObjectElement* pObject = pPageResources->LookupObject(rKind))
            // Kind is an indirect object.
            aItems = pObject->GetDictionaryItems();
    }
    if (aItems.empty())
        return OString();

    SvMemoryStream& rDocBuffer = rPage.GetDocument().GetEditBuffer();

    for (const auto& rItem : aItems)
    {
        // For each item copy it over to our output then insert it into aRet.
        auto pReference = dynamic_cast<filter::PDFReferenceElement*>(rItem.second);
        if (!pReference)
            continue;

        filter::PDFObjectElement* pValue = pReference->LookupObject();
        if (!pValue)
            continue;

        // Then copying over an object copy its dictionary and its stream.
        sal_Int32 nObject = copyExternalResource(rDocBuffer, *pValue, rCopiedResources);
        aRet[rItem.first] = nObject;
    }

    // Build the dictionary entry string.
    OStringBuffer sRet("/" + rKind + "<<");
    for (const auto& rPair : aRet)
    {
        sRet.append("/" + rPair.first + " ").append(rPair.second).append(" 0 R");
    }
    sRet.append(">>");

    return sRet.makeStringAndClear();
}

void PDFObjectCopier::copyPageResources(filter::PDFObjectElement* pPage, OStringBuffer& rLine)
{
    // Maps from source object id (PDF image) to target object id (export result).
    std::map<sal_Int32, sal_Int32> aCopiedResources;
    copyPageResources(pPage, rLine, aCopiedResources);
}

void PDFObjectCopier::copyPageResources(filter::PDFObjectElement* pPage, OStringBuffer& rLine,
                                        std::map<sal_Int32, sal_Int32>& rCopiedResources)
{
    rLine.append(" /Resources <<");
    static const std::initializer_list<OString> aKeys
        = { "ColorSpace", "ExtGState", "Font", "XObject", "Shading" };
    for (const auto& rKey : aKeys)
    {
        rLine.append(copyExternalResources(*pPage, rKey, rCopiedResources));
    }
    rLine.append(">>");
}

sal_Int32 PDFObjectCopier::copyPageStreams(std::vector<filter::PDFObjectElement*>& rContentStreams,
                                           SvMemoryStream& rStream, bool& rCompressed)
{
    for (auto pContent : rContentStreams)
    {
        filter::PDFStreamElement* pPageStream = pContent->GetStream();
        if (!pPageStream)
        {
            SAL_WARN("vcl.pdfwriter", "PDFObjectCopier::copyPageStreams: contents has no stream");
            continue;
        }

        SvMemoryStream& rPageStream = pPageStream->GetMemory();

        auto pFilter = dynamic_cast<filter::PDFNameElement*>(pContent->Lookup("Filter"));
        if (pFilter)
        {
            if (pFilter->GetValue() != "FlateDecode")
            {
                continue;
            }

            SvMemoryStream aMemoryStream;
            ZCodec aZCodec;
            rPageStream.Seek(0);
            aZCodec.BeginCompression();
            aZCodec.Decompress(rPageStream, aMemoryStream);
            if (!aZCodec.EndCompression())
            {
                SAL_WARN("vcl.pdfwriter", "PDFObjectCopier::copyPageStreams: decompression failed");
                continue;
            }

            rStream.WriteBytes(aMemoryStream.GetData(), aMemoryStream.GetSize());
        }
        else
        {
            rStream.WriteBytes(rPageStream.GetData(), rPageStream.GetSize());
        }
    }

    rCompressed = PDFWriterImpl::compressStream(&rStream);

    return rStream.Tell();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
