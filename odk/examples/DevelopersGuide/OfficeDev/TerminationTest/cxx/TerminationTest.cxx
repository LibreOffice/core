/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>

#include <iostream>

using namespace ::cppu;
using namespace ::css::uno;
using namespace ::css::lang;
using namespace ::css::frame;

namespace
{
bool atWork = false;
}

class TerminateListener : public WeakImplHelper1<XTerminateListener>
{
public:
    // XTerminateListener
    virtual void SAL_CALL notifyTermination(const EventObject& event) SAL_OVERRIDE
    {
        std::cout << "About to terminate...\n";
    }

    virtual void SAL_CALL queryTermination(const EventObject& event) SAL_OVERRIDE
    {
        // Test if we can terminate now
        if (atWork)
        {
            std::cout << "Terminate while we are at work? You can't be serious ;-)!\n";
            throw TerminationVetoException();
        }
    }

    // XEventListener
    virtual void SAL_CALL disposing(const EventObject& event) SAL_OVERRIDE {}
};

SAL_IMPLEMENT_MAIN()
{
    try
    {
        // Connect to, or create, an instance of the Office.
        Reference<XComponentContext> xContext(bootstrap());
        std::cout << "Connected to a running office...\n";

        // Get a reference to the multi-component factory, and use it
        // to create a Desktop reference.
        Reference<XMultiComponentFactory> xMCF(xContext->getServiceManager());
        Reference<XDesktop> xDesktop(
            xMCF->createInstanceWithContext("com.sun.star.frame.Desktop", xContext),
            UNO_QUERY_THROW);

        // Create our termination request listener, and register it.
        TerminateListener listener;
        Reference<XTerminateListener> xTerminateListener(listener, UNO_QUERY_THROW);
        xDesktop->addTerminateListener(xTerminateListener);

        // Try to terminate while we are at work.
        atWork = true;
        bool terminated = xDesktop->terminate();
        std::cout << "The Office "
                  << (terminated ? "has been terminated" : "is still running, we are at work.")
                  << '\n';

        // Try to terminate when we are NOT at work.
        atWork = false;
        terminated = xDesktop->terminate();
        std::cout << "The Office "
                  << (terminated ? "has been terminated"
                                 : "is still running. Something else prevents termination,"
                                   "such as the quickstarter.")
                  << '\n';
    }
    catch (const RuntimeException& e)
    {
        std::cerr << e.Message << '\n';
        return 1;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
