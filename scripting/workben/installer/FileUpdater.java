package installer;

import java.io.*;

public class FileUpdater {

    public static void updateProtocolHandler( String installPath ) {
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
                //System.out.println(count + s);
                if(s != null) {
                    s.trim();
                    xmlArray[count] = s;
                }
                else
                    break;
            }
        }
        catch( IOException ioe ) {
            System.out.println( "Error reading Netbeans location information" );
        }

        in_file.delete();

                out_file = new File( installPath+File.separator+"share"+File.separator+"registry"+File.separator+"data"+File.separator+"org"+File.separator+"openoffice"+File.separator+"Office"+File.separator+"ProtocolHandler.xcu" );
                out_file.createNewFile();
                out = new FileWriter( out_file );

        for(int i=0; i<count + 1; i++) {
                    //System.err.println(xmlArray[i]);
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
                System.out.println("\n Update ProtocolHandler Failed!");
                System.err.println(e);
            }
            finally {
                try {
                    out.close();
                    System.out.println("File closed");
                }
                catch(Exception e) {
                    System.out.println("\n Update ProtocolHandler Failed! (Write error)");
                    System.err.println(e);
                }
            }
    }// updateProtocolHandler

        /*
        public static void main( String[] args ) {
            FileUpdater.updateProtocolHandler( "/scriptdev/neil/openoffice1.0.1ScriptFrame" );
            FileUpdater.updateStarBasicXLC( "/scriptdev/neil/openoffice1.0.1ScriptFrame" );
        }*/

        public static void updateScriptXLC( String installPath ) {

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
                //System.out.println(count + s);
                if(s != null) {
                    s.trim();
                    xmlArray[count] = s;
                }
                else
                    break;
            }
        }
        catch( IOException ioe ) {
            System.out.println( "Error reading updateScriptXLC information" );
        }

        in_file.delete();


        /*
            File in_file = null;
            FileInputStream in = null;
            File out_file = null;
            FileWriter out = null;
            int count = 0;

            System.out.println("updateScriptXLC");
            try {
                in_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"script.xlc" );

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
            System.out.println( "Error reading Netbeans location information" );
        }



                in_file.delete();
                */

                out_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"script.xlc" );
                out_file.createNewFile();
                // FileWriter
                //out = new FileOutputStream( out_file );
                out = new FileWriter( out_file );

                //split the string into a string array with one line of xml in each element
                //String[] xmlArray = xmlLine.split("\n");
                //for(int i=0; i<xmlArray.length; i++) {
        for(int i=0; i<count + 1; i++) {
                    //System.out.println(xmlArray[i]);
                    //out.write(xmlArray[i].getBytes());
                    out.write(xmlArray[i]+"\n");
                    //if( ( xmlArray[i].indexOf( "<node oor:name=\"HandlerSet\">" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptProtocolHandler" ) == -1 ) ) {
                    if( ( xmlArray[i].indexOf( "<library:libraries xmlns:library" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptBindingLibrary" ) == -1 ) ) {
//<library:library library:name="Depot" xlink:href="file:///scriptdev/neil/openoffice1.0.1ScriptFrame/share/basic/Depot/script.xlb/" xlink:type="simple" library:link="true" library:readonly="false"/>
//<library:library library:name="Standard" xlink:href="file:///scriptdev/neil/openoffice1.0.1ScriptFrame/user/basic/Standard/script.xlb/" xlink:type="simple" library:link="false"/>
                        //System.out.println(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/user/basic/ScriptBindingLibrary/script.xlb/\" xlink:type=\"simple\" library:link=\"false\"/>\n" );

            String opSys = System.getProperty("os.name");
            if (opSys.indexOf("Windows") != -1) {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"false\"/>\n" );
            }
            else {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/user/basic/ScriptBindingLibrary/script.xlb/\" xlink:type=\"simple\" library:link=\"false\"/>\n" );
            }
                     }
                }
            }
            catch( Exception e ) {
                System.out.println("\n Update Script.xlc Failed!");
                System.err.println(e);
            }
            finally {
                try {
                    out.close();
                    //System.out.println("File closed");
                }
                catch(Exception e) {
                    System.out.println("\n Update Script.xlc Failed! (Write error)");
                    System.err.println(e);
                }
            }
        }// updateScriptXLC


        public static void updateDialogXLC( String installPath ) {
    //System.out.println( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"dialog.xlc" );
            File in_file = null;
            FileInputStream in = null;
            File out_file = null;
            FileWriter out = null;
            int count = 0;

            //System.out.println( "updateDialogXLC" );
            try {
                in_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"dialog.xlc" );
                //in = new FileInputStream( in_file );
                //parse the xml file
                //byte[] buffer = new byte[4096];
                //int bytes_read = 0;
                String xmlLine = "";

        String[] xmlArray = new String[50];
        try {
            BufferedReader reader = new BufferedReader(new FileReader(in_file));
            count = -1;
                for (String s = reader.readLine(); s != null; s = reader.readLine()) {
                //System.out.println( s );
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
            System.out.println( "Error reading Netbeans location information" );
        }


        /*
                while((bytes_read = in.read(buffer)) != -1) {
                    //read the contents of the file into a string
                    String tempXMLLine = new String(buffer, 0, bytes_read);
                    xmlLine = xmlLine + tempXMLLine;
                }
        */

                //in.close();
                in_file.delete();

                out_file = new File( installPath+File.separator+"user"+File.separator+"basic"+File.separator+"dialog.xlc" );
                out_file.createNewFile();
                // FileWriter
                //out = new FileOutputStream( out_file );
                out = new FileWriter( out_file );

                //split the string into a string array with one line of xml in each element
                // String[] xmlArray = xmlLine.split("\n");
                //for(int i=0; i<xmlArray.length; i++) {
        for(int i=0; i<count + 1; i++) {
                    //System.out.println(xmlArray[i]);
                    //out.write(xmlArray[i].getBytes());
                    out.write(xmlArray[i]+"\n");
                    //if( ( xmlArray[i].indexOf( "<node oor:name=\"HandlerSet\">" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptProtocolHandler" ) == -1 ) ) {
                    if( ( xmlArray[i].indexOf( "<library:libraries xmlns:library" ) != -1 ) && ( xmlArray[i+1].indexOf( "ScriptBindingLibrary" ) == -1 ) ) {
//<library:library library:name="Depot" xlink:href="file:///scriptdev/neil/openoffice1.0.1ScriptFrame/share/basic/Depot/script.xlb/" xlink:type="simple" library:link="true" library:readonly="false"/>
//<library:library library:name="Standard" xlink:href="file:///scriptdev/neil/openoffice1.0.1ScriptFrame/user/basic/Standard/script.xlb/" xlink:type="simple" library:link="false"/>
                        //System.out.println( " <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/user/basic/ScriptBindingLibrary/dialog.xlb/\" xlink:type=\"simple\" library:link=\"false\"/>\n" );
            String opSys = System.getProperty("os.name");
            if (opSys.indexOf("Windows") != -1) {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" library:link=\"false\"/>\n" );
            }
            else {
                out.write(" <library:library library:name=\"ScriptBindingLibrary\" xlink:href=\"file://"+installPath+"/user/basic/ScriptBindingLibrary/dialog.xlb/\" xlink:type=\"simple\" library:link=\"false\"/>\n" );
            }
                     }
                }
            }
            catch( Exception e ) {
                System.out.println("\n Update Dialog.xlc Failed!");
                System.err.println(e);
            }
            finally {
                try {
                    out.close();
                    //System.out.println("File closed");
                }
                catch(Exception e) {
                    System.out.println("\n Update Dialog.xlc Failed! (Write error)");
                    System.err.println(e);
                }
            }
        }// updateScriptXLC


}
