/*************************************************************************
 *
 *  $RCSfile: statusindicatorfactory.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2005-08-12 16:24:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//-----------------------------------------------
// my own includes

#include <algorithm>

#ifndef __FRAMEWORK_HELPER_STATUSINDICATORFACTORY_HXX_
#include <helper/statusindicatorfactory.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#include <helper/statusindicator.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_
#include <helper/vclstatusindicator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#ifndef __FRAMEWORK_PROPERTIES_H_
#include <properties.h>
#endif

//-----------------------------------------------
// interface includes

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLS_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_DEVICEINFO_HPP_
#include <com/sun/star/awt/DeviceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW2_HPP_
#include <com/sun/star/awt/XWindow2.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

//-----------------------------------------------
// includes of other projects

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

//-----------------------------------------------
// namespace

namespace framework{

//-----------------------------------------------
// definitions

sal_Int32 StatusIndicatorFactory::m_nInReschedule = 0;  /// static counter for rescheduling
static ::rtl::OUString PROGRESS_RESOURCE = ::rtl::OUString::createFromAscii("private:resource/progressbar/progressbar");

//-----------------------------------------------
DEFINE_XINTERFACE_5(StatusIndicatorFactory                              ,
                    OWeakObject                                         ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider          ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo           ),
                    DIRECT_INTERFACE(css::lang::XInitialization        ),
                    DIRECT_INTERFACE(css::task::XStatusIndicatorFactory),
                    DIRECT_INTERFACE(css::util::XUpdatable             ))

DEFINE_XTYPEPROVIDER_5(StatusIndicatorFactory            ,
                       css::lang::XTypeProvider          ,
                       css::lang::XServiceInfo           ,
                       css::lang::XInitialization        ,
                       css::task::XStatusIndicatorFactory,
                       css::util::XUpdatable             )

DEFINE_XSERVICEINFO_MULTISERVICE(StatusIndicatorFactory                   ,
                                 ::cppu::OWeakObject                      ,
                                 SERVICENAME_STATUSINDICATORFACTORY       ,
                                 IMPLEMENTATIONNAME_STATUSINDICATORFACTORY)

DEFINE_INIT_SERVICE(StatusIndicatorFactory,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
StatusIndicatorFactory::StatusIndicatorFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase      (         )
    , ::cppu::OWeakObject (         )
    , m_xSMGR             (xSMGR    )
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
    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_xFrame             = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_FRAME            , css::uno::Reference< css::frame::XFrame >());
    m_xPluggWindow       = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_WINDOW           , css::uno::Reference< css::awt::XWindow >() );
    m_bAllowParentShow   = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_ALLOWPARENTSHOW  , (sal_Bool)sal_False                        );
    m_bDisableReschedule = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_DISABLERESCHEDULE, (sal_Bool)sal_False                        );

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

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
                                   const ::rtl::OUString&                                    sText ,
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
        pItem->m_sText  = ::rtl::OUString();
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
    ::rtl::OUString                  sText;
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
        impl_stopWakeUpThread();
    }

    impl_reschedule(sal_True);
}

//-----------------------------------------------
void StatusIndicatorFactory::setText(const css::uno::Reference< css::task::XStatusIndicator >& xChild,
                                     const ::rtl::OUString&                                    sText )
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
        return;

    // Ok the window should be made visible ... becuase it isnt currently visible.
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

    css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
    if (xPropSet.is())
    {
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
        xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
        if (xLayoutManager.is())
            xLayoutManager->showElement(PROGRESS_RESOURCE);
    }

    if (xParentWindow.is())
        xParentWindow->setVisible(sal_True);
    css::uno::Reference< css::awt::XTopWindow > xParentWindowTop(xParentWindow, css::uno::UNO_QUERY);
    if (xParentWindowTop.is())
        xParentWindowTop->toFront();
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_createProgress()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::frame::XFrame >              xFrame (m_xFrame.get()      , css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >               xWindow(m_xPluggWindow.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = m_xSMGR;

    aReadLock.lock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::task::XStatusIndicator > xProgress;

    if (xWindow.is())
    {
        // use vcl based progress implementation in plugged mode
        VCLStatusIndicator* pVCLProgress = new VCLStatusIndicator(xSMGR, xWindow);
        xProgress = css::uno::Reference< css::task::XStatusIndicator >(static_cast< css::task::XStatusIndicator* >(pVCLProgress), css::uno::UNO_QUERY);
    }
    else
    if (xFrame.is())
    {
        // use frame layouted progress implementation
        css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
        if (xPropSet.is())
        {
            css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                xLayoutManager->lock();
                xLayoutManager->createElement( PROGRESS_RESOURCE );
                xLayoutManager->hideElement( PROGRESS_RESOURCE );

                css::uno::Reference< css::ui::XUIElement > xProgressBar = xLayoutManager->getElement(PROGRESS_RESOURCE);
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
void StatusIndicatorFactory::impl_reschedule(sal_Bool bForce)
{
    const sal_Int32 MAX_RESCHEDULE = 1000;

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

        // reschedule all pending events
        // but avoid an endless loop
        sal_Int32 nRescheduleCount = MAX_RESCHEDULE;
        while (Application::AnyInput() && nRescheduleCount-- > 0)
            Application::Reschedule();

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
