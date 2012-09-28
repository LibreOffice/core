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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.net.*;
import java.io.*;

public class InstallWizard extends javax.swing.JFrame implements ActionListener {
/*
    private static class ShutdownHook extends Thread {
        public void run()
    {
            if (InstallWizard.isInstallStarted())
            {
                // Check for and backup any config.xml files
                // Check for and backup any StarBasic macro files
                // Check for and backup ProtocolHandler

                if (!InstallWizard.isPatchedTypes())
        {
                    File backup = new File(InstUtil.getTmpDir(), "TypeDetection.xml");
                    File destination = new File(InstallWizard.getTypesPath());
                    InstUtil.copy(backup, destination); //Restore typedetection.xml
        }
        if (!InstallWizard.isPatchedJava())
        {
                    File backup = new File(InstUtil.getTmpDir(), "Java.xml");
                    File destination = new File(InstallWizard.getJavaPath());
                    InstUtil.copy(backup, destination); //Restore typedetection.xml
        }
        if (!InstallWizard.isPatchedRDB())
        {
                    File backup = new File(InstUtil.getTmpDir(), "applicat.rdb");
                    File destination = new File(InstallWizard.getJavaPath());
                    //InstUtil.copy(backup, destination); //Restore typedetection.xml
        }

                System.out.println( "ShutdownHook" );
            }

            InstUtil.removeTmpDir();
    }
    }// class ShutdownHook

    static {
        Runtime rt=Runtime.getRuntime();
        rt.addShutdownHook(new ShutdownHook());
    }
*/
    /** Creates new form InstallWizard */
    public InstallWizard() {
        super("Office Scripting Framework Installer - Early Developer Release");

     try {
         System.out.print("All diagnostic output is being redirected to SFrameworkInstall.log\n");
         System.out.print("Location: "+  System.getProperty( "user.dir" ) +
             File.separator + "SFrameworkInstall.log\n");

         LogStream log = new LogStream( "SFrameworkInstall.log" );
                 System.setErr(log);

                 System.setOut(log);
     }
     catch( FileNotFoundException fnfe ) {
         System.err.println("Office Scripting Framework Installer - Error: ");
         System.err.println("Unable to create log file for installation.");
         exitForm(null);
     }

    //setBackground(Color.WHITE);
    setBackground(new Color(0,0,0));
    locations = new ArrayList<String>();
    //Point center = GraphicsEnvironment.getLocalGraphicsEnvironment().getCenterPoint();
    Point center = new Point( 400, 400 );
    int windowWidth=200;
    int windowHeight=300;
    setSize(windowWidth,windowHeight);
    setBounds((center.x-windowWidth/2)-115,(center.y-windowWidth/2)-100, windowWidth,windowHeight);
    initComponents();
    setResizable(false);
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {
        navigation = new javax.swing.JPanel();
        navBack = new javax.swing.JButton();
        navNext = new javax.swing.JButton();
        navCancel = new javax.swing.JButton();
        screens = new javax.swing.JPanel();

        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                exitForm(evt);
            }
        });

        navigation.setLayout(new java.awt.GridBagLayout());
        java.awt.GridBagConstraints gridBagConstraints1;

        navBack.setText("<< Back");
        gridBagConstraints1 = new java.awt.GridBagConstraints();
        gridBagConstraints1.insets = new java.awt.Insets(1, 1, 1, 1);

        navNext.setText("Next >>");
        gridBagConstraints1 = new java.awt.GridBagConstraints();
        gridBagConstraints1.gridx = 2;
        gridBagConstraints1.gridy = 0;

        navCancel.setText("Cancel");
        gridBagConstraints1 = new java.awt.GridBagConstraints();
        gridBagConstraints1.gridx = 6;
        gridBagConstraints1.gridy = 0;
        //navigation.add(navNext, gridBagConstraints1);
        //navigation.add(navBack, gridBagConstraints1);
        //navigation.add(navCancel, gridBagConstraints1);

        getContentPane().add(navigation, java.awt.BorderLayout.SOUTH);
        screens.setLayout(new java.awt.CardLayout());
        screens.add(WELCOME, new Welcome(this));
        version = new Version(this);
        screens.add(VERSIONS, version);
    _final = new Final(this);
        screens.add(FINAL, _final);

    //boolean hasIDEInstallation = (InstUtil.hasNetbeansInstallation() || InstUtil.hasJeditInstallation()) ;
    boolean hasIDEInstallation = ( InstUtil.hasNetbeansInstallation() ) ;

    if( hasIDEInstallation )
    {
        idewelcome = new IdeWelcome(this);
        screens.add(IDEWELCOME, idewelcome);
        ideversion = new IdeVersion(this);
        screens.add(IDEVERSIONS, ideversion);
        idefinal = new IdeFinal(this);
        screens.add(IDEFINAL, idefinal);
    }
        getContentPane().add(screens, java.awt.BorderLayout.CENTER);

    navNext.addActionListener(this);
    navNext.addActionListener(version);
    navNext.addActionListener(_final);

    if( hasIDEInstallation )
    {
        navNext.addActionListener(ideversion);
        navNext.addActionListener(idefinal);
    }

    navCancel.addActionListener(this);
    navBack.addActionListener(this);


    URL url = this.getClass().getResource("sidebar.jpg");
    JLabel sideBar = new JLabel();
    sideBar.setIcon(new ImageIcon(url));
    getContentPane().add (sideBar, java.awt.BorderLayout.WEST);
        pack();
    }// initComponents

    /** Exit the Application */
    public void exitForm(java.awt.event.WindowEvent evt) {
        System.exit(0);
    }


    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == navNext)
    {
            ((CardLayout)screens.getLayout()).next(screens);
    }

    if (e.getSource() == navCancel)
    {
            exitForm(null);
    }

    if (e.getSource() == navBack)
    {
            ((CardLayout)screens.getLayout()).previous(screens);
    }
    }// actionPerformed

    public static void storeLocation(String path)
    {
        locations.add(path);
    }

    public static ArrayList<String> getLocations()
    {
        return locations;
    }

    public static void clearLocations()
    {
        locations.clear();
    }

    public void show(String cardName)
    {
        ((CardLayout)screens.getLayout()).show(screens, cardName);
    }

    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        String officePath = null;
        String netbeansPath = null;
        //String jeditPath = null;
        int i = 0;

        while (i < args.length) {
            if (args[i].equals("-help")) {
                printUsage();
                System.exit(0);
            }
            if (args[i].equals("-office"))
                officePath = args[++i];
            if (args[i].equals("-netbeans"))
                netbeansPath = args[++i];
            if (args[i].equals("-net"))
                bNetworkInstall = true;
            if (args[i].equals("-bindings"))
                bBindingsInstall = true;
            //if (args[i].equals("-jedit"))
            //    jeditPath = args[++i];
            i++;
        }

        //if (officePath == null && netbeansPath == null && jeditPath == null)
        if (officePath == null && netbeansPath == null)
            new InstallWizard().show();

        JLabel label = new JLabel();
        JProgressBar progressbar = new JProgressBar();

        try {
            System.out.println("Log file is: " +
                System.getProperty("user.dir") +
                File.separator + "SFrameworkInstall.log");

            LogStream log = new LogStream( "SFrameworkInstall.log" );
            System.setErr(log);
            System.setOut(log);
        }
        catch( FileNotFoundException fnfe ) {
            System.err.println("Error: Unable to create log file: "
                + fnfe.getMessage());
            System.exit(-1);
        }

        if (officePath != null) {
            XmlUpdater xud = new XmlUpdater(officePath, label, progressbar, bNetworkInstall, bBindingsInstall );
            xud.run();
        }

        if (netbeansPath != null) {
            IdeUpdater ideup = new IdeUpdater(netbeansPath, label, progressbar);
            ideup.run();
        }

        //if (jeditPath != null) {
        //    IdeUpdater ideup = new IdeUpdater(jeditPath, label, progressbar);
        //    ideup.run();
        //}
    }

    private static void printUsage() {
        System.err.println("java -jar SFrameworkInstall.jar");
        System.err.println("\t[-office <path_to_office_installation]");
        System.err.println("\t[-netbeans <path_to_netbeans_installation]");
        System.err.println("\t[-net]");
        System.err.println("\t[-bindings]");
        System.err.println("\n\n-net indicates that this is the network part of a network install.");
        System.err.println("-bindings will only install the menu & key bindings in user/config/soffice.cfg.");
        //System.err.println("\t[-jedit <path_to_jedit_installation]");
    }

    public static synchronized boolean isPatchedTypes()
    {
        return bPatchedTypes;
    }

    public static synchronized boolean isPatchedJava()
    {
        return bPatchedJava;
    }

    public static synchronized boolean isPatchedRDB()
    {
        return bPatchedRDB;
    }

    public static synchronized boolean isInstallStarted()
    {
        return bInstallStarted;
    }

    public static synchronized void setPatchedTypes(boolean value)
    {
        bPatchedTypes = value;
    }

    public static synchronized void setPatchedJava(boolean value)
    {
        bPatchedJava = value;
    }

    public static synchronized void setPatchedRDB(boolean value)
    {
        bPatchedRDB = value;
    }

    public static synchronized void setInstallStarted(boolean value)
    {
        bInstallStarted = value;
    }

    public static synchronized void setTypesPath(String path)
    {
        typesPath = path;
    }

    public static synchronized void setJavaPath(String path)
    {
        javaPath = path;
    }

    public static synchronized String getTypesPath()
    {
        return typesPath;
    }

    public static synchronized String getJavaPath()
    {
        return javaPath;
    }

    private javax.swing.JPanel  navigation;
    private javax.swing.JButton navBack;
    private javax.swing.JButton navNext;
    private javax.swing.JButton navCancel;
    private javax.swing.JPanel  screens;

    private Version version = null;
    private Final _final  = null;
    private IdeVersion ideversion = null;
    private IdeFinal idefinal = null;
    private IdeWelcome idewelcome = null;
    private static ArrayList<String> locations = null;

    public static String VERSIONS = "VERSIONS";
    public static String WELCOME  = "WELCOME";
    public static String FINAL    = "FINAL";
    public static String IDEVERSIONS = "IDEVERSIONS";
    public static String IDEWELCOME  = "IDEWELCOME";
    public static String IDEFINAL    = "IDEFINAL";

    public static int DEFWIDTH     = 480;
    public static int DEFHEIGHT    = 240;

    private static String typesPath = null;
    private static String javaPath  = null;

    public static boolean bNetworkInstall   = false;
    public static boolean bBindingsInstall   = false;

    private static boolean bPatchedTypes     = false;
    private static boolean bPatchedJava      = false;
    private static boolean bPatchedRDB       = false;
    private static boolean bInstallStarted   = false;

}// InstallWizard
