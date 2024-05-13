/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>

#include <animationcommandnode.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers slideshow/source/engine/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"slideshow/qa/engine/data/"_ustr)
    {
    }
};

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
    loadFromFile(u"video-loop.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY);
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
