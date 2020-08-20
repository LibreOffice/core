/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <random>

#include <oox/ole/vbaexport.hxx>

#include <tools/stream.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <ooo/vba/excel/XWorkbook.hpp>

#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>

#include <sot/storage.hxx>

#include <comphelper/xmltools.hxx>

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

/* Enable to see VBA Encryption work. For now the input data and length values
 * for encryption correspond to the case when the VBA macro is not protected.
 */
#define VBA_ENCRYPTION 1

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

bool isWorkbook(const css::uno::Reference<css::uno::XInterface>& xInterface)
{
    css::uno::Reference<ooo::vba::excel::XWorkbook> xWorkbook(xInterface, css::uno::UNO_QUERY);
    return xWorkbook.is();
}

OUString createHexStringFromDigit(sal_uInt8 nDigit)
{
    OUString aString = OUString::number( nDigit, 16 );
    if(aString.getLength() == 1)
        aString = OUString::number(0) + aString;
    return aString.toAsciiUpperCase();
}

}

VBACompressionChunk::VBACompressionChunk(SvStream& rCompressedStream, const sal_uInt8* pData, std::size_t nChunkSize)
    : mrCompressedStream(rCompressedStream)
    , mpUncompressedData(pData)
    , mpCompressedChunkStream(nullptr)
    , mnChunkSize(nChunkSize)
    , mnCompressedCurrent(0)
    , mnCompressedEnd(0)
    , mnDecompressedCurrent(0)
    , mnDecompressedEnd(0)
{
}

static void setUInt16(sal_uInt8* pBuffer, size_t nPos, sal_uInt16 nVal)
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
        mrCompressedStream.WriteBytes(pCompressedChunkStream, mnCompressedCurrent);
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
    sal_uInt16 nTemp2 = static_cast<sal_uInt16>(bCompressed) << 15;
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
    size_t nTemp1 = static_cast<int>(bVal) << index;
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
    assert(nDifference <= 4096);
    assert(nDifference >= 1);
    if (nDifference >= 2049)
        rBitCount = 12;
    else if (nDifference >= 1025)
        rBitCount = 11;
    else if (nDifference >= 513)
        rBitCount = 10;
    else if (nDifference >= 257)
        rBitCount = 9;
    else if (nDifference >= 129)
        rBitCount = 8;
    else if (nDifference >= 65)
        rBitCount = 7;
    else if (nDifference >= 33)
        rBitCount = 6;
    else if (nDifference >= 17)
        rBitCount = 5;
    else
        rBitCount = 4;
    rLengthMask = 0xffff >> rBitCount;
    rOffsetMask = ~rLengthMask;
    rMaximumLength = rLengthMask + 3;
}

// section 2.4.1.3.10
void VBACompressionChunk::writeRawChunk()
{
    // we need to use up to 4096 bytes of the original stream
    // and fill the rest with padding
    mrCompressedStream.WriteBytes(mpUncompressedData, mnChunkSize);
    std::size_t nPadding = 4096 - mnChunkSize;
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
    std::size_t nSize = mrUncompressedStream.GetEndOfData();
    std::size_t nRemainingSize = nSize;
    while(bStreamNotEnded)
    {
        std::size_t nChunkSize = std::min<size_t>(nRemainingSize, 4096);
        VBACompressionChunk aChunk(mrCompressedStream, &pData[nSize - nRemainingSize], nChunkSize);
        aChunk.write();

        // update the uncompressed chunk start marker
        nRemainingSize -= nChunkSize;
        bStreamNotEnded = nRemainingSize != 0;
    }
}

// section 2.4.3
#if VBA_ENCRYPTION

VBAEncryption::VBAEncryption(const sal_uInt8* pData, const sal_uInt16 length, SvStream& rEncryptedData, sal_uInt8 nProjKey)
    :mpData(pData)
    ,mnLength(length)
    ,mrEncryptedData(rEncryptedData)
    ,mnUnencryptedByte1(0)
    ,mnEncryptedByte1(0)
    ,mnEncryptedByte2(0)
    ,mnProjKey(nProjKey)
    ,mnIgnoredLength(0)
    ,mnSeed(0x00)
    ,mnVersionEnc(0)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    mnSeed = dis(gen);
}

void VBAEncryption::writeSeed()
{
    exportString(mrEncryptedData, createHexStringFromDigit(mnSeed));
}

void VBAEncryption::writeVersionEnc()
{
    static const sal_uInt8 mnVersion = 2; // the encrypted version
    mnVersionEnc = mnSeed ^ mnVersion;
    exportString(mrEncryptedData, createHexStringFromDigit(mnVersionEnc));
}

sal_uInt8 VBAEncryption::calculateProjKey(const OUString& rProjectKey)
{
    sal_uInt8 nProjKey = 0;
    sal_Int32 n = rProjectKey.getLength();
    const sal_Unicode* pString = rProjectKey.getStr();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        sal_Unicode character = pString[i];
        nProjKey += character;
    }

    return nProjKey;
}

void VBAEncryption::writeProjKeyEnc()
{
    sal_uInt8 nProjKeyEnc = mnSeed ^ mnProjKey;
    exportString(mrEncryptedData, createHexStringFromDigit(nProjKeyEnc));
    mnUnencryptedByte1 = mnProjKey;
    mnEncryptedByte1 = nProjKeyEnc; // ProjKeyEnc
    mnEncryptedByte2 = mnVersionEnc; // VersionEnc
}

void VBAEncryption::writeIgnoredEnc()
{
    mnIgnoredLength = (mnSeed & 6) / 2;
    for(sal_Int32 i = 1; i <= mnIgnoredLength; ++i)
    {
        sal_uInt8 nTempValue = 0xBE; // Any value can be assigned here
        sal_uInt8 nByteEnc = nTempValue ^ (mnEncryptedByte2 + mnUnencryptedByte1);
        exportString(mrEncryptedData, createHexStringFromDigit(nByteEnc));
        mnEncryptedByte2 = mnEncryptedByte1;
        mnEncryptedByte1 = nByteEnc;
        mnUnencryptedByte1 = nTempValue;
    }
}

void VBAEncryption::writeDataLengthEnc()
{
    sal_uInt16 temp = mnLength;
    for(sal_Int8 i = 0; i < 4; ++i)
    {
        sal_uInt8 nByte = temp & 0xFF;
        sal_uInt8 nByteEnc = nByte ^ (mnEncryptedByte2 + mnUnencryptedByte1);
        exportString(mrEncryptedData, createHexStringFromDigit(nByteEnc));
        mnEncryptedByte2 = mnEncryptedByte1;
        mnEncryptedByte1 = nByteEnc;
        mnUnencryptedByte1 = nByte;
        temp >>= 8;
    }
}

void VBAEncryption::writeDataEnc()
{
    for(sal_Int16 i = 0; i < mnLength; i++)
    {
        sal_uInt8 nByteEnc = mpData[i] ^ (mnEncryptedByte2 + mnUnencryptedByte1);
        exportString(mrEncryptedData, createHexStringFromDigit(nByteEnc));
        mnEncryptedByte2 = mnEncryptedByte1;
        mnEncryptedByte1 = nByteEnc;
        mnUnencryptedByte1 = mpData[i];
    }
}

void VBAEncryption::write()
{
    writeSeed();
    writeVersionEnc();
    writeProjKeyEnc();
    writeIgnoredEnc();
    writeDataLengthEnc();
    writeDataEnc();
}

#endif

VbaExport::VbaExport(css::uno::Reference<css::frame::XModel> const & xModel):
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
void writePROJECTNAME(SvStream& rStrm, const OUString& name)
{
    rStrm.WriteUInt16(0x0004); // id
    sal_uInt32 sizeOfProjectName = name.getLength();
    rStrm.WriteUInt32(sizeOfProjectName); // sizeOfProjectName
    exportString(rStrm, name); // ProjectName
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
    rStrm.WriteUInt32(1467127224); // VersionMajor // TODO: where is this magic number coming from
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
void writePROJECTINFORMATION(SvStream& rStrm, const OUString& projectName)
{
    writePROJECTSYSKIND(rStrm);
    writePROJECTLCID(rStrm);
    writePROJECTLCIDINVOKE(rStrm);
    writePROJECTCODEPAGE(rStrm);
    writePROJECTNAME(rStrm, projectName);
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
void writeMODULEOFFSET(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x0031); // id
    rStrm.WriteUInt32(0x00000004); // sizeOfTextOffset
    rStrm.WriteUInt32(0x00000000); // TextOffset
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
void writePROJECTMODULE(SvStream& rStrm, const OUString& name, const sal_uInt16 type)
{
    writeMODULENAME(rStrm, name);
    writeMODULENAMEUNICODE(rStrm, name);
    writeMODULESTREAMNAME(rStrm, name);
    writeMODULEDOCSTRING(rStrm);
    writeMODULEOFFSET(rStrm);
    writeMODULEHELPCONTEXT(rStrm);
    writeMODULECOOKIE(rStrm);
    writeMODULETYPE(rStrm, type);
    rStrm.WriteUInt16(0x002B); // terminator
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3
void writePROJECTMODULES(SvStream& rStrm, const css::uno::Reference<css::container::XNameContainer>& xNameContainer, const std::vector<sal_Int32>& rLibrayMap)
{
    const css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
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
        const OUString& rModuleName = aElementNames[rLibrayMap[i]];
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(rModuleName);
        writePROJECTMODULE(rStrm, rModuleName, aModuleInfo.ModuleType);
    }
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm, const css::uno::Reference<css::container::XNameContainer>& xNameContainer, const std::vector<sal_Int32>& rLibraryMap, const OUString& projectName)
{
    SvMemoryStream aDirStream(4096, 4096);

    writePROJECTINFORMATION(aDirStream, projectName);
    writePROJECTREFERENCES(aDirStream);
    writePROJECTMODULES(aDirStream, xNameContainer, rLibraryMap);
    aDirStream.WriteUInt16(0x0010); // terminator
    aDirStream.WriteUInt32(0x00000000); // reserved

    aDirStream.Seek(0);

#if VBA_EXPORT_DEBUG
    const OUString aDirFileName("/tmp/vba_dir_out.bin");
    SvFileStream aDirStreamDebug(aDirFileName, StreamMode::READWRITE);

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
void exportModuleStream(SvStream& rStrm, const OUString& rSourceCode, const OUString& aElementName, css::script::ModuleInfo const & rInfo)
{
    SvMemoryStream aModuleStream(4096, 4096);

    exportString(aModuleStream, "Attribute VB_Name = \"" + aElementName + "\"\r\n");
    if (rInfo.ModuleType == 4)
    {
        if (isWorkbook(rInfo.ModuleObject))
            exportString(aModuleStream, "Attribute VB_Base = \"0{00020819-0000-0000-C000-000000000046}\"\r\n");
        else
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
    SvFileStream aModuleStreamDebug(aModuleFileName, StreamMode::READWRITE);
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

// section 2.3.1 PROJECT Stream
void exportPROJECTStream(SvStream& rStrm, const css::uno::Reference<css::container::XNameContainer>& xNameContainer,
        const OUString& projectName, const std::vector<sal_Int32>& rLibraryMap)
{
    const css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength();
    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);
    assert(xModuleInfo.is());

    // section 2.3.1.1ProjectProperties

    // section 2.3.1.2 ProjectId
    exportString(rStrm, "ID=\"");
    OUString aProjectID
        = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_UTF8);
    exportString(rStrm, aProjectID);
    exportString(rStrm, "\"\r\n");

    // section 2.3.1.3 ProjectModule
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rModuleName = aElementNames[rLibraryMap[i]];
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(rModuleName);
        if(aModuleInfo.ModuleType == 1)
        {
            exportString(rStrm, "Module=" + rModuleName + "\r\n");
        }
        else if(aModuleInfo.ModuleType == 4)
        {
            exportString(rStrm, "Document=" + rModuleName + "/&H00000000\r\n");
        }
    }

    // section 2.3.1.11 ProjectName
    exportString(rStrm, "Name=\"" + projectName + "\"\r\n");

    // section 2.3.1.12 ProjectHelpId
    exportString(rStrm, "HelpContextID=\"0\"\r\n");

    // section 2.3.1.14 ProjectVersionCompat32
    exportString(rStrm, "VersionCompatible32=\"393222000\"\r\n");

    // section 2.3.1.15 ProjectProtectionState
#if VBA_ENCRYPTION
    exportString(rStrm, "CMG=\"");
    SvMemoryStream aProtectedStream(4096, 4096);
    aProtectedStream.WriteUInt32(0x00000000);
    const sal_uInt8* pData = static_cast<const sal_uInt8*>(aProtectedStream.GetData());
    sal_uInt8 nProjKey = VBAEncryption::calculateProjKey(aProjectID);
    VBAEncryption aProtectionState(pData, 4, rStrm, nProjKey);
    aProtectionState.write();
    exportString(rStrm, "\"\r\n");
#else
    exportString(rStrm, "CMG=\"BEBC9256EEAAA8AEA8AEA8AEA8AE\"\r\n");
#endif

    // section 2.3.1.16 ProjectPassword
#if VBA_ENCRYPTION
    exportString(rStrm, "DPB=\"");
    aProtectedStream.Seek(0);
    aProtectedStream.WriteUInt8(0x00);
    pData = static_cast<const sal_uInt8*>(aProtectedStream.GetData());
    VBAEncryption aProjectPassword(pData, 1, rStrm, nProjKey);
    aProjectPassword.write();
    exportString(rStrm, "\"\r\n");
#else
    exportString(rStrm, "DPB=\"7C7E5014B0D3B1D3B1D3\"\r\n");
#endif

    // section 2.3.1.17 ProjectVisibilityState
#if VBA_ENCRYPTION
    exportString(rStrm, "GC=\"");
    aProtectedStream.Seek(0);
    aProtectedStream.WriteUInt8(0xFF);
    pData = static_cast<const sal_uInt8*>(aProtectedStream.GetData());
    VBAEncryption aVisibilityState(pData, 1, rStrm, nProjKey);
    aVisibilityState.write();
    exportString(rStrm, "\"\r\n\r\n");
#else
    exportString(rStrm, "GC=\"3A3816DAD5DBD5DB2A\"\r\n\r\n");
#endif

    // section 2.3.1.18 HostExtenders
    exportString(rStrm, "[Host Extender Info]\r\n"
                        "&H00000001={3832D640-CF90-11CF-8E43-00A0C911005A};VBE;&H00000000\r\n\r\n"
    );

    // section 2.3.1.19 ProjectWorkspace
    exportString(rStrm, "[Workspace]\r\n");
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rModuleName = aElementNames[rLibraryMap[i]];
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(rModuleName);
        if(aModuleInfo.ModuleType == 1)
        {
            exportString(rStrm,  rModuleName + "=25, 25, 1439, 639, \r\n");
        }
        else
        {
            exportString(rStrm, rModuleName + "=0, 0, 0, 0, C\r\n");
        }
    }
}

// section 2.3.3.1 NAMEMAP
void writeNAMEMAP(SvStream& rStrm, const css::uno::Sequence<OUString>& rElementNames,
        const std::vector<sal_Int32>& rLibraryMap)
{
    int n = rElementNames.getLength();
    for(sal_Int32 i = 0; i < n; ++i)
    {
        const OUString& rModuleName = rElementNames[rLibraryMap[i]];
        exportString(rStrm, rModuleName);
        rStrm.WriteUInt8(0x00); // terminator
        exportUTF16String(rStrm, rModuleName);
        rStrm.WriteUInt16(0x0000); // terminator
    }
}

// section 2.3.3 PROJECTwm Stream
void exportPROJECTwmStream(SvStream& rStrm, const css::uno::Sequence<OUString>& rElementNames,
        const std::vector<sal_Int32>& rLibraryMap)
{
    writeNAMEMAP(rStrm, rElementNames, rLibraryMap);
    rStrm.WriteUInt16(0x0000); // terminator
}

void getCorrectExportOrder(const css::uno::Reference<css::container::XNameContainer>& xNameContainer, std::vector<sal_Int32>& rLibraryMap)
{
    const css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength();
    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);

    sal_Int32 nCurrentId = 0;
    // first all the non-document modules
    for (sal_Int32 i = 0; i < n; ++i)
    {
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        if (aModuleInfo.ModuleType != 4)
        {
            rLibraryMap[nCurrentId] = i;
            ++nCurrentId;
        }
    }

    sal_Int32 nWorkbookIndex = -1;
    // then possibly the workbook module
    for (sal_Int32 i = 0; i < n; ++i)
    {
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        bool bWorkbook = isWorkbook(aModuleInfo.ModuleObject);
        if (bWorkbook)
        {
            nWorkbookIndex = i;
            rLibraryMap[nCurrentId] = i;
            ++nCurrentId;
        }
    }

    // then the remaining modules
    for (sal_Int32 i = 0; i < n; ++i)
    {
        if (i == nWorkbookIndex)
            continue;

        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        if (aModuleInfo.ModuleType == 4)
        {
            rLibraryMap[nCurrentId] = i;
            ++nCurrentId;
        }
    }
}

}

#if VBA_USE_ORIGINAL_WM_STREAM || VBA_USE_ORIGINAL_DIR_STREAM \
    || VBA_USE_ORIGINAL_PROJECT_STREAM || VBA_USE_ORIGINAL_VBA_PROJECT \
    || VBA_USE_ORIGINAL_DIR_STREAM
void addFileStreamToSotStream(const OUString& rPath, SotStorageStream* pStream)
{
    SvFileStream aFileStream(rPath, StreamMode::READWRITE);
    pStream->WriteStream(aFileStream);
}
#endif

void VbaExport::exportVBA(SotStorage* pRootStorage)
{
    css::uno::Reference<css::container::XNameContainer> xNameContainer = getBasicLibrary();
    if (!xNameContainer.is()) {
        return;
    }
    const css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength(); // get the number of modules
    // export the elements in the order MSO expects them
    // we store the index of the
    std::vector<sal_Int32> aLibraryMap(n, 0);
    getCorrectExportOrder(xNameContainer, aLibraryMap);

    // start here with the VBA export
    tools::SvRef<SotStorage> xVBAStream = pRootStorage->OpenSotStorage("VBA", StreamMode::READWRITE);
    SotStorageStream* pDirStream = xVBAStream->OpenSotStream("dir", StreamMode::READWRITE);

    SotStorageStream* pVBAProjectStream = xVBAStream->OpenSotStream("_VBA_PROJECT", StreamMode::READWRITE);
    SotStorageStream* pPROJECTStream = pRootStorage->OpenSotStream("PROJECT", StreamMode::READWRITE);
    SotStorageStream* pPROJECTwmStream = pRootStorage->OpenSotStream("PROJECTwm", StreamMode::READWRITE);

#if VBA_USE_ORIGINAL_WM_STREAM
    OUString aProjectwmPath = "/home/moggi/Documents/testfiles/vba/PROJECTwm";
    addFileStreamToSotStream(aProjectwmPath, pPROJECTwmStream);
#else
    exportPROJECTwmStream(*pPROJECTwmStream, aElementNames, aLibraryMap);
#endif

#if VBA_USE_ORIGINAL_DIR_STREAM
    OUString aDirPath = "/home/moggi/Documents/testfiles/vba/VBA/dir";
    addFileStreamToSotStream(aDirPath, pDirStream);
#else
    exportDirStream(*pDirStream, xNameContainer, aLibraryMap, getProjectName());
#endif

#if VBA_USE_ORIGINAL_PROJECT_STREAM
    OUString aProjectPath = "/home/moggi/Documents/testfiles/vba/PROJECT";
    addFileStreamToSotStream(aProjectPath, pPROJECTStream);
#else
    exportPROJECTStream(*pPROJECTStream, xNameContainer, getProjectName(), aLibraryMap);
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
    SotStorageStream* pModule1Stream = xVBAStream->OpenSotStream("Module1", StreamMode::READWRITE);
    SotStorageStream* pSheet1Stream = xVBAStream->OpenSotStream("Sheet1", StreamMode::READWRITE);
    SotStorageStream* pSheet2Stream = xVBAStream->OpenSotStream("Sheet2", StreamMode::READWRITE);
    SotStorageStream* pSheet3Stream = xVBAStream->OpenSotStream("Sheet3", StreamMode::READWRITE);
    SotStorageStream* pWorkbookStream = xVBAStream->OpenSotStream("ThisWorkbook", StreamMode::READWRITE);
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
        const OUString& rModuleName = aElementNames[aLibraryMap[i]];
        SotStorageStream* pModuleStream = xVBAStream->OpenSotStream(rModuleName, StreamMode::READWRITE);
        css::uno::Any aCode = xNameContainer->getByName(rModuleName);
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(rModuleName);
        OUString aSourceCode;
        aCode >>= aSourceCode;
        exportModuleStream(*pModuleStream, aSourceCode, rModuleName, aModuleInfo);
        pModuleStream->Commit();
    }

#endif

    pVBAProjectStream->Commit();

    pDirStream->Commit();
    xVBAStream->Commit();
    pPROJECTStream->Commit();
    pPROJECTwmStream->Commit();
    pRootStorage->Commit();
}

css::uno::Reference<css::script::XLibraryContainer> VbaExport::getLibraryContainer() const
{
    oox::PropertySet aDocProp(mxModel);
    css::uno::Reference<css::script::XLibraryContainer> xLibContainer(aDocProp.getAnyProperty(oox::PROP_BasicLibraries), css::uno::UNO_QUERY);

    return xLibContainer;
}

css::uno::Reference<css::container::XNameContainer> VbaExport::getBasicLibrary() const
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

    bool bVBACompatibility = xVbaCompatibility->getVBACompatibilityMode();

    return bVBACompatibility;
}

OUString VbaExport::getProjectName() const
{
    css::uno::Reference<css::script::vba::XVBACompatibility> xVbaCompatibility(getLibraryContainer(), css::uno::UNO_QUERY);
    if (xVbaCompatibility.is())
        return xVbaCompatibility->getProjectName();

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
