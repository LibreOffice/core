/*************************************************************************
 *
 *  $RCSfile: intercept.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2003-10-29 12:59:21 $
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

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include "intercept.hxx"
#include "docholder.hxx"
#include "commonembobj.hxx"

using namespace ::com::sun::star;


#define IUL 5


uno::Sequence< ::rtl::OUString > Interceptor::m_aInterceptedURL(IUL);

struct equalOUString
{
    bool operator()(
        const rtl::OUString& rKey1,
        const rtl::OUString& rKey2 ) const
    {
        return !!( rKey1 == rKey2 );
    }
};


struct hashOUString
{
    size_t operator()( const rtl::OUString& rName ) const
    {
        return rName.hashCode();
    }
};



class StatusChangeListenerContainer
    : public ::cppu::OMultiTypeInterfaceContainerHelperVar<
rtl::OUString,hashOUString,equalOUString>
{
public:
    StatusChangeListenerContainer( ::osl::Mutex& aMutex )
        :  cppu::OMultiTypeInterfaceContainerHelperVar<
    rtl::OUString,hashOUString,equalOUString>(aMutex)
    {
    }
};


void Interceptor::DisconnectDocHolder()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_pDocHolder = NULL;
}

void SAL_CALL
Interceptor::addEventListener(
    const uno::Reference<lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ! m_pDisposeEventListeners )
        m_pDisposeEventListeners =
            new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


void SAL_CALL
Interceptor::removeEventListener(
    const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


void SAL_CALL Interceptor::dispose()
    throw( uno::RuntimeException )
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast< frame::XDispatch* >( this );

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        m_pDisposeEventListeners->disposeAndClear( aEvt );

    if(m_pStatCL)
        m_pStatCL->disposeAndClear( aEvt );

    m_xSlaveDispatchProvider = 0;
    m_xMasterDispatchProvider = 0;
}



Interceptor::Interceptor( DocumentHolder* pDocHolder )
    : m_pDocHolder( pDocHolder ),
      m_pStatCL(0),
      m_pDisposeEventListeners(0)
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


Interceptor::~Interceptor()
{
    if( m_pDisposeEventListeners )
        delete m_pDisposeEventListeners;

    if(m_pStatCL)
        delete m_pStatCL;
}



//XDispatch
void SAL_CALL
Interceptor::dispatch(
    const util::URL& URL,
    const uno::Sequence<
    beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if( m_pDocHolder )
        if(URL.Complete == m_aInterceptedURL[0])
            m_pDocHolder->GetEmbedObject()->SaveObject_Impl();
        else if(URL.Complete == m_aInterceptedURL[2] ||
                URL.Complete == m_aInterceptedURL[3] ||
                URL.Complete == m_aInterceptedURL[4])
        {
            try {
                m_pDocHolder->GetEmbedObject()->changeState( embed::EmbedStates::EMBED_RUNNING );
            }
            catch( uno::Exception& )
            {
            }
        }
}

void Interceptor::GenerateFeatureStateEvent()
{
    if(m_pStatCL)
    {
        for(int i = 0; i < IUL; ++i)
        {
            if( i == 1 )
                continue;

            cppu::OInterfaceContainerHelper* pICH =
                m_pStatCL->getContainer(m_aInterceptedURL[i]);
            uno::Sequence<uno::Reference<uno::XInterface> > aSeq;
            if(pICH)
                aSeq = pICH->getElements();
            if(!aSeq.getLength())
                continue;

            frame::FeatureStateEvent aStateEvent;
            aStateEvent.IsEnabled = sal_True;
            aStateEvent.Requery = sal_False;
            if(i == 0)
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
                aStateEvent.FeatureDescriptor = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Update"));
                aStateEvent.State <<= (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("($1) ")) + m_pDocHolder->GetTitle() );

            }
            else
            {
                aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
                aStateEvent.FeatureDescriptor = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Close and Return"));
                aStateEvent.State <<= (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("($2) ")) + m_pDocHolder->GetTitle() );

            }

            for(sal_Int32 k = 0; k < aSeq.getLength(); ++k)
            {
                uno::Reference<frame::XStatusListener>
                    Control(aSeq[k],uno::UNO_QUERY);
                if(Control.is())
                    Control->statusChanged(aStateEvent);

            }
        }
    }
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
    if(!Control.is())
        return;

    if(URL.Complete == m_aInterceptedURL[0])
    {   // Save
        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[0];
        aStateEvent.FeatureDescriptor = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Update"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("($1) ")) + m_pDocHolder->GetTitle() );
        Control->statusChanged(aStateEvent);

        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL =
                    new StatusChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
        return;
    }

    sal_Int32 i = 2;
    if(URL.Complete == m_aInterceptedURL[i] ||
       URL.Complete == m_aInterceptedURL[++i] ||
       URL.Complete == m_aInterceptedURL[++i] )
    {   // Close and return
        frame::FeatureStateEvent aStateEvent;
        aStateEvent.FeatureURL.Complete = m_aInterceptedURL[i];
        aStateEvent.FeatureDescriptor = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("Close and Return"));
        aStateEvent.IsEnabled = sal_True;
        aStateEvent.Requery = sal_False;
        aStateEvent.State <<= (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("($2) ")) + m_pDocHolder->GetTitle() );
        Control->statusChanged(aStateEvent);


        {
            osl::MutexGuard aGuard(m_aMutex);
            if(!m_pStatCL)
                m_pStatCL =
                    new StatusChangeListenerContainer(m_aMutex);
        }

        m_pStatCL->addInterface(URL.Complete,Control);
        return;
    }
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
    if(!(Control.is() && m_pStatCL))
        return;
    else {
        m_pStatCL->removeInterface(URL.Complete,Control);
        return;
    }
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
