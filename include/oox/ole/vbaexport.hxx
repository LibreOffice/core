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

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace frame { class XModel; }
} } }

class OOX_DLLPUBLIC VbaExport
{
public:
    VbaExport(css::uno::Reference<css::frame::XModel> xModel);

    void exportVBA();

private:

    css::uno::Reference<css::container::XNameContainer>
        getBasicLibrary();

    css::uno::Reference<css::frame::XModel> mxModel;

    OUString maProjectName;
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

    void PackCompressedChunkSize(size_t nSize, sal_uInt16& rHeader);

    void PackCompressedChunkFlag(bool bCompressed, sal_uInt16& rHeader);

    void PackCompressedChunkSignature(sal_uInt16& rHeader);

    void compressTokenSequence();

    void compressToken(size_t index, sal_uInt8& nFlagByte);

    void SetFlagBit(size_t index, bool bVal, sal_uInt8& rFlag);

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
