/*************************************************************************
 *
 *  $RCSfile: SwAccessiblePageView.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $
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
package mod._sw;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.WriterTools;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class SwAccessiblePageView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Switchs the document to Print Preview mode.
    * Obtains accissible object for the page view.
    *
    * @param tParam test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment()
    */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
        try {
            for (int i=0; i<25; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertString( oCursor,
                    " The quick brown fox jumps over the lazy Dog: SwAccessibleDocumentPageView",
                    false);
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertString( oCursor,
                    "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwAccessibleDocumentPageView",
                    false);
                oText.insertControlCharacter(oCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertControlCharacter(
                    oCursor, ControlCharacter.LINE_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace(log);
            throw new StatusException( "Couldn't insert lines", e );
        }

        shortWait();

        XController xController = xTextDoc.getCurrentController();

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xTextDoc);

        //switch to 'Print Preview' mode
        try {
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
            ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
                xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
            throw new StatusException(Status.failed("Couldn't change mode"));
        }

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot);

        System.out.println("Panel: "+AccessibleRole.PANEL);
        System.out.println("ScrollPane: "+AccessibleRole.SCROLL_PANE);
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL,"Page");

        log.println("ImplementationName " + utils.getImplName(oObj));


        TestEnvironment tEnv = new TestEnvironment(oObj);

        getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLL_BAR);
        final XAccessibleValue xAccVal = (XAccessibleValue) UnoRuntime.queryInterface
                                (XAccessibleValue.class, SearchedContext) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    Integer old = (Integer) xAccVal.getCurrentValue();
                    Integer newValue = new Integer(old.intValue()+10);
                    xAccVal.setCurrentValue(newValue);
                    xAccVal.setCurrentValue(old);
                }
            });

        return tEnv;

    }

    public static boolean first = false;
    public static XAccessibleContext SearchedContext = null;

    public static void getAccessibleObjectForRole(XAccessible xacc,short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        if (ac.getAccessibleRole()==role) {
            if (first) SearchedContext = ac;
                else first=true;
        } else {
            int k = ac.getAccessibleChildCount();
            for (int i=0;i<k;i++) {
                try {
                    getAccessibleObjectForRole(ac.getAccessibleChild(i),role);
                    if (SearchedContext != null) return ;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }
    }


    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes text document.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("dispose text document");
        xTextDoc.dispose();
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc((XMultiServiceFactory)Param.getMSF());
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}
