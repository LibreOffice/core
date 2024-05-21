/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/datatransfer/XTransferable2.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/string.hxx>

#include <docuno.hxx>

using namespace com::sun::star;

namespace
{
class Test : public UnoApiXmlTest
{
public:
    Test();
    void setUp() override;
    void tearDown() override;

    ScModelObj* createDoc(const char* pName);
};

Test::Test()
    : UnoApiXmlTest(u"/sc/qa/unit/tiledrendering2/data/"_ustr)
{
}

void Test::setUp()
{
    UnoApiXmlTest::setUp();

    comphelper::LibreOfficeKit::setActive(true);
}

void Test::tearDown()
{
    if (mxComponent.is())
    {
        mxComponent->dispose();
        mxComponent.clear();
    }

    comphelper::LibreOfficeKit::resetCompatFlag();

    comphelper::LibreOfficeKit::setActive(false);

    UnoApiXmlTest::tearDown();
}

ScModelObj* Test::createDoc(const char* pName)
{
    loadFromFile(OUString::createFromAscii(pName));

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    pModelObj->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    return pModelObj;
}

/// A view callback tracks callbacks invoked on one specific view.
class ViewCallback final
{
    SfxViewShell* mpViewShell;
    int mnView;

public:
    std::map<std::string, boost::property_tree::ptree> m_aStateChanges;
    tools::Rectangle m_aCellCursorBounds;
    TestLokCallbackWrapper m_callbackWrapper;

    ViewCallback()
        : m_callbackWrapper(&callback, this)
    {
        mpViewShell = SfxViewShell::Current();
        mpViewShell->setLibreOfficeKitViewCallback(&m_callbackWrapper);
        mnView = SfxLokHelper::getView();
        m_callbackWrapper.setLOKViewId(mnView);
    }

    ~ViewCallback()
    {
        if (mpViewShell)
        {
            SfxLokHelper::setView(mnView);
            mpViewShell->setLibreOfficeKitViewCallback(nullptr);
        }
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        switch (nType)
        {
            case LOK_CALLBACK_CELL_CURSOR:
            {
                uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(
                    OUString::createFromAscii(pPayload));
                m_aCellCursorBounds = tools::Rectangle();
                if (aSeq.getLength() >= 4)
                {
                    m_aCellCursorBounds.SetLeft(aSeq[0].toInt32());
                    m_aCellCursorBounds.SetTop(aSeq[1].toInt32());
                    m_aCellCursorBounds.setWidth(aSeq[2].toInt32());
                    m_aCellCursorBounds.setHeight(aSeq[3].toInt32());
                }
            }
            break;
            case LOK_CALLBACK_STATE_CHANGED:
            {
                std::stringstream aStream(pPayload);
                if (!aStream.str().starts_with("{"))
                {
                    break;
                }

                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                auto it = aTree.find("commandName");
                if (it == aTree.not_found())
                {
                    break;
                }

                std::string aCommandName = it->second.get_value<std::string>();
                m_aStateChanges[aCommandName] = aTree;
            }
            break;
        }
    }
};

CPPUNIT_TEST_FIXTURE(Test, testSidebarLocale)
{
    ScModelObj* pModelObj = createDoc("chart.ods");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetLOKLocale(u"en-US"_ustr);
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale(u"de-DE"_ustr);
    TestLokCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();
    aView2.m_aStateChanges.clear();

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    auto it = aView2.m_aStateChanges.find(".uno:Sidebar");
    CPPUNIT_ASSERT(it != aView2.m_aStateChanges.end());
    std::string aLocale = it->second.get<std::string>("locale");
    CPPUNIT_ASSERT_EQUAL(std::string("de-DE"), aLocale);
}

CPPUNIT_TEST_FIXTURE(Test, testCopyMultiSelection)
{
    // Given a document with A1 and A3 as selected cells:
    ScModelObj* pModelObj = createDoc("multi-selection.ods");
    ViewCallback aView1;
    // Get the center of A3:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    Point aPoint = aView1.m_aCellCursorBounds.Center();
    // Go to A1:
    aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    // Ctrl-click on A3:
    int nCtrl = KEY_MOD1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, nCtrl);

    // When getting the selection:
    uno::Reference<datatransfer::XTransferable> xTransferable = pModelObj->getSelection();

    // Make sure we get A1+A3 instead of an error:
    CPPUNIT_ASSERT(xTransferable.is());

    // Also make sure that just 2 cells is classified as a simple selection:
    uno::Reference<datatransfer::XTransferable2> xTransferable2(xTransferable, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTransferable2.is());
    // Without the fix, the text selection was complex.
    CPPUNIT_ASSERT(!xTransferable2->isComplex());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
