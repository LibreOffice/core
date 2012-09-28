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
package test;

import com.sun.star.task.XJob;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.comp.loader.FactoryHelper;


public class Job {

    static public class _Implementation implements XServiceInfo, XTypeProvider,
            XJob, XNamed {
    static private final String __serviceName = "test.Job";
        Object oDoc = null;
        String actionType = null;
        String actionParm = null;
        XMultiServiceFactory oMSF = null;


    public _Implementation(XMultiServiceFactory xMSF) {
            oMSF = xMSF;
    }

    // XServiceInfo
    public String getImplementationName() throws com.sun.star.uno.RuntimeException {
        return getClass().getName();
    }

    public boolean supportsService(String serviceName) throws com.sun.star.uno.RuntimeException {
        return __serviceName.equals(serviceName);
    }

    public String[] getSupportedServiceNames() throws com.sun.star.uno.RuntimeException {
        return new String[] {__serviceName};
    }

        public static int executed = 0;
        public Object execute(NamedValue[] args) {
            executed++;

            return null;
        }

        public String getName() {
            return  "" + executed;
        }

        public void setName(String n) {
        }

    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    public Type[] getTypes() {
        Class<?> interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
        types[i] = new Type(interfaces[i]);
        return types;
    }

    }

    public static XSingleServiceFactory __getServiceFactory(String implName,
                                XMultiServiceFactory multiFactory,
                                XRegistryKey regKey) {
    XSingleServiceFactory xSingleServiceFactory = null;
    xSingleServiceFactory = FactoryHelper.getServiceFactory(_Implementation.class,
                                    _Implementation.__serviceName,
                                    multiFactory,
                                    regKey);
    return xSingleServiceFactory;
    }


    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
    boolean result = true;
    result = FactoryHelper.writeRegistryServiceInfo(_Implementation.class.getName(),
                              _Implementation.__serviceName, regKey);
    return result;
    }
}
