package installer;

import java.io.*;
import javax.swing.JLabel;

public class FileUpdater {

    public static boolean updateProtocolHandler( String installPath, JLabel statusLabel ) {
            File in_file = null;
            FileInputStream in = null;
            File out_file = null;
            FileWriter out = null;
            int count = 0;

            try {
                in_file = new File( installPath+File.separator+"share"+File.separator+"registry"+File.separator+"data"+File.separator+"org"+File.separator+"openoffice"+File.separator+"Office"+File.separator+"ProtocolHandler.xcu" );

        String[] xmlArray = new String[50];
        try {
            BufferedReader reader = new BufferedReader(new FileReader(in_file));
            count = -1;
                for (String s = reader.readLine(); s != null; s = reader.readLine()) { //</oor:node>
                count = count + 1;
                if(s != null) {
                    s.trim();
                    xmlArray[count] = s;
                }
                else
                    break;
            }
        }
        catch( IOException ioe ) {
            String message = "\nError reading ProtocolHandler.xcu, please view SFrameworkInstall.log.";
            System.out.println(message);
            ioe.printStackTrace();
            statusLabel.setText(message);
            return false;
        }

        in_file.delete();

                out_file = new File( installPath+File.separator+"share"+File.separator+"registry"+File.separator+"data"+File.separator+"org"+File.separator+"openoffice"+File.separator+"Office"+File.separator+"ProtocolHandler.xcu" );
                out_file.createNewFile();
                out = new FileWriter( out_file );

        for(int i=0; i<count + 1; i++) {
                    out.write(xmlArray[i]+"\n");
                    if( ( xmlArray[i].indexOf( "<node oor:name=\"HandlerSet\">" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptProtocolHandler" ) == -1 ) ) {
                        out.write( "        <node oor:name=\"com.sun.star.comp.ScriptProtocolHandler\" oor:op=\"replace\">\n" );
                        out.write( "            <prop oor:name=\"Protocols\">\n" );
                        out.write( "                <value>script:*</value>\n" );
                        out.write( "            </prop>\n" );
                        out.write( "        </node>\n" );
                     }
                }
            }
            catch( Exception e ) {
        String message = "\nError updating ProtocolHandler.xcu, please view SFrameworkInstall.log.";
                System.out.println(message);
        e.printStackTrace();
        statusLabel.setText(message);
        return false;
            }
            finally {
                try {
                    out.close();
                    System.out.println("File closed");
                }
                catch(Exception e) {
                    System.out.println("Update ProtocolHandler Failed, please view SFrameworkInstall.log.");
            System.err.println(e);
            e.printStackTrace();
                }
            }
        return true;

    }// updateProtocolHandler


        public static boolean updateScriptXLC( String installPath, JLabel statusLabel ) {

            File in_file = null;
            FileInputStream in = null;
            File out_file = null;
            FileWriter out = null;
            int count = 0;

        //System.out.println("updateScriptXLC");
            try {
                in_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"script.xlc" );

        String[] xmlArray = new String[50];
        try {
            BufferedReader reader = new BufferedReader(new FileReader(in_file));
            count = -1;
                for (String s = reader.readLine(); s != null; s = reader.readLine()) { //</oor:node>
                count = count + 1;
                if(s != null) {
                    s.trim();
                    xmlArray[count] = s;
                }
                else
                    break;
            }
        }
        catch( IOException ioe ) {
            String message = "Error reading script.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            ioe.printStackTrace();
            statusLabel.setText(message);
            return false;
        }

        in_file.delete();

                out_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"script.xlc" );
                out_file.createNewFile();
                out = new FileWriter( out_file );

                //split the string into a string array with one line of xml in each element
                //String[] xmlArray = xmlLine.split("\n");
        for(int i=0; i<count + 1; i++) {
                    out.write(xmlArray[i]+"\n");
                    if( ( xmlArray[i].indexOf( "<library:libraries xmlns:library" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptBindingLibrary" ) == -1 ) ) {
            String opSys = System.getProperty("os.name");
            if (opSys.indexOf("Windows") != -1) {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"true\"/>\n" );
            }
            else {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/share/basic/ScriptBindingLibrary/script.xlb/\" xlink:type=\"simple\" library:link=\"true\"/>\n" );
            }
                     }
                }
            }
            catch( Exception e ) {
            String message = "\nError updating script.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
            }
            finally {
                try {
                    out.close();
                }
                catch(Exception e) {
                    System.out.println("Update Script.xlc Failed, please view SFrameworkInstall.log.");
            e.printStackTrace();
                    System.err.println(e);
                }
            }
        return true;
        }// updateScriptXLC


        public static boolean updateDialogXLC( String installPath, JLabel statusLabel ) {
            File in_file = null;
            FileInputStream in = null;
            File out_file = null;
            FileWriter out = null;
            int count = 0;

            //System.out.println( "updateDialogXLC" );
            try {
                in_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"dialog.xlc" );
                String xmlLine = "";

        String[] xmlArray = new String[50];
        try {
            BufferedReader reader = new BufferedReader(new FileReader(in_file));
            count = -1;
                for (String s = reader.readLine(); s != null; s = reader.readLine()) {
                count = count + 1;
                if(s != null) {
                    s.trim();
                    xmlArray[count] = s;
                }
                else
                    break;
            }
        }
        catch( IOException ioe ) {

            String message = "\nError reading dialog.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            statusLabel.setText(message);
            return false;
        }
                in_file.delete();

                out_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"dialog.xlc" );
                out_file.createNewFile();

                out = new FileWriter( out_file );

                //split the string into a string array with one line of xml in each element
                // String[] xmlArray = xmlLine.split("\n");
        for(int i=0; i<count + 1; i++) {
                    out.write(xmlArray[i]+"\n");
                    if( ( xmlArray[i].indexOf( "<library:libraries xmlns:library" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptBindingLibrary" ) == -1 ) ) {
            String opSys = System.getProperty("os.name");
            if (opSys.indexOf("Windows") != -1) {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"true\"/>\n" );
            }
            else {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/share/basic/ScriptBindingLibrary/dialog.xlb/\" xlink:type=\"simple\" library:link=\"true\"/>\n" );
            }
                     }
                }
            }
            catch( Exception e ) {
            String message = "\nError updating dialog.xlc, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
            }
            finally {
                try {
                    out.close();
                }
                catch(Exception e) {
                    System.out.println("Update dialog.xlc Failed, please view SFrameworkInstall.log.");
            e.printStackTrace();
                    System.err.println(e);
                }
            }
        return true;
        }// updateScriptXLC


}
