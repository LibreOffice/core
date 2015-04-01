/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>

#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdpage.hxx>
#include <unomodel.hxx>

using namespace css;

static const char* DATA_DIRECTORY = "/sd/qa/unit/tiledrendering/data/";

class SdTiledRenderingTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void testRegisterCallback();
    void testPostMouseEvent();

    CPPUNIT_TEST_SUITE(SdTiledRenderingTest);
#if !defined(WNT) && !defined(MACOSX)
    CPPUNIT_TEST(testRegisterCallback);
    CPPUNIT_TEST(testPostMouseEvent);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    SdXImpressDocument* createDoc(const char* pName);
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    uno::Reference<lang::XComponent> mxComponent;
    Rectangle m_aInvalidation;
};

void SdTiledRenderingTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SdTiledRenderingTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

SdXImpressDocument* SdTiledRenderingTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.presentation.PresentationDocument");
    SdXImpressDocument* pImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pImpressDocument);
    pImpressDocument->initializeForTiledRendering();
    return pImpressDocument;
}

void SdTiledRenderingTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<SdTiledRenderingTest*>(pData)->callbackImpl(nType, pPayload);
}

void SdTiledRenderingTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
    case LOK_CALLBACK_INVALIDATE_TILES:
    {
        OUString aPayload = OUString::createFromAscii(pPayload);
        if (aPayload != "EMPTY" && m_aInvalidation.IsEmpty())
        {
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(aPayload);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aInvalidation.setX(aSeq[0].toInt32());
            m_aInvalidation.setY(aSeq[1].toInt32());
            m_aInvalidation.setWidth(aSeq[2].toInt32());
            m_aInvalidation.setHeight(aSeq[3].toInt32());
        }
    }
    break;
    }
}

void SdTiledRenderingTest::testRegisterCallback()
{
    // Tests sd::Window::LogicInvalidate().
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    pXImpressDocument->registerCallback(&SdTiledRenderingTest::callback, this);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    // Append a character to the empty title shape.
    vcl::KeyCode aKeyCode(0, 0);
    KeyEvent aKeyEvent(static_cast<sal_Unicode>('x'), aKeyCode, 0);
    pViewShell->KeyInput(aKeyEvent, 0);
    mxComponent->dispose();

    // Check that the top left 256x256px tile would be invalidated.
    CPPUNIT_ASSERT(!m_aInvalidation.IsEmpty());
    Rectangle aTopLeft(0, 0, 256*15, 256*15); // 1 px = 15 twips, assuming 96 DPI.
    CPPUNIT_ASSERT(m_aInvalidation.IsOver(aTopLeft));
}

void SdTiledRenderingTest::testPostMouseEvent()
{
    SdXImpressDocument* pXImpressDocument = createDoc("dummy.odp");
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pActualPage = pViewShell->GetActualPage();
    SdrObject* pObject = pActualPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(OBJ_TITLETEXT), pObject->GetObjIdentifier());
    SdrTextObj* pTextObj = static_cast<SdrTextObj*>(pObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTextObj, pView->GetSdrPageView());
    SfxStringItem aInputString(SID_ATTR_CHAR, "x");
    pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_CHAR, SfxCallMode::SYNCHRON, &aInputString, 0);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    EditView& rEditView = pView->GetTextEditOutlinerView()->GetEditView();
    // Did we manage to go after the first character?
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), rEditView.GetSelection().nStartPos);

    vcl::Cursor* pCursor = rEditView.GetCursor();
    Point aPosition = pCursor->GetPos();
    aPosition.setX(aPosition.getX() - 1000);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()), 1);
    pXImpressDocument->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, convertMm100ToTwip(aPosition.getX()), convertMm100ToTwip(aPosition.getY()), 1);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    // The new cursor position must be before the first word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), rEditView.GetSelection().nStartPos);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdTiledRenderingTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
