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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.StringTokenizer;
import java.util.jar.JarEntry;

import share.DescEntry;
import share.DescGetter;

/**
 * This is the Office-API specific DescGetter<br>
 * <br>
 * Examples:<br><br>
 * -o sw.SwXBodyText<br>
 * runs the module test of <B>Sw.SwXBodyText</B><br>
 * <br>
 * -o sw.SwXBodyText::com::sun::star::text::Text<br>
 * runs only the interface test <B>com.sun.star.textText</B> of the module <B>Sw.SwXBodyText</B><br>
 * <br>
 * -o sw.SwXBodyText::com::sun::star::text::Text,com::sun::star::text::XSimpleText<br>
 * runs only the interfaces test <B>com.sun.star.textText</B> and <B>com.sun.star.text.XSimpleText</B> of the module <B>Sw.SwXBodyText</B><br>
 * <br>
 * -p sw<br>
 * runs all modules of the project <B>sw</B><br>
 * <br>
 * -p listall<br>
 * lists all known module tests<br>
 * <br>
 * -sce SCENARIO_FILE<br>
 * A scenario file is a property file which could cotain <B>-o</B> and <B>-p</B> properties<br>
 * <br>
 * -sce sw.SwXBodyText,sw.SwXBookmark<br>
 * runs the module test of <B>Sw.SwXBodyText</B> and <B>sw.SwXBookmark</B><br>
 */
public class APIDescGetter extends DescGetter
{

    private static String fullJob = null;

    /*
     * gets the needed information about a StarOffice component
     * @param descPath Path to the ComponentDescription
     * @param entry contains the entry name, e.g. sw.SwXBodyText
     * @param debug if true some debug information is displayed on standard out
     */
    public DescEntry[] getDescriptionFor(String job, String descPath,
            boolean debug)
    {

        if (job.startsWith("-o"))
        {
            job = job.substring(3, job.length()).trim();

            if (job.indexOf(".") < 0)
            {
                return null;
            }

            // special in case several Interfaces are given comma separated
            if (job.indexOf(",") < 0)
            {
                DescEntry entry = getDescriptionForSingleJob(job, descPath,
                        debug);

                if (entry != null)
                {
                    return new DescEntry[]
                            {
                                entry
                            };
                }
                else
                {
                    return null;
                }
            }
            else
            {
                ArrayList<String> subs = getSubInterfaces(job);
                String partjob = job.substring(0, job.indexOf(",")).trim();
                DescEntry entry = getDescriptionForSingleJob(partjob, descPath,
                        debug);

                if (entry != null)
                {
                    for (int i = 0; i < entry.SubEntryCount; i++)
                    {
                        String subEntry = entry.SubEntries[i].longName;
                        int cpLength = entry.longName.length();
                        subEntry = subEntry.substring(cpLength + 2,
                                subEntry.length());

                        if (subs.contains(subEntry))
                        {
                            entry.SubEntries[i].isToTest = true;
                        }
                    }

                    return new DescEntry[]
                            {
                                entry
                            };
                }
                else
                {
                    return null;
                }
            }
        }

        if (job.startsWith("-p"))
        {
            job = job.substring(3, job.length()).trim();

            String[] scenario = createScenario(descPath, job, debug);
            if (scenario == null)
            {
                return null;
            }
            DescEntry[] entries = new DescEntry[scenario.length];
            for (int i = 0; i < scenario.length; i++)
            {
                entries[i] = getDescriptionForSingleJob(
                        scenario[i].substring(3).trim(), descPath, debug);
            }
            if (job.equals("listall"))
            {
                util.dbg.printArray(scenario);
                System.exit(0);
            }
            return entries;
        }

        if (job.startsWith("-sce"))
        {
            job = job.substring(5, job.length()).trim();

            File sceFile = new File(job);
            if (sceFile.exists())
            {
                return getScenario(job, descPath, debug);
            }
            else
            {
                //look the scenarion like this? :
                // sw.SwXBodyText,sw.SwXTextCursor
                ArrayList<String> subs = getSubObjects(job);
                DescEntry[] entries = new DescEntry[subs.size()];

                for (int i = 0; i < subs.size(); i++)
                {
                    entries[i] = getDescriptionForSingleJob(
                            subs.get(i), descPath, debug);
                }
                return entries;
            }
        }
        else
        {
            return null;
        }
    }

    protected DescEntry getDescriptionForSingleJob(String job, String descPath,
            boolean debug)
    {
        boolean isSingleInterface = job.indexOf("::") > 0;
        fullJob = job;

        if (isSingleInterface)
        {
            job = job.substring(0, job.indexOf("::"));
        }

        if (job.startsWith("bugs"))
        {
            DescEntry Entry = new DescEntry();
            Entry.entryName = job;
            Entry.longName = job;
            Entry.EntryType = "bugdoc";
            Entry.isOptional = false;
            Entry.isToTest = true;
            Entry.SubEntryCount = 0;
            Entry.hasErrorMsg = false;
            Entry.State = "non possible";

            return Entry;
        }

        DescEntry entry = null;

        if (descPath != null)
        {
            if (debug)
            {
                System.out.println("## reading from File " + descPath);
            }

            entry = getFromDirectory(descPath, job, debug);
        }
        else
        {
            if (debug)
            {
                System.out.println("## reading from jar");
            }

            entry = getFromClassPath(job, debug);
        }

        boolean foundInterface = false;

        if (isSingleInterface && (entry != null))
        {
            for (int i = 0; i < entry.SubEntryCount; i++)
            {
                if (!(entry.SubEntries[i].longName).equals(fullJob))
                {
                    entry.SubEntries[i].isToTest = false;
                }
                else
                {
                    foundInterface = true;
                    entry.SubEntries[i].isToTest = true;
                }
            }
        }

        if (isSingleInterface && !foundInterface || entry == null)
        {
            return setErrorDescription(entry,
                    "couldn't find a description for test '" + fullJob + "'");
        }

        return entry;
    }

    protected static DescEntry[] getSubEntries(BufferedReader cvsFile,
            DescEntry parent, boolean debug)
    {
        String line = "";
        String old_ifc_name = "";
        ArrayList<DescEntry> ifc_names = new ArrayList<DescEntry>();
        ArrayList<DescEntry> meth_names = new ArrayList<DescEntry>();
        DescEntry ifcDesc = null;

        while (line != null)
        {
            try
            {
                line = cvsFile.readLine();
                if (line == null)
                {
                    continue;
                }
                if (line.startsWith("#"))
                {
                    continue;
                }
                if (line.length() <= 0)
                {
                    continue;
                }
// TODO Probleme here
                // int nFirstSemicolon = line.indexOf(";");
                // int nLastSemicolon = line.lastIndexOf(";");

                String ifc_name = ""; //  = line.substring(line.indexOf(";") + 2, line.lastIndexOf(";") - 1);
                String meth_name = ""; //  = line.substring(line.lastIndexOf(";") + 2, line.length() - 1);
                StringTokenizer aToken = new StringTokenizer(line, ";");
                if (aToken.countTokens() < 3)
                {
                    System.out.println("Wrong format: Line '" + line + "' is not supported.");
                    continue;
                }
                if (aToken.hasMoreTokens())
                {
                    StringHelper.removeQuoteIfExists(aToken.nextToken());
                }
                if (aToken.hasMoreTokens())
                {
                    ifc_name = StringHelper.removeQuoteIfExists(aToken.nextToken());
                }
                if (aToken.hasMoreTokens())
                {
                    meth_name = StringHelper.removeQuoteIfExists(aToken.nextToken());
                }

                // String ifc_name = line.substring(line.indexOf(";") + 2, line.lastIndexOf(";") - 1);
                // String meth_name = line.substring(line.lastIndexOf(";") + 2, line.length() - 1);

                DescEntry methDesc = createDescEntry(meth_name, ifc_name, parent);

                if (!ifc_name.equals(old_ifc_name))
                {
                    if (ifcDesc != null)
                    {
                        ifcDesc.SubEntries = getDescArray(meth_names.toArray());
                        ifcDesc.SubEntryCount = meth_names.size();

                        //mark service/interface as optional if all methods/properties are optional
                        boolean allOptional = true;

                        for (int k = 0; k < ifcDesc.SubEntryCount; k++)
                        {
                            if (!ifcDesc.SubEntries[k].isOptional)
                            {
                                allOptional = false;
                            }
                        }

                        if (!ifcDesc.isOptional && allOptional)
                        {
                            ifcDesc.isOptional = allOptional;
                        }

                        meth_names.clear();
                        ifc_names.add(ifcDesc);
                    }

                    ifcDesc = new DescEntry();
                    ifcDesc.isToTest = true;
                    old_ifc_name = ifc_name;

                    if (ifc_name.indexOf("#optional") > 0)
                    {
                        ifcDesc.isOptional = true;
                        ifc_name = ifc_name.substring(0, ifc_name.indexOf("#"));
                    }

                    String className = createClassName(ifc_name);

                    ifcDesc.EntryType = entryType;
                    ifcDesc.entryName = "ifc" + className;
                    ifcDesc.longName = parent.entryName + "::" + ifc_name;
                }
                meth_names.add(methDesc);

            }
            catch (java.io.IOException ioe)
            {
                parent.hasErrorMsg = true;
                parent.ErrorMsg = "IOException while reading the description";

                return null;
            }
        }

        ifcDesc.SubEntries = getDescArray(meth_names.toArray());
        ifcDesc.SubEntryCount = meth_names.size();

        //mark service/interface as optional if all methods/properties are optional
        boolean allOptional = true;

        for (int k = 0; k < ifcDesc.SubEntryCount; k++)
        {
            if (!ifcDesc.SubEntries[k].isOptional)
            {
                allOptional = false;
            }
        }

        if (!ifcDesc.isOptional && allOptional)
        {
            ifcDesc.isOptional = allOptional;
        }

        ifc_names.add(ifcDesc);

        return getDescArray(makeArray(ifc_names));
    }
    private static String createClassName(String _ifc_name)
    {
        StringTokenizer st = new StringTokenizer(_ifc_name, ":");
        String className = "";

        int count = 3;

        if (_ifc_name.startsWith("drafts"))
        {
            count = 4;
        }

        for (int i = 0; st.hasMoreTokens(); i++)
        {
            String token = st.nextToken();

            // skipping (drafts.)com.sun.star
            if (i >= count)
            {
                if (!st.hasMoreTokens())
                {
                    // inserting '_' before the last token
                    token = "_" + token;
                }

                className += ("." + token);
            }
        }
        return className;
    }

    private static String entryType;

    private static DescEntry createDescEntry(String meth_name, String ifc_name, DescEntry parent)
    {
        entryType = "service";
        DescEntry methDesc = new DescEntry();

        if (meth_name.indexOf("#optional") > 0)
        {
            methDesc.isOptional = true;
            meth_name = meth_name.substring(0, meth_name.indexOf("#"));
        }

        if (meth_name.endsWith("()"))
        {
            methDesc.EntryType = "method";
            entryType = "interface";
        }
        else
        {
            methDesc.EntryType = "property";
            entryType = "service";
        }

        methDesc.entryName = meth_name;
        methDesc.isToTest = true;


        String withoutHash = ifc_name;

        if (ifc_name.indexOf("#optional") > 0)
        {
            withoutHash = ifc_name.substring(0, ifc_name.indexOf("#"));
        }

        methDesc.longName = parent.entryName + "::" + withoutHash + "::" + meth_name;

        return methDesc;
    }

    /**
     * This method ensures that XComponent will be the last in the list of interfaces
     */
    protected static Object[] makeArray(ArrayList<DescEntry> entries)
    {
        Object[] entriesArray = entries.toArray();
        ArrayList<Object> returnArray = new ArrayList<Object>();
        Object addAtEnd = null;

        for (int k = 0; k < entriesArray.length; k++)
        {
            DescEntry entry = (DescEntry) entriesArray[k];

            if (entry.entryName.equals("ifc.lang._XComponent"))
            {
                addAtEnd = entry;
            }
            else
            {
                returnArray.add(entry);
            }
        }

        if (addAtEnd != null)
        {
            returnArray.add(addAtEnd);
        }

        return returnArray.toArray();
    }

    protected static DescEntry setErrorDescription(DescEntry entry,
            String ErrorMsg)
    {
        if (entry == null)
        {
            entry = new DescEntry();
        }
        entry.hasErrorMsg = true;
        entry.ErrorMsg = "Error while getting description for test '" +
                fullJob + "' as an API test: " + ErrorMsg;

        return entry;
    }

    protected static DescEntry[] getDescArray(Object[] list)
    {
        DescEntry[] entries = new DescEntry[list.length];

        for (int i = 0; i < list.length; i++)
        {
            entries[i] = (DescEntry) list[i];
        }

        return entries;
    }

    protected DescEntry getFromClassPath(String aEntry, boolean debug)
    {
        int dotindex = aEntry.indexOf('.');

        if (dotindex == -1)
        {
            return null;
        }

        String module = null;
        String shortName = null;

        if (aEntry.indexOf(".uno") == -1)
        {
            module = aEntry.substring(0, aEntry.indexOf('.'));
            shortName = aEntry.substring(aEntry.indexOf('.') + 1);
        }
        else
        {
            module = aEntry.substring(0, aEntry.lastIndexOf('.'));
            shortName = aEntry.substring(aEntry.lastIndexOf('.') + 1);
        }

        DescEntry theEntry = new DescEntry();
        theEntry.entryName = aEntry;
        theEntry.longName = aEntry;
        theEntry.isOptional = false;
        theEntry.EntryType = "component";
        theEntry.isToTest = true;

        BufferedReader csvFile = null;

        java.net.URL url = this.getClass().getResource("/objdsc/" + module);

        if (url == null)
        {
            return setErrorDescription(theEntry,
                    "couldn't find module '" + module + "'");
        }

        try
        {
            java.net.URLConnection con = url.openConnection();

            String sEndsWithCSVName = "." + shortName.trim() + ".csv";
            if (con instanceof java.net.JarURLConnection)
            {
                // get Jar file from connection
                java.util.jar.JarFile f = ((java.net.JarURLConnection) con).getJarFile();

                // Enumerate over all entries
                java.util.Enumeration<JarEntry> e = f.entries();

                String sStartModule = "/" + module + "/";
                while (e.hasMoreElements())
                {

                    String entry = e.nextElement().toString();

//                    if (debug) {
//                        System.out.println("### Read from connetion: " + entry);
//                    }

                    if ((entry.lastIndexOf(sStartModule) != -1) &&
                            entry.endsWith(sEndsWithCSVName))
                    {
                        InputStream input = this.getClass().getResourceAsStream("/" + entry);
                        csvFile = new BufferedReader(new InputStreamReader(input));
                        break;
                    }
                }
            }
            else
            {
                InputStream in = con.getInputStream();
                java.io.BufferedReader buf = new java.io.BufferedReader(new InputStreamReader(in));
                boolean found = false;

                while (buf.ready() && !found)
                {
                    String entry = buf.readLine();

                    if (entry.endsWith(sEndsWithCSVName))
                    {
                        System.out.println("FOUND  ####");
                        InputStream input = this.getClass().getResourceAsStream("/objdsc/" +
                                module +
                                "/" +
                                entry);
                        csvFile = new BufferedReader(
                                new InputStreamReader(input));
                        found = true;
                    }
                }

                buf.close();
            }
        }
        catch (java.io.IOException e)
        {
            e.printStackTrace();
        }

        if (csvFile == null)
        {
            return setErrorDescription(theEntry,
                    "couldn't find component '" +
                    theEntry.entryName + "'");
        }

        DescEntry[] subEntries = getSubEntries(csvFile, theEntry, debug);

        theEntry.SubEntryCount = subEntries.length;
        theEntry.SubEntries = subEntries;

        return theEntry;
    }

    protected static DescEntry getFromDirectory(String descPath, String entry,
            boolean debug)
    {
        int dotindex = entry.indexOf('.');

        if (dotindex == -1)
        {
            return null;
        }

        String fs = System.getProperty("file.separator");
        String module = null;
        String shortName = null;

        if (entry.indexOf(".uno") == -1)
        {
            module = entry.substring(0, entry.indexOf('.'));
            shortName = entry.substring(entry.indexOf('.') + 1);
        }
        else
        {
            module = entry.substring(0, entry.lastIndexOf('.'));
            shortName = entry.substring(entry.lastIndexOf('.') + 1);
        }

        DescEntry aEntry = new DescEntry();
        aEntry.entryName = entry;
        aEntry.longName = entry;
        aEntry.isOptional = false;
        aEntry.EntryType = "component";
        aEntry.isToTest = true;

        if (debug)
        {
            System.out.println("Parsing Description Path: " + descPath);
            System.out.println("Searching module: " + module);
            System.out.println("For the Component " + shortName);
        }

        File modPath = new File(descPath + fs + module);

        if (!modPath.exists())
        {
            return setErrorDescription(aEntry,
                    "couldn't find module '" + module + "'");
        }

        String[] files = modPath.list();
        String found = "none";

        for (int i = 0; i < files.length; i++)
        {
            if (files[i].endsWith("." + shortName + ".csv"))
            {
                found = files[i];
                System.out.println("found " + found);
                break;
            }
        }

        if (found.equals("none"))
        {
            return setErrorDescription(aEntry,
                    "couldn't find component '" + entry + "'");
        }

        String aUrl = descPath + fs + module + fs + found;

        BufferedReader csvFile = null;

        try
        {
            csvFile = new BufferedReader(new FileReader(aUrl));
        }
        catch (java.io.FileNotFoundException fnfe)
        {
            return setErrorDescription(aEntry, "couldn't find file '" + aUrl + "'");
        }

        DescEntry[] subEntries = getSubEntries(csvFile, aEntry, debug);

        aEntry.SubEntryCount = subEntries.length;
        aEntry.SubEntries = subEntries;

        return aEntry;
    }

    protected ArrayList<String> getSubInterfaces(String job)
    {
        ArrayList<String> namesList = new ArrayList<String>();
        StringTokenizer st = new StringTokenizer(job, ",");

        for (; st.hasMoreTokens();)
        {
            String token = st.nextToken();

            if (token.indexOf(".") < 0)
            {
                namesList.add(token);
            }
        }

        return namesList;
    }

    protected ArrayList<String> getSubObjects(String job)
    {
        ArrayList<String> namesList = new ArrayList<String>();
        StringTokenizer st = new StringTokenizer(job, ",");

        for (; st.hasMoreTokens();)
        {
            namesList.add(st.nextToken());
        }

        return namesList;
    }

    protected String[] createScenario(String descPath, String job,
            boolean debug)
    {
        String[] scenario = null;

        if (descPath != null)
        {
            if (debug)
            {
                System.out.println("## reading from File " + descPath);
            }

            scenario = getScenarioFromDirectory(descPath, job, debug);
        }
        else
        {
            if (debug)
            {
                System.out.println("## reading from jar");
            }

            scenario = getScenarioFromClassPath(job, debug);
        }

        return scenario;
    }

    protected String[] getScenarioFromDirectory(String descPath, String job,
            boolean debug)
    {
        String[] modules = null;
        ArrayList<String> componentList = new ArrayList<String>();

        if (!job.equals("unknown") && !job.equals("listall"))
        {
            modules = new String[]
                    {
                        job
                    };
        }
        else
        {
            File dirs = new File(descPath);

            if (!dirs.exists())
            {
                modules = null;
            }
            else
            {
                modules = dirs.list();
            }
        }

        for (int i = 0; i < modules.length; i++)
        {
            if (!isUnusedModule(modules[i]))
            {
                File moduleDir = new File(descPath + System.getProperty("file.separator") + modules[i]);
                if (moduleDir.exists())
                {
                    String[] components = moduleDir.list();
                    for (int j = 0; j < components.length; j++)
                    {
                        if (components[j].endsWith(".csv"))
                        {
                            String toAdd = getComponentForString(components[j], modules[i]);
                            toAdd = "-o " + modules[i] + "." + toAdd;
                            componentList.add(toAdd);
                        }
                    }
                }
            }
        }

        String[] scenario = new String[componentList.size()];
        Collections.sort(componentList);

        for (int i = 0; i < componentList.size(); i++)
        {
            scenario[i] = componentList.get(i);
        }

        return scenario;

    }

    protected String[] getScenarioFromClassPath(String job, boolean debug)
    {
        String subdir = "/";

        if (!job.equals("unknown") && !job.equals("listall"))
        {
            subdir += job;
        }

        java.net.URL url = this.getClass().getResource("/objdsc" + subdir);

        if (url == null)
        {
            return null;
        }

        ArrayList<String> scenarioList = new ArrayList<String>();

        try
        {
            java.net.URLConnection con = url.openConnection();

            if (con instanceof java.net.JarURLConnection)
            {
                // get Jar file from connection
                java.util.jar.JarFile f = ((java.net.JarURLConnection) con).getJarFile();

                // Enumerate over all entries
                java.util.Enumeration<JarEntry> e = f.entries();

                while (e.hasMoreElements())
                {
                    String entry = e.nextElement().toString();

                    if (entry.startsWith("objdsc" + subdir) &&
                            (entry.indexOf("CVS") < 0) &&
                            !entry.endsWith("/"))
                    {
                        int startMod = entry.indexOf("/");
                        int endMod = entry.lastIndexOf("/");
                        String module = entry.substring(startMod + 1, endMod);
                        String component = getComponentForString(
                                entry.substring(endMod + 1,
                                entry.length()),
                                module);

                        if (!isUnusedModule(module))
                        {
                            scenarioList.add("-o " + module + "." +
                                    component);
                        }
                    }
                }
            }
        }
        catch (java.io.IOException e)
        {
            e.printStackTrace();
        }

        String[] scenario = new String[scenarioList.size()];
        Collections.sort(scenarioList);

        for (int i = 0; i < scenarioList.size(); i++)
        {
            scenario[i] = scenarioList.get(i);
        }

        return scenario;
    }

    protected String getComponentForString(String full, String module)
    {
        String component = "";


        //cutting .csv
        full = full.substring(0, full.length() - 4);

        //cutting component
        int lastdot = full.lastIndexOf(".");
        component = full.substring(lastdot + 1, full.length());

        if (module.equals("file") || module.equals("xmloff"))
        {
            String withoutComponent = full.substring(0, lastdot);
            int preLastDot = withoutComponent.lastIndexOf(".");
            component = withoutComponent.substring(preLastDot + 1,
                    withoutComponent.length()) +
                    "." + component;
        }

        return component;
    }

    protected boolean isUnusedModule(String moduleName)
    {
        ArrayList<String> removed = new ArrayList<String>();
        removed.add("acceptor");
        removed.add("brdgfctr");
        removed.add("connectr");
        removed.add("corefl");
        removed.add("cpld");
        removed.add("defreg");
        removed.add("dynamicloader");
        removed.add("impreg");
        removed.add("insp");
        removed.add("inv");
        removed.add("invadp");
        removed.add("javaloader");
        removed.add("jen");
        removed.add("namingservice");
        removed.add("proxyfac");
        removed.add("rdbtdp");
        removed.add("remotebridge");
        removed.add("simreg");
        removed.add("smgr");
        removed.add("stm");
        removed.add("tcv");
        removed.add("tdmgr");
        removed.add("ucprmt");
        removed.add("uuresolver");

        return removed.contains(moduleName);
    }
}
