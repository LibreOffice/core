/*************************************************************************
 *
 *  $RCSfile: BasicIfcTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-05-27 12:01:18 $
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
package basicrunner;

import lib.TestResult;
import lib.TestEnvironment;
import lib.TestParameters;
import lib.Status;
import share.DescEntry;
import share.LogWriter;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.NoSuchElementException;

import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyValue;


import java.util.StringTokenizer;
import java.util.Vector;

/**
 * The BASIC interface test
 */
public class BasicIfcTest {
    /** The BasicHandler **/
    static BasicHandler oBasicHandler = null;
    /** The result orf the test **/
    protected TestResult tRes;
    /** the name of the test **/
    protected String testName;

    /** Constructor with test name.
     * @param name The name of the test.
     */
    public BasicIfcTest(String name) {
        testName = name;
    }

    /**
     * Let the test run.
     * @param xTestedEntry Informaton about the interface to test.
     * @param tEnv The environment of the test.
     * @param tParam The test parameters.
     * @return A result of the test.
     */
    public TestResult run(DescEntry xTestedEntry, TestEnvironment tEnv,
            TestParameters tParam) {

        String sResult = "";

        this.tRes = new TestResult();
        LogWriter log = xTestedEntry.Logger;

        // Get Handler, that was created during object creation.
        oBasicHandler = (BasicHandler)tEnv.getObjRelation("BasicHandler");

        if (!oBasicHandler.isUptodate((XMultiServiceFactory)tParam.getMSF())) {
            // If Handler uses old MSF (in case of Office's GPF) then don't test
            // interface.
            return null;
        }

        boolean objectWasCreated = ((Boolean)tEnv.getObjRelation("objectCreated")).booleanValue();

        if (objectWasCreated) {
            oBasicHandler.setTestedInterface(this, log);

            DescEntry methods[] = xTestedEntry.SubEntries;

            String  names[] = new String[methods.length + 1];
            boolean isOpt[] = new boolean[methods.length + 1];
            String other[] = new String[1];

            String aName = xTestedEntry.longName;
            aName = aName.substring(aName.indexOf("::")+2);
            int oldIndex = 0;
            int index = aName.indexOf("::");
            names[0] = "";
            while(index!=-1) {
                names[0] += aName.substring(oldIndex,index) + ".";
                oldIndex=index+2;
                index=aName.indexOf("::", oldIndex);
            }
            names[0] += aName.substring(oldIndex);
            isOpt[0] = xTestedEntry.isOptional;

            for (int i = 1; i < names.length; i++) {
                names[i] = methods[i - 1].entryName;
                isOpt[i] = methods[i - 1].isOptional;
            }

            // for reasons of compatibility with JSuite we change the first
            // character of EntryType to upper case.
            String eType = xTestedEntry.EntryType;
            other[0] = eType.toUpperCase().charAt(0)+eType.substring(1);

            Object params[] = {names, isOpt, other};

            try {
                PropertyValue Res = oBasicHandler.perform("testInterface", params);
                sResult = (String)Res.Value;
            } catch (BasicException e) {
                log.println(e.info);
                sResult = "SKIPPED.FAILED";
            }
        } else { // if object was not created...
            sResult = "SKIPPED.FAILED";
        }

        // now tRes has all substates: collect them
        DescEntry[] subs = xTestedEntry.SubEntries;
        for (int i = 0; i < subs.length ; i++) {
            if (sResult.equals("SKIPPED.FAILED"))
                subs[i].State = "SKIPPED.FAILED";
            else if (sResult.equals("SKIPPED.OK"))
                subs[i].State = "SKIPPED.OK";
            else
                if (tRes.getStatusFor(subs[i].entryName) == null) {
                    subs[i].State = "SKIPPED.FAILED";
                } else {
                    subs[i].State = tRes.getStatusFor(
                                                subs[i].entryName).toString();
                }
        }

        xTestedEntry.State = sResult;
        return null;
    }

    /**
     * Set the result of the method that is tested.
     * @param methodName The name of the method.
     * @param bResult The result of the test.
     */
    public void methodTested(String methodName, boolean bResult) {
        tRes.tested(methodName, bResult);
    }

    /**
     * @return The name of the interface or the service tested.
     */
    String getTestedClassName() {
        return testName;
    }
}
