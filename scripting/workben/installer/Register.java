package installer;

import java.lang.String;
import java.io.*;
import javax.swing.*;

public class Register{
    private static String[] singletonDefParams = { "drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava",
                                           "drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager",
                                           "drafts.com.sun.star.script.framework.theScriptRuntimeManager=drafts.com.sun.star.script.framework.ScriptRuntimeManager"};


    private static String quotedString ( String stringToQuote ) {
            String doubleQuote = "\"";
        String result = new String ( doubleQuote + stringToQuote + doubleQuote );
        return result;
    }
    private static boolean regSingletons( String path, String progPath, String opSys, JLabel statusLabel ) {
        try{
            int exitcode=0;
            Runtime rt = Runtime.getRuntime();
            Process p;
            String[] env = new String[1];
            String regCmd = null;
            for ( int i=0; i<singletonDefParams.length; i++){
                if ( opSys.indexOf( "Windows" ) == -1 ){
                // Not windows
                    env[0] = "LD_LIBRARY_PATH=" + progPath;
                    p=rt.exec("chmod a+x " + progPath + "regsingleton");
                    exitcode=p.waitFor();
                    regCmd = progPath + "regsingleton " + path + "user" + File.separator + "uno_packages" + File.separator + "cache" + File.separator + "services.rdb " + singletonDefParams[i];
                    p=rt.exec( regCmd, env );
                }
                else {
            // Windows
                    regCmd = quotedString( progPath + "regsingleton.exe" ) + " " + quotedString( path + "user" + File.separator + "uno_packages" + File.separator + "cache" + File.separator + "services.rdb" ) + " " + quotedString( singletonDefParams[i] );

                    p=rt.exec( regCmd );
                }
                exitcode = p.waitFor();
                if ( exitcode != 0 ){
                    System.out.println("Regsingleton cmd failed, cmd: " + regCmd );
                    statusLabel.setText("Regsingleton ScriptRuntimeForJava Failed, please view SFrameworkInstall.log");
                    return false;
        }
        }
    }
        catch ( Exception e ) {
            String message = "\nError installing scripting package, please view SFrameworkInstall.log.";
            System.out.println(message);
            e.printStackTrace();
            statusLabel.setText(message);
            return false;
        }
        return true;


    }
    public static boolean register(String path, JLabel statusLabel) {
    try {
        String s=null;
        int exitcode=0;
        String env[] = new String[1];
        Runtime rt = Runtime.getRuntime();

        String progpath = path.concat("program" + File.separator);
        Process p;

            statusLabel.setText("Registering Scripting Framework...");
        String opSys = System.getProperty("os.name");

            // pkgchk Scripting Framework Components
            statusLabel.setText("Registering Scripting Framework Components...");
        System.out.println("Registering Scripting Framework Components...");

        if (opSys.indexOf("Windows") == -1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;

        p=rt.exec("chmod a+x "+progpath+"pkgchk");
        exitcode=p.waitFor();
        if (exitcode ==0){
                    //scriptnm.zip is an old SF. Works with SO6.1 EA2
            System.err.println(progpath+"pkgchk "+progpath+"ooscriptframe.zip");
                    p=rt.exec(progpath+"pkgchk "+progpath+"ooscriptframe.zip", env);
                    //p=rt.exec(progpath+"pkgchk "+progpath+"scriptnm.zip" );
                }
            }
        else {
        //progpath = "C:\\Progra~1\\OpenOffice.org643\\program\\";
                //System.out.println( "Windows" );
        //String pkgStr = new String("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"\"");
        System.err.println("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"");
                p=rt.exec("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"");
        }
            exitcode=p.waitFor();
            if (exitcode !=0) {
        if(opSys.indexOf("Windows") == -1){
            System.out.println("\nPkgChk Failed \nCommand: " +progpath+"pkgchk "+progpath+"ooscriptframe.zip"+ "\n"+ env[0]);
        }
        else {
            System.out.println("\nPkgChk Failed \nCommand: \""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"");
        }
        statusLabel.setText("PkgChk Failed, please view SFrameworkInstall.log");
        return false;
        }

            // if ( !regSingletons( path, progpath, opSys, statusLabel ) )
        // {
            //     return false;
            // }
            // updating ProtocolHandler
            statusLabel.setText("Updating ProtocolHandler...");
            if(!FileUpdater.updateProtocolHandler(path, statusLabel)) {
            statusLabel.setText("Updating ProtocolHandler failed, please view SFrameworkInstall.log");
            return false;
        }

            // updating StarBasic libraries
            statusLabel.setText("Updating StarBasic libraries...");
            if(!FileUpdater.updateScriptXLC(path, statusLabel)) {
            statusLabel.setText("Updating user/basic/script.xlc failed, please view SFrameworkInstall.log");
            return false;
        }
            if(!FileUpdater.updateDialogXLC(path, statusLabel)) {
            statusLabel.setText("Updating user/basic/dialog.xlc failed, please view SFrameworkInstall.log");
            return false;
        }

    }
    catch(Exception e){
        String message = "\nError installing scripting package, please view SFrameworkInstall.log.";
        System.out.println(message);
        e.printStackTrace();
        statusLabel.setText(message);
        return false;
    }
    return true;
    }// register

}//Register
