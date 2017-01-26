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

#ifndef INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX
#define INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX

#include <unotools/unotoolsdllapi.h>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

namespace utl
{

class UNOTOOLS_DLLPUBLIC ProgressHandlerWrap : public ::cppu::WeakImplHelper< css::ucb::XProgressHandler >
{
    css::uno::Reference< css::task::XStatusIndicator > m_xStatusIndicator;

public:
    ProgressHandlerWrap( css::uno::Reference< css::task::XStatusIndicator > const & xSI );

    // XProgressHandler
    virtual void SAL_CALL push( const css::uno::Any& Status ) override;
    virtual void SAL_CALL update( const css::uno::Any& Status ) override;
    virtual void SAL_CALL pop() override;
};

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
