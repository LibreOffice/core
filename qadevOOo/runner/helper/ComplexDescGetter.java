/*************************************************************************
 *
 *  $RCSfile: ComplexDescGetter.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2004-11-02 11:33:06 $
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
package helper;

import complexlib.ComplexTestCase;
import util.DynamicClassLoader;
import share.DescEntry;
import share.DescGetter;
import share.ComplexTest;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.StringTokenizer;
import java.util.Vector;

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
        boolean returnVal = true;

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

/*        String testObjectName = testClass.getTestObjectName();
        if (testObjectName != null) {
            if (testObjectName.equals(""))
                testObjectName = className;
        }
        else */
        String testObjectName = className;

        String[] testMethodName = testClass.getTestMethodNames();
        if (methodNames != null) {
            testMethodName = methodNames;
        }

        DescEntry dEntry = new DescEntry();

        dEntry.entryName = testObjectName;
        dEntry.longName = className;
        dEntry.isOptional = false;
        dEntry.EntryType = "unit";
        dEntry.isToTest = true;
        dEntry.SubEntryCount = testMethodName.length;
        dEntry.SubEntries = new DescEntry[dEntry.SubEntryCount];
        for (int i=0; i<dEntry.SubEntryCount; i++) {
            DescEntry aEntry = new DescEntry();
            aEntry.entryName = testMethodName[i];
            aEntry.longName = testObjectName +"::" + aEntry.entryName;
            aEntry.isOptional = false;
            aEntry.EntryType = "method";
            aEntry.isToTest = true;
            dEntry.SubEntries[i] = aEntry;
        }

        return dEntry;
    }

    protected String[] createScenario(String descPath, String job, boolean debug) {
        return new String[]{};
    }

}
