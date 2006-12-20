/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_interact.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:26:37 $
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

#if ! defined INCLUDED_DP_INTERACT_H
#define INCLUDED_DP_INTERACT_H

#include "rtl/ref.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_INC_DP_MISC_API_HXX
#include "dp_misc_api.hxx"
#endif

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
