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

#if ! defined INCLUDED_DP_INTERACT_H
#define INCLUDED_DP_INTERACT_H

#include "rtl/ref.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"
#include "dp_misc_api.hxx"

namespace css = ::com::sun::star;

namespace dp_misc
{

inline void progressUpdate(
    ::rtl::OUString const & status,
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

//==============================================================================
class ProgressLevel
{
    css::uno::Reference<css::ucb::XProgressHandler> m_xProgressHandler;

public:
    inline ~ProgressLevel();
    inline ProgressLevel(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        ::rtl::OUString const & status );

    inline void update( ::rtl::OUString const & status ) const;
    inline void update( css::uno::Any const & status ) const;
};

//______________________________________________________________________________
inline ProgressLevel::ProgressLevel(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv,
    ::rtl::OUString const & status )
{
    if (xCmdEnv.is())
        m_xProgressHandler = xCmdEnv->getProgressHandler();
    if (m_xProgressHandler.is())
        m_xProgressHandler->push( css::uno::makeAny(status) );
}

//______________________________________________________________________________
inline ProgressLevel::~ProgressLevel()
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->pop();
}

//______________________________________________________________________________
inline void ProgressLevel::update( ::rtl::OUString const & status ) const
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( css::uno::makeAny(status) );
}

//______________________________________________________________________________
inline void ProgressLevel::update( css::uno::Any const & status ) const
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( status );
}

//##############################################################################

/** @return true if ia handler is present and any selection has been chosen
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC bool interactContinuation(
    css::uno::Any const & request,
    css::uno::Type const & continuation,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool * pcont, bool * pabort );

//##############################################################################

//==============================================================================
class DESKTOP_DEPLOYMENTMISC_DLLPUBLIC AbortChannel :
    public ::cppu::WeakImplHelper1<css::task::XAbortChannel>
{
    bool m_aborted;
    css::uno::Reference<css::task::XAbortChannel> m_xNext;

public:
    inline AbortChannel() : m_aborted( false ) {}
    inline static AbortChannel * get(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel )
        { return static_cast<AbortChannel *>(xAbortChannel.get()); }

    inline bool isAborted() const { return m_aborted; }

    // XAbortChannel
    virtual void SAL_CALL sendAbort() throw (css::uno::RuntimeException);

    class SAL_DLLPRIVATE Chain
    {
        const ::rtl::Reference<AbortChannel> m_abortChannel;
    public:
        inline Chain(
            ::rtl::Reference<AbortChannel> const & abortChannel,
            css::uno::Reference<css::task::XAbortChannel> const & xNext )
            : m_abortChannel( abortChannel )
            { if (m_abortChannel.is()) m_abortChannel->m_xNext = xNext; }
        inline ~Chain()
            { if (m_abortChannel.is()) m_abortChannel->m_xNext.clear(); }
    };
    friend class Chain;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
