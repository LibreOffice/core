/*
 * ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckModuleAPI.java,v $
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
 * **********************************************************************
 */
/** complex tests to check the UNO-API
 **/
package complex.unoapi;

// imports
import base.java_complex;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import helper.ParameterNotFoundException;
import helper.ProcessHandler;
import com.sun.star.lang.XMultiServiceFactory;
import helper.BuildEnvTools;
import helper.ComplexDescGetter;
import helper.CwsDataExchangeImpl;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
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
 *  <li><b>SHELL</b>:      fill this parameter with a shell</li>
 * </ul>
 *
 */
public class CheckModuleAPI extends ComplexTestCase {

    private static String mSRC_ROOT = null;
    private static boolean mIsInitialized = false;
    private final static boolean mContinue = true;
    private static boolean mDebug = false;
    private static BuildEnvTools bet = null;

    /**
     * Initialize the test environment.
     * This method checks for all neccesarry parameter and exit if not all parameter are set.
     *
     * Further this method starts an office instance and gets the office some more time to start. This is because
     * some freshly installed offices don not have such a user tree. The office will create it on its first start,
     * but this will take some time.
     * Note: This funktionality is only reasonable with parameter <CODE>-noOffice true</CODE>
     */
    public void before() {

        if (!mIsInitialized) {
            mIsInitialized = true;

            try {

                bet = new BuildEnvTools(param, log);

            } catch (ParameterNotFoundException ex) {
                this.failed(ex.toString(), false);
            }

            mSRC_ROOT = bet.getSrcRoot();

            mDebug = param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        }
    }

    /**
     * let API tests run.
     * @param module
     */
    public void checkModule(String module) {
        log.println(utils.getDateTime() + ": start testing module '" + module + "'");

        log.println(utils.getDateTime() + "start new Office instance...");
        final OfficeProvider officeProvider = new OfficeProvider();
        log.println(utils.getDateTime() + "Receiving the ServiceManager of the Office ");
        final XMultiServiceFactory msf = (XMultiServiceFactory) officeProvider.getManager(param);
        assure("couldnot get ServiceFarcotry", msf != null, mContinue);
        param.put("ServiceFactory", msf);

        final String sep = System.getProperty("file.separator");
        final String sUnoapi = getModulePath(module);
        final File fUnoapi = new File(sUnoapi);
        final String sMakeFile = sUnoapi + sep + "makefile.mk";
        final File fMakeFile = new File(sMakeFile);
        assure("ERROR: could not find makefile: '" + sMakeFile + "'", fMakeFile.exists(), mContinue);

        final String[] commands = getCommands(sUnoapi);

        final ProcessHandler procHdl = bet.runCommandsInEnvironmentShell(commands, fUnoapi, 0);
        log.println("exit code of dmake: " + procHdl.getExitCode());
        String test = procHdl.getOutputText();
        test = procHdl.getErrorText();
//        if (mDebug) {
//            log.println("---> Output of dmake:");
//            log.println(procHdl.getOutputText());
//            log.println("<--- Output of dmake:");
//            log.println("---> Error output of dmake:");
//            log.println(procHdl.getErrorText());
//            log.println("<--- Error output of dmake:");
//        }
        assure("module '" + module + "' failed", verifyOutput(procHdl.getOutputText()), mContinue);
        log.println(utils.getDateTime() + " module '" + module + "': kill existing office...");
        try {
            officeProvider.closeExistingOffice(param, true);
        } catch (java.lang.UnsatisfiedLinkError exception) {
            log.println("Office seems not to be running");
        }

    }

    private void addIfQaUnoApiFolderExist(File srcRoot, ArrayList moduleNames) {

        if (doesQaUnoApiFolderExist(srcRoot)) {
            moduleNames.add(srcRoot.getName());
        }
    }

    private boolean doesQaUnoApiFolderExist(File srcRoot) {
        final FolderFilter qaFilter = new FolderFilter("qa");
        final File[] qaTree = srcRoot.listFiles(qaFilter);
        if (qaTree != null) {
            for (int j = 0; j < qaTree.length; j++) {
                final File qaFolder = qaTree[j];
                final FolderFilter apiFilter = new FolderFilter("unoapi");
                final File[] apiTree = qaFolder.listFiles(apiFilter);
                if (apiTree != null && apiTree.length > 0) {
                    return true;
                }
            }
        }
        return false;
    }

    private String[] getAllModuleCommand() {
        String[] checkModules;

        final String[] names = getModulesFromSourceRoot();
        checkModules = getCheckModuleCommand(names);

        return checkModules;
    }

    private String[] getCheckModuleCommand(String[] names) {
        String[] checkModules;
        checkModules = new String[names.length];

        for (int i = 0; i < names.length; i++) {
            // if a module is not added to a cws it contains a dot in its name (forms.lnk)
            if (names[i].indexOf(".") != -1) {
                checkModules[i] = "checkModule(" + names[i].substring(0, names[i].indexOf(".")) + ")";
            } else {
                checkModules[i] = "checkModule(" + names[i] + ")";
            }
        }
        return checkModules;
    }

    private String[] getCommands(String sUnoapi) {

        String[] cmdLines = null;
        final String platform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        log.println("prepare command for platform " + platform);

        if (platform.equals(PropertyName.WNTMSCI)){
            if (param.getBool(PropertyName.CYGWIN)){
                cmdLines = new String[]{"cd `cygpath '" + sUnoapi.replaceAll("\\\\", "\\\\\\\\")+ "'`", "dmake"};
            } else{
            cmdLines = new String[]{"cdd " + sUnoapi, "dmake"};
            }
        } else {
                cmdLines = new String[]{"cd " + sUnoapi, "dmake"};
            }
        return cmdLines;
    }

    private String[] getCwsModuleCommand() {
        String[] checkModules;
        final String version = (String) param.get(PropertyName.VERSION);
        String[] names = null;
        if (version.startsWith("cws_")) {
            try {
                // cws version: all added modules must be tested
                final String cws = version.substring(4, version.length());
                final CwsDataExchangeImpl cde = new CwsDataExchangeImpl(cws, param, log);
                final ArrayList addedModules = cde.getModules();

                final ArrayList moduleNames = new ArrayList();
                Iterator iterator = addedModules.iterator();
                while (iterator.hasNext()) {
                    final File sourceRoot = new File(mSRC_ROOT + File.separator + (String) iterator.next());
                    addIfQaUnoApiFolderExist(sourceRoot, moduleNames);

                }
                names = (String[]) moduleNames.toArray(new String[0]);
            } catch (ParameterNotFoundException ex) {
                this.failed(ex.toString(), false);
            }


        } else {
            // major version: all modules must be tested
            names = getModulesFromSourceRoot();
        }
        checkModules = getCheckModuleCommand(names);

        return checkModules;
    }

    private String[] getDefinedModuleCommand(String module) {
        String[] checkModules = null;
        // list of modules to test: (sw,sc,sd)
        if (module.indexOf(",") != -1) {
            final String[] names = module.split(",");
            checkModules = new String[names.length];
            for (int i = 0; i < names.length; i++) {
                final String moduleName = names[i].trim();

                File sourceRoot = new File(mSRC_ROOT + File.separator + moduleName);
                if (!sourceRoot.exists()) {
                    sourceRoot = new File(mSRC_ROOT + File.separator + moduleName + ".lnk");
                }

                if (doesQaUnoApiFolderExist(sourceRoot)) {
                    checkModules[i] = "checkModule(" + moduleName + ")";
                }
            }
        } else {
            File sourceRoot = new File(mSRC_ROOT + File.separator + module);
            if (!sourceRoot.exists()) {
                sourceRoot = new File(mSRC_ROOT + File.separator + module + ".lnk");
            }
            if (doesQaUnoApiFolderExist(sourceRoot)) {
                checkModules = new String[]{"checkModule(" + module + ")"};
            }
        }
        return checkModules;
    }

    private String getModulePath(String module) {

        String sUnoapi = null;
        final String sep = System.getProperty("file.separator");
        final File srcRoot = new File(mSRC_ROOT);

        final FolderFilter qaFilter = new FolderFilter(module);
        final File[] moduleTree = srcRoot.listFiles(qaFilter);
        if (moduleTree != null) {
            if (mDebug) {
                log.println("moduleTree length:" + moduleTree.length);
                log.println("moduleTree: " + moduleTree[0].getAbsolutePath());
            }
            if (moduleTree != null) {
                sUnoapi = moduleTree[0].getAbsolutePath() + sep + "qa" + sep + "unoapi";
            }
        }
        return sUnoapi;
    }

    /**
    Some modules contains more the one project. This methods translates given project paramater to the
     *  correspind module name.
     *
     * fwk -> framework
     * fwl -> framework
     * sch -> chart2
     * sysmgr1 -> configmgr
     * cfgmgr2 -> configmgr
     * lnn -> lingu
     * lng -> linguistic
     * sfx -> sfx2
     * sm -> starmath
     */
    private String getTranslatedNames(String module) {

        final HashMap modulez = new HashMap();

        modulez.put("fwk", "framework");
        modulez.put("fwl", "framework");
        modulez.put("sch", "chart2");
        modulez.put("sysmgr1", "configmgr");
        modulez.put("cfgmag2", "configmgr");
        modulez.put("lnn", "lingu");
        modulez.put("lng", "linguistic");
        modulez.put("sfx", "sfx2");
        modulez.put("sm", "starmath");

        // it could the that the parameter looks like "fwk,fwl". This results in double "famework,framework".
        // The following loop correct this to only one "framework"

        final Set keys = modulez.keySet();
        final Iterator iterator = keys.iterator();
        while (iterator.hasNext()) {

            final String key = (String) iterator.next();
            final String value = (String) modulez.get(key);

            module = module.replaceAll(key, value);

            final int count = module.split(value).length;
            if (count > 2) {
                for (int i = 2; i < count; i++) {
                    module.replaceFirst("," + value, "");
                }

            }
        }
        return module;
    }

    private boolean verifyOutput(String output) {

        log.println("verify output...");
        boolean ok = false;
        final String[] outs = output.split("\n");

        for (int i = 0; i < outs.length; i++) {
            final String line = outs[i];
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

        if (!ok) {
            log.println("ERROR: could not find '0 of [0-9]+? tests failed' in output");
        }

        return ok;
    }

    private String[] getModulesFromSourceRoot() {
        log.println("**** run module tests over all modules ****");

        log.println("search for qa/unoapi foldres in all modules based in ");
        log.println("'" + mSRC_ROOT + "'");

        final ArrayList moduleNames = new ArrayList();
        final File sourceRoot = new File(mSRC_ROOT);
        final File[] sourceTree = sourceRoot.listFiles();

        assure("Could not find any files in SOURCE_ROOT=" + mSRC_ROOT, sourceTree != null, false);

        for (int i = 0; i < sourceTree.length; i++) {
            final File moduleName = sourceTree[i];
            addIfQaUnoApiFolderExist(moduleName, moduleNames);
        }

        final String[] names = (String[]) moduleNames.toArray(new String[0]);
        return names;
    }

    /**
     * This function generates a list of modules to test and call <CODE>checkModule</CODE> for every module.
     * <p>
     *
     * @param module names to test. This could be
     * <ul>
     * <li>a comma separated list of modules like 'sw,sc,sd'</li>
     * <li>'all' to test all modules </li>
     * <li>'auto' to check only modules which are added to the ChildWorkSpace</li>
     * </ul>
     */
    public void module(String module) {

        String[] checkModules;
        final ComplexDescGetter desc = new ComplexDescGetter();
        DescEntry entry = null;
        module = module.trim();

        /*
        all: check all modules which contains a qa/unoapi folder
        auto: check all modules which contains a qa/unoapi folder except the module is not added
         */
        if (module.equals("all")) {
            checkModules = getAllModuleCommand();
        } else if (module.equals("auto")) {
            checkModules = getCwsModuleCommand();
        } else {
            module = getTranslatedNames(module);
            checkModules = getDefinedModuleCommand(module);
        }

        if (checkModules != null && checkModules.length > 0) {

            entry = desc.createTestDesc("complex.unoapi.CheckModuleAPI", "complex.unoapi.CheckModuleAPI", checkModules,
                log);

            final java_complex complex = new java_complex();

            log.println("********** start test *************");
            final boolean result = complex.executeTest(param, new DescEntry[]{entry});
            log.println("********** end test *************");

            assure("CheckModuleAPI.module(" + module + ") PASSED.FAILED", result);

        } else {
            log.println("No modules containing qa/unoapi foder found => OK");
            state = true;
        }

        setUnoApiCwsStatus(state);

    }

    private void setUnoApiCwsStatus(boolean status) {

        if (!param.getBool(PropertyName.NO_CWS_ATTACH)) {

            final String version = (String) param.get(PropertyName.VERSION);
            if (version.startsWith("cws_")) {
                try {

                    // cws version: all added modules must be tested
                    final String cws = version.substring(4, version.length());
                    final CwsDataExchangeImpl cde = new CwsDataExchangeImpl(cws, param, log);
                    cde.setUnoApiCwsStatus(status);
                } catch (ParameterNotFoundException ex) {
                    log.println("ERROR: could not wirte status to EIS database: " + ex.toString());
                }
            }
        }
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
