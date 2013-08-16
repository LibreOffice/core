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

package lib;

import java.util.HashMap;
import util.PropertyName;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;

//import com.sun.star.lang.XMultiServiceFactory;

/**
 * TestParameters describes a parameters (in a form of pairs: name, value) to
 * be passed to tests and which may affect the test behaviour. That can be,
 * for example, standard paths, connection strings, etc. The TestParameters
 * also provides XMultiServiceFactory for the test (tests).
 */
public class TestParameters extends HashMap<String,Object> {

    /**
     * The ConnectionString for Office Connection<br>
     * default is 'socket,host=localhost,port=8100'
     */

    public String ConnectionString="socket,host=localhost,port=8100";

    /**
     * The AppProvider contains the Application Provider<br>
     * to control the ServiceFactory.
     */

    public Object AppProvider=null;

    /**
     * The Process contains the Process handler<br>
     * to control the Application.
     */

    public Object ProcessHandler=null;

    /**
     * The AppExecutionCmd contains the full qualified<br>
     * command to an Application to be started.
     */

    public String AppExecutionCommand="";

    /**
     * If this parameter is <CODE>true</CODE> the <CODE>OfficeProvider</CODE> tries
     * to get the URL to the binary of the office and to fill the
     * <CODE>AppExecutionCommand</CODE> with useful content if needet
     */
    public boolean AutoRestart = false;

    /**
     * Shoert wait time for the Office: default is 500 milliseconds
     */
    public int ShortWait = 500;


    /**
     * The OfficeProvider contains the full qualified
     * class that provides a connection to StarOffice<br>
     * default is helper.OfficeProvider
     */

    public String OfficeProvider = "helper.OfficeProvider";

    /**
     * The Testbase to be executed by the runner<br>
     * default is 'java_fat'
     */

    public String TestBase="java_fat";

    /**
     * The ServiceFactory to create instances
     */

    public Object ServiceFactory;

    /**
     * The Path to the component description
     */

    public String DescriptionPath;

    /**
     * The Path to the test documents that are loaded during the test <br>
     */

    public String TestDocumentPath="unknown";

    /**
     * 'true' is a log should be written, 'false' elsewhere <br>
     * these will be provided by the testcases<br>
     * default is true
     */

    public boolean LoggingIsActive=true;

    /**
     * 'true' is a debug information should be written, 'false' elsewhere
     * these will be provided by the framework.<br>
     * Debug information will always be written on standard out.<br>
     * default is true
     */

    public boolean DebugIsActive=false;

    /*
     * This parameter contains the testjob to be executed<br>
     * by the framework
     */

    public Object TestJob;

    /*
     * This parameter contains the class used<br>
     * for Logging
     */

    public String LogWriter="stats.SimpleLogWriter";

    /*
     * This parameter contains the class used<br>
     * for Logging
     */

    public String OutProducer="stats.SimpleOutProducer";

    /*
     * This parameter contains the timeout used<br>
     * by the watcher
     */
    public Integer TimeOut = new Integer(3000000);

    /*
     * This parameter contains the timeout used<br>
     * by the complex tests
     */
    public Integer ThreadTimeOut = new Integer(3000000);

    /*
     * This parameter contains the time which the office could use to close for
     * itself before its destroyed. Default is 15000 ms
     */
    public Integer OfficeCloseTimeOut = new Integer(15000);

    /**
     * Wraper around "get()" with some debug output
     * @param key A key of this table.
     * @return The value of this key.
     * @see java.util.HashMap
     */
    public Object get(Object key) {
        Object val = super.get(key);
        if (val == null && DebugIsActive) {
            System.out.print("Have been asked for key \""+key.toString());
            System.out.println("\" which is not part of params.");
        }
        return val;
    }

    /**
     * Special get method for boolean values: for convenience.
     * Will return 'false' if the value is not of "Boolean" type.
     * @param key A key of this table.
     * @return The value of this key, castet to a boolean type.
     */
    public boolean getBool(Object key) {
        Object val = super.get(key);
        if (val != null) {
            if (val instanceof String) {
                String sVal = (String)val;
                if (sVal.equalsIgnoreCase("true") ||
                                                sVal.equalsIgnoreCase("yes")) {
                    return true;
                }
                else if (sVal.equalsIgnoreCase("false") ||
                                                sVal.equalsIgnoreCase("no")) {
                    return false;
                }
            }
            if (val instanceof Boolean)
                return ((Boolean)val).booleanValue();
        }
        return false;
    }

    /**
     * Special get method for integer values: for convenience.
     * Will return 0 if the value cannot be interpreted as Integer.
     * @param key A key of this table.
     * @return The value of this key, castet to an int type.
     */
    public int getInt(Object key) {
        Object val = super.get(key);
        if ( val != null ) {
            if (val instanceof Integer) {
                return ((Integer)val).intValue();
            }
            else {
                try {
                    if ( val instanceof String ) {
                        Integer nr = new Integer((String)val);
                        if (nr.intValue() > 0) return nr.intValue();
                    }
                } catch ( java.lang.NumberFormatException nfe) {}
            }
        }
        return 0;
    }


    /**
     * Wrapper around "put()"
     * @param key A key of this table.
     * @param val The value of the key.
     * @return The value of this key.
     * @see java.util.HashMap
     */
    public Object put(String key, Object val) {
        return super.put(key,val);
    }

    /**
     * Constructor, defaults for Parameters are set.
     */
    public TestParameters() {
        //fill the propertyset
        String user = System.getProperty("user.name");
        if ( user != null)
        {
            String PipeConnectionString = "pipe,name=" + user;
            put(PropertyName.PIPE_CONNECTION_STRING,PipeConnectionString);
            put(PropertyName.USE_PIPE_CONNECTION, Boolean.TRUE);
        }
        put(PropertyName.CONNECTION_STRING,ConnectionString);
        put(PropertyName.TEST_BASE,TestBase);
        put(PropertyName.TEST_DOCUMENT_PATH,TestDocumentPath);
        put(PropertyName.LOGGING_IS_ACTIVE,LoggingIsActive?Boolean.TRUE:Boolean.FALSE);
        put(PropertyName.DEBUG_IS_ACTIVE,DebugIsActive?Boolean.TRUE:Boolean.FALSE);
        put(PropertyName.OUT_PRODUCER,OutProducer);
        put(PropertyName.SHORT_WAIT,new Integer(ShortWait));
        put(PropertyName.OFFICE_PROVIDER,OfficeProvider);
        put(PropertyName.LOG_WRITER,LogWriter);
        put(PropertyName.APP_EXECUTION_COMMAND,AppExecutionCommand);
        put(PropertyName.TIME_OUT,TimeOut);
        put(PropertyName.THREAD_TIME_OUT,ThreadTimeOut);
        put(PropertyName.AUTO_RESTART,AutoRestart?Boolean.TRUE:Boolean.FALSE);
        put(PropertyName.OFFICE_CLOSE_TIME_OUT, OfficeCloseTimeOut);

        // get the operating system
        put(PropertyName.OPERATING_SYSTEM, getSOCompatibleOSName());

        //For compatibility Reasons
        put("CNCSTR",ConnectionString);
        put("DOCPTH",TestDocumentPath);
        System.setProperty("DOCPTH",TestDocumentPath);
    }

    /**
     * @return a XMultiServiceFactory to be used by a test (tests).
     */
    public Object getMSF() {
        Object ret = null;
        ret = get("ServiceFactory");
        return ret;
    }

    public XComponentContext getComponentContext() {
        Object context = get( "ComponentContext" );
        if ( context == null )
        {
            XPropertySet factoryProps = com.sun.star.uno.UnoRuntime.queryInterface(
                XPropertySet.class, getMSF() );
            try
            {
                context = com.sun.star.uno.UnoRuntime.queryInterface(
                    XComponentContext.class, factoryProps.getPropertyValue( "DefaultContext" ) );
                put( "ComponentContext", context );
            }
            catch( com.sun.star.beans.UnknownPropertyException e ) { }
            catch( com.sun.star.lang.WrappedTargetException e ) { }
        }
        return (XComponentContext)context;
    }

    /**
     * Convert the system dependent operating system name to a name according
     * to OOo rules.
     * @return A valid OS name, or "" if the name is not known.
     */
    String getSOCompatibleOSName() {
        String osname = System.getProperty ("os.name").toLowerCase ();
        String osarch = System.getProperty ("os.arch");
        String operatingSystem = "";
        if (osname.indexOf ("windows")>-1) {
            operatingSystem = PropertyName.WNTMSCI;
        } else if (osname.indexOf ("linux")>-1 || osname.indexOf ("kfreebsd")>-1) {
            operatingSystem = PropertyName.UNXLNGI;
        } else if (osname.indexOf ("sunos")>-1) {
            if (osarch.equals ("x86")) {
                operatingSystem = PropertyName.UNXSOLI;
            } else {
                operatingSystem = PropertyName.UNXSOLS;
            }
        } else if (osname.indexOf ("mac")>-1) {
            operatingSystem = PropertyName.UNXMACXI;
        } else {
            System.out.println("ERROR: not supported platform: " + osname);
            System.exit(1);
        }
        return operatingSystem;
    }

}// finish class TestParamenters
