package org.openoffice.netbeans.modules.office.options;

import java.util.Hashtable;
import java.util.Enumeration;
import java.io.File;
import java.io.IOException;

import org.openide.options.SystemOption;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;

import org.openoffice.idesupport.SVersionRCFile;

/** Options for something or other.
 *
 * @author tomaso
 */
public class OfficeSettings extends SystemOption {

    // private static final long serialVersionUID = ...;

    public static final String OFFICE_DIRECTORY = "OfficeDirectory";
    public static final String WARN_BEFORE_DOC_DEPLOY = "WarnBeforeDocDeploy";
    public static final String WARN_AFTER_DIR_DEPLOY = "WarnAfterDirDeploy";
    public static final String WARN_BEFORE_MOUNT = "WarnBeforeMount";

    protected void initialize() {
        super.initialize();
        setWarnBeforeDocDeploy(true);
        setWarnAfterDirDeploy(true);
        setWarnBeforeMount(true);

        SVersionRCFile sversion = new SVersionRCFile();

        try {
            Hashtable versions = sversion.getVersions();
            Enumeration enum = versions.elements();
            String path;

            while (enum.hasMoreElements()) {
                path = (String)enum.nextElement();
                File f = new File(path);

                if (f.exists()) {
                    setOfficeDirectory(path);
                    return;
                }
            }
        }
        catch (IOException ioe) {
            /* do nothing a default will be used */
        }

        // if no office version is found try a default value
        setOfficeDirectory(System.getProperty("user.home") +
            System.getProperty("file.separator") + "StarOffice6.0");
    }

    public String displayName() {
        return "Office Settings";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    public static OfficeSettings getDefault() {
        return (OfficeSettings)findObject(OfficeSettings.class, true);
    }

    public String getOfficeDirectory() {
        return (String)getProperty(OFFICE_DIRECTORY);
    }

    public void setOfficeDirectory(String path) {
        putProperty(OFFICE_DIRECTORY, path, true);
    }

    public boolean getWarnBeforeDocDeploy() {
        return ((Boolean)getProperty(WARN_BEFORE_DOC_DEPLOY)).booleanValue();
    }

    public void setWarnBeforeDocDeploy(boolean value) {
        putProperty(WARN_BEFORE_DOC_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnAfterDirDeploy() {
        return ((Boolean)getProperty(WARN_AFTER_DIR_DEPLOY)).booleanValue();
    }

    public void setWarnAfterDirDeploy(boolean value) {
        putProperty(WARN_AFTER_DIR_DEPLOY, new Boolean(value), true);
    }

    public boolean getWarnBeforeMount() {
        return ((Boolean)getProperty(WARN_BEFORE_MOUNT)).booleanValue();
    }

    public void setWarnBeforeMount(boolean value) {
        putProperty(WARN_BEFORE_MOUNT, new Boolean(value), true);
    }
}
