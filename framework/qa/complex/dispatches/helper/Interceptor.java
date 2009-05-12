/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Interceptor.java,v $
 * $Revision: 1.3 $
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
package complex.dispatches;

// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;

// exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

// interfaces
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProviderInterceptor;
import com.sun.star.frame.XDispatchProviderInterception;
import com.sun.star.frame.XInterceptorInfo;

// helper
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

// others
//import java.lang.*;

// __________ Implementation __________

/**
 * implements a configurable interceptor for dispatch events.
 */
public class Interceptor implements com.sun.star.frame.XDispatchProvider,
                                    com.sun.star.frame.XDispatch,
                                    com.sun.star.frame.XDispatchProviderInterceptor,
                                    com.sun.star.frame.XInterceptorInfo
{
    // ____________________

    /** contains the list of interception URL schema's (wildcards are allowed there!)
        supported by this interceptor. It can be set from outside.
        If no external URLs are set, the default "*" is used instead.
        That would have the same effect as if this implementation would not support the
        interface XInterceptorInfo !
     */
    private String[] m_lURLs4InterceptionInfo = null;

    // ____________________

    /** These URL's will be blocked by this interceptor.
        Can be set from outside. Every queryDispatch() for these
        set of URL's will be answered with an empty dispatch object!
        If no external URLs are set the default "*" is used instead.
        So every incoming URL will be blocked .-)
     */
    private String[] m_lURLs4Blocking = null;

    // ____________________

    /** Every dispatch interceptor knows it's master and slave interceptor
        of the dispatch chain. These values must be stupid handled .-)
        They have to be set and reset in case the right interface methods are called.
        Nothing more. It's not allowed to dispose() it.
        The slave can be used inside queryDispatch() to forward requests,
        which are not handled by this interceptor instance.
     */
    private com.sun.star.frame.XDispatchProvider m_xSlave  = null;
    private com.sun.star.frame.XDispatchProvider m_xMaster = null;

    // ____________________

    /** counts calls of setSlave...().
        So the outside API test can use this value to know if this interceptor
        was realy added to the interceptor chain of OOo.
     */
    private int m_nRegistrationCount = 0;

    // ____________________

    /** indicates if this interceptor object is currently part of the interceptor
        chain of OOo. Only true if a valid slave or master dispatch is set on this
        instance.
     */
    private boolean m_bIsRegistered = false;

    // ____________________

    /** used for log output.
     */
    private LogWriter m_aLog;

    // ____________________

    /** ctor
     *  It's initialize an object of this class with default values.
     */
    public Interceptor(LogWriter aLog)
    {
        m_aLog = aLog;
    }

    // ____________________

    /** XInterceptorInfo */
    public synchronized String[] getInterceptedURLs()
    {
        return impl_getURLs4InterceptionInfo();
    }

    // ____________________

    /** XDispatchProviderInterceptor */
    public synchronized com.sun.star.frame.XDispatchProvider getSlaveDispatchProvider()
    {
        m_aLog.println("Interceptor.getSlaveDispatchProvider() called");
        return m_xSlave;
    }

    // ____________________

    /** XDispatchProviderInterceptor */
    public synchronized com.sun.star.frame.XDispatchProvider getMasterDispatchProvider()
    {
        m_aLog.println("Interceptor.getMasterDispatchProvider() called");
        return m_xMaster;
    }

    // ____________________

    /** XDispatchProviderInterceptor */
    public synchronized void setSlaveDispatchProvider(com.sun.star.frame.XDispatchProvider xSlave)
    {
        m_aLog.println("Interceptor.setSlaveDispatchProvider("+xSlave+") called");

        if (xSlave != null)
        {
            ++m_nRegistrationCount;
            m_bIsRegistered = true;
        }
        else
            m_bIsRegistered = false;

        m_xSlave = xSlave;
    }

    // ____________________

    /** XDispatchProviderInterceptor */
    public synchronized void setMasterDispatchProvider(com.sun.star.frame.XDispatchProvider xMaster)
    {
        m_aLog.println("Interceptor.setMasterDispatchProvider("+xMaster+") called");
        m_xMaster = xMaster;
    }

    // ____________________

    /** XDispatchProvider
     */
    public synchronized com.sun.star.frame.XDispatch queryDispatch(com.sun.star.util.URL aURL            ,
                                                                   String                sTargetFrameName,
                                                                   int                   nSearchFlags    )
    {
        m_aLog.println("Interceptor.queryDispatch('"+aURL.Complete+"', '"+sTargetFrameName+"', "+nSearchFlags+") called");

        if (impl_isBlockedURL(aURL.Complete))
        {
            m_aLog.println("Interceptor.queryDispatch(): URL blocked => returns NULL");
            return null;
        }

        if (m_xSlave != null)
        {
            m_aLog.println("Interceptor.queryDispatch(): ask slave ...");
            return m_xSlave.queryDispatch(aURL, sTargetFrameName, nSearchFlags);
        }

        m_aLog.println("Interceptor.queryDispatch(): no idea => returns this");
        return this;
    }

    // ____________________

    /** XDispatchProvider
     */
    public com.sun.star.frame.XDispatch[] queryDispatches(com.sun.star.frame.DispatchDescriptor[] lRequests)
    {
        int i = 0;
        int c = lRequests.length;

        com.sun.star.frame.XDispatch[] lResults = new com.sun.star.frame.XDispatch[c];
        for (i=0; i<c; ++i)
        {
            lResults[i] = queryDispatch(lRequests[i].FeatureURL ,
                                        lRequests[i].FrameName  ,
                                        lRequests[i].SearchFlags);
        }

        return lResults;
    }

    // ____________________

    /** XDispatch
     */
    public synchronized void dispatch(com.sun.star.util.URL              aURL      ,
                                      com.sun.star.beans.PropertyValue[] lArguments)
    {
        m_aLog.println("Interceptor.dispatch('"+aURL.Complete+"') called");
    }

    // ____________________

    /** XDispatch
     */
    public synchronized void addStatusListener(com.sun.star.frame.XStatusListener xListener,
                                               com.sun.star.util.URL              aURL     )
    {
        m_aLog.println("Interceptor.addStatusListener(..., '"+aURL.Complete+"') called");
    }

    // ____________________

    /** XDispatch
     */
    public synchronized void removeStatusListener(com.sun.star.frame.XStatusListener xListener,
                                                  com.sun.star.util.URL              aURL     )
    {
        m_aLog.println("Interceptor.removeStatusListener(..., '"+aURL.Complete+"') called");
    }

    // ____________________

    public synchronized int getRegistrationCount()
    {
        return m_nRegistrationCount;
    }

    // ____________________

    public synchronized boolean isRegistered()
    {
        return m_bIsRegistered;
    }

    // ____________________

    /** set a new list of URL's, which should be used on registration time
        (that's why it's neccessary to call this impl-method before the interceptor
        is used at the OOo API!) to optimize the interception chain.
     */
    public synchronized void setURLs4InterceptionInfo(String[] lURLs)
    {
        m_lURLs4InterceptionInfo = lURLs;
    }

    // ____________________

    /** set a new list of URL's, which should be blocked by this interceptor.
        (that's why it's neccessary to call this impl-method before the interceptor
        is used at the OOo API!)
     */
    public synchronized void setURLs4URLs4Blocking(String[] lURLs)
    {
        m_lURLs4Blocking = lURLs;
    }

    // ____________________

    /** must be used internal to access the member m_lURLs4InterceptionInfo
        - threadsafe
        - and to make sure it's initialized on demand
     */
    private synchronized String[] impl_getURLs4InterceptionInfo()
    {
        if (m_lURLs4InterceptionInfo == null)
        {
            m_lURLs4InterceptionInfo    = new String[1];
            m_lURLs4InterceptionInfo[0] = "*";
        }

        return m_lURLs4InterceptionInfo;
    }

    // ____________________

    /** must be used internal to access the member m_lURLs4Blocking
        - threadsafe
        - and to make sure it's initialized on demand
     */
    private synchronized String[] impl_getURLs4Blocking()
    {
        if (m_lURLs4Blocking == null)
        {
            m_lURLs4Blocking    = new String[1];
            m_lURLs4Blocking[0] = "*";
        }

        return m_lURLs4Blocking;
    }

    // ____________________
    private boolean impl_isBlockedURL(String sURL)
    {
        String[] lBlockedURLs = impl_getURLs4Blocking();
        int      i            = 0;
        int      c            = lBlockedURLs.length;

        for (i=0; i<c; ++i)
        {
            if (impl_match(sURL, lBlockedURLs[i]))
                return true;
        }

        return false;
    }

    // ____________________

    private boolean impl_match(String sVal1, String sVal2)
    {
        // TODO implement wildcard match
        return (sVal1.equals(sVal2));
    }
}
