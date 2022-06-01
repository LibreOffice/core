/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <rtl/crc.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
// Sample text for compression
constexpr const char* DUMMY_TEXT
    = "Lorem ipsum dolor sit amet, consectetur adipiscing elit,\n"
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
      "commodo consequat.\n";
constexpr auto DUMMY_SIZE = std::char_traits<char>::length(DUMMY_TEXT);

class ZCodecTest : public CppUnit::TestFixture
{
private:
    void testGzCompressDecompress();
    void testMakeDummyFile();
    void testZlibCompressDecompress();

    CPPUNIT_TEST_SUITE(ZCodecTest);
    CPPUNIT_TEST(testMakeDummyFile);
    CPPUNIT_TEST(testGzCompressDecompress);
    CPPUNIT_TEST(testZlibCompressDecompress);
    CPPUNIT_TEST_SUITE_END();
};

// Creates a stream from DUMMY_TEXT to compress and decompress
std::unique_ptr<SvMemoryStream> makeDummyFile()
{
    SvMemoryStream* pRet = new SvMemoryStream();
    pRet->WriteBytes(DUMMY_TEXT, DUMMY_SIZE);
    return std::unique_ptr<SvMemoryStream>(pRet);
}

// Test that the stream generated from makeDummyFile is valid
void ZCodecTest::testMakeDummyFile()
{
    auto pStream = makeDummyFile();
    // Check for null
    CPPUNIT_ASSERT_MESSAGE("pStream is null", pStream);
    decltype(DUMMY_SIZE) size = pStream->GetSize();
    // Check size
    CPPUNIT_ASSERT_EQUAL(DUMMY_SIZE, size);
}

// Test that ZCodec::Compress and ZCodec::Decompress work for gzlib = true
// Compares the CRC32 checksum of the initial stream and the compressed->decompressed stream
void ZCodecTest::testGzCompressDecompress()
{
    auto pInitialStream = makeDummyFile();
    SvMemoryStream pCompressedStream;
    SvMemoryStream pDecompressedStream;
    auto nInitialStreamCRC32 = rtl_crc32(0, pInitialStream->GetData(), pInitialStream->GetSize());
    ZCodec aCodec;
    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, true);
    // Set compression metadata for compressing a GZ file
    aCodec.SetCompressionMetadata("dummy.txt", 0, nInitialStreamCRC32);
    aCodec.Compress(*pInitialStream, pCompressedStream);
    auto nCompressedSize
        = aCodec.EndCompression(); // returns compressed size or -1 if compression failed
    // Check that the compression succeeded
    CPPUNIT_ASSERT_GREATER(static_cast<tools::Long>(0), nCompressedSize);
    // No need to Seek(0) here because ZCodec::InitDecompress does that already for gz
    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, true);
    aCodec.Decompress(pCompressedStream, pDecompressedStream);
    auto nDecompressedSize
        = aCodec.EndCompression(); // returns decompressed size or -1 if compression failed
    // Check that the decompressed text size is equal to the original
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(DUMMY_SIZE), nDecompressedSize);
    auto nCompressedDecompressedStreamCRC32
        = rtl_crc32(0, pDecompressedStream.GetData(), pDecompressedStream.GetSize());
    // Check that the initial and decompressed CRC32 checksums are the same -> gz (de)compression works
    CPPUNIT_ASSERT_EQUAL(nInitialStreamCRC32, nCompressedDecompressedStreamCRC32);
}

// Test that ZCodec::Compress and ZCodec::Decompress work for gzlib = false
// Compares the CRC32 checksum of the initial stream and the compressed->decompressed stream
void ZCodecTest::testZlibCompressDecompress()
{
    auto pInitialStream = makeDummyFile();
    SvMemoryStream pCompressedStream;
    SvMemoryStream pDecompressedStream;
    auto nInitialStreamCRC32 = rtl_crc32(0, pInitialStream->GetData(), pInitialStream->GetSize());
    ZCodec aCodec;
    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false);
    aCodec.Compress(*pInitialStream, pCompressedStream);
    auto nCompressedSize
        = aCodec.EndCompression(); // returns compressed size or -1 if compression failed
    // Check that the compression succeeded
    CPPUNIT_ASSERT_GREATER(static_cast<tools::Long>(0), nCompressedSize);
    pCompressedStream.Seek(0);
    aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false);
    aCodec.Decompress(pCompressedStream, pDecompressedStream);
    auto nDecompressedSize
        = aCodec.EndCompression(); // returns decompressed size or -1 if compression failed
    // Check that the decompressed text size is equal to the original
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(DUMMY_SIZE), nDecompressedSize);
    auto nCompressedDecompressedStreamCRC32
        = rtl_crc32(0, pDecompressedStream.GetData(), pDecompressedStream.GetSize());
    // Check that the initial and decompressed CRC32 checksums are the same -> zlib (de)compression works
    CPPUNIT_ASSERT_EQUAL(nInitialStreamCRC32, nCompressedDecompressedStreamCRC32);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(ZCodecTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
