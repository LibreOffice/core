/*************************************************************************
 *
 *  $RCSfile: java_complex.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-08-07 15:04:51 $
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

package base;

import java.lang.reflect.Constructor;
import java.io.InputStream;
import java.io.File;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Enumeration;
import complexlib.ComplexTestCase;
import lib.DynamicClassLoader;
import share.DescGetter;
import helper.ComplexDescGetter;
import helper.AppProvider;
import helper.CfgParser;
import share.DescEntry;
import share.LogWriter;
import stats.Summarizer;
import base.TestBase;
import lib.Status;
import lib.TestParameters;

/**
 * Test base for executing a java complex test.
 * @see base.TestBase
 */
public class java_complex implements TestBase{

    /**
     * Execute a test.
     * @param param The test parameters.
     * @return True, if the test was executed.
     */
    public boolean executeTest(TestParameters param) {
        // is there an ini file for the complex tests defined?
        String complexIniFileName = ((String)param.get("ComplexIni"));
        CfgParser ini = new CfgParser(complexIniFileName);
        ini.getIniParameters(param);

        // get the test job
        String testJob = ((String)param.get("TestJob"));

        DescGetter descGetter = new ComplexDescGetter();
        // get the test jobs
        DescEntry[] entries = descGetter.getDescriptionFor(testJob,null,true);
        if (entries == null) return false;

        DynamicClassLoader dcl = new DynamicClassLoader();
        ComplexTestCase testClass = null;
        boolean returnVal = true;

        param.put("TimeOut", new Integer(0));

        for (int i=0; i<entries.length; i++) {

            String iniName = entries[i].longName;
            iniName = iniName.replace('.', '/');
            getParamsForComplexTest(iniName+".props", param);

            LogWriter log = (LogWriter)dcl.getInstance(
                                                (String)param.get("LogWriter"));

            AppProvider office = null;
            if (!param.getBool("NoOffice")) {
                try {
                    office = (AppProvider)dcl.getInstance("helper.OfficeProvider");
                    Object msf = office.getManager(param);
                    if (msf == null) {
                        returnVal = false;
                        continue;
                    }
                    param.put("ServiceFactory",msf);
                }
                catch(IllegalArgumentException e) {
                    office = null;
                }
            }
            log.initialize(entries[i],true);
            entries[i].Logger = log;

            // create an instance
            try {
                testClass = (ComplexTestCase)dcl.getInstance(entries[i].longName);
            }
            catch(java.lang.Exception e) {
                e.printStackTrace();
                return false;
            }
            testClass.executeMethods(entries[i], param);

            Summarizer sum = new Summarizer();
            sum.summarizeUp(entries[i]);

            if (office != null) {
                office.closeExistingOffice(param, false);
            }

            LogWriter out = (LogWriter)dcl.getInstance(
                                            (String)param.get("OutProducer"));

            out.summary(entries[i]);
            returnVal &= entries[i].State.endsWith("OK");
        }
        return returnVal;
    }


    private void getParamsForComplexTest(String fileName, TestParameters param) {
        // get the resource file
        try {
            if ( param.DebugIsActive ) {
                System.out.println("Looking for "+fileName);
            }
            java.net.URL url = this.getClass().getResource("/"+fileName);
            if (url != null) {
                System.out.println("Parsing properties from "+fileName);
                java.net.URLConnection connection = url.openConnection();
                java.io.InputStream in = connection.getInputStream();
                Properties props = new Properties();
                props.load(in);
                Enumeration enum = props.keys();
                while (enum.hasMoreElements()) {
                    String key = (String)enum.nextElement();
                    String value = (String)props.get(key);
                    param.put(key.trim(), value.trim());
                }
            }
        }
        catch(java.io.IOException e) {
            System.out.println("Exception while reading property file '"+fileName+"'");
            e.printStackTrace();
        }
        catch(java.lang.NullPointerException e) {}
    }

}
