/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/ole/vbaexport.hxx>

#include <tools/stream.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <oox/helper/binaryoutputstream.hxx>
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

#include <sot/storage.hxx>

#define USE_UTF8_CODEPAGE 0
#if USE_UTF8_CODEPAGE
#define CODEPAGE_MS 65001
#define CODEPAGE RTL_TEXTENCODING_UTF8
#else
#define CODEPAGE_MS 1252
#define CODEPAGE RTL_TEXTENCODING_MS_1252
#endif

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

VBACompressionChunk::VBACompressionChunk(SvStream& rCompressedStream, const sal_uInt8* pData, sal_Size nChunkSize):
    mrCompressedStream(rCompressedStream),
    mpUncompressedData(pData),
    mnChunkSize(nChunkSize)
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
            SetFlagBit(index, 1, nFlagByte);
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
    sal_uInt16 nBitCount = std::ceil(std::log2(nDifference));
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
    const sal_uInt8* pData = (const sal_uInt8*)mrUncompressedStream.GetData();
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
    // TODO: how do we get the correct project name
    maProjectName = "VBAProject";
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
void writeREFERENCENAME(SvStream& rStrm, const OUString name)
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
void writeREFERENCEREGISTERED(SvStream& rStrm, const OUString libid)
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
void writeREFERENCE(SvStream& rStrm, const OUString name, const OUString libid)
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
void writeMODULENAME(SvStream& rStrm, const OUString name)
{
    rStrm.WriteUInt16(0x0019); // id
    sal_Int32 n = name.getLength(); // sizeOfModuleName
    rStrm.WriteUInt32(n);
    exportString(rStrm, name); // ModuleName
}

// section 2.3.4.2.3.2.2
void writeMODULENAMEUNICODE(SvStream& rStrm, const OUString name)
{
    rStrm.WriteUInt16(0x0047); // id
    sal_Int32 n = name.getLength() * 2; // sizeOfModuleNameUnicode // TODO: better calculation for unicode string length
    rStrm.WriteUInt32(n);
    exportUTF16String(rStrm, name); // ModuleNameUnicode
}

// section 2.3.4.2.3.2.3
void writeMODULESTREAMNAME(SvStream& rStrm, const OUString streamName)
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
void writeMODULETYPE(SvStream& rStrm, const OUString type)
{
    if(type == "procedure")
        rStrm.WriteUInt16(0x0021); // id for a procedural module
    else
        rStrm.WriteUInt16(0x0022); // id for document, class or design module
    rStrm.WriteUInt32(0x00000000); // reserved
}

// section 2.3.4.2.3.2
void writePROJECTMODULE(SvStream& rStrm, const OUString name, const OUString streamName, sal_uInt32 offset, const OUString type)
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
void writePROJECTMODULES(SvStream& rStrm)
{
    // TODO: this whole part is document specific
    rStrm.WriteUInt16(0x000F); // id
    rStrm.WriteUInt32(0x00000002); // size of Count
    sal_Int16 count = 5; // Number of modules // TODO: this is dependent on the document
    rStrm.WriteUInt16(count); // Count
    writePROJECTCOOKIE(rStrm);
    writePROJECTMODULE(rStrm, "Module1", "Module1", 0x00000379, "procedure");
    writePROJECTMODULE(rStrm, "ThisWorkbook", "ThisWorkbook", 0x00000325, "other");
    writePROJECTMODULE(rStrm, "Sheet1", "Sheet1", 0x00000325, "other");
    writePROJECTMODULE(rStrm, "Sheet2", "Sheet2", 0x00000325, "other");
    writePROJECTMODULE(rStrm, "Sheet3", "Sheet3", 0x00000325, "other");
}

// section 2.3.4.2
void exportDirStream(SvStream& rStrm)
{
    writePROJECTINFORMATION(rStrm);
    writePROJECTREFERENCES(rStrm);
    writePROJECTMODULES(rStrm);
    rStrm.WriteUInt16(0x0010); // terminator
    rStrm.WriteUInt32(0x00000000); // reserved
}


void exportThisWorkbookStream(SvStream& rStrm)
{
    const OUString thisWorkbookStream = "Attribute VB_Name = \"ThisWorkbook\"\r\n"
                                        "Attribute VB_Base = \"0{00020819-0000-0000-C000-000000000046}\"\r\n"
                                        "Attribute VB_GlobalNameSpace = False\r\n"
                                        "Attribute VB_Creatable = False\r\n"
                                        "Attribute VB_PredeclaredId = True\r\n"
                                        "Attribute VB_Exposed = True\r\n"
                                        "Attribute VB_TemplateDerived = False\r\n"
                                        "Attribute VB_Customizable = True\r\n";
    exportString(rStrm, thisWorkbookStream);
}

void exportVBAProjectStream(SvStream& rStrm)
{
    rStrm.WriteUInt16(0x61CC); // Reserved1
    rStrm.WriteUInt16(0xFFFF); // Version
    rStrm.WriteUInt8(0x00); // Reserved2
    rStrm.WriteUInt16(0x0000); // Undefined
}
}

void VbaExport::exportVBA()
{
    // start here with the VBA export

    const OUString aVbaStreamLocation("/tmp/vba_out.bin");
    SvFileStream aVbaStream(aVbaStreamLocation, STREAM_READWRITE);

    tools::SvRef<SotStorage> aStorage(new SotStorage(aVbaStream));
    SotStorage* pVBAStream = aStorage->OpenSotStorage("VBA", STREAM_READWRITE);
    SotStorageStream* pDirStream = pVBAStream->OpenSotStream("dir", STREAM_READWRITE);

    const OUString aDirFileName("/tmp/vba_dir_out.bin");
    SvFileStream aDirStream(aDirFileName, STREAM_READWRITE);

    const OUString aThisWorkbookFileName("/tmp/vba_workbook_out.bin");
    SvFileStream aThisWorkbookStream(aThisWorkbookFileName, STREAM_READWRITE);

    const OUString aVBAProjectFileName("/tmp/vba_project_stream_out.bin");
    SvFileStream aVBAProjectStream(aVBAProjectFileName, STREAM_READWRITE);

    // export
    exportDirStream(aDirStream);
    exportThisWorkbookStream(aThisWorkbookStream);
    exportVBAProjectStream(aVBAProjectStream);

    aDirStream.Seek(0);

    SvMemoryStream aMemoryStream(4096, 4096);
    aMemoryStream.WriteStream(aDirStream);

    VBACompression aCompression(*pDirStream, aMemoryStream);
    aCompression.write();

    css::uno::Reference<css::container::XNameContainer> xNameContainer = getBasicLibrary();
    css::uno::Sequence<OUString> aElementNames = xNameContainer->getElementNames();
    sal_Int32 n = aElementNames.getLength();
    css::uno::Reference<css::script::vba::XVBAModuleInfo> xModuleInfo(xNameContainer, css::uno::UNO_QUERY);
    assert(xModuleInfo.is());
    for (sal_Int32 i = 0; i < n; ++i)
    {
        SAL_DEBUG(aElementNames[i]);
        css::script::ModuleInfo aModuleInfo = xModuleInfo->getModuleInfo(aElementNames[i]);
        SAL_DEBUG(aModuleInfo.ModuleType);
    }
    pDirStream->Commit();
    pVBAStream->Commit();
    aStorage->Commit();
}

css::uno::Reference<css::container::XNameContainer> VbaExport::getBasicLibrary()
{
    css::uno::Reference<css::container::XNameContainer> xLibrary;
    try
    {
        oox::PropertySet aDocProp(mxModel);
        css::uno::Reference<css::script::XLibraryContainer> xLibContainer(aDocProp.getAnyProperty(oox::PROP_BasicLibraries), css::uno::UNO_QUERY_THROW);
        css::uno::Sequence<OUString> aElementNames = xLibContainer->getElementNames();
        sal_Int32 n = aElementNames.getLength();
        for (sal_Int32 i = 0; i < n; ++i)
        {
            SAL_DEBUG(aElementNames[i]);
        }
        xLibrary.set( xLibContainer->getByName(maProjectName), css::uno::UNO_QUERY_THROW );
    }
    catch(...)
    {
    }

    return xLibrary;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
