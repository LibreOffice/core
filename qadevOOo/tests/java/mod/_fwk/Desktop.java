/*************************************************************************
 *
 *  $RCSfile: Desktop.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:52:19 $
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

package mod._fwk;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.frame.Desktop</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::beans::XPropertySet</code></li>
*  <li><code>com::sun::star::frame::XComponentLoader</code></li>
*  <li><code>com::sun::star::frame::XDesktop</code></li>
*  <li><code>com::sun::star::frame::XDispatchProvider</code></li>
*  <li><code>com::sun::star::frame::XFrame</code></li>
*  <li><code>com::sun::star::frame::XFramesSupplier</code></li>
*  <li><code>com::sun::star::frame::XTasksSupplier</code></li>
*  <li><code>com::sun::star::lang::XComponent</code></li>
*  <li><code>com::sun::star::task::XStatusIndicatorFactory</code></li>
* </ul><p>
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.frame.XComponentLoader
* @see com.sun.star.frame.XDesktop
* @see com.sun.star.frame.XDispatchProvider
* @see com.sun.star.frame.XFrame
* @see com.sun.star.frame.XFramesSupplier
* @see com.sun.star.frame.XTasksSupplier
* @see com.sun.star.lang.XComponent
* @see com.sun.star.task.XStatusIndicatorFactory
* @see ifc.beans._XPropertySet
* @see ifc.frame._XComponentLoader
* @see ifc.frame._XDesktop
* @see ifc.frame._XDispatchProvider
* @see ifc.frame._XFrame
* @see ifc.frame._XFramesSupplier
* @see ifc.frame._XTasksSupplier
* @see ifc.lang._XComponent
* @see ifc.task._XStatusIndicatorFactory
*/
public class Desktop extends TestCase {

    XTextDocument xTextDoc;

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            try {
                xTextDoc.dispose();
            } catch (com.sun.star.lang.DisposedException de) {}
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.frame.Desktop</code>.
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        XInterface oObj = null;

        try {
            oObj = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.comp.framework.Desktop");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException(
                Status.failed("Couldn't create instance"));
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XDispatchProvider.URL", ".uno:Open");

        tEnv.addObjRelation("Desktop",(XDesktop)
                                UnoRuntime.queryInterface(XDesktop.class,oObj));

        return tEnv;
    } // finish method getTestEnvironment

}
