/*************************************************************************
 *
 *  $RCSfile: intercept.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: abi $ $Date: 2003-03-28 16:31:27 $
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


#include "intercept.hxx"
#ifndef _EMBEDDOC_HXX_
#include "embeddoc.hxx"
#endif



using namespace ::com::sun::star;



#define IUL 5



uno::Sequence<::rtl::OUString> Interceptor::m_aInterceptedURL(IUL);



Interceptor::Interceptor(EmbedDocument_Impl* pOLEInterface)
    : m_pOLEInterface(pOLEInterface)
{
    m_aInterceptedURL[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:Save"));
    m_aInterceptedURL[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:SaveAll"));
    m_aInterceptedURL[2] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseDoc"));
    m_aInterceptedURL[3] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseWin"));
    m_aInterceptedURL[4] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(".uno:CloseFrame"));
}




//XDispatch
void SAL_CALL
Interceptor::dispatch(
    const util::URL& URL,
    const uno::Sequence<
    beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    if(m_pOLEInterface)
        if(URL.Complete == m_aInterceptedURL[0])
            m_pOLEInterface->SaveObject();
        else if(URL.Complete == m_aInterceptedURL[2] ||
                URL.Complete == m_aInterceptedURL[3] ||
                URL.Complete == m_aInterceptedURL[4])
            m_pOLEInterface->Close(1);
}


void SAL_CALL
Interceptor::addStatusListener(
    const uno::Reference<
    frame::XStatusListener >& Control,
    const util::URL& URL )
    throw (
        uno::RuntimeException
    )
{
    return;
}


void SAL_CALL
Interceptor::removeStatusListener(
    const uno::Reference<
    frame::XStatusListener >& Control,
    const util::URL& URL )
    throw (
        uno::RuntimeException
    )
{
    return;
}


//XInterceptorInfo
uno::Sequence< ::rtl::OUString >
SAL_CALL
Interceptor::getInterceptedURLs(  )
    throw (
        uno::RuntimeException
    )
{
    // now implemented as update

    return m_aInterceptedURL;
}


// XDispatchProvider

uno::Reference< frame::XDispatch > SAL_CALL
Interceptor::queryDispatch(
    const util::URL& URL,
    const ::rtl::OUString& TargetFrameName,
    sal_Int32 SearchFlags )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    if(URL.Complete == m_aInterceptedURL[0])
        return (frame::XDispatch*)this;
    else if(URL.Complete == m_aInterceptedURL[1])
        return (frame::XDispatch*)0   ;
    else if(URL.Complete == m_aInterceptedURL[2])
        return (frame::XDispatch*)this;
    else if(URL.Complete == m_aInterceptedURL[3])
        return (frame::XDispatch*)this;
    else if(URL.Complete == m_aInterceptedURL[4])
        return (frame::XDispatch*)this;
    else {
        if(m_xSlaveDispatchProvider.is())
            return m_xSlaveDispatchProvider->queryDispatch(
                URL,TargetFrameName,SearchFlags);
        else
            return uno::Reference<frame::XDispatch>(0);
    }
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL
Interceptor::queryDispatches(
    const uno::Sequence<frame::DispatchDescriptor >& Requests )
    throw (
        uno::RuntimeException
    )
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aRet;
    osl::MutexGuard aGuard(m_aMutex);
    if(m_xSlaveDispatchProvider.is())
        aRet = m_xSlaveDispatchProvider->queryDispatches(Requests);
    else
        aRet.realloc(Requests.getLength());

    for(sal_Int32 i = 0; i < Requests.getLength(); ++i)
        if(m_aInterceptedURL[0] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) this;
        else if(m_aInterceptedURL[1] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) 0;
        else if(m_aInterceptedURL[2] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) this;
        else if(m_aInterceptedURL[3] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) this;
        else if(m_aInterceptedURL[4] == Requests[i].FeatureURL.Complete)
            aRet[i] = (frame::XDispatch*) this;

    return aRet;
}



//XDispatchProviderInterceptor

uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getSlaveDispatchProvider(  )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xSlaveDispatchProvider;
}

void SAL_CALL
Interceptor::setSlaveDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewDispatchProvider )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xSlaveDispatchProvider = NewDispatchProvider;
}


uno::Reference< frame::XDispatchProvider > SAL_CALL
Interceptor::getMasterDispatchProvider(  )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    return m_xMasterDispatchProvider;
}


void SAL_CALL
Interceptor::setMasterDispatchProvider(
    const uno::Reference< frame::XDispatchProvider >& NewSupplier )
    throw (
        uno::RuntimeException
    )
{
    osl::MutexGuard aGuard(m_aMutex);
    m_xMasterDispatchProvider = NewSupplier;
}
