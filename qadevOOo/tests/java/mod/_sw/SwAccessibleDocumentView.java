/*************************************************************************
 *
 *  $RCSfile: SwAccessibleDocumentView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sw $
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

import lib.TestCase;
import lib.TestParameters;
import java.io.PrintWriter;
import lib.TestEnvironment;
import com.sun.star.uno.XInterface;
import util.SOfficeFactory;
import lib.StatusException;
import com.sun.star.text.XTextDocument;
import util.WriterTools;
import com.sun.star.frame.XController;
import util.AccessibilityTools;
import drafts.com.sun.star.accessibility.AccessibleRole;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.frame.XModel;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.XWindow;
import drafts.com.sun.star.accessibility.XAccessible;
import util.utils;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPage;

/**
* Test of accessible object for the text document.<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>drafts::com::sun::star::accessibility::XAccessible</code></li>
* </ul>
* @see drafts.com.sun.star.accessibility.XAccessible
*/
public class SwAccessibleDocumentView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects. The method is called from
    * <code>getTestEnvironment()</code>. Obtains accissible object for
    * text document.
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

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xTextDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        oObj = at.SearchedContext;

        log.println("ImplementationName " + utils.getImplName(oObj));
        at.printAccessibleTree(log, xRoot);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLLBAR);
        final XAccessibleValue xAccVal = (XAccessibleValue) UnoRuntime.queryInterface
                                (XAccessibleValue.class, SearchedContext) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    xAccVal.setCurrentValue(xAccVal.getMinimumValue());
                    xAccVal.setCurrentValue(xAccVal.getMaximumValue());
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
     * Called while the <code>TestCase</code> initialization.
     * Creates a text document.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc(Param.getMSF());
    }
}
