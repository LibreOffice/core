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
            boolean goodResult = false;
            String[] env = new String[1];
            String regCmd = null;
            ExecCmd command = new ExecCmd();
            for ( int i=0; i<singletonDefParams.length; i++){
                if ( opSys.indexOf( "Windows" ) == -1 ){
                // Not windows
                    env[0] = "LD_LIBRARY_PATH=" + progPath;
                    command.exec( "chmod a+x " + progPath + "regsingleton", null );
                    regCmd = progPath + "regsingleton " + path + "user" + File.separator + "uno_packages" + File.separator + "cache" + File.separator + "services.rdb " + singletonDefParams[i];
                    goodResult = command.exec( regCmd, env );
                }
                else {
            // Windows
                    regCmd = quotedString( progPath + "regsingleton.exe" ) + " " + quotedString( path + "user" + File.separator + "uno_packages" + File.separator + "cache" + File.separator + "services.rdb" ) + " " + quotedString( singletonDefParams[i] );
                    goodResult = command.exec( regCmd,null );
                }
                if ( !goodResult ){
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
        String[] packages = {"ooscriptframe.zip", "bshruntime.zip"};

    try {
        String s=null;
        boolean goodResult = false;
        String env[] = new String[1];
            ExecCmd command = new ExecCmd();
        boolean isWindows =
                (System.getProperty("os.name").indexOf("Windows") != -1);

        String progpath = path.concat("program" + File.separator);

            statusLabel.setText("Registering Scripting Framework...");

            // pkgchk Scripting Framework Components
            statusLabel.setText("Registering Scripting Framework Components...");
        System.out.println("Registering Scripting Framework Components...");

            for (int i = 0; i < packages.length; i++) {
                String cmd = "";

            if (!isWindows) {
            env[0]="LD_LIBRARY_PATH=" + progpath;

            goodResult = command.exec("chmod a+x " + progpath + "pkgchk", null );

            if ( goodResult ){
                        cmd = progpath + "pkgchk " + progpath + packages[i];

                System.err.println(cmd);
                        goodResult = command.exec(cmd, env);
                    }
                }
            else {
                    cmd = "\"" + progpath + "pkgchk.exe\" \"" + progpath +
                        packages[i] + "\"";

            System.err.println(cmd);
                    goodResult =command.exec(cmd,null);

            }
                if (!goodResult) {
                    System.err.println("\nPkgChk Failed");

            if(!isWindows)
                System.err.println("Command: " + cmd + "\n" + env[0]);
            else
            System.err.println("Command: \"" + cmd + "\"");

            statusLabel.setText(
                        "PkgChk Failed, please view SFrameworkInstall.log");

            return false;
                }
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
