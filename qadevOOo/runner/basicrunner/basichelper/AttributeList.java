/*************************************************************************
 *
 *  $RCSfile: AttributeList.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 16:27:29 $
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
