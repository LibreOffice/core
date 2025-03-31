/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <test/testdllapi.hxx>

#include <deque>
#include <string>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <vcl/ITiledRenderable.hxx>
#include <vcl/window.hxx>

#include <rtl/ustring.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/a11y/eventposter.hxx>

#include "AccessibilityTools.hxx"

namespace test
{
class OOO_DLLPUBLIC_TEST AccessibleTestBase : public test::BootstrapFixture
{
private:
    void collectText(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext,
                     rtl::OUStringBuffer& buffer, bool onlyChildren = false);

protected:
    css::uno::Reference<css::frame::XDesktop2> mxDesktop;
    css::uno::Reference<css::lang::XComponent> mxDocument;
    css::uno::Reference<css::awt::XWindow> mxWindow;

    static bool isDocumentRole(const sal_Int16 role);

    virtual void load(const rtl::OUString& sURL);
    virtual void loadFromSrc(const rtl::OUString& sSrcPath);
    void close();
    css::uno::Reference<css::accessibility::XAccessibleContext> getWindowAccessibleContext();
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    getDocumentAccessibleContext();

    static css::uno::Reference<css::accessibility::XAccessibleContext> getPreviousFlowingSibling(
        const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);
    static css::uno::Reference<css::accessibility::XAccessibleContext> getNextFlowingSibling(
        const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    /** Collects contents of @p xContext in a dummy markup form */
    OUString
    collectText(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    /** Collects contents of the current document */
    OUString collectText() { return collectText(getDocumentAccessibleContext()); }

    void documentPostKeyEvent(int nType, int nCharCode, int nKeyCode)
    {
        vcl::ITiledRenderable* pTiledRenderable
            = dynamic_cast<vcl::ITiledRenderable*>(mxDocument.get());
        CPPUNIT_ASSERT(pTiledRenderable);
        pTiledRenderable->postKeyEvent(nType, nCharCode, nKeyCode);
    }

    static css::uno::Reference<css::accessibility::XAccessibleContext> getFirstRelationTargetOfType(
        const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext,
        css::accessibility::AccessibleRelationType relationType);

    /**
     * @brief Tries to list all children of an accessible
     * @param xContext An XAccessibleContext object
     * @returns The list of all children (but no more than @c AccessibilityTools::MAX_CHILDREN)
     *
     * This fetches children of @p xContext.  This would ideally just be the same than iterating
     * over children the regular way up to @c AccessibilityTools::MAX_CHILDREN, but unfortunately
     * some components (Writer, Impress, ...) do not provide all their children the regular way and
     * require specifics to include them.
     *
     * There is no guarantee on *which* children are returned if there are more than
     * @c AccessibilityTools::MAX_CHILDREN -- yet they will always be the same in a given context.
     */
    virtual std::deque<css::uno::Reference<css::accessibility::XAccessibleContext>>
    getAllChildren(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext);

    void dumpA11YTree(const css::uno::Reference<css::accessibility::XAccessibleContext>& xContext,
                      const int depth = 0);

    css::uno::Reference<css::accessibility::XAccessibleContext>
    getItemFromName(const css::uno::Reference<css::accessibility::XAccessibleContext>& xMenuCtx,
                    std::u16string_view name);
    bool
    activateMenuItem(const css::uno::Reference<css::accessibility::XAccessibleAction>& xAction);
    /* just convenience not to have to query accessibility::XAccessibleAction manually */
    bool activateMenuItem(const css::uno::Reference<css::accessibility::XAccessibleContext>& xCtx)
    {
        return activateMenuItem(css::uno::Reference<css::accessibility::XAccessibleAction>(
            xCtx, css::uno::UNO_QUERY_THROW));
    }

    /* convenience to get a menu item from a list of menu item names.  Unlike
     * getItemFromName(context, name), this requires subsequently found items to implement
     * XAccessibleAction, as each but the last item will be activated before looking for
     * the next one, to account for the fact menus might not be fully populated before being
     * activated. */
    template <typename... Ts>
    css::uno::Reference<css::accessibility::XAccessibleContext>
    getItemFromName(const css::uno::Reference<css::accessibility::XAccessibleContext>& xMenuCtx,
                    std::u16string_view name, Ts... names)
    {
        auto item = getItemFromName(xMenuCtx, name);
        CPPUNIT_ASSERT(item.is());
        activateMenuItem(item);
        return getItemFromName(item, names...);
    }

    /* convenience to activate an item by its name and all its parent menus up to xMenuCtx.
     * @see getItemFromName() */
    template <typename... Ts>
    bool
    activateMenuItem(const css::uno::Reference<css::accessibility::XAccessibleContext>& xMenuCtx,
                     Ts... names)
    {
        auto item = getItemFromName(xMenuCtx, names...);
        CPPUNIT_ASSERT(item.is());
        return activateMenuItem(item);
    }

    /* convenience to activate an item by its name and all its parent menus up to the main window
     * menu bar */
    template <typename... Ts> bool activateMenuItem(Ts... names)
    {
        auto menuBar = AccessibilityTools::getAccessibleObjectForRole(
            getWindowAccessibleContext(), css::accessibility::AccessibleRole::MENU_BAR);
        CPPUNIT_ASSERT(menuBar.is());
        return activateMenuItem(menuBar, names...);
    }

    /**
     * @brief Gets the focused accessible object at @p xAcc level or below
     * @param xAcc An accessible object
     * @returns The accessible context of the focused object, or @c nullptr
     *
     * Finds the accessible object context at or under @p xAcc that has the focused state (and is
     * showing).  Normally only one such object should exist in a given hierarchy, but in all cases
     * this function will return the first one found.
     *
     * @see AccessibilityTools::getAccessibleObjectForPredicate()
     */
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    getFocusedObject(const css::uno::Reference<css::accessibility::XAccessibleContext>& xCtx);

    static inline css::uno::Reference<css::accessibility::XAccessibleContext>
    getFocusedObject(const css::uno::Reference<css::accessibility::XAccessible>& xAcc)
    {
        return getFocusedObject(xAcc->getAccessibleContext());
    }

    /**
     * @brief Navigates through focusable elements using the Tab keyboard shortcut.
     * @param xRoot The root element to look for focused elements in.
     * @param role The accessible role of the element to tab to.
     * @param name The accessible name of the element to tab to.
     * @param pEventPosterHelper Pointer to a @c EventPosterHelper instance, or @c nullptr to obtain
     *                           it from @p xRoot.
     * @returns The element tabbed to, or @c nullptr if not found.
     *
     * Navigates through focusable elements in the top level containing @p xRoot using the Tab
     * keyboard key until the focused elements matches @p role and @p name.
     *
     * Note that usually @p xRoot should be the toplevel accessible, or at least contain all
     * focusable elements within that window.  It is however *not* a requirement, but only elements
     * actually inside it will be candidate for a match, and thus if focus goes outside it, it might
     * lead to not finding the target element.
     *
     * If @p pEventPosterHelper is @c nullptr, this function will try to construct one from
     * @p xRoot.  @see EventPosterHelper.
     */
    static css::uno::Reference<css::accessibility::XAccessibleContext>
    tabTo(const css::uno::Reference<css::accessibility::XAccessible>& xRoot, const sal_Int16 role,
          const std::u16string_view name,
          const EventPosterHelperBase* pEventPosterHelper = nullptr);

    static bool tabTo(const css::uno::Reference<css::accessibility::XAccessible>& xRoot,
                      const css::uno::Reference<css::accessibility::XAccessibleContext>& xChild,
                      const EventPosterHelperBase* pEventPosterHelper = nullptr);

#if !defined(MACOSX)
    /* Dialog handling */
    class Dialog : public test::AccessibleEventPosterHelper
    {
    private:
        bool mbAutoClose;
        css::uno::Reference<css::awt::XDialog2> mxDialog2;
        css::uno::Reference<css::accessibility::XAccessible> mxAccessible;

    public:
        Dialog(const css::uno::Reference<css::awt::XDialog2>& xDialog2, bool bAutoClose = true);
        virtual ~Dialog();

        void setAutoClose(bool bAutoClose) { mbAutoClose = bAutoClose; }

        const css::uno::Reference<css::accessibility::XAccessible>& getAccessible() const
        {
            return mxAccessible;
        }

        void close(sal_Int32 result = VclResponseType::RET_CANCEL);

        css::uno::Reference<css::accessibility::XAccessibleContext>
        tabTo(const sal_Int16 role, const std::u16string_view name)
        {
            return AccessibleTestBase::tabTo(getAccessible(), role, name, this);
        }

        bool tabTo(const css::uno::Reference<css::accessibility::XAccessibleContext>& xChild)
        {
            return AccessibleTestBase::tabTo(getAccessible(), xChild, this);
        }
    };

    class DialogWaiter
    {
    public:
        virtual ~DialogWaiter() {}

        /**
         * @brief Waits for the associated dialog to close
         * @param nTimeoutMs Maximum delay to wait the dialog for
         * @returns @c true if the dialog closed, @c false if timeout was reached
         *
         * @throws css::uno::RuntimeException if an unexpected dialog popped up instead of the
         *         expected one.
         * @throws Any exception that the user callback supplied to awaitDialog() might have thrown.
         */
        virtual bool waitEndDialog(sal_uInt64 nTimeoutMs = 3000) = 0;
    };

    /**
     * @brief Helper to call user code when a given dialog opens
     * @param name The title of the dialog window to wait for
     * @param callback The user code to run when the given dialog opens
     * @param bAutoClose Whether to automatically cancel the dialog after the user code finished, if
     *                   the dialog is still there.  You should leave this to @c true unless you
     *                   know exactly what you are doing, see below.
     * @returns A @c DialogWaiter wrapper on which call waitEndDialog() after having triggered the
     *          dialog in some way.
     *
     * This function makes it fairly easy and safe to execute code once a dialog pops up:
     * @code
     * auto waiter = awaitDialog(u"Special Characters", [this](Dialog &dialog) {
     *     // for example, something like this:
     *     // something();
     *     // CPPUNIT_ASSERT(dialog.tabTo(...));
     *     // CPPUNIT_ASSERT(somethingElse);
     *     // dialog.postKeyEventAsync(0, awt::Key::RETURN);
     * });
     * CPPUNIT_ASSERT(activateMenuItem(u"Some menu", u"Some Item Triggering a Dialog..."));
     * CPPUNIT_ASSERT(waiter->waitEndDialog());
     * @endcode
     *
     * @note The user code might actually be executed before DialogWaiter::waitEndDialog() is
     *       called.  It is actually likely to be called at the time the call that triggers the
     *       dialog happens.  However, as letting an exception slip in a event handler is likely to
     *       cause problems, exceptions are forwarded to the DialogWaiter::waitEndDialog() call.
     *       However, note that you cannot rely on something like this:
     *       @code
     *       int foo = 0;
     *       auto waiter = awaitDialog(u"Some Dialog", [&foo](Dialog&) {
     *           CPPUNIT_ASSERT_EQUAL(1, foo);
     *       });
     *       CPPUNIT_ASSERT(activateMenuItem(u"Some menu", u"Some Item Triggering a Dialog..."));
     *       foo = 1; // here, the callback likely already ran as a result of the
     *                // Scheduler::ProcessEventsToIdle() call that activateMenuItem() did.
     *       CPPUNIT_ASSERT(waiter->waitEndDialog());
     *       @endcode
     *
     * @warning You should almost certainly always leave @p bAutoClose to @c true. If it is set to
     *          @c false, you have to take extreme care:
     *          - The dialog will not be canceled if the user code raises an exception.
     *          - If the dialog is run through Dialog::Execute(), control won't return to the test
     *            body until the dialog is closed.  This means that the only ways to execute code
     *            until then is a separate thread or via code dispatched by the main loop.
     *            Thus, you have to make sure you DO close the dialog some way or another yourself
     *            in order for the test code to terminate at some point.
     *          - If the dialog doesn't use Dialog::Execute() but is rather similar to a second
     *            separate window (e.g. non-modal), you might still have to close the dialog before
     *            closing the test document is possible without a CloseVetoException -- which might
     *            badly break the test run.
     */
    static std::shared_ptr<DialogWaiter> awaitDialog(const std::u16string_view name,
                                                     std::function<void(Dialog&)> callback,
                                                     bool bAutoClose = true);
#endif //defined(MACOSX)

public:
    virtual void setUp() override;
    virtual void tearDown() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
