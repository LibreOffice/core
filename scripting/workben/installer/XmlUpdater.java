package installer;

import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.net.URL;
import java.net.JarURLConnection;
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

    private JLabel statusLabel;

    private Vector listeners;
    private Thread internalThread;
    private boolean threadSuspended;
    private JProgressBar progressBar;


    public XmlUpdater(String installPath, JLabel statusLabel,JProgressBar pBar) {
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

        String configPath=installPath;
        configPath= configPath.concat(File.separator+"user"+File.separator+"config"+File.separator+"soffice.cfg"+File.separator);
        //System.out.println( "Office configuration path: " + configPath );
        String manifestPath=configPath + File.separator + "META-INF" + File.separator;

        String starBasicPath=installPath;
        starBasicPath= starBasicPath.concat(File.separator+"user"+File.separator+"basic"+File.separator+"ScriptBindingLibrary"+File.separator);
        //System.out.println( "Office StarBasic path: " + starBasicPath );


        // Get the NetBeans installation
        //String netbeansPath=

    progressBar.setString("Unzipping Required Files");
        ZipData zd = new ZipData("SFrameworkInstall.jar");


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

        //Adding <Office>/user/config/soffice.cfg/
        File configDir = new File( configPath );
        if( !configDir.isDirectory() ) {
            if( !configDir.mkdir() ) {
                System.out.println( "soffice.cfg directory failed");
            }
            else {
                System.out.println( "soffice.cfg directory created");
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

//--------------------------------
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


//--------------------------------
// Robert Kinsella test 2

        String path = installPath + File.separator +
            "user" + File.separator + "Scripts" + File.separator;

        String[] dirs = {
        "java" + File.separator + "Highlight",
        "java" + File.separator + "MemoryUsage",
        "java" + File.separator + "ScriptFrmwrkHelper",
            "beanshell" + File.separator + "InteractiveBeanShell",
            "beanshell" + File.separator + "Highlight",
            "beanshell" + File.separator + "MemoryUsage"
        };

        String[] names = {
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
            "beanshell/InteractiveBeanShell/parcel-descriptor.xml",
            "beanshell/InteractiveBeanShell/interactive.bsh",
            "beanshell/Highlight/parcel-descriptor.xml",
            "beanshell/Highlight/highlighter.bsh",
            "beanshell/MemoryUsage/parcel-descriptor.xml",
            "beanshell/MemoryUsage/memusage.bsh"
        };

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

// Robert Kinsella test 2 end
//--------------------------------

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

//--------------------------------

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


//--------------------------------

        progressBar.setString("Registering Scripting Framework");
        progressBar.setValue(7);
    if(!Register.register(installPath+File.separator, statusLabel) )
        {
           onInstallComplete();
           return;
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
