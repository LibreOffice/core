/*
 * InstallWizard.java
 *
 * Created on 04 July 2002, 15:09
 */

/**
 *
 * @author  mike
 */

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.net.*;
import java.io.*;

public class InstallWizard extends javax.swing.JFrame implements ActionListener {

    private static class ShutdownHook extends Thread {
        public void run()
    {
            if (InstallWizard.isInstallStarted())
            {
                // Check for and backup any config.xml files
                // Check for and backup any StarBasic macro files
                // Check for and backup ProtocolHandler
                /*
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
                 */
                //System.out.println( "ShutdownHook" );
            }

            InstUtil.removeTmpDir();
    }
    }// class ShutdownHook
    
    static {
        Runtime rt=Runtime.getRuntime();
        rt.addShutdownHook(new ShutdownHook());
    }
    
    /** Creates new form InstallWizard */
    public InstallWizard() {
        super("Scripting Framework Installer (prototype)");
    //setBackground(Color.WHITE);
    setBackground(new Color(0,0,0));
    locations = new ArrayList();
    //Point center = GraphicsEnvironment.getLocalGraphicsEnvironment().getCenterPoint();
    Point center = new Point( 300, 300 );
    int windowWidth=200;
    int windowHeight=300;
    setSize(windowWidth,windowHeight);
    setBounds((center.x-windowWidth/2)-115,(center.y-windowWidth/2)-100, windowWidth,windowHeight);
    setResizable(false);        
    initComponents();
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
        //screens.add("EULA", new EULA());
    // ----------------
    netbeansVersion = new NetbeansVersion(this);
    screens.add(NETBEANSVERSIONS, netbeansVersion);
        // ----------------
        version = new Version(this);
        screens.add(VERSIONS, version);
    _final = new Final(this);
        screens.add(FINAL, _final);
        
        getContentPane().add(screens, java.awt.BorderLayout.CENTER);

    navNext.addActionListener(this);
    navNext.addActionListener(netbeansVersion);
    navNext.addActionListener(version);
    navNext.addActionListener(netbeansVersion);
    navNext.addActionListener(_final);
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

    public static ArrayList getLocations()
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
        new InstallWizard().show();
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

    private Version version;
    private NetbeansVersion netbeansVersion;
    private Final _final;
    private static ArrayList locations;

    public static String VERSIONS = "VERSIONS";
    public static String WELCOME  = "WELCOME";
    public static String FINAL    = "FINAL";
    public static String NETBEANSVERSIONS = "NETBEANSVERSIONS";
    
    public static int DEFWIDTH     = 480;
    public static int DEFHEIGHT    = 240;

    private static String typesPath = null;
    private static String javaPath  = null;

    private static boolean bPatchedTypes     = false;
    private static boolean bPatchedJava      = false;
    private static boolean bPatchedRDB       = false;
    private static boolean bInstallStarted   = false;

}// InstallWizard
