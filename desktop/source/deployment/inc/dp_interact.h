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

#pragma once

#include <config_options.h>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include "dp_misc_api.hxx"

namespace dp_misc
{

inline void progressUpdate(
    OUString const & status,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
{
    if (xCmdEnv.is()) {
        css::uno::Reference<css::ucb::XProgressHandler> xProgressHandler(
            xCmdEnv->getProgressHandler() );
        if (xProgressHandler.is()) {
            xProgressHandler->update( css::uno::makeAny(status) );
        }
    }
}


class ProgressLevel
{
    css::uno::Reference<css::ucb::XProgressHandler> m_xProgressHandler;

public:
    inline ~ProgressLevel();
    inline ProgressLevel(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        OUString const & status );

    inline void update( OUString const & status ) const;
    inline void update( css::uno::Any const & status ) const;
};


inline ProgressLevel::ProgressLevel(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    OUString const & status )
{
    if (xCmdEnv.is())
        m_xProgressHandler = xCmdEnv->getProgressHandler();
    if (m_xProgressHandler.is())
        m_xProgressHandler->push( css::uno::makeAny(status) );
}


inline ProgressLevel::~ProgressLevel()
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->pop();
}


inline void ProgressLevel::update( OUString const & status ) const
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( css::uno::makeAny(status) );
}


inline void ProgressLevel::update( css::uno::Any const & status ) const
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( status );
}



/** @return true if ia handler is present and any selection has been chosen
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool interactContinuation(
    css::uno::Any const & request,
    css::uno::Type const & continuation,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool * pcont, bool * pabort );




class UNLESS_MERGELIBS(DESKTOP_DEPLOYMENTMISC_DLLPUBLIC) AbortChannel :
    public ::cppu::WeakImplHelper<css::task::XAbortChannel>
{
    bool m_aborted;
    css::uno::Reference<css::task::XAbortChannel> m_xNext;

public:
    AbortChannel() : m_aborted( false ) {}
    static AbortChannel * get(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel )
        { return static_cast<AbortChannel *>(xAbortChannel.get()); }

    bool isAborted() const { return m_aborted; }

    // XAbortChannel
    virtual void SAL_CALL sendAbort() override;

    class SAL_DLLPRIVATE Chain
    {
        const ::rtl::Reference<AbortChannel> m_abortChannel;
    public:
        Chain(
            ::rtl::Reference<AbortChannel> const & abortChannel,
            css::uno::Reference<css::task::XAbortChannel> const & xNext )
            : m_abortChannel( abortChannel )
            { if (m_abortChannel.is()) m_abortChannel->m_xNext = xNext; }
        ~Chain()
            { if (m_abortChannel.is()) m_abortChannel->m_xNext.clear(); }
    };
    friend class Chain;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
