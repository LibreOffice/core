/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package basicrunner.basichelper;

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;
import com.sun.star.lang.XTypeProvider;
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
