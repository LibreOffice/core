/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/a11y/accessibletestbase.hxx>

#include <string>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XCloseable.hpp>

#include <vcl/scheduler.hxx>

#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

void test::AccessibleTestBase::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop = frame::Desktop::create(mxComponentContext);
}

void test::AccessibleTestBase::close()
{
    if (mxDocument.is())
    {
        uno::Reference<util::XCloseable> xCloseable(mxDocument, uno::UNO_QUERY_THROW);
        xCloseable->close(false);
        mxDocument.clear();
    }
}

void test::AccessibleTestBase::tearDown() { close(); }

void test::AccessibleTestBase::load(const rtl::OUString& sURL)
{
    // make sure there is no open document in case it is called more than once
    close();
    mxDocument = mxDesktop->loadComponentFromURL(sURL, "_blank", 40, {});

    uno::Reference<frame::XModel> xModel(mxDocument, uno::UNO_QUERY_THROW);
    mxWindow.set(xModel->getCurrentController()->getFrame()->getContainerWindow());

    // bring window to front
    uno::Reference<awt::XTopWindow> xTopWindow(mxWindow, uno::UNO_QUERY_THROW);
    xTopWindow->toFront();
}

void test::AccessibleTestBase::loadFromSrc(const rtl::OUString& sSrcPath)
{
    load(m_directories.getURLFromSrc(sSrcPath));
}

uno::Reference<accessibility::XAccessibleContext>
test::AccessibleTestBase::getWindowAccessibleContext()
{
    uno::Reference<accessibility::XAccessible> xAccessible(mxWindow, uno::UNO_QUERY_THROW);

    return xAccessible->getAccessibleContext();
}

bool test::AccessibleTestBase::isDocumentRole(const sal_Int16 role)
{
    return (role == accessibility::AccessibleRole::DOCUMENT
            || role == accessibility::AccessibleRole::DOCUMENT_PRESENTATION
            || role == accessibility::AccessibleRole::DOCUMENT_SPREADSHEET
            || role == accessibility::AccessibleRole::DOCUMENT_TEXT);
}

uno::Reference<accessibility::XAccessibleContext>
test::AccessibleTestBase::getDocumentAccessibleContext()
{
    uno::Reference<frame::XModel> xModel(mxDocument, uno::UNO_QUERY_THROW);
    uno::Reference<accessibility::XAccessible> xAccessible(
        xModel->getCurrentController()->getFrame()->getComponentWindow(), uno::UNO_QUERY_THROW);

    return AccessibilityTools::getAccessibleObjectForPredicate(
        xAccessible->getAccessibleContext(),
        [](const uno::Reference<accessibility::XAccessibleContext>& xCtx) {
            return (isDocumentRole(xCtx->getAccessibleRole())
                    && xCtx->getAccessibleStateSet() & accessibility::AccessibleStateType::SHOWING);
        });
}

uno::Reference<accessibility::XAccessibleContext>
test::AccessibleTestBase::getFirstRelationTargetOfType(
    const uno::Reference<accessibility::XAccessibleContext>& xContext, sal_Int16 relationType)
{
    auto relset = xContext->getAccessibleRelationSet();

    if (relset.is())
    {
        for (sal_Int32 i = 0; i < relset->getRelationCount(); ++i)
        {
            const auto& rel = relset->getRelation(i);
            if (rel.RelationType == relationType)
            {
                for (auto& target : rel.TargetSet)
                {
                    uno::Reference<accessibility::XAccessible> targetAccessible(target,
                                                                                uno::UNO_QUERY);
                    if (targetAccessible.is())
                        return targetAccessible->getAccessibleContext();
                }
            }
        }
    }

    return nullptr;
}

/* TODO: make this an iterator so it is cheaper -- but might be tricky to implement
 * in the Writer case (see swaccessibletestbase.cxx), which might not make it worth
 * it in a test scenario. */
std::deque<uno::Reference<accessibility::XAccessibleContext>>
test::AccessibleTestBase::getAllChildren(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    std::deque<uno::Reference<accessibility::XAccessibleContext>> children;
    auto childCount = xContext->getAccessibleChildCount();

    /* first, get all "natural" children */
    for (sal_Int32 i = 0; i < childCount && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto child = xContext->getAccessibleChild(i);
        children.push_back(child->getAccessibleContext());
    }

    return children;
}

/** Prints the tree of accessible objects starting at @p xContext to stdout */
void test::AccessibleTestBase::dumpA11YTree(
    const uno::Reference<accessibility::XAccessibleContext>& xContext, const int depth)
{
    Scheduler::ProcessEventsToIdle();
    auto xRelSet = xContext->getAccessibleRelationSet();

    std::cout << AccessibilityTools::debugString(xContext);
    // relation set is not included in the debug string, but might be useful here
    if (xRelSet.is())
    {
        auto relCount = xRelSet->getRelationCount();
        if (relCount)
        {
            std::cout << " rels=[";
            for (sal_Int32 i = 0; i < relCount; ++i)
            {
                if (i > 0)
                    std::cout << ", ";

                const auto& rel = xRelSet->getRelation(i);
                std::cout << "(type=" << AccessibilityTools::getRelationTypeName(rel.RelationType)
                          << " (" << rel.RelationType << ")";
                std::cout << " targets=[";
                int j = 0;
                for (auto& target : rel.TargetSet)
                {
                    if (j++ > 0)
                        std::cout << ", ";
                    uno::Reference<accessibility::XAccessible> ta(target, uno::UNO_QUERY_THROW);
                    std::cout << AccessibilityTools::debugString(ta);
                }
                std::cout << "])";
            }
            std::cout << "]";
        }
    }
    std::cout << std::endl;

    sal_Int32 i = 0;
    for (auto& child : getAllChildren(xContext))
    {
        for (int j = 0; j < depth; j++)
            std::cout << "  ";
        std::cout << " * child " << i++ << ": ";
        dumpA11YTree(child, depth + 1);
    }
}

/* see OAccessibleMenuItemComponent::GetAccessibleName() */
static bool accessibleNameMatches(const uno::Reference<accessibility::XAccessibleContext>& xContext,
                                  std::u16string_view name)
{
    const OUString actualName = xContext->getAccessibleName();

    if (actualName == name)
        return true;

#if defined(_WIN32)
    /* on Win32, ignore a \tSHORTCUT suffix on a menu item */
    switch (xContext->getAccessibleRole())
    {
        case accessibility::AccessibleRole::MENU_ITEM:
        case accessibility::AccessibleRole::RADIO_MENU_ITEM:
        case accessibility::AccessibleRole::CHECK_MENU_ITEM:
            return actualName.startsWith(name) && actualName[name.length()] == '\t';

        default:
            break;
    }
#endif

    return false;
}

/** Gets a child by name (usually in a menu) */
uno::Reference<accessibility::XAccessibleContext> test::AccessibleTestBase::getItemFromName(
    const uno::Reference<accessibility::XAccessibleContext>& xMenuCtx, std::u16string_view name)
{
    auto childCount = xMenuCtx->getAccessibleChildCount();

    std::cout << "looking up item " << OUString(name) << " in "
              << AccessibilityTools::debugString(xMenuCtx) << std::endl;
    for (sal_Int32 i = 0; i < childCount && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto item = xMenuCtx->getAccessibleChild(i)->getAccessibleContext();
        if (accessibleNameMatches(item, name))
        {
            std::cout << "-> found " << AccessibilityTools::debugString(item) << std::endl;
            return item;
        }
    }

    std::cout << "-> NOT FOUND!" << std::endl;
    std::cout << "   Contents was: ";
    dumpA11YTree(xMenuCtx, 1);

    return uno::Reference<accessibility::XAccessibleContext>();
}

bool test::AccessibleTestBase::activateMenuItem(
    const uno::Reference<accessibility::XAccessibleAction>& xAction)
{
    // assume first action is the right one, there's not description anyway
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xAction->getAccessibleActionCount());
    if (xAction->doAccessibleAction(0))
    {
        Scheduler::ProcessEventsToIdle();
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
