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

package installer;

import java.net.URLDecoder;
import java.io.*;
import java.util.*;
import java.net.*;

public class InstUtil {

    public static File buildSversionLocation() throws IOException {
        File theFile = null;
        StringBuffer str = new StringBuffer();
        str.append(System.getProperty("user.home"));
        str.append(File.separator);
        StringBuffer thePath = new StringBuffer(str.toString());

        String os = System.getProperty("os.name");

        if (os.indexOf("Windows") != -1) {
            boolean bSVersionInHomeDir = new File(thePath.toString() + "sversion.ini").exists();

            if (!bSVersionInHomeDir) {
                thePath.append("Application Data");
                thePath.append(File.separator);
            }
            theFile = findVersionFile(new File(thePath.toString()));
        } else if (os.indexOf("SunOS") != -1) {
            thePath.append(".sversionrc");
            theFile = new File(thePath.toString());
        } else if (os.indexOf("Linux") != -1) {
            thePath.append(".sversionrc");
            theFile = new File(thePath.toString());
        }

        if (theFile == null)
        {
            throw new IOException("Could not locate the OpenOffice settings file.\nAre you sure StarOffice is installed on your system?");
        }
        if  (!theFile.exists())
        {
            throw new IOException("Could not locate the OpenOffice settings file.\nAre you sure StarOffice is installed on your system?");
        }
        return theFile;
    }



    public static boolean hasNetbeansInstallation() {
        boolean result = false;
        try
        {
            result = checkForSupportedVersion( getNetbeansLocation(), versions );

            if (result == false)
                System.out.println("No supported version of NetBeans found.");
        }
        catch ( IOException ioe )
        {
            System.err.println("Exception caught trying to determine netbeans installation: " + ioe );
            ioe.printStackTrace();
            result = false;
        }
        return result;
    }

    private static boolean checkForSupportedVersion( Properties installs, String[] supportedVersions )
    {
        if ( installs != null )
        {
            for ( int index = 0; index < supportedVersions.length; index++ )
            {
                String key = supportedVersions[ index ];
                String path = null;
                if ( ( path = installs.getProperty(key) ) != null )
                {
                    // at least one supported version for netbeans present, so return;
                    return true;
                }

            }
        }
        return false;
    }


    public static boolean hasJeditInstallation() {
        boolean result = false;
        try
        {
            result = checkForSupportedVersion( getJeditLocation(), versions );
            if ( !result )
            {
                System.out.println("No supported version for JEdit found.");
            }
        }
        catch ( IOException ioe )
        {
            System.err.println("Exception caught trying to determine jedit installation: " + ioe );
            ioe.printStackTrace();
            result = false;
        }
        return result;
    }



    public static Properties getNetbeansLocation() throws IOException {
    File theFile = null;
    Properties results = new Properties();

    StringBuffer str = new StringBuffer();
        str.append(System.getProperty("user.home"));
        str.append(File.separator);
    StringBuffer thePath = new StringBuffer(str.toString());

        String os = System.getProperty("os.name");

    if (os.indexOf("Windows") != -1) {
        //theFile = findVersionFile(new File(str.toString()));
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("SunOS") != -1) {
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("Linux") != -1) {
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
    }

    if ( thePath.toString().indexOf( ".netbeans" ) == -1 )
        return null;
    else if ( new File( thePath.append( File.separator+"3.4"+File.separator ).toString() ).isDirectory() ) {

        System.out.println( "Found NetBeans 3.4 user directory: " + thePath );
        File netbeansLogFile = new File( thePath.toString() + File.separator + "system" + File.separator + "ide.log" );
        if( netbeansLogFile.exists() ) {
            String installPath = getNetbeansInstallation( netbeansLogFile );
            File f = new File(installPath);
            results.put("NetBeans 3.4", f.getPath()+File.separator);
            System.out.println( "NetBeans Installation directory: " + f.getPath());
        }
        else {
            System.out.println( "No NetBeans log file found" );
                        return null;
        }
    }
        else
        {
        System.out.println( "No NetBeans user directory found" );
        return null;
        }


    return results;
    }



    public static Properties getJeditLocation() throws IOException {

    /*if( !hasJeditInstallation() ) {
        System.out.println( "No Jedit found (line195 InstUtil");
        return null;
    }*/

    File theFile = null;
    Properties results = new Properties();

    StringBuffer str = new StringBuffer();
        str.append(System.getProperty("user.home"));
        str.append(File.separator);
    StringBuffer thePath = new StringBuffer(str.toString());

        String os = System.getProperty("os.name");
        thePath.append(".jedit");
    //System.out.println( ".jedit path " + thePath );

    File jeditLogFile = new File( thePath.toString() + File.separator + "activity.log" );
    if( jeditLogFile.exists() ) {
        String[] jeditDetails = getJeditInstallation( jeditLogFile );
        System.out.println( "getJeditLocation ) " + jeditDetails[0] );
        File f = new File(jeditDetails[0]);
        results.put("jEdit "+jeditDetails[1], jeditDetails[0]);
        System.out.println( "jeditDetails[0] is " + jeditDetails[0]);
    }
    else {
        System.out.println( "Prompt user for Jedit installation path" );
    }


    return results;
    }





    private static String getNetbeansInstallation( File logFile ) {
        String installPath = "";
        try {
        BufferedReader reader = new BufferedReader(new FileReader(logFile));

        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
        s.trim();
        if( s.indexOf( "IDE Install" ) != -1 ) {
            int pathStart = s.indexOf( "=" ) + 2;
            //System.out.println( "pathStart " + pathStart );
            installPath = s.substring( pathStart, s.length() );
            //System.out.println( "installPath 1" + installPath );
            int pathEnd = installPath.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            installPath = installPath.substring( 0, pathEnd ) +File.separator;
            //System.out.println( "pathStart " + pathStart );
            //int pathEnd = s.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //System.out.println( "s is " + s + " and " + s.length() + " long" );
            //installPath = s.substring( pathStart, pathEnd - 1 );
            installPath.trim();
            break;
        }
        }
        }
        catch( IOException ioe ) {
        System.out.println( "Error reading Netbeans location information" );
            }
        //catch( FileNotFoundException fnfe ) {
        //System.out.println( "NetBeans ide.log FileNotFoundException" );
        //}

        return installPath;
    }


    private static String[] getJeditInstallation( File logFile ) {
        String[] jeditDetails = new String[2];
        try {
        BufferedReader reader = new BufferedReader(new FileReader(logFile));
        String installPath = "";
        String version = "";

        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
        s.trim();
        if( s.indexOf( "jEdit home directory is" ) != -1 ) {
            int pathStart = new String( "[message] jEdit: jEdit home directory is " ).length();
            //System.out.println( "pathStart " + pathStart );
            installPath = s.substring( pathStart, s.length() ) +File.separator;
            System.out.println( "installPath 1" + installPath );
            //int pathEnd = installPath.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //installPath = installPath.substring( 0, pathEnd ) +File.separator;
            //System.out.println( "pathStart " + pathStart );
            //int pathEnd = s.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //System.out.println( "s is " + s + " and " + s.length() + " long" );
            //installPath = s.substring( pathStart, pathEnd - 1 );
            installPath.trim();
            //System.out.println( "installPath 2 " + installPath );
            //break;
            jeditDetails[0] = installPath;
        }
        if( s.indexOf( "jEdit: jEdit version" ) != -1 ) {
            int versionStart = s.indexOf( "version" ) + 8;
            System.out.println( "versionStart is: " + versionStart );
            version = s.substring( versionStart, s.length() );
            version.trim();
            System.out.println( "jEdit version is: " + version );
            jeditDetails[1] = version;
        }
        }
        }
        catch( IOException ioe ) {
        System.out.println( "Error reading Jedit location information" );
            }
        //catch( FileNotFoundException fnfe ) {
        //System.out.println( "Jedit activity.log FileNotFoundException" );
        //}

        return jeditDetails;
    }



    public static File findVersionFile(File start)
    {
        File versionFile = null;

        File files[] = start.listFiles(new VersionFilter());
        if (files.length == 0)
        {
            File dirs[] = start.listFiles(new DirFilter());
            for (int i=0; i< dirs.length; i++)
            {
                versionFile = findVersionFile(dirs[i]);
                if (versionFile != null)
                {
                    break;
                }
            }
        }
        else
        {
            versionFile = files[0];
        }

        return versionFile;
    }

    public static boolean verifySversionExists(File sversionFile) {
        if (!sversionFile.exists())
            return false;
        return true;
    }

    public static Properties getOfficeVersions(File sversionFile) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(sversionFile));
        String sectionName = null;
        Properties results = new Properties();

        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
            s.trim();
            //System.out.println(s);
            if (s.length() == 0)
                continue;
            if (s.charAt(0) == '[') {
                sectionName = s.substring(1, s.length() - 1);
                //System.out.println(sectionName);
                continue;
            }
            if ((sectionName != null) && sectionName.equalsIgnoreCase("Versions")) {
                int equals = s.indexOf( "=" );
        String officeName = s.substring(0, equals );

        String instPath = s.substring(equals + 8, s.length());
        String [] parts = new String[2];
        parts[0] = officeName;
        parts[1] = instPath + File.separator;
        //System.out.println( "InstUtil officeName " + officeName );
        //System.out.println( "InstUtil instPath " + instPath );

        //String [] parts = s.split("=");
                if (parts.length == 2) {
                    //ver.version = parts[0].trim();
                    //File f = new File(parts[1].trim());
            //results.put(parts[0].trim(), f.getPath());
            try {
                        URL url = new URL("file://" + parts[1].trim());
            String opSys =System.getProperty("os.name");
            if (opSys.indexOf("Windows")!=-1){
                String windowsPath = URLDecoder.decode( url.getPath() );
                boolean firstSlash = true;
                while( windowsPath.indexOf("/") != -1 ) {
                    int forwardSlashPos = windowsPath.indexOf("/");
                    String firstPart = windowsPath.substring( 0, forwardSlashPos );
                    String lastPart = windowsPath.substring( forwardSlashPos + 1, windowsPath.length() );
                    if( firstSlash ) {
                        windowsPath = lastPart;
                        firstSlash = false;
                    }
                    else {
                        windowsPath = firstPart + "\\" + lastPart;
                    }
                }
                int lastSlash = windowsPath.lastIndexOf("\\");
                windowsPath = windowsPath.substring( 0, lastSlash );
                results.put( parts[0].trim(), windowsPath );
            }
            else {
                //System.err.println( " InstUtil URLDecoder " + URLDecoder.decode(url.getPath()) );
                results.put(parts[0].trim(), URLDecoder.decode(url.getPath()));
            }
                        //File f = new File(url);

            //.sversion: OpenOffice.org 643=file:///scriptdev/neil/ScriptFrameOpenoffice1.0.1
            // parts = Installation name. f.getPath = Installation path
                        //results.put(parts[0].trim(), f.getPath());

                        //results.put(parts[0].trim(), URLDecoder.decode(url.getPath()));
            //results.put( parts[0].trim(), windowsPath );

                    }
                    catch (MalformedURLException eSyntax) {
                        //throw new IOException("Error while reading version information");
            results.put(parts[0].trim(), parts[1].trim());
            //System.out.println(parts[0].trim() + " : " + parts[1].trim());
            System.err.println("GotHereException");
                    }
                }
                else {
                    System.out.println("not splitting on equals");
                }
            }
        }

        return results;
    }

    public static String getJavaVersion() {
        return System.getProperty("java.version");
    }

    public static boolean isCorrectJavaVersion() {
        if (System.getProperty("java.version").startsWith("1.4"))
            return true;
        return false;
    }

    public static void main(String args[]) {
        InstUtil inst = new InstUtil();
        File f = null;
        try
        {
            f = inst.buildSversionLocation();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            System.out.println(e.getMessage());
        }
        if (!inst.verifySversionExists(f)) {
            System.err.println("Problem with sversion.ini");
        }
        try {
            Properties vers = inst.getOfficeVersions(f);
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println(e);
        }
        System.out.println(inst.getJavaVersion());
        if (!inst.isCorrectJavaVersion()) {
            System.err.println("Not correct Java Version");
        }
    }

    public static final String [] versions = {"NetBeans 3.4", "jEdit 4.0.3", "jEdit 4.1pre5" };
    private static File tmpDir = null;
}



class DirFilter implements java.io.FileFilter
{
    public boolean accept(File aFile)
    {
        return aFile.isDirectory();
    }
}
class VersionFilter implements java.io.FileFilter
{
    public boolean accept(File aFile)
    {
        if (aFile.getName().compareToIgnoreCase("sversion.ini") == 0)
        {
            return true;
        }

        return false;
    }
}
