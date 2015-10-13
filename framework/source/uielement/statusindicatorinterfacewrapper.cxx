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

#include <uielement/statusindicatorinterfacewrapper.hxx>
#include <uielement/progressbarwrapper.hxx>

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace framework
{

StatusIndicatorInterfaceWrapper::StatusIndicatorInterfaceWrapper(
    const css::uno::Reference< css::lang::XComponent >& rStatusIndicatorImpl ) :
    m_xStatusIndicatorImpl( rStatusIndicatorImpl )
{
}

StatusIndicatorInterfaceWrapper::~StatusIndicatorInterfaceWrapper()
{
}

void SAL_CALL StatusIndicatorInterfaceWrapper::start(
    const OUString& sText,
    sal_Int32              nRange )
throw( css::uno::RuntimeException, std::exception )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = static_cast<ProgressBarWrapper*>(xComp.get());
        if ( pProgressBar )
            pProgressBar->start( sText, nRange );
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::end()
throw( css::uno::RuntimeException, std::exception )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = static_cast<ProgressBarWrapper*>(xComp.get());
        if ( pProgressBar )
            pProgressBar->end();
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::reset()
throw( css::uno::RuntimeException, std::exception )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = static_cast<ProgressBarWrapper*>(xComp.get());
        if ( pProgressBar )
            pProgressBar->reset();
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setText(
    const OUString& sText )
throw( css::uno::RuntimeException, std::exception )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = static_cast<ProgressBarWrapper*>(xComp.get());
        if ( pProgressBar )
            pProgressBar->setText( sText );
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setValue(
    sal_Int32 nValue )
throw( css::uno::RuntimeException, std::exception )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = static_cast<ProgressBarWrapper*>(xComp.get());
        if ( pProgressBar )
            pProgressBar->setValue( nValue );
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
