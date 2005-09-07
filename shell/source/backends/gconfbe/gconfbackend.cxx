/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gconfbackend.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:46:33 $
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

#include "gconfbackend.hxx"

#ifndef GCONFCOMMONLAYER_HXX_
#include "gconfcommonlayer.hxx"
#endif

#ifndef GCONFINETLAYER_HXX_
#include "gconfinetlayer.hxx"
#endif

#ifndef GCONFVCLLAYER_HXX_
#include "gconfvcllayer.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _RTL_BYTESEQ_H_
#include <rtl/byteseq.h>
#endif

#include <stdio.h>

//==============================================================================

/*
void ONotificationThread::run()
{
    mLoop= NULL;
    //Need to start a GMain loop for notifications to work
    mLoop=g_main_loop_new(g_main_context_default(),FALSE);
    g_main_loop_run(mLoop);
} ;

*/

//------------------------------------------------------------------------------

GconfBackend* GconfBackend::mInstance= 0;

GconfBackend* GconfBackend::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    if(mInstance == 0)
    {
        mInstance = new GconfBackend (xContext);
    }

    return mInstance;
}

//------------------------------------------------------------------------------

GconfBackend::GconfBackend(const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException)
    : BackendBase(mMutex), m_xContext(xContext)
//    , mNotificationThread(NULL)


{
}

//------------------------------------------------------------------------------

GconfBackend::~GconfBackend(void) {


//    delete (mNotificationThread);
    GconfBackend::mClient = NULL;

}

//------------------------------------------------------------------------------

GConfClient* GconfBackend::mClient= 0;


GConfClient* GconfBackend::getGconfClient()
{

    if (mClient == NULL)
    {
        /* initialize glib object type library */
        g_type_init();

        GError* aError = NULL;
        if (!gconf_init(0, NULL, &aError))
        {
            rtl::OUStringBuffer msg;
            msg.appendAscii("GconfBackend:GconfLayer: Cannot Initialize Gconf connection - " );
            msg.appendAscii(aError->message);

            g_error_free(aError);
            aError = NULL;
            throw uno::RuntimeException(msg.makeStringAndClear(),NULL);
        }

        mClient = gconf_client_get_default();
        if (!mClient)
        {
            throw uno::RuntimeException(rtl::OUString::createFromAscii
                ("GconfBackend:GconfLayer: Cannot Initialize Gconf connection"),NULL);
        }
    }

    return mClient;
}

//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL GconfBackend::getLayer(
    const rtl::OUString& aComponent, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{
    uno::Reference<backend::XLayer> xLayer;

    if( aComponent.equalsAscii("org.openoffice.Office.Common" ) )
    {
        xLayer = new GconfCommonLayer(m_xContext);
    }
    else if( aComponent.equalsAscii("org.openoffice.Inet" ) )
    {
        xLayer = new GconfInetLayer(m_xContext);
    }
    else if( aComponent.equalsAscii("org.openoffice.VCL" ) )
    {
        xLayer = new GconfVCLLayer(m_xContext);
    }

    return xLayer;
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
GconfBackend::getUpdatableLayer(const rtl::OUString& aComponent)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException( rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("GconfBackend: No Update Operation allowed, Read Only access") ),
        *this) ;
}

//------------------------------------------------------------------------------

static void
keyChangedCallback(GConfClient* aClient,
                   guint aID,
                   GConfEntry* aEntry,
                   gpointer aBackend)
{

    OSL_TRACE("In KeyChangedCallback Function");
    rtl::OUString aGconfKey= rtl::OUString::createFromAscii(aEntry->key);
    GconfBackend * aGconfBe = (GconfBackend*) aBackend;

    aGconfBe->notifyListeners(aGconfKey);

}

//------------------------------------------------------------------------------

void GconfBackend::notifyListeners(const rtl::OUString& aGconfKey)
{
/*
    //look up associated component from Map using GconfKey
    KeyMappingTable::iterator aIter;
    rtl::OUString aComponent;
    for( aIter= mKeyMap.begin(); aIter != mKeyMap.end(); aIter++)
    {
        if (aIter->second.mGconfName == aGconfKey)
        {
            aComponent = aIter->first;
            break;
        }

    }


    //Need to store updated layer TimeStamp as Gconf library
    //has no access to TimeStamp via its api
    TimeValue aTimeValue ={0,0} ;
    osl_getSystemTime(&aTimeValue);

    oslDateTime aLayerTS;
    rtl::OUString aTimeStamp;

    if (osl_getDateTimeFromTimeValue(&aTimeValue, &aLayerTS)) {
        sal_Char asciiStamp [20] ;

        sprintf(asciiStamp, "%04d%02d%02d%02d%02d%02dZ",
                aLayerTS.Year, aLayerTS.Month, aLayerTS.Day,
                aLayerTS.Hours, aLayerTS.Minutes, aLayerTS.Seconds) ;
        aTimeStamp = rtl::OUString::createFromAscii(asciiStamp) ;
    }


    TSMappingTable::iterator aTSIter;
    aTSIter = mTSMap.find(aComponent);
    if (aTSIter == mTSMap.end())
    {
        mTSMap.insert(TSMappingTable::value_type(aComponent,aTimeStamp));
    }
    else
    {
        aTSIter->second = aTimeStamp;

    }
    typedef ListenerList::iterator LLIter;
    typedef std::pair<LLIter, LLIter> BFRange;
    BFRange aRange = mListenerList.equal_range(aComponent);
    while (aRange.first != aRange.second)
    {
        LLIter cur = aRange.first++;
        backend::ComponentChangeEvent aEvent;
        aEvent.Component = aComponent;
        aEvent.Source = *this;
        cur->second->componentDataChanged(aEvent);

    }
*/
}


//------------------------------------------------------------------------------
void SAL_CALL GconfBackend::addChangesListener(
    const uno::Reference<backend::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
/*
    osl::MutexGuard aGuard(mMutex);

    GConfClient* aClient = getGconfClient();

    ListenerList::iterator aIter;
    aIter = mListenerList.find(aComponent);
    if (aIter == mListenerList.end())
    {
        typedef KeyMappingTable::iterator KMTIter;
        typedef std::pair<KMTIter, KMTIter> BFRange;

        BFRange aRange = mKeyMap.equal_range(aComponent);

        while (aRange.first != aRange.second)
        {
            KMTIter cur = aRange.first++;

            sal_Int32 nIndex = cur->second.mGconfName.lastIndexOf('/');
            rtl::OUString aDirectory = cur->second.mGconfName.copy(0, nIndex);
            rtl::OString aDirectoryStr= rtl::OUStringToOString(aDirectory, RTL_TEXTENCODING_ASCII_US);
            GError* aError = NULL;
            gconf_client_add_dir(aClient,
                                 aDirectoryStr.getStr(),
                                 GCONF_CLIENT_PRELOAD_NONE,
                                 &aError);

            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot register listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }

            rtl::OString aKey =  rtl::OUStringToOString(cur->second.mGconfName,
                                                        RTL_TEXTENCODING_ASCII_US);

            GConfClientNotifyFunc aNotifyFunc = &keyChangedCallback;
            sal_uInt32 aID = gconf_client_notify_add(
                                                    aClient,
                                                    aKey.getStr(),
                                                    aNotifyFunc,
                                                    this,
                                                    NULL,
                                                    &aError);
            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot register listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }



        }

    }
*/

/*
    if (mNotificationThread == NULL)
    {

        mNotificationThread = new ONotificationThread();

        if ( mNotificationThread == NULL)
        {
          OSL_ENSURE(false,"Could not start Notification Thread ");
        }
        else
        {
            mNotificationThread->create();
        }
    }
*/
    //Store listener in list
    mListenerList.insert(ListenerList::value_type(aComponent, xListener));


}
//------------------------------------------------------------------------------
void SAL_CALL GconfBackend::removeChangesListener(
    const uno::Reference<backend::XBackendChangesListener>& xListener,
    const rtl::OUString& aComponent)
    throw (::com::sun::star::uno::RuntimeException)
{
/*
    osl::MutexGuard aGuard(mMutex);
    GConfClient* aClient = GconfBackend::getGconfClient();
    ListenerList::iterator aIter;
    aIter = mListenerList.find(aComponent);
    if (aIter == mListenerList.end())
    {

        OSL_TRACE("GconfBackend:: Cannot deregister listener for Component %s - Listner not registered",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );

        typedef KeyMappingTable::iterator KMTIter;
        typedef std::pair<KMTIter, KMTIter> BFRange;

        BFRange aRange = mKeyMap.equal_range(aComponent);

        while (aRange.first != aRange.second)
    {
            KMTIter cur = aRange.first++;

            sal_Int32 nIndex = cur->second.mGconfName.lastIndexOf('/');
            rtl::OUString aDirectory = cur->second.mGconfName.copy(0, nIndex);
            rtl::OString aDirectoryStr= rtl::OUStringToOString(aDirectory, RTL_TEXTENCODING_ASCII_US);
            GError* aError = NULL;
            gconf_client_remove_dir(aClient,
                                    aDirectoryStr.getStr(),
                                    &aError);

            if(aError != NULL)
            {
                OSL_TRACE("GconfBackend:: Cannot deRegister listener for Component %s",
                    rtl::OUStringToOString(aComponent, RTL_TEXTENCODING_ASCII_US).getStr() );
            }
       }
    }
*/
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL GconfBackend::getBackendName(void) {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.GconfBackend") );
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL GconfBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL GconfBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.GconfBackend")) ;
    aServices[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServices ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL GconfBackend::getSupportedComponents(void)
{
    uno::Sequence<rtl::OUString> aSupportedComponentsList(3) ;
    aSupportedComponentsList[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.VCL")) ;
    aSupportedComponentsList[1] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Inet")) ;
    aSupportedComponentsList[2] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common")) ;

    return aSupportedComponentsList ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL GconfBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL GconfBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

// ---------------------------------------------------------------------------------------


