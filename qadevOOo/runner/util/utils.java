/*************************************************************************
 *
 *  $RCSfile: utils.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change:$Date: 2004-07-23 10:44:13 $
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

package util;

import java.lang.System;
import java.util.StringTokenizer;
import java.io.*;
import java.util.ArrayList;
import java.io.RandomAccessFile;
import java.net.Socket;
import java.net.ServerSocket;

import com.sun.star.container.XNameAccess;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.lang.XServiceInfo;

import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

//For database connection
import java.sql.*;
import java.util.Collections;

public class utils {

    /**
     *
     * This method adds the DOCPTH to a given file
     *
     */
    public static String getFullTestDocName( String sDocName ) {
        String docpth = System.getProperty("DOCPTH");
        if (docpth.endsWith("\\") || docpth.endsWith("/"))
            docpth = docpth.substring(0, docpth.length() - 1) ;

        if (docpth.startsWith("http:")) {
            return docpth+"/"+sDocName;
        }
        String testdocPth = "";
        String pthSep = System.getProperty("file.separator");
        if (docpth == null ) {
            String objdscPth = System.getProperty("OBJDSCS");
            int i = objdscPth.indexOf("objdsc");
            String arcPth = objdscPth.substring(0,i-1);
            testdocPth = arcPth + pthSep + "doc" + pthSep + "java" +
                            pthSep + "testdocs" + pthSep + sDocName;
        }
        else {
            testdocPth = docpth + pthSep + sDocName;
        }
        return testdocPth;
    }

    /**
     *
     * This method adds the DOCPTH to a given file
     * and changes the format to an file URL
     *
     */

    public static String getFullTestURL( String sDocName ) {
        String fullDocPath = getFullTestDocName(sDocName);
        if (fullDocPath.startsWith("http:")) {
            return fullDocPath;
        }
        if (fullDocPath.startsWith("file:")) {
            return fullDocPath;
        }
        String prefix = null;

        //  Windows: \\\\margritte\\qaapi\\workspace\\qadev\\testdocs/emptyChart.sds
        if (fullDocPath.startsWith("\\\\"))
        prefix = "file:";

        fullDocPath = fullDocPath.replace('\\','/');
        if (prefix == null){
            if (fullDocPath.startsWith("//"))
                prefix="file:/";
            else if (fullDocPath.startsWith("/"))
                prefix="file://";
            else
                prefix="file:///";
        }
        if (!fullDocPath.endsWith("/")) {
            File aFile = new File(fullDocPath);
            if (aFile.isDirectory()) {
                fullDocPath +="/";
            }
        }
        String fulldocURL = prefix+fullDocPath;
        return fulldocURL;
   }

   /**
    *
    * This method changes a given URL to a valid file URL
    *
    */

   public static String getFullURL( String sDocName ) {
        String fullDocPath = sDocName;
        System.out.println("##### getFullURL (in): "+sDocName);
        if (fullDocPath.startsWith("http:")) {
            return fullDocPath;
        }
        if (fullDocPath.startsWith("file:")) {
            return fullDocPath;
        }
        if (fullDocPath.startsWith("ftp:")) {
            return fullDocPath;
        }
        fullDocPath = fullDocPath.replace('\\','/');
        String prefix = "";
        if (fullDocPath.startsWith("//")) {
            prefix="file:";
        } else {
            if (fullDocPath.startsWith("/")) prefix="file://";
                                    else prefix="file:///";
        }

        if (!fullDocPath.endsWith("/")) {
            File aFile = new File(fullDocPath);
            if (aFile.isDirectory()) {
                fullDocPath +="/";
            }
        }
        String fulldocURL = prefix+fullDocPath;
        System.out.println("##### getFullURL (out): "+fulldocURL);
        return fulldocURL;
    }

    /**
     *
     * This method creates folders needed
     *
     */


    public static void make_Directories(String first, String path) {
        String already_done = null;
        String fs = System.getProperty("file.separator");
        StringTokenizer path_tokenizer = new StringTokenizer(path,fs,false);
        already_done = first;
        while (path_tokenizer.hasMoreTokens()) {
            String part = path_tokenizer.nextToken();
            File new_dir = new File(already_done + File.separatorChar + part);
            already_done = new_dir.toString();
            //create the directory
            new_dir.mkdirs();
        }
        return;
    }

    /**
     *
     * This method get the version for a given TestBase/platform combination
     *
     */

    public static String getVersion(String aFile, String aPlatform, String aTestbase) {
        if ( (aFile == null) || (aPlatform == null) || (aTestbase == null) ){
            return "/";
        }

        File the_file = new File (aFile);
        try {
            RandomAccessFile raf = new RandomAccessFile(the_file, "r");
            String res="";
            while (!res.equals("["+aTestbase.toUpperCase()+"]")) {
                res = raf.readLine();
            }
            res="=/";
            while ( (!res.startsWith(aPlatform)) || (res.startsWith("[")) ) {
                res = raf.readLine();
            }
            raf.close();
            if (res.startsWith("[")) res="/";
            return res.substring(res.indexOf("=")+1);

        } catch (Exception e) {
            System.out.println("Couldn't find version");
            return "/";
        }
    }

    /**
     *
     * This method get's the temp dir of the connected office
     *
     */

    public static String getOfficeTemp (XMultiServiceFactory msf) {
        String tmpDir = System.getProperty("java.io.tmpdir");
        try {
            Object settings = msf.createInstance("com.sun.star.frame.Settings");
            if (settings == null) {
                String td = getFullURL(tmpDir);
                if (td == null)
                td = getFullTestDocName("");
                return td;
            }
            XNameAccess settingNames = (XNameAccess)
                            UnoRuntime.queryInterface(XNameAccess.class,settings);
            Object pSettings = settingNames.getByName("PathSettings");
            XPropertySet pthSettings = null;

            try {
                pthSettings = (XPropertySet) AnyConverter.toObject(
                                    new Type(XPropertySet.class),pSettings);
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                System.out.println("### couldn't convert Any");
            }

            String tmp = (String) pthSettings.getPropertyValue("UserPath");
            tmpDir = getFullURL(tmp+"/temp/");
        } catch (Exception e) {
            System.out.println("Couldn't get Office TEMP");
            e.printStackTrace();
        }
        return tmpDir;
    }

    /**
    * Gets StarOffice temp directory without 'file:///' prefix.
    * For example is usefull for Registry URL specifying.
    * @msf Office factory for accessing its settings.
    * @return SOffice temporary directory in form for example
    * 'd:/Office60/user/temp/'.
    */
    public static String getOfficeTempDir (XMultiServiceFactory msf) {

        String dir = getOfficeTemp(msf) ;

        int idx = dir.indexOf("file:///") ;

        if (idx < 0) return dir ;

        dir = dir.substring("file:///".length()) ;

        idx = dir.indexOf(":") ;

        // is the last character a '/' or a '\'?
        boolean lastCharSet = dir.endsWith("/") || dir.endsWith("\\");

        if (idx < 0) { // linux or solaris
                dir = "/"+dir ;
                dir += lastCharSet?"":"/";
        }
        else {  // windows
                dir += lastCharSet?"":"\\";
        }

        return dir;
    }

    /**
    * Gets StarOffice temp directory without 'file:///' prefix.
    * and System dependend file separator
    */
    public static String getOfficeTempDirSys (XMultiServiceFactory msf) {

        String dir = getOfficeTemp(msf) ;
        String sysDir = "";

        int idx = dir.indexOf("file://") ;

        // remove leading 'file://'
        if (idx < 0) sysDir = dir;
                    else sysDir = dir.substring("file://".length());

        // append '/' if not there (e.g. linux)
        if ( sysDir.charAt(sysDir.length()-1) != '/' )
            sysDir += "/";

        // remove leading '/' and replace others with '\' on windows machines
        String sep = System.getProperty("file.separator");
        if (sep.equalsIgnoreCase("\\")) {
            sysDir = sysDir.substring(1);
            sysDir = sysDir.replace('/','\\');
        }
        return sysDir;
    }

    /**
     *
     * This method copies a given file to a new one
     *
     */
    public static boolean copyFile (XMultiServiceFactory msf, String oldF, String newF) {
        boolean res = false;
        try {
            Object fileacc = msf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess simpleAccess = (XSimpleFileAccess)
                            UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
            if (!simpleAccess.exists(newF)) simpleAccess.copy(oldF,newF);

            res = true;
        } catch (Exception e) {
            System.out.println("Couldn't copy file");
            e.printStackTrace();
            res = false;
        }
        return res;
    }

    /**
     * Copies file to a new location using SOffice features. If the target
     * file already exists, the file is deleted.
     *
     * @returns <code>true</code> if the file was successfully copied,
     * <code>false</code> if some errors occured (e.g. file is locked, used
     * by another process).
     */
    public static boolean overwriteFile(XMultiServiceFactory msf, String oldF, String newF) {
        boolean res = false;
        try {
            Object fileacc = msf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");

            XSimpleFileAccess simpleAccess = (XSimpleFileAccess)
                UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
            if (simpleAccess.exists(newF)) simpleAccess.kill(newF);
            simpleAccess.copy(oldF,newF);
            res = true;
        } catch (com.sun.star.ucb.InteractiveAugmentedIOException e) {
            return false ;
        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Couldn't create a service.");
            e.printStackTrace();
        }

        return res;
    }

    public static boolean hasPropertyByName(XPropertySet props, String aName) {
        Property[] list = props.getPropertySetInfo().getProperties();
        boolean res = false;
        for (int i=0;i<list.length;i++) {
            String the_name = list[i].Name;
            if (aName.equals(the_name)) res=true;
        }
        return res;
    }

    /**
     *
     * This method returns the implementation name of a given object
     *
     */

    public static String getImplName (Object aObject) {
        String res = "Error getting Implementation name";
        try {
            XServiceInfo xSI = (XServiceInfo)
                UnoRuntime.queryInterface(XServiceInfo.class,aObject);
            res = xSI.getImplementationName();
        } catch (Exception e) {
            res = "Error getting Implementation name ( "+e+" )";
        }

        return res;
    }

    /**
     *
     * This method checks if an Object is void
     *
     */

    public static boolean isVoid (Object aObject) {
        if (aObject instanceof com.sun.star.uno.Any) {
            com.sun.star.uno.Any oAny = (com.sun.star.uno.Any) aObject;
            return (oAny.getType().getTypeName().equals("void"));
        } else {
            return false;
        }

    }

    /**
     *
     * This method replaces a substring with another
     *
     */

    public static String replacePart (String all, String toReplace, String replacement) {
        int where = 17;
        while (all.indexOf(toReplace) > 0) {
            where = all.indexOf(toReplace);
            String left = all.substring(0,where);
            String right = all.substring(where+toReplace.length(),all.length());
            all = left + replacement + right;
        }
        return all;
    }

    /**
     * Scan localhost for the next free port-number from a starting port
     * on. If the starting port is smaller than 1024, port number starts with
     * 10000 as default, because numbers < 1024 are never free on unix machines.
     * @param startPort The port where scanning starts.
     * @return The next free port.
     */
    public static int getNextFreePort(int startPort) {
        if (startPort < 1024) startPort = 10000;
        for (int port = startPort; port < 65536; port++) {
            System.out.println("Scan port " + port);
            try {
                // first trying to establish a server-socket on localhost
                // fails if there is already a server running
                ServerSocket sSock = new ServerSocket(port);
                sSock.close();
            }
            catch (IOException e) {
                System.out.println(" -> server: occupied port " + port);
                continue;
            }
            try {
                // now trying to establish a client-socket
                // fails if there is no server on any connectable machine
                Socket sock = new Socket("localhost", port);
                System.out.println(" -> socket: occupied port: " + port);
            }
            catch (IOException e) {
                System.out.println(" -> free port");
                return port;
            }
        }
        return 65535;
    }

    public static URL parseURL(XMultiServiceFactory xMSF, String url){
        URL[] rUrl = new URL[1];
        rUrl[0] = new URL();
        rUrl[0].Complete = url;

        XURLTransformer xTrans = null;
        try {
            Object inst = xMSF.createInstance
                ("com.sun.star.util.URLTransformer");
            xTrans = (XURLTransformer) UnoRuntime.queryInterface
                (XURLTransformer.class, inst);
        } catch (com.sun.star.uno.Exception e) {}

        xTrans.parseStrict(rUrl);

        return rUrl[0];
    }

    public static String getOfficeURL(XMultiServiceFactory msf) {
        try {
            Object settings = msf.createInstance("com.sun.star.util.PathSettings");
            XPropertySet settingProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class, settings);
            String path = (String) settingProps.getPropertyValue("Module");
            return path;
        } catch (Exception e) {
            System.out.println("Couldn't get Office Settings ");
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Get an array of all property names from the property set. With the include
     * and exclude parameters the properties can be filtered. <br>
     * Set excludePropertyAttribute = 0 and includePropertyAttribute = 0
     * to include all and exclude none.
     * @param props The instance of XPropertySet
     * @param includePropertyAttribute Properties without these attributes are filtered and will not be returned.
     * @param excludePropertyAttribute Properties with these attributes are filtered and will not be returned.
     * @return A String array with all property names.
     * @see com.sun.star.beans.XPropertySet
     * @see com.sun.star.beans.Property
     * @see com.sun.star.beans.PropertyAttribute
     */
    public static String[] getFilteredPropertyNames(XPropertySet props, short includePropertyAttribute, short excludePropertyAttribute) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        ArrayList l = new ArrayList();
        for (int i=0;i<the_props.length;i++) {
            boolean exclude = ((the_props[i].Attributes & excludePropertyAttribute) != 0);
            boolean include = (includePropertyAttribute == 0) || ((the_props[i].Attributes & includePropertyAttribute) != 0);
            if (include && !exclude) {
                l.add(the_props[i].Name);
            }
        }
        Collections.sort(l);
        String[] names = new String[l.size()];
        names = (String[])l.toArray(names);
        return names;
    }

}
