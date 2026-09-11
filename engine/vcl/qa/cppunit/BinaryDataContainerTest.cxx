/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <comphelper/hash.hxx>

#include <vcl/BinaryDataContainer.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/graph.hxx>

#include <impgraph.hxx>

#include "KitModeScope.hxx"

using namespace css;

namespace
{
class BinaryDataContainerTest : public CppUnit::TestFixture
{
    void testConstruct();
    void testSizeHolderCount();
    void testASwappedOutContainerStillAnswersItsSize();
    void testBytesComeBackAfterSwappingOut();
    void testAStreamWithTooFewBytesLeavesTheContainerEmpty();

    CPPUNIT_TEST_SUITE(BinaryDataContainerTest);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testSizeHolderCount);
    CPPUNIT_TEST(testASwappedOutContainerStillAnswersItsSize);
    CPPUNIT_TEST(testBytesComeBackAfterSwappingOut);
    CPPUNIT_TEST(testAStreamWithTooFewBytesLeavesTheContainerEmpty);
    CPPUNIT_TEST_SUITE_END();
};

void BinaryDataContainerTest::testConstruct()
{
    {
        BinaryDataContainer aContainer;
        CPPUNIT_ASSERT(aContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSize());
    }
    {
        // construct a data array
        sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
        SvMemoryStream stream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);

        BinaryDataContainer aContainer(stream, std::size(aTestByteArray));

        CPPUNIT_ASSERT(!aContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aContainer.getSize());

        // Test Copy
        BinaryDataContainer aCopyOfContainer = aContainer;
        CPPUNIT_ASSERT(!aCopyOfContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aCopyOfContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aCopyOfContainer.getData(), aContainer.getData());

        // Test Move
        BinaryDataContainer aMovedInContainer = std::move(aCopyOfContainer);
        CPPUNIT_ASSERT(!aMovedInContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(4), aMovedInContainer.getSize());
        CPPUNIT_ASSERT_EQUAL(aMovedInContainer.getData(), aContainer.getData());

        CPPUNIT_ASSERT(aCopyOfContainer.isEmpty());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aCopyOfContainer.getSize());
    }
}

// the size holder count follows the registered holders, not the number of container copies
void BinaryDataContainerTest::testSizeHolderCount()
{
    {
        BinaryDataContainer aEmpty;
        CPPUNIT_ASSERT_EQUAL(size_t(0), aEmpty.getSizeHolderCount());
        aEmpty.addSizeHolder();
        CPPUNIT_ASSERT_EQUAL(size_t(0), aEmpty.getSizeHolderCount());
    }

    sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
    SvMemoryStream aStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);
    BinaryDataContainer aContainer(aStream, std::size(aTestByteArray));
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSizeHolderCount());

    // copies alone do not count as holders
    BinaryDataContainer aCopy = aContainer;
    CPPUNIT_ASSERT_EQUAL(size_t(0), aCopy.getSizeHolderCount());

    // a holder registered through one copy is visible through every copy of the same bytes
    aContainer.addSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(1), aContainer.getSizeHolderCount());
    CPPUNIT_ASSERT_EQUAL(size_t(1), aCopy.getSizeHolderCount());

    aCopy.addSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aContainer.getSizeHolderCount());

    aContainer.removeSizeHolder();
    aCopy.removeSizeHolder();
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSizeHolderCount());
    CPPUNIT_ASSERT_EQUAL(size_t(0), aCopy.getSizeHolderCount());
}

// A swapped out container still answers how big it is, and answering leaves the bytes on disk.
void BinaryDataContainerTest::testASwappedOutContainerStillAnswersItsSize()
{
    KitModeScope aKitMode;

    sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
    SvMemoryStream aStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);

    // A graphic is the shortest way to a container that can be told to swap out.
    auto pGfxLink = std::make_shared<GfxLink>(
        BinaryDataContainer(aStream, std::size(aTestByteArray)), GfxLinkType::NativePng);
    Graphic aGraphic;
    aGraphic.SetGfxLink(pGfxLink);

    const BinaryDataContainer& rContainer = pGfxLink->getDataContainer();
    const size_t nHashInMemory = rContainer.calculateHash();
    CPPUNIT_ASSERT(nHashInMemory != 0);

    CPPUNIT_ASSERT(aGraphic.ImplGetImpGraphic()->swapOut());

    // Hashing only reaches bytes that are in memory, so it now finds nothing to hash.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rContainer.calculateHash());

    CPPUNIT_ASSERT_EQUAL(size_t(4), rContainer.getSize());
    CPPUNIT_ASSERT(!rContainer.isEmpty());

    // Both answers came from the remembered size rather than from reading the file back.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rContainer.calculateHash());

    // Asking for the bytes themselves does read them back.
    CPPUNIT_ASSERT(rContainer.getData() != nullptr);
    CPPUNIT_ASSERT_EQUAL(nHashInMemory, rContainer.calculateHash());
}

// Every way of asking a swapped out container for its bytes brings them back from the temporary
// file and hands over all of them.
void BinaryDataContainerTest::testBytesComeBackAfterSwappingOut()
{
    KitModeScope aKitMode;

    sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
    SvMemoryStream aSourceStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);

    // A graphic is the shortest way to a container that can be told to swap out.
    auto pGfxLink = std::make_shared<GfxLink>(
        BinaryDataContainer(aSourceStream, std::size(aTestByteArray)), GfxLinkType::NativePng);
    Graphic aGraphic;
    aGraphic.SetGfxLink(pGfxLink);

    const BinaryDataContainer& rContainer = pGfxLink->getDataContainer();

    // The digest of the same bytes in a container that never left memory, to compare against.
    SvMemoryStream aSameBytesStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);
    const BinaryDataContainer aContainerInMemory(aSameBytesStream, std::size(aTestByteArray));
    const std::string aDigestInMemory
        = comphelper::hashToString(aContainerInMemory.calculateSHA1());

    CPPUNIT_ASSERT(aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(size_t(0), rContainer.calculateHash());

    SvMemoryStream aCopyStream;
    CPPUNIT_ASSERT_EQUAL(size_t(4), rContainer.writeToStream(aCopyStream));
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(4), aCopyStream.TellEnd());

    aCopyStream.Seek(STREAM_SEEK_TO_BEGIN);
    for (sal_uInt8 nExpected : aTestByteArray)
    {
        sal_uInt8 nByte = 0;
        aCopyStream.ReadUChar(nByte);
        CPPUNIT_ASSERT_EQUAL(nExpected, nByte);
    }

    // Each of the other ways of asking starts from the bytes being in the temporary file again.
    CPPUNIT_ASSERT(aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(size_t(0), rContainer.calculateHash());

    const cpo::uno::Sequence<sal_Int8> aByteSequence = rContainer.getCopyAsByteSequence();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aByteSequence.getLength());
    for (sal_Int32 nIndex = 0; nIndex < aByteSequence.getLength(); ++nIndex)
        CPPUNIT_ASSERT_EQUAL(sal_Int8(aTestByteArray[nIndex]), aByteSequence[nIndex]);

    CPPUNIT_ASSERT(aGraphic.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(size_t(0), rContainer.calculateHash());

    // The digest is taken over the bytes themselves, so it comes out the same as for the container
    // that was never swapped out.
    CPPUNIT_ASSERT_EQUAL(aDigestInMemory, comphelper::hashToString(rContainer.calculateSHA1()));
}

// A stream that holds fewer bytes than the container was told to take leaves the container empty
// and its size at zero.
void BinaryDataContainerTest::testAStreamWithTooFewBytesLeavesTheContainerEmpty()
{
    sal_uInt8 aTestByteArray[] = { 1, 2, 3, 4 };
    SvMemoryStream aStream(aTestByteArray, std::size(aTestByteArray), StreamMode::READ);

    BinaryDataContainer aContainer(aStream, std::size(aTestByteArray) + 1);

    CPPUNIT_ASSERT(aContainer.isEmpty());
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.getSize());
    CPPUNIT_ASSERT_EQUAL(size_t(0), aContainer.calculateHash());
    CPPUNIT_ASSERT(aContainer.getData() == nullptr);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(BinaryDataContainerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
