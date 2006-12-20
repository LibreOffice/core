package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
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
            File destDir = new File(data.getInstallRoot(), data.getInstallDir());
            File destFileFile = new File(destDir, fileName);
            destFile = destFileFile.getPath();

            boolean success = SystemManager.copy(sourceFile, destFile);
        }

        return destFile;
    }

    static private void copyInstallDirectoryWithExtension(File destBaseDir, String subDirName, String fileExtension) {
        InstallData data = InstallData.getInstance();
        File sourceDir = data.getInfoRoot(subDirName);
        File destDir = new File(destBaseDir, subDirName);
        destDir.mkdir();
        SystemManager.copyAllFiles(sourceDir, destDir, fileExtension);
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
        File baseDir = new File(data.getInstallRoot(), data.getInstallDir());
        File uninstallDir = new File(baseDir, data.getUninstallDirName());
        uninstallDir.mkdir();
        return uninstallDir;
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

        if ( data.getAdminFileNameNoReloc() != null ) {
            File sourceFile = new File(data.getAdminFileNameNoReloc());
            String fileName = sourceFile.getName();
            File destFile = new File(dir, fileName);
            boolean success = SystemManager.copy(sourceFile.getPath(), destFile.getPath());
            data.setAdminFileNameNoReloc(destFile.getPath());
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
        line = "AdminFileNoReloc=" + data.getAdminFileNameNoReloc();
        fileContent.add(line);
        line = "InstallationDir=" + data.getInstallDir();
        fileContent.add(line);
        line = "InstallationRoot=" + data.getInstallRoot();
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

        if ( ! data.getAdminFileNameNoReloc().equals("null") ) {
            SystemManager.deleteFile(new File(data.getAdminFileNameNoReloc()));
        }

        if ( ! data.getGetUidPath().equals("null") ) {
            SystemManager.deleteFile(new File(data.getGetUidPath()));
        }
    }

    static private void removeInforootSubdir(String dir1, String dir2) {
        InstallData data = InstallData.getInstance();
        File subdir1 = data.getInfoRoot(dir1);
        File subdir2 = new File(subdir1, dir2);
        SystemManager.removeDirectory(subdir2);
    }

    static private void removeInforootSubdir(String dir) {
        InstallData data = InstallData.getInstance();
        File subdir = data.getInfoRoot(dir);
        SystemManager.removeDirectory(subdir);
    }

    static private void removeInforoot() {
        InstallData data = InstallData.getInstance();
        SystemManager.removeDirectory(data.getInfoRoot());
    }

    static public void prepareUninstallation() {
        // additional tasks for uninstallation
        String setupPath = copySourceFile("setup");
        SystemManager.setUnixPrivileges(setupPath, "775");
        InstallData data = InstallData.getInstance();
        File jarFile = data.getJarFilePath();
        copySourceFile(jarFile.getName());

        File uninstallDir = createUninstallDir();
        copyInstallDirectoryWithExtension(uninstallDir, "xpd", "xpd");
        copyInstallDirectoryWithExtension(uninstallDir, "html", "html");
        copyInstallDirectoryWithExtension(uninstallDir, "images", "gif");
        copyInstallDirectoryDoubleSubdir(uninstallDir, "html", "images");
        copyGetUidSoFile(uninstallDir);
        moveAdminFiles(uninstallDir);
        createInfoFile(uninstallDir);
    }

    static public void removeUninstallationFiles() {
        // removing selected File
        removeSpecialFiles();
        // removing directories html/images, html and xpd
        removeInforootSubdir("html", "images");
        removeInforootSubdir("html");
        removeInforootSubdir("xpd");
        removeInforootSubdir("images");
        removeInforoot();
    }

}
