/*************************************************************************
 *
 *  $RCSfile: RecoveryTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-26 14:30:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package complex.framework.recovery;

import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.presentation.XCustomPresentationSupplier;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.sheet.XSheetCellRange;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.style.XStyle;
import com.sun.star.table.XCellRange;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XSelectionSupplier;
import complexlib.ComplexTestCase;
import helper.OfficeProvider;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Toolkit;
import java.io.PrintWriter;
import java.util.Hashtable;
import java.util.Random;
import util.DesktopTools;
import util.SOfficeFactory;
import util.UITools;

public class RecoveryTest extends ComplexTestCase {

    static XMultiServiceFactory xMSF;
    static SOfficeFactory SOF;
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
    private Hashtable windowsPosSize = new Hashtable();

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
     */
    public String[] getTestMethodNames() {
        return new String[]{"makeCrash",
                            "handleRecoveryWindowAfterCrash"};
//        return new String[]{"generateDesktop",
//                            "makeCrash",
//                            "pause",
//                            "startOffice",
//                            "compareWindowPositions"};
//        return new String[] {"generateDesktop","compareWindowPositions"};
    }

        /** Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     * @throws Exception Exception
     */
    public void before() throws Exception {

        // create TypeDetection
        xMSF = (XMultiServiceFactory)param.getMSF();
        assure("Could not get XMultiServiceFactory", xMSF != null);

    SOF = SOfficeFactory.getFactory(xMSF);

        String office = (String) param.get("AppExecutionCommand");
        if ((office.indexOf("-norestore") != -1) && (office.indexOf("-nocrashreport") != -1)){
            log.println("try to close office...");
            disconnect();
            assure("Could not connect to office", connect());
        }
        // make window ranges
        makeWindowPositionRage();
    }

    public void startOffice(){
        assure("Could not connect to office", connect());
        pause();
        pause();
    }

    private boolean disconnect() {
        try {
            XDesktop desk = null;
            desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, xMSF.createInstance(
                    "com.sun.star.frame.Desktop"));
            xMSF = null;

            desk.terminate();
            log.println("Waiting 5 seconds for the Office to close down");
            try {
                Thread.sleep(5000);
            }
            catch(java.lang.InterruptedException e) {}
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
            failed("Cannot dispose the Office.");
            return false;
        }
        return true;
    }


    public void pause(){
        log.println("PAUSE");
        sleep(5000);
    }

    public boolean connect(){
        String office = (String) param.get("AppExecutionCommand");
        String oldOffice = office;
        try {
            String[] params = {"-norestore", "-nocrashreport"};

            for (int i = 0; i < params.length; i++){
                int index = office.indexOf(params[i]);
                int length = params[i].length();
                if (index != -1){
                    office = office.substring(0, index) + office.substring(index + length);
                    log.println("removed '" + params[i] + "' from AppExecutionCommand: " + office);
                }
            }
            param.put("AppExecutionCommand", office);
            System.out.println("connect: " + (String) param.get("AppExecutionCommand"));

            OfficeProvider oProvider = new OfficeProvider();
            xMSF = (XMultiServiceFactory)oProvider.getManager(param);

            SOF = SOfficeFactory.getFactory(xMSF);

            try {
                Thread.sleep(200);
            }
            catch(java.lang.InterruptedException e) {}
            param.put("AppExecutionCommand",office);
        }
        catch (java.lang.Exception e) {
            param.put("AppExecutionCommand",office);
            log.println(e.getClass().getName());
            log.println("Message: " + e.getMessage());
            failed("Cannot connect the Office.");
            return false;
        }
        return true;
    }

    /**
     * While creating the test environment the positions and sizes of the frames
     * was saved. After the Office has recovered the documents, this functions
     * compares the saved positions and sizes with the current frame.
     */
    public void compareWindowPositions(){
        System.out.println("all frames:########");
        System.out.println(windowsPosSize.entrySet().toString());

        XEnumeration allComp = DesktopTools.getAllComponents(xMSF);

        String msg=null;

        while (allComp.hasMoreElements()){
            try{
                // get all components from the desktop
                XComponent xComponent = (XComponent) UnoRuntime.queryInterface(
                                       XComponent.class, allComp.nextElement());

                XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xComponent);

                String frameName = xModel.getCurrentController().getFrame().getName();

                // check if this frame was used in creation of test environment
                if (windowsPosSize.containsKey(frameName)){

                    Rectangle oldRect = (Rectangle) windowsPosSize.get(frameName);

                    XWindow xWindow = xModel.getCurrentController().getFrame().getContainerWindow();
                    Rectangle newRect = xWindow.getPosSize();


                    boolean ok = oldRect.Height == newRect.Height;
                    ok &= oldRect.Width == newRect.Width;
                    ok &= oldRect.X == newRect.X;
                    ok &= oldRect.Y == newRect.Y;

                    if (!ok){
                        msg = "The frame '" + frameName + "' has a different position/size:\n";
                        msg += "original value -> restored value:\n";
                        msg += "X     : " + oldRect.X + " -> " + newRect.X + "\n";
                        msg += "Y     : " + oldRect.Y + " -> " + newRect.Y + "\n";
                        msg += "Height: " + oldRect.Height + " -> " + newRect.Height + "\n";
                        msg += "Width : " + oldRect.Width + " -> " + newRect.Width + "\n";
                    }

                    assure(msg, ok, CONTINUE);

                }
            } catch (com.sun.star.container.NoSuchElementException e) {
            } catch ( com.sun.star.lang.WrappedTargetException e) {}
        }

    }

    /**
     * This function crashes the office
     */
    public void makeCrash(){
        // get all documents
        Object[] allDocs = DesktopTools.getAllOpenDocuments(xMSF);

        // get one of them for dispatching
        XComponent xDoc = (XComponent) allDocs[0];
        XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xDoc);

        XController xController = xModel.getCurrentController();
        XFrame xFrame = xController.getFrame();
        XDispatchProvider xDispProv = (com.sun.star.frame.XDispatchProvider)
                UnoRuntime.queryInterface(com.sun.star.frame.XDispatchProvider.class,xFrame);
        com.sun.star.util.URL aURL = new com.sun.star.util.URL();
        aURL.Complete = ".uno:Crash";

        Object instance = null;
        try{
            instance = xMSF.createInstance("com.sun.star.util.URLTransformer");
        } catch (com.sun.star.uno.Exception e){
            failed("ERROR: could not create URLTransformer: " + e.toString());
        }
        com.sun.star.util.XURLTransformer atrans =
                (com.sun.star.util.XURLTransformer)UnoRuntime.queryInterface(
                                    com.sun.star.util.XURLTransformer.class,instance);
        com.sun.star.util.URL[] aURLA = new com.sun.star.util.URL[1];
        aURLA[0] = aURL;
        atrans.parseStrict(aURLA);
        aURL = aURLA[0];

        XDispatch xDisp = (com.sun.star.frame.XDispatch)xDispProv.queryDispatch(aURL, "",0);
        xDisp.dispatch(aURL, null);


    }

    public void handleRecoveryWindowAfterCrash(){
        try{
            XInterface xToolKit = null;
            try {
                xToolKit = (XInterface) xMSF.createInstance("com.sun.star.awt.Toolkit") ;
            } catch (com.sun.star.uno.Exception e) {
              failed("Could not get Toolkit: " + e.toString());
            }

            XDialog oDialog = getActiveDialog(xToolKit);
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class, oDialog);

            //util.dbg.printInterfaces(oDialog);
            System.out.println(oDialog.getTitle());

            UITools oUITools = new UITools(xMSF, xWindow);

            oUITools.printAccessibleTree((PrintWriter) log);

            String[] documents = oUITools.getListBoxItems("The following files will be recovered");
            log.println("there are " + documents.length + " documents to save");

            oUITools.clickButton("OK");

            // wait until unrecoveralError-Mesagebox occure
            pause();
            oDialog = getActiveDialog(xToolKit);

            System.out.println(oDialog.getTitle());
            //while (oDialog.getTitle().indexOf("Test")


        } catch (Exception e){
            failed("Could not handle crash-dialog: " + e.toString());
        }

    }

    public XDialog getActiveDialog(XInterface xToolKit){
        XExtendedToolkit tk = (XExtendedToolkit)
            UnoRuntime.queryInterface(XExtendedToolkit.class, xToolKit);
        Object atw = tk.getActiveTopWindow();
        return (XDialog) UnoRuntime.queryInterface(XDialog.class, atw);
    }

    /**
     * This function gets the current screen size and calculate the first
     * quarter of it. This qaurter was used to postion to Office windows.
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

    public void generateDesktop(){

        // create some documents with content
        makeCalcDoc("CalcDoc1", true);
        makeWriterDoc("WriterDoc1", true);
        makeDrawDoc("DrawDoc1", true);
        makeImpressDoc("ImpressDoc1", true);
        makeMathDoc("MathDoc1", true);

        // create some documents without content
        makeMathDoc("MathDocEmpty", false);
        makeDrawDoc("DrawDocEmpty", false);
        makeCalcDoc("CalcDocEmpty", false);
        makeWriterDoc("WriterDocEmpty", false);
        makeImpressDoc("ImpressDocEmpty", false);

        log.println("Test object successfully created.");

    }

    private void makeImpressDoc(String frameName, boolean withContent){
        log.println("creating Impress document '" + frameName + "'");
        XComponent xImpressDoc = createNewImpressDoc(frameName);
        if (withContent) fillImpressDocWithContent(xImpressDoc);
        positioningDocument((XModel) UnoRuntime.queryInterface(XModel.class,
                                                               xImpressDoc));
    }
    private void makeDrawDoc(String frameName, boolean withContent){
        log.println("creating Draw document '" + frameName + "'");
        XComponent xDrawDoc = createNewDrawDoc(frameName);
        if (withContent) fillDrawDocWithContent(xDrawDoc);
        positioningDocument((XModel) UnoRuntime.queryInterface(XModel.class,
                                                                 xDrawDoc));
    }

    private void makeCalcDoc(String frameName, boolean withContent){
        log.println("creating Calc document '" + frameName + "'");
        XSpreadsheetDocument xSpreadsheetDoc = createNewCalcDoc(frameName);
        if (withContent) fillCalcDocWithContent(xSpreadsheetDoc);
        positioningDocument((XModel) UnoRuntime.queryInterface(XModel.class,
                                                           xSpreadsheetDoc));
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

    private void makeMathDoc(String frameName, boolean withContent){
        log.println("creating Math document '" + frameName + "'");
        XComponent xMathDoc = createNewMathDoc(frameName);
        if (withContent) fillMathDocWithContent(xMathDoc);
        positioningDocument((XModel) UnoRuntime.queryInterface(XModel.class,
                                                               xMathDoc));
    }

    private XComponent createNewMathDoc(String frameName){
        XComponent xMathDoc = null;
        try{
            xMathDoc = SOF.createMathDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating math document '"+frameName+"':");
            failed("Couldn't create test environment");
        }
        return xMathDoc;
    }

    private void fillMathDocWithContent(XComponent xMathDoc){
        // setting a formula in document
        final String expFormula = "sum a cdot b";
        final XPropertySet xPS = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, xMathDoc);
        try {
            xPS.setPropertyValue("Formula", expFormula);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while filling math document with content.");
            failed("Couldn't create test environment");
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while filling math document with content.");
            failed("Couldn't create test environment");
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occured while filling math document with content.");
            failed("Couldn't create test environment");
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occured while filling math document with content.");
            failed("Couldn't create test environment");
        }
    }

    private XComponent createNewImpressDoc(String frameName){
        XComponent xImpressDoc = null;
        try{
            xImpressDoc = SOF.createImpressDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating impress document '"+frameName+"':");
            failed("Couldn't create test environment");
        }
        return xImpressDoc;
    }

    private void fillImpressDocWithContent(XComponent xImpressDoc){

        log.println( "get presentation" );
        XPresentationSupplier oPS = (XPresentationSupplier)
            UnoRuntime.queryInterface(XPresentationSupplier.class, xImpressDoc);
        XInterface oObj = oPS.getPresentation();

        log.println( "get custom presentation" );
        XCustomPresentationSupplier oCPS = (XCustomPresentationSupplier)
            UnoRuntime.queryInterface(
                XCustomPresentationSupplier.class, xImpressDoc);
        XNameContainer xCP = oCPS.getCustomPresentations();

        XInterface oInstance = null;

        XSingleServiceFactory oSingleMSF = (XSingleServiceFactory)
            UnoRuntime.queryInterface(XSingleServiceFactory.class, xCP);

        try{
            oInstance = (XInterface) oSingleMSF.createInstance();
        } catch (com.sun.star.uno.Exception e) {
            log.println("Could not create custom presentation while filling impress document with content.");
            failed("Couldn't create test environment");
        }

        try {
            xCP.insertByName("FirstPresentation",oInstance);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Could not instert custom presentation while filling impress document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("Could not instert custom presentation while filling impress document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Could not instert custom presentation while filling impress document with content.");
            failed("Couldn't create test environment");
        }
    }

    private XComponent createNewDrawDoc(String frameName){
        XComponent xDrawDoc = null;
        try{
            xDrawDoc = SOF.createDrawDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating draw document '"+frameName+"':");
            failed("Couldn't create test environment");
        }
        return xDrawDoc;
    }

    private void fillDrawDocWithContent(XComponent xDrawDoc){
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oDPn);
        XDrawPage oDP = null;
        try {
            oDP = (XDrawPage) AnyConverter.toObject(
                        new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Could not get draw pages while filling draw document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Could not get draw pages while filling draw document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Could not get draw pages while filling draw document with content.");
            failed("Couldn't create test environment");
        }

        //get a Shape
        log.println( "getting Shape" );
        XShapes oShapes = (XShapes) UnoRuntime.queryInterface
            (XShapes.class, oDP);
        XInterface oObj = SOF.createShape
            (xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle");
        for (int i=0; i < 10; i++) {
            oShapes.add(
                SOF.createShape(xDrawDoc,
                    5000, 3500, 7510 + 10 * i, 5010 + 10 * i, "Rectangle"));
        }
        XShape oShape = SOF.createShape
            (xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse");
        oShapes.add((XShape) oObj);
        oShapes.add((XShape) oShape);


        XPropertySet oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oObj);
        XStyle aStyle = null;
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
            oShapeProps.setPropertyValue("ZOrder", new Integer(1));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while setting or getting property value while filling draw document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occured while setting or getting property value while filling draw document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while setting or getting property value while filling draw document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception occured while setting or getting property value while filling draw document with content.");
            failed("Couldn't create test environment");
        }
    }

    private void makeWriterDoc(String frameName, boolean withContent){
        log.println("creating Writer document '" + frameName + "'");
        XTextDocument xTextDoc = createNewWriterDoc(frameName);
        if (withContent) fillWriterDocWithContent(xTextDoc);
        positioningDocument((XModel) UnoRuntime.queryInterface(XModel.class,
                                                                 xTextDoc));
    }

    private XTextDocument createNewWriterDoc(String frameName){
        XTextDocument xTextDoc = null;
        try {
            xTextDoc = SOF.createTextDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating text document '"+frameName+"':");
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
            log.println("Exception occured while filling text document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while filling text document with content.");
            failed("Couldn't create test environment");
        }
    }

    private XSpreadsheetDocument createNewCalcDoc(String frameName){

        XSpreadsheetDocument xSheetDoc = null;

        try {
            xSheetDoc = SOF.createCalcDoc(frameName);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while creating calc document '"+frameName+"':");
            failed("Couldn't create test environment");
        }
        return xSheetDoc;
    }

    private void fillCalcDocWithContent(XSpreadsheetDocument xSpreadsheetDoc){

        try{
            XSpreadsheets oSpreadsheets = xSpreadsheetDoc.getSheets();

            XSpreadsheet oSheet = (XSpreadsheet) AnyConverter.toObject(
                             new Type(XSpreadsheet.class),
                             oSpreadsheets.getByName(
                                     oSpreadsheets.getElementNames()[0]));

            XCellRange testRange = oSheet.getCellRangeByName("$A$1:$D$4");

            XSheetCellRange testSheetRange = (XSheetCellRange) UnoRuntime.queryInterface(
                                                     XSheetCellRange.class,
                                                     testRange);
            oSheet.getCellByPosition(1, 1).setValue(1);
            oSheet.getCellByPosition(4, 5).setValue(1);
            oSheet.getCellByPosition(3, 2).setFormula("xTextDoc");
            oSheet.getCellByPosition(3, 3).setFormula("xTextDoc");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occured while filling calc document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Exception occured while filling calc document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occured while filling calc document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occured while filling calc document with content.");
            failed("Couldn't create test environment");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occured while filling calc document with content.");
            failed("Couldn't create test environment");
        }
    }

    private void sleep(long millis){
        try{
            Thread.sleep(millis);
        }catch (java.lang.InterruptedException e){}
    }
}
