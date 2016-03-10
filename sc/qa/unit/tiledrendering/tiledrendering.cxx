/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>
#include <boost/property_tree/json_parser.hpp>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>

#include <comphelper/lok.hxx>

#include <document.hxx>
#include <docuno.hxx>

using namespace css;

#if !defined(WNT) && !defined(MACOSX)
static const char* DATA_DIRECTORY = "/sc/qa/unit/tiledrendering/data/";
#endif

class ScTiledRenderingTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    ScTiledRenderingTest();
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

#if !defined(WNT) && !defined(MACOSX)
    void testSortAscendingDescending();
#endif

    CPPUNIT_TEST_SUITE(ScTiledRenderingTest);
#if !defined(WNT) && !defined(MACOSX)
    CPPUNIT_TEST(testSortAscendingDescending);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
#if !defined(WNT) && !defined(MACOSX)
    ScModelObj* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
#endif

    uno::Reference<lang::XComponent> mxComponent;
#if !defined(WNT) && !defined(MACOSX)
    // TODO various test-related members - when needed
#endif
};

ScTiledRenderingTest::ScTiledRenderingTest()
#if !defined(WNT) && !defined(MACOSX)
    // TODO various test-related members - when needed
#endif
{
}

void ScTiledRenderingTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void ScTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

#if !defined(WNT) && !defined(MACOSX)
ScModelObj* ScTiledRenderingTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pModelObj;
}

void ScTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<ScTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

/* TODO when needed...
static std::vector<OUString> lcl_convertSeparated(const OUString& rString, sal_Unicode nSeparator)
{
    std::vector<OUString> aRet;

    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = rString.getToken(0, nSeparator, nIndex);
        aToken = aToken.trim();
        if (!aToken.isEmpty())
            aRet.push_back(aToken);
    }
    while (nIndex >= 0);

    return aRet;
}

static void lcl_convertRectangle(const OUString& rString, Rectangle& rRectangle)
{
    uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(rString);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
    rRectangle.setX(aSeq[0].toInt32());
    rRectangle.setY(aSeq[1].toInt32());
    rRectangle.setWidth(aSeq[2].toInt32());
    rRectangle.setHeight(aSeq[3].toInt32());
}
*/

void ScTiledRenderingTest::callbackImpl(int /*nType*/, const char* /*pPayload*/)
{
    // TODO when needed...
    //switch (nType)
    //{
    //}
}

void ScTiledRenderingTest::testSortAscendingDescending()
{
    comphelper::LibreOfficeKit::setActive();
    ScModelObj* pModelObj = createDoc("sort-range.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    // select the values in the first column
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, 551, 129, 1);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, 820, 1336, 1);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, 820, 1359, 1);

    // sort ascending
    uno::Sequence<beans::PropertyValue> aArgs;
    comphelper::dispatchCommand(".uno:SortAscending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(r + 1), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    // sort descending
    comphelper::dispatchCommand(".uno:SortDescending", aArgs);

    // check it's sorted
    for (SCROW r = 0; r < 6; ++r)
    {
        CPPUNIT_ASSERT_EQUAL(double(6 - r), pDoc->GetValue(ScAddress(0, r, 0)));
    }

    // nothing else was sorted
    CPPUNIT_ASSERT_EQUAL(double(1), pDoc->GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(double(3), pDoc->GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(double(2), pDoc->GetValue(ScAddress(1, 2, 0)));

    comphelper::LibreOfficeKit::setActive(false);
}

#endif

CPPUNIT_TEST_SUITE_REGISTRATION(ScTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
