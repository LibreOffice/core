package org.openoffice.setup.InstallerHelper;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.Converter;
import org.openoffice.setup.Util.ExecuteProcess;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Vector;public class LinuxHelper {

    public LinuxHelper() {
        super();
    }

    private void getPackageNamesContent(PackageDescription packageData, Vector packageNames) {
        if (( packageData.getPackageName() != null ) && ( ! packageData.getPackageName().equals("")))  {
            packageNames.add(packageData.getPackageName() + "=" + packageData.getFullPackageName());
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            getPackageNamesContent(child, packageNames);
        }
    }

    private String getPackageNameFromRpm(String rpmFileName, InstallData installData) {
        String fullPackageName = null;
        String packagePath = installData.getPackagePath();

        File rpmFile = new File(packagePath, rpmFileName);
        if ( rpmFile.exists() ) {
            String rpmCommand = "rpm -qp " + rpmFile.getPath();
            String[] rpmCommandArray = new String[3];
            rpmCommandArray[0] = "rpm";
            rpmCommandArray[1] = "-qp";
            rpmCommandArray[2] = rpmFile.getPath();

            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(rpmCommandArray, returnVector, returnErrorVector);
            String returnString = (String) returnVector.get(0);

            String log = rpmCommand + "<br><b>Returns: " + returnString + "</b><br>";
            LogManager.addCommandsLogfileComment(log);

            fullPackageName = returnString;

        } else {
            System.err.println("Error: Could not find file " + rpmFile.getPath());
        }

        return fullPackageName;
    }

    private boolean checkPackageExistence(String rpmFileName, InstallData installData) {
        boolean fileExists = false;
        String packagePath = installData.getPackagePath();

        File rpmFile = new File(packagePath, rpmFileName);
        if ( rpmFile.exists() ) {
            fileExists = true;
        }

        return fileExists;
    }

    private HashMap analyzeVersionString(String versionString) {

        // System.err.println("Version String: " + versionString);

        boolean errorOccured = false;

        Integer micro = null;
        Integer minor = null;
        Integer major = null;
        Integer release = null;

        String microString = null;
        String minorString = null;
        String majorString = null;
        String releaseString = null;

        int pos = versionString.lastIndexOf("_");  // this is a jre RPM (1.5.0_06)

        if ( pos > -1 ) {
            try {
                releaseString = versionString.substring(pos+1, versionString.length());
                versionString = versionString.substring(0, pos);
            } catch (IndexOutOfBoundsException ex) {
                System.err.println("Error: Could not get substring from " + versionString);
                errorOccured = true;
            }
            try {
                int releaseInt = Integer.parseInt(releaseString);
                release = new Integer(releaseInt);
            } catch (NumberFormatException ex) {
                System.err.println("Error: Could not convert " + releaseString + " to integer");
                errorOccured = true;
            }
        }

        // the standard analyzing mechanism

        pos = versionString.lastIndexOf(".");  // returns "-1", if not found

        if ( pos > -1 )
        {
            try {
                microString = versionString.substring(pos+1, versionString.length());
                versionString = versionString.substring(0, pos);
            } catch (IndexOutOfBoundsException ex) {
                System.err.println("Error: Could not get substring from " + versionString);
                errorOccured = true;
            }

            pos = versionString.lastIndexOf(".");
            if ( pos > -1 ) {
                try {
                    minorString = versionString.substring(pos+1, versionString.length());
                    majorString = versionString.substring(0, pos);
                } catch (IndexOutOfBoundsException ex) {
                    System.err.println("Error: Could not get substring from " + versionString);
                    errorOccured = true;
                }
                try {
                    int microInt = Integer.parseInt(microString);
                    int minorInt = Integer.parseInt(minorString);
                    int majorInt = Integer.parseInt(majorString);
                    micro = new Integer(microInt);
                    minor = new Integer(minorInt);
                    major = new Integer(majorInt);
                } catch (NumberFormatException ex) {
                    System.err.println("Error: Could not convert " + microString + "," +
                                       minorString + " or " + majorString + " to integer");
                    errorOccured = true;
                }
            }
        }

        if ( errorOccured ) {
            micro = null;
            minor = null;
            major = null;
            release = null;
        }

        HashMap hashRpm = new HashMap();

        hashRpm.put("micro", micro);
        hashRpm.put("minor", minor);
        hashRpm.put("major", major);
        hashRpm.put("release", release);

        // If one of this values is "null", procedure "compareTwoRpms" always delivers false.
        // This means, that the installed package is not older.

        // System.err.println("Analyzed: " + "micro: " + hashRpm.get("micro").toString() + " minor: " + hashRpm.get("minor").toString() + " major: " + hashRpm.get("major").toString());

        return hashRpm;
    }

    private HashMap analyzeReleaseString(HashMap hashRpm, String releaseString) {
        int release;

        try {
            release = Integer.parseInt(releaseString);
            Integer releaseObj = new Integer(release);
            hashRpm.put("release", releaseObj);
         }
        catch (NumberFormatException ex) {
            System.err.println("Error: Could not convert " + releaseString + " to integer");
        }

        return hashRpm;
    }

    private boolean compareTwoRpms(HashMap hash1, HashMap hash2) {
        boolean hash1IsOlder = false;

        if ( (! hash1.containsValue(null) ) && (! hash2.containsValue(null) )) {
            if ( ((Integer)hash1.get("major")).intValue() < ((Integer)hash2.get("major")).intValue() ) {
                hash1IsOlder = true;
            } else {
                if ( ((Integer)hash1.get("minor")).intValue() < ((Integer)hash2.get("minor")).intValue() ) {
                    hash1IsOlder = true;
                } else {
                    if ( ((Integer)hash1.get("micro")).intValue() < ((Integer)hash2.get("micro")).intValue() ) {
                        hash1IsOlder = true;
                    } else {
                        if ( ((Integer)hash1.get("release")).intValue() < ((Integer)hash2.get("release")).intValue() ) {
                            hash1IsOlder = true;
                        }
                    }
                }
            }
        }

        return hash1IsOlder;
    }

    public boolean compareVersionAndRelease(String versionString, String releaseString, PackageDescription packageData, boolean checkIfInstalledIsOlder) {
        // version and release are gotten from the rpm database. packageData contains
        // the information about the rpm, that shall be installed. It has to be installed,
        // if the installed product defined by version and release is older.
        // version is something like "2.0.3", release something like "164".
        // An exception is the jre package, where version is "1.5.0_06" and release "fcs".

        HashMap installedRpm = analyzeVersionString(versionString);
        if ( installedRpm.get("release") == null ) {
            installedRpm = analyzeReleaseString(installedRpm, releaseString);
        }

        // System.err.println("Package: " + packageData.getPackageName());

        // System.err.println("Installed RPM: " + "major: " + installedRpm.get("major").toString() +
        //                                       " minor: " + installedRpm.get("minor").toString() +
        //                                       " micro: " + installedRpm.get("micro").toString() +
        //                                       " release: " + installedRpm.get("release").toString());

        HashMap notInstalledRpm = analyzeVersionString(packageData.getPkgVersion());
        if ( notInstalledRpm.get("release") == null ) {
            notInstalledRpm = analyzeReleaseString(notInstalledRpm, packageData.getPkgRelease());
        }

        //  System.err.println("Not installed RPM: " + "major: " + notInstalledRpm.get("major").toString() +
        //                                            " minor: " + notInstalledRpm.get("minor").toString() +
        //                                            " micro: " + notInstalledRpm.get("micro").toString() +
        //                                            " release: " + notInstalledRpm.get("release").toString());

        boolean firstIsOlder = false;

        if ( checkIfInstalledIsOlder ) {
            firstIsOlder = compareTwoRpms(installedRpm, notInstalledRpm);
            //  System.err.println("Result: Installed RPM is older: " + firstIsOlder);
        } else {
            firstIsOlder = compareTwoRpms(notInstalledRpm, installedRpm);
            //  System.err.println("Result: Not installed RPM is older: " + firstIsOlder);
        }

        return firstIsOlder;
    }

    public void getLinuxPackageNamesFromRpmquery(PackageDescription packageData, InstallData installData) {

        if ((packageData.getPackageName() != null) && ( ! packageData.getPackageName().equals(""))) {

            boolean rpmExists = checkPackageExistence(packageData.getPackageName(), installData);

            if ( rpmExists ) {

                // Full package name not defined in xpd file
                if (( packageData.getFullPackageName() == null ) || ( packageData.getFullPackageName().equals(""))) {
                    // Now it is possible to query the rpm database for the packageName, if it is not defined in xpd file!
                    String fullPackageName = getPackageNameFromRpm(packageData.getPackageName(), installData);
                    if ( fullPackageName != null ) {
                        packageData.setFullPackageName(fullPackageName);
                    } else {
                        System.err.println("Error: Linux package name not defined in xpd file and could not be determined: "
                                + packageData.getPackageName());
                    }
                }
                packageData.setPkgExists(true);

            } else {
                packageData.setPkgExists(false);
            }

        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            getLinuxPackageNamesFromRpmquery(child, installData);
        }
    }

    public String getLinuxDatabasePath(InstallData data) {
        String databasePath = null;
        String installDir = data.getInstallDir();
        String databaseDir = SystemManager.getParentDirectory(installDir);
        String linuxDatabaseName = ".RPM_OFFICE_DATABASE";
        File databaseFile = new File(databaseDir, linuxDatabaseName);
        databasePath = databaseFile.getPath();
        return databasePath;
    }

    public void getLinuxFileInfo(PackageDescription packageData) {
        // analyzing a string like "openoffice-core01-2.0.3-159" as "name-version-release"
        InstallData data = InstallData.getInstance();
        if ( packageData.pkgExists() ) {
            if (( packageData.getFullPackageName() != null ) && ( ! packageData.getFullPackageName().equals(""))) {
                String longName = packageData.getFullPackageName();

                int pos = longName.lastIndexOf("-");
                if (data.isInstallationMode()) {
                    // not saving at uninstallation, because it can be updated without GUI installer
                    packageData.setPkgRelease(longName.substring(pos+1, longName.length()));
                }
                longName = longName.substring(0, pos);

                pos = longName.lastIndexOf("-");
                if (data.isInstallationMode()) {
                    // not saving at uninstallation, because it can be updated without GUI installer
                    packageData.setPkgVersion(longName.substring(pos+1, longName.length()));
                }
                packageData.setPkgRealName(longName.substring(0, pos));
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            getLinuxFileInfo(child);
        }

    }

    public void setFullPackageNameAtUninstall(PackageDescription packageData, HashMap packageNames) {

        if (( packageData.getPackageName() != null ) && ( ! packageData.getPackageName().equals("")))  {
            if (( packageData.getFullPackageName() == null ) || ( packageData.getFullPackageName().equals(""))) {
                String packageName = packageData.getPackageName();
                // Does this always exist? Should not be required!
                // But is there another way to get the packageNames, without this file?
                // During installation the packageNames can be determined by querying the rpm file
                // -> this is not possible during uninstallation
                String fullPackageName = (String) packageNames.get(packageName);
                packageData.setFullPackageName(fullPackageName);
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            setFullPackageNameAtUninstall(child, packageNames);
        }
    }

    public String getRelocationString(PackageDescription packageData, String packageName) {
        String relocationString = null;

        if ( packageData.isRelocatable() ) {
            // String rpmQuery = "rpm -qp --qf %{PREFIXES}=" + " " + packageName;
            String[] rpmQueryArray = new String[5];
            rpmQueryArray[0] = "rpm";
            rpmQueryArray[1] = "-qp";
            rpmQueryArray[2] = "--qf";
            rpmQueryArray[3] = "%{PREFIXES}=";
            rpmQueryArray[4] = packageName;

            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(rpmQueryArray, returnVector, returnErrorVector);
            relocationString = (String) returnVector.get(0);
        }

        return relocationString;
    }

    public void createPackageNameFileAtPostinstall(InstallData data, PackageDescription packageData) {

        // The file "packageNames" must not be an own database! It must be possible to install
        // and deinstall RPMs without this GUI installer. Therefore the file packageNames is
        // not always up to date. Nevertheless it makes the deinstallation faster, because of
        // all packages, whose "real" package name is not defined in xpd files (for example
        // "openoffice-core01-2.0.3-159.rpm" hat the "real" name "openoffice-core01" that is
        // used for deinstallation) this can be read in this file. Otherwise it would be
        // neccessary to determine the "real" name with a database question.
        // The version and release that are also stored in file "packageNames" must not be
        // used for deinstallation because they are probably not up to date.

        String destDirStr = data.getInstallDir();
        File uninstallDir = new File(destDirStr, data.getUninstallDirName());
        String fileName = "packageNames";
        File packageNamesFile = new File(uninstallDir, fileName);
        Vector packageNames = new Vector();
        getPackageNamesContent(packageData, packageNames);
        SystemManager.saveCharFileVector(packageNamesFile.getPath(), packageNames);
    }

    public HashMap readPackageNamesFile() {
        // package names are stored in file "packageNames" in data.getInfoRoot() directory
        String fileName = "packageNames";
        InstallData data = InstallData.getInstance();
        File dir = data.getInfoRoot();
        File file = new File(dir, fileName);
        Vector fileContent = SystemManager.readCharFileVector(file.getPath());
        HashMap map = Converter.convertVectorToHashmap(fileContent);
        return map;
    }

    public void saveModulesLogFile(InstallData data) {
        if ( data.logModuleStates() ) {
            Vector logContent = LogManager.getModulesLogFile();
            String destDirStr = data.getInstallDir();
            File uninstallDir = new File(destDirStr, data.getUninstallDirName());
            File modulesLogFile = new File(uninstallDir, "moduleSettingsLog.txt");
            SystemManager.saveCharFileVector(modulesLogFile.getPath(), logContent);
        }
    }

    public String fixInstallationDirectory(String installDir) {
        // inject a second slash to the last path segment to avoid rpm 3 concatenation bug
        int lastSlashPos = installDir.lastIndexOf('/');
        String sub1 = installDir.substring(0,lastSlashPos);
        String sub2 = installDir.substring(lastSlashPos);
        String fixedInstallDir = sub1 + "/" + sub2;
        // fixedInstallDir.replaceAll(" ", "%20");
        return fixedInstallDir;
    }

}
