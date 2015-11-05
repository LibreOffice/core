/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.dispatches;

// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;

// exceptions
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatchProviderInterceptor;
import com.sun.star.frame.XInterceptorInfo;
import com.sun.star.frame.XStatusListener;

// interfaces


// helper
import com.sun.star.util.URL;

// __________ Implementation __________

/**
 * implements a configurable interceptor for dispatch events.
 */
public class Interceptor implements XDispatch,
                                    XDispatchProviderInterceptor,
                                    XInterceptorInfo
{


    /** contains the list of interception URL schema's (wildcards are allowed there!)
        supported by this interceptor. It can be set from outside.
        If no external URLs are set, the default "*" is used instead.
        That would have the same effect as if this implementation would not support the
        interface XInterceptorInfo !
     */
    private String[] m_lURLs4InterceptionInfo = null;



    /** These URL's will be blocked by this interceptor.
        Can be set from outside. Every queryDispatch() for these
        set of URL's will be answered with an empty dispatch object!
        If no external URLs are set the default "*" is used instead.
        So every incoming URL will be blocked .-)
     */
    private String[] m_lURLs4Blocking = null;



    /** Every dispatch interceptor knows it's master and slave interceptor
        of the dispatch chain. These values must be stupid handled .-)
        They have to be set and reset in case the right interface methods are called.
        Nothing more. It's not allowed to dispose() it.
        The slave can be used inside queryDispatch() to forward requests,
        which are not handled by this interceptor instance.
     */
    private XDispatchProvider m_xSlave  = null;
    private XDispatchProvider m_xMaster = null;



    /** counts calls of setSlave...().
        So the outside API test can use this value to know if this interceptor
        was really added to the interceptor chain of OOo.
     */
    private int m_nRegistrationCount = 0;



    /** indicates if this interceptor object is currently part of the interceptor
        chain of OOo. Only true if a valid slave or master dispatch is set on this
        instance.
     */
    private boolean m_bIsRegistered = false;




    /** XInterceptorInfo */
    public synchronized String[] getInterceptedURLs()
    {
        return impl_getURLs4InterceptionInfo();
    }



    /** XDispatchProviderInterceptor */
    public synchronized XDispatchProvider getSlaveDispatchProvider()
    {
        System.out.println("Interceptor.getSlaveDispatchProvider() called");
        return m_xSlave;
    }



    /** XDispatchProviderInterceptor */
    public synchronized XDispatchProvider getMasterDispatchProvider()
    {
        System.out.println("Interceptor.getMasterDispatchProvider() called");
        return m_xMaster;
    }



    /** XDispatchProviderInterceptor */
    public synchronized void setSlaveDispatchProvider(XDispatchProvider xSlave)
    {
        System.out.println("Interceptor.setSlaveDispatchProvider("+xSlave+") called");

        if (xSlave != null)
        {
            ++m_nRegistrationCount;
            m_bIsRegistered = true;
        }
        else
        {
            m_bIsRegistered = false;
        }

        m_xSlave = xSlave;
    }



    /** XDispatchProviderInterceptor */
    public synchronized void setMasterDispatchProvider(XDispatchProvider xMaster)
    {
        System.out.println("Interceptor.setMasterDispatchProvider("+xMaster+") called");
        m_xMaster = xMaster;
    }



    /** XDispatchProvider
     */
    public synchronized XDispatch queryDispatch(URL aURL            ,
                                                                   String                sTargetFrameName,
                                                                   int                   nSearchFlags    )
    {
        System.out.println("Interceptor.queryDispatch('"+aURL.Complete+"', '"+sTargetFrameName+"', "+nSearchFlags+") called");

        if (impl_isBlockedURL(aURL.Complete))
        {
            System.out.println("Interceptor.queryDispatch(): URL blocked => returns NULL");
            return null;
        }

        if (m_xSlave != null)
        {
            System.out.println("Interceptor.queryDispatch(): ask slave ...");
            return m_xSlave.queryDispatch(aURL, sTargetFrameName, nSearchFlags);
        }

        System.out.println("Interceptor.queryDispatch(): no idea => returns this");
        return this;
    }



    /** XDispatchProvider
     */
    public XDispatch[] queryDispatches(DispatchDescriptor[] lRequests)
    {
        int i = 0;
        int c = lRequests.length;

        XDispatch[] lResults = new XDispatch[c];
        for (i=0; i<c; ++i)
        {
            lResults[i] = queryDispatch(lRequests[i].FeatureURL ,
                                        lRequests[i].FrameName  ,
                                        lRequests[i].SearchFlags);
        }

        return lResults;
    }



    /** XDispatch
     */
    public synchronized void dispatch(URL              aURL      ,
                                      PropertyValue[] lArguments)
    {
        System.out.println("Interceptor.dispatch('"+aURL.Complete+"') called");
    }



    /** XDispatch
     */
    public synchronized void addStatusListener(XStatusListener xListener,
                                               com.sun.star.util.URL              aURL     )
    {
        System.out.println("Interceptor.addStatusListener(..., '"+aURL.Complete+"') called");
    }



    /** XDispatch
     */
    public synchronized void removeStatusListener(XStatusListener xListener,
                                                  com.sun.star.util.URL              aURL     )
    {
        System.out.println("Interceptor.removeStatusListener(..., '"+aURL.Complete+"') called");
    }



    public synchronized int getRegistrationCount()
    {
        return m_nRegistrationCount;
    }



    public synchronized boolean isRegistered()
    {
        return m_bIsRegistered;
    }


    /** set a new list of URL's, which should be blocked by this interceptor.
        (that's why it's necessary to call this impl-method before the interceptor
        is used at the OOo API!)
     */
    public synchronized void setURLs4URLs4Blocking(String[] lURLs)
    {
        m_lURLs4Blocking = lURLs;
    }



    /** must be used internal to access the member m_lURLs4InterceptionInfo
        - threadsafe
        - and to make sure it's initialized on demand
     */
    private synchronized String[] impl_getURLs4InterceptionInfo()
    {
        if (m_lURLs4InterceptionInfo == null)
        {
            m_lURLs4InterceptionInfo    = new String[] { "*" };
        }

        return m_lURLs4InterceptionInfo;
    }



    /** must be used internal to access the member m_lURLs4Blocking
        - threadsafe
        - and to make sure it's initialized on demand
     */
    private synchronized String[] impl_getURLs4Blocking()
    {
        if (m_lURLs4Blocking == null)
        {
            m_lURLs4Blocking    = new String[] { "*" };
        }

        return m_lURLs4Blocking;
    }


    private boolean impl_isBlockedURL(String sURL)
    {
        String[] lBlockedURLs = impl_getURLs4Blocking();
        int      i            = 0;
        int      c            = lBlockedURLs.length;

        for (i=0; i<c; ++i)
        {
            if (impl_match(sURL, lBlockedURLs[i]))
            {
                return true;
            }
        }

        return false;
    }



    private boolean impl_match(String sVal1, String sVal2)
    {
        // TODO implement wildcard match
        return sVal1.equals(sVal2);
    }
}
