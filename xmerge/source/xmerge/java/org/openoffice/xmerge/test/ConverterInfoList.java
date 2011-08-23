/*************************************************************************
 *
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

package org.openoffice.xmerge.test;

import java.io.IOException;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.util.Properties;
import java.util.Vector;
import java.util.Enumeration;

/**
 *  Loads a properties file so that registry knows which plug-ins
 *  it needs to load.
 *
 *  @author: Martin Maher
 */
public class ConverterInfoList {

    private static String  defaultPropsFile = "ConverterInfoList.properties";

    private Vector     jars;
    private Properties props            = null;


    /**
     *  This constructor loads and reads the default properties file.
     *  The default property file name is:
     *  &quot;ConverterInfoList.properties&quot;.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public ConverterInfoList() throws IOException {
        this(defaultPropsFile);
    }

   /**
    *  This constructor loads and reads the properties file.
    *
    *  @param  propsFile  The properties file to load.
    *
    *  @throws  IOException  If any I/O error occurs.
    */
    public ConverterInfoList(String propsFile) throws IOException {

        Class c                 = this.getClass();
        InputStream is          = c.getResourceAsStream(propsFile);
        BufferedInputStream bis = new BufferedInputStream(is);
        props                   = new Properties();
        props.load(bis);
        bis.close();

        int i              = 1;
        String jarFileName = new String();
        jars               = new Vector();

        while ((jarFileName = props.getProperty("jarname" + i)) != null) {
            jars.add(jarFileName);
            i++;
        }
    }


   /**
    *  Returns a <code>Vector</code> containing a list of 
    *  <code>String</code> objects.  Each <code>String</code>
    *  describes a plug-in to be loaded into the registry.
    *
    *
    *  @return  A <code>Vector</code> containing a list of
    *           <code>String</code> objects.  Each
    *           <code>String</code> describes a plug-in to be
    *           loaded into the registry.
    */
    public Enumeration getJarFileEnum() {

        return jars.elements();
    }
}

