/*************************************************************************
 *
 *  $RCSfile: statusindicatorfactory.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:30:46 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

//-----------------------------------------------
// includes of other projects

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include <time.h>

//-----------------------------------------------
// namespace

namespace framework{

//-----------------------------------------------
// definitions

#define TIMEOUT_START_RESCHEDULE    10L /* 10th s */

sal_Int32 StatusIndicatorFactory::m_nInReschedule = 0;  /// static counter for rescheduling

//-----------------------------------------------
DEFINE_XINTERFACE_4(StatusIndicatorFactory                              ,
                    OWeakObject                                         ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider          ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo           ),
                    DIRECT_INTERFACE(css::lang::XInitialization        ),
                    DIRECT_INTERFACE(css::task::XStatusIndicatorFactory))

DEFINE_XTYPEPROVIDER_4(StatusIndicatorFactory            ,
                       css::lang::XTypeProvider          ,
                       css::lang::XServiceInfo           ,
                       css::lang::XInitialization        ,
                       css::task::XStatusIndicatorFactory)

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
    //  Init baseclasses first
    : ThreadHelpBase     (&Application::GetSolarMutex() )
    , ::cppu::OWeakObject(                              )
    // Init member
    , m_xSMGR            (xSMGR                         )
{
}

//-----------------------------------------------
StatusIndicatorFactory::~StatusIndicatorFactory()
{
}

//-----------------------------------------------
void SAL_CALL StatusIndicatorFactory::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    m_xFrame       = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_FRAME , css::uno::Reference< css::frame::XFrame >());
    m_xPluggWindow = lArgs.getUnpackedValueOrDefault(STATUSINDICATORFACTORY_PROPNAME_WINDOW, css::uno::Reference< css::awt::XWindow >() );

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
    m_nStartTime   = impl_get10ThSec();

    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    if (xProgress.is())
        xProgress->start(sText, nRange);

    impl_reschedule();
}

//-----------------------------------------------
void StatusIndicatorFactory::reset(const css::uno::Reference< css::task::XStatusIndicator >& xChild)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // reset the internal info structure related to this child
    IndicatorStack::iterator pItem = ::std::find(m_aStack.begin(), m_aStack.end(), xChild);
    if (pItem != m_aStack.end())
        pItem->reset();

    css::uno::Reference< css::task::XStatusIndicator > xActive   = m_xActiveChild;
    css::uno::Reference< css::task::XStatusIndicator > xProgress = m_xProgress;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    if (xChild != xActive)
        return; // not the top most child => dont change UI

    if (xProgress.is())
        xProgress->reset();

    impl_reschedule();
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

    if (!xProgress.is())
        return;

    if (xActive.is())
    {
        // There is at least one further child indicator.
        // Actualize our progress, so it shows these values from now.
        xProgress->setText (sText );
        xProgress->setValue(nValue);
    }
    else
    {
        // Our stack is empty. No further child exists.
        // Se we must "end" our progress realy
        xProgress->end();
    }

    impl_reschedule();
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

    if (xChild != xActive)
        return;

    if (xProgress.is())
        xProgress->setText(sText);

    impl_reschedule();
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
    sal_Int32                                          nStartTime = m_nStartTime;

    aWriteLock.unlock();
    // SAFE -> ----------------------------------

    if (xChild != xActive)
        return;

    if (
        (xProgress.is()     ) &&
        (nOldValue != nValue)
       )
    {
        xProgress->setValue(nValue);
    }

    // We start rescheduling only after 1 second - this code was successfully introduced by the sfx2
    // implementation of the progress bar.
    sal_Bool bReschedule = ((impl_get10ThSec()-nStartTime ) > TIMEOUT_START_RESCHEDULE);
    if (bReschedule)
        impl_reschedule();
}

//-----------------------------------------------
void StatusIndicatorFactory::impl_createProgress()
{
    ::rtl::OUString PROGRESSBAR_RES_STR = ::rtl::OUString::createFromAscii("private:resource/progressbar/progressbar");

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
            css::uno::Reference< dcss::frame::XLayoutManager > xLayoutManager;
            xPropSet->getPropertyValue(FRAME_PROPNAME_LAYOUTMANAGER) >>= xLayoutManager;
            if (xLayoutManager.is())
            {
                xLayoutManager->createElement(PROGRESSBAR_RES_STR);
                xLayoutManager->showElement  (PROGRESSBAR_RES_STR);

                css::uno::Reference< dcss::ui::XUIElement > xProgressBar = xLayoutManager->getElement(PROGRESSBAR_RES_STR);
                if (xProgressBar.is())
                    xProgress = css::uno::Reference< css::task::XStatusIndicator >(xProgressBar->getRealInterface(), css::uno::UNO_QUERY);
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
void StatusIndicatorFactory::impl_reschedule()
{
    // SAFE -> ----------------------------------
    WriteGuard aGlobalLock(LockHelper::getGlobalLock());

    if (m_nInReschedule == 0)
    {
        ++m_nInReschedule;
        aGlobalLock.unlock();
        Application::Reschedule();
        aGlobalLock.lock();
        --m_nInReschedule;
    }
}

//-----------------------------------------------
sal_uInt32 StatusIndicatorFactory::impl_get10ThSec()
{
    sal_uInt32 n10Ticks = 10 * (sal_uInt32)clock();
    return n10Ticks / CLOCKS_PER_SEC;
}

} // namespace framework
