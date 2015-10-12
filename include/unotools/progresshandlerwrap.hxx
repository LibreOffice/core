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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX
#define INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

namespace utl
{

class UNOTOOLS_DLLPUBLIC ProgressHandlerWrap : public ::cppu::WeakImplHelper1< ::com::sun::star::ucb::XProgressHandler >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > m_xStatusIndicator;

public:
    ProgressHandlerWrap( ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xSI );

    // XProgressHandler
    virtual void SAL_CALL push( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL update( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL pop()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_PROGRESSHANDLERWRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
