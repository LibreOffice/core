/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Job.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:43:49 $
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
package test;

import com.sun.star.lang.XInitialization;
import com.sun.star.task.XJob;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.util.XURLTransformer;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.lang.XMultiServiceFactory;
import util.utils;
import com.sun.star.lang.XComponent;
import lib.StatusException;
import util.SOfficeFactory;
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
        Class interfaces[] = getClass().getInterfaces();
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
