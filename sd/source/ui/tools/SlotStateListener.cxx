/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlotStateListener.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:24:33 $
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
#include "precompiled_sd.hxx"

#include "tools/SlotStateListener.hxx"

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHP_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace sd { namespace tools {


SlotStateListener::SlotStateListener (void)
    : SlotStateListenerInterfaceBase(maMutex),
      maCallback(),
      mxDispatchProviderWeak(NULL)
{
}




SlotStateListener::SlotStateListener (
    Link& rCallback,
    const Reference<frame::XDispatchProvider>& rxDispatchProvider,
    const ::rtl::OUString& rSlotName)
    : SlotStateListenerInterfaceBase(maMutex),
      maCallback(),
      mxDispatchProviderWeak(NULL)
{
    SetCallback(rCallback);
    ConnectToDispatchProvider(rxDispatchProvider);
    ObserveSlot(rSlotName);
}




SlotStateListener::~SlotStateListener (void)
{
    ReleaseListeners();
}




void SlotStateListener::SetCallback (const Link& rCallback)
{
    ThrowIfDisposed();

    maCallback = rCallback;
}




void SlotStateListener::ConnectToDispatchProvider (
    const Reference<frame::XDispatchProvider>& rxDispatchProvider)
{
    ThrowIfDisposed();

    // When we are listening to state changes of slots of another frame then
    // release these listeners first.
    if ( ! maRegisteredURLList.empty())
        ReleaseListeners();

    mxDispatchProviderWeak = rxDispatchProvider;
}




void SlotStateListener::ObserveSlot (const ::rtl::OUString& rSlotName)
{
    ThrowIfDisposed();

    if (maCallback.IsSet())
    {
        // Connect the state change listener.
        util::URL aURL (MakeURL(rSlotName));
        Reference<frame::XDispatch> xDispatch (GetDispatch(aURL));
        if (xDispatch.is())
        {
            maRegisteredURLList.push_back(aURL);
            xDispatch->addStatusListener(this,aURL);
        }
    }
}




bool SlotStateListener::IsValid (void) const
{
    return maRegisteredURLList.size() > 0;
}




void SlotStateListener::disposing (void)
{
    ReleaseListeners();
    mxDispatchProviderWeak = WeakReference<frame::XDispatchProvider>(NULL);
    maCallback = Link();
}




util::URL SlotStateListener::MakeURL (const OUString& rSlotName) const
{
    util::URL aURL;

    aURL.Complete = rSlotName;

    uno::Reference<lang::XMultiServiceFactory> xServiceManager (
        ::comphelper::getProcessServiceFactory());
    if (xServiceManager.is())
    {
        Reference<util::XURLTransformer> xTransformer(xServiceManager->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
            UNO_QUERY);
        if (xTransformer.is())
            xTransformer->parseStrict(aURL);
    }

    return aURL;
}




Reference<frame::XDispatch>
    SlotStateListener::GetDispatch (const util::URL& rURL) const
{
    Reference<frame::XDispatch> xDispatch;

    Reference<frame::XDispatchProvider> xDispatchProvider (mxDispatchProviderWeak);
    if (xDispatchProvider.is())
        xDispatch = xDispatchProvider->queryDispatch(rURL, OUString(), 0);

    return xDispatch;
}




void SlotStateListener::statusChanged (
    const ::com::sun::star::frame::FeatureStateEvent& rState)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    OUString sSlotName (rState.FeatureURL.Complete);
    if (maCallback.IsSet())
        maCallback.Call(&sSlotName);
}




void SlotStateListener::ReleaseListeners (void)
{
    if ( ! maRegisteredURLList.empty())
    {
        RegisteredURLList::iterator iURL (maRegisteredURLList.begin());
        RegisteredURLList::iterator iEnd (maRegisteredURLList.end());
        for (; iURL!=iEnd; ++iURL)
        {
            Reference<frame::XDispatch> xDispatch (GetDispatch(*iURL));
            if (xDispatch.is())
            {
                xDispatch->removeStatusListener(this,*iURL);
            }
        }
    }
}




//=====  lang::XEventListener  ================================================

void SAL_CALL SlotStateListener::disposing (
    const lang::EventObject& )
    throw (RuntimeException)
{
}




void SlotStateListener::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "SlideSorterController object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}




} } // end of namespace ::sd::tools
