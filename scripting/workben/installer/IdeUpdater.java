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
import javax.swing.*;

/**
 *  The <code>XmlUpdater</code> pulls a META-INF/converter.xml
 *  file out of a jar file and parses it, providing access to this
 *  information in a <code>Vector</code> of <code>ConverterInfo</code>
 *  objects.
 */
public class IdeUpdater extends Thread {

    private String classesPath = null;
    private String jarfilename;
    private String installPath;

    private JLabel statusLabel;

    private Vector<InstallListener> listeners;
    private Thread internalThread;
    private boolean threadSuspended;
    private JProgressBar progressBar;

    private boolean isNetbeansPath = false;


    public IdeUpdater(String installPath, JLabel statusLabel, JProgressBar pBar) {

        if (installPath.endsWith(File.separator) == false)
            installPath += File.separator;

    File netbeansLauncher = new File( installPath + "bin" );

    if( netbeansLauncher.isDirectory() ) {
        isNetbeansPath = true;
        installPath = installPath +"modules" + File.separator;
    }

    System.out.println( "IdeUpdater installPath is " + installPath + " isNetbeansPath is " + isNetbeansPath );
        this.installPath = installPath;
        this.statusLabel = statusLabel;
    listeners = new Vector<InstallListener>();
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
            if (threadSuspended)
            {
        synchronized(this)
        {
                    while (threadSuspended)
                    {
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

    internalThread = Thread.currentThread();

    progressBar.setString("Unzipping Required Files");
        ZipData zd = new ZipData("SFrameworkInstall.jar");

    // Adding IDE support
    if( isNetbeansPath ) {
        if (!zd.extractEntry("ide/office.jar",installPath, statusLabel))
            {
            onInstallComplete();
            return;
        }
    }
    else {
        if (!zd.extractEntry("ide/idesupport.jar",installPath, statusLabel))
            {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("ide/OfficeScripting.jar",installPath, statusLabel))
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
        Enumeration<InstallListener> e = listeners.elements();
        while (e.hasMoreElements())
        {
            InstallListener listener = e.nextElement();
            listener.installationComplete(null);
        }
    }// onInstallComplete

}// XmlUpdater class
