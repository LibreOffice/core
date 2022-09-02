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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/storagehelper.hxx>
#include <officecfg/Office/Common.hxx>

#include <animationcommandnode.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers slideshow/source/engine/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

/// Get the first command node in the animation tree of the page, assuming that it's the first child
/// (recursively).
uno::Reference<animations::XCommand>
GetFirstCommandNodeOfPage(const uno::Reference<drawing::XDrawPage>& xPage)
{
    uno::Reference<animations::XAnimationNodeSupplier> xAnimationNodeSupplier(xPage,
                                                                              uno::UNO_QUERY);
    uno::Reference<animations::XAnimationNode> xNode = xAnimationNodeSupplier->getAnimationNode();
    while (true)
    {
        if (xNode->getType() == animations::AnimationNodeType::COMMAND)
        {
            break;
        }
        uno::Reference<container::XEnumerationAccess> xEnumAccess(xNode, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xNodes = xEnumAccess->createEnumeration();
        xNode.set(xNodes->nextElement(), uno::UNO_QUERY);
    }
    uno::Reference<animations::XCommand> xRet(xNode, uno::UNO_QUERY);
    return xRet;
}
}

CPPUNIT_TEST_FIXTURE(Test, testLoopingFromAnimation)
{
    // Given a document with a looping video, the looping is defined as part of its auto-play
    // animation (and not on the media shape):
    OUString aURL = m_directories.getURLFromSrc(u"slideshow/qa/engine/data/video-loop.pptx");
    getComponent().set(loadFromDesktop(aURL));
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<animations::XCommand> xCommandNode = GetFirstCommandNodeOfPage(xPage);
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY);

    // When determining if the video should be looping or not:
    bool bLooping
        = slideshow::internal::AnimationCommandNode::GetLoopingFromAnimation(xCommandNode, xShape);

    // Then make sure that we detect the looping is wanted:
    CPPUNIT_ASSERT(bLooping);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
