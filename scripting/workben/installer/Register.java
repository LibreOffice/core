package installer;

import java.lang.String;
import java.io.*;
import javax.swing.*;

public class Register{
    
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

            // regsingleton ScriptRuntimeForJava
            statusLabel.setText("Registering Singleton ScriptRuntimeForJava...");
        System.out.println("Registering Singleton ScriptRuntimeForJava...");
        if (opSys.indexOf("Windows")==-1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;

        p=rt.exec("chmod a+x "+progpath+"regsingleton");
        exitcode=p.waitFor();
        if (exitcode ==0)
            System.out.println(progpath+"regsingleton "+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava");
                    p=rt.exec(progpath+"regsingleton "+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava", env );
            }
        else {
                //System.out.println( "Windows" );
        //path = "C:\\Progra~1\\OpenOffice.org643";
        //progpath = path + "\\program\\";
        System.out.println("\""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\"");
        p=rt.exec("\""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\"");
        }
            exitcode=p.waitFor();   
            if (exitcode !=0){
        //System.out.println("\n Regsingleton ScriptRuntimeForJava Failed!");
        if(opSys.indexOf("Windows")==-1){
            System.out.println("\nRegsingleton ScriptRuntimeForJava Failed.\nCommand: "+progpath+"regsingleton "+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\n" + env[0] );
        }
        else {
            System.out.println("\nRegsingleton ScriptRuntimeForJava Failed.\nCommand: \""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\"");
        }
        statusLabel.setText("Regsingleton ScriptRuntimeForJava Failed. please view SFrameworkInstall.log");
        return false;
        }            

            
            // regsingleton ScriptStorageManager
            statusLabel.setText("Registering Singleton ScriptStorageManager...");
        System.out.println("Registering Singleton ScriptStorageManager...");
        if (opSys.indexOf("Windows")==-1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;
                
        p=rt.exec("chmod a+x "+progpath+"regsingleton");
        exitcode=p.waitFor();
        if (exitcode ==0)
                    p=rt.exec(progpath+"regsingleton "+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager", env );
            }
        else {
                //System.out.println( "Windows" );
        System.out.println("\""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\"");
                p=rt.exec("\""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\"");
        }
            exitcode=p.waitFor();   
            if (exitcode !=0){
        //System.out.println("\n Regsingleton ScriptStorageManager Failed!");
        if(opSys.indexOf("Windows")==-1){
            System.out.println("\nRegsingleton ScriptRuntimeForJava Failed.\nCommand: " +progpath+"regsingleton "+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\n" + env[0] );
        }
        else {
            System.out.println("\nRegsingleton ScriptRuntimeForJava Failed.\nCommand: \""+progpath+"regsingleton.exe\" \""+path+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\"");
        }
        statusLabel.setText("Regsingleton ScriptRuntimeForJava Failed, please view SFrameworkInstall.log");
        return false;
        }               
        
        // Commands:
        //regsingleton <Office Installation>/user/uno_packages/cache/services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava
        //regsingleton <Office Installation>/user/uno_packages/cache/services.rdb drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager

            
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
