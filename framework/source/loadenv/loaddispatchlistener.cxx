/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loaddispatchlistener.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:05:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// includes of own project

#ifndef __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_
#include <loadenv/loaddispatchlistener.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

//_______________________________________________
// includes of uno interface

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

//_______________________________________________
// includes of an other project

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
