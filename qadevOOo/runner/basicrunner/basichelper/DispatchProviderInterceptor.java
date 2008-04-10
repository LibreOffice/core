/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DispatchProviderInterceptor.java,v $
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
package basicrunner.basichelper;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.frame.XDispatchProviderInterceptor;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.util.URL;

/**
 * This implementation provides an implementation of an interceptor.
 * @see com.sun.star.lang.XSingleServiceFactory
 * @see com.sun.star.lang.XServiceInfo
 */
public class DispatchProviderInterceptor implements XServiceInfo,
                                                    XSingleServiceFactory {
    /** The service name **/
    static final String __serviceName =
                            "basichelper.DispatchProviderInterceptor";

    /** Create an instance of the interceptor
     * Arguments are not supported here, so they will be ignored.
     * @param args The arguments.
     * @return A new instance of the interceptor.
     **/
    public Object createInstanceWithArguments(Object[] args) {
        return new InterceptorImpl();
    }

    /** Create an instance of the interceptor
     * @return A new instance of the interceptor.
     **/
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    /** Get the unique id for this implementation
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /** Get all implemented types.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();

        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);

        return types;
    }

    /**
     * Is this service supported?
     * @param name The name of a service.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    /**
     * Get all supported service names.
     * @return All service names.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    /**
     * Get the implementation name of this class.
     * @return The name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }
}

/**
 * The actual implementation of the interceptor.
 * @see com.sun.star.lang.XTypeProvider
 * @see com.sun.star.frame.XDispatchProviderInterceptor
 * @see com.sun.star.frame.XDispatchProvider
 */
class InterceptorImpl implements XDispatchProvider,
                                XDispatchProviderInterceptor, XTypeProvider {

    /** A master dispatch provider **/
    public XDispatchProvider master = null;
    /** A slave dispatch provider **/
    public XDispatchProvider slave = null;

    /** Get the slave dispatch provider
     * @return The slave.
     */
    public XDispatchProvider getSlaveDispatchProvider() {
        return slave;
    }
    /** Get the master dispatch provider
     * @return The master.
     */
    public XDispatchProvider getMasterDispatchProvider() {
        return master;
    }

    /** Set the slave dispatch provider
     * @param prov The new slave.
     */
    public void setSlaveDispatchProvider(XDispatchProvider prov) {
        slave = prov ;
    }

    /** Set the master dispatch provider
     * @param prov The new master.
     */
    public void setMasterDispatchProvider(XDispatchProvider prov) {
        master = prov ;
    }

    /** Searches for an <type>XDispatch</type> for the specified URL within
     * the specified target frame.
     * @param url The URL.
     * @param frame The target frame
     * @param flags Optional search flags.
     * @return The dispatch object which provides the queried functionality
     *         or null if no dispatch object is available.
     * @see com.sun.star.frame.XDispatch
     */
    public XDispatch queryDispatch(URL url, String frame, int flags) {
        return master.queryDispatch(url, frame, flags) ;
    }

    /**
     * Query for an array of <type>XDispatch</type>.
     * @param desc A list of dipatch requests.
     * @return A list of dispatch objects.
     */
    public XDispatch[] queryDispatches(DispatchDescriptor[] desc) {
        return master.queryDispatches(desc) ;
    }

    /** Get the unique id for this implementation
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /** Get all implemented types.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();

        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);

        return types;
    }
}
