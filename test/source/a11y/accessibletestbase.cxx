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
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <vcl/idle.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>

#include <cppuhelper/implbase.hxx>

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
    mxDocument = mxDesktop->loadComponentFromURL(sURL, "_blank", frame::FrameSearchFlag::AUTO, {});

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
                for (const uno::Reference<accessibility::XAccessible>& targetAccessible :
                     rel.TargetSet)
                {
                    if (targetAccessible.is())
                        return targetAccessible->getAccessibleContext();
                }
            }
        }
    }

    return nullptr;
}

std::deque<uno::Reference<accessibility::XAccessibleContext>>
test::AccessibleTestBase::getAllChildren(
    const uno::Reference<accessibility::XAccessibleContext>& xContext)
{
    std::deque<uno::Reference<accessibility::XAccessibleContext>> children;
    auto childCount = xContext->getAccessibleChildCount();

    for (sal_Int64 i = 0; i < childCount && i < AccessibilityTools::MAX_CHILDREN; i++)
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
    /* relation set is not included in AccessibilityTools::debugString(), but might be useful in
     * this context, so we compute it here */
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
                for (const uno::Reference<accessibility::XAccessible>& xTarget : rel.TargetSet)
                {
                    if (j++ > 0)
                        std::cout << ", ";
                    std::cout << AccessibilityTools::debugString(xTarget);
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

/** Gets a child by name (usually in a menu) */
uno::Reference<accessibility::XAccessibleContext> test::AccessibleTestBase::getItemFromName(
    const uno::Reference<accessibility::XAccessibleContext>& xMenuCtx, std::u16string_view name)
{
    auto childCount = xMenuCtx->getAccessibleChildCount();

    std::cout << "looking up item " << OUString(name) << " in "
              << AccessibilityTools::debugString(xMenuCtx) << std::endl;
    for (sal_Int64 i = 0; i < childCount && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto item = xMenuCtx->getAccessibleChild(i)->getAccessibleContext();
        if (AccessibilityTools::nameEquals(item, name))
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

uno::Reference<accessibility::XAccessibleContext> test::AccessibleTestBase::getFocusedObject(
    const uno::Reference<accessibility::XAccessibleContext>& xCtx)
{
    return AccessibilityTools::getAccessibleObjectForPredicate(
        xCtx, [](const uno::Reference<accessibility::XAccessibleContext>& xCandidateCtx) {
            const auto states = (accessibility::AccessibleStateType::FOCUSED
                                 | accessibility::AccessibleStateType::SHOWING);
            return (xCandidateCtx->getAccessibleStateSet() & states) == states;
        });
}

uno::Reference<accessibility::XAccessibleContext>
test::AccessibleTestBase::tabTo(const uno::Reference<accessibility::XAccessible>& xRoot,
                                const sal_Int16 role, const std::u16string_view name,
                                const EventPosterHelperBase* pEventPosterHelper)
{
    AccessibleEventPosterHelper eventHelper;
    if (!pEventPosterHelper)
    {
        eventHelper.setWindow(xRoot);
        pEventPosterHelper = &eventHelper;
    }

    auto xOriginalFocus = getFocusedObject(xRoot);
    auto xFocus = xOriginalFocus;
    int nSteps = 0;

    std::cout << "Tabbing to '" << OUString(name) << "'..." << std::endl;
    while (xFocus && (nSteps == 0 || xFocus != xOriginalFocus))
    {
        std::cout << "  focused object is: " << AccessibilityTools::debugString(xFocus)
                  << std::endl;
        if (xFocus->getAccessibleRole() == role && AccessibilityTools::nameEquals(xFocus, name))
        {
            std::cout << "  -> OK, focus matches" << std::endl;
            return xFocus;
        }
        if (++nSteps > 100)
        {
            std::cerr << "Object not found after tabbing 100 times! bailing out" << std::endl;
            break;
        }

        std::cout << "  -> no match, sending <TAB>" << std::endl;
        pEventPosterHelper->postKeyEventAsync(0, awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();

        const auto xPrevFocus = xFocus;
        xFocus = getFocusedObject(xRoot);
        if (!xFocus)
            std::cerr << "Focus lost after sending <TAB>!" << std::endl;
        else if (xPrevFocus == xFocus)
        {
            std::cerr << "Focus didn't move after sending <TAB>! bailing out" << std::endl;
            std::cerr << "Focused object(s):" << std::endl;
            int iFocusedCount = 0;
            // count and print out objects with focused state
            AccessibilityTools::getAccessibleObjectForPredicate(
                xRoot,
                [&iFocusedCount](const uno::Reference<accessibility::XAccessibleContext>& xCtx) {
                    const auto states = (accessibility::AccessibleStateType::FOCUSED
                                         | accessibility::AccessibleStateType::SHOWING);
                    if ((xCtx->getAccessibleStateSet() & states) == states)
                    {
                        std::cerr << " * " << AccessibilityTools::debugString(xCtx) << std::endl;
                        iFocusedCount++;
                    }
                    return false; // keep going
                });
            std::cerr << "Total focused element(s): " << iFocusedCount << std::endl;
            if (iFocusedCount > 1)
                std::cerr << "WARNING: there are more than one focused object! This usually means "
                             "there is a BUG in the focus handling of that accessibility tree."
                          << std::endl;
            break;
        }
    }

    std::cerr << "NOT FOUND" << std::endl;
    return nullptr;
}

bool test::AccessibleTestBase::tabTo(
    const uno::Reference<accessibility::XAccessible>& xRoot,
    const uno::Reference<accessibility::XAccessibleContext>& xChild,
    const EventPosterHelperBase* pEventPosterHelper)
{
    AccessibleEventPosterHelper eventHelper;
    if (!pEventPosterHelper)
    {
        eventHelper.setWindow(xRoot);
        pEventPosterHelper = &eventHelper;
    }

    std::cout << "Tabbing to " << AccessibilityTools::debugString(xChild) << "..." << std::endl;
    for (int i = 0; i < 100; i++)
    {
        if (xChild->getAccessibleStateSet() & accessibility::AccessibleStateType::FOCUSED)
            return true;

        std::cout << "  no match, sending <TAB>" << std::endl;
        pEventPosterHelper->postKeyEventAsync(0, awt::Key::TAB);
        Scheduler::ProcessEventsToIdle();
    }

    std::cerr << "NOT FOUND" << std::endl;
    return false;
}

#if !defined(MACOSX)
/* Dialog handling
 *
 * For now this doesn't actually work under macos, so the API is not available there not to create
 * confusion.  The problem there is we don't get notified of new dialogs, so we can't manage them
 * or interact with them.
 */

test::AccessibleTestBase::Dialog::Dialog(uno::Reference<awt::XDialog2>& xDialog2, bool bAutoClose)
    : mbAutoClose(bAutoClose)
    , mxDialog2(xDialog2)
{
    CPPUNIT_ASSERT(xDialog2.is());

    mxAccessible.set(xDialog2, uno::UNO_QUERY);
    if (mxAccessible)
        setWindow(mxAccessible);
    else
    {
        std::cerr << "WARNING: AccessibleTestBase::Dialog() constructed with awt::XDialog2 '"
                  << xDialog2->getTitle()
                  << "' not implementing accessibility::XAccessible. Event delivery will not work."
                  << std::endl;
    }
}

test::AccessibleTestBase::Dialog::~Dialog()
{
    if (mbAutoClose)
        close();
}

void test::AccessibleTestBase::Dialog::close(sal_Int32 result)
{
    if (mxDialog2)
    {
        mxDialog2->endDialog(result);
        mxDialog2.clear();
    }
}

std::shared_ptr<test::AccessibleTestBase::DialogWaiter>
test::AccessibleTestBase::awaitDialog(const std::u16string_view name,
                                      std::function<void(Dialog&)> callback, bool bAutoClose)
{
    /* Helper class to wait on a dialog to pop up and to close, running user code between the
     * two.  This has to work both for "other window"-style dialogues (non-modal), as well as
     * for modal dialogues using Dialog::Execute() (which runs a nested main loop, hence
     * blocking our test flow execution.
     * The approach here is to wait on the WindowActivate event for the dialog, and run the
     * test code in there. Then, close the dialog if not already done, resuming normal flow to
     * the caller. */
    class ListenerHelper : public DialogWaiter
    {
        DialogCancelMode miPreviousDialogCancelMode;
        uno::Reference<awt::XExtendedToolkit> mxToolkit;
        bool mbWaitingForDialog;
        std::exception_ptr mpException;
        std::u16string_view msName;
        std::function<void(Dialog&)> mCallback;
        bool mbAutoClose;
        Timer maTimeoutTimer;
        Idle maIdleHandler;
        uno::Reference<awt::XTopWindowListener> mxTopWindowListener;
        std::unique_ptr<Dialog> mxDialog;

    public:
        virtual ~ListenerHelper()
        {
            Application::SetDialogCancelMode(miPreviousDialogCancelMode);
            mxToolkit->removeTopWindowListener(mxTopWindowListener);
            maTimeoutTimer.Stop();
            maIdleHandler.Stop();
        }

        ListenerHelper(const std::u16string_view& name, std::function<void(Dialog&)> callback,
                       bool bAutoClose)
            : mbWaitingForDialog(true)
            , msName(name)
            , mCallback(std::move(callback))
            , mbAutoClose(bAutoClose)
            , maTimeoutTimer("workaround timer if we don't catch WindowActivate")
            , maIdleHandler("runs user callback in idle time")
        {
            mxTopWindowListener.set(new MyTopWindowListener(this));
            mxToolkit.set(Application::GetVCLToolkit(), uno::UNO_QUERY_THROW);
            mxToolkit->addTopWindowListener(mxTopWindowListener);

            maTimeoutTimer.SetInvokeHandler(LINK(this, ListenerHelper, timeoutTimerHandler));
            maTimeoutTimer.SetTimeout(60000);
            maTimeoutTimer.Start();

            maIdleHandler.SetInvokeHandler(LINK(this, ListenerHelper, idleHandler));
            maIdleHandler.SetPriority(TaskPriority::DEFAULT_IDLE);

            miPreviousDialogCancelMode = Application::GetDialogCancelMode();
            Application::SetDialogCancelMode(DialogCancelMode::Off);
        }

    private:
        // mimic IMPL_LINK inline
        static void LinkStubtimeoutTimerHandler(void* instance, Timer* timer)
        {
            static_cast<ListenerHelper*>(instance)->timeoutTimerHandler(timer);
        }

        void timeoutTimerHandler(Timer*)
        {
            std::cerr << "timeout waiting for dialog '" << OUString(msName) << "' to show up"
                      << std::endl;

            assert(mbWaitingForDialog);

            // This is not very nice, but it should help fail earlier if we never catch the dialog
            // yet we're in a sub-loop and waitEndDialog() didn't have a chance to run yet.
            throw new css::uno::RuntimeException("Timeout waiting for dialog");
        }

        class MyTopWindowListener : public ::cppu::WeakImplHelper<awt::XTopWindowListener>
        {
        private:
            ListenerHelper* mpHelper;

        public:
            MyTopWindowListener(ListenerHelper* pHelper)
                : mpHelper(pHelper)
            {
                assert(mpHelper);
            }

            // XTopWindowListener
            virtual void SAL_CALL windowOpened(const lang::EventObject&) override {}
            virtual void SAL_CALL windowClosing(const lang::EventObject&) override {}
            virtual void SAL_CALL windowClosed(const lang::EventObject&) override {}
            virtual void SAL_CALL windowMinimized(const lang::EventObject&) override {}
            virtual void SAL_CALL windowNormalized(const lang::EventObject&) override {}
            virtual void SAL_CALL windowDeactivated(const lang::EventObject&) override {}
            virtual void SAL_CALL windowActivated(const lang::EventObject& xEvent) override
            {
                assert(mpHelper->mbWaitingForDialog);

                if (!xEvent.Source)
                    return;

                uno::Reference<awt::XDialog2> xDialog(xEvent.Source, uno::UNO_QUERY);
                if (!xDialog)
                    return;

                // remove ourselves, we don't want to run again
                mpHelper->mxToolkit->removeTopWindowListener(this);

                mpHelper->mxDialog = std::make_unique<Dialog>(xDialog, true);

                mpHelper->maIdleHandler.Start();
            }

            // XEventListener
            virtual void SAL_CALL disposing(const lang::EventObject&) override {}
        };

        // mimic IMPL_LINK inline
        static void LinkStubidleHandler(void* instance, Timer* idle)
        {
            static_cast<ListenerHelper*>(instance)->idleHandler(idle);
        }

        void idleHandler(Timer*)
        {
            mbWaitingForDialog = false;

            maTimeoutTimer.ClearInvokeHandler();
            maTimeoutTimer.Stop();

            /* The popping up dialog ought to be the right one, or something's fishy and
             * we're bound to failure (e.g. waiting on a dialog that either will never come, or
             * that will not run after the current one -- deadlock style) */
            if (msName != mxDialog->getWindow()->GetText())
            {
                mpException = std::make_exception_ptr(css::uno::RuntimeException(
                    "Unexpected dialog '" + mxDialog->getWindow()->GetText()
                    + "' opened instead of the expected '" + msName + "'"));
            }
            else
            {
                std::cout << "found dialog, calling user callback" << std::endl;

                // set the real requested auto close now we're just calling the user callback
                mxDialog->setAutoClose(mbAutoClose);

                try
                {
                    mCallback(*mxDialog);
                }
                catch (...)
                {
                    mpException = std::current_exception();
                }
            }

            mxDialog.reset();
        }

    public:
        virtual bool waitEndDialog(sal_uInt64 nTimeoutMs) override
        {
            /* Usually this loop will actually never run at all because a previous
             * Scheduler::ProcessEventsToIdle() would have triggered the dialog already, but we
             * can't be sure of that or of delays, so be safe and wait with a timeout. */
            if (mbWaitingForDialog)
            {
                Timer aTimer("wait for dialog");
                aTimer.SetTimeout(nTimeoutMs);
                aTimer.Start();
                do
                {
                    Application::Yield();
                } while (mbWaitingForDialog && aTimer.IsActive());
            }

            if (mpException)
                std::rethrow_exception(mpException);

            return !mbWaitingForDialog;
        }
    };

    return std::make_shared<ListenerHelper>(name, callback, bAutoClose);
}
#endif //defined(MACOSX)

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
