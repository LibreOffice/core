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
public class XmlUpdater extends Thread {

    /*
    private final static String TAG_TYPES         = "Types";
    private final static String TAG_TYPEDETECTION = "TypeDetection";
    private final static String TAG_TYPE          = "Type";
    private final static String TAG_DATA          = "Data";
    private final static String TAG_FILTERS       = "Filters";
    private final static String TAG_FILTER        = "Filter";
    private final static String TAG_CLASSPATH     = "UserClassPath";
    */

    private String classesPath = null;
    private String jarfilename;
    private String installPath;

    /*
    private Document document;

    private Element filterNode;
    private Element installedNode;
    private Element uinameNode;
    private Element cfgvalueNode;
    private Element dataNode;
    private Element typeNode;
    */
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
                System.out.println( "ScriptBindingLibrary failed!!!!");
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
                System.out.println( "soffice.cfg directory failed!!!!");
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
    File highlightDir = new File( scriptsPath+File.separator+"Highlight"+File.separator );
    File memoryDir = new File( scriptsPath+File.separator+"MemoryUsage"+File.separator );
        if( !scriptsDir.isDirectory() ) {
        //File highlightDir = new File( scriptsPath+File.separator+"Highlight"+File.separator );
        //File spellDir = new File( scriptsPath+File.separator+"Memory"+File.separator );
            if( !highlightDir.mkdirs() ) {
                System.out.println( "Highlight script directory failed!!!!");
            }
            if( !memoryDir.mkdirs() ) {
                System.out.println( "MemoryUsage script directory failed!!!!");
            }
            else {
                System.out.println( "Scripts/java directory created");
            }
        }
        else
            System.out.println( "Scripts/java directory exists" );

//Robert Kinsella test 1 end



//--------------------------------
    // Adding Scripting Framework and tools
        if (!zd.extractEntry("sframework/ooscriptframe.zip",progpath, statusLabel))
        {
            onInstallComplete();
            return;
    }
        if (!zd.extractEntry("sframework/scripting64401.res",progpath+"resource"+File.separator, statusLabel))
        {
            onInstallComplete();
            return;
    }
    if (opSys.indexOf("Windows")!=-1){
            if (!zd.extractEntry("windows/regsingleton.exe",progpath, statusLabel))
            {
                onInstallComplete();
                return;
            }
        }
        else if (opSys.indexOf("Linux")!=-1){
            if (!zd.extractEntry("linux_x86/regsingleton",progpath, statusLabel))
            {
                onInstallComplete();
                return;
            }
        }
        else if (opSys.indexOf("SunOS")!=-1){
            if (!zd.extractEntry("solaris_sparc/regsingleton",progpath, statusLabel))
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

    // Adding IDE support
      /*  if (!zd.extractEntry("ide/netbeans/office.jar",netbeansPath, statusLabel))
        {
           onInstallComplete();
            return;
    } */
      /*  if (!zd.extractEntry("ide/jedit/?.jar",jeditPath, statusLabel))
        {
           onInstallComplete();
            return;
    } */

        //System.out.println("About to call register");
    Register.register(installPath+File.separator, statusLabel, progressBar);
    statusLabel.setText("Installation Complete");
    progressBar.setString("Installation Complete");
    progressBar.setValue(10);
    onInstallComplete();

    }// run

/*
    private void checkexists(String path){
        File checkFile = new File(path);
        String justPath= path.substring(0,path.lastIndexOf(File.separator)+1);
        //System.out.println("\n"+justPath);
        if(!checkFile.exists()){
            ZipData zd = new ZipData("XMergeInstall.jar");
            if (!zd.extractEntry("xml/Java.xml",justPath, statusLabel))
            {
                System.out.println("Fail");
            }
        }
    }// checkexists
*/

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
