/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SystemManager.java,v $
 * $Revision: 1.6 $
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

package org.openoffice.setup.Util;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URL;
import java.util.HashMap;
import java.util.Properties;
import java.util.Vector;

public class SystemManager {

    private SystemManager() {
    }

    /* the installation root is where the classes reside */
    static public File getJarFilePath() {

        File jarFile = null;

        try {
            Class c  = Class.forName("org.openoffice.setup.ResourceManager");
            URL url  = c.getResource("setupfiles.properties");

            String urlString = url.toString();

            if (urlString.startsWith("jar:")) {
                /* ResourceManager.class resides in a jar file. Strip it down to the "file:" part */
                urlString = urlString.substring(4, urlString.lastIndexOf("!"));
                jarFile = new File(new URI(urlString));
            }

        } catch (Exception ex) {
            /* handle URISyntaxException and ClassNotFoundException */
            ex.printStackTrace();
            System.exit(1);
        }

        if ( jarFile != null ) {
            System.err.println("Jar file: " + jarFile.getPath());
        } else {
            System.err.println("No jar file used for installation!");
        }

        return jarFile;
    }

    /* the installation root is where the classes reside */
    static public File getResourceRoot() {

        File dir = null;

        try {
            Class c  = Class.forName("org.openoffice.setup.ResourceManager");
            URL url  = c.getResource("setupfiles.properties");

            String urlString = url.toString();

            if (urlString.startsWith("jar:")) {
                /* ResourceManager.class resides in a jar file. Strip it down to the "file:" part */
                urlString = urlString.substring(4, urlString.lastIndexOf("!"));
            } else {
                /* ResourceManager.class resides in a directory tree. */
                urlString = urlString.substring(0, urlString.lastIndexOf("/org/openoffice/setup/setupfiles.properties"));
            }

            dir = new File(new URI(urlString));
            dir = dir.getParentFile();

        } catch (Exception ex) {
            /* handle URISyntaxException and ClassNotFoundException */
            ex.printStackTrace();
            System.exit(1);
        }
        // }

        if ( dir != null ) {
            // System.err.println("Resource Root: " + dir.getPath());
        } else {
            System.err.println("No resource root found!");
        }

        return dir;
    }

    static public String getPackagePath(String subdir) {

        String path = null;

        File dir = getResourceRoot();
        if (dir != null) {
            // System.err.println("Resource root: " + dir.getPath());
            dir = new File(dir, subdir);
            if (! dir.exists()) {
                System.err.println("Error: Directory \"" + subdir + "\" does not exist at resouce root");
            } else {
                path = dir.getPath();
            }
        }

        if ( path != null ) {
            if ( ! path.endsWith("/")) {
                path = path + "/";
            }
        }

        if ( path != null ) {
            System.err.println("Path to packages: " + path);
        } else {
            System.err.println("No path to packages found!");
        }

        return path;
    }

    static public boolean find_file(String fileName) {
        boolean found = false;
        File file = new File(fileName);
        found = file.exists();
        return found;
    }

    static public boolean exists_directory(String directory) {
        File dir = new File(directory);
        return dir.exists();
    }

    static public boolean create_directory(String directory) throws SecurityException {
        boolean created = false;
        File dir = new File(directory);
        try {
            created = dir.mkdirs();
        }
        catch (SecurityException ex) {
            throw ex;
        }

        return created;
    }

    static public String getParentDirectory(String dir) {
        File installFile = new File(dir);
        String parentDir = installFile.getParent();
        if ( parentDir == null ) {
            parentDir = "/";
        }
        return parentDir;
    }

    static public String getInstallationPrivileges() {

        String type = "";
        String user = java.lang.System.getProperty("user.name");
        // System.out.println("UserHome: " + java.lang.System.getProperty("user.home"));

        if ( user.equalsIgnoreCase("root")) {
            type = "root";
            System.err.println("Root privileges");
        } else {
            type = "user";
            System.err.println("User privileges");
        }

        return type;
    }

    static public boolean isUserInstallation() {

        boolean isUserInstallation = false;
        String user = java.lang.System.getProperty("user.name");

        if ( user.equalsIgnoreCase("root")) {
            isUserInstallation = false;
            System.err.println("Root privileges");
        } else {
            isUserInstallation = true;
            System.err.println("User privileges");
        }

        return isUserInstallation;
    }

    static public boolean isRootInstallation() {

        boolean isRootInstallation = false;
        String user = java.lang.System.getProperty("user.name");

        if ( user.equalsIgnoreCase("root")) {
            isRootInstallation = true;
        } else {
            isRootInstallation = false;
        }

        return isRootInstallation;
    }

    static public String getOSType() {
        String osVersion = java.lang.System.getProperty("os.name");
        System.err.println("OS: " + osVersion);
        return osVersion;
    }

    static public String getOSArchitecture() {
        String osArchitecture = java.lang.System.getProperty("os.arch");
        System.out.println("OSArchitecture: " + osArchitecture);
        return osArchitecture;
    }

    static public String getOSVersion() {
        String osVersion = java.lang.System.getProperty("os.version");
        System.out.println("OSVersion: " + osVersion);
        return osVersion;
    }

    static public HashMap getEnvironmentHashMap() {
        // readonly map from getenv()
        // System.getenv only supported in Java 1.5, properties have to be set in shell script
        // Map map = java.lang.System.getenv();
        // HashMap myMap = new HashMap(map);
        Properties props = System.getProperties();
        HashMap myMap = new HashMap(props);
        return myMap;
    }

    static public void dumpStringArray(String[] myStringArray) {
        for (int i = 0; i < myStringArray.length; i++) {
            System.out.println(myStringArray[i]);
        }
    }

    static public void dumpFile(String baseFileName, String dumpFileName) {
        Vector fileContent = readCharFileVector(baseFileName);
        saveCharFileVector(dumpFileName, fileContent);
    }

    static public Vector readCharFileVector(String fileName) {
        Vector fileContent = new Vector();

        File file = new File(fileName);
        if ( file.exists()) {
            try {
                FileInputStream fs = new FileInputStream(file);
                BufferedReader bs = new BufferedReader(new InputStreamReader(fs));
                String zeile;
                while((zeile = bs.readLine())!=null) {
                    fileContent.addElement(zeile);
                }
            }
            catch (IOException e) {
                System.out.println(e);
            }
        } else {
            System.out.println( "Error: File not found: " +  fileName);
        }

        return fileContent;
    }


    static public void saveCharFileVector(String fileName, Vector fileContent) {
        FileWriter fw = null;
        try
        {
            fw = new FileWriter(fileName);
            String fileContentStr = "";
            for (int i = 0; i < fileContent.size() ; i++) {
                fileContentStr = fileContentStr + fileContent.get(i) + "\n";
                // System.out.println(fileContent.get(i));
            }
            fw.write(fileContentStr);
        }
        catch ( IOException e ) {
            System.out.println( "Could not create file: " +  fileName);
        }
        finally {
            try {
                if ( fw != null ) fw.close();
            } catch (IOException e) {}
        }
    }

    static public void copyAllFiles(File source, File dest) {
        File[] file = source.listFiles();
        if (file != null) {
            for (int i = 0; i < file.length; i++) {
                copy(file[i].getPath(), dest.getPath());
            }
        }
    }

    static public void copyAllFiles(File source, File dest, String ext) {
        File[] file = source.listFiles(new FileExtensionFilter(ext));
        if (file != null) {
            for (int i = 0; i < file.length; i++) {
                copy(file[i].getPath(), dest.getPath());
            }
        }
    }

    // second parameter can be a complete file name or an existing directory
    static public boolean copy(String source, String dest) {

        // is the second parameter a file name or a directory?
        File dir = new File(dest);
        if ( dir.isDirectory() ) {
            File sourceFile = new File(source);
            String fileName = sourceFile.getName();
            File destFile = new File(dest, fileName);
            dest = destFile.getPath();
        }

        boolean file_copied = false;
        FileInputStream fis;
        BufferedInputStream bis;
        FileOutputStream fos;
        BufferedOutputStream bos;
        byte[] b;
        try {
            fis = new FileInputStream(source);
            fos = new FileOutputStream(dest);
        } catch (FileNotFoundException ex) {
            throw new Error("File not found");
        }
        // put file into buffer
        bis = new BufferedInputStream(fis);
        bos = new BufferedOutputStream(fos);
        try { // read file, write and close
            b = new byte[bis.available()];
            bis.read(b);
            bos.write(b);
            bis.close();
            bos.close();
            file_copied = true;
        } catch (IOException e) {
            System.out.println("Dateien wurden nicht kopiert!");
        }

        return file_copied;
    }

    static public boolean deleteFile(File file) {
        boolean success = false;
        if ( file.exists() && file != null ) {
            success = file.delete();
        }
        return success;
    }

    static public boolean createDirectory(File dir) throws SecurityException {
        boolean created = false;
        try {
            created = dir.mkdirs();
        }
        catch (SecurityException ex) {
            throw ex;
        }

        return created;
    }

    static public void removeDirectory(File dir) {
        if ( dir.exists() && dir.isDirectory() ) {
            File[] file = dir.listFiles();
            if (file != null) {
                for (int i = 0; i < file.length; i++) {
                    deleteFile(file[i]);
                }
            }
            dir.delete();
        }
    }

    static public boolean logModuleStates() {
        boolean logStates = false;
        // System.getenv only supported in Java 1.5, property set in shell script
        // String logStatesEnv = System.getenv("LOG_MODULE_STATES");
        String logStatesEnv = System.getProperty("LOG_MODULE_STATES");

        if ( logStatesEnv != null ) {
            logStates = true;
        }

        return logStates;
    }

    static public void setUnixPrivileges(String fileName, String unixRights) {
        // String command = "chmod " + unixRights + " " + fileName;
        String[] commandArray = new String[3];
        commandArray[0] = "chmod";
        commandArray[1] = unixRights;
        commandArray[2] = fileName;
        int value = ExecuteProcess.executeProcessReturnValue(commandArray);
    }

    static public void setUnixPrivilegesDirectory(File directoryName, String ext, String unixRights) {
        File[] file = directoryName.listFiles(new FileExtensionFilter(ext));
        if (file != null) {
            for (int i = 0; i < file.length; i++) {
                setUnixPrivileges(file[i].getPath(), unixRights);
            }
        }
    }

    static public int calculateDiscSpace(String directory) {
        String command = "df -k " + directory;
        String[] commandArray = new String[3];
        commandArray[0] = "df";
        commandArray[1] = "-k";
        commandArray[2] = directory;

        int size = 0;
        Vector returnVector = new Vector();
        Vector returnErrorVector = new Vector();
        int returnValue = ExecuteProcess.executeProcessReturnVector(commandArray, returnVector, returnErrorVector);
        if ( returnValue == 0) {
            int max = returnVector.size();
            if ( max > 0 ) {
                String returnLine = (String) returnVector.get(max-1);

                // The fourth value is the available disc space (if the first value is a path)
                // Otherwise it can also be the third value, if the first is not a path.
                // If the first value is not a path, the string starts with white spaces.

                int position = 3;
                if ( returnLine.startsWith(" ")) {
                    position = 2;
                }

                returnLine = returnLine.trim();
                String[] returnArray = returnLine.split("\\s+");

                if ( returnArray.length > 3 ) {
                    String sizeString = returnArray[position];

                    // Converting from String to int
                    size = Integer.parseInt(sizeString);
                }
            }
        }

        return size;
    }

}
