/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComplexDescGetter.java,v $
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

import complexlib.ComplexTestCase;
import util.DynamicClassLoader;
import share.DescEntry;
import share.DescGetter;
import share.ComplexTest;
import java.io.FileReader;
import java.io.BufferedReader;
import java.lang.reflect.Method;
import java.util.StringTokenizer;
import java.util.Vector;
import lib.TestParameters;
import share.LogWriter;

/**
 *
 */
public class ComplexDescGetter extends DescGetter {

    ComplexTest testClass;

    /** Creates new ComplexDescGetter */
    public ComplexDescGetter() {
        testClass = null;
    }

    public DescEntry[] getDescriptionFor(String entry, String DescPath,
                                                            boolean debug) {
        // read scenario file
        if (entry.startsWith("-sce")) {
            DescEntry[] entries = getScenario(entry.substring(5),null,debug);
            return entries;
        }
        // one single job
        else if (entry.startsWith("-o")) {
            DescEntry dEntry = getDescriptionForSingleJob(entry.substring(3), null, debug);
            if (dEntry != null)
                return new DescEntry[] {dEntry};
        }
        System.out.println("Could not get a testjob with parameter '"
                            + entry +"'");
        // no job available
        return null;
    }


    protected DescEntry getDescriptionForSingleJob(String className, String descPath, boolean debug) {
        DynamicClassLoader dcl = new DynamicClassLoader();
        String methodNames[] = null;

        if (debug) {
            System.out.println("Searching Class: " + className);
        }

        int index = className.indexOf("::");
        if (index != -1) {
            String method = className.substring(index + 2);
            className = className.substring(0, index);
            Vector methods = new Vector();
            StringTokenizer t = new StringTokenizer(method, ",");
            while (t.hasMoreTokens()) {
                String m = t.nextToken();
                if (m.endsWith("()"))
                    m = m.substring(0, m.length() - 2);
                methods.add(m);
            }
            methodNames = new String[methods.size()];
            methodNames = (String[])methods.toArray(methodNames);
        }

        // create an instance
        try {
            testClass = (ComplexTestCase)dcl.getInstance(className);
        }
        catch(java.lang.IllegalArgumentException e) {
            System.out.println("Error while getting description for test '" +className + "' as a Complex test.");
            return null;
        }
        catch(java.lang.ClassCastException e) {
            System.out.println("The given class '" +className + "' is not a Complex test.");
            return null;
        }


        if (debug) {
            System.out.println("Got test: "+((Object)testClass).toString());
        }

        String testObjectName = className;
        String[] testMethodNames = null;

        if (testMethodNames == null){
            testMethodNames = testClass.getTestMethodNames();
        }
        if (methodNames != null) {
            testMethodNames = methodNames;
        }

        DescEntry dEntry = createTestDesc(testObjectName, className, testMethodNames, null);

        return dEntry;
    }

    /**
     * Creates a description exntry for the given parameter
     * @param testObjectName the name of the object
     * @param className the class name of the class to load
     * @param testMethodNames list of all methods to test
     * @param log
     * @return filled description entry
     */
    public DescEntry createTestDesc(String testObjectName, String className, String[] testMethodNames, LogWriter log){

        DescEntry dEntry = new DescEntry();

        dEntry.entryName = testObjectName;
        dEntry.longName = className;
        dEntry.isOptional = false;
        dEntry.EntryType = "unit";
        dEntry.isToTest = true;
        dEntry.Logger = log;
        dEntry.SubEntryCount = testMethodNames.length;
        dEntry.SubEntries = new DescEntry[dEntry.SubEntryCount];
        for (int i=0; i<dEntry.SubEntryCount; i++) {
            DescEntry aEntry = new DescEntry();
            aEntry.entryName = testMethodNames[i];
            aEntry.longName = testObjectName +"::" + aEntry.entryName;
            aEntry.isOptional = false;
            aEntry.EntryType = "method";
            aEntry.isToTest = true;
            dEntry.SubEntries[i] = aEntry;
            dEntry.Logger = log;
        }

        return dEntry;
    }

    protected String[] createScenario(String descPath, String job, boolean debug) {
        return new String[]{};
    }

}
