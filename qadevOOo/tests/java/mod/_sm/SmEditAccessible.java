/*************************************************************************
 *
 *  $RCSfile: SmEditAccessible.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change:$Date: 2003-09-08 12:29:19 $
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

package mod._sm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessible</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessible
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessible
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleText
 */
public class SmEditAccessible extends TestCase {
    XComponent xMathDoc;

    /**
     * Creates a <code>StarMath</code> document and obtains an accessibility
     * component with the role <code>AccessibleRole.PANEL</code>.
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );
        try {
            xMathDoc = SOF.openDoc("smath","_blank");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.io.IOException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        }

        // setting a formula in document
        final String expFormula = "sum hat a";
        final XPropertySet xPS = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, xMathDoc);
        try {
            xPS.setPropertyValue("Formula", expFormula);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        }

        XInterface oObj = null;

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xMathDoc);


        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PANEL, "", "SmEditAccessible");

        log.println("ImplementationName " + utils.getImplName(oObj));
        //at.printAccessibleTree(log,xRoot);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("Destroy", new Boolean(true));

        final XAccessibleContext con = (XAccessibleContext) UnoRuntime.queryInterface(XAccessibleContext.class, oObj);
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        System.out.println("Childs "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", "sum hat x \n int a \n sum b");
                        shortWait();
                        System.out.println("Childs "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", expFormula);
                        shortWait();
                    } catch(com.sun.star.lang.WrappedTargetException e) {
                    } catch(com.sun.star.lang.IllegalArgumentException e) {
                    } catch(com.sun.star.beans.PropertyVetoException e) {
                    } catch(com.sun.star.beans.UnknownPropertyException e) {
                    }
                }
            });

        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}    // finish class SmModel
