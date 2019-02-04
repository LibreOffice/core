/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/filter/pdfdocument.hxx>

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/security/XCertificate.hpp>

#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <svl/cryptosign.hxx>
#include <tools/zcodec.hxx>
#include <vcl/pdfwriter.hxx>

using namespace com::sun::star;

namespace vcl
{
namespace filter
{
const int MAX_SIGNATURE_CONTENT_LENGTH = 50000;

class PDFTrailerElement;

/// A one-liner comment.
class PDFCommentElement : public PDFElement
{
    PDFDocument& m_rDoc;
    OString m_aComment;

public:
    explicit PDFCommentElement(PDFDocument& rDoc);
    bool Read(SvStream& rStream) override;
};

class PDFReferenceElement;

/// End of a dictionary: '>>'.
class PDFEndDictionaryElement : public PDFElement
{
    /// Offset before the '>>' token.
    sal_uInt64 m_nLocation = 0;

public:
    PDFEndDictionaryElement();
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetLocation() const;
};

/// End of a stream: 'endstream' keyword.
class PDFEndStreamElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// End of a object: 'endobj' keyword.
class PDFEndObjectElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// End of an array: ']'.
class PDFEndArrayElement : public PDFElement
{
    /// Location before the ']' token.
    sal_uInt64 m_nOffset = 0;

public:
    PDFEndArrayElement();
    bool Read(SvStream& rStream) override;
    sal_uInt64 GetOffset() const;
};

/// Boolean object: a 'true' or a 'false'.
class PDFBooleanElement : public PDFElement
{
public:
    explicit PDFBooleanElement(bool bValue);
    bool Read(SvStream& rStream) override;
};

/// Null object: the 'null' singleton.
class PDFNullElement : public PDFElement
{
public:
    bool Read(SvStream& rStream) override;
};

/// The trailer singleton is at the end of the doc.
class PDFTrailerElement : public PDFElement
{
    PDFDocument& m_rDoc;
    std::map<OString, PDFElement*> m_aDictionary;
    /// Location of the end of the trailer token.
    sal_uInt64 m_nOffset = 0;

public:
    explicit PDFTrailerElement(PDFDocument& rDoc);
    bool Read(SvStream& rStream) override;
    PDFElement* Lookup(const OString& rDictionaryKey);
    sal_uInt64 GetLocation() const;
};

XRefEntry::XRefEntry() = default;

PDFDocument::PDFDocument() = default;

bool PDFDocument::RemoveSignature(size_t nPosition)
{
    std::vector<PDFObjectElement*> aSignatures = GetSignatureWidgets();
    if (nPosition >= aSignatures.size())
    {
        SAL_WARN("vcl.filter", "PDFDocument::RemoveSignature: invalid nPosition");
        return false;
    }

    if (aSignatures.size() != m_aEOFs.size() - 1)
    {
        SAL_WARN("vcl.filter", "PDFDocument::RemoveSignature: no 1:1 mapping between signatures "
                               "and incremental updates");
        return false;
    }

    // The EOF offset is the end of the original file, without the signature at
    // nPosition.
    m_aEditBuffer.Seek(m_aEOFs[nPosition]);
    // Drop all bytes after the current position.
    m_aEditBuffer.SetStreamSize(m_aEditBuffer.Tell() + 1);

    return m_aEditBuffer.good();
}

sal_uInt32 PDFDocument::GetNextSignature()
{
    sal_uInt32 nRet = 0;
    for (const auto& pSignature : GetSignatureWidgets())
    {
        auto pT = dynamic_cast<PDFLiteralStringElement*>(pSignature->Lookup("T"));
        if (!pT)
            continue;

        const OString& rValue = pT->GetValue();
        const OString aPrefix = "Signature";
        if (!rValue.startsWith(aPrefix))
            continue;

        nRet = std::max(nRet, rValue.copy(aPrefix.getLength()).toUInt32());
    }

    return nRet + 1;
}

sal_Int32 PDFDocument::WriteSignatureObject(const OUString& rDescription, bool bAdES,
                                            sal_uInt64& rLastByteRangeOffset,
                                            sal_Int64& rContentOffset)
{
    // Write signature object.
    sal_Int32 nSignatureId = m_aXRef.size();
    XRefEntry aSignatureEntry;
    aSignatureEntry.SetOffset(m_aEditBuffer.Tell());
    aSignatureEntry.SetDirty(true);
    m_aXRef[nSignatureId] = aSignatureEntry;
    OStringBuffer aSigBuffer;
    aSigBuffer.append(nSignatureId);
    aSigBuffer.append(" 0 obj\n");
    aSigBuffer.append("<</Contents <");
    rContentOffset = aSignatureEntry.GetOffset() + aSigBuffer.getLength();
    // Reserve space for the PKCS#7 object.
    OStringBuffer aContentFiller(MAX_SIGNATURE_CONTENT_LENGTH);
    comphelper::string::padToLength(aContentFiller, MAX_SIGNATURE_CONTENT_LENGTH, '0');
    aSigBuffer.append(aContentFiller.makeStringAndClear());
    aSigBuffer.append(">\n/Type/Sig/SubFilter");
    if (bAdES)
        aSigBuffer.append("/ETSI.CAdES.detached");
    else
        aSigBuffer.append("/adbe.pkcs7.detached");

    // Time of signing.
    aSigBuffer.append(" /M (");
    aSigBuffer.append(vcl::PDFWriter::GetDateTime());
    aSigBuffer.append(")");

    // Byte range: we can write offset1-length1 and offset2 right now, will
    // write length2 later.
    aSigBuffer.append(" /ByteRange [ 0 ");
    // -1 and +1 is the leading "<" and the trailing ">" around the hex string.
    aSigBuffer.append(rContentOffset - 1);
    aSigBuffer.append(" ");
    aSigBuffer.append(rContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    aSigBuffer.append(" ");
    rLastByteRangeOffset = aSignatureEntry.GetOffset() + aSigBuffer.getLength();
    // We don't know how many bytes we need for the last ByteRange value, this
    // should be enough.
    OStringBuffer aByteRangeFiller;
    comphelper::string::padToLength(aByteRangeFiller, 100, ' ');
    aSigBuffer.append(aByteRangeFiller.makeStringAndClear());
    // Finish the Sig obj.
    aSigBuffer.append(" /Filter/Adobe.PPKMS");

    if (!rDescription.isEmpty())
    {
        aSigBuffer.append("/Reason<");
        vcl::PDFWriter::AppendUnicodeTextString(rDescription, aSigBuffer);
        aSigBuffer.append(">");
    }

    aSigBuffer.append(" >>\nendobj\n\n");
    m_aEditBuffer.WriteOString(aSigBuffer.toString());

    return nSignatureId;
}

sal_Int32 PDFDocument::WriteAppearanceObject()
{
    // Write appearance object.
    sal_Int32 nAppearanceId = m_aXRef.size();
    XRefEntry aAppearanceEntry;
    aAppearanceEntry.SetOffset(m_aEditBuffer.Tell());
    aAppearanceEntry.SetDirty(true);
    m_aXRef[nAppearanceId] = aAppearanceEntry;
    m_aEditBuffer.WriteUInt32AsString(nAppearanceId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<</Type/XObject\n/Subtype/Form\n");
    m_aEditBuffer.WriteCharPtr("/BBox[0 0 0 0]\n/Length 0\n>>\n");
    m_aEditBuffer.WriteCharPtr("stream\n\nendstream\nendobj\n\n");

    return nAppearanceId;
}

sal_Int32 PDFDocument::WriteAnnotObject(PDFObjectElement const& rFirstPage, sal_Int32 nSignatureId,
                                        sal_Int32 nAppearanceId)
{
    // Decide what identifier to use for the new signature.
    sal_uInt32 nNextSignature = GetNextSignature();

    // Write the Annot object, references nSignatureId and nAppearanceId.
    sal_Int32 nAnnotId = m_aXRef.size();
    XRefEntry aAnnotEntry;
    aAnnotEntry.SetOffset(m_aEditBuffer.Tell());
    aAnnotEntry.SetDirty(true);
    m_aXRef[nAnnotId] = aAnnotEntry;
    m_aEditBuffer.WriteUInt32AsString(nAnnotId);
    m_aEditBuffer.WriteCharPtr(" 0 obj\n");
    m_aEditBuffer.WriteCharPtr("<</Type/Annot/Subtype/Widget/F 132\n");
    m_aEditBuffer.WriteCharPtr("/Rect[0 0 0 0]\n");
    m_aEditBuffer.WriteCharPtr("/FT/Sig\n");
    m_aEditBuffer.WriteCharPtr("/P ");
    m_aEditBuffer.WriteUInt32AsString(rFirstPage.GetObjectValue());
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/T(Signature");
    m_aEditBuffer.WriteUInt32AsString(nNextSignature);
    m_aEditBuffer.WriteCharPtr(")\n");
    m_aEditBuffer.WriteCharPtr("/V ");
    m_aEditBuffer.WriteUInt32AsString(nSignatureId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/DV ");
    m_aEditBuffer.WriteUInt32AsString(nSignatureId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n");
    m_aEditBuffer.WriteCharPtr("/AP<<\n/N ");
    m_aEditBuffer.WriteUInt32AsString(nAppearanceId);
    m_aEditBuffer.WriteCharPtr(" 0 R\n>>\n");
    m_aEditBuffer.WriteCharPtr(">>\nendobj\n\n");

    return nAnnotId;
}

bool PDFDocument::WritePageObject(PDFObjectElement& rFirstPage, sal_Int32 nAnnotId)
{
    PDFElement* pAnnots = rFirstPage.Lookup("Annots");
    auto pAnnotsReference = dynamic_cast<PDFReferenceElement*>(pAnnots);
    if (pAnnotsReference)
    {
        // Write the updated Annots key of the Page object.
        PDFObjectElement* pAnnotsObject = pAnnotsReference->LookupObject();
        if (!pAnnotsObject)
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: invalid Annots reference");
            return false;
        }

        sal_uInt32 nAnnotsId = pAnnotsObject->GetObjectValue();
        m_aXRef[nAnnotsId].SetType(XRefEntryType::NOT_COMPRESSED);
        m_aXRef[nAnnotsId].SetOffset(m_aEditBuffer.Tell());
        m_aXRef[nAnnotsId].SetDirty(true);
        m_aEditBuffer.WriteUInt32AsString(nAnnotsId);
        m_aEditBuffer.WriteCharPtr(" 0 obj\n[");

        // Write existing references.
        PDFArrayElement* pArray = pAnnotsObject->GetArray();
        if (!pArray)
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: Page Annots is a reference to a non-array");
            return false;
        }

        for (size_t i = 0; i < pArray->GetElements().size(); ++i)
        {
            auto pReference = dynamic_cast<PDFReferenceElement*>(pArray->GetElements()[i]);
            if (!pReference)
                continue;

            if (i)
                m_aEditBuffer.WriteCharPtr(" ");
            m_aEditBuffer.WriteUInt32AsString(pReference->GetObjectValue());
            m_aEditBuffer.WriteCharPtr(" 0 R");
        }
        // Write our reference.
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R");

        m_aEditBuffer.WriteCharPtr("]\nendobj\n\n");
    }
    else
    {
        // Write the updated first page object, references nAnnotId.
        sal_uInt32 nFirstPageId = rFirstPage.GetObjectValue();
        if (nFirstPageId >= m_aXRef.size())
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: invalid first page obj id");
            return false;
        }
        m_aXRef[nFirstPageId].SetOffset(m_aEditBuffer.Tell());
        m_aXRef[nFirstPageId].SetDirty(true);
        m_aEditBuffer.WriteUInt32AsString(nFirstPageId);
        m_aEditBuffer.WriteCharPtr(" 0 obj\n");
        m_aEditBuffer.WriteCharPtr("<<");
        auto pAnnotsArray = dynamic_cast<PDFArrayElement*>(pAnnots);
        if (!pAnnotsArray)
        {
            // No Annots key, just write the key with a single reference.
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + rFirstPage.GetDictionaryOffset(),
                                     rFirstPage.GetDictionaryLength());
            m_aEditBuffer.WriteCharPtr("/Annots[");
            m_aEditBuffer.WriteUInt32AsString(nAnnotId);
            m_aEditBuffer.WriteCharPtr(" 0 R]");
        }
        else
        {
            // Annots key is already there, insert our reference at the end.
            PDFDictionaryElement* pDictionary = rFirstPage.GetDictionary();

            // Offset right before the end of the Annots array.
            sal_uInt64 nAnnotsEndOffset = pDictionary->GetKeyOffset("Annots")
                                          + pDictionary->GetKeyValueLength("Annots") - 1;
            // Length of beginning of the dictionary -> Annots end.
            sal_uInt64 nAnnotsBeforeEndLength = nAnnotsEndOffset - rFirstPage.GetDictionaryOffset();
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + rFirstPage.GetDictionaryOffset(),
                                     nAnnotsBeforeEndLength);
            m_aEditBuffer.WriteCharPtr(" ");
            m_aEditBuffer.WriteUInt32AsString(nAnnotId);
            m_aEditBuffer.WriteCharPtr(" 0 R");
            // Length of Annots end -> end of the dictionary.
            sal_uInt64 nAnnotsAfterEndLength = rFirstPage.GetDictionaryOffset()
                                               + rFirstPage.GetDictionaryLength()
                                               - nAnnotsEndOffset;
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + nAnnotsEndOffset,
                                     nAnnotsAfterEndLength);
        }
        m_aEditBuffer.WriteCharPtr(">>");
        m_aEditBuffer.WriteCharPtr("\nendobj\n\n");
    }

    return true;
}

bool PDFDocument::WriteCatalogObject(sal_Int32 nAnnotId, PDFReferenceElement*& pRoot)
{
    if (m_pXRefStream)
        pRoot = dynamic_cast<PDFReferenceElement*>(m_pXRefStream->Lookup("Root"));
    else
    {
        if (!m_pTrailer)
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: found no trailer");
            return false;
        }
        pRoot = dynamic_cast<PDFReferenceElement*>(m_pTrailer->Lookup("Root"));
    }
    if (!pRoot)
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: trailer has no root reference");
        return false;
    }
    PDFObjectElement* pCatalog = pRoot->LookupObject();
    if (!pCatalog)
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: invalid catalog reference");
        return false;
    }
    sal_uInt32 nCatalogId = pCatalog->GetObjectValue();
    if (nCatalogId >= m_aXRef.size())
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: invalid catalog obj id");
        return false;
    }
    PDFElement* pAcroForm = pCatalog->Lookup("AcroForm");
    auto pAcroFormReference = dynamic_cast<PDFReferenceElement*>(pAcroForm);
    if (pAcroFormReference)
    {
        // Write the updated AcroForm key of the Catalog object.
        PDFObjectElement* pAcroFormObject = pAcroFormReference->LookupObject();
        if (!pAcroFormObject)
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: invalid AcroForm reference");
            return false;
        }

        sal_uInt32 nAcroFormId = pAcroFormObject->GetObjectValue();
        m_aXRef[nAcroFormId].SetType(XRefEntryType::NOT_COMPRESSED);
        m_aXRef[nAcroFormId].SetOffset(m_aEditBuffer.Tell());
        m_aXRef[nAcroFormId].SetDirty(true);
        m_aEditBuffer.WriteUInt32AsString(nAcroFormId);
        m_aEditBuffer.WriteCharPtr(" 0 obj\n");

        // If this is nullptr, then the AcroForm object is not in an object stream.
        SvMemoryStream* pStreamBuffer = pAcroFormObject->GetStreamBuffer();

        if (!pAcroFormObject->Lookup("Fields"))
        {
            SAL_WARN("vcl.filter",
                     "PDFDocument::Sign: AcroForm object without required Fields key");
            return false;
        }

        PDFDictionaryElement* pAcroFormDictionary = pAcroFormObject->GetDictionary();
        if (!pAcroFormDictionary)
        {
            SAL_WARN("vcl.filter", "PDFDocument::Sign: AcroForm object has no dictionary");
            return false;
        }

        // Offset right before the end of the Fields array.
        sal_uInt64 nFieldsEndOffset = pAcroFormDictionary->GetKeyOffset("Fields")
                                      + pAcroFormDictionary->GetKeyValueLength("Fields")
                                      - strlen("]");
        // Length of beginning of the object dictionary -> Fields end.
        sal_uInt64 nFieldsBeforeEndLength = nFieldsEndOffset;
        if (pStreamBuffer)
            m_aEditBuffer.WriteBytes(pStreamBuffer->GetData(), nFieldsBeforeEndLength);
        else
        {
            nFieldsBeforeEndLength -= pAcroFormObject->GetDictionaryOffset();
            m_aEditBuffer.WriteCharPtr("<<");
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + pAcroFormObject->GetDictionaryOffset(),
                                     nFieldsBeforeEndLength);
        }

        // Append our reference at the end of the Fields array.
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(nAnnotId);
        m_aEditBuffer.WriteCharPtr(" 0 R");

        // Length of Fields end -> end of the object dictionary.
        if (pStreamBuffer)
        {
            sal_uInt64 nFieldsAfterEndLength = pStreamBuffer->GetSize() - nFieldsEndOffset;
            m_aEditBuffer.WriteBytes(static_cast<const char*>(pStreamBuffer->GetData())
                                         + nFieldsEndOffset,
                                     nFieldsAfterEndLength);
        }
        else
        {
            sal_uInt64 nFieldsAfterEndLength = pAcroFormObject->GetDictionaryOffset()
                                               + pAcroFormObject->GetDictionaryLength()
                                               - nFieldsEndOffset;
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + nFieldsEndOffset,
                                     nFieldsAfterEndLength);
            m_aEditBuffer.WriteCharPtr(">>");
        }

        m_aEditBuffer.WriteCharPtr("\nendobj\n\n");
    }
    else
    {
        // Write the updated Catalog object, references nAnnotId.
        auto pAcroFormDictionary = dynamic_cast<PDFDictionaryElement*>(pAcroForm);
        m_aXRef[nCatalogId].SetOffset(m_aEditBuffer.Tell());
        m_aXRef[nCatalogId].SetDirty(true);
        m_aEditBuffer.WriteUInt32AsString(nCatalogId);
        m_aEditBuffer.WriteCharPtr(" 0 obj\n");
        m_aEditBuffer.WriteCharPtr("<<");
        if (!pAcroFormDictionary)
        {
            // No AcroForm key, assume no signatures.
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + pCatalog->GetDictionaryOffset(),
                                     pCatalog->GetDictionaryLength());
            m_aEditBuffer.WriteCharPtr("/AcroForm<</Fields[\n");
            m_aEditBuffer.WriteUInt32AsString(nAnnotId);
            m_aEditBuffer.WriteCharPtr(" 0 R\n]/SigFlags 3>>\n");
        }
        else
        {
            // AcroForm key is already there, insert our reference at the Fields end.
            auto it = pAcroFormDictionary->GetItems().find("Fields");
            if (it == pAcroFormDictionary->GetItems().end())
            {
                SAL_WARN("vcl.filter", "PDFDocument::Sign: AcroForm without required Fields key");
                return false;
            }

            auto pFields = dynamic_cast<PDFArrayElement*>(it->second);
            if (!pFields)
            {
                SAL_WARN("vcl.filter", "PDFDocument::Sign: AcroForm Fields is not an array");
                return false;
            }

            // Offset right before the end of the Fields array.
            sal_uInt64 nFieldsEndOffset = pAcroFormDictionary->GetKeyOffset("Fields")
                                          + pAcroFormDictionary->GetKeyValueLength("Fields") - 1;
            // Length of beginning of the Catalog dictionary -> Fields end.
            sal_uInt64 nFieldsBeforeEndLength = nFieldsEndOffset - pCatalog->GetDictionaryOffset();
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + pCatalog->GetDictionaryOffset(),
                                     nFieldsBeforeEndLength);
            m_aEditBuffer.WriteCharPtr(" ");
            m_aEditBuffer.WriteUInt32AsString(nAnnotId);
            m_aEditBuffer.WriteCharPtr(" 0 R");
            // Length of Fields end -> end of the Catalog dictionary.
            sal_uInt64 nFieldsAfterEndLength = pCatalog->GetDictionaryOffset()
                                               + pCatalog->GetDictionaryLength() - nFieldsEndOffset;
            m_aEditBuffer.WriteBytes(static_cast<const char*>(m_aEditBuffer.GetData())
                                         + nFieldsEndOffset,
                                     nFieldsAfterEndLength);
        }
        m_aEditBuffer.WriteCharPtr(">>\nendobj\n\n");
    }

    return true;
}

void PDFDocument::WriteXRef(sal_uInt64 nXRefOffset, PDFReferenceElement const* pRoot)
{
    if (m_pXRefStream)
    {
        // Write the xref stream.
        // This is a bit meta: the xref stream stores its own offset.
        sal_Int32 nXRefStreamId = m_aXRef.size();
        XRefEntry aXRefStreamEntry;
        aXRefStreamEntry.SetOffset(nXRefOffset);
        aXRefStreamEntry.SetDirty(true);
        m_aXRef[nXRefStreamId] = aXRefStreamEntry;

        // Write stream data.
        SvMemoryStream aXRefStream;
        const size_t nOffsetLen = 3;
        // 3 additional bytes: predictor, the first and the third field.
        const size_t nLineLength = nOffsetLen + 3;
        // This is the line as it appears before tweaking according to the predictor.
        std::vector<unsigned char> aOrigLine(nLineLength);
        // This is the previous line.
        std::vector<unsigned char> aPrevLine(nLineLength);
        // This is the line as written to the stream.
        std::vector<unsigned char> aFilteredLine(nLineLength);
        for (const auto& rXRef : m_aXRef)
        {
            const XRefEntry& rEntry = rXRef.second;

            if (!rEntry.GetDirty())
                continue;

            // Predictor.
            size_t nPos = 0;
            // PNG prediction: up (on all rows).
            aOrigLine[nPos++] = 2;

            // First field.
            unsigned char nType = 0;
            switch (rEntry.GetType())
            {
                case XRefEntryType::FREE:
                    nType = 0;
                    break;
                case XRefEntryType::NOT_COMPRESSED:
                    nType = 1;
                    break;
                case XRefEntryType::COMPRESSED:
                    nType = 2;
                    break;
            }
            aOrigLine[nPos++] = nType;

            // Second field.
            for (size_t i = 0; i < nOffsetLen; ++i)
            {
                size_t nByte = nOffsetLen - i - 1;
                // Fields requiring more than one byte are stored with the
                // high-order byte first.
                unsigned char nCh = (rEntry.GetOffset() & (0xff << (nByte * 8))) >> (nByte * 8);
                aOrigLine[nPos++] = nCh;
            }

            // Third field.
            aOrigLine[nPos++] = 0;

            // Now apply the predictor.
            aFilteredLine[0] = aOrigLine[0];
            for (size_t i = 1; i < nLineLength; ++i)
            {
                // Count the delta vs the previous line.
                aFilteredLine[i] = aOrigLine[i] - aPrevLine[i];
                // Remember the new reference.
                aPrevLine[i] = aOrigLine[i];
            }

            aXRefStream.WriteBytes(aFilteredLine.data(), aFilteredLine.size());
        }

        m_aEditBuffer.WriteUInt32AsString(nXRefStreamId);
        m_aEditBuffer.WriteCharPtr(
            " 0 obj\n<</DecodeParms<</Columns 5/Predictor 12>>/Filter/FlateDecode");

        // ID.
        auto pID = dynamic_cast<PDFArrayElement*>(m_pXRefStream->Lookup("ID"));
        if (pID)
        {
            const std::vector<PDFElement*>& rElements = pID->GetElements();
            m_aEditBuffer.WriteCharPtr("/ID [ <");
            for (size_t i = 0; i < rElements.size(); ++i)
            {
                auto pIDString = dynamic_cast<PDFHexStringElement*>(rElements[i]);
                if (!pIDString)
                    continue;

                m_aEditBuffer.WriteOString(pIDString->GetValue());
                if ((i + 1) < rElements.size())
                    m_aEditBuffer.WriteCharPtr("> <");
            }
            m_aEditBuffer.WriteCharPtr("> ] ");
        }

        // Index.
        m_aEditBuffer.WriteCharPtr("/Index [ ");
        for (const auto& rXRef : m_aXRef)
        {
            if (!rXRef.second.GetDirty())
                continue;

            m_aEditBuffer.WriteUInt32AsString(rXRef.first);
            m_aEditBuffer.WriteCharPtr(" 1 ");
        }
        m_aEditBuffer.WriteCharPtr("] ");

        // Info.
        auto pInfo = dynamic_cast<PDFReferenceElement*>(m_pXRefStream->Lookup("Info"));
        if (pInfo)
        {
            m_aEditBuffer.WriteCharPtr("/Info ");
            m_aEditBuffer.WriteUInt32AsString(pInfo->GetObjectValue());
            m_aEditBuffer.WriteCharPtr(" ");
            m_aEditBuffer.WriteUInt32AsString(pInfo->GetGenerationValue());
            m_aEditBuffer.WriteCharPtr(" R ");
        }

        // Length.
        m_aEditBuffer.WriteCharPtr("/Length ");
        {
            ZCodec aZCodec;
            aZCodec.BeginCompression();
            aXRefStream.Seek(0);
            SvMemoryStream aStream;
            aZCodec.Compress(aXRefStream, aStream);
            aZCodec.EndCompression();
            aXRefStream.Seek(0);
            aXRefStream.SetStreamSize(0);
            aStream.Seek(0);
            aXRefStream.WriteStream(aStream);
        }
        m_aEditBuffer.WriteUInt32AsString(aXRefStream.GetSize());

        if (!m_aStartXRefs.empty())
        {
            // Write location of the previous cross-reference section.
            m_aEditBuffer.WriteCharPtr("/Prev ");
            m_aEditBuffer.WriteUInt32AsString(m_aStartXRefs.back());
        }

        // Root.
        m_aEditBuffer.WriteCharPtr("/Root ");
        m_aEditBuffer.WriteUInt32AsString(pRoot->GetObjectValue());
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(pRoot->GetGenerationValue());
        m_aEditBuffer.WriteCharPtr(" R ");

        // Size.
        m_aEditBuffer.WriteCharPtr("/Size ");
        m_aEditBuffer.WriteUInt32AsString(m_aXRef.size());

        m_aEditBuffer.WriteCharPtr("/Type/XRef/W[1 3 1]>>\nstream\n");
        aXRefStream.Seek(0);
        m_aEditBuffer.WriteStream(aXRefStream);
        m_aEditBuffer.WriteCharPtr("\nendstream\nendobj\n\n");
    }
    else
    {
        // Write the xref table.
        m_aEditBuffer.WriteCharPtr("xref\n");
        for (const auto& rXRef : m_aXRef)
        {
            size_t nObject = rXRef.first;
            size_t nOffset = rXRef.second.GetOffset();
            if (!rXRef.second.GetDirty())
                continue;

            m_aEditBuffer.WriteUInt32AsString(nObject);
            m_aEditBuffer.WriteCharPtr(" 1\n");
            OStringBuffer aBuffer;
            aBuffer.append(static_cast<sal_Int32>(nOffset));
            while (aBuffer.getLength() < 10)
                aBuffer.insert(0, "0");
            if (nObject == 0)
                aBuffer.append(" 65535 f \n");
            else
                aBuffer.append(" 00000 n \n");
            m_aEditBuffer.WriteOString(aBuffer.toString());
        }

        // Write the trailer.
        m_aEditBuffer.WriteCharPtr("trailer\n<</Size ");
        m_aEditBuffer.WriteUInt32AsString(m_aXRef.size());
        m_aEditBuffer.WriteCharPtr("/Root ");
        m_aEditBuffer.WriteUInt32AsString(pRoot->GetObjectValue());
        m_aEditBuffer.WriteCharPtr(" ");
        m_aEditBuffer.WriteUInt32AsString(pRoot->GetGenerationValue());
        m_aEditBuffer.WriteCharPtr(" R\n");
        auto pInfo = dynamic_cast<PDFReferenceElement*>(m_pTrailer->Lookup("Info"));
        if (pInfo)
        {
            m_aEditBuffer.WriteCharPtr("/Info ");
            m_aEditBuffer.WriteUInt32AsString(pInfo->GetObjectValue());
            m_aEditBuffer.WriteCharPtr(" ");
            m_aEditBuffer.WriteUInt32AsString(pInfo->GetGenerationValue());
            m_aEditBuffer.WriteCharPtr(" R\n");
        }
        auto pID = dynamic_cast<PDFArrayElement*>(m_pTrailer->Lookup("ID"));
        if (pID)
        {
            const std::vector<PDFElement*>& rElements = pID->GetElements();
            m_aEditBuffer.WriteCharPtr("/ID [ <");
            for (size_t i = 0; i < rElements.size(); ++i)
            {
                auto pIDString = dynamic_cast<PDFHexStringElement*>(rElements[i]);
                if (!pIDString)
                    continue;

                m_aEditBuffer.WriteOString(pIDString->GetValue());
                if ((i + 1) < rElements.size())
                    m_aEditBuffer.WriteCharPtr(">\n<");
            }
            m_aEditBuffer.WriteCharPtr("> ]\n");
        }

        if (!m_aStartXRefs.empty())
        {
            // Write location of the previous cross-reference section.
            m_aEditBuffer.WriteCharPtr("/Prev ");
            m_aEditBuffer.WriteUInt32AsString(m_aStartXRefs.back());
        }

        m_aEditBuffer.WriteCharPtr(">>\n");
    }
}

bool PDFDocument::Sign(const uno::Reference<security::XCertificate>& xCertificate,
                       const OUString& rDescription, bool bAdES)
{
    m_aEditBuffer.Seek(STREAM_SEEK_TO_END);
    m_aEditBuffer.WriteCharPtr("\n");

    sal_uInt64 nSignatureLastByteRangeOffset = 0;
    sal_Int64 nSignatureContentOffset = 0;
    sal_Int32 nSignatureId = WriteSignatureObject(
        rDescription, bAdES, nSignatureLastByteRangeOffset, nSignatureContentOffset);

    sal_Int32 nAppearanceId = WriteAppearanceObject();

    std::vector<PDFObjectElement*> aPages = GetPages();
    if (aPages.empty() || !aPages[0])
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: found no pages");
        return false;
    }

    PDFObjectElement& rFirstPage = *aPages[0];
    sal_Int32 nAnnotId = WriteAnnotObject(rFirstPage, nSignatureId, nAppearanceId);

    if (!WritePageObject(rFirstPage, nAnnotId))
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: failed to write the updated Page object");
        return false;
    }

    PDFReferenceElement* pRoot = nullptr;
    if (!WriteCatalogObject(nAnnotId, pRoot))
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: failed to write the updated Catalog object");
        return false;
    }

    sal_uInt64 nXRefOffset = m_aEditBuffer.Tell();
    WriteXRef(nXRefOffset, pRoot);

    // Write startxref.
    m_aEditBuffer.WriteCharPtr("startxref\n");
    m_aEditBuffer.WriteUInt32AsString(nXRefOffset);
    m_aEditBuffer.WriteCharPtr("\n%%EOF\n");

    // Finalize the signature, now that we know the total file size.
    // Calculate the length of the last byte range.
    sal_uInt64 nFileEnd = m_aEditBuffer.Tell();
    sal_Int64 nLastByteRangeLength
        = nFileEnd - (nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    // Write the length to the buffer.
    m_aEditBuffer.Seek(nSignatureLastByteRangeOffset);
    OStringBuffer aByteRangeBuffer;
    aByteRangeBuffer.append(nLastByteRangeLength);
    aByteRangeBuffer.append(" ]");
    m_aEditBuffer.WriteOString(aByteRangeBuffer.toString());

    // Create the PKCS#7 object.
    css::uno::Sequence<sal_Int8> aDerEncoded = xCertificate->getEncoded();
    if (!aDerEncoded.hasElements())
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: empty certificate");
        return false;
    }

    m_aEditBuffer.Seek(0);
    sal_uInt64 nBufferSize1 = nSignatureContentOffset - 1;
    std::unique_ptr<char[]> aBuffer1(new char[nBufferSize1]);
    m_aEditBuffer.ReadBytes(aBuffer1.get(), nBufferSize1);

    m_aEditBuffer.Seek(nSignatureContentOffset + MAX_SIGNATURE_CONTENT_LENGTH + 1);
    sal_uInt64 nBufferSize2 = nLastByteRangeLength;
    std::unique_ptr<char[]> aBuffer2(new char[nBufferSize2]);
    m_aEditBuffer.ReadBytes(aBuffer2.get(), nBufferSize2);

    OStringBuffer aCMSHexBuffer;
    svl::crypto::Signing aSigning(xCertificate);
    aSigning.AddDataRange(aBuffer1.get(), nBufferSize1);
    aSigning.AddDataRange(aBuffer2.get(), nBufferSize2);
    if (!aSigning.Sign(aCMSHexBuffer))
    {
        SAL_WARN("vcl.filter", "PDFDocument::Sign: PDFWriter::Sign() failed");
        return false;
    }

    assert(aCMSHexBuffer.getLength() <= MAX_SIGNATURE_CONTENT_LENGTH);

    m_aEditBuffer.Seek(nSignatureContentOffset);
    m_aEditBuffer.WriteOString(aCMSHexBuffer.toString());

    return true;
}

bool PDFDocument::Write(SvStream& rStream)
{
    m_aEditBuffer.Seek(0);
    rStream.WriteStream(m_aEditBuffer);
    return rStream.good();
}

bool PDFDocument::Tokenize(SvStream& rStream, TokenizeMode eMode,
                           std::vector<std::unique_ptr<PDFElement>>& rElements,
                           PDFObjectElement* pObjectElement)
{
    // Last seen object token.
    PDFObjectElement* pObject = pObjectElement;
    PDFNameElement* pObjectKey = nullptr;
    PDFObjectElement* pObjectStream = nullptr;
    bool bInXRef = false;
    // The next number will be an xref offset.
    bool bInStartXRef = false;
    // Dictionary depth, so we know when we're outside any dictionaries.
    int nDictionaryDepth = 0;
    // Array depth, only the offset/length of the toplevel array is tracked.
    int nArrayDepth = 0;
    // Last seen array token that's outside any dictionaries.
    PDFArrayElement* pArray = nullptr;
    // If we're inside an obj/endobj pair.
    bool bInObject = false;
    while (true)
    {
        char ch;
        rStream.ReadChar(ch);
        if (rStream.eof())
            break;

        switch (ch)
        {
            case '%':
            {
                auto pComment = new PDFCommentElement(*this);
                rElements.push_back(std::unique_ptr<PDFElement>(pComment));
                rStream.SeekRel(-1);
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::Tokenize: PDFCommentElement::Read() failed");
                    return false;
                }
                if (eMode == TokenizeMode::EOF_TOKEN && !m_aEOFs.empty()
                    && m_aEOFs.back() == rStream.Tell())
                {
                    // Found EOF and partial parsing requested, we're done.
                    return true;
                }
                break;
            }
            case '<':
            {
                // Dictionary or hex string.
                rStream.ReadChar(ch);
                rStream.SeekRel(-2);
                if (ch == '<')
                {
                    rElements.push_back(std::unique_ptr<PDFElement>(new PDFDictionaryElement()));
                    ++nDictionaryDepth;
                }
                else
                    rElements.push_back(std::unique_ptr<PDFElement>(new PDFHexStringElement));
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::Tokenize: PDFDictionaryElement::Read() failed");
                    return false;
                }
                break;
            }
            case '>':
            {
                rElements.push_back(std::unique_ptr<PDFElement>(new PDFEndDictionaryElement()));
                --nDictionaryDepth;
                rStream.SeekRel(-1);
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::Tokenize: PDFEndDictionaryElement::Read() failed");
                    return false;
                }
                break;
            }
            case '[':
            {
                auto pArr = new PDFArrayElement(pObject);
                rElements.push_back(std::unique_ptr<PDFElement>(pArr));
                if (nDictionaryDepth == 0 && nArrayDepth == 0)
                {
                    // The array is attached directly, inform the object.
                    pArray = pArr;
                    if (pObject)
                    {
                        pObject->SetArray(pArray);
                        pObject->SetArrayOffset(rStream.Tell());
                    }
                }
                ++nArrayDepth;
                rStream.SeekRel(-1);
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter", "PDFDocument::Tokenize: PDFArrayElement::Read() failed");
                    return false;
                }
                break;
            }
            case ']':
            {
                rElements.push_back(std::unique_ptr<PDFElement>(new PDFEndArrayElement()));
                --nArrayDepth;
                if (nArrayDepth == 0)
                    pArray = nullptr;
                rStream.SeekRel(-1);
                if (nDictionaryDepth == 0 && nArrayDepth == 0)
                {
                    if (pObject)
                    {
                        pObject->SetArrayLength(rStream.Tell() - pObject->GetArrayOffset());
                    }
                }
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::Tokenize: PDFEndArrayElement::Read() failed");
                    return false;
                }
                break;
            }
            case '/':
            {
                auto pNameElement = new PDFNameElement();
                rElements.push_back(std::unique_ptr<PDFElement>(pNameElement));
                rStream.SeekRel(-1);
                if (!pNameElement->Read(rStream))
                {
                    SAL_WARN("vcl.filter", "PDFDocument::Tokenize: PDFNameElement::Read() failed");
                    return false;
                }
                if (pObject && pObjectKey && pObjectKey->GetValue() == "Type"
                    && pNameElement->GetValue() == "ObjStm")
                    pObjectStream = pObject;
                else
                    pObjectKey = pNameElement;
                break;
            }
            case '(':
            {
                rElements.push_back(std::unique_ptr<PDFElement>(new PDFLiteralStringElement));
                rStream.SeekRel(-1);
                if (!rElements.back()->Read(rStream))
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::Tokenize: PDFLiteralStringElement::Read() failed");
                    return false;
                }
                break;
            }
            default:
            {
                if (rtl::isAsciiDigit(static_cast<unsigned char>(ch)) || ch == '-')
                {
                    // Numbering object: an integer or a real.
                    auto pNumberElement = new PDFNumberElement();
                    rElements.push_back(std::unique_ptr<PDFElement>(pNumberElement));
                    rStream.SeekRel(-1);
                    if (!pNumberElement->Read(rStream))
                    {
                        SAL_WARN("vcl.filter",
                                 "PDFDocument::Tokenize: PDFNumberElement::Read() failed");
                        return false;
                    }
                    if (bInStartXRef)
                    {
                        bInStartXRef = false;
                        m_aStartXRefs.push_back(pNumberElement->GetValue());

                        auto it = m_aOffsetObjects.find(pNumberElement->GetValue());
                        if (it != m_aOffsetObjects.end())
                            m_pXRefStream = it->second;
                    }
                    else if (bInObject && !nDictionaryDepth && !nArrayDepth && pObject)
                        // Number element inside an object, but outside a
                        // dictionary / array: remember it.
                        pObject->SetNumberElement(pNumberElement);
                }
                else if (rtl::isAsciiAlpha(static_cast<unsigned char>(ch)))
                {
                    // Possible keyword, like "obj".
                    rStream.SeekRel(-1);
                    OString aKeyword = ReadKeyword(rStream);

                    bool bObj = aKeyword == "obj";
                    if (bObj || aKeyword == "R")
                    {
                        size_t nElements = rElements.size();
                        if (nElements < 2)
                        {
                            SAL_WARN("vcl.filter", "PDFDocument::Tokenize: expected at least two "
                                                   "tokens before 'obj' or 'R' keyword");
                            return false;
                        }

                        auto pObjectNumber
                            = dynamic_cast<PDFNumberElement*>(rElements[nElements - 2].get());
                        auto pGenerationNumber
                            = dynamic_cast<PDFNumberElement*>(rElements[nElements - 1].get());
                        if (!pObjectNumber || !pGenerationNumber)
                        {
                            SAL_WARN("vcl.filter", "PDFDocument::Tokenize: missing object or "
                                                   "generation number before 'obj' or 'R' keyword");
                            return false;
                        }

                        if (bObj)
                        {
                            pObject = new PDFObjectElement(*this, pObjectNumber->GetValue(),
                                                           pGenerationNumber->GetValue());
                            rElements.push_back(std::unique_ptr<PDFElement>(pObject));
                            m_aOffsetObjects[pObjectNumber->GetLocation()] = pObject;
                            m_aIDObjects[pObjectNumber->GetValue()] = pObject;
                            bInObject = true;
                        }
                        else
                        {
                            auto pReference = new PDFReferenceElement(*this, *pObjectNumber,
                                                                      *pGenerationNumber);
                            rElements.push_back(std::unique_ptr<PDFElement>(pReference));
                            if (pArray)
                                // Reference is part of a direct (non-dictionary) array, inform the array.
                                pArray->PushBack(rElements.back().get());
                            if (bInObject && nDictionaryDepth > 0 && pObject)
                                // Inform the object about a new in-dictionary reference.
                                pObject->AddDictionaryReference(pReference);
                        }
                        if (!rElements.back()->Read(rStream))
                        {
                            SAL_WARN("vcl.filter",
                                     "PDFDocument::Tokenize: PDFElement::Read() failed");
                            return false;
                        }
                    }
                    else if (aKeyword == "stream")
                    {
                        // Look up the length of the stream from the parent object's dictionary.
                        size_t nLength = 0;
                        for (size_t nElement = 0; nElement < rElements.size(); ++nElement)
                        {
                            // Iterate in reverse order.
                            size_t nIndex = rElements.size() - nElement - 1;
                            PDFElement* pElement = rElements[nIndex].get();
                            auto pObj = dynamic_cast<PDFObjectElement*>(pElement);
                            if (!pObj)
                                continue;

                            PDFElement* pLookup = pObj->Lookup("Length");
                            auto pReference = dynamic_cast<PDFReferenceElement*>(pLookup);
                            if (pReference)
                            {
                                // Length is provided as a reference.
                                nLength = pReference->LookupNumber(rStream);
                                break;
                            }

                            auto pNumber = dynamic_cast<PDFNumberElement*>(pLookup);
                            if (pNumber)
                            {
                                // Length is provided directly.
                                nLength = pNumber->GetValue();
                                break;
                            }

                            SAL_WARN(
                                "vcl.filter",
                                "PDFDocument::Tokenize: found no Length key for stream keyword");
                            return false;
                        }

                        PDFDocument::SkipLineBreaks(rStream);
                        auto pStreamElement = new PDFStreamElement(nLength);
                        if (pObject)
                            pObject->SetStream(pStreamElement);
                        rElements.push_back(std::unique_ptr<PDFElement>(pStreamElement));
                        if (!rElements.back()->Read(rStream))
                        {
                            SAL_WARN("vcl.filter",
                                     "PDFDocument::Tokenize: PDFStreamElement::Read() failed");
                            return false;
                        }
                    }
                    else if (aKeyword == "endstream")
                    {
                        rElements.push_back(std::unique_ptr<PDFElement>(new PDFEndStreamElement));
                        if (!rElements.back()->Read(rStream))
                        {
                            SAL_WARN("vcl.filter",
                                     "PDFDocument::Tokenize: PDFEndStreamElement::Read() failed");
                            return false;
                        }
                    }
                    else if (aKeyword == "endobj")
                    {
                        rElements.push_back(std::unique_ptr<PDFElement>(new PDFEndObjectElement));
                        if (!rElements.back()->Read(rStream))
                        {
                            SAL_WARN("vcl.filter",
                                     "PDFDocument::Tokenize: PDFEndObjectElement::Read() failed");
                            return false;
                        }
                        if (eMode == TokenizeMode::END_OF_OBJECT)
                        {
                            // Found endobj and only object parsing was requested, we're done.
                            return true;
                        }

                        if (pObjectStream)
                        {
                            // We're at the end of an object stream, parse the stored objects.
                            pObjectStream->ParseStoredObjects();
                            pObjectStream = nullptr;
                            pObjectKey = nullptr;
                        }
                        bInObject = false;
                    }
                    else if (aKeyword == "true" || aKeyword == "false")
                        rElements.push_back(std::unique_ptr<PDFElement>(
                            new PDFBooleanElement(aKeyword.toBoolean())));
                    else if (aKeyword == "null")
                        rElements.push_back(std::unique_ptr<PDFElement>(new PDFNullElement));
                    else if (aKeyword == "xref")
                        // Allow 'f' and 'n' keywords.
                        bInXRef = true;
                    else if (bInXRef && (aKeyword == "f" || aKeyword == "n"))
                    {
                    }
                    else if (aKeyword == "trailer")
                    {
                        auto pTrailer = new PDFTrailerElement(*this);

                        // Make it possible to find this trailer later by offset.
                        pTrailer->Read(rStream);
                        m_aOffsetTrailers[pTrailer->GetLocation()] = pTrailer;

                        // When reading till the first EOF token only, remember
                        // just the first trailer token.
                        if (eMode != TokenizeMode::EOF_TOKEN || !m_pTrailer)
                            m_pTrailer = pTrailer;
                        rElements.push_back(std::unique_ptr<PDFElement>(pTrailer));
                    }
                    else if (aKeyword == "startxref")
                    {
                        bInStartXRef = true;
                    }
                    else
                    {
                        SAL_WARN("vcl.filter", "PDFDocument::Tokenize: unexpected '"
                                                   << aKeyword << "' keyword at byte position "
                                                   << rStream.Tell());
                        return false;
                    }
                }
                else
                {
                    if (!rtl::isAsciiWhiteSpace(static_cast<unsigned char>(ch)))
                    {
                        SAL_WARN("vcl.filter", "PDFDocument::Tokenize: unexpected character: "
                                                   << ch << " at byte position " << rStream.Tell());
                        return false;
                    }
                }
                break;
            }
        }
    }

    return true;
}

void PDFDocument::SetIDObject(size_t nID, PDFObjectElement* pObject)
{
    m_aIDObjects[nID] = pObject;
}

bool PDFDocument::Read(SvStream& rStream)
{
    // Check file magic.
    std::vector<sal_Int8> aHeader(5);
    rStream.Seek(0);
    rStream.ReadBytes(aHeader.data(), aHeader.size());
    if (aHeader[0] != '%' || aHeader[1] != 'P' || aHeader[2] != 'D' || aHeader[3] != 'F'
        || aHeader[4] != '-')
    {
        SAL_WARN("vcl.filter", "PDFDocument::Read: header mismatch");
        return false;
    }

    // Allow later editing of the contents in-memory.
    rStream.Seek(0);
    m_aEditBuffer.WriteStream(rStream);

    // Look up the offset of the xref table.
    size_t nStartXRef = FindStartXRef(rStream);
    SAL_INFO("vcl.filter", "PDFDocument::Read: nStartXRef is " << nStartXRef);
    if (nStartXRef == 0)
    {
        SAL_WARN("vcl.filter", "PDFDocument::Read: found no xref start offset");
        return false;
    }
    while (true)
    {
        rStream.Seek(nStartXRef);
        OString aKeyword = ReadKeyword(rStream);
        if (aKeyword.isEmpty())
            ReadXRefStream(rStream);

        else
        {
            if (aKeyword != "xref")
            {
                SAL_WARN("vcl.filter", "PDFDocument::Read: xref is not the first keyword");
                return false;
            }
            ReadXRef(rStream);
            if (!Tokenize(rStream, TokenizeMode::EOF_TOKEN, m_aElements, nullptr))
            {
                SAL_WARN("vcl.filter", "PDFDocument::Read: failed to tokenizer trailer after xref");
                return false;
            }
        }

        PDFNumberElement* pPrev = nullptr;
        if (m_pTrailer)
        {
            pPrev = dynamic_cast<PDFNumberElement*>(m_pTrailer->Lookup("Prev"));

            // Remember the offset of this trailer in the correct order. It's
            // possible that newer trailers don't have a larger offset.
            m_aTrailerOffsets.push_back(m_pTrailer->GetLocation());
        }
        else if (m_pXRefStream)
            pPrev = dynamic_cast<PDFNumberElement*>(m_pXRefStream->Lookup("Prev"));
        if (pPrev)
            nStartXRef = pPrev->GetValue();

        // Reset state, except the edit buffer.
        m_aElements.clear();
        m_aOffsetObjects.clear();
        m_aIDObjects.clear();
        m_aStartXRefs.clear();
        m_aEOFs.clear();
        m_pTrailer = nullptr;
        m_pXRefStream = nullptr;
        if (!pPrev)
            break;
    }

    // Then we can tokenize the stream.
    rStream.Seek(0);
    return Tokenize(rStream, TokenizeMode::END_OF_STREAM, m_aElements, nullptr);
}

OString PDFDocument::ReadKeyword(SvStream& rStream)
{
    OStringBuffer aBuf;
    char ch;
    rStream.ReadChar(ch);
    if (rStream.eof())
        return OString();
    while (rtl::isAsciiAlpha(static_cast<unsigned char>(ch)))
    {
        aBuf.append(ch);
        rStream.ReadChar(ch);
        if (rStream.eof())
            return aBuf.toString();
    }
    rStream.SeekRel(-1);
    return aBuf.toString();
}

size_t PDFDocument::FindStartXRef(SvStream& rStream)
{
    // Find the "startxref" token, somewhere near the end of the document.
    std::vector<char> aBuf(1024);
    rStream.Seek(STREAM_SEEK_TO_END);
    if (rStream.Tell() > aBuf.size())
        rStream.SeekRel(static_cast<sal_Int64>(-1) * aBuf.size());
    else
        // The document is really short, then just read it from the start.
        rStream.Seek(0);
    size_t nBeforePeek = rStream.Tell();
    size_t nSize = rStream.ReadBytes(aBuf.data(), aBuf.size());
    rStream.Seek(nBeforePeek);
    if (nSize != aBuf.size())
        aBuf.resize(nSize);
    OString aPrefix("startxref");
    // Find the last startxref at the end of the document.
    auto itLastValid = aBuf.end();
    auto it = aBuf.begin();
    while (true)
    {
        it = std::search(it, aBuf.end(), aPrefix.getStr(), aPrefix.getStr() + aPrefix.getLength());
        if (it == aBuf.end())
            break;

        itLastValid = it;
        ++it;
    }
    if (itLastValid == aBuf.end())
    {
        SAL_WARN("vcl.filter", "PDFDocument::FindStartXRef: found no startxref");
        return 0;
    }

    rStream.SeekRel(itLastValid - aBuf.begin() + aPrefix.getLength());
    if (rStream.eof())
    {
        SAL_WARN("vcl.filter",
                 "PDFDocument::FindStartXRef: unexpected end of stream after startxref");
        return 0;
    }

    PDFDocument::SkipWhitespace(rStream);
    PDFNumberElement aNumber;
    if (!aNumber.Read(rStream))
        return 0;
    return aNumber.GetValue();
}

void PDFDocument::ReadXRefStream(SvStream& rStream)
{
    // Look up the stream length in the object dictionary.
    if (!Tokenize(rStream, TokenizeMode::END_OF_OBJECT, m_aElements, nullptr))
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: failed to read object");
        return;
    }

    if (m_aElements.empty())
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: no tokens found");
        return;
    }

    PDFObjectElement* pObject = nullptr;
    for (const auto& pElement : m_aElements)
    {
        if (auto pObj = dynamic_cast<PDFObjectElement*>(pElement.get()))
        {
            pObject = pObj;
            break;
        }
    }
    if (!pObject)
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: no object token found");
        return;
    }

    // So that the Prev key can be looked up later.
    m_pXRefStream = pObject;

    PDFElement* pLookup = pObject->Lookup("Length");
    auto pNumber = dynamic_cast<PDFNumberElement*>(pLookup);
    if (!pNumber)
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: stream length is not provided");
        return;
    }
    sal_uInt64 nLength = pNumber->GetValue();

    // Look up the stream offset.
    PDFStreamElement* pStream = nullptr;
    for (const auto& pElement : m_aElements)
    {
        if (auto pS = dynamic_cast<PDFStreamElement*>(pElement.get()))
        {
            pStream = pS;
            break;
        }
    }
    if (!pStream)
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: no stream token found");
        return;
    }

    // Read and decompress it.
    rStream.Seek(pStream->GetOffset());
    std::vector<char> aBuf(nLength);
    rStream.ReadBytes(aBuf.data(), aBuf.size());

    auto pFilter = dynamic_cast<PDFNameElement*>(pObject->Lookup("Filter"));
    if (!pFilter)
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: no Filter found");
        return;
    }

    if (pFilter->GetValue() != "FlateDecode")
    {
        SAL_WARN("vcl.filter",
                 "PDFDocument::ReadXRefStream: unexpected filter: " << pFilter->GetValue());
        return;
    }

    int nColumns = 1;
    int nPredictor = 1;
    if (auto pDecodeParams = dynamic_cast<PDFDictionaryElement*>(pObject->Lookup("DecodeParms")))
    {
        const std::map<OString, PDFElement*>& rItems = pDecodeParams->GetItems();
        auto it = rItems.find("Columns");
        if (it != rItems.end())
            if (auto pColumns = dynamic_cast<PDFNumberElement*>(it->second))
                nColumns = pColumns->GetValue();
        it = rItems.find("Predictor");
        if (it != rItems.end())
            if (auto pPredictor = dynamic_cast<PDFNumberElement*>(it->second))
                nPredictor = pPredictor->GetValue();
    }

    SvMemoryStream aSource(aBuf.data(), aBuf.size(), StreamMode::READ);
    SvMemoryStream aStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    aZCodec.Decompress(aSource, aStream);
    if (!aZCodec.EndCompression())
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: decompression failed");
        return;
    }

    // Look up the first and the last entry we need to read.
    auto pIndex = dynamic_cast<PDFArrayElement*>(pObject->Lookup("Index"));
    std::vector<size_t> aFirstObjects;
    std::vector<size_t> aNumberOfObjects;
    if (!pIndex)
    {
        auto pSize = dynamic_cast<PDFNumberElement*>(pObject->Lookup("Size"));
        if (pSize)
        {
            aFirstObjects.push_back(0);
            aNumberOfObjects.push_back(pSize->GetValue());
        }
        else
        {
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: Index and Size not found");
            return;
        }
    }
    else
    {
        const std::vector<PDFElement*>& rIndexElements = pIndex->GetElements();
        size_t nFirstObject = 0;
        for (size_t i = 0; i < rIndexElements.size(); ++i)
        {
            if (i % 2 == 0)
            {
                auto pFirstObject = dynamic_cast<PDFNumberElement*>(rIndexElements[i]);
                if (!pFirstObject)
                {
                    SAL_WARN("vcl.filter",
                             "PDFDocument::ReadXRefStream: Index has no first object");
                    return;
                }
                nFirstObject = pFirstObject->GetValue();
                continue;
            }

            auto pNumberOfObjects = dynamic_cast<PDFNumberElement*>(rIndexElements[i]);
            if (!pNumberOfObjects)
            {
                SAL_WARN("vcl.filter",
                         "PDFDocument::ReadXRefStream: Index has no number of objects");
                return;
            }
            aFirstObjects.push_back(nFirstObject);
            aNumberOfObjects.push_back(pNumberOfObjects->GetValue());
        }
    }

    // Look up the format of a single entry.
    const int nWSize = 3;
    auto pW = dynamic_cast<PDFArrayElement*>(pObject->Lookup("W"));
    if (!pW || pW->GetElements().size() < nWSize)
    {
        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: W not found or has < 3 elements");
        return;
    }
    int aW[nWSize];
    // First character is the (kind of) repeated predictor.
    int nLineLength = 1;
    for (size_t i = 0; i < nWSize; ++i)
    {
        auto pI = dynamic_cast<PDFNumberElement*>(pW->GetElements()[i]);
        if (!pI)
        {
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: W contains non-number");
            return;
        }
        aW[i] = pI->GetValue();
        nLineLength += aW[i];
    }

    if (nPredictor > 1 && nLineLength - 1 != nColumns)
    {
        SAL_WARN("vcl.filter",
                 "PDFDocument::ReadXRefStream: /DecodeParms/Columns is inconsistent with /W");
        return;
    }

    aStream.Seek(0);
    for (size_t nSubSection = 0; nSubSection < aFirstObjects.size(); ++nSubSection)
    {
        size_t nFirstObject = aFirstObjects[nSubSection];
        size_t nNumberOfObjects = aNumberOfObjects[nSubSection];

        // This is the line as read from the stream.
        std::vector<unsigned char> aOrigLine(nLineLength);
        // This is the line as it appears after tweaking according to nPredictor.
        std::vector<unsigned char> aFilteredLine(nLineLength);
        for (size_t nEntry = 0; nEntry < nNumberOfObjects; ++nEntry)
        {
            size_t nIndex = nFirstObject + nEntry;

            aStream.ReadBytes(aOrigLine.data(), aOrigLine.size());
            if (nPredictor > 1 && aOrigLine[0] + 10 != nPredictor)
            {
                SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: in-stream predictor is "
                                       "inconsistent with /DecodeParms/Predictor for object #"
                                           << nIndex);
                return;
            }

            for (int i = 0; i < nLineLength; ++i)
            {
                switch (nPredictor)
                {
                    case 1:
                        // No prediction.
                        break;
                    case 12:
                        // PNG prediction: up (on all rows).
                        aFilteredLine[i] = aFilteredLine[i] + aOrigLine[i];
                        break;
                    default:
                        SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: unexpected predictor: "
                                                   << nPredictor);
                        return;
                        break;
                }
            }

            // First character is already handled above.
            int nPos = 1;
            size_t nType = 0;
            // Start of the current field in the stream data.
            int nOffset = nPos;
            for (; nPos < nOffset + aW[0]; ++nPos)
            {
                unsigned char nCh = aFilteredLine[nPos];
                nType = (nType << 8) + nCh;
            }

            // Start of the object in the file stream.
            size_t nStreamOffset = 0;
            nOffset = nPos;
            for (; nPos < nOffset + aW[1]; ++nPos)
            {
                unsigned char nCh = aFilteredLine[nPos];
                nStreamOffset = (nStreamOffset << 8) + nCh;
            }

            // Generation number of the object.
            size_t nGenerationNumber = 0;
            nOffset = nPos;
            for (; nPos < nOffset + aW[2]; ++nPos)
            {
                unsigned char nCh = aFilteredLine[nPos];
                nGenerationNumber = (nGenerationNumber << 8) + nCh;
            }

            // Ignore invalid nType.
            if (nType <= 2)
            {
                if (m_aXRef.find(nIndex) == m_aXRef.end())
                {
                    XRefEntry aEntry;
                    switch (nType)
                    {
                        case 0:
                            aEntry.SetType(XRefEntryType::FREE);
                            break;
                        case 1:
                            aEntry.SetType(XRefEntryType::NOT_COMPRESSED);
                            break;
                        case 2:
                            aEntry.SetType(XRefEntryType::COMPRESSED);
                            break;
                    }
                    aEntry.SetOffset(nStreamOffset);
                    m_aXRef[nIndex] = aEntry;
                }
            }
        }
    }
}

void PDFDocument::ReadXRef(SvStream& rStream)
{
    PDFDocument::SkipWhitespace(rStream);

    while (true)
    {
        PDFNumberElement aFirstObject;
        if (!aFirstObject.Read(rStream))
        {
            // Next token is not a number, it'll be the trailer.
            return;
        }

        if (aFirstObject.GetValue() < 0)
        {
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: expected first object number >= 0");
            return;
        }

        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumberOfEntries;
        if (!aNumberOfEntries.Read(rStream))
        {
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: failed to read number of entries");
            return;
        }

        if (aNumberOfEntries.GetValue() < 0)
        {
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: expected zero or more entries");
            return;
        }

        size_t nSize = aNumberOfEntries.GetValue();
        for (size_t nEntry = 0; nEntry < nSize; ++nEntry)
        {
            size_t nIndex = aFirstObject.GetValue() + nEntry;
            PDFDocument::SkipWhitespace(rStream);
            PDFNumberElement aOffset;
            if (!aOffset.Read(rStream))
            {
                SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: failed to read offset");
                return;
            }

            PDFDocument::SkipWhitespace(rStream);
            PDFNumberElement aGenerationNumber;
            if (!aGenerationNumber.Read(rStream))
            {
                SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: failed to read generation number");
                return;
            }

            PDFDocument::SkipWhitespace(rStream);
            OString aKeyword = ReadKeyword(rStream);
            if (aKeyword != "f" && aKeyword != "n")
            {
                SAL_WARN("vcl.filter", "PDFDocument::ReadXRef: unexpected keyword");
                return;
            }
            // xrefs are read in reverse order, so never update an existing
            // offset with an older one.
            if (m_aXRef.find(nIndex) == m_aXRef.end())
            {
                XRefEntry aEntry;
                aEntry.SetOffset(aOffset.GetValue());
                // Initially only the first entry is dirty.
                if (nIndex == 0)
                    aEntry.SetDirty(true);
                m_aXRef[nIndex] = aEntry;
            }
            PDFDocument::SkipWhitespace(rStream);
        }
    }
}

void PDFDocument::SkipWhitespace(SvStream& rStream)
{
    char ch = 0;

    while (true)
    {
        rStream.ReadChar(ch);
        if (rStream.eof())
            break;

        if (!rtl::isAsciiWhiteSpace(static_cast<unsigned char>(ch)))
        {
            rStream.SeekRel(-1);
            return;
        }
    }
}

void PDFDocument::SkipLineBreaks(SvStream& rStream)
{
    char ch = 0;

    while (true)
    {
        rStream.ReadChar(ch);
        if (rStream.eof())
            break;

        if (ch != '\n' && ch != '\r')
        {
            rStream.SeekRel(-1);
            return;
        }
    }
}

size_t PDFDocument::GetObjectOffset(size_t nIndex) const
{
    auto it = m_aXRef.find(nIndex);
    if (it == m_aXRef.end() || it->second.GetType() == XRefEntryType::COMPRESSED)
    {
        SAL_WARN("vcl.filter", "PDFDocument::GetObjectOffset: wanted to look up index #"
                                   << nIndex << ", but failed");
        return 0;
    }

    return it->second.GetOffset();
}

const std::vector<std::unique_ptr<PDFElement>>& PDFDocument::GetElements() { return m_aElements; }

/// Visits the page tree recursively, looking for page objects.
static void visitPages(PDFObjectElement* pPages, std::vector<PDFObjectElement*>& rRet)
{
    auto pKids = dynamic_cast<PDFArrayElement*>(pPages->Lookup("Kids"));
    if (!pKids)
    {
        SAL_WARN("vcl.filter", "visitPages: pages has no kids");
        return;
    }

    pPages->setVisiting(true);

    for (const auto& pKid : pKids->GetElements())
    {
        auto pReference = dynamic_cast<PDFReferenceElement*>(pKid);
        if (!pReference)
            continue;

        PDFObjectElement* pKidObject = pReference->LookupObject();
        if (!pKidObject)
            continue;

        // detect if visiting reenters itself
        if (pKidObject->alreadyVisiting())
        {
            SAL_WARN("vcl.filter", "visitPages: loop in hierarchy");
            continue;
        }

        auto pName = dynamic_cast<PDFNameElement*>(pKidObject->Lookup("Type"));
        if (pName && pName->GetValue() == "Pages")
            // Pages inside pages: recurse.
            visitPages(pKidObject, rRet);
        else
            // Found an actual page.
            rRet.push_back(pKidObject);
    }

    pPages->setVisiting(false);
}

std::vector<PDFObjectElement*> PDFDocument::GetPages()
{
    std::vector<PDFObjectElement*> aRet;

    PDFReferenceElement* pRoot = nullptr;

    PDFTrailerElement* pTrailer = nullptr;
    if (!m_aTrailerOffsets.empty())
    {
        // Get access to the latest trailer, and work with the keys of that
        // one.
        auto it = m_aOffsetTrailers.find(m_aTrailerOffsets[0]);
        if (it != m_aOffsetTrailers.end())
            pTrailer = it->second;
    }

    if (pTrailer)
        pRoot = dynamic_cast<PDFReferenceElement*>(pTrailer->Lookup("Root"));
    else if (m_pXRefStream)
        pRoot = dynamic_cast<PDFReferenceElement*>(m_pXRefStream->Lookup("Root"));

    if (!pRoot)
    {
        SAL_WARN("vcl.filter", "PDFDocument::GetPages: trailer has no Root key");
        return aRet;
    }

    PDFObjectElement* pCatalog = pRoot->LookupObject();
    if (!pCatalog)
    {
        SAL_WARN("vcl.filter", "PDFDocument::GetPages: trailer has no catalog");
        return aRet;
    }

    PDFObjectElement* pPages = pCatalog->LookupObject("Pages");
    if (!pPages)
    {
        SAL_WARN("vcl.filter", "PDFDocument::GetPages: catalog (obj " << pCatalog->GetObjectValue()
                                                                      << ") has no pages");
        return aRet;
    }

    visitPages(pPages, aRet);

    return aRet;
}

void PDFDocument::PushBackEOF(size_t nOffset) { m_aEOFs.push_back(nOffset); }

std::vector<PDFObjectElement*> PDFDocument::GetSignatureWidgets()
{
    std::vector<PDFObjectElement*> aRet;

    std::vector<PDFObjectElement*> aPages = GetPages();

    for (const auto& pPage : aPages)
    {
        if (!pPage)
            continue;

        PDFElement* pAnnotsElement = pPage->Lookup("Annots");
        auto pAnnots = dynamic_cast<PDFArrayElement*>(pAnnotsElement);
        if (!pAnnots)
        {
            // Annots is not an array, see if it's a reference to an object
            // with a direct array.
            auto pAnnotsRef = dynamic_cast<PDFReferenceElement*>(pAnnotsElement);
            if (pAnnotsRef)
            {
                if (PDFObjectElement* pAnnotsObject = pAnnotsRef->LookupObject())
                {
                    pAnnots = pAnnotsObject->GetArray();
                }
            }
        }

        if (!pAnnots)
            continue;

        for (const auto& pAnnot : pAnnots->GetElements())
        {
            auto pReference = dynamic_cast<PDFReferenceElement*>(pAnnot);
            if (!pReference)
                continue;

            PDFObjectElement* pAnnotObject = pReference->LookupObject();
            if (!pAnnotObject)
                continue;

            auto pFT = dynamic_cast<PDFNameElement*>(pAnnotObject->Lookup("FT"));
            if (!pFT || pFT->GetValue() != "Sig")
                continue;

            aRet.push_back(pAnnotObject);
        }
    }

    return aRet;
}

std::vector<unsigned char> PDFDocument::DecodeHexString(PDFHexStringElement const* pElement)
{
    return svl::crypto::DecodeHexString(pElement->GetValue());
}

PDFCommentElement::PDFCommentElement(PDFDocument& rDoc)
    : m_rDoc(rDoc)
{
}

bool PDFCommentElement::Read(SvStream& rStream)
{
    // Read from (including) the % char till (excluding) the end of the line/stream.
    OStringBuffer aBuf;
    char ch;
    rStream.ReadChar(ch);
    while (true)
    {
        if (ch == '\n' || ch == '\r' || rStream.eof())
        {
            m_aComment = aBuf.makeStringAndClear();

            if (m_aComment.startsWith("%%EOF"))
                m_rDoc.PushBackEOF(rStream.Tell());

            SAL_INFO("vcl.filter", "PDFCommentElement::Read: m_aComment is '" << m_aComment << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

PDFNumberElement::PDFNumberElement() = default;

bool PDFNumberElement::Read(SvStream& rStream)
{
    OStringBuffer aBuf;
    m_nOffset = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (rStream.eof())
    {
        return false;
    }
    if (!rtl::isAsciiDigit(static_cast<unsigned char>(ch)) && ch != '-' && ch != '.')
    {
        rStream.SeekRel(-1);
        return false;
    }
    while (!rStream.eof())
    {
        if (!rtl::isAsciiDigit(static_cast<unsigned char>(ch)) && ch != '-' && ch != '.')
        {
            rStream.SeekRel(-1);
            m_nLength = rStream.Tell() - m_nOffset;
            m_fValue = aBuf.makeStringAndClear().toDouble();
            SAL_INFO("vcl.filter", "PDFNumberElement::Read: m_fValue is '" << m_fValue << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

sal_uInt64 PDFNumberElement::GetLocation() const { return m_nOffset; }

sal_uInt64 PDFNumberElement::GetLength() const { return m_nLength; }

PDFBooleanElement::PDFBooleanElement(bool /*bValue*/) {}

bool PDFBooleanElement::Read(SvStream& /*rStream*/) { return true; }

bool PDFNullElement::Read(SvStream& /*rStream*/) { return true; }

bool PDFHexStringElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_INFO("vcl.filter", "PDFHexStringElement::Read: expected '<' as first character");
        return false;
    }
    rStream.ReadChar(ch);

    OStringBuffer aBuf;
    while (!rStream.eof())
    {
        if (ch == '>')
        {
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("vcl.filter",
                     "PDFHexStringElement::Read: m_aValue length is " << m_aValue.getLength());
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFHexStringElement::GetValue() const { return m_aValue; }

bool PDFLiteralStringElement::Read(SvStream& rStream)
{
    char nPrevCh = 0;
    char ch = 0;
    rStream.ReadChar(ch);
    if (ch != '(')
    {
        SAL_INFO("vcl.filter", "PDFHexStringElement::Read: expected '(' as first character");
        return false;
    }
    nPrevCh = ch;
    rStream.ReadChar(ch);

    // Start with 1 nesting level as we read a '(' above already.
    int nDepth = 1;
    OStringBuffer aBuf;
    while (!rStream.eof())
    {
        if (ch == '(' && nPrevCh != '\\')
            ++nDepth;

        if (ch == ')' && nPrevCh != '\\')
            --nDepth;

        if (nDepth == 0)
        {
            // ')' of the outermost '(' is reached.
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("vcl.filter",
                     "PDFLiteralStringElement::Read: m_aValue is '" << m_aValue << "'");
            return true;
        }
        aBuf.append(ch);
        nPrevCh = ch;
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFLiteralStringElement::GetValue() const { return m_aValue; }

PDFTrailerElement::PDFTrailerElement(PDFDocument& rDoc)
    : m_rDoc(rDoc)
{
}

bool PDFTrailerElement::Read(SvStream& rStream)
{
    m_nOffset = rStream.Tell();
    return true;
}

PDFElement* PDFTrailerElement::Lookup(const OString& rDictionaryKey)
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return PDFDictionaryElement::Lookup(m_aDictionary, rDictionaryKey);
}

sal_uInt64 PDFTrailerElement::GetLocation() const { return m_nOffset; }

double PDFNumberElement::GetValue() const { return m_fValue; }

PDFObjectElement::PDFObjectElement(PDFDocument& rDoc, double fObjectValue, double fGenerationValue)
    : m_rDoc(rDoc)
    , m_fObjectValue(fObjectValue)
    , m_fGenerationValue(fGenerationValue)
    , m_pNumberElement(nullptr)
    , m_nDictionaryOffset(0)
    , m_nDictionaryLength(0)
    , m_pDictionaryElement(nullptr)
    , m_nArrayOffset(0)
    , m_nArrayLength(0)
    , m_pArrayElement(nullptr)
    , m_pStreamElement(nullptr)
{
}

bool PDFObjectElement::Read(SvStream& /*rStream*/)
{
    SAL_INFO("vcl.filter",
             "PDFObjectElement::Read: " << m_fObjectValue << " " << m_fGenerationValue << " obj");
    return true;
}

PDFDictionaryElement::PDFDictionaryElement() = default;

size_t PDFDictionaryElement::Parse(const std::vector<std::unique_ptr<PDFElement>>& rElements,
                                   PDFElement* pThis, std::map<OString, PDFElement*>& rDictionary)
{
    // The index of last parsed element, in case of nested dictionaries.
    size_t nRet = 0;

    if (!rDictionary.empty())
        return nRet;

    pThis->setParsing(true);

    auto pThisObject = dynamic_cast<PDFObjectElement*>(pThis);
    // This is set to non-nullptr here for nested dictionaries only.
    auto pThisDictionary = dynamic_cast<PDFDictionaryElement*>(pThis);

    // Find out where the dictionary for this object starts.
    size_t nIndex = 0;
    for (size_t i = 0; i < rElements.size(); ++i)
    {
        if (rElements[i].get() == pThis)
        {
            nIndex = i;
            break;
        }
    }

    OString aName;
    sal_uInt64 nNameOffset = 0;
    std::vector<PDFNumberElement*> aNumbers;
    // The array value we're in -- if any.
    PDFArrayElement* pArray = nullptr;
    sal_uInt64 nDictionaryOffset = 0;
    int nDictionaryDepth = 0;
    // Toplevel dictionary found (not inside an array).
    bool bDictionaryFound = false;
    // Toplevel array found (not inside a dictionary).
    bool bArrayFound = false;
    for (size_t i = nIndex; i < rElements.size(); ++i)
    {
        // Dictionary tokens can be nested, track enter/leave.
        if (auto pDictionary = dynamic_cast<PDFDictionaryElement*>(rElements[i].get()))
        {
            bDictionaryFound = true;
            if (++nDictionaryDepth == 1)
            {
                // First dictionary start, track start offset.
                nDictionaryOffset = pDictionary->m_nLocation;
                if (pThisObject)
                {
                    if (!bArrayFound)
                        // Then the toplevel dictionary of the object.
                        pThisObject->SetDictionary(pDictionary);
                    pThisDictionary = pDictionary;
                    pThisObject->SetDictionaryOffset(nDictionaryOffset);
                }
            }
            else if (!pDictionary->alreadyParsing())
            {
                // Nested dictionary.
                const size_t nexti
                    = PDFDictionaryElement::Parse(rElements, pDictionary, pDictionary->m_aItems);
                if (nexti >= i) // ensure we go forwards and not endlessly loop
                {
                    i = nexti;
                    rDictionary[aName] = pDictionary;
                    aName.clear();
                }
            }
        }

        if (auto pEndDictionary = dynamic_cast<PDFEndDictionaryElement*>(rElements[i].get()))
        {
            if (--nDictionaryDepth == 0)
            {
                // Last dictionary end, track length and stop parsing.
                if (pThisObject)
                    pThisObject->SetDictionaryLength(pEndDictionary->GetLocation()
                                                     - nDictionaryOffset);
                nRet = i;
                break;
            }
        }

        auto pName = dynamic_cast<PDFNameElement*>(rElements[i].get());
        if (pName)
        {
            if (!aNumbers.empty())
            {
                PDFNumberElement* pNumber = aNumbers.back();
                rDictionary[aName] = pNumber;
                if (pThisDictionary)
                {
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                    pThisDictionary->SetKeyValueLength(
                        aName, pNumber->GetLocation() + pNumber->GetLength() - nNameOffset);
                }
                aName.clear();
                aNumbers.clear();
            }

            if (aName.isEmpty())
            {
                // Remember key.
                aName = pName->GetValue();
                nNameOffset = pName->GetLocation();
            }
            else
            {
                if (pArray)
                {
                    if (bDictionaryFound)
                        // Array inside dictionary.
                        pArray->PushBack(pName);
                }
                else
                {
                    // Name-name key-value.
                    rDictionary[aName] = pName;
                    if (pThisDictionary)
                    {
                        pThisDictionary->SetKeyOffset(aName, nNameOffset);
                        pThisDictionary->SetKeyValueLength(aName, pName->GetLocation()
                                                                      + PDFNameElement::GetLength()
                                                                      - nNameOffset);
                    }
                    aName.clear();
                }
            }
            continue;
        }

        auto pArr = dynamic_cast<PDFArrayElement*>(rElements[i].get());
        if (pArr)
        {
            bArrayFound = true;
            pArray = pArr;
            continue;
        }

        auto pEndArr = dynamic_cast<PDFEndArrayElement*>(rElements[i].get());
        if (pArray && pEndArr)
        {
            for (auto& pNumber : aNumbers)
                pArray->PushBack(pNumber);
            aNumbers.clear();
            rDictionary[aName] = pArray;
            if (pThisDictionary)
            {
                pThisDictionary->SetKeyOffset(aName, nNameOffset);
                // Include the ending ']' in the length of the key - (array)value pair length.
                pThisDictionary->SetKeyValueLength(aName, pEndArr->GetOffset() - nNameOffset + 1);
            }
            aName.clear();
            pArray = nullptr;
            continue;
        }

        auto pReference = dynamic_cast<PDFReferenceElement*>(rElements[i].get());
        if (pReference)
        {
            if (!pArray)
            {
                rDictionary[aName] = pReference;
                if (pThisDictionary)
                {
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                    pThisDictionary->SetKeyValueLength(aName,
                                                       pReference->GetOffset() - nNameOffset);
                }
                aName.clear();
            }
            else
            {
                if (bDictionaryFound)
                    // Array inside dictionary.
                    pArray->PushBack(pReference);
            }
            aNumbers.clear();
            continue;
        }

        auto pLiteralString = dynamic_cast<PDFLiteralStringElement*>(rElements[i].get());
        if (pLiteralString)
        {
            rDictionary[aName] = pLiteralString;
            if (pThisDictionary)
                pThisDictionary->SetKeyOffset(aName, nNameOffset);
            aName.clear();
            continue;
        }

        auto pBoolean = dynamic_cast<PDFBooleanElement*>(rElements[i].get());
        if (pBoolean)
        {
            rDictionary[aName] = pBoolean;
            if (pThisDictionary)
                pThisDictionary->SetKeyOffset(aName, nNameOffset);
            aName.clear();
            continue;
        }

        auto pHexString = dynamic_cast<PDFHexStringElement*>(rElements[i].get());
        if (pHexString)
        {
            if (!pArray)
            {
                rDictionary[aName] = pHexString;
                if (pThisDictionary)
                    pThisDictionary->SetKeyOffset(aName, nNameOffset);
                aName.clear();
            }
            else
            {
                pArray->PushBack(pHexString);
            }
            continue;
        }

        if (dynamic_cast<PDFEndObjectElement*>(rElements[i].get()))
            break;

        // Just remember this, so that in case it's not a reference parameter,
        // we can handle it later.
        auto pNumber = dynamic_cast<PDFNumberElement*>(rElements[i].get());
        if (pNumber)
            aNumbers.push_back(pNumber);
    }

    if (!aNumbers.empty())
    {
        rDictionary[aName] = aNumbers.back();
        if (pThisDictionary)
            pThisDictionary->SetKeyOffset(aName, nNameOffset);
        aName.clear();
        aNumbers.clear();
    }

    pThis->setParsing(false);

    return nRet;
}

PDFElement* PDFDictionaryElement::Lookup(const std::map<OString, PDFElement*>& rDictionary,
                                         const OString& rKey)
{
    auto it = rDictionary.find(rKey);
    if (it == rDictionary.end())
        return nullptr;

    return it->second;
}

PDFObjectElement* PDFDictionaryElement::LookupObject(const OString& rDictionaryKey)
{
    auto pKey = dynamic_cast<PDFReferenceElement*>(
        PDFDictionaryElement::Lookup(m_aItems, rDictionaryKey));
    if (!pKey)
    {
        SAL_WARN("vcl.filter",
                 "PDFDictionaryElement::LookupObject: no such key with reference value: "
                     << rDictionaryKey);
        return nullptr;
    }

    return pKey->LookupObject();
}

PDFElement* PDFDictionaryElement::LookupElement(const OString& rDictionaryKey)
{
    return PDFDictionaryElement::Lookup(m_aItems, rDictionaryKey);
}

PDFElement* PDFObjectElement::Lookup(const OString& rDictionaryKey)
{
    if (m_aDictionary.empty())
    {
        if (!m_aElements.empty())
            // This is a stored object in an object stream.
            PDFDictionaryElement::Parse(m_aElements, this, m_aDictionary);
        else
            // Normal object: elements are stored as members of the document itself.
            PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);
    }

    return PDFDictionaryElement::Lookup(m_aDictionary, rDictionaryKey);
}

PDFObjectElement* PDFObjectElement::LookupObject(const OString& rDictionaryKey)
{
    auto pKey = dynamic_cast<PDFReferenceElement*>(Lookup(rDictionaryKey));
    if (!pKey)
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::LookupObject: no such key with reference value: "
                                   << rDictionaryKey);
        return nullptr;
    }

    return pKey->LookupObject();
}

double PDFObjectElement::GetObjectValue() const { return m_fObjectValue; }

void PDFObjectElement::SetDictionaryOffset(sal_uInt64 nDictionaryOffset)
{
    m_nDictionaryOffset = nDictionaryOffset;
}

sal_uInt64 PDFObjectElement::GetDictionaryOffset()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return m_nDictionaryOffset;
}

void PDFObjectElement::SetArrayOffset(sal_uInt64 nArrayOffset) { m_nArrayOffset = nArrayOffset; }

sal_uInt64 PDFObjectElement::GetArrayOffset() { return m_nArrayOffset; }

void PDFDictionaryElement::SetKeyOffset(const OString& rKey, sal_uInt64 nOffset)
{
    m_aDictionaryKeyOffset[rKey] = nOffset;
}

void PDFDictionaryElement::SetKeyValueLength(const OString& rKey, sal_uInt64 nLength)
{
    m_aDictionaryKeyValueLength[rKey] = nLength;
}

sal_uInt64 PDFDictionaryElement::GetKeyOffset(const OString& rKey) const
{
    auto it = m_aDictionaryKeyOffset.find(rKey);
    if (it == m_aDictionaryKeyOffset.end())
        return 0;

    return it->second;
}

sal_uInt64 PDFDictionaryElement::GetKeyValueLength(const OString& rKey) const
{
    auto it = m_aDictionaryKeyValueLength.find(rKey);
    if (it == m_aDictionaryKeyValueLength.end())
        return 0;

    return it->second;
}

const std::map<OString, PDFElement*>& PDFDictionaryElement::GetItems() const { return m_aItems; }

void PDFObjectElement::SetDictionaryLength(sal_uInt64 nDictionaryLength)
{
    m_nDictionaryLength = nDictionaryLength;
}

sal_uInt64 PDFObjectElement::GetDictionaryLength()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return m_nDictionaryLength;
}

void PDFObjectElement::SetArrayLength(sal_uInt64 nArrayLength) { m_nArrayLength = nArrayLength; }

sal_uInt64 PDFObjectElement::GetArrayLength() { return m_nArrayLength; }

PDFDictionaryElement* PDFObjectElement::GetDictionary()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);
    return m_pDictionaryElement;
}

void PDFObjectElement::SetDictionary(PDFDictionaryElement* pDictionaryElement)
{
    m_pDictionaryElement = pDictionaryElement;
}

void PDFObjectElement::SetNumberElement(PDFNumberElement* pNumberElement)
{
    m_pNumberElement = pNumberElement;
}

PDFNumberElement* PDFObjectElement::GetNumberElement() const { return m_pNumberElement; }

const std::vector<PDFReferenceElement*>& PDFObjectElement::GetDictionaryReferences() const
{
    return m_aDictionaryReferences;
}

void PDFObjectElement::AddDictionaryReference(PDFReferenceElement* pReference)
{
    m_aDictionaryReferences.push_back(pReference);
}

const std::map<OString, PDFElement*>& PDFObjectElement::GetDictionaryItems()
{
    if (m_aDictionary.empty())
        PDFDictionaryElement::Parse(m_rDoc.GetElements(), this, m_aDictionary);

    return m_aDictionary;
}

void PDFObjectElement::SetArray(PDFArrayElement* pArrayElement) { m_pArrayElement = pArrayElement; }

void PDFObjectElement::SetStream(PDFStreamElement* pStreamElement)
{
    m_pStreamElement = pStreamElement;
}

PDFStreamElement* PDFObjectElement::GetStream() const { return m_pStreamElement; }

PDFArrayElement* PDFObjectElement::GetArray() const { return m_pArrayElement; }

void PDFObjectElement::ParseStoredObjects()
{
    if (!m_pStreamElement)
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::ParseStoredObjects: no stream");
        return;
    }

    auto pType = dynamic_cast<PDFNameElement*>(Lookup("Type"));
    if (!pType || pType->GetValue() != "ObjStm")
    {
        if (!pType)
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: missing unexpected type");
        else
            SAL_WARN("vcl.filter",
                     "PDFDocument::ReadXRefStream: unexpected type: " << pType->GetValue());
        return;
    }

    auto pFilter = dynamic_cast<PDFNameElement*>(Lookup("Filter"));
    if (!pFilter || pFilter->GetValue() != "FlateDecode")
    {
        if (!pFilter)
            SAL_WARN("vcl.filter", "PDFDocument::ReadXRefStream: missing filter");
        else
            SAL_WARN("vcl.filter",
                     "PDFDocument::ReadXRefStream: unexpected filter: " << pFilter->GetValue());
        return;
    }

    auto pFirst = dynamic_cast<PDFNumberElement*>(Lookup("First"));
    if (!pFirst)
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::ParseStoredObjects: no First");
        return;
    }

    auto pN = dynamic_cast<PDFNumberElement*>(Lookup("N"));
    if (!pN)
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::ParseStoredObjects: no N");
        return;
    }
    size_t nN = pN->GetValue();

    auto pLength = dynamic_cast<PDFNumberElement*>(Lookup("Length"));
    if (!pLength)
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::ParseStoredObjects: no length");
        return;
    }
    size_t nLength = pLength->GetValue();

    // Read and decompress it.
    SvMemoryStream& rEditBuffer = m_rDoc.GetEditBuffer();
    rEditBuffer.Seek(m_pStreamElement->GetOffset());
    std::vector<char> aBuf(nLength);
    rEditBuffer.ReadBytes(aBuf.data(), aBuf.size());
    SvMemoryStream aSource(aBuf.data(), aBuf.size(), StreamMode::READ);
    SvMemoryStream aStream;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    aZCodec.Decompress(aSource, aStream);
    if (!aZCodec.EndCompression())
    {
        SAL_WARN("vcl.filter", "PDFObjectElement::ParseStoredObjects: decompression failed");
        return;
    }

    nLength = aStream.TellEnd();
    aStream.Seek(0);
    std::vector<size_t> aObjNums;
    std::vector<size_t> aOffsets;
    std::vector<size_t> aLengths;
    // First iterate over and find out the lengths.
    for (size_t nObject = 0; nObject < nN; ++nObject)
    {
        PDFNumberElement aObjNum;
        if (!aObjNum.Read(aStream))
        {
            SAL_WARN("vcl.filter",
                     "PDFObjectElement::ParseStoredObjects: failed to read object number");
            return;
        }
        aObjNums.push_back(aObjNum.GetValue());

        PDFDocument::SkipWhitespace(aStream);

        PDFNumberElement aByteOffset;
        if (!aByteOffset.Read(aStream))
        {
            SAL_WARN("vcl.filter",
                     "PDFObjectElement::ParseStoredObjects: failed to read byte offset");
            return;
        }
        aOffsets.push_back(pFirst->GetValue() + aByteOffset.GetValue());

        if (aOffsets.size() > 1)
            aLengths.push_back(aOffsets.back() - aOffsets[aOffsets.size() - 2]);
        if (nObject + 1 == nN)
            aLengths.push_back(nLength - aOffsets.back());

        PDFDocument::SkipWhitespace(aStream);
    }

    // Now create streams with the proper length and tokenize the data.
    for (size_t nObject = 0; nObject < nN; ++nObject)
    {
        size_t nObjNum = aObjNums[nObject];
        size_t nOffset = aOffsets[nObject];
        size_t nLen = aLengths[nObject];

        aStream.Seek(nOffset);
        m_aStoredElements.push_back(std::make_unique<PDFObjectElement>(m_rDoc, nObjNum, 0));
        PDFObjectElement* pStored = m_aStoredElements.back().get();

        aBuf.clear();
        aBuf.resize(nLen);
        aStream.ReadBytes(aBuf.data(), aBuf.size());
        SvMemoryStream aStoredStream(aBuf.data(), aBuf.size(), StreamMode::READ);

        m_rDoc.Tokenize(aStoredStream, TokenizeMode::STORED_OBJECT, pStored->GetStoredElements(),
                        pStored);
        // This is how references know the object is stored inside this object stream.
        m_rDoc.SetIDObject(nObjNum, pStored);

        // Store the stream of the object in the object stream for later use.
        std::unique_ptr<SvMemoryStream> pStreamBuffer(new SvMemoryStream());
        aStoredStream.Seek(0);
        pStreamBuffer->WriteStream(aStoredStream);
        pStored->SetStreamBuffer(pStreamBuffer);
    }
}

std::vector<std::unique_ptr<PDFElement>>& PDFObjectElement::GetStoredElements()
{
    return m_aElements;
}

SvMemoryStream* PDFObjectElement::GetStreamBuffer() const { return m_pStreamBuffer.get(); }

void PDFObjectElement::SetStreamBuffer(std::unique_ptr<SvMemoryStream>& pStreamBuffer)
{
    m_pStreamBuffer = std::move(pStreamBuffer);
}

PDFDocument& PDFObjectElement::GetDocument() { return m_rDoc; }

PDFReferenceElement::PDFReferenceElement(PDFDocument& rDoc, PDFNumberElement& rObject,
                                         PDFNumberElement const& rGeneration)
    : m_rDoc(rDoc)
    , m_fObjectValue(rObject.GetValue())
    , m_fGenerationValue(rGeneration.GetValue())
    , m_rObject(rObject)
{
}

PDFNumberElement& PDFReferenceElement::GetObjectElement() const { return m_rObject; }

bool PDFReferenceElement::Read(SvStream& rStream)
{
    SAL_INFO("vcl.filter",
             "PDFReferenceElement::Read: " << m_fObjectValue << " " << m_fGenerationValue << " R");
    m_nOffset = rStream.Tell();
    return true;
}

sal_uInt64 PDFReferenceElement::GetOffset() const { return m_nOffset; }

double PDFReferenceElement::LookupNumber(SvStream& rStream) const
{
    size_t nOffset = m_rDoc.GetObjectOffset(m_fObjectValue);
    if (nOffset == 0)
    {
        SAL_WARN("vcl.filter", "PDFReferenceElement::LookupNumber: found no offset for object #"
                                   << m_fObjectValue);
        return 0;
    }

    sal_uInt64 nOrigPos = rStream.Tell();
    comphelper::ScopeGuard g([&]() { rStream.Seek(nOrigPos); });

    rStream.Seek(nOffset);
    {
        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumber;
        bool bRet = aNumber.Read(rStream);
        if (!bRet || aNumber.GetValue() != m_fObjectValue)
        {
            SAL_WARN("vcl.filter",
                     "PDFReferenceElement::LookupNumber: offset points to not matching object");
            return 0;
        }
    }

    {
        PDFDocument::SkipWhitespace(rStream);
        PDFNumberElement aNumber;
        bool bRet = aNumber.Read(rStream);
        if (!bRet || aNumber.GetValue() != m_fGenerationValue)
        {
            SAL_WARN("vcl.filter",
                     "PDFReferenceElement::LookupNumber: offset points to not matching generation");
            return 0;
        }
    }

    {
        PDFDocument::SkipWhitespace(rStream);
        OString aKeyword = PDFDocument::ReadKeyword(rStream);
        if (aKeyword != "obj")
        {
            SAL_WARN("vcl.filter",
                     "PDFReferenceElement::LookupNumber: offset doesn't point to an obj keyword");
            return 0;
        }
    }

    PDFDocument::SkipWhitespace(rStream);
    PDFNumberElement aNumber;
    if (!aNumber.Read(rStream))
    {
        SAL_WARN("vcl.filter",
                 "PDFReferenceElement::LookupNumber: failed to read referenced number");
        return 0;
    }

    return aNumber.GetValue();
}

PDFObjectElement* PDFReferenceElement::LookupObject()
{
    return m_rDoc.LookupObject(m_fObjectValue);
}

PDFObjectElement* PDFDocument::LookupObject(size_t nObjectNumber)
{
    auto itIDObjects = m_aIDObjects.find(nObjectNumber);

    if (itIDObjects != m_aIDObjects.end())
        return itIDObjects->second;

    SAL_WARN("vcl.filter", "PDFDocument::LookupObject: can't find obj " << nObjectNumber);
    return nullptr;
}

SvMemoryStream& PDFDocument::GetEditBuffer() { return m_aEditBuffer; }

int PDFReferenceElement::GetObjectValue() const { return m_fObjectValue; }

int PDFReferenceElement::GetGenerationValue() const { return m_fGenerationValue; }

bool PDFDictionaryElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_WARN("vcl.filter", "PDFDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    if (rStream.eof())
    {
        SAL_WARN("vcl.filter", "PDFDictionaryElement::Read: unexpected end of file");
        return false;
    }

    rStream.ReadChar(ch);
    if (ch != '<')
    {
        SAL_WARN("vcl.filter", "PDFDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    m_nLocation = rStream.Tell();

    SAL_INFO("vcl.filter", "PDFDictionaryElement::Read: '<<'");

    return true;
}

PDFEndDictionaryElement::PDFEndDictionaryElement() = default;

sal_uInt64 PDFEndDictionaryElement::GetLocation() const { return m_nLocation; }

bool PDFEndDictionaryElement::Read(SvStream& rStream)
{
    m_nLocation = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (ch != '>')
    {
        SAL_WARN("vcl.filter", "PDFEndDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    if (rStream.eof())
    {
        SAL_WARN("vcl.filter", "PDFEndDictionaryElement::Read: unexpected end of file");
        return false;
    }

    rStream.ReadChar(ch);
    if (ch != '>')
    {
        SAL_WARN("vcl.filter", "PDFEndDictionaryElement::Read: unexpected character: " << ch);
        return false;
    }

    SAL_INFO("vcl.filter", "PDFEndDictionaryElement::Read: '>>'");

    return true;
}

PDFNameElement::PDFNameElement() = default;

bool PDFNameElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '/')
    {
        SAL_WARN("vcl.filter", "PDFNameElement::Read: unexpected character: " << ch);
        return false;
    }
    m_nLocation = rStream.Tell();

    if (rStream.eof())
    {
        SAL_WARN("vcl.filter", "PDFNameElement::Read: unexpected end of file");
        return false;
    }

    // Read till the first white-space.
    OStringBuffer aBuf;
    rStream.ReadChar(ch);
    while (!rStream.eof())
    {
        if (rtl::isAsciiWhiteSpace(static_cast<unsigned char>(ch)) || ch == '/' || ch == '['
            || ch == ']' || ch == '<' || ch == '>' || ch == '(')
        {
            rStream.SeekRel(-1);
            m_aValue = aBuf.makeStringAndClear();
            SAL_INFO("vcl.filter", "PDFNameElement::Read: m_aValue is '" << m_aValue << "'");
            return true;
        }
        aBuf.append(ch);
        rStream.ReadChar(ch);
    }

    return false;
}

const OString& PDFNameElement::GetValue() const { return m_aValue; }

sal_uInt64 PDFNameElement::GetLocation() const { return m_nLocation; }

PDFStreamElement::PDFStreamElement(size_t nLength)
    : m_nLength(nLength)
    , m_nOffset(0)
{
}

bool PDFStreamElement::Read(SvStream& rStream)
{
    SAL_INFO("vcl.filter", "PDFStreamElement::Read: length is " << m_nLength);
    m_nOffset = rStream.Tell();
    std::vector<unsigned char> aBytes(m_nLength);
    rStream.ReadBytes(aBytes.data(), aBytes.size());
    m_aMemory.WriteBytes(aBytes.data(), aBytes.size());

    return rStream.good();
}

SvMemoryStream& PDFStreamElement::GetMemory() { return m_aMemory; }

sal_uInt64 PDFStreamElement::GetOffset() const { return m_nOffset; }

bool PDFEndStreamElement::Read(SvStream& /*rStream*/) { return true; }

bool PDFEndObjectElement::Read(SvStream& /*rStream*/) { return true; }

PDFArrayElement::PDFArrayElement(PDFObjectElement* pObject)
    : m_pObject(pObject)
{
}

bool PDFArrayElement::Read(SvStream& rStream)
{
    char ch;
    rStream.ReadChar(ch);
    if (ch != '[')
    {
        SAL_WARN("vcl.filter", "PDFArrayElement::Read: unexpected character: " << ch);
        return false;
    }

    SAL_INFO("vcl.filter", "PDFArrayElement::Read: '['");

    return true;
}

void PDFArrayElement::PushBack(PDFElement* pElement)
{
    if (m_pObject)
        SAL_INFO("vcl.filter",
                 "PDFArrayElement::PushBack: object is " << m_pObject->GetObjectValue());
    m_aElements.push_back(pElement);
}

const std::vector<PDFElement*>& PDFArrayElement::GetElements() { return m_aElements; }

PDFEndArrayElement::PDFEndArrayElement() = default;

bool PDFEndArrayElement::Read(SvStream& rStream)
{
    m_nOffset = rStream.Tell();
    char ch;
    rStream.ReadChar(ch);
    if (ch != ']')
    {
        SAL_WARN("vcl.filter", "PDFEndArrayElement::Read: unexpected character: " << ch);
        return false;
    }

    SAL_INFO("vcl.filter", "PDFEndArrayElement::Read: ']'");

    return true;
}

sal_uInt64 PDFEndArrayElement::GetOffset() const { return m_nOffset; }

} // namespace filter
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
