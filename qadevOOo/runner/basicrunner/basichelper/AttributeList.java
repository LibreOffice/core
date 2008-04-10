/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AttributeList.java,v $
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
import com.sun.star.uno.Type;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.xml.sax.XAttributeList;
import util.XMLTools;

/**
* The class provides an implementation of the service
* <code>com.sun.star.xml.sax.XAttributeList</code>.
* @see com.sun.star.xml.sax.XAttributeList
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.lang.XSingleServiceFactory
*/
public class AttributeList implements XServiceInfo, XSingleServiceFactory {
    /** The service name of this class  **/
    static final String __serviceName = "basichelper.AttributeList";

    /**
     * Returns True, of the service is supported.
     * @param name The service name.
     * @return True, if the service is supported.
     */
    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    /**
     * Get all supported services.
     * @return The supported services.
     */
    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    /**
     * Ask for the implementation name.
     * @return The implementation name.
     */
    public String getImplementationName() {
        return getClass().getName();
    }

    /**
     * Create an instance of the actual implementation of the AttributeList.
     * Arguments are not supported, so they will bge ignored.
     * @param args The arguments.
     * @return A new instance of this class.
     */
    public Object createInstanceWithArguments(Object[] args) {
        return new AttributeListImpl();
    }

    /**
     * Create an instance of this class.
     * @return A new instance of this class.
     */
    public Object createInstance() {
        return createInstanceWithArguments(null);
    }
}

/**
 * The actual implementation of the service
 * <code>com.sun.star.xml.sax.XAttributeList</code>.
 * Extends the class util.XMLTools.AttributeList.
 * @see util.XMLTools.AttributeList
 * @see com.sun.star.xml.sax.XAttributeList
 * @see com.sun.star.lang.XTypeProvider
 * @see com.sun.star.lang.XInitialization
 */
class AttributeListImpl extends XMLTools.AttributeList
                            implements XTypeProvider, XInitialization {

                                /**
                                 * Initialize this class.
                                 * @param p0 An array of XML attributes that are added to the list.
                                 * @throws Exception Initialize failed.
                                 */
    public void initialize(Object[] p0) throws com.sun.star.uno.Exception {
        for(int i = 0; i + 2 < p0.length; i += 3) {
            add((String)p0[i], (String)p0[i + 1], (String)p0[i + 2]);
        }
    }

    /**
     * Return all implemented types of this class.
     * @return All UNO types of this class.
     */
    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Class superInterfaces[] = getClass().getSuperclass().getInterfaces();

        Type types[] = new Type[interfaces.length + superInterfaces.length];
        int i = 0;
        for(; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        for(; i < interfaces.length + superInterfaces.length; ++ i)
            types[i] = new Type(superInterfaces[i - interfaces.length]);
        return types;
    }

    /**
     * Get a unique id for this class
     * @return The id.
     */
    public byte[] getImplementationId() {
        return toString().getBytes();
    }
}
