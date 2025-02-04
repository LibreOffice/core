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

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <vcl/scheduler.hxx>
#include <tabvwsh.hxx>

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
    : UnoApiXmlTest("/sc/qa/unit/tiledrendering2/data/")
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
    std::string decimalSeparator;
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
            case LOK_CALLBACK_JSDIALOG:
            {
                std::stringstream aStream(pPayload);
                boost::property_tree::ptree aTree;
                boost::property_tree::read_json(aStream, aTree);
                if (aTree.get_child("jsontype").get_value<std::string>() == "formulabar")
                {
                    if (aTree.find("data") != aTree.not_found())
                    {
                        if (aTree.get_child("data").find("separator")
                            != aTree.get_child("data").not_found())
                        {
                            decimalSeparator = aTree.get_child("data")
                                                   .get_child("separator")
                                                   .get_value<std::string>();
                        }
                    }
                }
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
    pView1->SetLOKLocale("en-US");
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale("de-DE");
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

CPPUNIT_TEST_FIXTURE(Test, testDecimalSeparatorInfo)
{
    createDoc("decimal-separator.ods");

    ViewCallback aView1;

    // Go to cell A1.
    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue("ToPoint", OUString("$A$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    // Cell A1 has language set to English. Decimal separator should be ".".
    CPPUNIT_ASSERT_EQUAL(std::string("."), aView1.decimalSeparator);

    // Go to cell B1.
    aPropertyValues = { comphelper::makePropertyValue("ToPoint", OUString("B$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    // Cell B1 has language set to Turkish. Decimal separator should be ",".
    CPPUNIT_ASSERT_EQUAL(std::string(","), aView1.decimalSeparator);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
