/************************************************************************
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

package org.openoffice.xmerge.test;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.FileReader;
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

