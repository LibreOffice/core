/*************************************************************************
 *
 *  $RCSfile: DispatchRecorder.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-10-06 13:31:50 $
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

import com.sun.star.beans.PropertyValue;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.SysUtils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.DispatchStatement;
import com.sun.star.frame.XDispatchRecorder;
import com.sun.star.frame.XDispatchRecorderSupplier;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.util.URL;
import util.utils;

/**
* Test for object that implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::container::XElementAccess</code></li>
*  <li><code>com::sun::star::container::XIndexAccess</code></li>
*  <li><code>com::sun::star::container::XIndexReplace</code></li>
*  <li><code>com::sun::star::frame::XDispatchRecorder</code></li>
* </ul><p>
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XIndexReplace
* @see com.sun.star.frame.XDispatchRecorder
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XIndexReplace
* @see ifc.frame._XDispatchRecorder
*/
public class DispatchRecorder extends TestCase {
    XComponent oDoc = null;

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.frame.Desktop</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        XFrame xFrame = null;
        XDispatchRecorder xDR = null;

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)Param.getMSF());
            oDoc = SOF.createTextDoc(null);
            try {
                Thread.sleep(1000);
            }
            catch (InterruptedException ex) {
            }

            xFrame = SysUtils.getActiveFrame((XMultiServiceFactory)Param.getMSF());

            XPropertySet xFramePS = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class, xFrame);
            XDispatchRecorderSupplier xDRS = null;
            xDRS = (XDispatchRecorderSupplier) AnyConverter.toObject(
                             new Type(XDispatchRecorderSupplier.class),
                          xFramePS.getPropertyValue("DispatchRecorderSupplier"));
            if (xDRS == null) {

                Object oDRS = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                    "com.sun.star.comp.framework.DispatchRecorderSupplier");
                xFramePS.setPropertyValue("DispatchRecorderSupplier", oDRS);
                xDRS = (XDispatchRecorderSupplier)
                    UnoRuntime.queryInterface(XDispatchRecorderSupplier.class,oDRS);
            }

            xDR = xDRS.getDispatchRecorder();
            if (xDR != null) {
                oObj = xDR;
            } else {
                oObj = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance(
                    "com.sun.star.comp.framework.DispatchRecorder");
                xDR = (XDispatchRecorder) UnoRuntime.queryInterface
                    (XDispatchRecorder.class, oObj);
                xDRS.setDispatchRecorder(xDR);
            }
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create component", e);
        }


        // fill recorder with content. It's needed for XIndexReplace
        URL dispURL = utils.parseURL((XMultiServiceFactory) Param.getMSF(), ".uno:InsertDateField");
        PropertyValue prop = new PropertyValue();
        prop.Name = "Text";
        prop.Value = "XDispatchRecorder.recordDispatch()";
        PropertyValue[] dispArgs = new PropertyValue[] {prop};
        xDR.recordDispatch(dispURL, dispArgs);


        TestEnvironment tEnv = new TestEnvironment( oObj );

        // INSTANCEn : _XIndexReplace
        log.println("adding INSTANCEn as obj relation to environment");

        int THRCNT = Integer.parseInt((String) Param.get("THRCNT"));

        URL instanceURL = null;
        DispatchStatement instance = new DispatchStatement();
        PropertyValue dispProp = new PropertyValue();

        for (int n = 1; n < (THRCNT + 1); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");
            instanceURL = utils.parseURL((XMultiServiceFactory) Param.getMSF(), ".uno:InsertText");
            dispProp.Name = "Text";
            dispProp.Value = "Instance " + n;
            dispArgs = new PropertyValue[] {dispProp};
            instance.aCommand = instanceURL.Complete;
            instance.aArgs = dispArgs;
            instance.aTarget = "_top";
            instance.nFlags = com.sun.star.frame.FrameSearchFlag.ALL;

            tEnv.addObjRelation("INSTANCE" + n, instance);
        }

        tEnv.addObjRelation("XDispatchRecorder.Frame", xFrame);
        log.println("Object created: TRUE");
        return tEnv;
    } // finish method getTestEnvironment

    protected void cleanup( TestParameters Param, PrintWriter log) {
        //oDoc.dispose();
    }

}
