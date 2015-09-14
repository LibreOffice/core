/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cmath>

#include <oox/ole/vbaexport.hxx>

#include <tools/stream.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <oox/helper/binaryoutputstream.hxx>
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

#include <sot/storage.hxx>

#include <rtl/uuid.h>

#include <comphelper/string.hxx>

#define USE_UTF8_CODEPAGE 0
#if USE_UTF8_CODEPAGE
#define CODEPAGE_MS 65001
#define CODEPAGE RTL_TEXTENCODING_UTF8
#else
#define CODEPAGE_MS 1252
#define CODEPAGE RTL_TEXTENCODING_MS_1252
#endif

#define VBA_EXPORT_DEBUG 0
#define VBA_USE_ORIGINAL_WM_STREAM 0
#define VBA_USE_ORIGINAL_DIR_STREAM 0
#define VBA_USE_ORIGINAL_PROJECT_STREAM 0
#define VBA_USE_ORIGINAL_VBA_PROJECT 0

namespace {

void exportString(SvStream& rStrm, const OUString& rString)
{
    OString aStringCorrectCodepage = OUStringToOString(rString, CODEPAGE);
    rStrm.WriteOString(aStringCorrectCodepage);
}

void exportUTF16String(SvStream& rStrm, const OUString& rString)
{
    sal_Int32 n = rString.getLength();
    const sal_Unicode* pString = rString.getStr();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode character = pString[i];
        rStrm.WriteUnicode(character);
    }
}

}

VBACompressionChunk::VBACompressionChunk(SvStream& rCompressedStream, const sal_uInt8* pData, sal_Size nChunkSize)
    : mrCompressedStream(rCompressedStream)
    , mpUncompressedData(pData)
    , mpCompressedChunkStream(0)
    , mnChunkSize(nChunkSize)
    , mnCompressedCurrent(0)
    , mnCompressedEnd(0)
    , mnDecompressedCurrent(0)
    , mnDecompressedEnd(0)
{
}

void setUInt16(sal_uInt8* pBuffer, size_t nPos, sal_uInt16 nVal)
{
    pBuffer[nPos] = nVal & 0xFF;
    pBuffer[nPos+1] = (nVal & 0xFF00) >> 8;
}

sal_uInt16 VBACompressionChunk::handleHeader(bool bCompressed)
{
    // handle header bytes
    size_t nSize = mnCompressedCurrent;
    sal_uInt16 nHeader = 0;
    PackCompressedChunkSize(nSize, nHeader);
    PackCompressedChunkFlag(bCompressed, nHeader);
    PackCompressedChunkSignature(nHeader);

    return nHeader;
}

// section 2.4.1.3.7
void VBACompressionChunk::write()
{

    mnDecompressedCurrent = 0;
    mnCompressedCurrent = 2;
    mnCompressedEnd = 4098;
    mnDecompressedEnd = std::min<sal_uInt64>(4096, mnChunkSize);

    // if that stream becomes larger than 4096 bytes then
    // we use the uncompressed stream
    sal_uInt8 pCompressedChunkStream[4098];
    mpCompressedChunkStream = pCompressedChunkStream;

    while (mnDecompressedCurrent < mnDecompressedEnd
            && mnCompressedCurrent < mnCompressedEnd)
    {
        // compress token sequence
        compressTokenSequence();
    }

    if (mnDecompressedCurrent < mnDecompressedEnd)
    {
        sal_uInt64 nChunkStart = mrCompressedStream.Tell();
        mrCompressedStream.WriteUInt16(0);
        writeRawChunk();
        mrCompressedStream.Seek(nChunkStart);
        sal_uInt16 nHeader = handleHeader(false);
        mrCompressedStream.WriteUInt16(nHeader);
    }
    else
    {
        sal_uInt16 nHeader = handleHeader(true);
        setUInt16(pCompressedChunkStream, 0, nHeader);
        // copy the compressed stream to our output stream
        mrCompressedStream.Write(pCompressedChunkStream, mnCompressedCurrent);
    }
}

// section 2.4.1.3.13
void VBACompressionChunk::PackCompressedChunkSize(size_t nSize, sal_uInt16& rHeader)
{
    sal_uInt16 nTemp1 = rHeader & 0xF000;
    sal_uInt16 nTemp2 = nSize - 3;
    rHeader = nTemp1 | nTemp2;
}

// section 2.4.1.3.16
void VBACompressionChunk::PackCompressedChunkFlag(bool bCompressed, sal_uInt16& rHeader)
{
    sal_uInt16 nTemp1 = rHeader & 0x7FFF;
    sal_uInt16 nTemp2 = ((sal_uInt16)bCompressed) << 15;
    rHeader = nTemp1 | nTemp2;
}

// section 2.4.1.3.14
void VBACompressionChunk::PackCompressedChunkSignature(sal_uInt16& rHeader)
{
    sal_Int32 nTemp = rHeader & 0x8FFFF;
    rHeader = nTemp | 0x3000;
}

// section 2.4.1.3.8
void VBACompressionChunk::compressTokenSequence()
{
    sal_uInt64 nFlagByteIndex = mnCompressedCurrent;
    sal_uInt8 nFlagByte = 0;
    ++mnCompressedCurrent;
    for (size_t index = 0; index <= 7; ++index)
    {
        if (mnDecompressedCurrent < mnDecompressedEnd
                && mnCompressedCurrent < mnCompressedEnd)
        {
            compressToken(index, nFlagByte);
        }
    }
    mpCompressedChunkStream[nFlagByteIndex] = nFlagByte;
}

// section 2.4.1.3.9
void VBACompressionChunk::compressToken(size_t index, sal_uInt8& nFlagByte)
{
    size_t nLength = 0;
    size_t nOffset = 0;
    match(nLength, nOffset);
    if (nOffset != 0)
    {
        if (mnCompressedCurrent + 1 < mnCompressedEnd)
        {
            sal_uInt16 nToken = CopyToken(nLength, nOffset);
            setUInt16(mpCompressedChunkStream, mnCompressedCurrent, nToken);
            SetFlagBit(index, true, nFlagByte);
            mnCompressedCurrent += 2;
            mnDecompressedCurrent += nLength;
        }
        else
        {
            mnCompressedCurrent = mnCompressedEnd;
        }
    }
    else
    {
        if (mnCompressedCurrent + 1 < mnCompressedEnd)
        {
            mpCompressedChunkStream[mnCompressedCurrent] = mpUncompressedData[mnDecompressedCurrent];
            ++mnCompressedCurrent;
            ++mnDecompressedCurrent;
        }
        else
        {
            mnCompressedCurrent = mnCompressedEnd;
        }
    }
}

// section 2.4.1.3.18
void VBACompressionChunk::SetFlagBit(size_t index, bool bVal, sal_uInt8& rFlag)
{
    size_t nTemp1 = ((int)bVal) << index;
    sal_uInt8 nTemp2 = rFlag & (~nTemp1);
    rFlag = nTemp2 | nTemp1;
}

// section 2.4.1.3.19.3
sal_uInt16 VBACompressionChunk::CopyToken(size_t nLength, size_t nOffset)
{
    sal_uInt16 nLengthMask = 0;
    sal_uInt16 nOffsetMask = 0;
    sal_uInt16 nBitCount = 0;
    sal_uInt16 nMaxLength;
    CopyTokenHelp(nLengthMask, nOffsetMask, nBitCount, nMaxLength);
    sal_uInt16 nTemp1 = nOffset -1;
    sal_uInt16 nTemp2 = 16 - nBitCount;
    sal_uInt16 nTemp3 = nLength - 3;
    sal_uInt16 nToken = (nTemp1 << nTemp2) | nTemp3;
    return nToken;
}

// section 2.4.1.3.19.4
void VBACompressionChunk::match(size_t& rLength, size_t& rOffset)
{
    size_t nBestLen = 0;
    sal_Int32 nCandidate = mnDecompressedCurrent - 1;
    sal_Int32 nBestCandidate = nCandidate;
    while (nCandidate >= 0)
    {
        sal_Int32 nC = nCandidate;
        sal_Int32 nD = mnDecompressedCurrent;
        size_t nLen = 0;
        while (nD < static_cast<sal_Int32>(mnChunkSize) // TODO: check if this needs to be including a minus -1
                && mpUncompressedData[nC] == mpUncompressedData[nD])
        {
            ++nLen;
            ++nC;
            ++nD;
        }
        if (nLen > nBestLen)
        {
            nBestLen = nLen;
            nBestCandidate = nCandidate;
        }
        --nCandidate;
    }

    if (nBestLen >= 3)
    {
        sal_uInt16 nMaximumLength = 0;
        sal_uInt16 nLengthMask, nOffsetMask, nBitCount;
        CopyTokenHelp(nLengthMask, nOffsetMask, nBitCount, nMaximumLength);
        rLength = std::min<sal_uInt16>(nMaximumLength, nBestLen);
        rOffset = mnDecompressedCurrent - nBestCandidate;
    }
    else
    {
        rLength = 0;
        rOffset = 0;
    }
}

// section 2.4.1.3.19.1
void VBACompressionChunk::CopyTokenHelp(sal_uInt16& rLengthMask, sal_uInt16& rOffsetMask,
        sal_uInt16& rBitCount, sal_uInt16& rMaximumLength)
{
    sal_uInt16 nDifference = mnDecompressedCurrent;
    sal_uInt16 nBitCount = std::ceil(std::log(nDifference)/std::log(2));
    rBitCount = std::max<sal_uInt16>(nBitCount, 4);
    rLengthMask = 0xffff >> rBitCount;
    rOffsetMask = ~rLengthMask;
    rMaximumLength = rLengthMask + 3;
}

// section 2.4.1.3.10
void VBACompressionChunk::writeRawChunk()
{
    // we need to use up to 4096 bytes of the original stream
    // and fill the rest with padding
    mrCompressedStream.Write(mpUncompressedData, mnChunkSize);
    sal_Size nPadding = 4096 - mnChunkSize;
    for (size_t i = 0; i < nPadding; ++i)
    {
        mrCompressedStream.WriteUInt8(0);
    }
}

VBACompression::VBACompression(SvStream& rCompressedStream,
        SvMemoryStream& rUncompressedStream):
    mrCompressedStream(rCompressedStream),
    mrUncompressedStream(rUncompressedStream)
{
}

// section 2.4.1.3.6
void VBACompression::write()
{
    // section 2.4.1.1.1
    mrCompressedStream.WriteUInt8(0x01); // signature byte of a compressed container
    bool bStreamNotEnded = true;
    const sal_uInt8* pData = static_cast<const sal_uInt8*>(mrUncompressedStream.GetData());
    sal_Size nSize = mrUncompressedStream.GetEndOfData();
    sal_Size nRemainingSize = nSize;
    while(bStreamNotEnded)
    {
        sal_Size nChunkSize = nRemainingSize > 4096 ? 4096 : nRemainingSize;
        VBACompressionChunk aChunk(mrCompressedStream, &pData[nSize - nRemainingSize], nChunkSize);
        aChunk.write();

        // update the uncompressed chunk start marker
        nRemainingSize -= nChunkSize;
        bStreamNotEnded = nRemainingSize != 0;
    }
}

VbaExport::VbaExport(css::uno::Reference<css::frame::XModel> xModel):
    mxModel(xModel)
{
}

namespace {

// section 2.3.4.2.1.1
void writePROJECTSYSKIND(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0001); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000001); // SysKind, hard coded to 32-bin windows for now
}

// section 2.3.4.2.1.2
void writePROJECTLCID(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0002); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // Lcid
}

// section 2.3.4.2.1.3
void writePROJECTLCIDINVOKE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0014); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000409); // LcidInvoke
}

// section 2.3.4.2.1.4
void writePROJECTCODEPAGE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0003); // id
    rStrm.WriteUInt32(0x00000002); // size
    rStrm.WriteUInt16(CODEPAGE_MS); // CodePage
}

//section 2.3.4.2.1.5
void writePROJECTNAME(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0004); // id
    sal_uInt32 sizeOfProjectName = 0x0000000a; // for project name "VBAProject"
    rStrm.WriteUInt32(sizeOfProjectName); // sizeOfProjectName
    exportString(rStrm, "VBAProject"); // ProjectName
}

//section 2.3.4.2.1.6
void writePROJECTDOCSTRING(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0005); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfDocString
    rStrm.WriteUInt16(0x0040); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfDocStringUnicode, MUST be even
}

//section 2.3.4.2.1.7
void writePROJECTHELPFILEPATH(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0006); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfHelpFile1
    rStrm.WriteUInt16(0x003D); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfHelpFile2
}

//section 2.3.4.2.1.8
void writePROJECTHELPCONTEXT(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0007); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000000); // HelpContext
}

//section 2.3.4.2.1.9
void writePROJECTLIBFLAGS(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0008); // id
    rStrm.WriteUInt32(0x00000004); // size
    rStrm.WriteUInt32(0x00000000); // ProjectLibFlags
}

//section 2.3.4.2.1.10
void writePROJECTVERSION(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0009); // id
    rStrm.WriteUInt32(0x00000004); // Reserved
    rStrm.WriteUInt32(1467127224); // VersionMajor // TODO: where is this magic number comming from
    rStrm.WriteUInt16(5); // VersionMinor // TODO: where is this magic number coming from
}

//section 2.3.4.2.1.11
void writePROJECTCONSTANTS(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x000C); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfConstants
    rStrm.WriteUInt16(0x003C); // Reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfConstantsUnicode
}

// section 2.3.4.2.1
void writePROJECTINFORMATION(SvStream& rStrm)
{
    writePROJECTSYSKIND(rStrm);
    writePROJECTLCID(rStrm);
    writePROJECTLCIDINVOKE(rStrm);
    writePROJECTCODEPAGE(rStrm);
    writePROJECTNAME(rStrm);
    writePROJECTDOCSTRING(rStrm);
    writePROJECTHELPFILEPATH(rStrm);
    writePROJECTHELPCONTEXT(rStrm);
    writePROJECTLIBFLAGS(rStrm);
    writePROJECTVERSION(rStrm);
    writePROJECTCONSTANTS(rStrm);
}

// section 2.3.4.2.2.2
void writeREFERENCENAME(SvStream& rStrm, const OUString& name)
{
    rStrm.WriteUInt16(0x0016); // id
    sal_Int32 size = name.getLength();
    rStrm.WriteUInt32(size); // sizeOfName
    exportString(rStrm, name); // name
    rStrm.WriteUInt16(0x003E); // reserved
    sal_Int32 unicodesize = size * 2;
    rStrm.WriteUInt32(unicodesize); // sizeOfNameUnicode
    exportUTF16String(rStrm, name); // nameUnicode
}

// section 2.3.4.2.2.5
void writeREFERENCEREGISTERED(SvStream& rStrm, const OUString& libid)
{
    rStrm.WriteUInt16(0x000D); // id
    sal_Int32 sizeOfLibid = libid.getLength();
    sal_Int32 size = sizeOfLibid + 10; // size of Libid, sizeOfLibid(4 bytes), reserved 1(4 bytes) and reserved 2(2 bytes)
    rStrm.WriteUInt32(size); // size
    rStrm.WriteUInt32(sizeOfLibid); // sizeOfLibid
    exportString(rStrm, libid); // Libid
    rStrm.WriteUInt32(0x00000000); // reserved 1
    rStrm.WriteUInt16(0x0000); // reserved 2
}

// section 2.3.4.2.2.1
void writeREFERENCE(SvStream& rStrm, const OUString& name, const OUString& libid)
{
    writeREFERENCENAME(rStrm, name);
    writeREFERENCEREGISTERED(rStrm, libid);
}

// section 2.3.4.2.2
void writePROJECTREFERENCES(SvStream& rStrm)
{
    // TODO: find out where these references are coming from
    writeREFERENCE(rStrm, "stdole", "*\\G{00020430-0000-0000-C000-000000000046}#2.0#0#C:\\Windows\\SysWOW64\\stdole2.tlb#OLE Automation");
    writeREFERENCE(rStrm, "Office", "*\\G{2DF8D04C-5BFA-101B-BDE5-00AA0044DE52}#2.0#0#C:\\Program Files (x86)\\Common Files\\Microsoft Shared\\OFFICE14\\MSO.DLL#Microsoft Office 14.0 Object Library");
}

// section 2.3.4.2.3.1
void writePROJECTCOOKIE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0013); // id
    rStrm.WriteUInt32(0x00000002); // size
    rStrm.WriteUInt16(0xFFFF); // cookie
}

// section 2.3.4.2.3.2.1
void writeMODULENAME(SvStream& rStrm, const OUString& name)
{
    rStrm.WriteUInt16(0x0019); // id
    sal_Int32 n = name.getLength(); // sizeOfModuleName
    rStrm.WriteUInt32(n);
    exportString(rStrm, name); // ModuleName
}

// section 2.3.4.2.3.2.2
void writeMODULENAMEUNICODE(SvStream& rStrm, const OUString& name)
{
    rStrm.WriteUInt16(0x0047); // id
    sal_Int32 n = name.getLength() * 2; // sizeOfModuleNameUnicode // TODO: better calculation for unicode string length
    rStrm.WriteUInt32(n);
    exportUTF16String(rStrm, name); // ModuleNameUnicode
}

// section 2.3.4.2.3.2.3
void writeMODULESTREAMNAME(SvStream& rStrm, const OUString& streamName)
{
    rStrm.WriteUInt16(0x001A); // id
    sal_Int32 n = streamName.getLength(); // sizeOfStreamName
    rStrm.WriteUInt32(n);
    exportString(rStrm, streamName); // StreamName
    rStrm.WriteUInt16(0x0032); // reserved
    rStrm.WriteUInt32(n * 2); // sizeOfStreamNameUnicode // TODO: better calculation for unicode string length
    exportUTF16String(rStrm, streamName); // StreamNameUnicode
}

// section 2.3.4.2.3.2.4
void writeMODULEDOCSTRING(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x001C); // id
    rStrm.WriteUInt32(0x00000000); // sizeOfDocString
    rStrm.WriteUInt16(0x0048); // reserved
    rStrm.WriteUInt32(0x00000000); // sizeOfDocStringUnicode
}

// section 2.3.4.2.3.2.5
void writeMODULEOFFSET(SvStream& rStrm, sal_uInt32 offset)
{
    rStrm.WriteUInt16(0x0031); // id
    rStrm.WriteUInt32(0x00000004); // sizeOfTextOffset
    rStrm.WriteUInt32(offset); // TextOffset
}

// section 2.3.4.2.3.2.6
void writeMODULEHELPCONTEXT(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x001E); // id
    rStrm.WriteUInt32(0x00000004); // sizeOfHelpContext
    rStrm.WriteUInt32(0x00000000); // HelpContext
}

// section 2.3.4.2.3.2.7
void writeMODULECOOKIE(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x002C); // id
    rStrm.WriteUInt32(0x00000002); // sizeOfHelpContext
    rStrm.WriteUInt16(0xFFFF); // HelpContext
}

// section 2.3.4.2.3.2.8
void writeMODULETYPE(SvStream& rStrm, const sal_uInt16 type)
{
    if(type == 1)
        rStrm.WriteUInt16(0x0021); // id for a procedural module
    else
        rStrm.WriteUInt16(0x0022); // id for document, class or design module
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3.2
void writePROJECTMODULE(SvStream& rStrm, const OUString& name, const OUString& streamName, sal_uInt32 offset, const sal_uInt16 type)
{
    writeMODULENAME(rStrm, name);
    writeMODULENAMEUNICODE(rStrm, name);
    writeMODULESTREAMNAME(rStrm, streamName);
    writeMODULEDOCSTRING(rStrm);
    writeMODULEOFFSET(rStrm, offset);
    writeMODULEHELPCONTEXT(rStrm);
    writeMODULECOOKIE(rStrm);
    writeMODULETYPE(rStrm, type);
    rStrm.WriteUInt16(0x002B); // terminator
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3
void writePROJECTMODULES(SvStream& rStrm, css::uno::Reference<css::container::XNameContainer> xNameContainer)
{
    css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength();
    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);
    assert(xModuleInfo.is());

    // TODO: this whole part is document specific
    rStrm.WriteUInt16(0x000F); // id
    rStrm.WriteUInt32(0x00000002); // size of Count
    sal_Int16 count = n; // Number of modules // TODO: this is dependent on the document
    rStrm.WriteUInt16(count); // Count
    writePROJECTCOOKIE(rStrm);

    for (sal_Int32 i = 0; i < n; ++i)
    {
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        writePROJECTMODULE(rStrm, aElementNames[i], aElementNames[i], 0x00000000, aModuleInfo.ModuleType);
    }
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm, css::uno::Reference<css::container::XNameContainer> xNameContainer)
{
    SvMemoryStream aDirStream(4096, 4096);

    writePROJECTINFORMATION(aDirStream);
    writePROJECTREFERENCES(aDirStream);
    writePROJECTMODULES(aDirStream, xNameContainer);
    aDirStream.WriteUInt16(0x0010); // terminator
    aDirStream.WriteUInt32(0x00000000); // reserved

    aDirStream.Seek(0);

#if VBA_EXPORT_DEBUG
    const OUString aDirFileName("/tmp/vba_dir_out.bin");
    SvFileStream aDirStreamDebug(aDirFileName, STREAM_READWRITE);

    aDirStreamDebug.WriteStream(aDirStream);
    aDirStream.Seek(0);
#endif

    // the stream for the compression
    SvMemoryStream aMemoryStream(4096, 4096);
    aMemoryStream.WriteStream(aDirStream);

    VBACompression aCompression(rStrm, aDirStream);
    aCompression.write();
}

// section 2.3.4.3 Module Stream
void exportModuleStream(SvStream& rStrm, const OUString& rSourceCode, const OUString& aElementName, sal_Int32 nModuleType)
{
    SvMemoryStream aModuleStream(4096, 4096);

    exportString(aModuleStream, "Attribute VB_Name = \"" + aElementName + "\"\r\n");
    if (nModuleType == 4)
    {
        exportString(aModuleStream, "Attribute VB_Base = \"0{00020820-0000-0000-C000-000000000046}\"\r\n");
        exportString(aModuleStream, "Attribute VB_GlobalNameSpace = False\r\n");
        exportString(aModuleStream, "Attribute VB_Creatable = False\r\n");
        exportString(aModuleStream, "Attribute VB_PredeclaredId = True\r\n");
        exportString(aModuleStream, "Attribute VB_Exposed = True\r\n");
        exportString(aModuleStream, "Attribute VB_TemplateDerived = False\r\n");
        exportString(aModuleStream, "Attribute VB_Customizable = True\r\n");
    }
    OUString aSourceCode = rSourceCode.replaceFirst("Option VBASupport 1\n", "");
    const sal_Int32 nPos = aSourceCode.indexOf("Rem Attribute VBA_ModuleType=");
    const sal_Int32 nEndPos = nPos != -1 ? aSourceCode.indexOf("\n", nPos) : -1;
    if (nPos != -1 && nEndPos != -1)
        aSourceCode = aSourceCode.replaceAt(nPos, nEndPos - nPos+1, "");
    aSourceCode = aSourceCode.replaceAll("\n", "\r\n");
    exportString(aModuleStream, aSourceCode);
    aModuleStream.Seek(0);

#if VBA_EXPORT_DEBUG
    OUString aModuleFileName("/tmp/vba_" + aElementName + "_out.bin");
    SvFileStream aModuleStreamDebug(aModuleFileName, STREAM_READWRITE);
    aModuleStreamDebug.WriteStream(aModuleStream);
    aModuleStream.Seek(0);
#endif

    // the stream for the compression
    SvMemoryStream aMemoryStream(4096, 4096);
    aMemoryStream.WriteStream(aModuleStream);

    VBACompression aCompression(rStrm, aModuleStream);
    aCompression.write();
}

// section 2.3.4.1 _VBA_PROJECT Stream
void exportVBAProjectStream(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x61CC); // Reserved1
    rStrm.WriteUInt16(0xFFFF); // Version
    rStrm.WriteUInt8(0x00); // Reserved2
    rStrm.WriteUInt16(0x0000); // Undefined
}

OString createHexStringFromDigit(sal_uInt8 nDigit)
{
    OString aString = OString::number( nDigit, 16 );
    if(aString.getLength() == 1)
        aString = aString + OString::number(0);
    return aString;
}

OString createGuidStringFromInt(sal_uInt8 nGuid[16])
{
    OStringBuffer aBuffer;
    aBuffer.append('{');
    for(size_t i = 0; i < 16; ++i)
    {
        aBuffer.append(createHexStringFromDigit(nGuid[i]));
        if(i == 3|| i == 5 || i == 7 || i == 9 )
            aBuffer.append('-');
    }
    aBuffer.append('}');
    OString aString = aBuffer.makeStringAndClear();
    return aString.toAsciiUpperCase();
}

OString generateGUIDString()
{
    sal_uInt8 nGuid[16];
    rtl_createUuid(nGuid, NULL, true);
    return createGuidStringFromInt(nGuid);
}

// section 2.3.1 PROJECT Stream
void exportPROJECTStream(SvStream& rStrm, css::uno::Reference<css::container::XNameContainer> xNameContainer, const OUString& projectName)
{
    css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength();
    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);
    assert(xModuleInfo.is());

    // section 2.3.1.1ProjectProperties

    // section 2.3.1.2 ProjectId
    exportString(rStrm, "ID=\"");
    rStrm.WriteOString(generateGUIDString());
    exportString(rStrm, "\"\r\n");

    // section 2.3.1.3 ProjectModule
    for (sal_Int32 i = 0; i < n; ++i)
    {
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        if(aModuleInfo.ModuleType == 1)
        {
            exportString(rStrm, "Module=" + aElementNames[i] + "\r\n");
        }
        else if(aModuleInfo.ModuleType == 4)
        {
            exportString(rStrm, "Document=" + aElementNames[i] + "/&H00000000\r\n");
        }
    }

    // section 2.3.1.11 ProjectName
    exportString(rStrm, "Name=\"" + projectName + "\"\r\n");

    // section 2.3.1.12 ProjectHelpId
    exportString(rStrm, "HelpContextID=\"0\"\r\n");

    // section 2.3.1.14 ProjectVersionCompat32
    exportString(rStrm, "VersionCompatible32=\"393222000\"\r\n");

    // section 2.3.1.15 ProjectProtectionState
    exportString(rStrm, "CMG=\"BEBC9256EEAAA8AEA8AEA8AEA8AE\"\r\n");

    // section 2.3.1.16 ProjectPassword
    exportString(rStrm, "DPB=\"7C7E5014B0D3B1D3B1D3\"\r\n");

    // section 2.3.1.17 ProjectVisibilityState
    exportString(rStrm, "GC=\"3A3816DAD5DBD5DB2A\"\r\n\r\n");

    // section 2.3.1.18 HostExtenders
    exportString(rStrm, "[Host Extender Info]\r\n"
                        "&H00000001={3832D640-CF90-11CF-8E43-00A0C911005A};VBE;&H00000000\r\n"
    );
}

// section 2.3.3.1 NAMEMAP
void writeNAMEMAP(SvStream& rStrm, const css::uno::Sequence<OUString>& rElementNames)
{
    int n = rElementNames.getLength();
    for(sal_Int32 i = 0; i < n; ++i)
    {
        exportString(rStrm, rElementNames[i]);
        rStrm.WriteUInt8(0x00); // terminator
        exportUTF16String(rStrm, rElementNames[i]);
        rStrm.WriteUInt16(0x0000); // terminator
    }
}

// section 2.3.3 PROJECTwm Stream
void exportPROJECTwmStream(SvStream& rStrm, const css::uno::Sequence<OUString>& rElementNames)
{
    writeNAMEMAP(rStrm, rElementNames);
    rStrm.WriteUInt16(0x0000); // terminator
}

}

void addFileStreamToSotStream(const OUString& rPath, SotStorageStream* pStream)
{
    SvFileStream aFileStream(rPath, STREAM_READWRITE);
    pStream->WriteStream(aFileStream);
}

void VbaExport::exportVBA(SotStorage* pRootStorage)
{
    css::uno::Reference<css::container::XNameContainer> xNameContainer = getBasicLibrary();
    if (!xNameContainer.is()) {
        return;
    }
    css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength(); // get the number of modules

    // start here with the VBA export
    SotStorage* pVBAStream = pRootStorage->OpenSotStorage("VBA", STREAM_READWRITE);
    SotStorageStream* pDirStream = pVBAStream->OpenSotStream("dir", STREAM_READWRITE);

    SotStorageStream* pVBAProjectStream = pVBAStream->OpenSotStream("_VBA_PROJECT", STREAM_READWRITE);
    SotStorageStream* pPROJECTStream = pRootStorage->OpenSotStream("PROJECT", STREAM_READWRITE);
    SotStorageStream* pPROJECTwmStream = pRootStorage->OpenSotStream("PROJECTwm", STREAM_READWRITE);

#if VBA_USE_ORIGINAL_WM_STREAM
    OUString aProjectwmPath = "/home/moggi/Documents/testfiles/vba/PROJECTwm";
    addFileStreamToSotStream(aProjectwmPath, pPROJECTwmStream);
#else
    exportPROJECTwmStream(*pPROJECTwmStream, aElementNames);
#endif

#if VBA_USE_ORIGINAL_DIR_STREAM
    OUString aDirPath = "/home/moggi/Documents/testfiles/vba/VBA/dir";
    addFileStreamToSotStream(aDirPath, pDirStream);
#else
    std::vector<SotStorageStream*> aModuleStreams;
    exportDirStream(*pDirStream, xNameContainer);
    aModuleStreams.reserve(n);
    for (sal_Int32 i = 0; i < n; ++i)
    {
        aModuleStreams.push_back(pVBAStream->OpenSotStream(aElementNames[i], STREAM_READWRITE));
    }
#endif

#if VBA_USE_ORIGINAL_PROJECT_STREAM
    OUString aProjectPath = "/home/moggi/Documents/testfiles/vba/PROJECT";
    addFileStreamToSotStream(aProjectPath, pPROJECTStream);
#else
    exportPROJECTStream(*pPROJECTStream, xNameContainer, getProjectName());
#endif

#if VBA_USE_ORIGINAL_VBA_PROJECT
    OUString a_VBA_ProjectPath = "/home/moggi/Documents/testfiles/vba/VBA/_VBA_PROJECT";
    addFileStreamToSotStream(a_VBA_ProjectPath, pVBAProjectStream);
#else
    exportVBAProjectStream(*pVBAProjectStream);
#endif

#if VBA_USE_ORIGINAL_DIR_STREAM
    OUString aModule1Path = "/home/moggi/Documents/testfiles/vba/VBA/Module1";
    OUString aSheet1Path = "/home/moggi/Documents/testfiles/vba/VBA/Sheet1";
    OUString aSheet2Path = "/home/moggi/Documents/testfiles/vba/VBA/Sheet2";
    OUString aSheet3Path = "/home/moggi/Documents/testfiles/vba/VBA/Sheet3";
    OUString aWorkbookPath = "/home/moggi/Documents/testfiles/vba/VBA/ThisWorkbook";
    SotStorageStream* pModule1Stream = pVBAStream->OpenSotStream("Module1", STREAM_READWRITE);
    SotStorageStream* pSheet1Stream = pVBAStream->OpenSotStream("Sheet1", STREAM_READWRITE);
    SotStorageStream* pSheet2Stream = pVBAStream->OpenSotStream("Sheet2", STREAM_READWRITE);
    SotStorageStream* pSheet3Stream = pVBAStream->OpenSotStream("Sheet3", STREAM_READWRITE);
    SotStorageStream* pWorkbookStream = pVBAStream->OpenSotStream("ThisWorkbook", STREAM_READWRITE);
    addFileStreamToSotStream(aModule1Path, pModule1Stream);
    addFileStreamToSotStream(aSheet1Path, pSheet1Stream);
    addFileStreamToSotStream(aSheet2Path, pSheet2Stream);
    addFileStreamToSotStream(aSheet3Path, pSheet3Stream);
    addFileStreamToSotStream(aWorkbookPath, pWorkbookStream);

    pModule1Stream->Commit();
    pSheet1Stream->Commit();
    pSheet2Stream->Commit();
    pSheet3Stream->Commit();
    pWorkbookStream->Commit();
#else

    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);
    for (sal_Int32 i = 0; i < n; ++i)
    {
        css::uno::Any aCode = xNameContainer->getByName(aElementNames[i]);
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        OUString aSourceCode;
        aCode >>= aSourceCode;
        exportModuleStream(*aModuleStreams[i], aSourceCode, aElementNames[i], aModuleInfo.ModuleType);
        aModuleStreams[i]->Commit();
    }

#endif

    pVBAProjectStream->Commit();

    pDirStream->Commit();
    pVBAStream->Commit();
    pPROJECTStream->Commit();
    pPROJECTwmStream->Commit();
    pRootStorage->Commit();
}

css::uno::Reference<css::script::XLibraryContainer> VbaExport::getLibraryContainer()
{
    oox::PropertySet aDocProp(mxModel);
    css::uno::Reference<css::script::XLibraryContainer> xLibContainer(aDocProp.getAnyProperty(oox::PROP_BasicLibraries), css::uno::UNO_QUERY);

    return xLibContainer;
}

css::uno::Reference<css::container::XNameContainer> VbaExport::getBasicLibrary()
{
    css::uno::Reference<css::container::XNameContainer> xLibrary;
    try
    {
        css::uno::Reference<css::script::XLibraryContainer> xLibContainer = getLibraryContainer();
        OUString aProjectName = getProjectName();
        xLibrary.set( xLibContainer->getByName(aProjectName), css::uno::UNO_QUERY_THROW );
    }
    catch(...)
    {
    }

    return xLibrary;
}

bool VbaExport::containsVBAProject()
{
    css::uno::Reference<css::script::XLibraryContainer> xLibContainer = getLibraryContainer();
    if (!xLibContainer.is())
        return false;

    css::uno::Reference<css::script::vba::XVBACompatibility> xVbaCompatibility (xLibContainer, css::uno::UNO_QUERY);
    if (!xVbaCompatibility.is())
        return false;

    bool bVBACompatibilty = xVbaCompatibility->getVBACompatibilityMode();

    return bVBACompatibilty;
}

OUString VbaExport::getProjectName()
{
    css::uno::Reference<css::script::vba::XVBACompatibility> xVbaCompatibility(getLibraryContainer(), css::uno::UNO_QUERY);
    if (xVbaCompatibility.is())
        return xVbaCompatibility->getProjectName();

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
