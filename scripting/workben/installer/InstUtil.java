package installer;

import java.net.URLDecoder;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.net.*;

public class InstUtil {
    public static boolean createTmpDir() {
        String sysTemp = System.getProperty("java.io.tmpdir");
        if (tmpDir == null) {
            tmpDir = new File(sysTemp + File.separator + "xmergeinst");
            if (tmpDir.exists())
                return true;
        }
        
        return tmpDir.mkdir();
    }
    
    public static File getTmpDir() {
        return tmpDir;
    }
    
    public static void removeTmpDir() {
        if ((tmpDir != null) && (tmpDir.exists())) {
            File types = new File(tmpDir, "TypeDetection.xml");
            if (types.exists()) 
            {
                if (types.delete())
                {
                    //System.out.println("Success:type");;
                }
            }
            File java = new File(tmpDir, "Java.xml");
            if (java.exists()) java.delete();
            File rdb = new File(tmpDir, "applicat.rdb");
            if (rdb.exists()) rdb.delete();
            tmpDir.delete();
            tmpDir = null;
        }
    }
    
    public static boolean copy(File inputFile, File outputFile) {
        try {
            FileInputStream in = new FileInputStream(inputFile);
            FileOutputStream out = new FileOutputStream(outputFile);
            byte buffer[] = new byte[4096];
            int c;
            
            while ((c = in.read(buffer)) != -1) {
                //out.write(buffer, 0, c);
                out.write(buffer, 0, c);
            }
            
            in.close();
            out.close();
        } catch (IOException eIO) {
            return false;
        }
        
        return true;
    }
    
    public static File buildSversionLocation() throws IOException {
        File theFile = null;
        StringBuffer str = new StringBuffer();
        String sep = System.getProperty("file.separator");
        str.append(System.getProperty("user.home"));
        str.append(sep);
        StringBuffer thePath = new StringBuffer(str.toString());
        
        String os = System.getProperty("os.name");
        
        if (os.indexOf("Windows") != -1) {
            /*String appData = winGetAppData;
            if (os.equals("Windows 2000")) {
                thePath.append("Application Data");
                thePath.append(sep);
                theFile = new File(thePath.toString());
            } else if (os.indexOf("Windows") != -1) {
                thePath.append(sep);
                thePath.append("sversion.ini");
                theFile = new File(thePath.toString());
                if (!theFile.exists())
                {
                    thePath.delete(0, thePath.length());
                    thePath.append(str);
                    thePath.append("Application Data");
                    thePath.append(sep);
                    thePath.append("sversion.ini");
                    theFile = new File(thePath.toString());
                }
            }*/
            theFile = findVersionFile(new File(str.toString()));
        } else if (os.indexOf("SunOS") != -1) {
            thePath.append(".sversionrc");
            theFile = new File(thePath.toString());
        } else if (os.indexOf("Linux") != -1) {
            thePath.append(".sversionrc");
            theFile = new File(thePath.toString());
        }
        
        if (theFile == null) 
        {
            throw new IOException("Could not locate the OpenOffice settings file.\nAre you sure StarOffice is installed on your system?");
        }
        if  (!theFile.exists()) 
        {
            throw new IOException("Could not locate the OpenOffice settings file.\nAre you sure StarOffice is installed on your system?");
        }
        return theFile;
    }

    
    
    public static boolean hasNetbeansInstallation() {
    File theFile = null;
    StringBuffer str = new StringBuffer();
        String sep = System.getProperty("file.separator");
        str.append(System.getProperty("user.home"));
        str.append(sep);
    StringBuffer thePath = new StringBuffer(str.toString());    
    thePath.append(".netbeans");
    theFile = new File(thePath.toString());
    
    return theFile.isDirectory();
    }       
 

    public static boolean hasJeditInstallation() {
    File theFile = null;
    StringBuffer str = new StringBuffer();
        String sep = System.getProperty("file.separator");
        str.append(System.getProperty("user.home"));
        str.append(sep);
    StringBuffer thePath = new StringBuffer(str.toString());    
    thePath.append(".jedit");
    theFile = new File(thePath.toString());
    
    return theFile.isDirectory();
    }   
       
    
    
    public static Properties getNetbeansLocation() throws IOException {
    File theFile = null;
    Properties results = new Properties();
        
    StringBuffer str = new StringBuffer();
        String sep = System.getProperty("file.separator");
        str.append(System.getProperty("user.home"));
        str.append(sep);
    StringBuffer thePath = new StringBuffer(str.toString());
        
        String os = System.getProperty("os.name");
        
    if (os.indexOf("Windows") != -1) {
        //theFile = findVersionFile(new File(str.toString()));
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("SunOS") != -1) {
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("Linux") != -1) {
        thePath.append(".netbeans");
        //theFile = new File(thePath.toString());
    }

    if ( thePath.toString().indexOf( ".netbeans" ) == -1 )
        return null;
    else if ( new File( thePath.append( sep+"3.4"+sep ).toString() ).isDirectory() ) {
        System.out.println( "Found NetBeans 3.4 on user home Directory " + thePath );
        File netbeansLogFile = new File( thePath.toString() + sep + "system" + sep + "ide.log" );
        if( netbeansLogFile.exists() ) {
            String installPath = getNetbeansInstallation( netbeansLogFile );
            File f = new File(installPath);
            results.put("NetBeans 3.4", f.getPath()+File.separator);
            System.out.println( "f.getPath() " + f.getPath()+File.separator );
        }
        else {
            System.out.println( "Prompt user for NetBeans installation path" );
        }
    }
    
    
    return results; 
    }    
    


    public static Properties getJeditLocation() throws IOException {
    
    if( !hasJeditInstallation() ) {
        System.out.println( "No Jedit found (line195 InstUtil");
        return null;    
    }
    
    File theFile = null;
    Properties results = new Properties();
        
    StringBuffer str = new StringBuffer();
        String sep = System.getProperty("file.separator");
        str.append(System.getProperty("user.home"));
        str.append(sep);
    StringBuffer thePath = new StringBuffer(str.toString());
        
        String os = System.getProperty("os.name");
        thePath.append(".jedit");
    //System.out.println( ".jedit path " + thePath );
    
    /*
    if (os.indexOf("Windows") != -1) {
        //theFile = findVersionFile(new File(str.toString()));
        thePath.append(".jedit");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("SunOS") != -1) {
        thePath.append(".jedit");
        //theFile = new File(thePath.toString());
        } else if (os.indexOf("Linux") != -1) {
        thePath.append(".jedit");
        //theFile = new File(thePath.toString());
    }
    */

    File jeditLogFile = new File( thePath.toString() + sep + "activity.log" );
    if( jeditLogFile.exists() ) {
        String[] jeditDetails = getJeditInstallation( jeditLogFile );
        System.out.println( "installPath (line 229) " + jeditDetails[0] );
        File f = new File(jeditDetails[0]);
        results.put("jEdit "+jeditDetails[1], jeditDetails[0]);
        System.out.println( "jeditDetails[0] is " + jeditDetails[0]);
    }
    else {
        System.out.println( "Prompt user for Jedit installation path" );
    }
    
    
    return results; 
    }



    
    
    private static String getNetbeansInstallation( File logFile ) {
        String installPath = "";
        try {
        BufferedReader reader = new BufferedReader(new FileReader(logFile));
        
        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
        s.trim();
        if( s.indexOf( "IDE Install" ) != -1 ) {
            int pathStart = s.indexOf( "=" ) + 2;
            //System.out.println( "pathStart " + pathStart );
            installPath = s.substring( pathStart, s.length() );
            //System.out.println( "installPath 1" + installPath );
            int pathEnd = installPath.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            installPath = installPath.substring( 0, pathEnd ) +File.separator;
            //System.out.println( "pathStart " + pathStart );
            //int pathEnd = s.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //System.out.println( "s is " + s + " and " + s.length() + " long" );
            //installPath = s.substring( pathStart, pathEnd - 1 );
            installPath.trim();
            System.out.println( "Netbeans installPath (line267 InstUtil " + installPath );
            break;
        }
        }
        }
        catch( IOException ioe ) {
        System.out.println( "Error reading Netbeans location information" );    
            }
        //catch( FileNotFoundException fnfe ) {
        //System.out.println( "NetBeans ide.log FileNotFoundException" );
        //}
        
        return installPath;
    }       

    
    private static String[] getJeditInstallation( File logFile ) {
        String[] jeditDetails = new String[2];
        try {
        BufferedReader reader = new BufferedReader(new FileReader(logFile));
        String installPath = "";
        String version = "";
        
        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
        s.trim();
        if( s.indexOf( "jEdit home directory is" ) != -1 ) {
            int pathStart = s.indexOf( "is /" ) + 3;
            //System.out.println( "pathStart " + pathStart );
            installPath = s.substring( pathStart, s.length() ) +File.separator;
            System.out.println( "installPath 1" + installPath );
            //int pathEnd = installPath.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //installPath = installPath.substring( 0, pathEnd ) +File.separator;
            //System.out.println( "pathStart " + pathStart );
            //int pathEnd = s.indexOf( ";");
            //System.out.println( "pathEnd " + pathEnd );
            //System.out.println( "s is " + s + " and " + s.length() + " long" );
            //installPath = s.substring( pathStart, pathEnd - 1 );
            installPath.trim();
            //System.out.println( "installPath 2 " + installPath );
            //break;
            jeditDetails[0] = installPath;
        }
        if( s.indexOf( "jEdit: jEdit version" ) != -1 ) {
            int versionStart = s.indexOf( "version" ) + 8;
            System.out.println( "versionStart is: " + versionStart );
            version = s.substring( versionStart, s.length() );
            version.trim();
            System.out.println( "jEdit version is: " + version );
            jeditDetails[1] = version;
        }
        }
        }
        catch( IOException ioe ) {
        System.out.println( "Error reading Jedit location information" );    
            }
        //catch( FileNotFoundException fnfe ) {
        //System.out.println( "Jedit activity.log FileNotFoundException" );
        //}
        
        return jeditDetails;
    }

    
    
    public static File findVersionFile(File start)
    {
        File versionFile = null;

        File files[] = start.listFiles(new VersionFilter());
        if (files.length == 0)
        {
            File dirs[] = start.listFiles(new DirFilter());
            for (int i=0; i< dirs.length; i++)
            {
                versionFile = findVersionFile(dirs[i]);
                if (versionFile != null)
                {
                    break;
                }
            }
        }
        else
        {
            versionFile = files[0];
        }

        return versionFile;
    }   
    
    public static boolean verifySversionExists(File sversionFile) {
        if (!sversionFile.exists())
            return false;
        return true;
    }
    
    public static Properties getOfficeVersions(File sversionFile) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(sversionFile));
        Vector values;
        String sectionName = null;
        Properties results = new Properties();
        
        for (String s = reader.readLine(); s != null; s = reader.readLine()) {
            s.trim();
            //System.out.println(s);
            if (s.length() == 0)
                continue;
            if (s.charAt(0) == '[') {
                sectionName = s.substring(1, s.length() - 1);
                //System.out.println(sectionName);
                continue;
            }
            if ((sectionName != null) && sectionName.equalsIgnoreCase("Versions")) {
                int equals = s.indexOf( "=" );
        String officeName = s.substring(0, equals );
        
        String instPath = s.substring(equals + 8, s.length());
        String [] parts = new String[2];
        parts[0] = officeName;
        parts[1] = instPath + File.separator;
        //System.out.println( "InstUtil officeName " + officeName );
        //System.out.println( "InstUtil instPath " + instPath );
        
        //String [] parts = s.split("=");
                if (parts.length == 2) {
                    //ver.version = parts[0].trim();
                    //File f = new File(parts[1].trim());
            //results.put(parts[0].trim(), f.getPath());
            try {
                        URL url = new URL("file://" + parts[1].trim());
            System.err.println( "InstUtil URLDecoder path: " + URLDecoder.decode( url.getPath() ) );
                        //File f = new File(url);
            
            //.sversion: OpenOffice.org 643=file:///scriptdev/neil/ScriptFrameOpenoffice1.0.1
            // parts = Installation name. f.getPath = Installation path
                        //results.put(parts[0].trim(), f.getPath());
                        results.put(parts[0].trim(), URLDecoder.decode(url.getPath()));
            
                        //System.out.println("Putting " + parts[0] + " : " + f.getPath());
                    }
                    catch (MalformedURLException eSyntax) {
                        //throw new IOException("Error while reading version information");
            results.put(parts[0].trim(), parts[1].trim());
            //System.out.println(parts[0].trim() + " : " + parts[1].trim());
            System.err.println("GotHereException");
                    }
                }
                else {
                    System.out.println("not splitting on equals");
                }
            }
        }
        
        return results;
    }
    
    public static String getJavaVersion() {
        return System.getProperty("java.version");
    }
    
    public static boolean isCorrectJavaVersion() {
        if (System.getProperty("java.version").startsWith("1.4"))
            return true;
        return false;
    }
    
    public static void main(String args[]) {
        InstUtil inst = new InstUtil();
        File f = null;
        try
        {
            f = inst.buildSversionLocation();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            System.out.println(e.getMessage());
        }
        if (!inst.verifySversionExists(f)) {
            System.err.println("Problem with sversion.ini");
        }
        try {
            Properties vers = inst.getOfficeVersions(f);
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println(e);
        }
        System.out.println(inst.getJavaVersion());
        if (!inst.isCorrectJavaVersion()) {
            System.err.println("Not correct Java Version");
        }
    }
    
    private static File tmpDir = null;
}



class DirFilter implements java.io.FileFilter
{
    public boolean accept(File aFile)
    {
        return aFile.isDirectory();
    }
}
class VersionFilter implements java.io.FileFilter
{
    public boolean accept(File aFile)
    {
        if (aFile.getName().compareToIgnoreCase("sversion.ini") == 0)
        {
            return true;
        }

        return false;
    }
}
