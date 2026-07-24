/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <lib/l10ntranslate.hxx>

#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string_view>

namespace desktop::l10n
{
namespace
{
constexpr const char* TEXT_NS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
constexpr const char* TABLE_NS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
constexpr const char* STYLE_NS = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
constexpr const char* CALCEXT_NS
    = "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0";

// The closed set of attributes that may reference a sheet by name; renaming a
// marked sheet rewrites exactly these (the extractor lint in online enforces
// that marked names appear nowhere else).
struct SheetRefAttr
{
    const char* pNsHref;
    const char* pLocalName;
};
constexpr SheetRefAttr aSheetRefAttrs[] = {
    { TABLE_NS, "base-cell-address" },
    { STYLE_NS, "base-cell-address" },
    { CALCEXT_NS, "target-range-address" },
    { CALCEXT_NS, "base-cell-address" },
};

OUString toFileURL(const OUString& rPathOrUrl)
{
    if (rPathOrUrl.startsWithIgnoreAsciiCase(u"file:"))
        return rPathOrUrl;
    OUString aURL;
    if (osl::FileBase::getFileURLFromSystemPath(rPathOrUrl, aURL) == osl::FileBase::E_None)
        return aURL;
    return rPathOrUrl;
}

bool readFile(const OUString& rPathOrUrl, std::vector<sal_uInt8>& rBytes)
{
    osl::File aFile(toFileURL(rPathOrUrl));
    if (aFile.open(osl_File_OpenFlag_Read) != osl::FileBase::E_None)
        return false;
    sal_uInt64 nSize = 0;
    if (aFile.getSize(nSize) != osl::FileBase::E_None)
        return false;
    rBytes.resize(nSize);
    sal_uInt64 nRead = 0;
    if (nSize != 0
        && (aFile.read(rBytes.data(), nSize, nRead) != osl::FileBase::E_None || nRead != nSize))
        return false;
    return true;
}

bool writeFile(const OUString& rPathOrUrl, const void* pData, sal_uInt64 nSize)
{
    const OUString aURL = toFileURL(rPathOrUrl);
    const OUString aTmpURL = aURL + ".l10ntmp";
    osl::File::remove(aTmpURL);
    osl::File aFile(aTmpURL);
    if (aFile.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create) != osl::FileBase::E_None)
        return false;
    sal_uInt64 nWritten = 0;
    if (nSize != 0
        && (aFile.write(pData, nSize, nWritten) != osl::FileBase::E_None || nWritten != nSize))
    {
        aFile.close();
        osl::File::remove(aTmpURL);
        return false;
    }
    aFile.close();
    if (osl::File::move(aTmpURL, aURL) != osl::FileBase::E_None)
    {
        // some filesystems refuse a replacing move; retry after a remove
        osl::File::remove(aURL);
        if (osl::File::move(aTmpURL, aURL) != osl::FileBase::E_None)
        {
            osl::File::remove(aTmpURL);
            return false;
        }
    }
    return true;
}

sal_uInt16 readU16(const std::vector<sal_uInt8>& rB, size_t nPos)
{
    return static_cast<sal_uInt16>(rB[nPos]) | (static_cast<sal_uInt16>(rB[nPos + 1]) << 8);
}

sal_uInt32 readU32(const std::vector<sal_uInt8>& rB, size_t nPos)
{
    return static_cast<sal_uInt32>(rB[nPos]) | (static_cast<sal_uInt32>(rB[nPos + 1]) << 8)
           | (static_cast<sal_uInt32>(rB[nPos + 2]) << 16)
           | (static_cast<sal_uInt32>(rB[nPos + 3]) << 24);
}

void pushU16(std::vector<sal_uInt8>& rB, sal_uInt16 n)
{
    rB.push_back(n & 0xff);
    rB.push_back((n >> 8) & 0xff);
}

void pushU32(std::vector<sal_uInt8>& rB, sal_uInt32 n)
{
    rB.push_back(n & 0xff);
    rB.push_back((n >> 8) & 0xff);
    rB.push_back((n >> 16) & 0xff);
    rB.push_back((n >> 24) & 0xff);
}

struct ZipEntryInfo
{
    OString aName;
    sal_uInt16 nFlags = 0;
    sal_uInt16 nMethod = 0;
    sal_uInt32 nCrc = 0;
    sal_uInt32 nCompressedSize = 0;
    sal_uInt32 nUncompressedSize = 0;
    // byte ranges within the archive buffer
    size_t nLocalStart = 0;
    size_t nLocalLen = 0; // local header + payload (+ data descriptor)
    size_t nDataStart = 0; // payload begin
    size_t nCentralStart = 0;
    size_t nCentralLen = 0; // central record incl. name/extra/comment
};

bool inflateRaw(const sal_uInt8* pIn, size_t nIn, std::vector<sal_uInt8>& rOut, size_t nExpected)
{
    rOut.resize(nExpected);
    if (nExpected == 0)
        return true; // an empty member may still carry a tiny deflate stream
    z_stream aStrm;
    std::memset(&aStrm, 0, sizeof(aStrm));
    if (inflateInit2(&aStrm, -MAX_WBITS) != Z_OK)
        return false;
    aStrm.next_in = pIn;
    aStrm.avail_in = static_cast<uInt>(nIn);
    aStrm.next_out = rOut.data();
    aStrm.avail_out = static_cast<uInt>(nExpected);
    const int nRet = inflate(&aStrm, Z_FINISH);
    inflateEnd(&aStrm);
    return nRet == Z_STREAM_END && aStrm.total_out == nExpected;
}

bool deflateRaw(const sal_uInt8* pIn, size_t nIn, std::vector<sal_uInt8>& rOut)
{
    z_stream aStrm;
    std::memset(&aStrm, 0, sizeof(aStrm));
    if (deflateInit2(&aStrm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY)
        != Z_OK)
        return false;
    rOut.resize(deflateBound(&aStrm, nIn));
    aStrm.next_in = pIn;
    aStrm.avail_in = static_cast<uInt>(nIn);
    aStrm.next_out = rOut.data();
    aStrm.avail_out = static_cast<uInt>(rOut.size());
    const int nRet = deflate(&aStrm, Z_FINISH);
    const size_t nOut = aStrm.total_out;
    deflateEnd(&aStrm);
    if (nRet != Z_STREAM_END)
        return false;
    rOut.resize(nOut);
    return true;
}

struct ZipArchive
{
    std::vector<sal_uInt8> aBytes;
    std::vector<ZipEntryInfo> aEntries; // central-directory order

    bool parse();
    const ZipEntryInfo* find(std::string_view aName) const
    {
        for (const auto& rEntry : aEntries)
            if (std::string_view(rEntry.aName) == aName)
                return &rEntry;
        return nullptr;
    }
    bool readEntry(const ZipEntryInfo& rEntry, std::vector<sal_uInt8>& rOut) const
    {
        if (rEntry.nMethod == 0)
        {
            if (rEntry.nCompressedSize != rEntry.nUncompressedSize)
                return false;
            rOut.assign(aBytes.begin() + rEntry.nDataStart,
                        aBytes.begin() + rEntry.nDataStart + rEntry.nCompressedSize);
            return true;
        }
        return inflateRaw(aBytes.data() + rEntry.nDataStart, rEntry.nCompressedSize, rOut,
                          rEntry.nUncompressedSize);
    }
};

bool ZipArchive::parse()
{
    const size_t n = aBytes.size();
    if (n < 22)
        return false;

    // Find the end-of-central-directory record (may be followed by a comment).
    const size_t nScanEnd = n - 22;
    const size_t nScanStart = nScanEnd > 65535 ? nScanEnd - 65535 : 0;
    size_t nEocd = SIZE_MAX;
    for (size_t i = nScanEnd + 1; i-- > nScanStart;)
    {
        if (aBytes[i] == 0x50 && aBytes[i + 1] == 0x4b && aBytes[i + 2] == 0x05
            && aBytes[i + 3] == 0x06)
        {
            nEocd = i;
            break;
        }
    }
    if (nEocd == SIZE_MAX)
        return false;

    const sal_uInt16 nDisk = readU16(aBytes, nEocd + 4);
    const sal_uInt16 nCdDisk = readU16(aBytes, nEocd + 6);
    const sal_uInt16 nEntriesHere = readU16(aBytes, nEocd + 8);
    const sal_uInt16 nEntriesTotal = readU16(aBytes, nEocd + 10);
    const sal_uInt32 nCdSize = readU32(aBytes, nEocd + 12);
    const sal_uInt32 nCdOffset = readU32(aBytes, nEocd + 16);
    if (nDisk != 0 || nCdDisk != 0 || nEntriesHere != nEntriesTotal)
        return false;
    if (nEntriesTotal == 0xffff || nCdOffset == 0xffffffff || nCdSize == 0xffffffff)
        return false; // zip64: not supported
    if (static_cast<size_t>(nCdOffset) + nCdSize > n)
        return false;

    size_t nPos = nCdOffset;
    for (sal_uInt16 i = 0; i < nEntriesTotal; ++i)
    {
        if (nPos + 46 > n || readU32(aBytes, nPos) != 0x02014b50)
            return false;
        ZipEntryInfo aEntry;
        aEntry.nFlags = readU16(aBytes, nPos + 8);
        aEntry.nMethod = readU16(aBytes, nPos + 10);
        aEntry.nCrc = readU32(aBytes, nPos + 16);
        aEntry.nCompressedSize = readU32(aBytes, nPos + 20);
        aEntry.nUncompressedSize = readU32(aBytes, nPos + 24);
        const sal_uInt16 nNameLen = readU16(aBytes, nPos + 28);
        const sal_uInt16 nExtraLen = readU16(aBytes, nPos + 30);
        const sal_uInt16 nCommentLen = readU16(aBytes, nPos + 32);
        const sal_uInt32 nLocalOffset = readU32(aBytes, nPos + 42);
        aEntry.nCentralStart = nPos;
        aEntry.nCentralLen = 46u + nNameLen + nExtraLen + nCommentLen;
        if (nPos + aEntry.nCentralLen > n)
            return false;
        aEntry.aName = OString(reinterpret_cast<const char*>(aBytes.data() + nPos + 46), nNameLen);

        if (aEntry.nFlags & 0x1)
            return false; // encrypted: not supported
        if (aEntry.nMethod != 0 && aEntry.nMethod != 8)
            return false;
        if (aEntry.nCompressedSize == 0xffffffff || aEntry.nUncompressedSize == 0xffffffff
            || nLocalOffset == 0xffffffff)
            return false; // zip64

        // Locate the payload via the local header (its name/extra lengths can
        // legitimately differ from the central record's).
        const size_t nLocal = nLocalOffset;
        if (nLocal + 30 > n || readU32(aBytes, nLocal) != 0x04034b50)
            return false;
        const sal_uInt16 nNameLenL = readU16(aBytes, nLocal + 26);
        const sal_uInt16 nExtraLenL = readU16(aBytes, nLocal + 28);
        aEntry.nLocalStart = nLocal;
        aEntry.nDataStart = nLocal + 30u + nNameLenL + nExtraLenL;
        size_t nEnd = aEntry.nDataStart + aEntry.nCompressedSize;
        if (nEnd > n)
            return false;
        if (aEntry.nFlags & 0x8)
        {
            // data descriptor, with or without its optional signature
            if (nEnd + 16 <= n && readU32(aBytes, nEnd) == 0x08074b50)
                nEnd += 16;
            else if (nEnd + 12 <= n)
                nEnd += 12;
            else
                return false;
        }
        aEntry.nLocalLen = nEnd - nLocal;

        aEntries.push_back(aEntry);
        nPos += aEntry.nCentralLen;
    }
    return true;
}

void patchU32(std::vector<sal_uInt8>& rB, size_t nPos, sal_uInt32 nVal)
{
    rB[nPos] = nVal & 0xff;
    rB[nPos + 1] = (nVal >> 8) & 0xff;
    rB[nPos + 2] = (nVal >> 16) & 0xff;
    rB[nPos + 3] = (nVal >> 24) & 0xff;
}

void patchU16(std::vector<sal_uInt8>& rB, size_t nPos, sal_uInt16 nVal)
{
    rB[nPos] = nVal & 0xff;
    rB[nPos + 1] = (nVal >> 8) & 0xff;
}

struct Replacement
{
    std::vector<sal_uInt8> aData; // new uncompressed content
    std::vector<sal_uInt8> aCompressed;
    sal_uInt32 nCrc = 0;
};

bool buildOutput(const ZipArchive& rIn, std::map<OString, Replacement>& rReplacements,
                 std::vector<sal_uInt8>& rOut)
{
    for (auto& rPair : rReplacements)
    {
        rPair.second.nCrc = crc32(0, rPair.second.aData.data(), rPair.second.aData.size());
        if (!deflateRaw(rPair.second.aData.data(), rPair.second.aData.size(),
                        rPair.second.aCompressed))
            return false;
    }

    std::vector<const ZipEntryInfo*> aPhysical;
    aPhysical.reserve(rIn.aEntries.size());
    for (const auto& rEntry : rIn.aEntries)
        aPhysical.push_back(&rEntry);
    std::sort(aPhysical.begin(), aPhysical.end(), [](const ZipEntryInfo* a, const ZipEntryInfo* b) {
        return a->nLocalStart < b->nLocalStart;
    });

    std::map<const ZipEntryInfo*, sal_uInt32> aNewOffsets;
    for (const ZipEntryInfo* pEntry : aPhysical)
    {
        aNewOffsets[pEntry] = static_cast<sal_uInt32>(rOut.size());
        auto it = rReplacements.find(pEntry->aName);
        if (it == rReplacements.end())
        {
            rOut.insert(rOut.end(), rIn.aBytes.begin() + pEntry->nLocalStart,
                        rIn.aBytes.begin() + pEntry->nLocalStart + pEntry->nLocalLen);
            continue;
        }
        const Replacement& rRepl = it->second;
        // fresh local header; keep the original DOS timestamp from the
        // central record (time at +12, date at +14)
        pushU32(rOut, 0x04034b50);
        pushU16(rOut, 20); // version needed
        pushU16(rOut, 0); // flags
        pushU16(rOut, 8); // deflate
        pushU16(rOut, readU16(rIn.aBytes, pEntry->nCentralStart + 12));
        pushU16(rOut, readU16(rIn.aBytes, pEntry->nCentralStart + 14));
        pushU32(rOut, rRepl.nCrc);
        pushU32(rOut, static_cast<sal_uInt32>(rRepl.aCompressed.size()));
        pushU32(rOut, static_cast<sal_uInt32>(rRepl.aData.size()));
        pushU16(rOut, static_cast<sal_uInt16>(pEntry->aName.getLength()));
        pushU16(rOut, 0); // no extra field
        rOut.insert(rOut.end(), pEntry->aName.getStr(),
                    pEntry->aName.getStr() + pEntry->aName.getLength());
        rOut.insert(rOut.end(), rRepl.aCompressed.begin(), rRepl.aCompressed.end());
    }

    const sal_uInt32 nCdStart = static_cast<sal_uInt32>(rOut.size());
    for (const auto& rEntry : rIn.aEntries) // keep original central-directory order
    {
        const size_t nRecordStart = rOut.size();
        rOut.insert(rOut.end(), rIn.aBytes.begin() + rEntry.nCentralStart,
                    rIn.aBytes.begin() + rEntry.nCentralStart + rEntry.nCentralLen);
        patchU32(rOut, nRecordStart + 42, aNewOffsets[&rEntry]);
        auto it = rReplacements.find(rEntry.aName);
        if (it != rReplacements.end())
        {
            const Replacement& rRepl = it->second;
            patchU16(rOut, nRecordStart + 8, rEntry.nFlags & ~sal_uInt16(0x8));
            patchU16(rOut, nRecordStart + 10, 8);
            patchU32(rOut, nRecordStart + 16, rRepl.nCrc);
            patchU32(rOut, nRecordStart + 20, static_cast<sal_uInt32>(rRepl.aCompressed.size()));
            patchU32(rOut, nRecordStart + 24, static_cast<sal_uInt32>(rRepl.aData.size()));
        }
    }
    const sal_uInt32 nCdSize = static_cast<sal_uInt32>(rOut.size()) - nCdStart;

    pushU32(rOut, 0x06054b50);
    pushU16(rOut, 0);
    pushU16(rOut, 0);
    pushU16(rOut, static_cast<sal_uInt16>(rIn.aEntries.size()));
    pushU16(rOut, static_cast<sal_uInt16>(rIn.aEntries.size()));
    pushU32(rOut, nCdSize);
    pushU32(rOut, nCdStart);
    pushU16(rOut, 0); // no comment
    return true;
}

bool isElement(const xmlNode* pNode, const char* pNsHref, const char* pLocalName)
{
    return pNode->type == XML_ELEMENT_NODE && pNode->ns && pNode->ns->href
           && std::strcmp(reinterpret_cast<const char*>(pNode->ns->href), pNsHref) == 0
           && std::strcmp(reinterpret_cast<const char*>(pNode->name), pLocalName) == 0;
}

OString getNsAttr(xmlNodePtr pNode, const char* pNsHref, const char* pLocalName)
{
    xmlChar* pVal = xmlGetNsProp(pNode, BAD_CAST pLocalName, BAD_CAST pNsHref);
    if (!pVal)
        return OString();
    OString aResult(reinterpret_cast<const char*>(pVal));
    xmlFree(pVal);
    return aResult;
}

// Run collection and key derivation: the contract shared with the Python
// extractor (l10n_odf.py in online).

struct RunNode
{
    xmlNodePtr pNode; // the text node (bare run) or span element
    bool bIsSpan;
    OString aText;
};

UnitState collectRuns(xmlNodePtr pPara, std::vector<RunNode>& rRuns)
{
    bool bForbidden = false;
    for (xmlNodePtr pChild = pPara->children; pChild; pChild = pChild->next)
    {
        switch (pChild->type)
        {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
                if (pChild->content && *pChild->content)
                {
                    // bare text separated only by an invisible node (comment,
                    // PI, empty span) is one run, so both sides derive the
                    // same run list however their XML parser splits text
                    // nodes (ElementTree folds CDATA into text, libxml2
                    // keeps it separate)
                    const std::string_view aContent(
                        reinterpret_cast<const char*>(pChild->content));
                    if (!rRuns.empty() && !rRuns.back().bIsSpan)
                        rRuns.back().aText += OString::Concat(aContent);
                    else
                        rRuns.push_back({ pChild, false, OString(aContent) });
                }
                break;
            case XML_COMMENT_NODE:
            case XML_PI_NODE:
                break; // invisible: must not split the surrounding bare text
            case XML_ELEMENT_NODE:
                if (isElement(pChild, TEXT_NS, "span"))
                {
                    OStringBuffer aText;
                    for (xmlNodePtr pGrand = pChild->children; pGrand; pGrand = pGrand->next)
                    {
                        if (pGrand->type == XML_TEXT_NODE || pGrand->type == XML_CDATA_SECTION_NODE)
                            aText.append(reinterpret_cast<const char*>(pGrand->content));
                        else if (pGrand->type != XML_COMMENT_NODE && pGrand->type != XML_PI_NODE)
                            bForbidden = true; // nested span, field, line-break, ...
                    }
                    if (!aText.isEmpty())
                        rRuns.push_back({ pChild, true, aText.makeStringAndClear() });
                }
                else
                    bForbidden = true; // text:s, text:tab, text:a, fields, ...
                break;
            default:
                bForbidden = true;
                break;
        }
    }

    if (rRuns.empty() || !rRuns[0].aText.startsWith("_"))
    {
        rRuns.clear();
        return UnitState::NotMarked;
    }
    if (bForbidden)
    {
        rRuns.clear();
        return UnitState::Forbidden;
    }

    rRuns[0].aText = rRuns[0].aText.copy(1);
    if (rRuns[0].aText.isEmpty())
        rRuns.erase(rRuns.begin());
    return UnitState::Marked;
}

OString buildKey(const std::vector<RunNode>& rRuns)
{
    if (rRuns.size() == 1)
        return rRuns[0].aText;
    OStringBuffer aKey;
    sal_Int32 nSpan = 0;
    for (const RunNode& rRun : rRuns)
    {
        if (rRun.bIsSpan)
        {
            ++nSpan;
            aKey.append("<" + OString::number(nSpan) + ">" + rRun.aText + "</"
                        + OString::number(nSpan) + ">");
        }
        else
            aKey.append(rRun.aText);
    }
    return aKey.makeStringAndClear();
}

/// Miss fallback: strip one leading "_" from the paragraph's first non-empty
/// text segment (bare text node or span text), leaving everything else as-is.
void stripMarkerInPlace(xmlNodePtr pPara)
{
    for (xmlNodePtr pChild = pPara->children; pChild; pChild = pChild->next)
    {
        if ((pChild->type == XML_TEXT_NODE || pChild->type == XML_CDATA_SECTION_NODE)
            && pChild->content && *pChild->content)
        {
            if (pChild->content[0] == '_')
            {
                OString aStripped(reinterpret_cast<const char*>(pChild->content + 1));
                xmlNodePtr pNew = xmlNewDocText(pPara->doc, BAD_CAST aStripped.getStr());
                xmlReplaceNode(pChild, pNew);
                xmlFreeNode(pChild);
            }
            return;
        }
        if (pChild->type == XML_ELEMENT_NODE)
        {
            for (xmlNodePtr pGrand = pChild->children; pGrand; pGrand = pGrand->next)
            {
                if ((pGrand->type == XML_TEXT_NODE || pGrand->type == XML_CDATA_SECTION_NODE)
                    && pGrand->content && *pGrand->content)
                {
                    if (pGrand->content[0] == '_')
                    {
                        OString aStripped(reinterpret_cast<const char*>(pGrand->content + 1));
                        xmlNodePtr pNew = xmlNewDocText(pPara->doc, BAD_CAST aStripped.getStr());
                        xmlReplaceNode(pGrand, pNew);
                        xmlFreeNode(pGrand);
                    }
                    return;
                }
            }
        }
    }
}

void rebuildParagraph(xmlNodePtr pPara, const std::vector<RunNode>& rRuns,
                      const std::vector<MsgstrToken>& rTokens)
{
    xmlDocPtr pDoc = pPara->doc;

    // <N> addresses the N-th span; untagged text is bare paragraph text,
    // except in a single-run paragraph, where it takes that run's formatting
    const bool bSingleRun = rRuns.size() == 1;
    std::vector<const RunNode*> aSpanRuns;
    for (const RunNode& rRun : rRuns)
        if (rRun.bIsSpan)
            aSpanRuns.push_back(&rRun);

    // build the new children first: the runs still reference the old ones
    std::vector<xmlNodePtr> aNewChildren;
    for (const MsgstrToken& rToken : rTokens)
    {
        if (rToken.aText.isEmpty())
            continue;
        const RunNode* pRun = nullptr;
        if (bSingleRun)
            pRun = rRuns.data();
        else if (rToken.nRun != 0)
            pRun = aSpanRuns[rToken.nRun - 1];
        if (pRun && pRun->bIsSpan)
        {
            const RunNode& rRun = *pRun;
            xmlNodePtr pSpan;
            if (rRun.pNode->nsDef != nullptr)
            {
                // rare: the span declares a namespace itself; a plain copy
                // (which duplicates the declarations) is the safe route
                pSpan = xmlDocCopyNode(rRun.pNode, pDoc, 2);
            }
            else
            {
                // reuse the ancestors' in-scope namespace declarations so the
                // rebuilt span serializes without redundant xmlns attributes
                pSpan = xmlNewDocNode(pDoc, rRun.pNode->ns, rRun.pNode->name, nullptr);
                for (xmlAttrPtr pAttr = rRun.pNode->properties; pAttr; pAttr = pAttr->next)
                {
                    xmlChar* pVal = xmlNodeListGetString(pDoc, pAttr->children, 1);
                    if (pAttr->ns)
                        xmlNewNsProp(pSpan, pAttr->ns, pAttr->name, pVal);
                    else
                        xmlNewProp(pSpan, pAttr->name, pVal);
                    xmlFree(pVal);
                }
            }
            xmlAddChild(pSpan, xmlNewDocText(pDoc, BAD_CAST rToken.aText.getStr()));
            aNewChildren.push_back(pSpan);
        }
        else
            aNewChildren.push_back(xmlNewDocText(pDoc, BAD_CAST rToken.aText.getStr()));
    }

    while (pPara->children)
    {
        xmlNodePtr pChild = pPara->children;
        xmlUnlinkNode(pChild);
        xmlFreeNode(pChild);
    }
    for (xmlNodePtr pChild : aNewChildren)
        xmlAddChild(pPara, pChild); // adjacent text nodes merge automatically
}

/// Translate a plain "_"-marked value (sheet name, help-message title).
OString translatePlain(std::string_view rMarkedValue, const L10nStream& rStream)
{
    const OString aKey(rMarkedValue.substr(1));
    auto it = rStream.aMap.find(aKey);
    // an empty translation leaves the English source in place, which reads
    // better than an empty sheet tab or help title
    if (it == rStream.aMap.end() || it->second.isEmpty())
        return aKey;
    return it->second;
}

void handleParagraph(xmlNodePtr pPara, const L10nStream& rStream)
{
    std::vector<RunNode> aRuns;
    const UnitState eState = collectRuns(pPara, aRuns);
    if (eState == UnitState::NotMarked)
        return;
    if (eState == UnitState::Forbidden || aRuns.empty())
    {
        SAL_INFO_IF(eState == UnitState::Forbidden, "desktop.lib",
                    "l10n: skipping paragraph with unsupported content, stripping marker only");
        stripMarkerInPlace(pPara);
        return;
    }

    const OString aKey = buildKey(aRuns);
    auto it = rStream.aMap.find(aKey);
    if (it == rStream.aMap.end())
    {
        stripMarkerInPlace(pPara);
        return;
    }
    if (it->second.isEmpty())
    {
        // rebuilding from an empty msgstr would empty the paragraph, so the
        // English source stays and only the marker comes off
        SAL_WARN("desktop.lib", "l10n: empty translation for key '" << aKey << "'");
        stripMarkerInPlace(pPara);
        return;
    }

    // a msgstr may use only the tags its msgid shows: one per span, none
    // for a one-run msgid, whose key is tagless (mirrors the build-side
    // msgid_tag_count())
    sal_Int32 nTagCount = 0;
    if (aRuns.size() > 1)
        for (const RunNode& rRun : aRuns)
            if (rRun.bIsSpan)
                ++nTagCount;
    std::vector<MsgstrToken> aTokens;
    if (!parseMsgstrTokens(it->second, nTagCount, aTokens))
    {
        SAL_WARN("desktop.lib", "l10n: invalid msgstr for key '" << aKey << "', stripping marker");
        stripMarkerInPlace(pPara);
        return;
    }
    // a msgstr that is only tag scaffolding, "<1></1>", carries no text
    // either, and rebuilding from it would empty the paragraph just as an
    // empty msgstr would
    const bool bNoText = std::all_of(aTokens.begin(), aTokens.end(),
                                     [](const MsgstrToken& rToken)
                                     { return rToken.aText.isEmpty(); });
    if (bNoText)
    {
        SAL_WARN("desktop.lib", "l10n: translation for key '" << aKey << "' has no text");
        stripMarkerInPlace(pPara);
        return;
    }
    rebuildParagraph(pPara, aRuns, aTokens);
}

void walkParagraphs(xmlNodePtr pNode, const L10nStream& rStream)
{
    for (xmlNodePtr pChild = pNode->children; pChild; pChild = pChild->next)
    {
        if (pChild->type != XML_ELEMENT_NODE)
            continue;
        if (isElement(pChild, TEXT_NS, "p") || isElement(pChild, TEXT_NS, "h"))
            handleParagraph(pChild, rStream);
        else
            walkParagraphs(pChild, rStream);
    }
}

void translateHelpMessageTitles(xmlNodePtr pNode, const L10nStream& rStream)
{
    for (xmlNodePtr pChild = pNode->children; pChild; pChild = pChild->next)
    {
        if (pChild->type != XML_ELEMENT_NODE)
            continue;
        if (isElement(pChild, TABLE_NS, "help-message"))
        {
            const OString aTitle = getNsAttr(pChild, TABLE_NS, "title");
            if (aTitle.startsWith("_"))
            {
                const OString aNew = translatePlain(aTitle, rStream);
                xmlNsPtr pNs = xmlSearchNsByHref(pChild->doc, pChild, BAD_CAST TABLE_NS);
                xmlSetNsProp(pChild, pNs, BAD_CAST "title", BAD_CAST aNew.getStr());
            }
        }
        translateHelpMessageTitles(pChild, rStream);
    }
}

OString escapeSheetName(const OString& rName) { return rName.replaceAll("'"_ostr, "''"_ostr); }

bool isSimpleSheetName(const OString& rName)
{
    if (rName.isEmpty())
        return false;
    for (sal_Int32 i = 0; i < rName.getLength(); ++i)
    {
        const char c = rName[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
              || c == '_'))
            return false;
    }
    return true;
}

struct SheetRename
{
    OString aOld; // marked name, including the "_"
    OString aNew;
};

/// True when a name is one a sheet can carry: not empty, none of the
/// characters a sheet reference cannot hold, and not wrapped in the quote
/// character a reference uses.
bool isUsableSheetName(const OString& rName)
{
    if (rName.isEmpty())
        return false;
    for (sal_Int32 nPos = 0; nPos < rName.getLength(); ++nPos)
    {
        switch (rName[nPos])
        {
            case '[':
            case ']':
            case '*':
            case '?':
            case ':':
            case '/':
            case '\\':
                return false;
            default:
                break;
        }
    }
    return !rName.startsWith("'") && !rName.endsWith("'");
}

void collectAndApplySheetNames(xmlNodePtr pNode, const L10nStream& rStream,
                               std::vector<SheetRename>& rRenames)
{
    for (xmlNodePtr pChild = pNode->children; pChild; pChild = pChild->next)
    {
        if (pChild->type != XML_ELEMENT_NODE)
            continue;
        if (isElement(pChild, TABLE_NS, "table"))
        {
            const OString aName = getNsAttr(pChild, TABLE_NS, "name");
            if (aName.startsWith("_"))
            {
                OString aNew = translatePlain(aName, rStream);
                // Translations come from .po files we do not control, so a
                // name a sheet cannot carry, or one a sheet already has,
                // falls back to the English source.
                const bool bTaken
                    = std::any_of(rRenames.begin(), rRenames.end(),
                                  [&aNew](const SheetRename& r) { return r.aNew == aNew; });
                if (!isUsableSheetName(aNew) || bTaken)
                {
                    SAL_WARN("desktop.lib", "l10n: cannot use '" << aNew
                                                                 << "' as a sheet name, "
                                                                    "keeping the source");
                    aNew = aName.copy(1);
                }
                xmlNsPtr pNs = xmlSearchNsByHref(pChild->doc, pChild, BAD_CAST TABLE_NS);
                xmlSetNsProp(pChild, pNs, BAD_CAST "name", BAD_CAST aNew.getStr());
                rRenames.push_back({ aName, aNew });
            }
        }
        collectAndApplySheetNames(pChild, rStream, rRenames);
    }
}

/// Rewrite one referencing-attribute value.  Quoted occurrences ('old', with
/// '' escaping) are replaced first; any remaining bare occurrence is replaced
/// with the new name, quoted unless it needs no quoting.  Renames are applied
/// longest-old-name first so no name that is a prefix of another can corrupt
/// the longer one.  Mirrored verbatim in the Python extractor's lint.
OString rewriteSheetRefs(const OString& rValue, const std::vector<SheetRename>& rRenames)
{
    OString aValue = rValue;
    for (const SheetRename& rRename : rRenames)
    {
        const OString aQuotedOld = "'" + escapeSheetName(rRename.aOld) + "'";
        const OString aQuotedNew = "'" + escapeSheetName(rRename.aNew) + "'";
        aValue = aValue.replaceAll(aQuotedOld, aQuotedNew);
        if (aValue.indexOf(rRename.aOld) != -1)
        {
            SAL_WARN("desktop.lib",
                     "l10n: unquoted sheet-name reference for '" << rRename.aOld << "'");
            aValue = aValue.replaceAll(rRename.aOld,
                                       isSimpleSheetName(rRename.aNew) ? rRename.aNew : aQuotedNew);
        }
    }
    return aValue;
}

void rewriteSheetRefAttrs(xmlNodePtr pNode, const std::vector<SheetRename>& rRenames)
{
    for (xmlNodePtr pChild = pNode->children; pChild; pChild = pChild->next)
    {
        if (pChild->type != XML_ELEMENT_NODE)
            continue;
        for (const SheetRefAttr& rAttr : aSheetRefAttrs)
        {
            const OString aValue = getNsAttr(pChild, rAttr.pNsHref, rAttr.pLocalName);
            if (aValue.isEmpty())
                continue;
            const OString aNew = rewriteSheetRefs(aValue, rRenames);
            if (aNew != aValue)
            {
                xmlNsPtr pNs = xmlSearchNsByHref(pChild->doc, pChild, BAD_CAST rAttr.pNsHref);
                xmlSetNsProp(pChild, pNs, BAD_CAST rAttr.pLocalName, BAD_CAST aNew.getStr());
            }
        }
        rewriteSheetRefAttrs(pChild, rRenames);
    }
}

bool rewriteXmlMember(const std::vector<sal_uInt8>& rIn, const L10nStream& rStream,
                      std::vector<sal_uInt8>& rOut, std::vector<SheetRename>& rRenames)
{
    xmlDocPtr pDoc
        = xmlReadMemory(reinterpret_cast<const char*>(rIn.data()), static_cast<int>(rIn.size()),
                        "member.xml", nullptr, XML_PARSE_NONET);
    if (!pDoc)
        return false;
    xmlNodePtr pRoot = xmlDocGetRootElement(pDoc);
    if (!pRoot)
    {
        xmlFreeDoc(pDoc);
        return false;
    }

    // rRenames carries in what an earlier member declared, so a reference
    // in styles.xml to a sheet named in content.xml is rewritten too
    collectAndApplySheetNames(pRoot, rStream, rRenames);
    // longest old name first so a name that is a prefix of another cannot
    // corrupt the longer one
    std::sort(rRenames.begin(), rRenames.end(), [](const SheetRename& a, const SheetRename& b)
              { return a.aOld.getLength() > b.aOld.getLength(); });
    if (!rRenames.empty())
        rewriteSheetRefAttrs(pRoot, rRenames);
    translateHelpMessageTitles(pRoot, rStream);
    walkParagraphs(pRoot, rStream);

    xmlChar* pOut = nullptr;
    int nOut = 0;
    xmlDocDumpMemory(pDoc, &pOut, &nOut);
    xmlFreeDoc(pDoc);
    if (!pOut)
        return false;
    rOut.assign(pOut, pOut + nOut);
    xmlFree(pOut);
    return true;
}

} // namespace

UnitState deriveKeyFromParagraphXml(std::string_view rParagraphXml, OString& rKey)
{
    rKey = OString();
    const OString aWrapped = OString::Concat("<root xmlns:text=\"") + std::string_view(TEXT_NS)
                             + "\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">" + rParagraphXml
                             + "</root>";
    xmlDocPtr pDoc = xmlReadMemory(aWrapped.getStr(), aWrapped.getLength(), "para.xml", nullptr,
                                   XML_PARSE_NONET | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!pDoc)
        return UnitState::NotMarked;
    xmlNodePtr pRoot = xmlDocGetRootElement(pDoc);
    xmlNodePtr pPara = pRoot ? xmlFirstElementChild(pRoot) : nullptr;
    if (!pPara)
    {
        xmlFreeDoc(pDoc);
        return UnitState::NotMarked;
    }
    std::vector<RunNode> aRuns;
    const UnitState eState = collectRuns(pPara, aRuns);
    if (eState == UnitState::Marked && !aRuns.empty())
        rKey = buildKey(aRuns);
    xmlFreeDoc(pDoc);
    return eState;
}

bool parseMsgstrTokens(const OString& rMsgstr, sal_Int32 nTagCount,
                       std::vector<MsgstrToken>& rTokens)
{
    rTokens.clear();
    const sal_Int32 nLen = rMsgstr.getLength();

    // parse "<digits>" or "</digits>" at nPos; returns the number and whether
    // it is a closing tag, advancing nEnd past the ">"; -1 if not a tag
    auto parseTag = [&rMsgstr, nLen](sal_Int32 nPos, bool& rClosing, sal_Int32& rEnd) -> sal_Int32 {
        sal_Int32 i = nPos + 1;
        rClosing = i < nLen && rMsgstr[i] == '/';
        if (rClosing)
            ++i;
        const sal_Int32 nDigitsStart = i;
        sal_Int64 nNum = 0;
        while (i < nLen && rMsgstr[i] >= '0' && rMsgstr[i] <= '9' && i - nDigitsStart < 9)
            nNum = nNum * 10 + (rMsgstr[i++] - '0');
        if (i == nDigitsStart || i >= nLen || rMsgstr[i] != '>')
            return -1;
        rEnd = i + 1;
        return static_cast<sal_Int32>(nNum);
    };

    sal_Int32 nPos = 0;
    OStringBuffer aLiteral;
    while (nPos < nLen)
    {
        if (rMsgstr[nPos] != '<')
        {
            aLiteral.append(rMsgstr[nPos++]);
            continue;
        }
        bool bClosing = false;
        sal_Int32 nAfterTag = 0;
        const sal_Int32 nNum = parseTag(nPos, bClosing, nAfterTag);
        if (nNum < 0)
        {
            aLiteral.append(rMsgstr[nPos++]); // a lone "<" is literal text
            continue;
        }
        if (bClosing || nNum < 1 || nNum > nTagCount)
        {
            rTokens.clear();
            return false; // stray close, or span index out of range
        }
        if (!aLiteral.isEmpty())
            rTokens.push_back({ 0, aLiteral.makeStringAndClear() });

        // scan the group content up to the matching close; nested tags invalid
        sal_Int32 nScan = nAfterTag;
        OStringBuffer aContent;
        bool bClosed = false;
        while (nScan < nLen)
        {
            if (rMsgstr[nScan] != '<')
            {
                aContent.append(rMsgstr[nScan++]);
                continue;
            }
            bool bInnerClosing = false;
            sal_Int32 nInnerEnd = 0;
            const sal_Int32 nInner = parseTag(nScan, bInnerClosing, nInnerEnd);
            if (nInner < 0)
            {
                aContent.append(rMsgstr[nScan++]);
                continue;
            }
            if (!bInnerClosing || nInner != nNum)
            {
                rTokens.clear();
                return false; // nesting, or mismatched close
            }
            bClosed = true;
            nScan = nInnerEnd;
            break;
        }
        if (!bClosed)
        {
            rTokens.clear();
            return false; // unclosed tag
        }
        rTokens.push_back({ nNum, aContent.makeStringAndClear() });
        nPos = nScan;
    }
    if (!aLiteral.isEmpty())
        rTokens.push_back({ 0, aLiteral.makeStringAndClear() });
    return true;
}

bool isXmlSafeUtf8(const OString& rText)
{
    OUString aTmp;
    if (!rtl_convertStringToUString(
            &aTmp.pData, rText.getStr(), rText.getLength(), RTL_TEXTENCODING_UTF8,
            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR))
        return false; // not valid UTF-8; lone surrogates included

    for (sal_Int32 i = 0; i < aTmp.getLength(); ++i)
    {
        const sal_Unicode c = aTmp[i]; // the XML 1.0 Char production
        if (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D)
            return false; // NUL, vertical tab, form feed, the other C0 controls
        if (c == 0xFFFE || c == 0xFFFF)
            return false;
    }
    return true;
}

void parseL10nStream(const OString& rStreamText, const OUString& rBcp47Language,
                     L10nStream& rStream)
{
    rStream.aMatchedLocale = OString();
    rStream.aMap.clear();

    // LanguageTag reads an empty tag as the system locale.  A caller that
    // names no language wants no translation, so the fallback list stays
    // empty and no locale in the stream can match it.
    std::vector<OString> aFallbacks;
    if (!rBcp47Language.isEmpty())
    {
        for (const OUString& rFallback : LanguageTag(rBcp47Language).getFallbackStrings(true))
            aFallbacks.push_back(OUStringToOString(rFallback, RTL_TEXTENCODING_UTF8));
    }

    OString aText = rStreamText;
    if (aText.startsWith("\xef\xbb\xbf")) // tolerate a UTF-8 BOM
        aText = aText.copy(3);

    bool bSeenLocaleList = false;
    OString aKey;
    bool bHaveKey = false;
    sal_Int32 nPos = 0;
    const sal_Int32 nLen = aText.getLength();
    while (nPos <= nLen)
    {
        sal_Int32 nEol = aText.indexOf('\n', nPos);
        if (nEol == -1)
            nEol = nLen;
        OString aLine = aText.copy(nPos, nEol - nPos);
        nPos = nEol + 1;
        if (aLine.endsWith("\r"))
            aLine = aLine.copy(0, aLine.getLength() - 1);

        if (aLine.isEmpty() || aLine[0] == '#')
        {
            bHaveKey = false;
            if (nEol == nLen)
                break;
            continue;
        }
        if (!bSeenLocaleList)
        {
            bSeenLocaleList = true;
            std::vector<OString> aStreamLocales;
            sal_Int32 nIdx = 0;
            while (nIdx >= 0)
                aStreamLocales.push_back(aLine.getToken(0, ',', nIdx));
            // getFallbackStrings() runs from the most specific tag to the
            // least, so walking it outermost picks the "pt-BR" translation
            // for a "pt-BR" user however the stream orders its locales.
            for (const OString& rFallback : aFallbacks)
            {
                if (std::find(aStreamLocales.begin(), aStreamLocales.end(), rFallback)
                    != aStreamLocales.end())
                {
                    rStream.aMatchedLocale = rFallback;
                    break;
                }
            }
        }
        else if (!bHaveKey)
        {
            aKey = aLine;
            bHaveKey = true;
        }
        else
        {
            const sal_Int32 nTab = aLine.indexOf('\t');
            if (nTab == -1)
            {
                SAL_WARN("desktop.lib", "l10n stream: malformed value line: " << aLine);
            }
            else if (!rStream.aMatchedLocale.isEmpty()
                     && aLine.subView(0, nTab) == rStream.aMatchedLocale)
            {
                // A value we cannot serialise as XML is dropped rather than
                // scrubbed: the key then simply misses, the marker is
                // stripped, and the document still opens.  Scrubbing would
                // ship a silently altered translation instead.
                const OString aValue = aLine.copy(nTab + 1);
                if (isXmlSafeUtf8(aValue))
                    rStream.aMap[aKey] = aValue;
                else
                    SAL_WARN("desktop.lib", "l10n stream: dropping the "
                                                << rStream.aMatchedLocale << " translation of \""
                                                << aKey
                                                << "\": not valid UTF-8, or it holds a character "
                                                   "illegal in XML");
            }
        }
        if (nEol == nLen)
            break;
    }
}

/// Prefix of the stream comment recording the locale the last rewrite used.
constexpr std::string_view APPLIED_PREFIX = "# applied: ";
/// Recorded in place of a locale when a rewrite only stripped the markers.
constexpr std::string_view APPLIED_NONE = "-";

/// The locale the last rewrite of this file used, or empty when no rewrite
/// has recorded one.
static OString readAppliedLocale(std::string_view rStreamText)
{
    sal_Int32 nPos = 0;
    while (nPos >= 0)
    {
        std::string_view aLine = o3tl::getToken(rStreamText, 0, '\n', nPos);
        if (o3tl::ends_with(aLine, "\r"))
            aLine.remove_suffix(1);
        if (o3tl::starts_with(aLine, APPLIED_PREFIX))
            return OString(o3tl::trim(aLine.substr(APPLIED_PREFIX.size())));
    }
    return OString();
}

/// The stream text with its applied-locale record set to rLocale.
static OString withAppliedLocale(std::string_view rStreamText, std::string_view rLocale)
{
    OStringBuffer aOut(static_cast<sal_Int32>(rStreamText.size()) + 32);
    sal_Int32 nPos = rStreamText.empty() ? -1 : 0;
    while (nPos >= 0)
    {
        const sal_Int32 nLineStart = nPos;
        const std::string_view aLine = o3tl::getToken(rStreamText, 0, '\n', nPos);
        // splitting text that ends in a newline yields one empty token past
        // it, which would add a blank line on every rewrite
        if (nPos < 0 && aLine.empty() && nLineStart > 0)
            break;
        if (!o3tl::starts_with(aLine, APPLIED_PREFIX))
            aOut.append(OString::Concat(aLine) + "\n");
    }
    aOut.append(OString::Concat(APPLIED_PREFIX) + rLocale + "\n");
    return aOut.makeStringAndClear();
}

/// True when content.xml is still exactly what the recorded locale produced,
/// so rewriting it from the template cannot lose anyone's work.
bool isContentStillOurs(const ZipArchive& rArchive, const OString& rStreamText,
                        const OString& rApplied)
{
    const ZipEntryInfo* pContent = rArchive.find("content.xml");
    const ZipEntryInfo* pTemplate = rArchive.find("l10n_template.xml");
    if (!pContent || !pTemplate)
        return true; // nothing to compare against; the rewrite is a no-op

    OUString aLanguage;
    if (rApplied != APPLIED_NONE)
        aLanguage = OUString(rApplied.getStr(), rApplied.getLength(), RTL_TEXTENCODING_UTF8);
    L10nStream aApplied;
    parseL10nStream(rStreamText, aLanguage, aApplied);

    std::vector<sal_uInt8> aCurrent, aTemplateIn, aExpected;
    std::vector<SheetRename> aRenames;
    if (!rArchive.readEntry(*pContent, aCurrent) || !rArchive.readEntry(*pTemplate, aTemplateIn)
        || !rewriteXmlMember(aTemplateIn, aApplied, aExpected, aRenames))
        return true;
    return aExpected == aCurrent;
}

TranslateResult translateDocument(const OUString& rInputPathOrUrl, const OUString& rOutputPathOrUrl,
                                  const OUString& rBcp47Language)
{
    ZipArchive aArchive;
    if (!readFile(rInputPathOrUrl, aArchive.aBytes))
    {
        SAL_WARN("desktop.lib", "l10n: cannot read " << rInputPathOrUrl);
        return TranslateResult::Error;
    }
    if (!aArchive.parse())
    {
        SAL_WARN("desktop.lib", "l10n: unsupported or broken zip: " << rInputPathOrUrl);
        return TranslateResult::Error;
    }

    const bool bInPlace = toFileURL(rInputPathOrUrl) == toFileURL(rOutputPathOrUrl);

    const ZipEntryInfo* pL10n = aArchive.find("l10n");
    if (!pL10n)
    {
        // no stream: plain byte-identical copy (raw authoring checkouts keep
        // their markers); rewriting a file in place would change nothing
        if (!bInPlace
            && !writeFile(rOutputPathOrUrl, aArchive.aBytes.data(), aArchive.aBytes.size()))
            return TranslateResult::Error;
        return TranslateResult::CopiedUntranslated;
    }

    // A stream that cannot be read is treated like one that matches no
    // locale: the markers still come off, so the document opens with plain
    // untranslated text instead of a "_" in front of every sentence.
    std::vector<sal_uInt8> aStreamBytes;
    OString aStreamText;
    L10nStream aStream;
    if (aArchive.readEntry(*pL10n, aStreamBytes))
    {
        aStreamText = OString(reinterpret_cast<const char*>(aStreamBytes.data()),
                              static_cast<sal_Int32>(aStreamBytes.size()));
        parseL10nStream(aStreamText, rBcp47Language, aStream);
    }
    else
        SAL_WARN("desktop.lib", "l10n: cannot read l10n stream in " << rInputPathOrUrl
                                                                    << ", stripping markers only");

    // The stream and the templates outlive an ODF save, so a file whose text
    // the user has changed must never be rebuilt from its template.  The
    // stream records the locale the last rewrite used, and the file is ours
    // to rewrite again only while content.xml is still exactly what that
    // locale produced.
    const OString aApplied = readAppliedLocale(aStreamText);
    if (!aApplied.isEmpty() && !isContentStillOurs(aArchive, aStreamText, aApplied))
    {
        SAL_INFO("desktop.lib", "l10n: " << rInputPathOrUrl
                                         << " has been edited since it was translated into "
                                         << aApplied << ", leaving its text alone");
        // out of place the caller still expects its output file, as in the
        // no-stream case above
        if (!bInPlace
            && !writeFile(rOutputPathOrUrl, aArchive.aBytes.data(), aArchive.aBytes.size()))
            return TranslateResult::Error;
        return TranslateResult::CopiedUntranslated;
    }

    // Each member is rewritten from its embedded l10n template when one
    // exists, from the member itself otherwise.  The templates and
    // the stream survive in the output, so the same file can be translated
    // again into another language on a later open - until the first save,
    // which rewrites the package and drops them.
    struct MemberPair
    {
        const char* pTarget;
        const char* pTemplate;
    };
    std::map<OString, Replacement> aReplacements;
    std::vector<SheetRename> aRenames;
    for (const MemberPair& rPair : { MemberPair{ "content.xml", "l10n_template.xml" },
                                     MemberPair{ "styles.xml", "l10n_template_styles.xml" } })
    {
        const ZipEntryInfo* pTarget = aArchive.find(rPair.pTarget);
        if (!pTarget)
        {
            if (std::strcmp(rPair.pTarget, "content.xml") == 0)
            {
                SAL_WARN("desktop.lib", "l10n: no content.xml in " << rInputPathOrUrl);
                return TranslateResult::Error;
            }
            continue;
        }
        const ZipEntryInfo* pSource = aArchive.find(rPair.pTemplate);
        std::vector<sal_uInt8> aIn;
        Replacement aRepl;
        // A template that cannot be read or parsed is skipped in favour of
        // the member itself, which still carries the markers until the
        // first translation.
        if (pSource
            && !(aArchive.readEntry(*pSource, aIn)
                 && rewriteXmlMember(aIn, aStream, aRepl.aData, aRenames)))
        {
            SAL_WARN("desktop.lib", "l10n: cannot rewrite from " << rPair.pTemplate << ", using "
                                                                 << rPair.pTarget);
            pSource = nullptr;
            aIn.clear();
            aRepl.aData.clear();
        }
        if (!pSource
            && !(aArchive.readEntry(*pTarget, aIn)
                 && rewriteXmlMember(aIn, aStream, aRepl.aData, aRenames)))
        {
            SAL_WARN("desktop.lib", "l10n: cannot rewrite " << rPair.pTarget);
            return TranslateResult::Error;
        }
        aReplacements[OString(rPair.pTarget)] = std::move(aRepl);
    }

    const TranslateResult eResult = aStream.aMatchedLocale.isEmpty()
                                        ? TranslateResult::CopiedUntranslated
                                        : TranslateResult::Translated;

    const OString aNowApplied
        = aStream.aMatchedLocale.isEmpty() ? OString(APPLIED_NONE) : aStream.aMatchedLocale;
    if (aNowApplied != aApplied)
    {
        const OString aNewStream = withAppliedLocale(aStreamText, aNowApplied);
        Replacement aRepl;
        aRepl.aData.assign(aNewStream.getStr(), aNewStream.getStr() + aNewStream.getLength());
        aReplacements["l10n"_ostr] = std::move(aRepl);
    }

    std::vector<sal_uInt8> aOut;
    if (!buildOutput(aArchive, aReplacements, aOut))
    {
        SAL_WARN("desktop.lib", "l10n: cannot rebuild " << rInputPathOrUrl);
        return TranslateResult::Error;
    }
    if (bInPlace && aOut == aArchive.aBytes)
        return eResult; // already in this language: leave the file untouched
    if (!writeFile(rOutputPathOrUrl, aOut.data(), aOut.size()))
    {
        SAL_WARN("desktop.lib", "l10n: cannot write " << rOutputPathOrUrl);
        return TranslateResult::Error;
    }

    return eResult;
}

namespace zip
{
bool readArchive(const OUString& rPathOrUrl, std::vector<Entry>& rEntries)
{
    rEntries.clear();
    ZipArchive aArchive;
    if (!readFile(rPathOrUrl, aArchive.aBytes) || !aArchive.parse())
        return false;
    for (const ZipEntryInfo& rInfo : aArchive.aEntries)
    {
        Entry aEntry;
        aEntry.aName = rInfo.aName;
        aEntry.nMethod = rInfo.nMethod;
        if (!aArchive.readEntry(rInfo, aEntry.aData))
            return false;
        aEntry.aRawData.assign(aArchive.aBytes.begin() + rInfo.nDataStart,
                               aArchive.aBytes.begin() + rInfo.nDataStart + rInfo.nCompressedSize);
        rEntries.push_back(std::move(aEntry));
    }
    return true;
}

bool writeArchive(const OUString& rPathOrUrl, const std::vector<Entry>& rEntries)
{
    std::vector<sal_uInt8> aOut;
    struct Central
    {
        OString aName;
        sal_uInt16 nMethod;
        sal_uInt32 nCrc, nCSize, nUSize, nOffset;
    };
    std::vector<Central> aCentrals;
    // the zip format requires a timestamp field; write the zip epoch
    // (1980-01-01, the same "no date" convention python's zipfile uses)
    const sal_uInt16 nDosTime = 0;
    const sal_uInt16 nDosDate = (1 << 5) | 1;

    for (const Entry& rEntry : rEntries)
    {
        std::vector<sal_uInt8> aPayload;
        if (rEntry.nMethod == 8)
        {
            if (!deflateRaw(rEntry.aData.data(), rEntry.aData.size(), aPayload))
                return false;
        }
        else
            aPayload = rEntry.aData;
        const sal_uInt32 nCrc = crc32(0, rEntry.aData.data(), rEntry.aData.size());
        aCentrals.push_back(
            { rEntry.aName, rEntry.nMethod, nCrc, static_cast<sal_uInt32>(aPayload.size()),
              static_cast<sal_uInt32>(rEntry.aData.size()), static_cast<sal_uInt32>(aOut.size()) });
        pushU32(aOut, 0x04034b50);
        pushU16(aOut, 20);
        pushU16(aOut, 0);
        pushU16(aOut, rEntry.nMethod);
        pushU16(aOut, nDosTime);
        pushU16(aOut, nDosDate);
        pushU32(aOut, nCrc);
        pushU32(aOut, static_cast<sal_uInt32>(aPayload.size()));
        pushU32(aOut, static_cast<sal_uInt32>(rEntry.aData.size()));
        pushU16(aOut, static_cast<sal_uInt16>(rEntry.aName.getLength()));
        pushU16(aOut, 0);
        aOut.insert(aOut.end(), rEntry.aName.getStr(),
                    rEntry.aName.getStr() + rEntry.aName.getLength());
        aOut.insert(aOut.end(), aPayload.begin(), aPayload.end());
    }

    const sal_uInt32 nCdStart = static_cast<sal_uInt32>(aOut.size());
    for (const Central& rCentral : aCentrals)
    {
        pushU32(aOut, 0x02014b50);
        pushU16(aOut, 20); // version made by
        pushU16(aOut, 20); // version needed
        pushU16(aOut, 0); // flags
        pushU16(aOut, rCentral.nMethod);
        pushU16(aOut, nDosTime);
        pushU16(aOut, nDosDate);
        pushU32(aOut, rCentral.nCrc);
        pushU32(aOut, rCentral.nCSize);
        pushU32(aOut, rCentral.nUSize);
        pushU16(aOut, static_cast<sal_uInt16>(rCentral.aName.getLength()));
        pushU16(aOut, 0); // extra
        pushU16(aOut, 0); // comment
        pushU16(aOut, 0); // disk
        pushU16(aOut, 0); // internal attrs
        pushU32(aOut, 0); // external attrs
        pushU32(aOut, rCentral.nOffset);
        aOut.insert(aOut.end(), rCentral.aName.getStr(),
                    rCentral.aName.getStr() + rCentral.aName.getLength());
    }
    const sal_uInt32 nCdSize = static_cast<sal_uInt32>(aOut.size()) - nCdStart;
    pushU32(aOut, 0x06054b50);
    pushU16(aOut, 0);
    pushU16(aOut, 0);
    pushU16(aOut, static_cast<sal_uInt16>(aCentrals.size()));
    pushU16(aOut, static_cast<sal_uInt16>(aCentrals.size()));
    pushU32(aOut, nCdSize);
    pushU32(aOut, nCdStart);
    pushU16(aOut, 0);

    return writeFile(rPathOrUrl, aOut.data(), aOut.size());
}

} // namespace zip

} // namespace desktop::l10n

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
