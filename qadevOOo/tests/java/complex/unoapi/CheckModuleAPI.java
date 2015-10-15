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

/** complex tests to check the UNO-API
 **/
package complex.unoapi;

import base.java_complex;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import helper.ParameterNotFoundException;
import helper.ProcessHandler;
import com.sun.star.lang.XMultiServiceFactory;
import helper.BuildEnvTools;
import helper.ComplexDescGetter;
import java.io.File;
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
public class CheckModuleAPI extends ComplexTestCase
{

    private static String mSRC_ROOT = null;
    private static boolean mIsInitialized = false;
    private static final boolean mContinue = true;
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
    public void before()
    {

        if (!mIsInitialized)
        {
            mIsInitialized = true;

            try
            {

                bet = new BuildEnvTools(param, log);

            }
            catch (ParameterNotFoundException ex)
            {
                this.failed(ex.toString(), false);
            }

            mSRC_ROOT = bet.getSrcRoot();
        }
    }

    /**
     * let API tests run.
     */
    public void checkModule(String module)
    {
        log.println(utils.getDateTime() + ": start testing module '" + module + "'");

        log.println(utils.getDateTime() + "start new Office instance...");

        // TODO: is Office started with this program?
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

        final String[] commands = getDmakeCommands(sUnoapi);

        final ProcessHandler procHdl = bet.runCommandsInEnvironmentShell(commands, fUnoapi, 0);
        log.println("exit code of dmake: " + procHdl.getExitCode());
        String test = procHdl.getOutputText();
        test += procHdl.getErrorText();
        assure("module '" + module + "' failed", verifyOutput(test), mContinue);
        log.println(utils.getDateTime() + " module '" + module + "': kill existing office...");

        // TODO: how to check if the office is not started with this process.
        boolean bNoOffice = param.getBool("NoOffice");
        if (!bNoOffice)
        {
            try
            {
                officeProvider.closeExistingOffice(param, true);
            }
            catch (java.lang.UnsatisfiedLinkError exception)
            {
                log.println("Office seems not to be running");
            }
        }
    }
    private String getQaUnoApiPath(String srcRoot, String _sModul)
    {
        File aFile = new File(srcRoot);
        if (!aFile.exists())
        {
            System.out.println("ERROR: srcRoot '" + srcRoot + "' does not exist.");
            return null;
        }
        String sModulePath = srcRoot;
        sModulePath += File.separator;
        sModulePath += _sModul;

        File aModulePath = new File(sModulePath);
        if (! aModulePath.exists())
        {
            aModulePath = new File(sModulePath + ".lnk");
            if (! aModulePath.exists())
            {
                aModulePath = new File(sModulePath + ".link");
                if (! aModulePath.exists())
                {
                    // module does not exist.
                    return null;
                }
            }
        }
        sModulePath = aModulePath.getAbsolutePath();
        sModulePath += File.separator;
        sModulePath += "qa";
        sModulePath += File.separator;
        sModulePath += "unoapi";
        File aModulePathQaUnoApi = new File(sModulePath);
        if (aModulePathQaUnoApi.exists())
        {
            return aModulePathQaUnoApi.getAbsolutePath();
        }
        return null;
    }
    private boolean doesQaUnoApiFolderExist(String srcRoot, String _sModul)
    {
        return getQaUnoApiPath(srcRoot, _sModul) != null;
    }

    private String[] getAllModuleCommand()
    {
        final String[] names = getModulesFromSourceRoot();
        String[] checkModules = getCheckModuleCommand(names);
        return checkModules;
    }

    private String[] getCheckModuleCommand(String[] names)
    {
        String[] checkModules;
        checkModules = new String[names.length];

        for (int i = 0; i < names.length; i++)
        {
            // if a module is not added to a cws it contains a dot in its name (forms.lnk)
            if (names[i].indexOf(".") != -1)
            {
                checkModules[i] = "checkModule(" + names[i].substring(0, names[i].indexOf(".")) + ")";
            }
            else
            {
                checkModules[i] = "checkModule(" + names[i] + ")";
            }
        }
        return checkModules;
    }

    private String[] getDmakeCommands(String sUnoapi)
    {

        String[] cmdLines = null;
        final String platform = (String) param.get(PropertyName.OPERATING_SYSTEM);
        log.println("prepare command for platform " + platform);

        if (platform.equals(PropertyName.WNTMSCI))
        {
            if (param.getBool(PropertyName.CYGWIN))
            {
                // cygwin stuff
                cmdLines = new String[]
                        {
                            "cd `cygpath '" + sUnoapi.replaceAll("\\\\", "\\\\\\\\") + "'`",
                            "dmake"
                        };
            }
            else
            {
                // old 4NT
                cmdLines = new String[]
                        {
                            "cdd " + sUnoapi,
                            "dmake"
                        };
            }
        }
        else
        {
            // unix
            cmdLines = new String[]
                    {
                        "cd " + sUnoapi,
                        "dmake"
                    };
        }
        return cmdLines;
    }

    private String[] getDefinedModuleCommand(String module)
    {
        String[] checkModules = null;
        // list of modules to test: (sw,sc,sd)
        if (module.indexOf(",") != -1)
        {
            final String[] names = module.split(",");
            checkModules = new String[names.length];
            for (int i = 0; i < names.length; i++)
            {
                final String moduleName = names[i].trim();

                if (doesQaUnoApiFolderExist(mSRC_ROOT, moduleName))
                {
                    checkModules[i] = "checkModule(" + moduleName + ")";
                }
            }
        }
        else
        {
            if (doesQaUnoApiFolderExist(mSRC_ROOT, module))
            {
                checkModules = new String[]
                        {
                            "checkModule(" + module + ")"
                        };
            }
        }
        return checkModules;
    }

    private String getModulePath(String module)
    {
        String sUnoapi = getQaUnoApiPath(mSRC_ROOT, module);
        return sUnoapi;
    }

    /**
    Some modules contains more the one project. This methods translates given project parameter to the
     *  correspind module name.
     *
     * fwk -> framework
     * fwl -> framework
     * sch -> chart2
     * lnn -> lingu
     * lng -> linguistic
     * sfx -> sfx2
     * sm -> starmath
     */
    private String getTranslatedNames(String module)
    {

        final HashMap<String,String> aModuleHashMap = new HashMap<String,String>();

        aModuleHashMap.put("fwk", "framework");
        aModuleHashMap.put("fwl", "framework");
        aModuleHashMap.put("sch", "chart2");
        aModuleHashMap.put("lnn", "lingu");
        aModuleHashMap.put("lng", "linguistic");
        aModuleHashMap.put("sfx", "sfx2");
        aModuleHashMap.put("sm", "starmath");

        // it could be that the parameter looks like "fwk,fwl". This results in double "famework,framework".
        // The following loop correct this to only one "framework"

        final Set<String> keys = aModuleHashMap.keySet();
        final Iterator<String> iterator = keys.iterator();
        while (iterator.hasNext())
        {

            final String key = iterator.next();
            final String value = aModuleHashMap.get(key);

            module = module.replaceAll(key, value);

            final int count = module.split(value).length;
            if (count > 2)
            {
                for (int i = 2; i < count; i++)
                {
                    module.replaceFirst("," + value, "");
                }

            }
        }
        return module;
    }

    private boolean verifyOutput(String output)
    {

        log.println("verify output...");
        boolean ok = false;
        final String[] outs = output.split("\n");

        for (int i = 0; i < outs.length; i++)
        {
            final String line = outs[i];
            if (line.matches("[0-9]+? of [0-9]+? tests failed"))
            {
                log.println("matched line: " + line);
                if (line.matches("0 of [0-9]+? tests failed"))
                {
                    ok = true;
                    log.println("Module passed OK");
                }
                else
                {
                    log.println("Module passed FAILED");
                }
            }
        }

        if (!ok)
        {
            log.println("ERROR: could not find '0 of [0-9]+? tests failed' in output");
        }

        return ok;
    }

    private String[] getModulesFromSourceRoot()
    {
        log.println("**** run module tests over all modules ****");

        log.println("search for qa/unoapi foldres in all modules based in ");
        log.println("'" + mSRC_ROOT + "'");

        final ArrayList<String> moduleNames = new ArrayList<String>();
        final File sourceRoot = new File(mSRC_ROOT);
        final File[] sourceTree = sourceRoot.listFiles();

        for (int i = 0; i < sourceTree.length; i++)
        {
            final File moduleName = sourceTree[i];
            String sModuleName = moduleName.getName(); // (String)moduleNames.get(i);
            if (doesQaUnoApiFolderExist(mSRC_ROOT, sModuleName))
            {
                moduleNames.add(sModuleName);
            }
        }

        final String[] names = moduleNames.toArray(new String[moduleNames.size()]);
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
    public void module(String module)
    {

        String[] checkModules;
        final ComplexDescGetter desc = new ComplexDescGetter();
        DescEntry entry = null;
        module = module.trim();

        /*
        all: check all modules which contains a qa/unoapi folder
        auto: check all modules which contains a qa/unoapi folder except the module is not added
         */
        if (module.equals("all") || module.equals("auto"))
        {
            checkModules = getAllModuleCommand();
        }
        else
        {
            module = getTranslatedNames(module);
            checkModules = getDefinedModuleCommand(module);
        }

        if (checkModules != null && checkModules.length > 0)
        {

            entry = desc.createTestDesc("complex.unoapi.CheckModuleAPI", "complex.unoapi.CheckModuleAPI", checkModules,
                    log);

            final java_complex complex = new java_complex();

            log.println("********** start test *************");
            final boolean result = complex.executeTest(param, new DescEntry[] { entry });
            log.println("********** end test *************");

            assure("CheckModuleAPI.module(" + module + ") PASSED.FAILED", result);

        }
        else
        {
            log.println("No modules containing qa/unoapi folder found => OK");
            state = true;
        }
    }

    @Override
    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "module(all)"
                };
    }

}
