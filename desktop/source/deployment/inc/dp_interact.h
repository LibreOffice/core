/*************************************************************************
 *
 *  $RCSfile: dp_interact.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:05:47 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if ! defined INCLUDED_DP_INTERACT_H
#define INCLUDED_DP_INTERACT_H

#include "rtl/ref.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/task/XAbortChannel.hpp"


namespace css = ::com::sun::star;

namespace dp_misc
{

//==============================================================================
class ProgressLevel
{
    css::uno::Reference< css::ucb::XProgressHandler > m_xProgressHandler;

public:
    inline ProgressLevel(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    inline ProgressLevel(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        ::rtl::OUString const & status );
    inline ~ProgressLevel();

    inline void update( ::rtl::OUString const & status );
    inline void update();
};

//______________________________________________________________________________
inline ProgressLevel::ProgressLevel(
    css::uno::Reference< css::ucb::XCommandEnvironment > const & xCmdEnv )
{
    if (xCmdEnv.is())
        m_xProgressHandler = xCmdEnv->getProgressHandler();
    if (m_xProgressHandler.is())
        m_xProgressHandler->push( css::uno::Any() );
}

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
inline void ProgressLevel::update( ::rtl::OUString const & status )
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( css::uno::makeAny(status) );
}

//______________________________________________________________________________
inline void ProgressLevel::update()
{
    if (m_xProgressHandler.is())
        m_xProgressHandler->update( css::uno::Any() );
}

//##############################################################################

//==============================================================================
bool interactContinuation(
    css::uno::Any const & request,
    css::uno::Type const & continuation,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    bool * pabort = 0 );

//==============================================================================
void interactContinuation_throw(
    css::deployment::DeploymentException const & exc,
    css::uno::Type const & continuation,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );

//##############################################################################

//==============================================================================
class AbortChannel : public ::cppu::WeakImplHelper1<css::task::XAbortChannel>
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

    class Chain
    {
        ::rtl::Reference<AbortChannel> m_abortChannel;
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
