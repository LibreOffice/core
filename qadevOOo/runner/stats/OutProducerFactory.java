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
package stats;

import share.LogWriter;
import java.util.HashMap;
import util.DynamicClassLoader;

/**
 * A factory class for creating out producers.
 */
public class OutProducerFactory {

    /**
     * Create an out producer. The type that is created depends on the
     * parameters given. These are:
     * <ul>
     *   <li>DataBaseOut - If set to true, a database outproducer is created.
     *   <li>OutProducer - The value of this parameter names the class that is created.
     * </ul>
     * @param param Parameters of the test.
     * @return The created out producer.
     */
    public static LogWriter createOutProducer(HashMap<String,Object> param) {
        LogWriter dbOut = null;
        boolean getDatabase = convertToBool(param.get("DataBaseOut"));
        if (getDatabase) {
            dbOut = createDataBaseOutProducer(param);
        }
        if (dbOut == null) {
            DynamicClassLoader dcl = new DynamicClassLoader();
            String outProducerName = (String)param.get("OutProducer");
            if (outProducerName != null) {
                try {
                    dbOut = (LogWriter)dcl.getInstance(outProducerName);
                }
                catch(IllegalArgumentException e) {
                    e.printStackTrace();
                }
            }
        }
        if (dbOut == null) {
            dbOut = createSimpleOutProducer();
        }
        return dbOut;
    }

    /**
     * Create a database out producer.
     * @param param The test parameters
     * @return The database out producer, or null if it couldn't be created.
     */
    public static LogWriter createDataBaseOutProducer(HashMap<String,Object> param) {
        String dataProducerName = (String)param.get("DataBaseOutProducer");
        if (dataProducerName == null) {
            String testBaseName = (String)param.get("TestBase");
            dataProducerName = testBaseName.substring(testBaseName.indexOf("_")+1);
            dataProducerName = "stats." + makeFirstCharUpperCase(dataProducerName)
                            + "DataBaseOutProducer";
        }
        DynamicClassLoader dcl = new DynamicClassLoader();
        LogWriter dbOut = null;
        try {
            dbOut = (LogWriter)dcl.getInstance(dataProducerName,
                new Class[]{HashMap.class}, new Object[]{param});
        }
        catch(IllegalArgumentException e) {
            e.printStackTrace();
        }
        return dbOut;
    }

    /**
     * As a fallback, create a simple out producer, if all else failed.
     * @return A simple out producer, writing to the screen.
     */
    public static LogWriter createSimpleOutProducer() {
        return new SimpleOutProducer();
    }

    private static boolean convertToBool(Object val) {
        if(val != null) {
            if ( val instanceof String ) {
                String sVal = (String)val;
                if ( sVal.equalsIgnoreCase("true") || sVal.equalsIgnoreCase("yes") ) {
                    return true;
                }

            }
            else if (val instanceof Boolean) {
                return ((Boolean)val).booleanValue();
            }
        }
        return false;
    }

    /**
     * Make the first character to an upper case char.
     * @param name The String to change
     * @return The String with an upper case first char.
     */
    private static String makeFirstCharUpperCase(String name) {
        return name.substring(0,1).toUpperCase() + name.substring(1);
    }

/*    public static void main(String[] args) {
        Hashtable p = new Hashtable();
        p.put("DataBaseOut", "yes");
        p.put("TestBase", "java_complex");
        p.put("Version", "srx645gggg");
        createDataBaseOutProducer(p);
    } */
}
