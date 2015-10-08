/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __INCLUDED_INCLUDE_OOX_OLE_VBAEXPORT_HXX__
#define __INCLUDED_INCLUDE_OOX_OLE_VBAEXPORT_HXX__

#include <com/sun/star/uno/XInterface.hpp>

#include <tools/stream.hxx>

#include <oox/dllapi.h>

class SotStorage;

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace frame { class XModel; }
    namespace script { class XLibraryContainer; }
} } }

class OOX_DLLPUBLIC VbaExport
{
public:
    VbaExport(css::uno::Reference<css::frame::XModel> xModel);

    void exportVBA(SotStorage* pRootStorage);

    bool containsVBAProject();

private:

    css::uno::Reference<css::container::XNameContainer>
        getBasicLibrary();

    css::uno::Reference<css::script::XLibraryContainer>
        getLibraryContainer();

    OUString getProjectName();

    css::uno::Reference<css::frame::XModel> mxModel;
};

class VBACompressionChunk
{
public:

    VBACompressionChunk(SvStream& rCompressedStream, const sal_uInt8* pData, sal_Size nChunkSize);

    void write();

private:
    SvStream& mrCompressedStream;
    const sal_uInt8* mpUncompressedData;
    sal_uInt8* mpCompressedChunkStream;

    // same as DecompressedChunkEnd in the spec
    sal_Size mnChunkSize;

    // CompressedCurrent according to the spec
    sal_uInt64 mnCompressedCurrent;

    // CompressedEnd according to the spec
    sal_uInt64 mnCompressedEnd;

    // DecompressedCurrent according to the spec
    sal_uInt64 mnDecompressedCurrent;

    // DecompressedEnd according to the spec
    sal_uInt64 mnDecompressedEnd;

    static void PackCompressedChunkSize(size_t nSize, sal_uInt16& rHeader);

    static void PackCompressedChunkFlag(bool bCompressed, sal_uInt16& rHeader);

    static void PackCompressedChunkSignature(sal_uInt16& rHeader);

    void compressTokenSequence();

    void compressToken(size_t index, sal_uInt8& nFlagByte);

    static void SetFlagBit(size_t index, bool bVal, sal_uInt8& rFlag);

    sal_uInt16 CopyToken(size_t nLength, size_t nOffset);

    void match(size_t& rLength, size_t& rOffset);

    void CopyTokenHelp(sal_uInt16& rLengthMask, sal_uInt16& rOffsetMask,
            sal_uInt16& rBitCount, sal_uInt16& rMaximumLength);

    void writeRawChunk();

    sal_uInt16 handleHeader(bool bCompressed);
};

class OOX_DLLPUBLIC VBACompression
{
public:
    VBACompression(SvStream& rCompressedStream,
            SvMemoryStream& rUncompressedStream);

    void write();

private:
    SvStream& mrCompressedStream;
    SvMemoryStream& mrUncompressedStream;
};

class OOX_DLLPUBLIC VBAEncryption
{
public:
    VBAEncryption(const sal_uInt8* pData,
                  const sal_uInt16 nLength,
                  SvStream& rEncryptedData,
                  sal_uInt8* pSeed,
                  sal_uInt8 nProjKey);

    void write();

    static sal_uInt8 calculateProjKey(const OUString& rString);

private:
    const sal_uInt8* mpData; // an array of bytes to be obfuscated
    const sal_uInt16 mnLength; // the length of Data
    SvStream& mrEncryptedData; // Encrypted Data Structure
    sal_uInt8 mnUnencryptedByte1; // the last unencrypted byte read or written
    sal_uInt8 mnEncryptedByte1; // the last encrypted byte read or written
    sal_uInt8 mnEncryptedByte2; // the next-to-last encrypted byte read or written
    sal_uInt8 mnVersion; // the encrypted version
    sal_Unicode mnProjKey; // a project-specific encryption key
    sal_uInt8 mnIgnoredLength; // the length in bytes of IgnoredEnc

    sal_uInt8 mnSeed; // the seed value
    sal_uInt8 mnVersionEnc; // the version encoding

    void writeSeed();
    void writeVersionEnc();
    void writeProjKeyEnc();
    void writeIgnoredEnc();
    void writeDataLengthEnc();
    void writeDataEnc();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
