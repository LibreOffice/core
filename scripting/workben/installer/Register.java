package installer;

import java.lang.String;
import java.io.*;
import javax.swing.*;

public class Register{
    
    private static JProgressBar progressBar;

    // not required
    private static void unx(String url,String path, javax.swing.JLabel statusLabel){
        System.out.println("\nFound a Unix path:"+path);
    System.out.println("\nThis Script does not support Unix");
    }// unx

    private static void windows(String url,String path, javax.swing.JLabel statusLabel){
    try {
        String s=null;
        String classpath="";
        int exitcode=0;
        boolean passed=true;
        String env[] = new String[1]; 
        Runtime rt= Runtime.getRuntime();
        String progpath=path.concat(File.separator+"program"+File.separator);
        BufferedReader stdInput;
        Process p;
            
            statusLabel.setText("Registering Scripting Framework...");
            progressBar.setString("Registering Scripting Framework ");
            progressBar.setValue(7);
        String opSys =System.getProperty("os.name");

            // pkgchk Scripting Framework Components
            statusLabel.setText("Registering Scripting Framework Components...");
        if (opSys.indexOf("Windows")==-1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;
        
        p=rt.exec("chmod a+x "+progpath+"pkgchk");
        exitcode=p.waitFor();
        if (exitcode ==0){
                    //scriptnm.zip is an old SF. Works with SO6.1 EA2
                    p=rt.exec(progpath+"./pkgchk "+progpath+"ooscriptframe.zip", env);
                    //p=rt.exec(progpath+"pkgchk "+progpath+"scriptnm.zip" );
                }
            }
        else {
        //progpath = "C:\\Progra~1\\OpenOffice.org643\\program\\";
                System.out.println( "Windows" );
        String pkgStr = new String("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"\"");
        System.err.println("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"");
                p=rt.exec("\""+progpath+"pkgchk.exe\" \""+progpath+"ooscriptframe.zip\"");
        }
            exitcode=p.waitFor();   
            if (exitcode !=0){
        System.out.println("\n PkgChk Failed!");
        passed=false;
        }

            // regsingleton ScriptRuntimeForJava
            statusLabel.setText("Registering Singleton ScriptRuntimeForJava...");
        if (opSys.indexOf("Windows")==-1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;
                
                /*
                p=rt.exec("mkdir /scriptdev/neil/DeleteThisVV");
        exitcode=p.waitFor();
        if (exitcode ==0) {
                    System.out.println( "mkdir cmd succeeded" );
                }else{ System.out.println( "mkdir cmd failed" ); }
                */
                
        p=rt.exec("chmod a+x "+progpath+"regsingleton");
        exitcode=p.waitFor();
        if (exitcode ==0)
                    p=rt.exec(progpath+"./regsingleton "+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava", env );
            }
        else {
                System.out.println( "Windows" );
        //path = "C:\\Progra~1\\OpenOffice.org643";
        //progpath = path + "\\program\\";
        System.out.println("\""+progpath+"regsingleton.exe\" \""+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\"");
        p=rt.exec("\""+progpath+"regsingleton.exe\" \""+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava\"");
        }
            exitcode=p.waitFor();   
            if (exitcode !=0){
        //System.out.println("\n Regsingleton ScriptRuntimeForJava Failed!");
        passed=false;
        }            

            
            // regsingleton ScriptStorageManager
            statusLabel.setText("Registering Singleton ScriptStorageManager...");
        if (opSys.indexOf("Windows")==-1){
                //System.out.println( "Not Windows");
        env[0]="LD_LIBRARY_PATH="+progpath;
                
        p=rt.exec("chmod a+x "+progpath+"regsingleton");
        exitcode=p.waitFor();
        if (exitcode ==0)
                    p=rt.exec(progpath+"./regsingleton "+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager", env );
            }
        else {
                //System.out.println( "Windows" );
        System.out.println("\""+progpath+"regsingleton.exe\" \""+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\"");
                p=rt.exec("\""+progpath+"regsingleton.exe\" \""+path+File.separator+"user"+File.separator+"uno_packages"+File.separator+"cache"+File.separator+"services.rdb\" \"drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager\"");
        }
            exitcode=p.waitFor();   
            if (exitcode !=0){
        //System.out.println("\n Regsingleton ScriptStorageManager Failed!");
        passed=false;
        }               
        
        // Commands:
        //regsingleton <Office Installation>/user/uno_packages/cache/services.rdb drafts.com.sun.star.script.framework.theScriptRuntimeForJava=drafts.com.sun.star.script.framework.ScriptRuntimeForJava
        //regsingleton <Office Installation>/user/uno_packages/cache/services.rdb drafts.com.sun.star.script.framework.storage.theScriptStorageManager=drafts.com.sun.star.script.framework.storage.ScriptStorageManager

            
            // updating ProtocolHandler
            statusLabel.setText("Updating ProtocolHandler...");            
            FileUpdater.updateProtocolHandler(path);
            
            // updating StarBasic libraries
            statusLabel.setText("Updating StarBasic libraries...");            
            FileUpdater.updateScriptXLC(path);
            FileUpdater.updateDialogXLC(path);
    }
    catch(Exception e){
        System.out.println("Error:"+e);
    }
    }// windows
    
 

    public static void register(String path, javax.swing.JLabel statusLabel,JProgressBar pBar){
        progressBar=pBar;
    boolean win =false;
    String newString= "file://";
    if (path.indexOf(":")==1){
            newString=newString.concat("/");
        win=true;
    }
    String tmpStr1="";
    String tmpStr2="";
    newString=newString.concat(path.replace('\\','/'));
    int i=0;
    if (newString.indexOf(" ")>0){
            tmpStr1=tmpStr1.concat(newString.substring(i,newString.indexOf(" ")));
            tmpStr1=tmpStr1.concat("\\ ");
        newString=newString.substring(newString.indexOf(" ")+1,newString.length()); 
    }
    tmpStr1=tmpStr1.concat(newString);
    //System.out.println(""+tmpStr1);
    char url[]=path.toCharArray();
    char test[]=new char[path.length()*2];
    int j=0;
    
    path = path.substring (0,path.length()-1);
    windows(tmpStr1,path, statusLabel);
    
    }// register

}//Register
