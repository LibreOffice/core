package installer;

import java.io.*;
import java.util.*;
import java.util.jar.*;
//import org.xml.sax.*;
//import org.w3c.dom.*;
//import javax.xml.parsers.*;
import java.net.URL;
import java.net.JarURLConnection;
//import javax.xml.parsers.*;
import javax.swing.*;

/**
 *  The <code>XmlUpdater</code> pulls a META-INF/converter.xml
 *  file out of a jar file and parses it, providing access to this
 *  information in a <code>Vector</code> of <code>ConverterInfo</code>
 *  objects.
 *
 *  @author  Aidan Butler
 */
public class IdeUpdater extends Thread {

    private String classesPath = null;
    private String jarfilename;
    private String installPath;

    private JLabel statusLabel;

    private Vector listeners;
    private Thread internalThread;
    private boolean threadSuspended;
    private JProgressBar progressBar;

    private boolean isNetbeansPath = false;


    public IdeUpdater(String installPath, JLabel statusLabel, JProgressBar pBar) {

        if (installPath.endsWith(File.separator) == false)
            installPath += File.separator;

    //File jeditLauncher = new File( installPath + "jedit.jar" );
    File netbeansLauncher = new File( installPath + "bin" );

    if( netbeansLauncher.isDirectory() ) {
        isNetbeansPath = true;
        installPath = installPath +"modules" + File.separator;
    }
    /*
    else if( jeditLauncher.isFile() ){
        isNetbeansPath =  false;
        installPath = installPath + "jars" + File.separator;
    }
    */

    System.out.println( "IdeUpdater installPath is " + installPath + " isNetbeansPath is " + isNetbeansPath );
        this.installPath = installPath;
        this.statusLabel = statusLabel;
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

        //InputStream istream;
        //URL url;
        //String fileName = null;

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

        //System.out.println("About to call register");
    //Register.register(installPath+File.separator, statusLabel, progressBar);

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
