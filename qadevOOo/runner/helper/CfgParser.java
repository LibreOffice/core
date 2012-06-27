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
package helper;

import java.io.FileInputStream;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Properties;

import lib.TestParameters;
import util.PropertyName;

/**
 * This class parses the ini files and stores the data
 * <br>
 * inside TestParameters
 */
public class CfgParser
{

    protected boolean debug = false;
    protected String iniFile = "";

    public CfgParser(String ini)
    {
        if (ini != null)
        {
            this.iniFile = ini;
        }
    }

    public void getIniParameters(TestParameters param)
    {
        debug = param.DebugIsActive;
        Properties cfg = null;
        if (iniFile.equals(""))
        {
            //no iniFile given, search one in the users home directory
            cfg = getProperties(getDefaultFileName(true));
            //try to search the user dir if no iniFile could be found yet
            if (cfg == null)
            {
                cfg = getProperties(getDefaultFileName(false));
            }
        }
        else
        {
            cfg = getProperties(iniFile);
        }

        if (cfg != null)
        {
            Enumeration<Object> cfgEnum = cfg.keys();
            while (cfgEnum.hasMoreElements())
            {
                String pName = (String) cfgEnum.nextElement();
                Object pValue = cfg.getProperty(pName);

                if (pValue instanceof String)
                {
                    pValue = ((String) pValue).trim();
                }

                param.put(pName.trim(), pValue);

                if (pName.equals(PropertyName.TEST_DOCUMENT_PATH))
                {

                    param.put("DOCPTH", pValue);
                    System.setProperty("DOCPTH", (String) pValue);

                }
                else if (pName.equals(PropertyName.SRC_ROOT))
                {

                    System.setProperty(pName, (String) pValue);

                }
            }
        }

        debug = param.DebugIsActive;

        //check for platform dependent parameters
        //this would have a $OperatingSystem as prefix
        String os = (String) param.get(PropertyName.OPERATING_SYSTEM);
        if (os != null && os.length() > 1)
        {

            //found something that could be a prefix
            //check all parameters for this
            Iterator<String> keys = param.keySet().iterator();
            while (keys.hasNext())
            {
                String key = keys.next();
                if (key.startsWith(os))
                {
                    Object oldValue = param.get(key);
                    String newKey = key.substring(os.length() + 1);
                    param.remove(key);
                    param.put(newKey, oldValue);
                }
            }

        }
    }

    protected Properties getProperties(String name)
    {
        // get the resource file
        Properties prop = new Properties();
        if (debug)
        {
            System.out.println("Looking for " + name);
        }
        try
        {
            FileInputStream propFile = new FileInputStream(name);
            prop.load(propFile);
            System.out.println("Parsing properties from " + name);
            propFile.close();
        }
        catch (Exception e)
        {
            try
            {
                java.net.URL url = this.getClass().getResource("/" + name);
                if (url != null)
                {
                    System.out.println("Parsing properties from " + name);
                    java.net.URLConnection connection = url.openConnection();
                    java.io.InputStream in = connection.getInputStream();
                    prop.load(in);
                }
            }
            catch (Exception ex)
            {
                //Exception while reading prop-file, returning null
                return null;
            }
        }

        return prop;
    }

    protected String getDefaultFileName(boolean home)
    {
        String fileSeparator = System.getProperty("file.separator");
        String path = "";
        if (home)
        {
            //look inside the home directory
            path = System.getProperty("user.home");
        }
        else
        {
            path = System.getProperty("user.dir");
        }
        if (fileSeparator.equals("/"))
        {
            //suppose I'm on Unix-platform
            return path + fileSeparator + ".runner.props";
        }
        else
        {
            //suppose I'm on Windows
            return path + fileSeparator + "runner.props";
        }
    }
}
