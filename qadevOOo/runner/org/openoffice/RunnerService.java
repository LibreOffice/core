/*************************************************************************
 *
 *  $RCSfile: RunnerService.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-03-31 17:01:51 $
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

package org.openoffice;

import share.LogWriter;
import stats.InternalLogWriter;
import lib.TestParameters;
import lib.DynamicClassLoader;
import base.TestBase;
import helper.AppProvider;
import helper.ClParser;
import helper.CfgParser;
import com.sun.star.beans.XPropertyAccess;
import com.sun.star.beans.PropertyValue;
import com.sun.star.task.XJob;
import com.sun.star.uno.XInterface;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.beans.NamedValue;

import java.util.Vector;
/**
 * The main class, will call ClParser and CfgParser to <br>
 * fill the TestParameters.<br>
 * Will then call the appropriate Testbase to run the tests.
 */
public class RunnerService implements XJob, XServiceInfo,
                                            XTypeProvider, XPropertyAccess {

    static public final String __serviceName = "org.openoffice.Runner";
    static public final String __implName = "org.openoffice.RunnerService";
    static private XMultiServiceFactory xMSF = null;

    /**
     * ct'tor
     * Construct an own office provider for tests
     */
    public RunnerService(XMultiServiceFactory xMSF) {
    }

    public Object execute(NamedValue[] args) {
        // construct valid arguments from the given stuff
        int arg_length=args.length;
        String[] arguments = new String[arg_length*2];
        for ( int i=0; i< arg_length; i++ ) {
            arguments[i*2] = args[i].Name;
            Object o = args[i].Value;
            arguments[i*2+1] = o.toString();
        }

        TestParameters param = new TestParameters();
        DynamicClassLoader dcl = new DynamicClassLoader();


        // take the standard log writer
        String standardLogWriter = param.LogWriter;
        String standardOutProducer = param.OutProducer;

        ClParser cli = new ClParser();

        //parse the arguments if an ini-parameter is given
        String iniFile = cli.getIniPath(arguments);

        //initialize cfgParser with ini-path
        CfgParser ini = new CfgParser(iniFile);

        //parse ConfigFile
        ini.getIniParameters(param);

        //parse the commandline arguments
        cli.getCommandLineParameter(param,arguments);

        // now compare the standard log writer with the parameters:
        // if we have a new one, use the new, else use the internal
        // log writer
        if (((String)param.get("LogWriter")).equals(standardLogWriter))
            param.put("LogWriter", "stats.InternalLogWriter");
        if (((String)param.get("OutProducer")).equals(standardOutProducer))
            param.put("OutProducer", "stats.InternalLogWriter");
        LogWriter log = (LogWriter) dcl.getInstance(
                                            (String)param.get("LogWriter"));

        param.put("ServiceFactory", xMSF);

        param.ServiceFactory = xMSF; //(XMultiServiceFactory)
                                     //       appProvider.getManager(param);

        log.println("TestJob: "+param.get("TestJob"));

        TestBase toExecute = (TestBase)dcl.getInstance("base.java_fat_service");

        boolean worked = toExecute.executeTest(param);
        if (!worked)
            log.println("Test did not execute correctly.");

        String returnString = "";
        if (log instanceof InternalLogWriter)
            returnString = ((InternalLogWriter)log).getLog();
        return returnString;
    }

    /**
     * This function provides the service name
     * @return the service name
     */
    public String getServiceName() {
        return __serviceName;
    }

    /**
     * Get all implemented types of this class.
     * @return An array of implemented interface types.
     * @see com.sun.star.lang.XTypeProvider
     */
    public Type[] getTypes() {
        Type[] type = new Type[5];
        type[0] = new Type(XInterface.class);
        type[1] = new Type(XTypeProvider.class);
        type[2] = new Type(XJob.class);
        type[3] = new Type(XServiceInfo.class);
        type[4] = new Type(XPropertyAccess.class);
        return type;
    }

    /**
     * Get the implementation id.
     * @return An empty implementation id.
     * @see com.sun.star.lang.XTypeProvider
     */
    public byte[] getImplementationId() {
        return new byte[0];
    }
    /**
     * Function for reading the implementation name.
     *
     * @return the implementation name
     * @see com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName() {
        return __implName;
    }

    /**
     * Does the implementation support this service?
     *
     * @param serviceName The name of the service in question
     * @return true, if service is supported, false otherwise
     * @see com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService(String serviceName) {
        if(serviceName.equals(__serviceName))
            return true;
        return false;
    }

    /**
     * Function for reading all supported services
     *
     * @return An aaray with all supported service names
     * @see com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames() {
        String[] supServiceNames = {__serviceName};
        return supServiceNames;
    }

    /**
     * Return all valid testcases from the object descriptions
     * @return The valid testcases as property values
     */
    public PropertyValue[] getPropertyValues() {
        PropertyValue[] pVal = null;
        java.net.URL url = this.getClass().getResource("/objdsc");
        if (url == null) {
           pVal = new PropertyValue[1];
           pVal[0] = new PropertyValue();
           pVal[0].Name = "Error";
           pVal[0].Value = "OOoRunner.jar file doesn't contain object " +
                           "descriptions: don't know what to test.";
           return pVal;
        }

        Vector v = new Vector(600);
        try {
            // open connection to  Jar
            java.net.JarURLConnection con =
                                (java.net.JarURLConnection)url.openConnection();
            // get Jar file from connection
            java.util.jar.JarFile f = con.getJarFile();
            // Enumerate over all entries
            java.util.Enumeration enum = f.entries();

            while (enum.hasMoreElements()) {
                String entry = enum.nextElement().toString();
                if (entry.endsWith(".csv")) {
                    String module = null;
                    String object = null;

                    int startIndex = entry.indexOf("objdsc/") + 7;
                    int endIndex = entry.indexOf('.');
                    module = entry.substring(startIndex, endIndex);
                    startIndex = 0;
                    endIndex = module.lastIndexOf('/');
                    module = module.substring(startIndex, endIndex);

                    // special cases
                    if (entry.indexOf("/file/") != -1 || entry.indexOf("/xmloff/") != -1) {
                        endIndex = entry.indexOf(".csv");
                        object = entry.substring(0, endIndex);
                        endIndex = object.lastIndexOf('.');
                        startIndex = object.indexOf('.');
                        while (startIndex != endIndex) {
                            object = object.substring(startIndex+1);
                            startIndex = object.indexOf('.');
                            endIndex = object.lastIndexOf('.');
                        }
                    }
/*                    else if (entry.indexOf("/xmloff/") != -1) {
                        endIndex = entry.indexOf(".csv");
                        object = entry.substring(0, endIndex);
                        endIndex = entry.lastIndexOf('.');
                        while (object.indexOf('.') != endIndex) {
                            object = object.substring(object.indexOf('.')+1);
                        }
                    } */
                    else {
                        startIndex = 0;
                        endIndex = entry.indexOf(".csv");
                        object = entry.substring(startIndex, endIndex);
                        startIndex = object.lastIndexOf('.');
                        object = object.substring(startIndex+1);
                    }
                    v.add(module+"."+object);
                }
            }
        }
        catch(java.io.IOException e) {
           e.printStackTrace();
        }

        int size = v.size();

        String[] sTestCases = new String[size];
        v.toArray(sTestCases);
        java.util.Arrays.sort(sTestCases);

        pVal = new PropertyValue[size];
        for (int i=0; i<size; i++) {
            pVal[i] = new PropertyValue();
            pVal[i].Name = "TestCase"+i;
            pVal[i].Value = sTestCases[i];
        }
        return pVal;
   }


   /**
   *
   * Gives a factory for creating the service.
   * This method is called by the <code>JavaLoader</code>
   * <p>
   * @return  returns a <code>XSingleServiceFactory</code> for creating the component
   * @param   implName     the name of the implementation for which a service is desired
   * @param   multiFactory the service manager to be used if needed
   * @param   regKey       the registryKey
   * @see                  com.sun.star.comp.loader.JavaLoader
   */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                    XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(RunnerService.class.getName()))
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                RunnerService.class, __serviceName, multiFactory, regKey);
        xMSF = multiFactory;
        return xSingleServiceFactory;
    }

  /**
   * Writes the service information into the given registry key.
   * This method is called by the <code>JavaLoader</code>
   * <p>
   * @return  returns true if the operation succeeded
   * @param   regKey       the registryKey
   * @see                  com.sun.star.comp.loader.JavaLoader
   */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(RunnerService.class.getName(),
        __serviceName, regKey);
    }

    /**
     * empty: not needed here.
     */
    public void setPropertyValues(PropertyValue[] propertyValue)
                        throws com.sun.star.beans.UnknownPropertyException,
                               com.sun.star.beans.PropertyVetoException,
                               com.sun.star.lang.IllegalArgumentException,
                               com.sun.star.lang.WrappedTargetException {
      // empty implementation
    }

}



















































