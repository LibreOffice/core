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

        String starBasicPath=installPath;
        starBasicPath= starBasicPath.concat(File.separator+"user"+File.separator+"basic"+File.separator+"ScriptBindingLibrary"+File.separator);
        //System.out.println( "Office StarBasic path: " + starBasicPath );

    String scriptsPath=installPath;
    scriptsPath= scriptsPath.concat(File.separator+"user"+File.separator+"Scripts"+File.separator+"java"+File.separator);
    //System.out.println( " Office Scripts Path: " + scriptsPath );

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

// Robert Kinsella test 1

        //Adding <Office>/user/Scripts/java/
        File scriptsDir = new File( scriptsPath );
    File highlightDir = new File( scriptsPath+"Highlight" );
    File memoryDir = new File( scriptsPath+"MemoryUsage" );
    File bshDir = new File( scriptsPath+"InteractiveBeanShell" );
        if( !highlightDir.mkdirs() ) {
            System.out.println( "Highlight script directory failed");
        }
        if( !bshDir.mkdirs() )
    {
            System.out.println( "InteractiveBeanShell script directory failed");
        }
        if( !memoryDir.mkdirs() )
    {
            System.out.println( "MemoryUsage script directory failed");
        }
        else
    {
            System.out.println( "Scripts/java directory created");
        }

//Robert Kinsella test 1 end



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

    // Check for OpenOffice Scxripting Security Resource
    // Check for OpenOffice Scxripting Security Resource
        if (!zd.extractEntry("sframework/scripting64301.res",progpath+"resource"+File.separator, statusLabel))
        {
        // No OpenOffice Scxripting Security Resource - check for Star Office one
        if (!zd.extractEntry("sframework/scripting64401.res",progpath+"resource"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
    }

//--------------------------------
// Robert Kinsella test 2

    // adding (JAVA) script examples
    File highlightScript = new File( scriptsPath+File.separator+"Highlight"+File.separator+"HighlightUtil.java" );
    if( !highlightScript.exists() ) {
        if (!zd.extractEntry("examples/Highlight/HighlightUtil.java",scriptsPath+File.separator+"Highlight"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/Highlight/HighlightText.java",scriptsPath+File.separator+"Highlight"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/Highlight/Highlight.jar",scriptsPath+File.separator+"Highlight"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/Highlight/parcel-descriptor.xml",scriptsPath+File.separator+"Highlight"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
    }
    else {
        System.out.println( "Highlight script already deployed" );
    }
    File memoryScript = new File( scriptsPath+File.separator+"MemoryUsage"+File.separator+"MemoryUsage.java" );
    if( !memoryScript.exists() ) {
        if (!zd.extractEntry("examples/MemoryUsage/MemoryUsage.java",scriptsPath+File.separator+"MemoryUsage"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/MemoryUsage/MemoryUsage.class",scriptsPath+File.separator+"MemoryUsage"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/MemoryUsage/parcel-descriptor.xml",scriptsPath+File.separator+"MemoryUsage"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/MemoryUsage/ExampleSpreadSheet.sxc",scriptsPath+File.separator+"MemoryUsage"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
    }
    else {
        System.out.println( "MemoryUsage script already deployed" );
        }

    File script = new File( scriptsPath+File.separator+"InteractiveBeanShell"+File.separator+"interactive.bsh" );
    if( !script.exists() ) {
        if (!zd.extractEntry("examples/InteractiveBeanShell/interactive.bsh",scriptsPath+File.separator+"InteractiveBeanShell"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
        if (!zd.extractEntry("examples/InteractiveBeanShell/parcel-descriptor.xml",scriptsPath+File.separator+"InteractiveBeanShell"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
        }
    }
    else {
        System.out.println( "InteractiveBeanShell script already deployed" );
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
