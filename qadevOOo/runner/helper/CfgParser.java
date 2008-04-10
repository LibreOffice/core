/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CfgParser.java,v $
 * $Revision: 1.8 $
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

package helper;

import lib.TestParameters;
import java.util.Properties;
import java.util.Enumeration;
import java.io.FileInputStream;
import util.PropertyName;

/**
 * This class parses the ini files and stores the data
 * <br>
 * inside TestParameters
 */
public class CfgParser {
    protected boolean debug = false;
    protected String iniFile="";

    public CfgParser(String ini) {
        if (ini != null)
            this.iniFile = ini;
    }

    public void getIniParameters(TestParameters param) {
        debug = param.DebugIsActive;
        Properties cfg = null;
        if (iniFile.equals("")) {
            //no iniFile given, search one in the users home directory
            cfg = getProperties(getDefaultFileName(true));
            //try to search the user dir if no iniFile could be found yet
            if (cfg == null) {
                cfg = getProperties(getDefaultFileName(false));
            }
        } else {
            cfg = getProperties(iniFile);
        }

        if (cfg != null) {
            Enumeration cfgEnum = cfg.keys();
            while (cfgEnum.hasMoreElements()) {
                String pName = (String) cfgEnum.nextElement();
                Object pValue = cfg.getProperty(pName);

                if (pValue instanceof String) pValue = ((String)pValue).trim();

                param.put(pName.trim(),pValue);

                if (pName.equals(PropertyName.TEST_DOCUMENT_PATH)) {

                    param.put("DOCPTH",(String)pValue);
                    System.setProperty("DOCPTH",(String)pValue);

                }
            }
        }

        debug = param.DebugIsActive;

        //check for platform dependend parameters
        //this would have a $OperatingSystem as prefix
        String os = (String) param.get(PropertyName.OPERATING_SYSTEM);
        if (os != null && os.length()>1) {

            //found something that could be a prefex
            //check all parameters for this
            Enumeration keys = param.keys();
            while (keys.hasMoreElements()) {
                String key = (String) keys.nextElement();
                if (key.startsWith(os)) {
                    Object oldValue = param.get(key);
                    String newKey = key.substring(os.length()+1);
                    param.remove(key);
                    param.put(newKey,oldValue);
                }
            }

        }
    }

    protected Properties getProperties(String name) {
        // get the resource file
        Properties prop = new Properties();
        if ( debug ) {
            System.out.println("Looking for "+name);
        }
        try {
            FileInputStream propFile = new FileInputStream(name);
            prop.load(propFile);
            System.out.println("Parsing properties from "+name);
            propFile.close();
        } catch (Exception e) {
            try {
                java.net.URL url = this.getClass().getResource("/"+name);
                if (url != null) {
                    System.out.println("Parsing properties from "+name);
                    java.net.URLConnection connection = url.openConnection();
                    java.io.InputStream in = connection.getInputStream();
                    prop.load(in);
                }
            } catch (Exception ex) {
                //Exception while reading prop-file, returning null
                return null;
            }
        }

        return prop;
    }

    protected String getDefaultFileName(boolean home) {
        String fileSeparator = System.getProperty("file.separator");
        String path = "";
        if (home) {
            //look inside the home directory
            path = System.getProperty("user.home");
        } else {
            path = System.getProperty("user.dir");
        }
        if (fileSeparator.equals("/")) {
            //suppose I'm on Unix-platform
            return path+fileSeparator+".runner.props";
        } else {
            //suppose I'm on Windows
            return path+fileSeparator+"runner.props";
        }
    }

}
