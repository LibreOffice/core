/*************************************************************************
 *
 *  $RCSfile: APIDescGetter.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change:$Date: 2004-05-03 08:47:32 $
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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import java.util.ArrayList;
import java.util.Collections;
import java.util.StringTokenizer;

import share.DescEntry;
import share.DescGetter;


/*
 * This is the Office-API specific DescGetter
 *
 */
public class APIDescGetter extends DescGetter {
    /*
     * gets the needed information about a StarOffice component
     * @param descPath Path to the ComponentDescription
     * @param entry contains the entry name, e.g. sw.SwXBodyText
     * @param debug if true some debug information is displayed on standard out
     */
    public DescEntry[] getDescriptionFor(String job, String descPath,
                                         boolean debug) {

        if (job.startsWith("-o")) {
            job = job.substring(3, job.length()).trim();

            if (job.indexOf(".") < 0) {
                return null;
            }

            // special in case several Interfaces are given comma separated
            if (job.indexOf(",") < 0) {
                DescEntry entry = getDescriptionForSingleJob(job, descPath,
                                                             debug);

                if (entry != null) {
                    return new DescEntry[] { entry };
                } else {
                    return null;
                }
            } else {
                ArrayList subs = getSubInterfaces(job);
                String partjob = job.substring(0, job.indexOf(",")).trim();
                DescEntry entry = getDescriptionForSingleJob(partjob, descPath,
                                                             debug);

                if (entry != null) {
                    for (int i = 0; i < entry.SubEntryCount; i++) {
                        String subEntry = entry.SubEntries[i].longName;
                        int cpLength = entry.longName.length();
                        subEntry = subEntry.substring(cpLength + 2,
                                                      subEntry.length());

                        if (subs.contains(subEntry)) {
                            entry.SubEntries[i].isToTest = true;
                        }
                    }

                    return new DescEntry[] { entry };
                } else {
                    return null;
                }
            }
        }

        if (job.startsWith("-p")) {
            job = job.substring(3, job.length()).trim();

            String[] scenario = createScenario(descPath, job, debug);
            if (scenario == null) {
                return null;
            }
            DescEntry[] entries = new DescEntry[scenario.length];
            for (int i=0;i<scenario.length;i++) {
                entries[i] = getDescriptionForSingleJob(
                                    scenario[i].substring(3).trim(), descPath, debug);
            }
            if (job.equals("listall")) {
                util.dbg.printArray(scenario);
                System.exit(0);
            }
            return entries;
        }

        if (job.startsWith("-sce")) {
            job = job.substring(5, job.length()).trim();

            return getScenario(job, descPath, debug);
        } else {
            return null;
        }
    }

    protected DescEntry getDescriptionForSingleJob(String job, String descPath,
                                                   boolean debug) {
        boolean isSingleInterface = job.indexOf("::") > 0;
        String fullJob = job;

        if (isSingleInterface) {
            job = job.substring(0, job.indexOf("::"));
        }

        if (job.startsWith("bugs")) {
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

        if (descPath != null) {
            if (debug) {
                System.out.println("## reading from File " + descPath);
            }

            entry = getFromDirectory(descPath, job, debug);
        } else {
            if (debug) {
                System.out.println("## reading from jar");
            }

            entry = getFromClassPath(job, debug);
        }

        boolean foundInterface = false;

        if (isSingleInterface && (entry != null)) {
            for (int i = 0; i < entry.SubEntryCount; i++) {
                if (!(entry.SubEntries[i].longName).equals(fullJob)) {
                    entry.SubEntries[i].isToTest = false;
                } else {
                    foundInterface = true;
                    entry.SubEntries[i].isToTest = true;
                }
            }
        }

        if (isSingleInterface && !foundInterface) {
            entry.hasErrorMsg = true;
            entry.ErrorMsg = "Couldn't find a description for " + fullJob;
        }

        return entry;
    }

    protected static DescEntry[] getSubEntries(BufferedReader cvsFile,
                                               DescEntry parent, boolean debug) {
        String line = "";
        String old_ifc_name = "";
        ArrayList ifc_names = new ArrayList();
        ArrayList meth_names = new ArrayList();
        DescEntry ifcDesc = null;
        DescEntry methDesc = null;
        String entryType = "service";

        while (line != null) {
            try {
                line = cvsFile.readLine();

                if ((line != null) && (line.length() > 0)) {
                    String ifc_name = line.substring(line.indexOf(";") + 2,
                                                     line.lastIndexOf(";") - 1);
                    String meth_name = line.substring(line.lastIndexOf(";") + 2,
                                                      line.length() - 1);

                    methDesc = new DescEntry();

                    if (meth_name.indexOf("#optional") > 0) {
                        methDesc.isOptional = true;
                        meth_name = meth_name.substring(0,
                                                        meth_name.indexOf("#"));
                    }

                    if (meth_name.endsWith("()")) {
                        methDesc.EntryType = "method";
                        entryType = "interface";
                    } else {
                        methDesc.EntryType = "property";
                        entryType = "service";
                    }

                    methDesc.entryName = meth_name;
                    methDesc.isToTest = true;

                    String withoutHash = ifc_name;

                    if (ifc_name.indexOf("#optional") > 0) {
                        withoutHash = ifc_name.substring(0,
                                                         ifc_name.indexOf("#"));
                    }

                    methDesc.longName = parent.entryName + "::" +
                                        withoutHash + "::" + meth_name;

                    if (!ifc_name.equals(old_ifc_name)) {
                        if (ifcDesc != null) {
                            ifcDesc.SubEntries = getDescArray(
                                                         meth_names.toArray());
                            ifcDesc.SubEntryCount = meth_names.size();

                            //mark service/interface as optional if all methods/properties are optional
                            boolean allOptional = true;

                            for (int k = 0; k < ifcDesc.SubEntryCount; k++) {
                                if (!ifcDesc.SubEntries[k].isOptional) {
                                    allOptional = false;
                                }
                            }

                            if (!ifcDesc.isOptional && allOptional) {
                                ifcDesc.isOptional = allOptional;
                            }

                            meth_names.clear();
                            ifc_names.add(ifcDesc);
                        }

                        ifcDesc = new DescEntry();
                        ifcDesc.isToTest = true;
                        old_ifc_name = ifc_name;

                        if (ifc_name.indexOf("#optional") > 0) {
                            ifcDesc.isOptional = true;
                            ifc_name = ifc_name.substring(0,
                                                          ifc_name.indexOf("#"));
                        }

                        StringTokenizer st = new StringTokenizer(ifc_name, ":");
                        String className = "";

                        int count = 3;

                        if (ifc_name.startsWith("drafts")) {
                            count = 4;
                        }

                        for (int i = 0; st.hasMoreTokens(); i++) {
                            String token = st.nextToken();

                            // skipping (drafts.)com.sun.star
                            if (i >= count) {
                                if (!st.hasMoreTokens()) {
                                    // inserting '_' before the last token
                                    token = "_" + token;
                                }

                                className += ("." + token);
                            }
                        }

                        ifcDesc.EntryType = entryType;
                        ifcDesc.entryName = "ifc" + className;
                        ifcDesc.longName = parent.entryName + "::" +
                                           ifc_name;
                    }

                    meth_names.add(methDesc);
                }
            } catch (java.io.IOException ioe) {
                parent.hasErrorMsg = true;
                parent.ErrorMsg = "IOException while reading the description";

                return null;
            }
        }

        ifcDesc.SubEntries = getDescArray(meth_names.toArray());
        ifcDesc.SubEntryCount = meth_names.size();

        //mark service/interface as optional if all methods/properties are optional
        boolean allOptional = true;

        for (int k = 0; k < ifcDesc.SubEntryCount; k++) {
            if (!ifcDesc.SubEntries[k].isOptional) {
                allOptional = false;
            }
        }

        if (!ifcDesc.isOptional && allOptional) {
            ifcDesc.isOptional = allOptional;
        }

        ifc_names.add(ifcDesc);

        return getDescArray(makeArray(ifc_names));
    }

    /**
     * This method ensures that XComponent will be the last in the list of interfaces
     */
    protected static Object[] makeArray(ArrayList entries) {
        Object[] entriesArray = entries.toArray();
        ArrayList returnArray = new ArrayList();
        Object addAtEnd = null;

        for (int k = 0; k < entriesArray.length; k++) {
            DescEntry entry = (DescEntry) entriesArray[k];

            if (entry.entryName.equals("ifc.lang._XComponent")) {
                addAtEnd = entry;
            } else {
                returnArray.add(entry);
            }
        }

        if (addAtEnd != null) {
            returnArray.add(addAtEnd);
        }

        return returnArray.toArray();
    }

    protected static DescEntry setErrorDescription(DescEntry entry,
                                                   String ErrorMsg) {
        entry.hasErrorMsg = true;
        entry.ErrorMsg = ErrorMsg;

        return entry;
    }

    protected static DescEntry[] getDescArray(Object[] list) {
        DescEntry[] entries = new DescEntry[list.length];

        for (int i = 0; i < list.length; i++) {
            entries[i] = (DescEntry) list[i];
        }

        return entries;
    }

    protected DescEntry getFromClassPath(String aEntry, boolean debug) {
        int dotindex = aEntry.indexOf('.');

        if (dotindex == -1) {
            return null;
        }

        String module = null;
        String shortName = null;

        if (aEntry.indexOf(".uno") == -1) {
            module = aEntry.substring(0, aEntry.indexOf('.'));
            shortName = aEntry.substring(aEntry.indexOf('.') + 1);
        } else {
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

        if (url == null) {
            return setErrorDescription(theEntry,
                                       "Couldn't find module " + module);
        }

        try {
            java.net.URLConnection con = url.openConnection();

            if (con instanceof java.net.JarURLConnection) {
                // get Jar file from connection
                java.util.jar.JarFile f = ((java.net.JarURLConnection) con).getJarFile();

                // Enumerate over all entries
                java.util.Enumeration e = f.entries();

                while (e.hasMoreElements()) {
                    String entry = e.nextElement().toString();

                    if (entry.endsWith("." + shortName.trim() + ".csv")) {
                        InputStream input = this.getClass()
                                                .getResourceAsStream("/" +
                                                                     entry);
                        csvFile = new BufferedReader(
                                          new InputStreamReader(input));

                        break;
                    }
                }
            } else {
                InputStream in = con.getInputStream();
                java.io.BufferedReader buf = new java.io.BufferedReader(
                                                     new InputStreamReader(in));
                boolean found = false;

                while (buf.ready() && !found) {
                    String entry = buf.readLine();

                    if (debug) {
                        System.out.println("Read: " + entry);
                    }

                    if (entry.endsWith(shortName.trim() + ".csv")) {
                        InputStream input = this.getClass()
                                                .getResourceAsStream("/objdsc/" +
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
        } catch (java.io.IOException e) {
            e.printStackTrace();
        }

        if (csvFile == null) {
            return setErrorDescription(theEntry,
                                       "Couldn't find component " +
                                       theEntry.entryName);
        }

        DescEntry[] subEntries = getSubEntries(csvFile, theEntry, debug);

        theEntry.SubEntryCount = subEntries.length;
        theEntry.SubEntries = subEntries;

        return theEntry;
    }

    protected static DescEntry getFromDirectory(String descPath, String entry,
                                                boolean debug) {
        int dotindex = entry.indexOf('.');

        if (dotindex == -1) {
            return null;
        }

        String fs = System.getProperty("file.separator");
        String module = null;
        String shortName = null;

        if (entry.indexOf(".uno") == -1) {
            module = entry.substring(0, entry.indexOf('.'));
            shortName = entry.substring(entry.indexOf('.') + 1);
        } else {
            module = entry.substring(0, entry.lastIndexOf('.'));
            shortName = entry.substring(entry.lastIndexOf('.') + 1);
        }

        DescEntry aEntry = new DescEntry();
        aEntry.entryName = entry;
        aEntry.longName = entry;
        aEntry.isOptional = false;
        aEntry.EntryType = "component";
        aEntry.isToTest = true;

        if (debug) {
            System.out.println("Parsing Description Path: " + descPath);
            System.out.println("Searching module: " + module);
            System.out.println("For the Component " + shortName);
        }

        File modPath = new File(descPath + fs + module);

        if (!modPath.exists()) {
            return setErrorDescription(aEntry,
                                       "Couldn't find module " + module);
        }

        String[] files = modPath.list();
        String found = "none";

        for (int i = 0; i < files.length; i++) {
            if (files[i].endsWith("." + shortName + ".csv")) {
                found = files[i];

                break;
            }
        }

        if (found.equals("none")) {
            return setErrorDescription(aEntry,
                                       "Couldn't find component " + entry);
        }

        String aUrl = descPath + fs + module + fs + found;

        BufferedReader csvFile = null;

        try {
            csvFile = new BufferedReader(new FileReader(aUrl));
        } catch (java.io.FileNotFoundException fnfe) {
            return setErrorDescription(aEntry, "Couldn't find file " + aUrl);
        }

        DescEntry[] subEntries = getSubEntries(csvFile, aEntry, debug);

        aEntry.SubEntryCount = subEntries.length;
        aEntry.SubEntries = subEntries;

        return aEntry;
    }

    protected ArrayList getSubInterfaces(String job) {
        ArrayList namesList = new ArrayList();
        StringTokenizer st = new StringTokenizer(job, ",");

        for (int i = 0; st.hasMoreTokens(); i++) {
            String token = st.nextToken();

            if (token.indexOf(".") < 0) {
                namesList.add(token);
            }
        }

        return namesList;
    }

    protected String[] createScenario(String descPath, String job,
                                      boolean debug) {
        String[] scenario = null;

        if (descPath != null) {
            if (debug) {
                System.out.println("## reading from File " + descPath);
            }

            scenario = getScenarioFromDirectory(descPath, job, debug);
        } else {
            if (debug) {
                System.out.println("## reading from jar");
            }

            scenario = getScenarioFromClassPath(job, debug);
        }

        return scenario;
    }

    protected String[] getScenarioFromDirectory(String descPath, String job,
                                                boolean debug) {
        String[] modules = null;
        ArrayList componentList = new ArrayList();

        if (!job.equals("unknown") && !job.equals("listall")) {
            modules = new String[] { job };
        } else {
            File dirs = new File(descPath);

            if (!dirs.exists()) {
                modules = null;
            } else {
                modules = dirs.list();
            }
        }

        for (int i=0;i<modules.length;i++) {
            if (! isUnusedModule(modules[i])) {
                File moduleDir = new File(descPath+System.getProperty("file.separator")+modules[i]);
                String[] components = moduleDir.list();
                for (int j=0;j<components.length;j++) {
                    if (components[j].endsWith(".csv")) {
                        String toAdd = getComponentForString(components[j], modules[i]);
                        toAdd = "-o "+modules[i]+"."+toAdd;
                        componentList.add(toAdd);
                    }
                }
            }
        }

        String[] scenario = new String[componentList.size()];
        Collections.sort(componentList);

        for (int i = 0; i < componentList.size(); i++) {
            scenario[i] = (String) componentList.get(i);
        }

        return scenario;

    }

    protected String[] getScenarioFromClassPath(String job, boolean debug) {
        String subdir = "/";

        if (!job.equals("unknown") && !job.equals("listall")) {
            subdir += job;
        }

        java.net.URL url = this.getClass().getResource("/objdsc" + subdir);

        if (url == null) {
            return null;
        }

        ArrayList scenarioList = new ArrayList();

        try {
            java.net.URLConnection con = url.openConnection();

            if (con instanceof java.net.JarURLConnection) {
                // get Jar file from connection
                java.util.jar.JarFile f = ((java.net.JarURLConnection) con).getJarFile();

                // Enumerate over all entries
                java.util.Enumeration e = f.entries();

                while (e.hasMoreElements()) {
                    String entry = e.nextElement().toString();

                    if (entry.startsWith("objdsc" + subdir) &&
                            (entry.indexOf("CVS") < 0) &&
                            !entry.endsWith("/")) {
                        int startMod = entry.indexOf("/");
                        int endMod = entry.lastIndexOf("/");
                        String module = entry.substring(startMod + 1, endMod);
                        String component = getComponentForString(
                                                   entry.substring(endMod + 1,
                                                                   entry.length()),
                                                   module);

                        if (!isUnusedModule(module)) {
                            scenarioList.add("-o " + module + "." +
                                             component);
                        }
                    }
                }
            }
        } catch (java.io.IOException e) {
            e.printStackTrace();
        }

        String[] scenario = new String[scenarioList.size()];
        Collections.sort(scenarioList);

        for (int i = 0; i < scenarioList.size(); i++) {
            scenario[i] = (String) scenarioList.get(i);
        }

        return scenario;
    }

    protected String getComponentForString(String full, String module) {
        String component = "";


        //cutting .csv
        full = full.substring(0, full.length() - 4);

        //cutting component
        int lastdot = full.lastIndexOf(".");
        component = full.substring(lastdot + 1, full.length());

        if (module.equals("file") || module.equals("xmloff")) {
            String withoutComponent = full.substring(0, lastdot);
            int preLastDot = withoutComponent.lastIndexOf(".");
            component = withoutComponent.substring(preLastDot + 1,
                                                   withoutComponent.length()) +
                        "." + component;
        }

        return component;
    }

    protected boolean isUnusedModule(String moduleName) {
        ArrayList removed = new ArrayList();
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