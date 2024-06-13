/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextDocument.hpp>

#include <vcl/event.hxx>

#include <docsh.hxx>
#include <edtwin.hxx>
#include <flyfrm.hxx>
#include <frameformats.hxx>
#include <view.hxx>
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
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    vcl::Window& rEditWin = pDoc->GetDocShell()->GetView()->GetEditWin();
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
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    vcl::Window& rEditWin = pDoc->GetDocShell()->GetView()->GetEditWin();
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
