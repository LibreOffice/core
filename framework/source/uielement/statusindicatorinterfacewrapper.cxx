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

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace framework
{

StatusIndicatorInterfaceWrapper::StatusIndicatorInterfaceWrapper(
    const rtl::Reference< ProgressBarWrapper >& rStatusIndicatorImpl ) :
    m_xStatusIndicatorImpl( rStatusIndicatorImpl )
{
}

StatusIndicatorInterfaceWrapper::~StatusIndicatorInterfaceWrapper()
{
}

void SAL_CALL StatusIndicatorInterfaceWrapper::start(
    const OUString& sText,
    sal_Int32 nRange )
{
    rtl::Reference< ProgressBarWrapper > xProgressBar( m_xStatusIndicatorImpl );
    if ( xProgressBar.is() )
        xProgressBar->start( sText, nRange );
}

void SAL_CALL StatusIndicatorInterfaceWrapper::end()
{
    rtl::Reference< ProgressBarWrapper > xProgressBar( m_xStatusIndicatorImpl );
    if ( xProgressBar.is() )
        xProgressBar->end();
}

void SAL_CALL StatusIndicatorInterfaceWrapper::reset()
{
    rtl::Reference< ProgressBarWrapper > xProgressBar( m_xStatusIndicatorImpl );
    if ( xProgressBar.is() )
        xProgressBar->reset();
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setText(
    const OUString& sText )
{
    rtl::Reference< ProgressBarWrapper > xProgressBar( m_xStatusIndicatorImpl );
    if ( xProgressBar.is() )
        xProgressBar->setText( sText );
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setValue(
    sal_Int32 nValue )
{
    rtl::Reference< ProgressBarWrapper > xProgressBar( m_xStatusIndicatorImpl );
    if ( xProgressBar.is() )
        xProgressBar->setValue( nValue );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
