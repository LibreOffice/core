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

