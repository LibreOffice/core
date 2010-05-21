/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
