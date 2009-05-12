/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loaddispatchlistener.cxx,v $
 * $Revision: 1.6 $
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
// includes of own project
#include <loadenv/loaddispatchlistener.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>

//_______________________________________________
// includes of uno interface
#include <com/sun/star/frame/DispatchResultState.hpp>

//_______________________________________________
// includes of an other project
#include <vcl/svapp.hxx>

//_______________________________________________
// namespace

namespace framework{

// may there exist already a define .-(
#ifndef css
namespace css = ::com::sun::star;
#endif

//_______________________________________________
// declarations

//-----------------------------------------------
DEFINE_XINTERFACE_2(LoadDispatchListener                                                             ,
                    OWeakObject                                                                      ,
                    DIRECT_INTERFACE (css::frame::XDispatchResultListener                           ),
                    DERIVED_INTERFACE(css::lang::XEventListener, css::frame::XDispatchResultListener))

//-----------------------------------------------
LoadDispatchListener::LoadDispatchListener()
    : ThreadHelpBase(&Application::GetSolarMutex())
{
    // reset the condition object - so our user can wait there.
    m_aUserWait.reset();
    // set defined state for our result value
    m_aResult.State = css::frame::DispatchResultState::DONTKNOW;
    m_aResult.Result.clear();
}

//-----------------------------------------------
LoadDispatchListener::~LoadDispatchListener()
{
}

//-----------------------------------------------
void SAL_CALL LoadDispatchListener::dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_aResult = aEvent;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // inform user about this arrived event
    m_aUserWait.set();
}

//-----------------------------------------------
void SAL_CALL LoadDispatchListener::disposing(const css::lang::EventObject&)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_aResult.State = css::frame::DispatchResultState::DONTKNOW;
    m_aResult.Result.clear();
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    // inform user about this arrived event
    m_aUserWait.set();
}

//-----------------------------------------------
void LoadDispatchListener::setURL(const ::rtl::OUString & sURL)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_sURL = sURL;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
sal_Bool LoadDispatchListener::wait(sal_Int32 /*nWait_ms*/)
{
    // Wait till an event occures
    m_aUserWait.wait(0);
    // reset the condition, so this method can be called again.
    // Of course a new action has to be started outside too!
    m_aUserWait.reset();

    // TODO implement real timeout :-)
    return sal_True;
}

//-----------------------------------------------
css::frame::DispatchResultEvent LoadDispatchListener::getResult() const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    return m_aResult;
    // <- SAFE ----------------------------------
}

} // namespace framework
