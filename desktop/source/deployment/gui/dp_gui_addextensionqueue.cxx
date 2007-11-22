/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_addextensionqueue.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:00:28 $
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
#include "com/sun/star/task/XAbortChannel.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/TypeClass.hpp"
#include "osl/conditn.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "ucbhelper/content.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/anytostring.hxx"
#include "vcl/msgbox.hxx"


#include "dp_gui.h"
#include "dp_gui_thread.hxx"
#include "dp_gui_addextensionqueue.hxx"

#include <queue>


namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

using dp_gui::AddExtensionQueue;
using ::rtl::OUString;

namespace {

namespace css = com::sun::star;

}

class AddExtensionQueue::Thread: public dp_gui::Thread {
public:
    Thread(dp_gui::DialogImpl * pDialog ,
        css::uno::Reference< css::uno::XComponentContext > const & componentContext,
        css::uno::Reference< css::deployment::XPackageManager > const & packageManager
    );

    void addExtension(const ::rtl::OUString & extensionURL);

    void stop();

    bool hasTerminated();

private:
    Thread(Thread &); // not defined
    void operator =(Thread &); // not defined

    virtual ~Thread();

    virtual void execute();
    virtual void SAL_CALL onTerminated();


    enum Input { NONE, START, STOP };

    dp_gui::DialogImpl * m_pDialog;
    const css::uno::Reference<css::uno::XComponentContext > m_componentContext;
    const css::uno::Reference<css::deployment::XPackageManager > m_packageManager;
    OUString m_strAddingPackages;

    osl::Condition m_wakeup;
    osl::Mutex m_mutex;
    Input m_input;
    css::uno::Reference< css::task::XAbortChannel > m_abort;
    std::queue< ::rtl::OUString > m_queue;
    bool m_bTerminated;
    bool m_bStopped;
};

AddExtensionQueue::Thread::Thread(dp_gui::DialogImpl * pDialog ,
    css::uno::Reference< css::uno::XComponentContext > const & componentContext,
    css::uno::Reference< css::deployment::XPackageManager > const & packageManager
):
    m_pDialog(pDialog),
    m_componentContext(componentContext),
    m_packageManager(packageManager),
    m_strAddingPackages( dp_gui::DialogImpl::getResourceString(RID_STR_ADDING_PACKAGES) ),
    m_input(NONE),
    m_bTerminated(false),
    m_bStopped(false)
{
    OSL_ASSERT(pDialog);
}

void AddExtensionQueue::Thread::addExtension(const ::rtl::OUString & extensionURL)
{
    ::osl::MutexGuard g(m_mutex);
    //If someone called stop then we do not add the extensions- > game over!
    if (m_bStopped)
        return;
    if (extensionURL.getLength())
    {
        m_queue.push(extensionURL);
        m_input = START;
        m_wakeup.set();
    }
}

//Stopping this thread will not abort the installation of extensions.
void AddExtensionQueue::Thread::stop()
{
    osl::MutexGuard g1(m_mutex);
    m_bStopped = true;
    m_input = STOP;
    m_wakeup.set();
}
bool AddExtensionQueue::Thread::hasTerminated()
{
    ::osl::MutexGuard g(m_mutex);
    return m_bTerminated;
}

AddExtensionQueue::Thread::~Thread() {}

void AddExtensionQueue::Thread::execute()
{
    for (;;)
    {
        if (m_wakeup.wait() != osl::Condition::result_ok) {
            OSL_TRACE(
                "dp_gui::AddExtensionQueue::Thread::run: ignored "
                "osl::Condition::wait failure");
        }
        m_wakeup.reset();
        Input input;
        {
            osl::MutexGuard g(m_mutex);
            input = m_input;
            m_input = NONE;
        }
        // If this thread has been woken up by anything else except start, stop
        // then input is NONE and we wait again.
        if (input == NONE) {
            continue;
        }

        if (input == STOP) {
            break;
        }

        //If the user is running some task in the Extension Manager, that is he clicked
        //the add, update, options, etc. button, then we will not interfere. We proceed
        //only when that action is concluded.
        ::osl::MutexGuard actionGuard(ActionMutex::get());
        //disable all buttons
//        m_pDialog->m_bAddingExtensions = true;
//        m_pDialog->updateButtonStates();

        ::rtl::Reference<ProgressCommandEnv> currentCmdEnv(
            new ProgressCommandEnv(m_componentContext, m_pDialog, m_strAddingPackages, true) );

        css::uno::Reference<css::task::XAbortChannel> xAbortChannel(
            m_packageManager->createAbortChannel() );
        {
            osl::MutexGuard g(m_mutex);
            input = m_input;
            m_input = NONE;
            if (input == NONE || input == START) {
                //In case input would be STOP then we would later break out of the loop
                //before further calls to the XPackageManger are done. That is, the abort
                //channel would not be used anyway.
                m_abort = xAbortChannel;
            }
            else if (input == STOP) {
                break;
            }
            else
                OSL_ASSERT(0);
        }

        //We only install the extension which are currently in the queue.
        //The progressbar will be set to show the progress of the current number
        //of extensions. If we allowed to add extensions now then the progressbar may
        //have reached the end while we still install newly added extensions.
        int size = 0;
        {
            ::osl::MutexGuard queueGuard(m_mutex);
            size = m_queue.size();
        }
        if (size == 0)
            continue;

        //Do not lock the following part with addExtension. addExtension may be called in the main thread.
        //If the message box "Do you want to install the extension (or similar)" is shown and then
        //addExtension is called, which then blocks the main thread, then we deadlock.
        currentCmdEnv->showProgress(size);
        while (!currentCmdEnv->isAborted() && --size >= 0)
        {
            try
            {
                OUString file;
                {
                    ::osl::MutexGuard queueGuard2(m_mutex);
                    file = m_queue.front();
                    m_queue.pop();
                }

                css::uno::Any anyTitle =
                    ::ucbhelper::Content( file, currentCmdEnv.get() ).getPropertyValue(OUSTR("Title") );
                //check if we have a string in anyTitle. For example "unopkg gui \" caused anyTitle to be void
                //and anyTitle.get<OUString> throws as RuntimeException.
                OUString sTitle;
                if ( ! (anyTitle >>= sTitle))
                {
                    OSL_ENSURE(0, "Could not get file name for extension.");
                    continue;
                }
                currentCmdEnv->progressSection(sTitle, xAbortChannel );
                css::uno::Reference<css::deployment::XPackage> xPackage(
                    m_packageManager->addPackage(
                    file, OUString() /* detect media-type */,
                    xAbortChannel, currentCmdEnv.get() ) );
                OSL_ASSERT( xPackage.is() );
                m_pDialog->m_treelb->select(xPackage);
            }
            //catch (css::deployment::DeploymentException &)
            //{
            //}
            //catch (css::lang::IllegalArgumentException &)
            //{
            //}
            catch (css::ucb::CommandAbortedException &)
            {
                //This exception is thrown when the user clicks cancel on the progressbar.
                //Then we cancel the installation of all extensions and remove them from
                //the queue.
                {
                    ::osl::MutexGuard queueGuard2(m_mutex);
                    while (--size >= 0)
                        m_queue.pop();
                }
                break;
            }
            catch (css::ucb::CommandFailedException &)
            {
                //This exception is thrown when a user clicked cancel in the messagebox which was
                //startet by the interaction handler. For example the user will be asked if he/she
                //really wants to install the extension.
                //These interaction are run for exectly one extension at a time. Therefore we continue
                //with installing the remaining extensions.
                continue;
            }
            catch (css::uno::Exception &)
            {
                //Todo display the user an error
                //see also DialogImpl::SyncPushButton::Click()
                css::uno::Any exc( ::cppu::getCaughtException() );
                OUString msg;
                css::deployment::DeploymentException dpExc;
                if ((exc >>= dpExc) &&
                    dpExc.Cause.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
                {
                    // notify error cause only:
                    msg = reinterpret_cast<css::uno::Exception const *>(
                        dpExc.Cause.getValue() )->Message;
                }
                if (msg.getLength() == 0) // fallback for debugging purposes
                    msg = ::comphelper::anyToString(exc);

                const ::vos::OGuard guard( Application::GetSolarMutex() );
                ::std::auto_ptr<ErrorBox> box(
                    new ErrorBox( currentCmdEnv->activeDialog(), WB_OK, msg ) );
                box->SetText( m_pDialog->GetText() );
                box->Execute();
                    //Continue with installation of the remaining extensions
            }

        }
        //end while
        //enable all buttons
//         m_pDialog->m_bAddingExtensions = false;
//         m_pDialog->updateButtonStates();

    }
    //end for
    //enable all buttons
    //ToDo: Investigate why calling updateButtonStates causes a deadlock when pressing
    //the close button. updateButtonSTates uses the solar mutex.
//     m_pDialog->m_bAddingExtensions = false;
//     m_pDialog->updateButtonStates();
}


void AddExtensionQueue::Thread::onTerminated()
{
    ::osl::MutexGuard g(m_mutex);
    m_bTerminated = true;
}


AddExtensionQueue::AddExtensionQueue(dp_gui::DialogImpl * pDialogImpl,
    css::uno::Reference<css::uno::XComponentContext > const & componentContext,
    css::uno::Reference< css::deployment::XPackageManager > const & packageManager):
    m_thread(new Thread(pDialogImpl,componentContext, packageManager))
{
    m_thread->launch();
}

AddExtensionQueue::~AddExtensionQueue() {
    stop();
}

void AddExtensionQueue::addExtension(const ::rtl::OUString & extensionURL)
{
    m_thread->addExtension(extensionURL);
}

void AddExtensionQueue::stop()
{
    m_thread->stop();
}

void AddExtensionQueue::stopAndWait()
{
    m_thread->stop();
    m_thread->join();
}

bool AddExtensionQueue::hasTerminated()
{
    return m_thread->hasTerminated();
}
