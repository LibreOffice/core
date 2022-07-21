/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/a11y/swaccessibletestbase.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustrbuf.hxx>

#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

uno::Reference<accessibility::XAccessibleContext>
test::SwAccessibleTestBase::getPreviousFlowingSibling(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    return getFirstRelationTargetOfType(xContext,
                                        accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM);
}

uno::Reference<accessibility::XAccessibleContext> test::SwAccessibleTestBase::getNextFlowingSibling(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    return getFirstRelationTargetOfType(xContext,
                                        accessibility::AccessibleRelationType::CONTENT_FLOWS_TO);
}

/* Care has to be taken not to walk sideways as the relation is also used
 * with children of nested containers (possibly as the "natural"/"perceived" flow?). */
std::deque<uno::Reference<accessibility::XAccessibleContext>>
test::SwAccessibleTestBase::getAllChildren(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    /* first, get all "natural" children */
    auto children = AccessibleTestBase::getAllChildren(xContext);
    if (!children.size())
        return children;

    /* then, try and find flowing siblings at the same levels that are not included in the list */
    /* first, backwards: */
    auto child = getPreviousFlowingSibling(children.front());
    while (child.is() && children.size() < AccessibilityTools::MAX_CHILDREN)
    {
        auto childParent = child->getAccessibleParent();
        if (childParent.is()
            && AccessibilityTools::equals(xContext, childParent->getAccessibleContext()))
            children.push_front(child);
        child = getPreviousFlowingSibling(child);
    }
    /* then forward */
    child = getNextFlowingSibling(children.back());
    while (child.is() && children.size() < AccessibilityTools::MAX_CHILDREN)
    {
        auto childParent = child->getAccessibleParent();
        if (childParent.is()
            && AccessibilityTools::equals(xContext, childParent->getAccessibleContext()))
            children.push_back(child);
        child = getNextFlowingSibling(child);
    }

    return children;
}

void test::SwAccessibleTestBase::collectText(
    const uno::Reference<accessibility::XAccessibleContext>& xContext, rtl::OUStringBuffer& buffer,
    bool onlyChildren)
{
    const auto& roleName = AccessibilityTools::getRoleName(xContext->getAccessibleRole());

    std::cout << "collecting text for child of role " << roleName << "..." << std::endl;

    if (!onlyChildren)
    {
        const struct
        {
            std::u16string_view name;
            rtl::OUString value;
        } attrs[] = {
            { u"name", xContext->getAccessibleName() },
            { u"description", xContext->getAccessibleDescription() },
        };

        buffer.append('<');
        buffer.append(roleName);
        for (auto& attr : attrs)
        {
            if (attr.value.getLength() == 0)
                continue;
            buffer.append(' ');
            buffer.append(attr.name);
            buffer.append(u"=\"" + attr.value.replaceAll(u"\"", u"&quot;") + "\"");
        }
        buffer.append('>');
    }
    auto openTagLength = buffer.getLength();

    uno::Reference<accessibility::XAccessibleText> xText(xContext, uno::UNO_QUERY);
    if (xText.is())
        buffer.append(xText->getText());

    for (auto& childContext : getAllChildren(xContext))
        collectText(childContext, buffer);

    if (!onlyChildren)
    {
        if (buffer.getLength() != openTagLength)
            buffer.append("</" + roleName + ">");
        else
        {
            /* there was no content, so make is a short tag for more concise output */
            buffer[openTagLength - 1] = '/';
            buffer.append('>');
        }
    }
}

OUString test::SwAccessibleTestBase::collectText(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    rtl::OUStringBuffer buf;
    collectText(xContext, buf, isDocumentRole(xContext->getAccessibleRole()));
    return buf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
