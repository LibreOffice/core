/*************************************************************************
 *
 *  $RCSfile: dispatchhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:50 $
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

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_SERVICES_DISPATCHHELPER_HXX_
#include <services/dispatchhelper.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

//_______________________________________________
// includes of other projects

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// non exported const

//_______________________________________________
// non exported definitions

//_______________________________________________
// declarations

//_______________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_4(DispatchHelper                                       ,
                    OWeakObject                                          ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider           ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo            ),
                    DIRECT_INTERFACE(css::frame::XDispatchHelper        ),
                    DIRECT_INTERFACE(css::frame::XDispatchResultListener))

DEFINE_XTYPEPROVIDER_4(DispatchHelper                     ,
                       css::lang::XTypeProvider           ,
                       css::lang::XServiceInfo            ,
                       css::frame::XDispatchHelper        ,
                       css::frame::XDispatchResultListener)

DEFINE_XSERVICEINFO_MULTISERVICE(DispatchHelper                   ,
                                 ::cppu::OWeakObject              ,
                                 SERVICENAME_DISPATCHHELPER       ,
                                 IMPLEMENTATIONNAME_DISPATCHHELPER)

DEFINE_INIT_SERVICE( DispatchHelper, {} )

//_______________________________________________

/** ctor.

    @param xSMGR    the global uno service manager, which can be used to create own needed services.
*/
DispatchHelper::DispatchHelper( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        :   ThreadHelpBase(     )
        ,   OWeakObject   (     )
        // Init member
        ,   m_xSMGR       (xSMGR)
{
}

//_______________________________________________

/** dtor.
*/
DispatchHelper::~DispatchHelper()
{
}

//_______________________________________________

/** capsulate all steps of a dispatch request and provide so an easy way for dispatches.

    @param xDispatchProvider
                identifies the object, which provides may be valid dispatch objects for this execute.

    @param sURL
                describes the requested feature.

    @param sTargetFrameName
                points to the frame, which must be used (or may be created) for this dispatch.

    @param nSearchFlags
                in case the <var>sTargetFrameName</var> isn't unique, these flags regulate further searches.

    @param lArguments
                optional arguments for this request.

    @return An Any which capsulate a possible result of the internal wrapped dispatch.
 */
css::uno::Any SAL_CALL DispatchHelper::executeDispatch(
                                const css::uno::Reference< css::frame::XDispatchProvider >& xDispatchProvider ,
                                const ::rtl::OUString&                                      sURL              ,
                                const ::rtl::OUString&                                      sTargetFrameName  ,
                                      sal_Int32                                             nSearchFlags      ,
                                const css::uno::Sequence< css::beans::PropertyValue >&      lArguments        )
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xTHIS(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    // check for valid parameters
    if (
        (!xDispatchProvider.is()) ||
        (sURL.getLength()<1     )
       )
    {
        throw css::uno::RuntimeException(DECLARE_ASCII("called with invalid parameters"), xTHIS);
    }

    // parse given URL
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::util::XURLTransformer > xParser(m_xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), css::uno::UNO_QUERY);
    aReadLock.unlock();
    /* } SAFE */

    if (!xParser.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("couldn't create neccessary URL parser"), xTHIS);

    css::util::URL aURL;
    aURL.Complete = sURL;
    xParser->parseStrict(aURL);

    // search dispatcher
    css::uno::Reference< css::frame::XDispatch >          xDispatch       = xDispatchProvider->queryDispatch(aURL, sTargetFrameName, nSearchFlags);
    css::uno::Reference< css::frame::XNotifyingDispatch > xNotifyDispatch (xDispatch, css::uno::UNO_QUERY);

    if (!xDispatch.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("no dispatch available"), xTHIS);

    css::uno::Any aResult;
    if (xNotifyDispatch.is())
    {
        // dispatch it with guaranteed notification
        // Here we can hope for a result ... instead of the normal dispatch.
        css::uno::Reference< css::frame::XDispatchResultListener > xListener(xTHIS, css::uno::UNO_QUERY);
        /* SAFE { */
        WriteGuard aWriteLock(m_aLock);
        m_xBroadcaster = css::uno::Reference< css::uno::XInterface >(xNotifyDispatch, css::uno::UNO_QUERY);
        m_aResult      = css::uno::Any();
        m_aBlock.reset();
        aWriteLock.unlock();
        /* } SAFE */

        // dispatch it and wait for a notification
        xNotifyDispatch->dispatchWithNotification(aURL, lArguments, xListener);
        m_aBlock.wait();
        aResult = m_aResult;
    }
    else
    if (xDispatch.is())
    {
        // dispatch it without any chance to get a result
        xDispatch->dispatch(aURL,lArguments);
    }

    return aResult;
}

//_______________________________________________

/** callback for started dispatch with guaranteed notifications.

    We must save the result, so the method executeDispatch() can return it.
    Further we must release the broadcaster (otherwhise it can't die)
    and unblock the waiting executeDispatch() request.

    @param  aResult
                describes the result of the dispatch operation
 */
void SAL_CALL DispatchHelper::dispatchFinished( const css::frame::DispatchResultEvent& aResult )
    throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_aResult      = aResult.Result;
    m_xBroadcaster = css::uno::Reference< css::uno::XInterface >();
    m_aBlock.set();
    aWriteLock.unlock();
    /* } SAFE */
}

//_______________________________________________

/** we has to realease our broadcaster reference.

    @param aEvent
                describe the source of this event and MUST be our save broadcaster!
 */
void SAL_CALL DispatchHelper::disposing( const css::lang::EventObject& aEvent )
    throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    if (aEvent.Source == m_xBroadcaster)
        m_xBroadcaster = css::uno::Reference< css::uno::XInterface >();
    aWriteLock.unlock();
    /* } SAFE */
}

}
