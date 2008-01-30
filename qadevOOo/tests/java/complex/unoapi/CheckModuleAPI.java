/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CheckModuleAPI.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 13:15:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/** complex tests to check the UNO-API
 **/
package complex.unoapi;

// imports
import base.java_complex;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import helper.ProcessHandler;
import com.sun.star.lang.XMultiServiceFactory;
import helper.ComplexDescGetter;
import helper.OfficeWatcher;
import helper.ProcessHandler;
import java.io.File;
import java.io.FileFilter;
import java.io.PrintWriter;
import java.util.Vector;
import share.DescEntry;
import util.PropertyName;
import util.utils;

/**
 * This Complex Test will test the UNO-API by calling dmake in <B>$MODULE</B>/qa/unoapi<p>
 * This test depends on some requirments:
 * The <B>TestJob</B> is <b>-o complex.unoapi.CheckModuleAPI::module(<CODE>MODULE</CODE>)</b><p> where <CODE>MODULE</CODE>
 * could be the following:<p>
 * <ul>
 *  <li><b>all</b>  iterates over <CODE>SRC_ROOT</CODE> and call <CODE>dmake</CODE> in all qa/unoapi folder</li><p>
 *  <li><b>$module</b>  call dmake in $module/qa/unoapi folder</li><p>
 *  <li><b>$module1,$module2,...</b>    call dmake in $module1/qa/unoapi folder then in $module2/qa/unoapi folder and so on</li><p>
 * </ul><p>
 *
 * Also you have to fill the following parameter:
 * <ul>
 *  <li><b>OOO_ENVSET</b>: this parameter must ponit to the script to create a build environment like
 *                  <b>$SRC_ROOT/SolarisX86Env.Set.sh</b> or <b>%SRC_ROOT\winenv.set.sh</b>
 *                  In case you are able to use <b>setsolar</b> or <b>setcws</b> just type <b>setsolar</b> as value.
 *                  </li><p>
 *
 *  <li><b>COMP_ENV</b>:   In case you use <b>setsolar</b> you need to fill parameter <b>COMP_ENV</b> with accordant
 *                  value like 'unxsols4' or 'wntmsci10'</li><p>
 *
 *  <li><b>SHELL</b>:      fill this parameter with a shell which can start <CODE>OOO_ENVSET</CODE></li>
 * </ul>
 *
 */
public class CheckModuleAPI extends ComplexTestCase {

    String mSRC_ROOT = null;
    String mCompiler = null;
    boolean mIsInitialized = false;
    boolean mContinue = false;
    boolean mDebug = false;

    public void before() {

        if (!mIsInitialized) {
            mIsInitialized = true;
            boolean error = false;
            String msg = "\nERROR: the following parameter must be set before executing the test:\n";
            String envSet = (String) param.get(PropertyName.OOO_ENVSET);
            if (envSet == null) {
                msg += PropertyName.OOO_ENVSET + "\n\tThis parameter must ponit to the script to create a build " + "environment like '$SRC_ROOT/SolarisX86Env.Set.sh' or '%SRC_ROOT\\winenv.set.sh'\n" + "In case you are able to use \'setsolar\' or \'setcws\' just type \'setsolar\' value.\n ";
                error = true;
            }
            String shell = (String) param.get(PropertyName.SHELL);
            if (shell == null) {
                msg += PropertyName.SHELL + "\n\tFill this parameter with a shell which can start 'OOO_ENVSET'" + "\n\t/bin/tcsh c:\\myShell\\myShell.exe\n";
                error = true;
            }
            makeEnvironment();
            if (mSRC_ROOT == null) {
                msg += PropertyName.SRC_ROOT + "\n\tTry to fill this parameter with content of '$SRC_ROOT' like:\n\t" + PropertyName.SRC_ROOT + "=`$SRC_ROOT`\n";
                error = true;
            }
            mDebug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);
            assure(msg, !error, false);
        }
    }

    /**
     * let API tests run.
     * @param module
     */
    public void checkModule(String module) {
        log.println(utils.getDateTime() + ": start testing module '" + module + "'");

        log.println("start new Office instance...");
        OfficeProvider officeProvider = new OfficeProvider();
        log.println("Receiving the ServiceManager of the Office ");
        XMultiServiceFactory msf = (XMultiServiceFactory) officeProvider.getManager(param);
        assure("couldnot get ServiceFarcotry", msf != null, mContinue);
        param.put("ServiceFactory", msf);

        String sep = System.getProperty("file.separator");
        String sUnoapi = getModulePath(module);
        File fUnoapi = new File(sUnoapi);
        String sMakeFile = sUnoapi + sep + "makefile.mk";
        File fMakeFile = new File(sMakeFile);
        assure("ERROR: could not find makefile: '" + sMakeFile + "'", fMakeFile.exists(), mContinue);

        String[] cmdLines = getCmdLines(sUnoapi);
        ProcessHandler procHdl = runShellCommand(cmdLines, fUnoapi, false);
        if (mDebug) {
            log.println("---> Output of dmake:");
            log.println(procHdl.getOutputText());
            log.println("<--- Output of dmake file:");
            log.println("---> Error output of dmake file");
            log.println(procHdl.getErrorText());
            log.println("<--- Error output of dmake file");
        }
        assure("module failed", verifyOutput(procHdl.getOutputText()), mContinue);

        log.println("kill existing office...");
        try {
            officeProvider.closeExistingOffice(param, true);
        } catch (java.lang.UnsatisfiedLinkError exception) {
            log.println("Office seems not to be running");
        }

    }

    private String[] getCmdLines(String sUnoapi) {

        String shell = (String) param.get(PropertyName.SHELL);
        String envcmd = getEnvCmd();
        String[] cmdLines = null;
        String platform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        log.println("prepare command for platform " + platform);
        if (platform.equals(PropertyName.WNTMSCI)) {
            cmdLines = new String[]{shell, "/C ", envcmd + " ^ cdd " + sUnoapi + "^ dmake"};
        } else {
            cmdLines = new String[]{shell, "-c ", envcmd + " ; cd " + sUnoapi + "; dmake"};
        }
        return cmdLines;
    }

    private String getEnvValue(String line) {
        String[] split = line.split("=");
        return split[1];
    }

    private String getModulePath(String module) {

        String sUnoapi = null;
        String sep = System.getProperty("file.separator");
        File srcRoot = new File(mSRC_ROOT);

        FolderFilter qaFilter = new FolderFilter(module);
        File[] moduleTree = srcRoot.listFiles(qaFilter);
        if (mDebug) {
            log.println("moduleTree: " + moduleTree[0].getAbsolutePath());
        }
        if (moduleTree != null) {
            sUnoapi = moduleTree[0].getAbsolutePath() + sep + "qa" + sep + "unoapi";
        }
        return sUnoapi;
    }

    private String getSetSolarCmd() {
        String cmd = null;

        String version = (String) param.get(PropertyName.VERSION);

        if (version.startsWith("cws_")) {
            cmd = "setcws " + version.substring(4, version.length());
        } else {
            mCompiler = (String) param.get(PropertyName.COMP_ENV);
            assure("If you use 'setsolar' you need to fill parameter 'COMP_ENV' with accordant " +
                   "value like 'unxsols4' or wntmsci10", mCompiler != null);

            String[] versions = version.split("_");

            cmd = "setsolar -" + versions[0] + " -ver " + versions[1] + " -jdk14  -pro " + mCompiler;
        }
        return cmd;
    }

    private String getEnvCmd() {
        String envSet = (String) param.get(PropertyName.OOO_ENVSET);
        String cmd = null;
        String platform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        log.println("prepare command for platform " + platform);

        if (envSet.equals("setsolar")) {
            cmd = getSetSolarCmd();
            if (platform.equals(PropertyName.WNTMSCI)) {
                cmd = "call " + cmd;
            }
        } else {
            cmd = "source " + envSet;
        }
        return cmd;
    }

    private void makeEnvironment() {

        String cmd = getEnvCmd();

        String shell = (String) param.get(PropertyName.SHELL);
        String platform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        String[] cmdLines = null;

        if (platform.equals(PropertyName.WNTMSCI)) {
            cmdLines = new String[]{shell, "/C ", cmd + " ^ echo SRC_ROOT=%SRC_ROOT"};
        } else {
            cmdLines = new String[]{shell, "-c ", cmd + " ; echo \"SRC_ROOT=$SRC_ROOT\""};
        }

        ProcessHandler procHdl = runShellCommand(cmdLines, null, true);
        if (mDebug) {
            log.println("---> Output of command:");
            log.println(procHdl.getOutputText());
            log.println("<--- Output of command:");
            log.println("---> Error output of command");
            log.println(procHdl.getErrorText());
            log.println("<--- Error output of command");
        }
        String output = procHdl.getOutputText();
        String[] outs = output.split("\n");

        for (int i = 0; i < outs.length; i++) {
            String line = outs[i];
            if (line.startsWith("SRC_ROOT")) {
                mSRC_ROOT = getEnvValue(line);
            }
        }
    }

    private ProcessHandler runShellCommand(String[] cmdLines, File workDir, boolean shortWait) {

        boolean changedText = true;
        int count = 0;
        String memText = "";
        ProcessHandler procHdl = new ProcessHandler(cmdLines, (PrintWriter) log, workDir);
        procHdl.executeAsynchronously();
        int timeOut = 0;

        if (shortWait) {
            timeOut = 3000;
        } else {
            timeOut = param.getInt(PropertyName.TIME_OUT) / 2;
        }

        OfficeWatcher ow = (OfficeWatcher) param.get(PropertyName.OFFICE_WATCHER);

        while (changedText && !procHdl.isFinished()) {
            count++;
            if (ow != null) {
                ow.ping();
            }
            log.println("CheckModuleAPI: waiting... " + count);
            utils.shortWait(timeOut);
            if (ow != null) {
                ow.ping();
            }
            // check for changes in the output stream. If there are no changes, the process maybe hangs
            if (procHdl.getOutputText().equals(memText)) {
                changedText = false;
            }
            memText = procHdl.getOutputText();
        }

        if (!procHdl.isFinished()) {
            log.println("Process ist not finished but there are no changes in output stream.");
        }
        return procHdl;
    }

    private boolean verifyOutput(String output) {

        log.println("verify output...");
        boolean ok = false;
        String[] outs = output.split("\n");

        for (int i = 0; i < outs.length; i++) {
            String line = outs[i];
            if (line.matches("[0-9]+? of [0-9]+? tests failed")) {
                log.println("mached line: " + line);
                if (line.matches("0 of [0-9]+? tests failed")) {
                    ok = true;
                    log.println("Module passed OK");
                } else {
                    log.println("Module passed FAILED");
                }
            }
        }
        return ok;
    }

    private String[] getModulesFromSourceRoot() {
        log.println("**** run module tests over all modules ****");

        log.println("search for qa/unoapi foldres in all modules based in ");
        log.println("'" + mSRC_ROOT + "'");

        Vector moduleNames = new Vector();
        File sourceRoot = new File(mSRC_ROOT);
        File[] sourceTree = sourceRoot.listFiles();

        assure("Could not find any files in SOURCE_ROOT=" + mSRC_ROOT, sourceTree != null, false);

        for (int i = 0; i < sourceTree.length; i++) {
            File moduleName = sourceTree[i];
            FolderFilter qaFilter = new FolderFilter("qa");
            File[] qaTree = moduleName.listFiles(qaFilter);
            if (qaTree != null) {
                for (int j = 0; j < qaTree.length; j++) {
                    File qaFolder = qaTree[j];
                    FolderFilter apiFilter = new FolderFilter("unoapi");
                    File[] apiTree = qaFolder.listFiles(apiFilter);
                    if (apiTree != null && apiTree.length > 0) {
                        moduleNames.add(moduleName.getName());
                    }
                }
            }
        }

        String[] names = (String[]) moduleNames.toArray( new String[0] );
        return names;
    }

    public void module(String module) {

        String[] checkModules = null;
        ComplexDescGetter desc = new ComplexDescGetter();
        DescEntry entry = null;
        module = module.trim();

        /* all: check all modules which contains a qa/unoapi folder
        auto: check all modules which contains a qa/unoapi folder except the module is not added
         */
        if (module.equals("all")) {

            String[] names = getModulesFromSourceRoot();
            checkModules = new String[names.length];

            for (int i = 0; i < names.length; i++) {
                // if a module is not added to a cws it contains a dot in its name (forms.lnk)
                if (names[i].indexOf(".") != -1) {
                    checkModules[i] = "checkModule(" + names[i].substring(0, names[i].indexOf(".")) + ")";
                } else {
                    checkModules[i] = "checkModule(" + names[i] + ")";
                }
            }
        } else {
            // list of modules to test: (sw,sc,sd)
            if (module.indexOf(",") != -1) {
                String[] names = module.split(",");
                checkModules = new String[names.length];
                for (int i = 0; i < names.length; i++) {
                    String trim = names[i].trim();
                    checkModules[i] = "checkModule(" + trim + ")";
                }
            } else {
                checkModules = new String[]{"checkModule(" + module + ")"};
            }
        }
        entry = desc.createTestDesc("complex.unoapi.CheckModuleAPI", "complex.unoapi.CheckModuleAPI", checkModules, log);

        java_complex complex = new java_complex();

        log.println("********** start test *************");
        boolean result = complex.executeTest(param, new DescEntry[]{entry});
        log.println("********** end test *************");

        assure("CheckModuleAPI.module(" + module + ") PASSED.FAILED", result);
    }

    public String[] getTestMethodNames() {
        return new String[]{"module(all)"};
    }

    class FolderFilter implements FileFilter {

        private String mFolderName;

        public FolderFilter(String folderName) {
            mFolderName = folderName;
        }

        public boolean accept(File pathname) {

            boolean found = false;
            if (pathname.isDirectory()) {
                if (pathname.getName().equals(mFolderName)) {
                    found = true;
                } else if (pathname.getName().equals(mFolderName + ".lnk")) {
                    found = true;
                }
            }
            return found;
        }
    }
}
