/*************************************************************************
 *
 *  $RCSfile: TestParameters.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:16:03 $
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

package lib;

import java.util.Hashtable;
//import com.sun.star.lang.XMultiServiceFactory;

/**
 * TestParameters describes a parameters (in a form of pairs: name, value) to
 * be passed to tests and which may affect the test behaviour. That can be,
 * for example, standard paths, connection strings, etc. The TestParameters
 * also provides XMultiServiceFactory for the test (tests).
 */
public class TestParameters extends Hashtable {

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
     * default will be the tmp dir
     */

    public String TestDocumentPath=System.getProperty("java.io.tmpdir");

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
    public Integer TimeOut = new Integer(30000);

    /**
     * Wraper around "get()" with some debug output
     * @param key A key of this table.
     * @return The value of this key.
     * @see java.util.Hashtable
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
     * Wraper around "put()"
     * @param key A key of this table.
     * @param val The value of the key.
     * @return The value of this key.
     * @see java.util.Hashtable
     */
    public Object put(Object key, Object val) {
        return super.put(key,val);
    }

    /**
     * Constructor, defaults for Parameters are set.
     */

    public TestParameters() {
        //fill the propertyset
        put("ConnectionString",ConnectionString);
        put("TestBase",TestBase);
        put("TestDocumentPath",TestDocumentPath);
        put("LoggingIsActive",Boolean.valueOf(LoggingIsActive));
        put("DebugIsActive",Boolean.valueOf(DebugIsActive));
        put("OutProducer",OutProducer);
        put("OfficeProvider",OfficeProvider);
        put("LogWriter",LogWriter);
        put("AppExecutionCommand",AppExecutionCommand);
        put("TimeOut",TimeOut);

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

}// finish class TestParamenters
