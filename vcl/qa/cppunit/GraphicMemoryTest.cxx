/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/BitmapReadAccess.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <tools/stream.hxx>

#include <impgraph.hxx>
#include <graphic/GraphicFormatDetector.hxx>
#include <graphic/MemoryManaged.hxx>

using namespace css;

namespace
{
BitmapEx createBitmap(Size aSize, bool bAlpha = false)
{
    Bitmap aBitmap(aSize, vcl::PixelFormat::N24_BPP);
    aBitmap.Erase(COL_LIGHTRED);

    aBitmap.SetPrefSize(Size(aSize.Width() * 2, aSize.Height() * 3));
    aBitmap.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

    if (bAlpha)
    {
        sal_uInt8 uAlphaValue = 0x80;
        AlphaMask aAlphaMask(aSize, &uAlphaValue);

        return BitmapEx(aBitmap, aAlphaMask);
    }
    else
    {
        return BitmapEx(aBitmap);
    }
}

void createBitmapAndExportForType(SvStream& rStream, std::u16string_view sType,
                                  Size aSize = Size(120, 100), bool bAlpha = false)
{
    BitmapEx aBitmapEx = createBitmap(aSize, bAlpha);

    uno::Sequence<beans::PropertyValue> aFilterData;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilterFormat = rGraphicFilter.GetExportFormatNumberForShortName(sType);
    rGraphicFilter.ExportGraphic(aBitmapEx, u"none", rStream, nFilterFormat, &aFilterData);

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
}

Graphic makeUnloadedGraphic(std::u16string_view sType, Size aSize = Size(120, 100),
                            bool bAlpha = false)
{
    SvMemoryStream aStream;
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    createBitmapAndExportForType(aStream, sType, aSize, bAlpha);
    return rGraphicFilter.ImportUnloadedGraphic(aStream);
}

constexpr OUString DATA_DIRECTORY = u"/vcl/qa/cppunit/data/"_ustr;

Graphic loadGraphic(std::u16string_view rFilename)
{
    test::Directories aDirectories;
    OUString aFilename = aDirectories.getURLFromSrc(DATA_DIRECTORY) + rFilename;
    SvFileStream aFileStream(aFilename, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();

    Graphic aGraphic;
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, rGraphicFilter.ImportGraphic(aGraphic, u"", aFileStream,
                                                                    GRFILTER_FORMAT_DONTKNOW));
    return aGraphic;
}

Graphic loadUnloadedGraphic(std::u16string_view rFilename)
{
    test::Directories aDirectories;
    OUString aFilename = aDirectories.getURLFromSrc(DATA_DIRECTORY) + rFilename;
    SvFileStream aFileStream(aFilename, StreamMode::READ);
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    return rGraphicFilter.ImportUnloadedGraphic(aFileStream);
}

} // end anonymous

class GraphicMemoryTest : public test::BootstrapFixture
{
};

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManager_Empty)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic1;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic2;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic3;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    aGraphic1 = aGraphic2;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    aGraphic2 = aGraphic3;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());
}

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManager_Unloaded)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic_100 = makeUnloadedGraphic(u"jpg", Size(100, 100));
    Graphic aGraphic_200 = makeUnloadedGraphic(u"png", Size(200, 100));
    Graphic aGraphic_300 = makeUnloadedGraphic(u"jpg", Size(300, 100));

    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    aGraphic_100.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(30000), rManager.getTotalSize());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(30000), aGraphic_100.GetSizeBytes());

    aGraphic_200.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(90000), rManager.getTotalSize());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(30000), aGraphic_100.GetSizeBytes());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(60000), aGraphic_200.GetSizeBytes());

    aGraphic_300.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(size_t(3), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), rManager.getTotalSize());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(30000), aGraphic_100.GetSizeBytes());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(60000), aGraphic_200.GetSizeBytes());
    CPPUNIT_ASSERT_EQUAL(sal_uLong(90000), aGraphic_300.GetSizeBytes());
}

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManager_Destruction)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic_100 = makeUnloadedGraphic(u"jpg", Size(100, 100));
    Graphic aGraphic_200 = makeUnloadedGraphic(u"png", Size(200, 100));
    Graphic aGraphic_300 = makeUnloadedGraphic(u"jpg", Size(300, 100));

    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    aGraphic_100.makeAvailable();
    aGraphic_200.makeAvailable();
    aGraphic_300.makeAvailable();
    CPPUNIT_ASSERT_EQUAL(size_t(3), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(180000), rManager.getTotalSize());

    aGraphic_200 = Graphic();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(120000), rManager.getTotalSize());

    aGraphic_300 = Graphic();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(30000), rManager.getTotalSize());
}

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManager_Copy)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic_100 = makeUnloadedGraphic(u"jpg", Size(100, 100));
    Graphic aGraphic_200 = makeUnloadedGraphic(u"png", Size(200, 100));
    Graphic aGraphic_300 = makeUnloadedGraphic(u"jpg", Size(300, 100));

    Graphic aSVG = loadUnloadedGraphic(u"SimpleExample.svg");

    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    aGraphic_100.makeAvailable();
    aGraphic_200.makeAvailable();
    aGraphic_300.makeAvailable();
}

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManager)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphic;
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    Graphic aGraphicPng = loadGraphic(u"TypeDetectionExample.png");
    {
        CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, aGraphicPng.GetType());
        CPPUNIT_ASSERT_EQUAL(sal_uLong(300), aGraphicPng.GetSizeBytes());
        CPPUNIT_ASSERT_EQUAL(true, aGraphicPng.isAvailable());
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(300), rManager.getTotalSize());

    CPPUNIT_ASSERT_EQUAL(false, aGraphicPng.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphicPng.ImplGetImpGraphic()->swapOut());
    CPPUNIT_ASSERT_EQUAL(true, aGraphicPng.ImplGetImpGraphic()->isSwappedOut());
    CPPUNIT_ASSERT_EQUAL(false, aGraphicPng.isAvailable());

    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), rManager.getTotalSize());

    Graphic aGraphicJpg = makeUnloadedGraphic(u"jpg");
    {
        CPPUNIT_ASSERT_EQUAL(tools::Long(120), aGraphicJpg.GetSizePixel().Width());
        CPPUNIT_ASSERT_EQUAL(tools::Long(100), aGraphicJpg.GetSizePixel().Height());
        CPPUNIT_ASSERT_EQUAL(false, aGraphicJpg.isAvailable());
    }

    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(0), rManager.getTotalSize());

    aGraphicJpg.makeAvailable();

    CPPUNIT_ASSERT_EQUAL(size_t(2), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(36000), rManager.getTotalSize());

    CPPUNIT_ASSERT_EQUAL(true, aGraphicPng.makeAvailable());
    CPPUNIT_ASSERT_EQUAL(false, aGraphicPng.ImplGetImpGraphic()->isSwappedOut());

    CPPUNIT_ASSERT_EQUAL(size_t(2), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(36300), rManager.getTotalSize());

    aGraphicJpg = aGraphic;

    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(300), rManager.getTotalSize());
}

namespace
{
class TestManaged : public vcl::graphic::MemoryManaged
{
public:
    std::chrono::high_resolution_clock::time_point maLastUsed
        = std::chrono::high_resolution_clock::now();

    TestManaged(bool bRegister, sal_Int64 nSize = 0)
        : MemoryManaged(bRegister)
    {
        updateCurrentSizeInBytes(nSize);
    }

    void setCurrentSize(sal_Int64 nSize) { updateCurrentSizeInBytes(nSize); }

    void callRegister() { registerIntoManager(); }

    void callUnregister() { unregisterFromManager(); }

    bool canReduceMemory() const override { return false; }

    bool reduceMemory() override { return false; }

    std::chrono::high_resolution_clock::time_point getLastUsed() const override
    {
        return maLastUsed;
    }

    void dumpState(rtl::OStringBuffer& /*rState*/) override {}
};
}

CPPUNIT_TEST_FIXTURE(GraphicMemoryTest, testMemoryManagerX)
{
    auto& rManager = vcl::graphic::MemoryManager::get();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    TestManaged aTestManaged(false, 1000);
    CPPUNIT_ASSERT_EQUAL(size_t(0), rManager.getManagedObjects().size());

    TestManaged aTestManaged2(true, 100);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(100), rManager.getTotalSize());

    aTestManaged2.setCurrentSize(400);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(400), rManager.getTotalSize());

    aTestManaged.setCurrentSize(600);
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(400), rManager.getTotalSize());

    aTestManaged.callRegister();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(1000), rManager.getTotalSize());

    aTestManaged.callUnregister();
    CPPUNIT_ASSERT_EQUAL(size_t(1), rManager.getManagedObjects().size());
    CPPUNIT_ASSERT_EQUAL(sal_Int64(400), rManager.getTotalSize());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
