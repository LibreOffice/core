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
#include <helper/statusindicatorfactory.hxx>
#include <helper/statusindicator.hxx>
#include <helper/vclstatusindicator.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/awt/Rectangle.hpp>

#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager2.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <officecfg/Office/Common.hxx>

namespace framework{


sal_Int32 StatusIndicatorFactory::m_nInReschedule = 0;  /// static counter for rescheduling
const char PROGRESS_RESOURCE[] = "private:resource/progressbar/progressbar";

//-----------------------------------------------

DEFINE_XSERVICEINFO_MULTISERVICE_2(StatusIndicatorFactory                   ,
                                   ::cppu::OWeakObject                      ,
                                   OUString("com.sun.star.task.StatusIndicatorFactory"),
                                   OUString("com.sun.star.comp.framework.StatusIndicatorFactory"))

DEFINE_INIT_SERVICE(StatusIndicatorFactory,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

//-----------------------------------------------
StatusIndicatorFactory::StatusIndicatorFactory(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase      (         )
    , m_xContext          (xContext )
    , m_pWakeUp           (0        )
    , m_bAllowReschedule  (sal_False)
    , m_bAllowParentShow  (sal_False)
    , m_bDisableReschedule(sal_False)
{
}

//-----------------------------------------------
StatusIndicatorFactory::~StatusIndicatorFactory()
{
    impl_stopWakeUpThread();
}

//-----------------------------------------------
void SAL_CALL StatusIndicatorFactory::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    if (lArguments.getLength() > 0) {
        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);

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
            m_bAllowParentShow   = lArgs.getUnpackedValueOrDefault("AllowParentShow"  , (sal_Bool)sal_False                        );
            m_bDisableReschedule = lArgs.getUnpackedValueOrDefault("DisableReschedule", (sal_Bool)sal_False                        );

            aWriteLock.unlock();
           // <- SAFE ----------------------------------
       }
    }

    impl_createProgress();
}

//-----------------------------------------------
css::uno::Reference< css::task::XStatusIndicator > SAL_CALL StatusIndicatorFactory::createStatusIndicator()
    throw(css::uno::RuntimeException)
{
    StatusIndicator* pIndicator = new StatusIndicator(this);
    css::uno::Reference< css::task::XStatusIndicator > xIndicator(static_cast< ::cppu::OWeakObject* >(pIndicator), css::uno::UNO_QUERY_THROW);

    return xIndicator;
}

//-----------------------------------------------
void SAL_CALL StatusIndicatorFactory::update()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_bAllowReschedule = sal_True;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void StatusIndicatorFactory::start(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                   const OUString&                                    sText ,
                                         sal_Int32                                           nRange)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // create new info structure for this child or move it to the front of our stack
    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
        m_aStack.erase(pItem);
    IndicatorInfo aInfo(xChild, sText, nRange);
    m_aStack.push_back (aInfo                );

    m_xActiveChild = xChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    implts_makeParentVisibleIfAllowed();

    if (xProgress.is())
        xProgress->start(sText, nRange);

    impl_startWakeUpThread();
    impl_reschedule(sal_True);
}

//-----------------------------------------------
void StatusIndicatorFactory::reset(const css::uno::Reference< css::task::XStatusIndicator >& xChild)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // reset the internal info structure related to this child
    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
    {
        pItem->m_nValue = 0;
        pItem->m_sText  = OUString();
    }

    css::uno::Reference< css::task::XStatusIndicator > xActive   = m_xActiveChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // not the top most child => dont change UI
    // But dont forget Reschedule!
    if (
        (xChild == xActive) &&
        (xProgress.is()   )
       )
        xProgress->reset();

    impl_reschedule(sal_True);
}

//-----------------------------------------------
void StatusIndicatorFactory::end(const css::uno::Reference< css::task::XStatusIndicator >& xChild)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // remove this child from our stack
    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
        m_aStack.erase(pItem);

    // activate next child ... or finish the progress if there is no further one.
    m_xActiveChild.clear();
    OUString                  sText;
    sal_Int32                        nValue = 0;
    IndicatorStack::reverse_iterator pNext  = m_aStack.rbegin();
    if (pNext != m_aStack.rend())
    {
        m_xActiveChild = pNext->m_xIndicator;
        sText          = pNext->m_sText;
        nValue         = pNext->m_nValue;
    }

    css::uno::Reference< css::task::XStatusIndicator > xActive   = m_xActiveChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aWriteLock.unlock();
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
        // Se we must "end" our progress realy
        if (xProgress.is())
            xProgress->end();
        // Now hide the progress bar again.
        impl_hideProgress();

        impl_stopWakeUpThread();
    }

    impl_reschedule(sal_True);
}

//-----------------------------------------------
void StatusIndicatorFactory::setText(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                     const OUString&                                    sText )
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
        pItem->m_sText = sText;

    css::uno::Reference< css::task::XStatusIndicator > xActive   = m_xActiveChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aWriteLock.unlock();
    // SAFE -> ----------------------------------

    // paint only the top most indicator
    // but dont forget to Reschedule!
    if (
        (xChild == xActive) &&
        (xProgress.is()   )
       )
    {
        xProgress->setText(sText);
    }

    impl_reschedule(sal_True);
}

//-----------------------------------------------
void StatusIndicatorFactory::setValue( const css::uno::Reference< css::task::XStatusIndicator >& xChild ,
                                             sal_Int32                                           nValue )
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    sal_Int32 nOldValue = 0;
    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
    {
        nOldValue       = pItem->m_nValue;
        pItem->m_nValue = nValue;
    }

    css::uno::Reference< css::task::XStatusIndicator > xActive    = m_xActiveChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress  = m_xProgress;

    aWriteLock.unlock();
    // SAFE -> ----------------------------------

    if (
        (xChild    == xActive) &&
        (nOldValue != nValue ) &&
        (xProgress.is()      )
       )
    {
        xProgress->setValue(nValue);
    }

    impl_reschedule(sal_False);
}

//-----------------------------------------------
void StatusIndicatorFactory::implts_makeParentVisibleIfAllowed()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    if (!m_bAllowParentShow)
        return;

    css::uno::Reference< css::frame::XFrame > xFrame      (m_xFrame.get()      , css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >  xPluggWindow(m_xPluggWindow.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::uno::XComponentContext > xContext( m_xContext);

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::awt::XWindow > xParentWindow;
    if (xFrame.is())
        xParentWindow = xFrame->getContainerWindow();
    else
        xParentWindow = xPluggWindow;

    // dont disturb user in case he put the loading document into the background!
    // Supress any setVisible() or toFront() call in case the initial show was
    // already made.
    css::uno::Reference< css::awt::XWindow2 > xVisibleCheck(xParentWindow, css::uno::UNO_QUERY);
    sal_Bool bIsVisible = sal_False;
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
        xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
        if (xLayoutManager.is())
        {
            if ( !xLayoutManager->isVisible() )
                return;
        }
    }

    // Ok the window should be made visible ... because it isnt currently visible.
    // BUT ..!
    // We need a Hack for our applications: They get her progress from the frame directly
    // on saving documents. Because there is no progress set on the MediaDescriptor.
    // But that's wrong. In case the document was opened hidden, they shouldnt use any progress .-(
    // They only possible workaround: dont show the parent window here, if the document was opened hidden.
    sal_Bool bHiddenDoc = sal_False;
    if (xFrame.is())
    {
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel >      xModel     ;
        xController = xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();
        if (xModel.is())
        {
            ::comphelper::MediaDescriptor lDocArgs(xModel->getArgs());
            bHiddenDoc = lDocArgs.getUnpackedValueOrDefault(
                ::comphelper::MediaDescriptor::PROP_HIDDEN(),
                (sal_Bool)sal_False);
        }
    }

    if (bHiddenDoc)
        return;

    // OK: The document was not opened in hidden mode ...
    // and the window isnt already visible.
    // Show it and bring it to front.
    // But before we have to be sure, that our internal used helper progress
    // is visible too.
    impl_showProgress();

    SolarMutexGuard aSolarGuard;
    Window* pWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( pWindow )
    {
        bool bForceFrontAndFocus(officecfg::Office::Common::View::NewDocumentHandling::ForceFocusAndToFront::get(xContext));
        pWindow->Show(sal_True, bForceFrontAndFocus ? SHOW_FOREGROUNDTASK : 0 );
    }

}

//-----------------------------------------------
void StatusIndicatorFactory::impl_createProgress()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFrame >              xFrame (m_xFrame.get()      , css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >               xWindow(m_xPluggWindow.get(), css::uno::UNO_QUERY);

    aReadLock.lock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xProgress;

    if (xWindow.is())
    {
        // use vcl based progress implementation in plugged mode
        VCLStatusIndicator* pVCLProgress = new VCLStatusIndicator(xWindow);
        xProgress = css::uno::Reference< css::task::XStatusIndicator >(static_cast< css::task::XStatusIndicator* >(pVCLProgress), css::uno::UNO_QUERY);
    }
    else if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                xLayoutManager->lock();
                OUString sPROGRESS_RESOURCE(PROGRESS_RESOURCE);
                xLayoutManager->createElement( sPROGRESS_RESOURCE );
                xLayoutManager->hideElement( sPROGRESS_RESOURCE );

                css::uno::Reference< css::ui::XUIElement > xProgressBar = xLayoutManager->getElement(sPROGRESS_RESOURCE);
                if (xProgressBar.is())
                    xProgress = css::uno::Reference< css::task::XStatusIndicator >(xProgressBar->getRealInterface(), css::uno::UNO_QUERY);
                xLayoutManager->unlock();
            }
        }
    }

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xProgress = xProgress;
    aWriteLock.lock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_showProgress()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFrame >              xFrame (m_xFrame.get()      , css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >               xWindow(m_xPluggWindow.get(), css::uno::UNO_QUERY);

    aReadLock.lock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xProgress;

    if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
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
                    xProgress = css::uno::Reference< css::task::XStatusIndicator >(xProgressBar->getRealInterface(), css::uno::UNO_QUERY);
            }
        }

        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);
        m_xProgress = xProgress;
        aWriteLock.lock();
        // <- SAFE ----------------------------------
    }
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_hideProgress()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFrame >              xFrame (m_xFrame.get()      , css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >               xWindow(m_xPluggWindow.get(), css::uno::UNO_QUERY);

    aReadLock.lock();
    // <- SAFE ----------------------------------

    if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager2 > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
                xLayoutManager->hideElement( OUString(PROGRESS_RESOURCE) );
        }
    }
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_reschedule(sal_Bool bForce)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    if (m_bDisableReschedule)
        return;
    aReadLock.unlock();
    // <- SAFE

    sal_Bool bReschedule = bForce;
    if (!bReschedule)
    {
        // SAFE ->
        WriteGuard aWriteLock(m_aLock);
        bReschedule        = m_bAllowReschedule;
        m_bAllowReschedule = sal_False;
        aWriteLock.unlock();
        // <- SAFE
    }

    if (!bReschedule)
        return;

    // SAFE ->
    WriteGuard aGlobalLock(LockHelper::getGlobalLock());

    if (m_nInReschedule == 0)
    {
        ++m_nInReschedule;
        aGlobalLock.unlock();
        // <- SAFE

        Application::Reschedule(true);

        // SAFE ->
        aGlobalLock.lock();
        --m_nInReschedule;
    }
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_startWakeUpThread()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    if (m_bDisableReschedule)
        return;

    if (!m_pWakeUp)
    {
        m_pWakeUp = new WakeUpThread(this);
        m_pWakeUp->create();
    }
    aWriteLock.unlock();
    // <- SAFE
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_stopWakeUpThread()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);
    if (m_pWakeUp)
    {
        // Thread kill itself after terminate()!
        m_pWakeUp->terminate();
        m_pWakeUp = 0;
    }
    aWriteLock.unlock();
    // <- SAFE
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
