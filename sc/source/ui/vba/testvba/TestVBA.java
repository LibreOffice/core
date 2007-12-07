//package org.openoffice.vba;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.script.provider.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.AnyConverter;
import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.OutputStream;
public class TestVBA
{
private com.sun.star.lang.XMultiComponentFactory xMCF = null;
private com.sun.star.uno.XComponentContext xContext = null;
private com.sun.star.frame.XComponentLoader xCompLoader = null;
private File outDir;

public TestVBA( com.sun.star.uno.XComponentContext _xContext, com.sun.star.lang.XMultiComponentFactory _xMCF, com.sun.star.frame.XComponentLoader _xCompLoader, File _outDir
)
{
    xMCF = _xMCF;
    xContext = _xContext;
    xCompLoader = _xCompLoader;
    outDir = _outDir;
}

public String getLogLocation() throws  com.sun.star.beans.UnknownPropertyException,  com.sun.star.lang.IllegalArgumentException, com.sun.star.lang.WrappedTargetException,  com.sun.star.uno.Exception
{
    XPropertySet pathSettings = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,  xMCF.createInstanceWithContext( "com.sun.star.comp.framework.PathSettings", xContext) );
        String sLogLocation = AnyConverter.toString ( pathSettings.getPropertyValue( "Work" ) );
        sLogLocation = sLogLocation + "/" + "HelperAPI-test.log";
    return sLogLocation;
}

public void init()
{
    // blow away previous logs?
}

public void traverse( File fileDirectory )
{
        if ( !fileDirectory.isDirectory() )
        {
            throw new IllegalArgumentException(
                "not a directory: " + fileDirectory.getName()
                );
        }

        // Getting all files and directories in the current directory
        File[] entries = fileDirectory.listFiles();


        // Iterating for each file and directory
        for ( int i = 0; i < entries.length; ++i )
        {
            // Testing, if the entry in the list is a directory
/*
            if ( entries[ i ].isDirectory() )
            {
                // ignore Recursive call for the new directory
                // traverse( entries[ i ] );
            }
            else
*/
            if ( !entries[ i ].isDirectory() &&
              entries[ i ].getName().endsWith(".xls") )
            {
                try
                {
                    // Composing the URL by replacing all backslashs
                    String sUrl = "file:///"
                        + entries[ i ].getAbsolutePath().replace( '\\', '/' );
                    System.out.println( "processing " + sUrl );
                    // Loading the wanted document
                    com.sun.star.beans.PropertyValue propertyValues[] =
                        new com.sun.star.beans.PropertyValue[1];
                    propertyValues[0] = new com.sun.star.beans.PropertyValue();
                    propertyValues[0].Name = "Hidden";
                    propertyValues[0].Value = new Boolean(true);
                    propertyValues[0].Value = new Boolean(false);

                    Object oDoc =
                        xCompLoader.loadComponentFromURL(
                            sUrl, "_blank", 0, propertyValues);
                    String logFileURL = getLogLocation();
                    java.net.URI logURI = new java.net.URI( logFileURL );
                    java.net.URL logURL = logURI.toURL();
                    try
                    {
                        XScriptProviderSupplier xSupplier =
                            (XScriptProviderSupplier)UnoRuntime.queryInterface(
                                XScriptProviderSupplier.class, oDoc );
            File logFile = new File( logURI );
                        if ( logFile.exists() )
                        {
                            if ( !logFile.delete() )
                                 throw new java.lang.RuntimeException("failed to delete " + logFileURL );
                        }

                        XScriptProvider xProv = xSupplier.getScriptProvider();
                        XScript xScript = xProv.getScript("vnd.sun.star.script:Standard.TestMacros.Main?language=Basic&location=document");
                        System.out.println("Got script for doc " + entries[ i ] );
                        Object[][] out = new Object[1][0];
                        out[0] = new Object[0];

                        short[][] num = new short[1][0];
                        num[0] = new short[0];

                        xScript.invoke(new Object[0], num, out);

            logFile = new File( logURI );
                           System.out.println( "fileName is " + entries[ i ].getName().substring(0,  entries[ i ].getName().lastIndexOf( ".xls" ) ) );
                        File newLoc = new File( outDir,  entries[ i ].getName().substring(0,  entries[ i ].getName().lastIndexOf( ".xls" ) ) + ".log" );
            System.out.println("Moving logfile to " + newLoc.getAbsolutePath() );
            boolean ret = logFile.renameTo( newLoc );

            System.out.println("Have run and move of log file worked = " + ret );
                    }
                    catch ( Exception ex )
                    {
                       ex.printStackTrace(System.err);

                    }
                    // interface is supported, otherwise use XComponent.dispose
                    com.sun.star.util.XCloseable xCloseable =
                        (com.sun.star.util.XCloseable)UnoRuntime.queryInterface(
                            com.sun.star.util.XCloseable.class, oDoc);

                    if ( xCloseable != null )
                    {
                        xCloseable.close(false);
                    }
                    else
                    {
                        com.sun.star.lang.XComponent xComp =
                            (com.sun.star.lang.XComponent)UnoRuntime.queryInterface(
                                com.sun.star.lang.XComponent.class, oDoc);

                        xComp.dispose();
                    }
                }
                catch( Exception e )
                {
                    e.printStackTrace(System.err);
                }

            }
        }
}
public static void usage()
{
    System.out.println("usage: java  TestVba.java " +
                "\"<path to testdocument dir>\" " +
                " \"<output_directory>\"");
        System.exit(1);

}
public static  void  main( String[] args )
{
    if ( !( args.length > 1 ) )
        usage();

    com.sun.star.uno.XComponentContext xContext = null;

        try
    {
            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");

            // get the remote office service manager
            com.sun.star.lang.XMultiComponentFactory xMCF =
                xContext.getServiceManager();

            Object oDesktop = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xContext);

            com.sun.star.frame.XComponentLoader  xCompLoader = (com.sun.star.frame.XComponentLoader)
                UnoRuntime.queryInterface(com.sun.star.frame.XComponentLoader.class,
                                          oDesktop);

            File testDir = new File(args[0]);
            File outDir = new File(args[1]);
            if ( !testDir.isDirectory() || !outDir.isDirectory() )
                usage();
            TestVBA inst = new TestVBA( xContext, xMCF, xCompLoader, outDir );
            inst.traverse( testDir );
            System.out.println("Log dir is " + inst.getLogLocation() );

    }
    catch( Exception e )
    {
        System.out.println("Caught exception");
        System.out.println(e);
    }
    System.exit(0);
}
}
