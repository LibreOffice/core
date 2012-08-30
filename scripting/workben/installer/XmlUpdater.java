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

import java.io.*;
import java.util.*;
import java.net.URL;
import javax.swing.*;

/**
 *
 *
 *  @author  Aidan Butler
 */
public class XmlUpdater extends Thread {

    private String classesPath = null;
    private String jarfilename;
    private String installPath;
    private boolean netInstall;
    private boolean bindingsInstall;

    private JLabel statusLabel;

    private Vector listeners;
    private Thread internalThread;
    private boolean threadSuspended;
    private JProgressBar progressBar;

    private final String[] bakFiles =
    {
        "writermenubar.xml",
        "writerkeybinding.xml",
        "calcmenubar.xml",
        "calckeybinding.xml",
        "impressmenubar.xml",
        "impresskeybinding.xml",
        "drawmenubar.xml",
        "drawkeybinding.xml",
        "eventbindings.xml",
        "META-INF" + File.separator + "manifest.xml"
    };

    private final String[] dirs =
    {
        "java" + File.separator + "Highlight",
        "java" + File.separator + "MemoryUsage",
        "java" + File.separator + "ScriptFrmwrkHelper",
        "java" + File.separator + "debugger",
        "java" + File.separator + "debugger" + File.separator + "rhino",
        "beanshell" + File.separator + "InteractiveBeanShell",
        "beanshell" + File.separator + "Highlight",
        "beanshell" + File.separator + "MemoryUsage",
        "javascript" + File.separator + "ExportSheetsToHTML"
    };

    private final String[] names =
    {
        "java/Highlight/HighlightUtil.java",
        "java/Highlight/HighlightText.java",
        "java/Highlight/Highlight.jar",
        "java/Highlight/parcel-descriptor.xml",
        "java/MemoryUsage/MemoryUsage.java",
        "java/MemoryUsage/MemoryUsage.class",
        "java/MemoryUsage/parcel-descriptor.xml",
        "java/MemoryUsage/ExampleSpreadSheet.sxc",
        "java/ScriptFrmwrkHelper/parcel-descriptor.xml",
        "java/ScriptFrmwrkHelper/ScriptFrmwrkHelper.java",
        "java/ScriptFrmwrkHelper/ScriptFrmwrkHelper.class",
        "java/ScriptFrmwrkHelper/ScriptFrmwrkHelper.jar",
        "java/debugger/debugger.jar",
        "java/debugger/OOBeanShellDebugger.java",
        "java/debugger/OOScriptDebugger.java",
        "java/debugger/DebugRunner.java",
        "java/debugger/OORhinoDebugger.java",
        "java/debugger/parcel-descriptor.xml",
        "java/debugger/rhino/Main.java",
        "beanshell/InteractiveBeanShell/parcel-descriptor.xml",
        "beanshell/InteractiveBeanShell/interactive.bsh",
        "beanshell/Highlight/parcel-descriptor.xml",
        "beanshell/Highlight/highlighter.bsh",
        "beanshell/MemoryUsage/parcel-descriptor.xml",
        "beanshell/MemoryUsage/memusage.bsh",
        "javascript/ExportSheetsToHTML/parcel-descriptor.xml",
        "javascript/ExportSheetsToHTML/exportsheetstohtml.js"
    };


    public XmlUpdater(String installPath, JLabel statusLabel,JProgressBar pBar, boolean netInstall, boolean bindingsInstall) {
        this.installPath = installPath;
        this.statusLabel = statusLabel;
        this.netInstall = netInstall;
        this.bindingsInstall = bindingsInstall;
        listeners = new Vector();
        threadSuspended = false;
        progressBar=pBar;
        progressBar.setStringPainted(true);
    }// XmlUpdater


    public boolean checkStop()
    {
        if (internalThread == Thread.currentThread())
            return false;
        return true;
    }// checkStop


    public void checkSuspend()
    {
        if (threadSuspended) {
            synchronized(this) {
                while (threadSuspended) {
                    try {
                        wait();
                    } catch (InterruptedException eInt) {
                        //...
                    }
                }
            }
        }
    }// checkSuspend


    public void setSuspend()
    {
        threadSuspended = true;
    }// setSuspend


    public void setResume()
    {
        threadSuspended = false;
        notify();
    }// setResume


    public void setStop()
    {
        internalThread = null;
    }// setStop


    public void run() {

        InputStream            istream;
        //InputSource            isource;
        //DocumentBuilderFactory builderFactory;
        //DocumentBuilder        builder = null;
        URL                    url;
        String                 fileName = null;

        internalThread = Thread.currentThread();

        //System.out.println("\n\n\n\nFileName: "+installPath);
        classesPath= installPath.concat(File.separator+"program"+File.separator+"classes"+File.separator);
        String opSys =System.getProperty("os.name");
        //System.out.println("\n System "+opSys);

        String progpath=installPath;
        progpath= progpath.concat(File.separator+"program"+File.separator);
        //System.out.println("Office progpath" + progpath );
        //System.out.println("\nModifying Installation "+installPath);

        String starBasicPath=installPath;
        starBasicPath= starBasicPath.concat(File.separator+"share"+File.separator+"basic"+File.separator+"ScriptBindingLibrary"+File.separator);
        //System.out.println( "Office StarBasic path: " + starBasicPath );

        String regSchemaOfficePath=installPath;
        regSchemaOfficePath= regSchemaOfficePath.concat(File.separator+"share"+File.separator+"registry"+File.separator+"schema"+File.separator+"org"+File.separator+"openoffice"+File.separator+"Office"+File.separator);
        //System.out.println( "Office schema path: " + regSchemaOfficePath );

        // Get the NetBeans installation
        //String netbeansPath=

        progressBar.setString("Unzipping Required Files");
        ZipData zd = new ZipData("SFrameworkInstall.jar");


        if( (!netInstall) || bindingsInstall) {
            String configPath=installPath;
            configPath= configPath.concat(File.separator+"user"+File.separator+"config"+File.separator+"soffice.cfg"+File.separator);
            //System.out.println( "Office configuration path: " + configPath );
            String manifestPath=configPath + "META-INF" + File.separator;

            //Adding <Office>/user/config/soffice.cfg/
            File configDir = new File( configPath );
            if( !configDir.isDirectory() ) {
                if( !configDir.mkdir() ) {
                    System.out.println( "creating  " + configDir + "directory failed");
                }
                else {
                    System.out.println( configDir + "directory created");
                }
            }
            else
                System.out.println( "soffice.cfg exists" );

            File manifestDir = new File( manifestPath );
            if( !manifestDir.isDirectory() ) {
                if( !manifestDir.mkdir() ) {
                    System.out.println( "creating " + manifestPath + "directory failed");
                }
                else {
                    System.out.println( manifestPath + " directory created");
                }
            }
            else
                System.out.println( manifestPath + " exists" );

            // Backup the confguration files in
            // <office>/user/config/soffice.cfg/
            // If they already exist.

            for( int i=0; i < bakFiles.length; i++ )
            {
                String pathNameBak = configPath + bakFiles[i];
                File origFile = new File( pathNameBak );
                if( origFile.exists() )
                {
                    System.out.println( "Attempting to backup " + pathNameBak + " to " + pathNameBak + ".bak" );
                    if(! origFile.renameTo( new File( pathNameBak + ".bak" ) ) )
                    {
                        System.out.println( "Failed to backup " + pathNameBak + " to " + pathNameBak + ".bak" );
                    }
                }
            }

            // Adding Office configuration files
            if (!zd.extractEntry("bindingdialog/writermenubar.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/writerkeybinding.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/calcmenubar.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/calckeybinding.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/impressmenubar.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/impresskeybinding.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/drawmenubar.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/drawkeybinding.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/eventbindings.xml",configPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/manifest.xml",manifestPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
        }

        if(!bindingsInstall) {
            // Adding new directories to Office
            // Adding <Office>/user/basic/ScriptBindingLibrary/
            File scriptBindingLib = new File( starBasicPath );
            if( !scriptBindingLib.isDirectory() ) {
                if( !scriptBindingLib.mkdir() ) {
                    System.out.println( "ScriptBindingLibrary failed");
                }
                else {
                    System.out.println( "ScriptBindingLibrary directory created");
                }
            }
            else
                System.out.println( "ScriptBindingLibrary exists" );

            // Adding Scripting Framework and tools
            if (!zd.extractEntry("sframework/ooscriptframe.zip",progpath, statusLabel))
            {
                onInstallComplete();
                return;
            }

            if (!zd.extractEntry("sframework/bshruntime.zip",progpath, statusLabel))
            {
                onInstallComplete();
                return;
            }

            if (!zd.extractEntry("sframework/jsruntime.zip",progpath, statusLabel))
            {
                onInstallComplete();
                return;
            }

            if (!zd.extractEntry("schema/Scripting.xcs",regSchemaOfficePath, statusLabel))
            {
                onInstallComplete();
                return;
            }

    //--------------------------------

            progressBar.setString("Registering Scripting Framework");
            progressBar.setValue(3);
            if(!Register.register(installPath+File.separator, statusLabel) ) {
               onInstallComplete();
               return;
            }
            progressBar.setValue(5);

            String path = installPath + File.separator +
                "share" + File.separator + "Scripts" + File.separator;

            for (int i = 0; i < dirs.length; i++) {
                File dir = new File(path + dirs[i]);

                if (!dir.exists()) {
                    if (!dir.mkdirs()) {
                        System.err.println("Error making dir: " +
                            dir.getAbsolutePath());
                        onInstallComplete();
                        return;
                    }
                }
            }

            for (int i = 0; i < names.length; i++) {
                String source = "/examples/" + names[i];
                String dest = path + names[i].replace('/', File.separatorChar);

                if (!zd.extractEntry(source, dest, statusLabel)) {
                    onInstallComplete();
                    return;
                }
            }


            // Adding binding dialog
            if (!zd.extractEntry("bindingdialog/ScriptBinding.xba",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/MenuBinding.xdl",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/KeyBinding.xdl",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/EventsBinding.xdl",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/HelpBinding.xdl",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/EditDebug.xdl",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/dialog.xlb",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
            if (!zd.extractEntry("bindingdialog/script.xlb",starBasicPath, statusLabel))
            {
                onInstallComplete();
                return;
            }
        }


        statusLabel.setText("Installation Complete");
        progressBar.setString("Installation Complete");
        progressBar.setValue(10);
        onInstallComplete();

    }// run


    public void addInstallListener(InstallListener listener)
    {
        listeners.addElement(listener);
    }// addInstallListener


    private void onInstallComplete()
    {
        Enumeration e = listeners.elements();
        while (e.hasMoreElements())
        {
            InstallListener listener = (InstallListener)e.nextElement();
            listener.installationComplete(null);
        }
    }// onInstallComplete

}// XmlUpdater class
