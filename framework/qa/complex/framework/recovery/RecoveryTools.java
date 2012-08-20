/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.framework.recovery;

import com.sun.star.awt.XDialog;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.NamedValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import helper.FileTools;
import helper.OfficeProvider;
import helper.UnoProvider;
import java.io.File;
import java.io.PrintWriter;
import java.util.HashMap;
import lib.TestParameters;
import share.LogWriter;
import util.PropertyName;
import util.UITools;
import util.utils;

/**
 * this class supports the <CODE>RecoverTest</CODE>. You will find here some helper
 * functions.
 */
public class RecoveryTools {

    private final TestParameters param;
    private final LogWriter log;

    /**
     * Creates new OfficeWatcher
     * @param param the test parameter
     * @param log a log writer
     */
    public RecoveryTools(TestParameters param, LogWriter log) {
        this.param = param;
        this.log = log;

    }

    /**
     * get the active dialog from the top of the desktop
     * @param xToolKit xToolKit the <CODE> XExtendedToolkit</CODE> to get the dialog from the top of the desktop.
     * @return a <CODE>XDialog</CODE> interface of the dialog
     */
    public XDialog getActiveDialog( XMultiServiceFactory xMSF){
        XWindow xWin = getActiveWindow(xMSF);
        return UnoRuntime.queryInterface(XDialog.class, xWin);
    }

    public XWindow getActiveWindow( XMultiServiceFactory xMSF){
        XInterface xToolKit = null;
        try {
            xToolKit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
          return null;
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, xToolKit);
        Object atw = tk.getActiveTopWindow();
        return UnoRuntime.queryInterface(XWindow.class, atw);
    }

    /**
     * After a crash the office start with a recovery diaolg. It could be that the office
     * is connectable but not all services to get the dialog a loaded. This function
     * tries to get the dialog until the <CODE>OfficeWatcher</CODE> kills the office.
     * @param xToolKit the <CODE> XExtendedToolkit</CODE> to get the dialog from the top of the desktop.
     * @return a <CODE>XDialog</CODE> interface of the dialog
     */
    public XDialog getActiveDialogAfterStartup(XMultiServiceFactory xMSF){
        // while the office starts it takes some time to get the dialog.

        // the dialog is accessible AFTER the office has recoverd all documents.
        // This could consumes more time then the TimeOut allow.
        int counter = 0;
        int multi = 5;
        int pause = param.getInt(PropertyName.SHORT_WAIT)*10;
        int timeOut = param.getInt(PropertyName.THREAD_TIME_OUT)*5;
        int maximum = (timeOut / pause) * multi;

        XDialog oDialog = getActiveDialog(xMSF);

        while (oDialog == null && (counter < maximum)){
            log.println("waiting until the office has recoverd... remaining " + (timeOut * multi - pause * counter)/1000 + " seconds");
            pause(pause);
            oDialog = getActiveDialog(xMSF);
            counter ++;
        }
        return oDialog;
    }

    /**
     * halt the thread for some time
     */
    public void pause(){
       pause(param.getInt(PropertyName.SHORT_WAIT));
    }

    /**
     * halt the thread for some time
     */
    public void pause(int sleepTime){
        sleep(sleepTime);
    }

    private void sleep(long millis){
        try{
            Thread.sleep(millis);
        }catch (java.lang.InterruptedException e){}
    }

    /**
     * remove the content of the user backup folder and removes the Recovery.xcu. This
     * was done from the Office via XSimpleFileAccess
     * @throws com.sun.star.io.IOException the exception was thrown if something goes wrong.
     */
    public void cleanRecoveryData()
        throws com.sun.star.io.IOException
    {
        try{
            HashMap<String, String> recFiles = getRecoveryFiles();

            String recoveryFolder = recFiles.get("recoveryFolder");
            String recoveryXCU = recFiles.get("recoveryXCU");

            log.println("try to remove content of '" + recoveryFolder + "'");

            File rf = new File(recoveryFolder);

            boolean success = FileTools.cleanDir(rf);
            log.println("removed " + recoveryFolder + ": " + success);

            log.println("try to remove '" + recoveryXCU + "'");

            File xcu = new File(recoveryXCU);
            if (xcu.isFile()){
                success = xcu.delete();
                log.println("removed " + recoveryXCU + " : " + success);
            }

        } catch (Exception e){
            throw new com.sun.star.io.IOException("could not remove old recovery data: " + e.toString());
        }
    }

    public HashMap<String, String> getRecoveryFiles()
        throws com.sun.star.io.IOException
    {
        try{
            log.println("try to get UnoProvider...");
            UnoProvider unoProv = new UnoProvider();
            XMultiServiceFactory xMSF = (XMultiServiceFactory) unoProv.getManager(param);

            String userPath = utils.expandMacro(xMSF, "${$ORIGIN/bootstraprc:UserInstallation}");
            System.out.println("userPath:'" + userPath + "'");

            if (userPath.equals(""))userPath = utils.expandMacro(xMSF, "${$ORIGIN/bootstrap.ini:UserInstallation}");
            System.out.println("userPath:'" + userPath + "'");

            if (userPath.equals("")) throw new com.sun.star.io.IOException("could not get user path at bootstraping");

            String recoveryFolder = utils.getSystemURL(userPath + "/user/backup");

            String recoveryXCU = utils.getSystemURL(userPath + "/user/registry/data/org/openoffice/Office/Recovery.xcu");

            HashMap<String, String> recFiles = new HashMap<String, String>();

            recFiles.put("recoveryFolder", recoveryFolder);
            recFiles.put("recoveryXCU", recoveryXCU);
            return recFiles;

        } catch (Exception e){
            throw new com.sun.star.io.IOException("could not get recovery folder: " + e.toString());
        }

    }
    /**
     * This function close the office while calling terminate on the desktop. If
     * this failed, the <CODE>ProcessHandler</CODE> kills the process.
     * @param xMSF the <CODE>XMultiServiceFactory</CODE>
     * @return <CODE>TRUE</CODE> if no exception was thrown, otherwise <CODE>FALSE</CODE>
     */
    public boolean closeOffice(XMultiServiceFactory xMSF) {
        try {
            XDesktop desk = UnoRuntime.queryInterface(
                    XDesktop.class, xMSF.createInstance(
                    "com.sun.star.frame.Desktop"));
            xMSF = null;

            desk.terminate();
            log.println("Waiting until ProcessHandler loose the office...");

        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
            return false;
        }
        waitForClosedOffice();
        return true;
    }

    /**
     * This function waits until the office is closed. If the closing time reach
     * the value of parameter <CODE>THREAD_TIME_OUT</CODE> the office was killed.
     */
    public void waitForClosedOffice(){
        // check for the office process
        helper.ProcessHandler ph = (helper.ProcessHandler) param.get("AppProvider");

        int timeOut = param.getInt(PropertyName.THREAD_TIME_OUT)*5;
        int pause = param.getInt(PropertyName.SHORT_WAIT)*20;
        int multi = 0;
        while ((ph != null) && (ph.getExitCode()<0) && (pause*multi < timeOut)) {
            log.println("waiting until the office is closed... remaining " + (timeOut - pause * multi)/1000 + " seconds");
            pause(pause);
            multi ++;
        }

        // be shure that office is closed
        if (ph != null) ph.kill();
    }

    public void killOffice(){
        helper.ProcessHandler ph = (helper.ProcessHandler) param.get("AppProvider");
        ph.kill();
    }

    /**
     * The office must be started WITH restore and crashreporter functionality.
     * Therefore the paramater '<CODE>--norestore</CODE>' and '<CODE>--nocrashreport</CODE>'
     * was removed from the <CODE>AppExecutionCommand</CODE> parameter
     */
    public void removeParametersFromAppExecutionCommand(){

        //remove some params to start office
        String office = (String) param.get("AppExecutionCommand");
        String[] params = {"--norestore", "--nocrashreport"};

        for (int i = 0; i < params.length; i++){
            int index = office.indexOf(params[i]);
            int length = params[i].length();
            if (index != -1){
                office = office.substring(0, index) + office.substring(index + length);
                log.println("removed '" + params[i] + "' from AppExecutionCommand: " + office);
            }
        }
        param.put("AppExecutionCommand", office);
        log.println("connect: " + (String) param.get("AppExecutionCommand"));

    }

    /**
     * This function uses accessibility to handle modal dialogs like the
     * "Are you sure" dialog.
     * It clicks the named button given in parameter <CODE>buttonName</CODE>
     * @param buttonName the name of the button wich should be clicked
     */
    public void handleModalDialog(XMultiServiceFactory xMSF, String buttonName)
                throws com.sun.star.accessibility.IllegalAccessibleComponentStateException
    {

        log.println("try to get modal Dialog...");

        pause();

        XWindow oDialog = getActiveWindow(xMSF);

        if (oDialog == null) throw new com.sun.star.accessibility.IllegalAccessibleComponentStateException("could not get modal Dialog");


        UITools oUITools = new UITools(xMSF, oDialog);
        oUITools.printAccessibleTree((PrintWriter) log, param.getBool(PropertyName.DEBUG_IS_ACTIVE));

        try{
            log.println("click ' " + buttonName + "' button..");
            oUITools.clickButton(buttonName);
        } catch ( java.lang.Exception e){
            throw new com.sun.star.accessibility.IllegalAccessibleComponentStateException("Could not click '"+buttonName +"' at modal dialog: " + e.toString());
        }
        pause();
    }

    public void clickThreadButton(XMultiServiceFactory xMSF, XWindow xWindow, String buttonName)
                throws com.sun.star.accessibility.IllegalAccessibleComponentStateException
    {
        KlickButtonThread kbt = new KlickButtonThread(xMSF, xWindow, buttonName);
        kbt.start();
        pause(param.getInt(PropertyName.SHORT_WAIT) * 10);
    }

    public void copyRecoveryData(boolean backup)
        throws com.sun.star.io.IOException, java.io.IOException
    {
        HashMap<String, String> recFiles = null;

        try{
            recFiles = getRecoveryFiles();
        } catch ( com.sun.star.io.IOException e){
            throw new  com.sun.star.io.IOException("Could not get recovery files: " + e.toString());
        }

        try{
            String recoveryFolder = recFiles.get("recoveryFolder");
            String recoveryXCU = recFiles.get("recoveryXCU");

            File recFolder = new File(recoveryFolder);
            File recFolderBackup = new File(recoveryFolder+".recoveryTest");

            File recXCU = new File(recoveryXCU);
            File recXCUBackup = new File(recoveryXCU + ".recoveryTest");

            if (backup){
                FileTools.copyDirectory(recFolder, recFolderBackup);
                FileTools.copyFile(recXCU, recXCUBackup);
            } else {
                FileTools.copyDirectory(recFolderBackup, recFolder);
                FileTools.copyFile(recXCUBackup, recXCU);

            }
        } catch (java.io.IOException e){
            throw new java.io.IOException("Could not copy recovery files: " + e.toString());
        }
   }


}
