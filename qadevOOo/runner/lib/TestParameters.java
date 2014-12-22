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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XComponentContext;

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
    private static final String DefaultConnectionString = "socket,host=localhost,port=8100";

    /**
     * The Path to the test documents that are loaded during the test <br>
     */
    private static final String DefaultTestDocumentPath = "unknown";

    /**
     * 'true' is a debug information should be written, 'false' elsewhere
     * these will be provided by the framework.<br>
     * Debug information will always be written on standard out.<br>
     * default is true
     */
    private static final boolean DebugIsActive = false;

    /**
     * Wrapper around "get()" with some debug output
     * @param key A key of this table.
     * @return The value of this key.
     * @see java.util.HashMap
     */
    @Override
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
     * @return The value of this key, cast to a boolean type.
     */
    public boolean getBool(Object key) {
        final Object val = super.get(key);
        if (val == null) {
            return false;
        }
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
        else if (val instanceof Boolean)
            return ((Boolean)val).booleanValue();
        return false;
    }

    /**
     * Special get method for integer values: for convenience.
     * Will return 0 if the value cannot be interpreted as Integer.
     * @param key A key of this table.
     * @return The value of this key, cast to an int type.
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
                        Integer nr = Integer.valueOf((String)val);
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
    @Override
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
        put(PropertyName.CONNECTION_STRING, DefaultConnectionString);
        put(PropertyName.TEST_BASE, "java_fat");
        put(PropertyName.TEST_DOCUMENT_PATH, DefaultTestDocumentPath);
        put(PropertyName.LOGGING_IS_ACTIVE, Boolean.TRUE);
        put(PropertyName.DEBUG_IS_ACTIVE, Boolean.valueOf(DebugIsActive));
        put(PropertyName.OUT_PRODUCER, "stats.SimpleOutProducer");
        put(PropertyName.OFFICE_PROVIDER, "helper.OfficeProvider");
        put(PropertyName.LOG_WRITER, "stats.SimpleLogWriter");
        put(PropertyName.APP_EXECUTION_COMMAND, "");
        put(PropertyName.TIME_OUT, Integer.valueOf(3000000));
        put(PropertyName.THREAD_TIME_OUT, Integer.valueOf(3000000));
        put(PropertyName.AUTO_RESTART, Boolean.FALSE);
        put(PropertyName.OFFICE_CLOSE_TIME_OUT, Integer.valueOf(15000));

        // get the operating system
        put(PropertyName.OPERATING_SYSTEM, getSOCompatibleOSName());

        //For compatibility Reasons
        System.setProperty("DOCPTH", DefaultTestDocumentPath);
    }

    /**
     * @return a XMultiServiceFactory to be used by a test (tests).
     */
    public XMultiServiceFactory getMSF() {
        Object ret = null;
        ret = get("ServiceFactory");
        return (XMultiServiceFactory) ret;
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
    private String getSOCompatibleOSName() {
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
