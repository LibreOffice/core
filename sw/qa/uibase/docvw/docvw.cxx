/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/text/XTextDocument.hpp>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/lokcallback.hxx>
#include <vcl/event.hxx>
#include <vcl/scheduler.hxx>

#include <docsh.hxx>
#include <edtwin.hxx>
#include <flyfrm.hxx>
#include <frameformats.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <unotxdoc.hxx>
#include <view.hxx>
#include <viscrs.hxx>
#include <wrtsh.hxx>

namespace
{
/// Covers sw/source/uibase/docvw/ fixes.
class Test : public SwModelTestBase
{
};
}

CPPUNIT_TEST_FIXTURE(Test, testShiftClickOnImage)
{
    // Given a document with a fly frame:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xTextGraphic->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(5000, 5000)));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    pWrtShell->SttEndDoc(/*bStt=*/false);

    // When shift-clicking on that fly frame:
    auto& rSpzFormats = *pDoc->GetSpzFrameFormats();
    auto pFrameFormat = dynamic_cast<SwFlyFrameFormat*>(rSpzFormats[0]);
    CPPUNIT_ASSERT(pFrameFormat);
    SwFlyFrame* pFlyFrame = pFrameFormat->GetFrame();
    vcl::Window& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    Point aFlyCenter = rEditWin.LogicToPixel(pFlyFrame->getFrameArea().Center());
    MouseEvent aClickEvent(aFlyCenter, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, KEY_SHIFT);
    rEditWin.MouseButtonDown(aClickEvent);
    rEditWin.MouseButtonUp(aClickEvent);

    // Then make sure that the fly frame is selected:
    SelectionType eType = pWrtShell->GetSelectionType();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2 (SelectionType::Graphic)
    // - Actual  : 1 (SelectionType::Text)
    // i.e. the fly frame was not selected, while a plain click or ctrl-click selected it.
    CPPUNIT_ASSERT_EQUAL(SelectionType::Graphic, eType);
}

namespace
{
/// Interception implementation that catches the graphic dialog.
class GraphicDialogInterceptor : public cppu::WeakImplHelper<frame::XDispatchProviderInterceptor>
{
    uno::Reference<frame::XDispatchProvider> m_xMaster;
    uno::Reference<frame::XDispatchProvider> m_xSlave;
    int m_nGraphicDialogs = 0;

public:
    // XDispatchProviderInterceptor
    uno::Reference<frame::XDispatchProvider> SAL_CALL getMasterDispatchProvider() override;
    uno::Reference<frame::XDispatchProvider> SAL_CALL getSlaveDispatchProvider() override;
    void SAL_CALL setMasterDispatchProvider(
        const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;
    void SAL_CALL
    setSlaveDispatchProvider(const uno::Reference<frame::XDispatchProvider>& xNewSupplier) override;

    // XDispatchProvider
    uno::Reference<frame::XDispatch> SAL_CALL queryDispatch(const util::URL& rURL,
                                                            const OUString& rTargetFrameName,
                                                            sal_Int32 SearchFlags) override;
    uno::Sequence<uno::Reference<frame::XDispatch>> SAL_CALL
    queryDispatches(const uno::Sequence<frame::DispatchDescriptor>& rRequests) override;

    int GetGraphicDialogs() const;
};
}

uno::Reference<frame::XDispatchProvider> GraphicDialogInterceptor::getMasterDispatchProvider()
{
    return m_xMaster;
}

uno::Reference<frame::XDispatchProvider> GraphicDialogInterceptor::getSlaveDispatchProvider()
{
    return m_xSlave;
}

void GraphicDialogInterceptor::setMasterDispatchProvider(
    const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    m_xMaster = xNewSupplier;
}

void GraphicDialogInterceptor::setSlaveDispatchProvider(
    const uno::Reference<frame::XDispatchProvider>& xNewSupplier)
{
    m_xSlave = xNewSupplier;
}

uno::Reference<frame::XDispatch>
GraphicDialogInterceptor::queryDispatch(const util::URL& rURL, const OUString& rTargetFrameName,
                                        sal_Int32 nSearchFlags)
{
    if (rURL.Complete == ".uno:GraphicDialog")
    {
        ++m_nGraphicDialogs;
    }

    return m_xSlave->queryDispatch(rURL, rTargetFrameName, nSearchFlags);
}

uno::Sequence<uno::Reference<frame::XDispatch>> GraphicDialogInterceptor::queryDispatches(
    const uno::Sequence<frame::DispatchDescriptor>& /*rRequests*/)
{
    return {};
}

int GraphicDialogInterceptor::GetGraphicDialogs() const { return m_nGraphicDialogs; }

CPPUNIT_TEST_FIXTURE(Test, testShiftDoubleClickOnImage)
{
    // Given a document with a fly frame, and an interceptor to catch the graphic dialog:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue(u"AnchorType"_ustr,
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xTextGraphic->setPropertyValue(u"Size"_ustr, uno::Any(awt::Size(5000, 5000)));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    pWrtShell->SttEndDoc(/*bStt=*/false);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XDispatchProviderInterception> xRegistration(
        xModel->getCurrentController()->getFrame(), uno::UNO_QUERY);
    rtl::Reference pInterceptor(new GraphicDialogInterceptor);
    xRegistration->registerDispatchProviderInterceptor(pInterceptor);

    // When shift-double-clicking on that fly frame:
    auto& rSpzFormats = *pDoc->GetSpzFrameFormats();
    auto pFrameFormat = dynamic_cast<SwFlyFrameFormat*>(rSpzFormats[0]);
    CPPUNIT_ASSERT(pFrameFormat);
    SwFlyFrame* pFlyFrame = pFrameFormat->GetFrame();
    vcl::Window& rEditWin = getSwDocShell()->GetView()->GetEditWin();
    Point aFlyCenter = rEditWin.LogicToPixel(pFlyFrame->getFrameArea().Center());
    MouseEvent aClickEvent(aFlyCenter, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, KEY_SHIFT);
    rEditWin.MouseButtonDown(aClickEvent);
    rEditWin.MouseButtonUp(aClickEvent);
    aClickEvent
        = MouseEvent(aFlyCenter, 2, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, KEY_SHIFT);
    rEditWin.MouseButtonDown(aClickEvent);
    rEditWin.MouseButtonUp(aClickEvent);

    // Then make sure that the fly frame's dialog is dispatched:
    int nGraphicDialogs = pInterceptor->GetGraphicDialogs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 0 (2)
    // - Actual  : 0
    // i.e. the fly frame's dialog was not dispatched, while a plain click or ctrl-click dispatched
    // it.
    CPPUNIT_ASSERT_GREATER(0, nGraphicDialogs);
}

namespace
{
/// Test LOK callback, handling just LOK_CALLBACK_TOOLTIP.
struct TooltipCallback
{
    std::string rect;
    std::string anchorRectangles;
    std::string redlineType;

    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);
};

void TooltipCallback::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<TooltipCallback*>(pData)->callbackImpl(nType, pPayload);
}

void TooltipCallback::callbackImpl(int nType, const char* pPayload)
{
    if (nType == LOK_CALLBACK_TOOLTIP)
    {
        std::stringstream aStream(pPayload);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        rect = aTree.get_child("rectangle").get_value<std::string>();
        auto it = aTree.find("anchorRectangles");
        if (it != aTree.not_found())
        {
            std::vector<std::string> aRects;
            for (const auto& rRect : it->second)
                aRects.push_back(rRect.second.get_value<std::string>());
            std::stringstream aRectStream;
            for (size_t i = 0; i < aRects.size(); ++i)
            {
                if (i > 0)
                    aRectStream << "; ";
                aRectStream << aRects[i];
            }
            anchorRectangles = aRectStream.str();
        }
        auto itType = aTree.find("redlineType");
        if (itType != aTree.not_found())
            redlineType = itType->second.get_value<std::string>();
    }
}
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineTooltipAnchorRectangles)
{
    // Set up LOK:
    comphelper::LibreOfficeKit::setActive(true);

    // Given a document with a redline:
    createSwDoc();
    getSwTextDoc()->initializeForTiledRendering(uno::Sequence<beans::PropertyValue>());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    TooltipCallback aCallback;
    TestLokCallbackWrapper aCallbackWrapper(&TooltipCallback::callback, &aCallback);
    pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(&aCallbackWrapper);
    aCallbackWrapper.setLOKViewId(SfxLokHelper::getView(*pWrtShell->GetSfxViewShell()));
    pWrtShell->SetRedlineFlagsAndCheckInsMode(RedlineFlags::On | RedlineFlags::ShowMask);
    pWrtShell->Insert(u"test"_ustr);

    // When moving the mouse over the redline:
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT(pShellCursor);
    pWrtShell->EndOfSection(/*bSelect=*/false);
    Point aEnd = pShellCursor->GetSttPos();
    pWrtShell->StartOfSection(/*bSelect=*/false);
    Point aStart = pShellCursor->GetSttPos();
    Point aMiddle((aStart.getX() + aEnd.getX()) / 2, (aStart.getY() + aEnd.getY()) / 2);
    getSwTextDoc()->postMouseEvent(LOK_MOUSEEVENT_MOUSEMOVE, aMiddle.getX(), aMiddle.getY(), 1, 0,
                                   0);
    Scheduler::ProcessEventsToIdle();

    // Then make sure the tooltip callback has redlineType and anchorRectangles:
    // Without the accompanying fix in place, this test would have failed, no anchor rectangles were
    // emitted.
    CPPUNIT_ASSERT(!aCallback.anchorRectangles.empty());
    CPPUNIT_ASSERT_EQUAL(std::string("Insert"), aCallback.redlineType);

    // Tear down LOK:
    pWrtShell->GetSfxViewShell()->setLibreOfficeKitViewCallback(nullptr);
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
