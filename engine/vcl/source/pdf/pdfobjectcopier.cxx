/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/log.hxx>
#include <sal/types.h>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>

#include <vcl/filter/pdfdocument.hxx>
#include <vcl/filter/pdfobjectcontainer.hxx>

#include <pdf/objectcopier.hxx>
#include <pdf/pdfwriter_impl.hxx>

#include <o3tl/numeric.hxx>
#include <o3tl/string_view.hxx>

namespace vcl
{
namespace
{
// the bytes a literal string stands for, with the escapes of ISO 32000-2 7.3.4.2 resolved
OString decodeLiteral(std::string_view aValue)
{
    OStringBuffer aOut(aValue.size());
    for (size_t i = 0; i < aValue.size(); ++i)
    {
        if (aValue[i] == '\r' || aValue[i] == '\n')
        {
            // an end-of-line marker of its own stands for a line feed, whichever form it took
            if (aValue[i] == '\r' && i + 1 < aValue.size() && aValue[i + 1] == '\n')
                ++i;
            aOut.append('\n');
            continue;
        }
        if (aValue[i] != '\\' || i + 1 == aValue.size())
        {
            aOut.append(aValue[i]);
            continue;
        }
        const char cNext = aValue[++i];
        switch (cNext)
        {
            case 'n':
                aOut.append('\n');
                break;
            case 'r':
                aOut.append('\r');
                break;
            case 't':
                aOut.append('\t');
                break;
            case 'b':
                aOut.append('\b');
                break;
            case 'f':
                aOut.append('\f');
                break;
            case '\n':
                break; // a split line, which the string does not carry
            case '\r':
                if (i + 1 < aValue.size() && aValue[i + 1] == '\n')
                    ++i;
                break;
            default:
                if (rtl::isAsciiOctalDigit(static_cast<unsigned char>(cNext)))
                {
                    // one to three octal digits stand for a byte
                    size_t nDigits = 1;
                    for (unsigned char candidate : aValue.substr(i + 1, 2))
                    {
                        if (!rtl::isAsciiOctalDigit(candidate))
                            break;
                        ++nDigits;
                    }
                    aOut.append(static_cast<char>(o3tl::toInt32(aValue.substr(i, nDigits), 8)));
                    i += nDigits - 1;
                }
                else
                {
                    // a backslash before anything else is dropped
                    aOut.append(cNext);
                }
                break;
        }
    }
    return aOut.makeStringAndClear();
}

// the bytes a hex string stands for, per ISO 32000-2 7.3.4.3
OString decodeHex(std::string_view aValue)
{
    OStringBuffer aOut(aValue.size() / 2 + 1);
    int nHighNibble = -1;
    for (const char c : aValue)
    {
        const int nNibble = o3tl::convertToHex<int>(c);
        if (nNibble == -1)
        {
            // only whitespace may stand between the digits
            SAL_WARN_IF(!rtl::isAsciiWhiteSpace(static_cast<unsigned char>(c)) && c != 0,
                        "vcl.pdfwriter", "PDFObjectCopier: hex string has '" << c << "' in it");
            continue;
        }
        if (nHighNibble == -1)
        {
            nHighNibble = nNibble;
            continue;
        }
        aOut.append(static_cast<char>((nHighNibble << 4) | nNibble));
        nHighNibble = -1;
    }
    // an odd count of digits stands for a last byte whose low nibble is zero
    if (nHighNibble != -1)
        aOut.append(static_cast<char>(nHighNibble << 4));
    return aOut.makeStringAndClear();
}
}

PDFObjectCopier::PDFObjectCopier(PDFObjectContainer& rContainer)
    : m_rContainer(rContainer)
{
}

void PDFObjectCopier::copyRecursively(OStringBuffer& rLine, filter::PDFElement& rInputElement,
                                      SvMemoryStream& rDocBuffer,
                                      std::map<sal_Int32, sal_Int32>& rCopiedResources,
                                      sal_Int32 nObject)
{
    if (auto pReference = dynamic_cast<filter::PDFReferenceElement*>(&rInputElement))
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
    else if (auto pInputArray = dynamic_cast<filter::PDFArrayElement*>(&rInputElement))
    {
        rLine.append("[ ");
        for (auto const& pElement : pInputArray->GetElements())
        {
            copyRecursively(rLine, *pElement, rDocBuffer, rCopiedResources, nObject);
            rLine.append(" ");
        }
        rLine.append("] ");
    }
    else if (auto pInputDictionary = dynamic_cast<filter::PDFDictionaryElement*>(&rInputElement))
    {
        rLine.append("<< ");
        for (auto const& pPair : pInputDictionary->GetItems())
        {
            rLine.append("/");
            rLine.append(pPair.first);
            rLine.append(" ");
            copyRecursively(rLine, *pPair.second, rDocBuffer, rCopiedResources, nObject);
            rLine.append(" ");
        }
        rLine.append(">> ");
    }
    else if (auto pLiteral = dynamic_cast<filter::PDFLiteralStringElement*>(&rInputElement))
    {
        m_rContainer.appendStringObject(rLine, decodeLiteral(pLiteral->GetValue()), nObject);
    }
    else if (auto pHex = dynamic_cast<filter::PDFHexStringElement*>(&rInputElement))
    {
        m_rContainer.appendStringObject(rLine, decodeHex(pHex->GetValue()), nObject);
    }
    else
    {
        rInputElement.writeString(rLine);
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

    OStringBuffer aLine = OString::number(nObject) + " 0 obj\n";

    filter::PDFStreamElement* pStream = rObject.GetStream();

    // With encryption, the stream will be longer than the length of the
    // source document.
    sal_uInt64 const nDataSize{ pStream ? pStream->GetMemory().GetSize() : 0 };
    sal_uInt64 const nStreamSize{ m_rContainer.calculateStreamSize(nDataSize) };

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

            aLine.append("/" + rPair.first + " ");
            if (rPair.first == "Length" && pStream && nStreamSize != nDataSize)
            {
                aLine.append(static_cast<sal_Int64>(nStreamSize));
            }
            else
            {
                copyRecursively(aLine, *rPair.second, rDocBuffer, rCopiedResources, nObject);
            }
        }

        aLine.append(" >>\n");
    }

    if (pStream)
    {
        aLine.append("stream\n");
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
            copyRecursively(aLine, *pElement, rDocBuffer, rCopiedResources, nObject);
        }
        aLine.append("]\n");
    }

    // If the object has a number element outside a dictionary or array, copy that.
    if (filter::PDFNumberElement* pNumber = rObject.GetNumberElement())
    {
        pNumber->writeString(aLine);
        aLine.append("\n");
    }
    // If the object has a name element outside a dictionary or array, copy that.
    else if (filter::PDFNameElement* pName = rObject.GetNameElement())
    {
        // currently just handle the exact case seen in the real world
        if (pName->GetValue() == "DeviceRGB")
        {
            pName->writeString(aLine);
            aLine.append("\n");
        }
        else
        {
            SAL_INFO("vcl.pdfwriter",
                     "PDFObjectCopier::copyExternalResource: skipping: " << pName->GetValue());
        }
    }

    // We have the whole object, now write it to the output.
    if (!m_rContainer.updateObject(nObject))
        return -1;
    if (!m_rContainer.writeBuffer(aLine))
        return -1;
    aLine.setLength(0);

    if (pStream)
    {
        SvMemoryStream& rStream = pStream->GetMemory();
        m_rContainer.checkAndEnableStreamEncryption(nObject);
        aLine.append(static_cast<const char*>(rStream.GetData()), rStream.GetSize());
        if (!m_rContainer.writeBuffer(aLine))
            return -1;
        aLine.setLength(0);
        m_rContainer.finishStreamEncryption();

        aLine.append("\nendstream\n");
        if (!m_rContainer.writeBuffer(aLine))
            return -1;
        aLine.setLength(0);
    }

    aLine.append("endobj\n\n");
    if (!m_rContainer.writeBuffer(aLine))
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
    filter::PDFObjectElement* pKindObject = nullptr;
    if (auto pResources
        = dynamic_cast<filter::PDFDictionaryElement*>(rPage.Lookup("Resources"_ostr)))
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
                return {};
            }

            pKindObject = pReferenced;
            aItems = pReferenced->GetDictionaryItems();
        }
    }
    else if (filter::PDFObjectElement* pPageResources = rPage.LookupObject("Resources"_ostr))
    {
        // Resources is an indirect object.
        filter::PDFElement* pValue = pPageResources->Lookup(rKind);
        if (auto pDictionary = dynamic_cast<filter::PDFDictionaryElement*>(pValue))
        {
            // Kind is a direct dictionary.
            aItems = pDictionary->GetItems();
        }
        else if (filter::PDFObjectElement* pObject = pPageResources->LookupObject(rKind))
        {
            // Kind is an indirect object.
            aItems = pObject->GetDictionaryItems();
            pKindObject = pObject;
        }
    }
    if (aItems.empty())
        return {};

    SvMemoryStream& rDocBuffer = rPage.GetDocument().GetEditBuffer();
    bool bHasDictValue = false;

    for (const auto& rItem : aItems)
    {
        // For each item copy it over to our output then insert it into aRet.
        auto pReference = dynamic_cast<filter::PDFReferenceElement*>(rItem.second);
        if (!pReference)
        {
            if (pKindObject && dynamic_cast<filter::PDFDictionaryElement*>(rItem.second))
            {
                bHasDictValue = true;
                break;
            }

            continue;
        }

        filter::PDFObjectElement* pValue = pReference->LookupObject();
        if (!pValue)
            continue;

        // Then copying over an object copy its dictionary and its stream.
        sal_Int32 nObject = copyExternalResource(rDocBuffer, *pValue, rCopiedResources);
        aRet[rItem.first] = nObject;
    }

    if (bHasDictValue && pKindObject)
    {
        sal_Int32 nObject = copyExternalResource(rDocBuffer, *pKindObject, rCopiedResources);
        return "/" + rKind + " " + OString::number(nObject) + " 0 R";
    }

    // Build the dictionary entry string.
    OStringBuffer sRet("/" + rKind + "<<");
    for (const auto& rPair : aRet)
    {
        sRet.append("/" + rPair.first + " " + OString::number(rPair.second) + " 0 R");
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
        = { "ColorSpace"_ostr, "ExtGState"_ostr, "Font"_ostr,
            "XObject"_ostr,    "Shading"_ostr,   "Pattern"_ostr };
    for (const auto& rKey : aKeys)
    {
        rLine.append(copyExternalResources(*pPage, rKey, rCopiedResources));
    }
    rLine.append(">>");
}

sal_Int32 PDFObjectCopier::copyPageStreams(std::vector<filter::PDFObjectElement*>& rContentStreams,
                                           SvMemoryStream& rStream, bool& rCompressed,
                                           bool bIsTaggedNonReferenceXObject)
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

        auto pFilter = dynamic_cast<filter::PDFNameElement*>(pContent->Lookup("Filter"_ostr));
        auto pFilterArray = dynamic_cast<filter::PDFArrayElement*>(pContent->Lookup("Filter"_ostr));
        if (!pFilter && pFilterArray)
        {
            auto& aElements = pFilterArray->GetElements();
            if (!aElements.empty())
                pFilter = dynamic_cast<filter::PDFNameElement*>(aElements[0]);
        }

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

            bool bHasArtifact = false;
            if (bIsTaggedNonReferenceXObject)
            {
                auto pStart = static_cast<const char*>(aMemoryStream.GetData());
                const char* const pEnd = pStart + aMemoryStream.GetSize();
                std::string_view aStreamView(pStart, pEnd - pStart);

                std::string_view sArtifact = "/Artifact";
                std::size_t nPosArtifact = aStreamView.find(sArtifact);
                if (nPosArtifact != std::string_view::npos)
                {
                    bHasArtifact = true;
                    SvMemoryStream aTmpStream;
                    std::string_view sBMC = "BMC";
                    std::string_view sBDC = "BDC";
                    std::string_view sEMC = "EMC";

                    while (!aStreamView.empty())
                    {
                        aTmpStream.WriteOString(aStreamView.substr(0, nPosArtifact));
                        aStreamView.remove_prefix(nPosArtifact + sArtifact.size());

                        std::size_t nPosBMC = aStreamView.find(sBMC);
                        std::size_t nPosBDC = aStreamView.find(sBDC);
                        std::size_t nPos = std::min(nPosBMC, nPosBDC);

                        if (nPos != std::string_view::npos)
                        {
                            if (nPos == nPosBMC)
                                aStreamView.remove_prefix(nPos + sBMC.size() + 1);
                            else
                                aStreamView.remove_prefix(nPos + sBDC.size() + 1);

                            std::size_t nPosEMC = aStreamView.find(sEMC);
                            if (nPosEMC != std::string_view::npos)
                            {
                                aTmpStream.WriteOString(aStreamView.substr(0, nPosEMC));
                                aStreamView.remove_prefix(nPosEMC + sEMC.size() + 1);
                            }
                        }

                        nPosArtifact = aStreamView.find(sArtifact);
                        if (nPosArtifact == std::string_view::npos)
                        {
                            aTmpStream.WriteOString(aStreamView);
                            break;
                        }
                    }
                    rStream.WriteBytes(aTmpStream.GetData(), aTmpStream.GetSize());
                }
            }

            if (!bHasArtifact)
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
