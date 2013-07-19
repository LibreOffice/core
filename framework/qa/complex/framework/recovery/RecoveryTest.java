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

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XWindow;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Toolkit;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Random;
import util.DesktopTools;
import util.PropertyName;
import util.SOfficeFactory;
import util.UITools;
import util.utils;

public class RecoveryTest extends ComplexTestCase {

    static XMultiServiceFactory xMSF;
    static SOfficeFactory SOF;
    static RecoveryTools rt;
    /**
     * If you devid the screen in four parts in the first of them the office
     * windows should be placed. The range of the first quarter is stored in the variable.
     */
    static Point windowMaxPosition;
    /**
     * The office windows starts in the first quarter of the screen. In this variable
     * the maximum size for the windows was stored so the windows can be placed
     * visible on the screen.
     */
    static Size windowMaxSize;

    /**
     * All office windows will be placed by this test on randomized positions.
     * This positions was stored in this Hashmap. The keys are the frame names
     * and the values are com sun.star.awt.Rectangle.
     * @see com.sun.star.awt.Rectangle
     */
    private HashMap<String, Rectangle> windowsPosSize = new HashMap<String, Rectangle>();

    /**
     * A function to tell the framework, which test functions are available.
     * @return All test methods.
     *
     * @todo: hidden documents
     * @todo: running presentation
     * @todo: modular dialogs like Hpyerlink-Dialog
     * @todo: sceond view of a document
     * @todo: remove recovery data before start test
     * @todo: after a second start after the crash there should no documents recovered anymore
     * @todo: enable remove of recovery files
     */
    public String[] getTestMethodNames() {
        return new String[]{"testCrash"};
    }

    /** Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     * @throws Exception Exception
     */

    public void normalCrash(){
        cleanRecoveryData();
        startOffice();
        generateDesktop();
        makeCrash();
        int expectedDocumentCount = windowsPosSize.size() + 1;
        handleRecoveryDialogAfterCrash(expectedDocumentCount);
        startOffice();
        handleRecoveryDialog_QuickExit(expectedDocumentCount);
        handleCrashReporterDialog(true, true);
        checkDocumentCount(expectedDocumentCount);
    }

    public void testCrash(){
        cleanRecoveryData();
        restoreBackupRecoveryData();
        startOffice();
        int expectedDocumentCount = 3;
        handleRecoveryDialog_QuickExitAndSave(expectedDocumentCount);
    }

    public void before() throws Exception {

        String msg ="\n\n\tPATH TO OFFICE BINARY MISSING!\n";
        msg +="\tPlease run your command with the following parameter:\n\n";
        msg +="\t-AppExecutionCommand=OFFICEBINARY CONNECTIONSTRING\n\n";
        msg +="Example Windows:\n";
        msg +="-AppExecutionCommand=C:\\office\\soffice.exe --accept=socket,host=localhost,port=8101;urp;\n\n";
        msg +="Example UNIX:\n";
        msg +="-AppExecutionCommand=/office/soffice \"--accept=socket,host=localhost,port=8101;urp;\"\n\n";
        msg+="NOTE: on UNIX be shure to have the connection string inside quotation mark!\n";

        assure(msg, param.get("AppExecutionCommand") != null && ! param.get("AppExecutionCommand").equals(""));
        System.out.println("HALLO" + param.get("AppExecutionCommand"));
        msg = "\n\nONE PARAMETER IS MISSING!\n";
        msg += "Please append to your command the following parameter:\n\n";
        msg += "\t-NoOffice=true";
        assure(msg, param.getBool("NoOffice"));


        rt = new RecoveryTools(param ,log);

        rt.removeParametersFromAppExecutionCommand();

        log.println("start the office to test recovery feature...");

        // make window ranges
        makeWindowPositionRage();

    }

    private void startOffice(){
        assure("Could not connect to office", connect());
        log.setWatcher(param.get("Watcher"));
    }


    private void checkDocumentCount(int expectedDocumentCount){
        XEnumeration allComp = DesktopTools.getAllComponents(xMSF);
        int documentCount = 0;

        try{
            while (allComp.hasMoreElements()){
                allComp.nextElement();
                documentCount ++;
            }
        }
        catch ( com.sun.star.container.NoSuchElementException e){}
        catch ( com.sun.star.lang.WrappedTargetException e){}

        String msg ="The amount of documents to recover is different form the expected amount:\n";
        msg += "\texpected:\t" + expectedDocumentCount + "\n";
        msg += "\tto recover:\t" + documentCount;

        assure(msg, expectedDocumentCount == documentCount);
    }

    /**
     * This function starts an office instance. It uses the AppExecutionCommad parameter.
     * @return TRUE if office is connected otherwise FALSE
     */
    private boolean connect(){
        try {

            OfficeProvider oProvider = new OfficeProvider();
            xMSF = (XMultiServiceFactory)oProvider.getManager(param);

            SOF = SOfficeFactory.getFactory(xMSF);

        }
        catch (java.lang.Exception e) {
            log.println(e.getClass().getName());
            log.println("Message: " + e.getMessage());
            failed("Cannot connect the Office.");
            return false;
        }
        return true;
    }

    /**
     * This function crashes the office
     */
    private void makeCrash(){
        // get all documents
        Object[] allDocs = DesktopTools.getAllOpenDocuments(xMSF);

        // get one of them for dispatching
        XComponent xDoc = (XComponent) allDocs[0];
        log.println("make the crash in second thread");

        CrashThread crash = new CrashThread(xDoc, xMSF);
        crash.start();
        rt.pause();
        rt.pause();
    }

    /**
     *  This function uses accessibility to handle the dialog which appears while the
     * office is crashed. It click the button "OK" to continue.
     */
    private void handleRecoveryDialogAfterCrash(int expectedDocumentCount){
        try{

            // if the office crashes, the recovery feature needs some time
            // to save all docs. Therefore the recovery dialog could need some
            // time to pop up.
            log.println("wating for recovery dialog...");

            int counter = 0;
            int maximum = param.getInt(PropertyName.THREAD_TIME_OUT) / param.getInt(PropertyName.SHORT_WAIT);

            XDialog oDialog = rt.getActiveDialog(xMSF);

            while ( oDialog == null && (counter < maximum))
            {
                rt.pause();
                oDialog = rt.getActiveDialog(xMSF);
                counter ++;
            }

            assure("could not get Recovery Window",(oDialog != null));

            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, oDialog);

            UITools oUITools = new UITools(xMSF, xWindow);

            oUITools.printAccessibleTree((PrintWriter) log, param.getBool(PropertyName.DEBUG_IS_ACTIVE));

            String[] documents = oUITools.getListBoxItems("The following files will be recovered");
            log.println("there are " + documents.length + " documents to save");

            String msg ="The amount of documents to recover is different form the expected amount:\n";
            msg += "\texpected:\t" + expectedDocumentCount + "\n";
            msg += "\tto recover:\t" + documents.length;

            assure(msg, expectedDocumentCount == documents.length);

            log.println("disable automatically launch of Office");
            oUITools.setCheckBoxValue("Launch StarOffice automatically", new Integer(0));

            log.println("start saving...");
            oUITools.clickButton("OK");

            rt.waitForClosedOffice();

        } catch (Exception e){
            e.printStackTrace();
            failed("Could not handle crash-dialog: " + e.toString());
        }
    }

     private void handleCrashReporterDialog(boolean cancel, boolean YesNo){
        try{

            log.println("try to get Crash Reporter Dialog...");

            XDialog oDialog = rt.getActiveDialog(xMSF);
            assure("could not get CrashReporter Dialog", oDialog != null);

            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, oDialog);

            log.println(oDialog.getTitle());

            UITools oUITools = new UITools(xMSF, xWindow);

            if (cancel) {
                log.println("clicking 'Cancel' button...");

                try{
                    rt.clickThreadButton(xMSF, xWindow, "Cancel");
                } catch (com.sun.star.accessibility.IllegalAccessibleComponentStateException e){
                    failed("Could not click 'Cancel' at CrashReporter Dialog");
                }

            }
            else {
                log.println("clicking 'Next' button...");
                oUITools.clickButton("Next>>");
            }

        } catch (Exception e){
            failed("Could not handle CrashReporter Dialog: " + e.toString());
        }
    }

    private void handleRecoveryDialog_QuickExit(int expectedDocumentCount){
        log.println("handle Recovery Dialog at restart: quick exit");
        handleRecoveryDialogAtRestart(expectedDocumentCount, false, true);
        handleAreYouSureDialog(true);
        handleSaveDocumentsDialog(false);

    }
    private void handleRecoveryDialog_QuickExitAndSave(int expectedDocumentCount){
        log.println("handle Recovery Dialog at restart: quick exit");
        handleRecoveryDialogAtRestart(expectedDocumentCount, false, true);
        handleAreYouSureDialog(true);
        handleSaveDocumentsDialog(true);
    }
     /**
      * This function uses accessibility to handle the dialog which appears while the
      * office is started after a crash. It waits until the "next>>" button is enabled
      * and click it then to continue.
      * @param expectedDocumentCount the amount of documents which must be displayed in the recovery dialog
      * @param recover If the documenst should be recoverd this variable must be true. If it is fasle
      * the recovery process was stoped and the button cancel was klicked.
      * @param cancel If the recovery is fifnished, this parameter desicdes to klick the "Next" button
      * or the click cancel. If the value is true, the cancel button was clicked.
      */
    private void handleRecoveryDialogAtRestart(int expectedDocumentCount, boolean recover, boolean cancel){
        try{

            log.println("try to get Recovery Dialog...");

            XDialog oDialog = null;
            oDialog = rt.getActiveDialogAfterStartup(xMSF);

            assure("could not get Recovery Dialog at start of office", (oDialog != null), CONTINUE);

            XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, oDialog);
            log.println("got the following dialog: '" +oDialog.getTitle() + "'");

            UITools oUITools = new UITools(xMSF, xWindow);

            String listBoxName = "Status of recovered documents";
            String[] documents = oUITools.getListBoxItems(listBoxName);
            log.println("there are " + documents.length + " documents to recover");
            log.println("The following files will be recovered:");
            for (int i=0;i<documents.length;i++){
                log.println(documents[i]);
            }

            String msg ="The amount of documents to recover is different form the expected amount:\n";
            msg += "\texpected:\t" + expectedDocumentCount + "\n";
            msg += "\tto recover:\t" + documents.length;

            assure(msg, expectedDocumentCount ==documents.length);

            if (recover){

                log.println("clicking 'Start Recovery' button...");
                oUITools.clickButton("Start Recovery >");

                rt.pause();

                int counter = 0;
                int maximum = param.getInt(PropertyName.THREAD_TIME_OUT) / param.getInt(PropertyName.SHORT_WAIT);

                XAccessibleContext oButton = null;
                while ((oButton == null) && (counter < maximum)){
                    log.println("recovering...");

                    try{
                       oButton = oUITools.getButton("Next >");
                    } catch (java.lang.NullPointerException e){
                        // no fault: The title "Start Recovery" switches to "Next"
                        // while all documents are recoverd
                    }
                    rt.pause();
                    counter++;
                }

                if (cancel) {
                    log.println("clicking 'Cancel' button...");

                    try{
                        rt.clickThreadButton(xMSF, xWindow, "Cancel");
                    } catch (com.sun.star.accessibility.IllegalAccessibleComponentStateException e){
                        failed("Could not click 'Cancel' at recovery-dialog.");
                    }

                }
                else {
                    log.println("clicking 'Next' button...");
                    oUITools.clickButton("Next >");
                }

                rt.pause();

            } else {
                    log.println("do not recover: clicking 'Cancel' button...");

                    try{
                        rt.clickThreadButton(xMSF, xWindow, "Cancel");
                    } catch (com.sun.star.accessibility.IllegalAccessibleComponentStateException e){
                        failed("Could not click 'Cancel' at recovery-dialog");
                    }
            }

        } catch (Exception e){
            failed("Could not handle recovery-dialog at restart: " + e.toString());
        }

    }

    /**
     * This function uses accessibility to handle the dialog "Are you sure".
     * It click "Yes" or "No", dependend on the value of the parameter <CODE>Yes</CODE>
     * @param yes If value is <CODE>TRUE</CODE> the button "Yes" was clicked, otherwise the button
     * "No".
     */
    private void handleAreYouSureDialog(boolean yes)
    {
        try{
            if (yes){
                rt.handleModalDialog(xMSF, "Yes");
            } else{
                rt.handleModalDialog(xMSF, "Cancel");
            }
        } catch (com.sun.star.accessibility.IllegalAccessibleComponentStateException e){
            failed("Could not handle 'Are you sure' dialog.");
        }
    }

    /**
     * This function uses accessibility to handle the dialog "Are you sure".
     * It click "Yes" or "No", depending on the value of the parameter <CODE>Yes</CODE>
     * @param saveDocuments If value is <CODE>TRUE</CODE> the button "Yes" was clicked, otherwise the button
     * "No".
     */
    private void handleSaveDocumentsDialog(boolean saveDocuments)
    {
        try{
            if (!saveDocuments){
                rt.handleModalDialog(xMSF, "Cancel");
            } else{
                XWindow oDialog = null;
                oDialog = rt.getActiveWindow(xMSF);

                assure("could not get 'Save Documents' Dialog: ", (oDialog != null), CONTINUE);

                UITools oUITools = new UITools(xMSF, oDialog);

                oUITools.printAccessibleTree((PrintWriter) log, param.getBool(PropertyName.DEBUG_IS_ACTIVE));

                String listBoxName = "Documents";
                String[] documents = null;
                try{
                    documents = oUITools.getListBoxItems(listBoxName);
                } catch (java.lang.Exception e){
                    failed("could not get the document names from the 'Save Documents' dialog", CONTINUE);
                }
                log.println("there are " + documents.length + " documents to save");
                log.println("The following documents will be saved:");
                for (int i=0;i<documents.length;i++){
                    log.println(documents[i]);
                }
                String tempURL = utils.getOfficeTempDir(xMSF);

                log.println("the destination for saveing is: " + tempURL);
                try{
                    oUITools.setTextEditFiledText("Save to", tempURL);
                } catch (java.lang.Exception e){
                    failed("could not set target directory for saving documents at 'Save Documents' dialog", CONTINUE);
                }
                try{
                    oUITools.clickButton("OK");
                } catch (java.lang.Exception e){
                    failed("could not click 'OK' at 'Save Documents' dialog", CONTINUE);
                }
            }
        } catch (com.sun.star.accessibility.IllegalAccessibleComponentStateException e){
            failed("Could not handle 'Are you sure' dialog.");
        }
    }

    /**
     * This function gets the current screen size and calculate the first
     * quarter of it. This quarter was used to position to Office windows.
     * Further this function calculates the maximum window size so the window
     * is visible if it placed on extreme position.
     */
    private void makeWindowPositionRage(){
        Dimension screenDim = Toolkit.getDefaultToolkit().getScreenSize();
        Point pos = new Point();
        Size size = new Size();

        // get the max position of the first quarter of the screen
        pos.x = screenDim.width / 2;
        pos.y = screenDim.height / 2;
        windowMaxPosition = pos;

        // get the max size of the windows while they placed in windowMaxPosition
        // range and not outside the visibility
        size.Height = screenDim.height;
        size.Width = screenDim.width;
        windowMaxSize = size;
    }

    private void generateDesktop(){

        // create some documents with content
        makeWriterDoc("WriterDoc1", true);


        makeWriterDoc("WriterDocEmpty", false);

        log.println("Test object successfully created.");

    }

    private void positioningDocument(XModel model){

        XWindow xWindow = model.getCurrentController().getFrame().getContainerWindow();
        String frameName = model.getCurrentController().getFrame().getName();

        // get randomized position and size
        Rectangle posSize = makePosZize();

        // save position and size
        windowsPosSize.put(frameName, posSize);

        xWindow.setPosSize(posSize.X, posSize.Y, posSize.Width, posSize.Height,
                           com.sun.star.awt.PosSize.POSSIZE);
        Rectangle test = xWindow.getPosSize();
        log.println("x: "+test.X+" y:"+test.Y+" width:"+test.Width+" height:"+test.Height);
    }

    private Rectangle makePosZize(){

        Rectangle posSize = new Rectangle();
        Random rand = new Random();

        // Random integers that range from from 0 to n
        posSize.X = rand.nextInt(windowMaxPosition.x + 1);
        posSize.Y = rand.nextInt(windowMaxPosition.y + 1);

        int maxHeight = windowMaxSize.Height-posSize.X;
        int maxWidth = windowMaxSize.Width-posSize.Y;
        int height = rand.nextInt(maxHeight + 1);
        int width = rand.nextInt((windowMaxSize.Width-posSize.Y) + 1);

        // be shure that the new size his greater then the half of windowMaxSize
        posSize.Height = (height < (maxHeight / 2)) ? height + (maxHeight / 2) : height;
        posSize.Width =  (width < (maxWidth / 2)) ? width + (maxWidth / 2) : width;

        return posSize;
    }

    private void makeWriterDoc(String frameName, boolean withContent){
        log.println("creating Writer document '" + frameName + "'");
        XTextDocument xTextDoc = createNewWriterDoc(frameName);
        if (withContent) fillWriterDocWithContent(xTextDoc);
        positioningDocument(UnoRuntime.queryInterface(XModel.class,
                                                                 xTextDoc));
    }

    private XTextDocument createNewWriterDoc(String frameName){
        XTextDocument xTextDoc = null;
        try {
            xTextDoc = SOF.createTextDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred while creating text document '"+frameName+"':");
            failed("Couldn't create test environment");
        }
        return xTextDoc;
    }

    private void fillWriterDocWithContent(XTextDocument xTextDoc){
        try{
            log.println( "inserting some lines" );
            XText oText = xTextDoc.getText();
            XTextCursor oCursor = oText.createTextCursor();
            for (int i=0; i<5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertString( oCursor,
                " The quick brown fox jumps over the lazy Dog: SwXParagraph",
                false);
                oText.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertString( oCursor,
                "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                false);
                oText.insertControlCharacter(oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            log.println("Exception occurred while filling text document with content.");
            failed("Couldn't create test environment");
        }
    }

    /**
     * copies all files from the backup.recoveryTest folder into the backup folder
     * and copies the Recovery.xcu.recoveryTest to recovery.xcu
     */
    private void restoreBackupRecoveryData()
    {
        log.println("restore backup recovery data...");
        try{
            rt.copyRecoveryData(false);
        }catch (com.sun.star.io.IOException e){
            failed("could not copy recovery data: " + e.toString());
        }catch (java.io.IOException e){
            failed("could not copy recovery data: " + e.toString());
        }
    }

    private void cleanRecoveryData(){
        try{
            log.println("bootstrapping the office to get user path to remove old recovery data...");

            rt.cleanRecoveryData();

        } catch (com.sun.star.io.IOException e){
            failed("could not clean recovery data: " + e.toString());
        }
    }
}
