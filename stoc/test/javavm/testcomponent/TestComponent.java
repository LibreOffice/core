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

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.*;
import com.sun.star.uno.*;
import com.sun.star.registry.XRegistryKey;
import java.io.*;
import java.net.*;
//import com.sun.star.lib.sandbox.*;


/** This component implements XTypeProvider for use with StarBasic.
 *  The XServiceInfo is implemented to have an interface in which we can put some
 *  code just for the sake of debugging.
 *
 *  To debug with JPDA (jdk 1.3), put these lines in the java.ini within the  [Java] section:
 *  -Xdebug
 *  -Xrunjdwp:transport=dt_socket,address=8000,server=y,suspend=y
 *
 *  When the Virtual Machine service is instantiated it will block until the debugger
 *  attaches to it on port 8000. You can chose a different port. You attach to the VM using
 *  jdb by
 *
 *  jdb -connect com.sun.jdi.SocketAttach:hostname=myhost,port=8000
 *
 *  myhost is the hostname where the VM is running.
*/
public class TestComponent implements XServiceInfo, XTypeProvider
{
    public static final String __serviceName="JavaTestComponent";

    // XTypeProvider
    public com.sun.star.uno.Type[] getTypes(  )
    {
        Type[] retValue= new Type[2];
        retValue[0]= new Type( XServiceInfo.class);
        retValue[1]= new Type( XTypeProvider.class);
        return retValue;
    }
    // XTypeProvider
    public byte[] getImplementationId(  )
    {
        return new byte[0];
    }


    // XServiceName
    public String getImplementationName(  )
    {
        String a= "the functions are for debugging";
        int abc= 34;
        String prop= System.getProperty("ftp.proxyHost");
        prop= System.getProperty("ftp.proxyPort");
        prop= System.getProperty("http.proxyHost");
        prop= System.getProperty("http.proxyPort");
        prop= System.getProperty("ftp.nonProxyHosts");
        prop= System.getProperty("http.nonProxyHosts");
        prop= System.getProperty("socksProxyHost");
        prop= System.getProperty("socksProxyPort");

        prop= System.getProperty("stardiv.security.disableSecurity");
        prop= System.getProperty("appletviewer.security.mode");

        // Test security settings
        File f= new File("c:/temp/javasecurity.txt");
        try {
            f.createNewFile();

               // local connection
        URL url= new URL("http://localhost:8080/index.html");
        InputStream is= url.openStream();
        // remote connection
        url= new URL("http://www.w3.org/index.html");
        is= url.openStream();
        }catch( MalformedURLException mue) {
        }catch( IOException e) {
            String s= e.getMessage();
            System.out.println(s);
        }/*catch( SandboxSecurityException sse) {
            String s= sse.getMessage();
            System.out.println("s");
        }
*/

        return __serviceName;
    }
    // XServiceName
    public boolean supportsService( /*IN*/String ServiceName )
    {

        return false;
    }

    //XServiceName
    public String[] getSupportedServiceNames(  )
    {
        String[] retValue= new String[0];
        return retValue;
    }

    public static XSingleServiceFactory __getServiceFactory(String implName,
    XMultiServiceFactory multiFactory,
    XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals( TestComponent.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory( TestComponent.class,
            TestComponent.__serviceName,
            multiFactory,
            regKey);

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
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)
    {
        return FactoryHelper.writeRegistryServiceInfo( TestComponent.class.getName(),
        TestComponent.__serviceName, regKey);
    }

}
