/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutProducerFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:29:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package stats;

import share.LogWriter;
import java.util.Hashtable;
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
     * @param Parameters of the test.
     * @return The created out producer.
     */
    public static LogWriter createOutProducer(Hashtable param) {
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
     * Create a databbase out producer.
     * @param The test parameters
     * @return The database out producer, or null if it couldn't be created.
     */
    public static LogWriter createDataBaseOutProducer(Hashtable param) {
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
                new Class[]{new Hashtable().getClass()}, new Object[]{param});
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
