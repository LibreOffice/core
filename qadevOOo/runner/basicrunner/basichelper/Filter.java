/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Filter.java,v $
 * $Revision: 1.5 $
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
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.document.XFilter;
import com.sun.star.beans.PropertyValue;


/**
 * Provides an implementation of XFilter.
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.lang.XSingleServiceFactory
 */
public class Filter implements XServiceInfo, XSingleServiceFactory {
    /** The service name of this class **/
    static final String __serviceName = "basichelper.Filter";
    /** The actual filter **/
    static FilterImpl oFilter = null;

    /**
     * Construct a new filter
     */
    public Filter() {
        oFilter = new FilterImpl();
    }

    /**
     * Returns an instance of the filter.
     * Arguments are not supported here and will be ignored.
     * @param args The arguments.
     * @return The filter.
     */
    public Object createInstanceWithArguments(Object[] args) {
        return oFilter;
    }

    /**
     * Returns an instance of the filter.
     * @return The filter.
     */
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    /**
     * Get a unique id for this implementation.
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Return all implemented types of this class.
     * @return The implemented UNO types.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();

        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);

        return types;
    }

    /** Is this servioce supported?
     * @param name The service name.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    /**
     * Get all supported service names.
     * @return All supported servcices.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    /**
     * Get the implementation name of this class.
     * @return The implementation name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }
}

/**
 * The actual filter implementation
 * @see com.sun.star.lang.XInitialization;
 * @see com.sun.star.lang.XTypeProvider;
 * @see com.sun.star.container.XNameAccess;
 */
class FilterImpl implements XInitialization, XTypeProvider, XNameAccess {
    /** A state **/
    static String aState;
    /** A result **/
    static boolean bResult;

    /**
     * Constructs a new filter.
     */
    public FilterImpl() {
        aState = "just created";
        bResult = false;
    }

    /**
     * Get the element names
     * @return All element names.
     */
    public String[] getElementNames() {
        return new String[]{"State", "Result"};
    }

    /**
     * Does this element exist?
     * @param name The element name.
     * @return True, if the name exists.
     */
    public boolean hasByName(String name) {
        return (name.equals("State") || name.equals("Result"));
    }

    /**
     * Get an element by its name.
     * @param name The name of the element.
     * @return The value of the element.
     * @throws NoSuchElementException The element does not exist.
     */
    public Object getByName(String name) throws NoSuchElementException{
        if (name.equals("State"))
            return aState;
        else if (name.equals("Result"))
            return new Boolean(bResult);
        else
            throw new NoSuchElementException();
    }

    /**
     * Are there elements
     * @return Always true.
     */
    public boolean hasElements() {
        return true;
    }

    /**
     * Get element type.
     * @return null.
     */
    public Type getElementType() {
        return null;
    }

    /**
     * Get a unique id for this implementation.
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    /**
     * Return all implemented types of this class.
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
     * Method initialize() creates a new thread that will try to start
     * filtering
     * @param parm1 An instance of XFilter
     * @see com.sun.star.document.XFilter
     * @throws Exception Is thrown, when initialize fails.
     */
    public void initialize(Object[] parm1) throws com.sun.star.uno.Exception {
        XFilter oFilter = (XFilter)UnoRuntime.queryInterface(
                                        XFilter.class, parm1[0]);
        PropertyValue[] FilterDesc = (PropertyValue[])UnoRuntime.queryInterface(PropertyValue[].class, parm1[1]);
        aState = "just initialized";
        FilterThread aThread = new FilterThread(oFilter, FilterDesc);
        aThread.start();
    }
}

/**
 * A thread for filtering.
 */
class FilterThread extends Thread {
    /** The filter that is used **/
    XFilter oFilter = null;
    /** Filter descriptions **/
    PropertyValue[] FilterDesc = null;

    /**
     * Construct the thread.
     * @param oObj The filter.
     * @param Desc The descriptions.
     */
    public FilterThread(XFilter oObj, PropertyValue[] Desc){
      oFilter = oObj;
      FilterDesc = Desc;
    }

    /**
     * Let the thread run
     */
    public void run(){
        boolean bOK;
        try {
            FilterImpl.aState = "before filtering";
            bOK = oFilter.filter(FilterDesc);
            FilterImpl.aState = "filtering finished";
            FilterImpl.bResult = bOK;
        } catch (Exception e) {
            ConnectorImpl.aState = "error";
            throw new RuntimeException("Can't filtering exception"
                                                            + e.toString());
        }
    }
}
