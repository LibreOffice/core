/*************************************************************************
 *
 *  $RCSfile: APIDescGetter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-10-06 12:38:32 $
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

import share.DescGetter;
import share.DescEntry;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.StringTokenizer;

/*
 * This is the Office-API specific DescGetter
 *
 */

public class APIDescGetter extends DescGetter{

    /*
     * gets the needed information about a StarOffice component
     * @param descPath Path to the ComponentDescription
     * @param entry contains the entry name, e.g. sw.SwXBodyText
     * @param debug if true some debug information is displayed on standard out
     */

    public DescEntry[] getDescriptionFor(String job, String descPath,
                                                                boolean debug) {
        if (job.startsWith("-o")) {
            job = job.substring(3,job.length()).trim();
            if (job.indexOf(".") < 0) {
                return null;
            }
            DescEntry entry = getDescriptionForSingleJob(job,descPath,debug);
            if (entry != null) {
                return new DescEntry[]{entry};
            } else return null;
        } if (job.startsWith("-sce")) {
            job = job.substring(5,job.length()).trim();
            return getScenario(job,descPath,debug);
        } else return null;
    }


    protected DescEntry getDescriptionForSingleJob(
                                    String job, String descPath, boolean debug) {
        boolean isSingleInterface = job.indexOf("::")>0;
        String fullJob = job;
        if (isSingleInterface) {
            job = job.substring(0,job.indexOf("::"));
        }
        if (job.startsWith("bugs")) {
            DescEntry Entry = new DescEntry();
            Entry.entryName=job;
            Entry.longName=job;
            Entry.EntryType="bugdoc";
            Entry.isOptional=false;
            Entry.isToTest=true;
            Entry.SubEntryCount=0;
            Entry.hasErrorMsg=false;
            Entry.State="non possible";
            return Entry;
        }
        DescEntry entry = null;
        if (descPath != null) {
            System.out.println("## reading from File "+descPath);
            entry = getFromDirectory(descPath, job, debug);
        } else {
            System.out.println("## reading from jar");
            entry = getFromClassPath(job, debug);
        }
        boolean foundInterface = false;
        if (isSingleInterface && entry !=null) {
            for (int i = 0; i<entry.SubEntryCount; i++) {
                if (!(entry.SubEntries[i].longName).equals(fullJob)) {
                    entry.SubEntries[i].isToTest=false;
                } else {
                    foundInterface=true;
                    entry.SubEntries[i].isToTest=true;
                }
            }
        }
        if (isSingleInterface && !foundInterface) {
            entry.hasErrorMsg=true;
            entry.ErrorMsg = "Couldn't find a description for "+fullJob;
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
        while (line != null ) {
            try {
                line = cvsFile.readLine();
                if (line != null && line.length()>0) {
                    String ifc_name = line.substring(line.indexOf(";")+2,
                                                        line.lastIndexOf(";")-1);
                    String meth_name = line.substring(line.lastIndexOf(";")+2,
                                                                line.length()-1);

                    methDesc=new DescEntry();
                    if (meth_name.indexOf("#optional")>0) {
                        methDesc.isOptional=true;
                        meth_name = meth_name.substring(0,meth_name.indexOf("#"));
                    }

                    if (meth_name.endsWith("()")) {
                        methDesc.EntryType="method";
                        entryType = "interface";
                    } else {
                        methDesc.EntryType="property";
                        entryType = "service";
                    }
                    methDesc.entryName=meth_name;
                    methDesc.isToTest = true;
                    String withoutHash = ifc_name;
                    if (ifc_name.indexOf("#optional") > 0) {
                        withoutHash = ifc_name.substring(0,ifc_name.indexOf("#"));
                    }
                    methDesc.longName=parent.entryName+"::"+ withoutHash
                                                                +"::"+meth_name;


                    if (!ifc_name.equals(old_ifc_name)) {
                        if (ifcDesc != null) {
                            ifcDesc.SubEntries=getDescArray(meth_names.toArray());
                            ifcDesc.SubEntryCount=meth_names.size();
                            meth_names.clear();
                            ifc_names.add(ifcDesc);
                        }
                        ifcDesc = new DescEntry();
                        ifcDesc.isToTest = true;
                        old_ifc_name = ifc_name;
                        if (ifc_name.indexOf("#optional") > 0) {
                            ifcDesc.isOptional = true;
                            ifc_name = ifc_name.substring(0,ifc_name.indexOf("#"));
                        }

                        StringTokenizer st = new StringTokenizer(ifc_name, ":");
                        String className="";

                        int count=3;

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
                                className += "." + token;
                            }
                        }
                        ifcDesc.EntryType=entryType;
                        ifcDesc.entryName="ifc"+className;
                        ifcDesc.longName=parent.entryName+"::"+ifc_name;
                    }

                    meth_names.add(methDesc);
                }
            } catch (java.io.IOException ioe) {
                parent.hasErrorMsg = true;
                parent.ErrorMsg = "IOException while reading the description";
                return null;
            }
        }

        ifcDesc.SubEntries=getDescArray(meth_names.toArray());
        ifcDesc.SubEntryCount=meth_names.size();
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
        for (int k=0;k<entriesArray.length;k++) {
            DescEntry entry = (DescEntry) entriesArray[k];
            if (entry.entryName.equals("ifc.lang._XComponent")){
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

    protected static DescEntry setErrorDescription(DescEntry entry, String ErrorMsg) {
        entry.hasErrorMsg = true;
        entry.ErrorMsg = ErrorMsg;
        return entry;
    }

    protected static DescEntry[] getDescArray(Object[] list) {
        DescEntry[] entries = new DescEntry[list.length];
        for (int i = 0; i<list.length;i++) {
            entries[i] = (DescEntry) list[i];
        }
        return entries;
    }

    protected DescEntry getFromClassPath(String aEntry, boolean debug) {
        int dotindex = aEntry.indexOf('.');
        if (dotindex == -1) return null;
        String module = null;
        String shortName = null;
        if (aEntry.indexOf(".uno")==-1) {
            module = aEntry.substring(0,aEntry.indexOf('.'));
            shortName = aEntry.substring(aEntry.indexOf('.')+1);
        }
        else {
            module = aEntry.substring(0,aEntry.lastIndexOf('.'));
            shortName = aEntry.substring(aEntry.lastIndexOf('.')+1);
        }
        DescEntry theEntry = new DescEntry();
        theEntry.entryName = aEntry;
        theEntry.longName = aEntry;
        theEntry.isOptional = false;
        theEntry.EntryType = "component";
        theEntry.isToTest = true;
        BufferedReader csvFile = null;

        java.net.URL url = this.getClass().getResource("/objdsc/"+module);
        if (url == null && debug) {
           System.out.println("Classpath doesn't contain descriptions for" +
                            " module '" + module +"'.");
           return null;
        }

        try {
            java.net.URLConnection con = url.openConnection();
            if (con instanceof java.net.JarURLConnection) {
                // get Jar file from connection
                java.util.jar.JarFile f = ((java.net.JarURLConnection)con).getJarFile();
                // Enumerate over all entries
                java.util.Enumeration e = f.entries();
                while (e.hasMoreElements()) {
                   String entry = e.nextElement().toString();
                   if (entry.endsWith("."+shortName.trim()+".csv")) {
                       InputStream input =
                                this.getClass().getResourceAsStream("/" + entry);
                       csvFile =
                                new BufferedReader(new InputStreamReader(input));
                   }
                }
            }
            else {
                InputStream in = con.getInputStream();
                java.io.BufferedReader buf = new java.io.BufferedReader(new InputStreamReader(in));
                boolean found = false;
                while(buf.ready() && !found) {
                    String entry = buf.readLine();
                    System.out.println("Read: "+ entry);
                    if (entry.endsWith(shortName.trim()+".csv")) {
                        InputStream input =
                            this.getClass().getResourceAsStream("/objdsc/"+module+"/" + entry);
                        csvFile =
                            new BufferedReader(new InputStreamReader(input));
                        found = true;
                    }

                }
            }
        }
        catch(java.io.IOException e) {
           e.printStackTrace();
        }

        DescEntry[] subEntries = getSubEntries(csvFile,theEntry,debug);

        theEntry.SubEntryCount=subEntries.length;
        theEntry.SubEntries=subEntries;

        return theEntry;
    }

    protected static DescEntry getFromDirectory(String descPath, String entry,
                                                            boolean debug) {

        int dotindex = entry.indexOf('.');
        if (dotindex == -1) return null;
        String fs = System.getProperty("file.separator");
        String module = null;
        String shortName = null;
        if (entry.indexOf(".uno")==-1) {
            module = entry.substring(0,entry.indexOf('.'));
            shortName = entry.substring(entry.indexOf('.')+1);
        }
        else {
            module = entry.substring(0,entry.lastIndexOf('.'));
            shortName = entry.substring(entry.lastIndexOf('.')+1);
        }
        DescEntry aEntry = new DescEntry();
        aEntry.entryName = entry;
        aEntry.longName = entry;
        aEntry.isOptional = false;
        aEntry.EntryType = "component";
        aEntry.isToTest = true;

        if (debug) {
            System.out.println("Parsing Description Path: "+descPath);
            System.out.println("Searching module: "+module);
            System.out.println("For the Component "+shortName);
        }

        File modPath = new File(descPath+fs+module);

        if (!modPath.exists()) {
            return setErrorDescription(aEntry,"Couldn't find module "+module);
        }

        String[] files = modPath.list();
        String found = "none";

        for (int i=0;i<files.length;i++) {
            if (files[i].endsWith(shortName+".csv")) {
                found = files[i];
                break;
            }
        }

        if (found.equals("none")) {
            return setErrorDescription(aEntry,"Couldn't find component "+entry);
        }

        String aUrl = descPath+fs+module+fs+found;

        BufferedReader csvFile = null;
        try {
            csvFile = new BufferedReader(new FileReader(aUrl));
        } catch (java.io.FileNotFoundException fnfe) {
            return setErrorDescription(aEntry,"Couldn't find file "+aUrl);
        }

        DescEntry[] subEntries = getSubEntries(csvFile,aEntry,debug);

        aEntry.SubEntryCount=subEntries.length;
        aEntry.SubEntries=subEntries;

        return aEntry;

    }

}
