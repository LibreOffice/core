/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updatability.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:01:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/deployment/UpdateInformationProvider.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/XUpdateInformationProvider.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vos/mutex.hxx"

#include "dp_misc.h"
#include "dp_gui_thread.hxx"
#include "dp_gui_updatability.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

using dp_gui::Updatability;

namespace {

namespace css = com::sun::star;

}

class Updatability::Thread: public dp_gui::Thread {
public:
    Thread(
        css::uno::Sequence< css::uno::Reference<
            css::deployment::XPackageManager > > const & packageManagers,
        Window & enabled);

    void start();

    void stop();

private:
    Thread(Thread &); // not defined
    void operator =(Thread &); // not defined

    virtual ~Thread();

    virtual void execute();

    enum Input { NONE, START, STOP };

    bool m_predeterminedUpdateUrl;
    css::uno::Sequence< css::uno::Reference<
        css::deployment::XPackageManager > > m_packageManagers;

    osl::Condition m_wakeup;
    osl::Mutex m_mutex;
    Window * m_enabled;
    Input m_input;
    css::uno::Reference< css::task::XAbortChannel > m_abort;
};

Updatability::Thread::Thread(
    css::uno::Sequence< css::uno::Reference<
        css::deployment::XPackageManager > > const & packageManagers,
    Window & enabled):
    m_predeterminedUpdateUrl(dp_misc::getExtensionDefaultUpdateURL().getLength() > 0),
    m_packageManagers(packageManagers),
    m_enabled(&enabled),
    m_input(NONE)
{}

void Updatability::Thread::start() {
    css::uno::Reference< css::task::XAbortChannel > abort;
    {
        osl::MutexGuard g(m_mutex);
        m_input = START;
        abort = m_abort;
        m_abort.clear();
    }
    m_wakeup.set();
    if (abort.is()) {
        abort->sendAbort();
    }
}

void Updatability::Thread::stop() {
    css::uno::Reference< css::task::XAbortChannel > abort;
    {
        vos::OGuard g1(Application::GetSolarMutex());
        osl::MutexGuard g2(m_mutex);
        m_input = STOP;
        m_enabled = NULL;
        abort = m_abort;
        m_abort.clear();
    }
    m_wakeup.set();
    if (abort.is()) {
        abort->sendAbort();
    }
}

Updatability::Thread::~Thread() {}

void Updatability::Thread::execute() {
    for (;;) {
        if (m_wakeup.wait() != osl::Condition::result_ok) {
            OSL_TRACE(
                "dp_gui::Updatability::Thread::run: ignored "
                "osl::Condition::wait failure");
        }
        m_wakeup.reset();
        Input input;
        {
            osl::MutexGuard g(m_mutex);
            input = m_input;
            m_input = NONE;
        }
        if (input == NONE) {
            continue;
        }
    start:
        if (input == STOP) {
            break;
        }
        bool enabled = false;
        for (sal_Int32 i = 0; !enabled && i < m_packageManagers.getLength();
             ++i)
        {
            css::uno::Reference< css::task::XAbortChannel > abort(
                m_packageManagers[i]->createAbortChannel());
            {
                osl::MutexGuard g(m_mutex);
                input = m_input;
                m_input = NONE;
                if (input == NONE) {
                    //In case input would be STOP then we would later break out of the loop
                    //before further calls to the XPackageManger are done. That is, the abort
                    //channel would not be used anyway.
                    m_abort = abort;
                }
                if (input != NONE) {
                    goto start;
                }
            }
            css::uno::Sequence<
                css::uno::Reference< css::deployment::XPackage > > ps;
            try {
                ps = m_packageManagers[i]->getDeployedPackages(
                    abort,
                    css::uno::Reference< css::ucb::XCommandEnvironment >());
            } catch (css::deployment::DeploymentException &) {
                // If there are any problematic package managers, enable the
                // update button and let the update process report any problems
                // to the user:
                enabled = true;
                continue;
            } catch (css::ucb::CommandFailedException &) {
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "CommandFailedException: cannot happen")),
                    css::uno::Reference< css::uno::XInterface >());
            } catch (css::ucb::CommandAbortedException &) {
                osl::MutexGuard g(m_mutex);
                input = m_input;
                m_input = NONE;
                OSL_ASSERT(input != NONE);
                goto start;
            } catch (css::lang::IllegalArgumentException &) {
                throw css::uno::RuntimeException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "IllegalArgumentException: cannot happen")),
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (m_predeterminedUpdateUrl && ps.getLength() != 0) {
                enabled = true;
            } else {
                for (sal_Int32 j = 0; j < ps.getLength(); ++j) {
                    if (ps[j]->getUpdateInformationURLs().getLength() != 0) {
                        enabled = true;
                        break;
                    }
                    {
                        osl::MutexGuard g(m_mutex);
                        input = m_input;
                        m_input = NONE;
                    }
                    if (input != NONE) {
                        goto start;
                    }
                }
            }
        }
        vos::OGuard g1(Application::GetSolarMutex());
        Window * e;
        {
            osl::MutexGuard g2(m_mutex);
            e = m_enabled;
        }
        if (e != NULL) {
            e->Enable(enabled);
        }
    }
}

Updatability::Updatability(
    css::uno::Sequence<
        css::uno::Reference< css::deployment::XPackageManager > > const &
        packageManagers,
    Window & enabled):
    m_thread(new Thread(packageManagers, enabled))
{
    m_thread->launch();
}

Updatability::~Updatability() {

}

void Updatability::start() {
    m_thread->start();
}

void Updatability::stop() {
    m_thread->stop();
    // Bad hack; m_thread calls Application::GetSolarMutex, which only works
    // as long as DeInitVCL has not been called:
    ULONG n = Application::ReleaseSolarMutex();
    m_thread->join();
    Application::AcquireSolarMutex(n);
}
