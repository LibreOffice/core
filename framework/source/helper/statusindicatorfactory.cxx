/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <algorithm>
#include <utility>
#include <helper/statusindicatorfactory.hxx>
#include <helper/statusindicator.hxx>
#include <helper/vclstatusindicator.hxx>
#include <properties.h>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager2.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <vcl/svapp.hxx>
#include <mutex>
#include <rtl/ref.hxx>

#include <officecfg/Office/Common.hxx>

namespace framework{

sal_Int32 StatusIndicatorFactory::m_nInReschedule = 0;  ///< static counter for rescheduling

constexpr OUString PROGRESS_RESOURCE = u"private:resource/progressbar/progressbar"_ustr;

StatusIndicatorFactory::StatusIndicatorFactory(css::uno::Reference< css::uno::XComponentContext >  xContext)
    : m_xContext          (std::move(xContext ))
    , m_bAllowReschedule  (false)
    , m_bAllowParentShow  (false)
    , m_bDisableReschedule(false)
{
}

StatusIndicatorFactory::~StatusIndicatorFactory()
{
    impl_stopWakeUpThread();
}

void SAL_CALL StatusIndicatorFactory::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
{
    if (lArguments.hasElements()) {
        std::scoped_lock g(m_mutex);

        css::uno::Reference< css::frame::XFrame > xTmpFrame;
        css::uno::Reference< css::awt::XWindow > xTmpWindow;
        bool b1 = lArguments[0] >>= xTmpFrame;
        bool b2 = lArguments[0] >>= xTmpWindow;
        if (lArguments.getLength() == 3 && b1) {
           // it's the first service constructor "createWithFrame"
            m_xFrame = xTmpFrame;
            lArguments[1] >>= m_bDisableReschedule;
            lArguments[2] >>= m_bAllowParentShow;
        } else if (lArguments.getLength() == 3 && b2) {
           // it's the second service constructor "createWithWindow"
            m_xPluggWindow = xTmpWindow;
            lArguments[1] >>= m_bDisableReschedule;
            lArguments[2] >>= m_bAllowParentShow;
        } else {
           // it's an old-style initialisation using properties
            ::comphelper::SequenceAsHashMap lArgs(lArguments);

            m_xFrame             = lArgs.getUnpackedValueOrDefault("Frame"            , css::uno::Reference< css::frame::XFrame >());
            m_xPluggWindow       = lArgs.getUnpackedValueOrDefault("Window"           , css::uno::Reference< css::awt::XWindow >() );
            m_bAllowParentShow   = lArgs.getUnpackedValueOrDefault("AllowParentShow"  , false );
            m_bDisableReschedule = lArgs.getUnpackedValueOrDefault("DisableReschedule", false );
       }
    }

#ifdef EMSCRIPTEN
    m_bDisableReschedule = true;
#endif
    impl_createProgress();
}

css::uno::Reference< css::task::XStatusIndicator > SAL_CALL StatusIndicatorFactory::createStatusIndicator()
{
    return new StatusIndicator(this);
}

void SAL_CALL StatusIndicatorFactory::update()
{
    std::scoped_lock g(m_mutex);
    m_bAllowReschedule = true;
}

void StatusIndicatorFactory::start(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                   const OUString&                                    sText ,
                                         sal_Int32                                           nRange)
{
    css::uno::Reference< css::task::XStatusIndicator > xProgress;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aWriteLock(m_mutex);

        // create new info structure for this child or move it to the front of our stack
        IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
        if (pItem != m_aStack.end())
            m_aStack.erase(pItem);
        IndicatorInfo aInfo(xChild, sText);
        m_aStack.push_back (aInfo                );

        m_xActiveChild = xChild;
        xProgress = m_xProgress;
    }
    // <- SAFE ----------------------------------

    implts_makeParentVisibleIfAllowed();

    if (xProgress.is())
        xProgress->start(sText, nRange);

    impl_startWakeUpThread();
    impl_reschedule(true);
}

void StatusIndicatorFactory::reset(const css::uno::Reference< css::task::XStatusIndicator >& xChild)
{
    css::uno::Reference< css::task::XStatusIndicator > xActive;
    css::uno::Reference< css::task::XStatusIndicator > xProgress;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aReadLock(m_mutex);

        // reset the internal info structure related to this child
        IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
        if (pItem != m_aStack.end())
        {
            pItem->m_nValue = 0;
            pItem->m_sText.clear();
        }

        xActive   = m_xActiveChild;
        xProgress = m_xProgress;
    }
    // <- SAFE ----------------------------------

    // not the top most child => don't change UI
    // But don't forget Reschedule!
    if (
        (xChild == xActive) &&
        (xProgress.is()   )
       )
        xProgress->reset();

    impl_reschedule(true);
}

void StatusIndicatorFactory::end(const css::uno::Reference< css::task::XStatusIndicator >& xChild)
{
    css::uno::Reference< css::task::XStatusIndicator > xActive;
    css::uno::Reference< css::task::XStatusIndicator > xProgress;
    OUString sText;
    sal_Int32 nValue = 0;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aWriteLock(m_mutex);

        // remove this child from our stack
        IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
        if (pItem != m_aStack.end())
            m_aStack.erase(pItem);

        // activate next child ... or finish the progress if there is no further one.
        m_xActiveChild.clear();
        IndicatorStack::reverse_iterator pNext  = m_aStack.rbegin();
        if (pNext != m_aStack.rend())
        {
            m_xActiveChild = pNext->m_xIndicator;
            sText          = pNext->m_sText;
            nValue         = pNext->m_nValue;
        }

        xActive   = m_xActiveChild;
        xProgress = m_xProgress;
    }
    // <- SAFE ----------------------------------

    if (xActive.is())
    {
        // There is at least one further child indicator.
        // Actualize our progress, so it shows these values from now.
        if (xProgress.is())
        {
            xProgress->setText (sText );
            xProgress->setValue(nValue);
        }
    }
    else
    {
        // Our stack is empty. No further child exists.
        // Se we must "end" our progress really
        if (xProgress.is())
            xProgress->end();
        // Now hide the progress bar again.
        impl_hideProgress();

        impl_stopWakeUpThread();
    }

    impl_reschedule(true);
}

void StatusIndicatorFactory::setText(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                     const OUString&                                    sText )
{
    css::uno::Reference< css::task::XStatusIndicator > xActive;
    css::uno::Reference< css::task::XStatusIndicator > xProgress;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aWriteLock(m_mutex);

        IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
        if (pItem != m_aStack.end())
            pItem->m_sText = sText;

        xActive   = m_xActiveChild;
        xProgress = m_xProgress;
    }
    // SAFE -> ----------------------------------

    // paint only the top most indicator
    // but don't forget to Reschedule!
    if (
        (xChild == xActive) &&
        (xProgress.is()   )
       )
    {
        xProgress->setText(sText);
    }

    impl_reschedule(true);
}

void StatusIndicatorFactory::setValue( const css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                             sal_Int32                                           nValue )
{
    sal_Int32 nOldValue = 0;
    css::uno::Reference< css::task::XStatusIndicator > xActive;
    css::uno::Reference< css::task::XStatusIndicator > xProgress;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aWriteLock(m_mutex);

        IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
        if (pItem != m_aStack.end())
        {
            nOldValue       = pItem->m_nValue;
            pItem->m_nValue = nValue;
        }

        xActive    = m_xActiveChild;
        xProgress  = m_xProgress;
    }
    // SAFE -> ----------------------------------

    if (
        (xChild    == xActive) &&
        (nOldValue != nValue ) &&
        (xProgress.is()      )
       )
    {
        xProgress->setValue(nValue);
    }

    impl_reschedule(false);
}

void StatusIndicatorFactory::implts_makeParentVisibleIfAllowed()
{
    css::uno::Reference< css::frame::XFrame > xFrame;
    css::uno::Reference< css::awt::XWindow >  xPluggWindow;
    css::uno::Reference< css::uno::XComponentContext > xContext;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aReadLock(m_mutex);

        if (!m_bAllowParentShow)
            return;

        xFrame = m_xFrame;
        xPluggWindow = m_xPluggWindow;
        xContext = m_xContext;
    }
    // <- SAFE ----------------------------------

    css::uno::Reference< css::awt::XWindow > xParentWindow;
    if (xFrame.is())
        xParentWindow = xFrame->getContainerWindow();
    else
        xParentWindow = xPluggWindow;

    // don't disturb user in case he put the loading document into the background!
    // Suppress any setVisible() or toFront() call in case the initial show was
    // already made.
    css::uno::Reference< css::awt::XWindow2 > xVisibleCheck(xParentWindow, css::uno::UNO_QUERY);
    bool bIsVisible = false;
    if (xVisibleCheck.is())
        bIsVisible = xVisibleCheck->isVisible();

    if (bIsVisible)
    {
        impl_showProgress();
        return;
    }

    // Check if the layout manager has been set to invisible state. It this case we are also
    // not allowed to set the frame visible!
    css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
        xPropSet->getPropertyValue(FRAME_PROPNAME_ASCII_LAYOUTMANAGER) >>= xLayoutManager;
        if (xLayoutManager.is())
        {
            if ( !xLayoutManager->isVisible() )
                return;
        }
    }

    // Ok the window should be made visible... because it is not currently visible.
    // BUT..!
    // We need a Hack for our applications: They get her progress from the frame directly
    // on saving documents. Because there is no progress set on the MediaDescriptor.
    // But that's wrong. In case the document was opened hidden, they should not use any progress .-(
    // They only possible workaround: don't show the parent window here, if the document was opened hidden.
    bool bHiddenDoc = false;
    if (xFrame.is())
    {
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel >      xModel;
        xController = xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();
        if (xModel.is())
        {
            utl::MediaDescriptor lDocArgs(xModel->getArgs());
            bHiddenDoc = lDocArgs.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_HIDDEN,
                false);
        }
    }

    if (bHiddenDoc)
        return;

    // OK: The document was not opened in hidden mode ...
    // and the window isn't already visible.
    // Show it and bring it to front.
    // But before we have to be sure, that our internal used helper progress
    // is visible too.
    impl_showProgress();

    SolarMutexGuard aSolarGuard;
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( pWindow )
    {
        bool bForceFrontAndFocus(officecfg::Office::Common::View::NewDocumentHandling::ForceFocusAndToFront::get());
        pWindow->Show(true, bForceFrontAndFocus ? ShowFlags::ForegroundTask : ShowFlags::NONE );
    }

}

void StatusIndicatorFactory::impl_createProgress()
{
    css::uno::Reference< css::frame::XFrame > xFrame;
    css::uno::Reference< css::awt::XWindow > xWindow;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aReadLock(m_mutex);

        xFrame = m_xFrame;
        xWindow = m_xPluggWindow;
    }
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xProgress;

    if (xWindow.is())
    {
        // use vcl based progress implementation in plugged mode
        xProgress = new VCLStatusIndicator(xWindow);
    }
    else if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_ASCII_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                xLayoutManager->lock();
                OUString sPROGRESS_RESOURCE(PROGRESS_RESOURCE);
                xLayoutManager->createElement( sPROGRESS_RESOURCE );
                xLayoutManager->hideElement( sPROGRESS_RESOURCE );

                css::uno::Reference< css::ui::XUIElement > xProgressBar = xLayoutManager->getElement(sPROGRESS_RESOURCE);
                if (xProgressBar.is())
                    xProgress.set(xProgressBar->getRealInterface(), css::uno::UNO_QUERY);
                xLayoutManager->unlock();
            }
        }
    }

    std::scoped_lock g(m_mutex);
    m_xProgress = xProgress;
}

void StatusIndicatorFactory::impl_showProgress()
{
    css::uno::Reference< css::frame::XFrame > xFrame;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aReadLock(m_mutex);

        xFrame = m_xFrame;
    }
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xProgress;

    if (!xFrame.is())
        return;

    // use frame layouted progress implementation
    css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
        xPropSet->getPropertyValue(FRAME_PROPNAME_ASCII_LAYOUTMANAGER) >>= xLayoutManager;
        if (xLayoutManager.is())
        {
            // Be sure that we have always a progress. It can be that our frame
            // was recycled and therefore the progress was destroyed!
            // CreateElement does nothing if there is already a valid progress.
            OUString sPROGRESS_RESOURCE(PROGRESS_RESOURCE);
            xLayoutManager->createElement( sPROGRESS_RESOURCE );
            xLayoutManager->showElement( sPROGRESS_RESOURCE );

            css::uno::Reference< css::ui::XUIElement > xProgressBar = xLayoutManager->getElement(sPROGRESS_RESOURCE);
            if (xProgressBar.is())
                xProgress.set(xProgressBar->getRealInterface(), css::uno::UNO_QUERY);
        }
    }

    std::scoped_lock g(m_mutex);
    m_xProgress = xProgress;
}

void StatusIndicatorFactory::impl_hideProgress()
{
    css::uno::Reference< css::frame::XFrame > xFrame;
    // SAFE -> ----------------------------------
    {
        std::scoped_lock aReadLock(m_mutex);

        xFrame = m_xFrame;
    }
    // <- SAFE ----------------------------------

    if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_ASCII_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
                xLayoutManager->hideElement( PROGRESS_RESOURCE );
        }
    }
}

void StatusIndicatorFactory::impl_reschedule(bool bForce)
{
    // SAFE ->
    {
        std::scoped_lock aReadLock(m_mutex);
        if (m_bDisableReschedule)
            return;
    }
    // <- SAFE

    bool bReschedule = bForce;
    if (!bReschedule)
    {
        std::scoped_lock g(m_mutex);
        bReschedule        = m_bAllowReschedule;
        m_bAllowReschedule = false;
    }

    if (!bReschedule)
        return;

    static std::mutex rescheduleLock;
    // SAFE ->
    std::unique_lock aRescheduleGuard(rescheduleLock);

    if (m_nInReschedule != 0)
        return;

    // coverity[missing_lock: FALSE] - coverity fails to see the aRescheduleGuard ctor as taking a lock
    ++m_nInReschedule;
    aRescheduleGuard.unlock();
    // <- SAFE

    {
        SolarMutexGuard g;
        Application::Reschedule(true);
    }

    // SAFE ->
    aRescheduleGuard.lock();
    --m_nInReschedule;
}

void StatusIndicatorFactory::impl_startWakeUpThread()
{
    std::scoped_lock g(m_mutex);

    if (m_bDisableReschedule)
        return;

    if (!m_pWakeUp)
        m_pWakeUp.reset(new WakeUpThread(this));
}

void StatusIndicatorFactory::impl_stopWakeUpThread()
{
    std::unique_ptr<WakeUpThread> wakeUp;
    {
        std::scoped_lock g(m_mutex);
        std::swap(wakeUp, m_pWakeUp);
    }
    if (wakeUp)
        wakeUp->stop();
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_StatusIndicatorFactory_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new framework::StatusIndicatorFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
