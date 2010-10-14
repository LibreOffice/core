/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// include files of own module
#include <helper/statusindicator.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
