/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: statusindicator.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:00:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
#include "precompiled_framework.hxx"

//_______________________________________________
// include files of own module

#ifndef __FRAMEWORK_HELPER_STATUSINDICATOR_HXX_
#include <helper/statusindicator.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// declarations

//***********************************************
// XInterface
DEFINE_XINTERFACE_2(StatusIndicator                              ,
                    OWeakObject                                  ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider   ),
                    DIRECT_INTERFACE(css::task::XStatusIndicator))

//***********************************************
// XInterface
DEFINE_XTYPEPROVIDER_2(StatusIndicator            ,
                       css::lang::XTypeProvider   ,
                       css::task::XStatusIndicator)

//***********************************************
StatusIndicator::StatusIndicator(StatusIndicatorFactory* pFactory)
    : ThreadHelpBase     (        )
    , ::cppu::OWeakObject(        )
    , m_xFactory         (pFactory)
{
}

//***********************************************
StatusIndicator::~StatusIndicator()
{
}

//***********************************************
void SAL_CALL StatusIndicator::start(const ::rtl::OUString& sText ,
                                           sal_Int32        nRange)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = (StatusIndicatorFactory*)xFactory.get();
        pFactory->start(this, sText, nRange);
    }
}

//***********************************************
void SAL_CALL StatusIndicator::end()
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = (StatusIndicatorFactory*)xFactory.get();
        pFactory->end(this);
    }
}

//***********************************************
void SAL_CALL StatusIndicator::reset()
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = (StatusIndicatorFactory*)xFactory.get();
        pFactory->reset(this);
    }
}

//***********************************************
void SAL_CALL StatusIndicator::setText(const ::rtl::OUString& sText)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = (StatusIndicatorFactory*)xFactory.get();
        pFactory->setText(this, sText);
    }
}

//***********************************************
void SAL_CALL StatusIndicator::setValue(sal_Int32 nValue)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::task::XStatusIndicatorFactory > xFactory(m_xFactory.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    // <- SAFE
    if (xFactory.is())
    {
        StatusIndicatorFactory* pFactory = (StatusIndicatorFactory*)xFactory.get();
        pFactory->setValue(this, nValue);
    }
}

} // namespace framework
