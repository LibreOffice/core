/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.ResourceManager;
import java.io.File;
import java.util.Vector;

public class InfoDir {

    private InfoDir() {
    }

    static private String copySourceFile(String fileName) {
        InstallData data = InstallData.getInstance();
        File jarFile = data.getJarFilePath();
        String destFile = null;

        if ( jarFile != null ) {
            String sourceDir = jarFile.getParent();
            File sourceFileFile = new File(sourceDir, fileName);
            String sourceFile = sourceFileFile.getPath();

            // String jarFileName = jarFile.getName();
            File destDir = new File(data.getInstallDefaultDir(), data.getProductDir());
            File destFileFile = new File(destDir, fileName);
            destFile = destFileFile.getPath();

            boolean success = SystemManager.copy(sourceFile, destFile);
        }

        return destFile;
    }

    static private void copyInstallDirectoryWithExtension(File destBaseDir, String subDirName, String fileExtension) {
        InstallData data = InstallData.getInstance();
        File sourceDir = data.getInfoRoot(subDirName);
        if ( sourceDir != null ) {
            File destDir = new File(destBaseDir, subDirName);
            destDir.mkdir();
            SystemManager.copyAllFiles(sourceDir, destDir, fileExtension);
        }
    }

    static private void copyInstallDirectoryWithExtension(File destBaseDir, String subDirName, String fileExtension, String unixRights) {
        InstallData data = InstallData.getInstance();
        File sourceDir = data.getInfoRoot(subDirName);
        if ( sourceDir != null ) {
            File destDir = new File(destBaseDir, subDirName);
            destDir.mkdir();
            SystemManager.copyAllFiles(sourceDir, destDir, fileExtension);
            SystemManager.setUnixPrivilegesDirectory(destDir, fileExtension, unixRights);
        }
    }

    static private void copyInstallDirectoryDoubleSubdir(File destBaseDir, String dir1, String dir2) {
        InstallData data = InstallData.getInstance();
        File sourceDir1 = data.getInfoRoot(dir1);
        File sourceDir = new File(sourceDir1, dir2);

        destBaseDir.mkdir();
        File destDir1 = new File(destBaseDir, dir1);
        destDir1.mkdir();
        File destDir = new File(destDir1, dir2);
        destDir.mkdir();

        SystemManager.copyAllFiles(sourceDir, destDir);
    }

    static private File createUninstallDir() {
        InstallData data = InstallData.getInstance();
        File baseDir = new File(data.getInstallDefaultDir(), data.getProductDir());
        baseDir = new File(baseDir, data.getUninstallDirName());
        baseDir.mkdir();
        return baseDir;
    }

    static private void copyGetUidSoFile(File dir) {
        InstallData data = InstallData.getInstance();
        String uidFileSource = data.getGetUidPath();
        if ( uidFileSource != null ) {
            // Copying the "getuid.so" file into installation
            String fileName = "getuid.so";
            File destFile = new File(dir, fileName);
            String uidFileDest = destFile.getPath();
            boolean success = SystemManager.copy(uidFileSource, uidFileDest);
            data.setGetUidPath(uidFileDest);
        }
    }

    static private void copyJreFile(File dir) {
        InstallData data = InstallData.getInstance();
        String jrefilename = System.getProperty("JRE_FILE");

        if ( jrefilename != null ) {
            // For Solaris, JRE_FILE can already contain the complete path.
            // Otherwise it contains only the filename
            File jreFile = new File(jrefilename);

            if ( ! jreFile.exists()) {
                jreFile = new File(data.getPackagePath(), jrefilename);
            }

            if ( jreFile.exists() ) {
                String jreFileSource = jreFile.getPath();
                File destDir = new File(dir, "jre");
                destDir.mkdir();
                String onlyFileName = jreFile.getName();
                File destFile = new File(destDir, onlyFileName);

                // In maintenance mode the file already exists
                if ( ! destFile.exists() ) {
                    String jreFileDest = destFile.getPath();
                    boolean success = SystemManager.copy(jreFileSource, jreFileDest);
                }
            }
        }
    }

    static private void moveAdminFiles(File dir) {
        InstallData data = InstallData.getInstance();

        if ( data.getAdminFileNameReloc() != null ) {
            File sourceFile = new File(data.getAdminFileNameReloc());
            String fileName = sourceFile.getName();
            File destFile = new File(dir, fileName);
            boolean success = SystemManager.copy(sourceFile.getPath(), destFile.getPath());
            data.setAdminFileNameReloc(destFile.getPath());
            sourceFile.delete();
        }

        if ( data.getAdminFileNameRelocNoDepends() != null ) {
            File sourceFile = new File(data.getAdminFileNameRelocNoDepends());
            String fileName = sourceFile.getName();
            File destFile = new File(dir, fileName);
            boolean success = SystemManager.copy(sourceFile.getPath(), destFile.getPath());
            data.setAdminFileNameRelocNoDepends(destFile.getPath());
            sourceFile.delete();
        }

        if ( data.getAdminFileNameNoReloc() != null ) {
            File sourceFile = new File(data.getAdminFileNameNoReloc());
            String fileName = sourceFile.getName();
            File destFile = new File(dir, fileName);
            boolean success = SystemManager.copy(sourceFile.getPath(), destFile.getPath());
            data.setAdminFileNameNoReloc(destFile.getPath());
            sourceFile.delete();
        }

        if ( data.getAdminFileNameNoRelocNoDepends() != null ) {
            File sourceFile = new File(data.getAdminFileNameNoRelocNoDepends());
            String fileName = sourceFile.getName();
            File destFile = new File(dir, fileName);
            boolean success = SystemManager.copy(sourceFile.getPath(), destFile.getPath());
            data.setAdminFileNameNoRelocNoDepends(destFile.getPath());
            sourceFile.delete();
        }
    }

    static private void createInfoFile(File dir) {
        Vector fileContent = new Vector();
        String line = null;
        InstallData data = InstallData.getInstance();

        line = "PackagePath=" + data.getPackagePath();
        fileContent.add(line);
        line = "InstallationPrivileges=" + data.getInstallationPrivileges();
        fileContent.add(line);
        line = "AdminFileReloc=" + data.getAdminFileNameReloc();
        fileContent.add(line);
        line = "AdminFileRelocNoDepends=" + data.getAdminFileNameRelocNoDepends();
        fileContent.add(line);
        line = "AdminFileNoReloc=" + data.getAdminFileNameNoReloc();
        fileContent.add(line);
        line = "AdminFileNoRelocNoDepends=" + data.getAdminFileNameNoRelocNoDepends();
        fileContent.add(line);
        line = "InstallationDir=" + data.getInstallDir();
        fileContent.add(line);
        line = "DatabasePath=" + data.getDatabasePath();
        fileContent.add(line);
        line = "GetUidFile=" + data.getGetUidPath();
        fileContent.add(line);

        String infoFileName = "infoFile";
        File infoFile = new File(dir, infoFileName);
        SystemManager.saveCharFileVector(infoFile.getPath(), fileContent);
    }

    static private void removeSpecialFiles() {
        InstallData data = InstallData.getInstance();
        File jarFile = data.getJarFilePath();
        SystemManager.deleteFile(jarFile);

        String jarFilePath = jarFile.getParent();
        File setupFile = new File(jarFilePath, "setup");
        SystemManager.deleteFile(setupFile);

        if ( ! data.getAdminFileNameReloc().equals("null") ) {
            SystemManager.deleteFile(new File(data.getAdminFileNameReloc()));
        }

        if ( ! data.getAdminFileNameRelocNoDepends().equals("null") ) {
            SystemManager.deleteFile(new File(data.getAdminFileNameRelocNoDepends()));
        }

        if ( ! data.getAdminFileNameNoReloc().equals("null") ) {
            SystemManager.deleteFile(new File(data.getAdminFileNameNoReloc()));
        }

        if ( ! data.getAdminFileNameNoRelocNoDepends().equals("null") ) {
            SystemManager.deleteFile(new File(data.getAdminFileNameNoRelocNoDepends()));
        }

        if ( ! data.getGetUidPath().equals("null") ) {
            SystemManager.deleteFile(new File(data.getGetUidPath()));
        }
    }

    static private void removeInforootSubdir(String dir1, String dir2) {
        InstallData data = InstallData.getInstance();
        File subdir1 = data.getInfoRoot(dir1);
        File subdir2 = new File(subdir1, dir2);
        if (subdir2 != null) {
            if ( subdir2.exists() ) {
                SystemManager.removeDirectory(subdir2);
            }
        }
    }

    static private void removeInforootSubdir(String dir) {
        InstallData data = InstallData.getInstance();
        File subdir = data.getInfoRoot(dir);
        if (subdir != null) {
            if ( subdir.exists() ) {
                SystemManager.removeDirectory(subdir);
            }
        }
    }

    static private void removeInforoot() {
        InstallData data = InstallData.getInstance();
        SystemManager.removeDirectory(data.getInfoRoot());
    }

    static public void prepareUninstallation() {
        // additional tasks for uninstallation
        // Directory destDir has to exist!
        InstallData data = InstallData.getInstance();
        File destDir = new File(data.getInstallDefaultDir(), data.getProductDir());
        boolean directoryExists = true;

        if ( ! destDir.exists() ) {
            try {
                directoryExists = SystemManager.create_directory(destDir.getPath());
            }
            catch (SecurityException ex) {
                String message = ResourceManager.getString("String_ChooseDirectory_No_Write_Access") + ": " + destDir.getPath();
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
            }
        }

        if ( directoryExists ) {
            String setupPath = copySourceFile("setup");
            SystemManager.setUnixPrivileges(setupPath, "775");
            File jarFile = data.getJarFilePath();
            copySourceFile(jarFile.getName());

            File uninstallDir = createUninstallDir();
            copyInstallDirectoryWithExtension(uninstallDir, "xpd", "xpd");
            copyInstallDirectoryWithExtension(uninstallDir, "html", "html");
            copyInstallDirectoryWithExtension(uninstallDir, "images", "gif");
            copyInstallDirectoryDoubleSubdir(uninstallDir, "html", "images");
            copyGetUidSoFile(uninstallDir);
            copyJreFile(uninstallDir);
            moveAdminFiles(uninstallDir);
            createInfoFile(uninstallDir);
        }
    }

    static public void removeUninstallationFiles() {
        // removing selected File
        removeSpecialFiles();
        // removing directories html/images, html and xpd
        removeInforootSubdir("html", "images");
        removeInforootSubdir("html");
        removeInforootSubdir("xpd");
        removeInforootSubdir("images");
        removeInforootSubdir("jre");
        removeInforoot();
    }

}
