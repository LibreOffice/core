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


package com.example;

import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;
import java.io.IOException;
import java.lang.reflect.Method;
import java.net.JarURLConnection;
import java.net.URL;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

/**
 *
 * @author Marc Neumann
 */
public class CentralRegistrationClass
{

    public static XSingleComponentFactory __getComponentFactory(String sImplementationName)
    {
        String regClassesList = getRegistrationClasses();
        StringTokenizer t = new StringTokenizer(regClassesList, " ");
        while (t.hasMoreTokens())
        {
            String className = t.nextToken();
            if (className != null && className.length() != 0)
            {
                try
                {
                    Class regClass = Class.forName(className);
                    Method writeRegInfo = regClass.getDeclaredMethod("__getComponentFactory", new Class[]
                            {
                                String.class
                            });
                    Object result = writeRegInfo.invoke(regClass, sImplementationName);
                    if (result != null)
                    {
                        return (XSingleComponentFactory) result;
                    }
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        return null;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey xRegistryKey)
    {
        boolean bResult = true;
        String regClassesList = getRegistrationClasses();
        StringTokenizer t = new StringTokenizer(regClassesList, " ");
        while (t.hasMoreTokens())
        {
            String className = t.nextToken();
            if (className != null && className.length() != 0)
            {
                try
                {
                    Class regClass = Class.forName(className);
                    Method writeRegInfo = regClass.getDeclaredMethod("__writeRegistryServiceInfo", new Class[]
                            {
                                XRegistryKey.class
                            });
                    Object result = writeRegInfo.invoke(regClass, xRegistryKey);
                    bResult &= ((Boolean) result).booleanValue();
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        return bResult;
    }

    private static String getRegistrationClasses()
    {
        CentralRegistrationClass c = new CentralRegistrationClass();
        String name = c.getClass().getCanonicalName().replace('.', '/').concat(".class");
        try
        {
            Enumeration<URL> urlEnum = c.getClass().getClassLoader().getResources("META-INF/MANIFEST.MF");
            while (urlEnum.hasMoreElements())
            {
                URL url = urlEnum.nextElement();
                String file = url.getFile();
                JarURLConnection jarConnection = (JarURLConnection) url.openConnection();
                Manifest mf = jarConnection.getManifest();

                Attributes attrs = (Attributes) mf.getAttributes(name);
                if (attrs != null)
                {
                    String classes = attrs.getValue("RegistrationClasses");
                    return classes;
                }
            }
        }
        catch (IOException ex)
        {
            ex.printStackTrace();
        }

        return "";
    }

    /** Creates a new instance of CentralRegistrationClass */
    private CentralRegistrationClass()
    {
    }
}
