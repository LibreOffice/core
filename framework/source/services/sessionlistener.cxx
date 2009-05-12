/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sessionlistener.cxx,v $
 * $Revision: 1.12 $
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
// my own includes

#ifndef __FRAMEWORK_SERVICES_TYPEDETECTION_HXX_
#include <services/sessionlistener.hxx>
#endif
#include <threadhelp/readguard.hxx>
#include <threadhelp/resetableguard.hxx>
#include <protocols.h>
#include <services.h>

#include <osl/thread.h>


#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <tools/tempfile.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <osl/time.h>
#include <comphelper/processfactory.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/internaloptions.hxx>
#include <stdio.h>
//_______________________________________________
// interface includes
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>
//_______________________________________________
// includes of other projects

//_______________________________________________
// namespace

using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

using namespace rtl;

namespace framework{

//_______________________________________________
// non exported const

//_______________________________________________
// non exported definitions

//_______________________________________________
// declarations

//***********************************************
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5(
        SessionListener,
        OWeakObject,
        DIRECT_INTERFACE(css::lang::XTypeProvider),
        DIRECT_INTERFACE(css::lang::XInitialization),
        DIRECT_INTERFACE(css::frame::XSessionManagerListener),
        DIRECT_INTERFACE(css::frame::XStatusListener),
        DIRECT_INTERFACE(css::lang::XServiceInfo))

DEFINE_XTYPEPROVIDER_5(
        SessionListener,
        css::lang::XTypeProvider,
        css::lang::XInitialization,
        css::frame::XSessionManagerListener,
        css::frame::XStatusListener,
        css::lang::XServiceInfo)

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE(
       SessionListener,
       cppu::OWeakObject,
       SERVICENAME_SESSIONLISTENER,
       IMPLEMENTATIONNAME_SESSIONLISTENER)

DEFINE_INIT_SERVICE(SessionListener,
                    {
                        /* Add special code for initialization here, if you have to use your own instance
                           during your ctor is still in progress! */
                    }
                   )

SessionListener::SessionListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        : ThreadHelpBase      (&Application::GetSolarMutex())
        , OWeakObject         (                             )
        , m_xSMGR             (xSMGR                        )
        , m_bRestored( false )
{
}

SessionListener::~SessionListener()
{
    if (m_rSessionManager.is())
    {
        css::uno::Reference< XSessionManagerListener> me(this);
        m_rSessionManager->removeSessionManagerListener(me);
    }
}

void SAL_CALL SessionListener::disposing(const com::sun::star::lang::EventObject&) throw (RuntimeException)
{
}

void SAL_CALL SessionListener::initialize(const Sequence< Any  >& args)
    throw (RuntimeException)
{

    OUString aSMgr = OUString::createFromAscii("com.sun.star.frame.SessionManagerClient");
    if (args.getLength() > 0)
    {
        NamedValue v;
        for (int i = 0; i < args.getLength(); i++)
        {
            if (args[i] >>= v)
            {
                if (v.Name.equalsAscii("SessionManagerName"))
                    v.Value >>= aSMgr;
                else if (v.Name.equalsAscii("SessionManager"))
                    v.Value >>= m_rSessionManager;
            }
        }
    }
    if (!m_rSessionManager.is())
        m_rSessionManager = css::uno::Reference< XSessionManagerClient >
            (m_xSMGR->createInstance(aSMgr), UNO_QUERY);

    if (m_rSessionManager.is())
    {
        m_rSessionManager->addSessionManagerListener(this);
    }
}

void SAL_CALL SessionListener::statusChanged(const FeatureStateEvent& event)
    throw (css::uno::RuntimeException)
{
   if (event.FeatureURL.Complete.equalsAscii("vnd.sun.star.autorecovery:/doSessionRestore"))
    {
        if (event.FeatureDescriptor.compareToAscii("update")==0)
            m_bRestored = sal_True; // a document was restored
        // if (event.FeatureDescriptor.compareToAscii("stop")==0)

    }
    else if (event.FeatureURL.Complete.equalsAscii("vnd.sun.star.autorecovery:/doSessionSave"))
    {
        if (event.FeatureDescriptor.compareToAscii("stop")==0)
        {
            if (m_rSessionManager.is())
                m_rSessionManager->saveDone(this); // done with save
        }
    }
}


sal_Bool SAL_CALL SessionListener::doRestore()
    throw (RuntimeException)
{
    ResetableGuard aGuard(m_aLock);
    m_bRestored = sal_False;
    try {
        css::uno::Reference< XDispatch > xDispatch(m_xSMGR->createInstance(SERVICENAME_AUTORECOVERY), UNO_QUERY_THROW);

        URL aURL;
        aURL.Complete = OUString::createFromAscii("vnd.sun.star.autorecovery:/doSessionRestore");
        css::uno::Reference< XURLTransformer > xURLTransformer(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), UNO_QUERY_THROW);
        xURLTransformer->parseStrict(aURL);
        Sequence< PropertyValue > args;
        xDispatch->addStatusListener(this, aURL);
        xDispatch->dispatch(aURL, args);
        m_bRestored = sal_True;

    } catch (com::sun::star::uno::Exception& e) {
        OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_ENSURE(sal_False, aMsg.getStr());
    }

    return m_bRestored;
}


void SAL_CALL SessionListener::doSave( sal_Bool bShutdown, sal_Bool /*bCancelable*/ )
    throw (RuntimeException)
{
    if (bShutdown)
    {
        sal_Bool bDispatched = sal_False;
        ResetableGuard aGuard(m_aLock);
        try
        {
            // xd create SERVICENAME_AUTORECOVERY -> XDispatch
            // xd->dispatch("vnd.sun.star.autorecovery:/doSessionSave, async=true
            // on stop event m_rSessionManager->saveDone(this);

            css::uno::Reference< XDispatch > xDispatch(m_xSMGR->createInstance(SERVICENAME_AUTORECOVERY), UNO_QUERY_THROW);
            css::uno::Reference< XURLTransformer > xURLTransformer(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), UNO_QUERY_THROW);
            URL aURL;
            aURL.Complete = OUString::createFromAscii("vnd.sun.star.autorecovery:/doSessionSave");
            xURLTransformer->parseStrict(aURL);
            xDispatch->addStatusListener(this, aURL);
            Sequence< PropertyValue > args(1);
            args[0] = PropertyValue(OUString::createFromAscii("DispatchAsynchron"),-1,makeAny(sal_True),PropertyState_DIRECT_VALUE);
            xDispatch->dispatch(aURL, args);
            bDispatched = sal_True;
            // on stop event set call m_rSessionManager->saveDone(this);
        } catch (com::sun::star::uno::Exception& e) {
            OString aMsg = OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
            OSL_ENSURE(sal_False, aMsg.getStr());
            // save failed, but tell manager to go on if we havent yet dispatched the request
            if (m_rSessionManager.is() && !bDispatched)
                m_rSessionManager->saveDone(this);
        }
    }
    // we don't have anything to do so tell the session manager we're done
    else if( m_rSessionManager.is() )
        m_rSessionManager->saveDone( this );
}



void SAL_CALL SessionListener::approveInteraction( sal_Bool /*bInteractionGranted*/ )
    throw (RuntimeException)
{}

void SessionListener::shutdownCanceled()
    throw (RuntimeException)
{}

}
