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

import java.util.ArrayList;

import share.ComplexTest;
import share.DescEntry;
import share.DescGetter;
import share.LogWriter;
import util.DynamicClassLoader;

import complexlib.ComplexTestCase;

/**
 *
 */
public class ComplexDescGetter extends DescGetter
{

    private ComplexTest testClass;

    /** Creates new ComplexDescGetter */
    public ComplexDescGetter()
    {
        testClass = null;
    }

    @Override
    public DescEntry[] getDescriptionFor(String entry, String DescPath,
            boolean debug)
    {
        // read scenario file
        if (entry.startsWith("-sce"))
        {
            DescEntry[] entries = getScenario(entry.substring(5), null, debug);
            return entries;
        }
        // one single job
        else if (entry.startsWith("-o"))
        {
            DescEntry dEntry = getDescriptionForSingleJob(entry.substring(3), null, debug);
            if (dEntry != null)
            {
                return new DescEntry[]
                        {
                            dEntry
                        };
            }
        }
        System.out.println("Could not get a testjob with parameter '" + entry + "'");
        // no job available
        return null;
    }

    @Override
    protected DescEntry getDescriptionForSingleJob(String className, String descPath, boolean debug)
    {
        DynamicClassLoader dcl = new DynamicClassLoader();
        String methodNames[] = null;

        if (debug)
        {
            System.out.println("Searching Class: " + className);
        }

        int index = className.indexOf("::");
        if (index != -1)
        {
            // case1: method()
            // case2: method(param1,param2)
            // case3: method1(param1,param2),method2(param1,param2)
            String method = className.substring(index + 2);
            className = className.substring(0, index);
            ArrayList<String> methods = new ArrayList<String>();

            String[] split = method.split("(?<=\\)),(?=\\w+)");

            for (int i = 0; i < split.length; i++)
            {
                String meth = split[i];

                if (meth.endsWith("()"))
                {
                    meth = meth.substring(0, meth.length() - 2);
                }

                methods.add(meth);
            }

            methodNames = methods.toArray(new String[methods.size()]);
        }

        // create an instance
        try
        {
            testClass = (ComplexTestCase) dcl.getInstance(className);
        }
        catch (java.lang.IllegalArgumentException e)
        {
            System.out.println("Error while getting description for test '" + className + "' as a Complex test.");
            return null;
        }
        catch (java.lang.ClassCastException e)
        {
            System.out.println("The given class '" + className + "' is not a Complex test.");
            return null;
        }


        if (debug)
        {
            System.out.println("Got test: " + ((Object) testClass).toString());
        }

        String testObjectName = className;
        String[] testMethodNames = testClass.getTestMethodNames();
        if (methodNames != null)
        {
            testMethodNames = methodNames;
        }

        DescEntry dEntry = createTestDesc(testObjectName, className, testMethodNames, null);

        return dEntry;
    }

    /**
     * Creates a description entry for the given parameter
     * @param testObjectName the name of the object
     * @param className the class name of the class to load
     * @param testMethodNames list of all methods to test
     * @return filled description entry
     */
    private DescEntry createTestDesc(String testObjectName, String className, String[] testMethodNames, LogWriter log)
    {

        DescEntry dEntry = new DescEntry();

        dEntry.entryName = testObjectName;
        dEntry.longName = className;
        dEntry.isOptional = false;
        dEntry.EntryType = "unit";
        dEntry.isToTest = true;
        dEntry.Logger = log;
        dEntry.SubEntryCount = testMethodNames.length;
        dEntry.SubEntries = new DescEntry[dEntry.SubEntryCount];
        for (int i = 0; i < dEntry.SubEntryCount; i++)
        {
            DescEntry aEntry = new DescEntry();
            aEntry.entryName = testMethodNames[i];
            aEntry.longName = testObjectName + "::" + aEntry.entryName;
            aEntry.isOptional = false;
            aEntry.EntryType = "method";
            aEntry.isToTest = true;
            dEntry.SubEntries[i] = aEntry;
            dEntry.Logger = log;
        }

        return dEntry;
    }

    @Override
    protected String[] createScenario(String descPath, String job, boolean debug)
    {
        return new String[] {};
    }
}
