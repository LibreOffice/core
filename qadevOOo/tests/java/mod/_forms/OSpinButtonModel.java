/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OSpinButtonModel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-04 13:38:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._forms;

import com.sun.star.beans.PropertyValue;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;

public class OSpinButtonModel extends TestCase {

    XTextDocument xTextDoc;

    /**
     * Creates a Writer document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        log.println( "creating a textdocument" );
        xTextDoc = WriterTools.createTextDoc(((XMultiServiceFactory) tParam.getMSF()));
    }

    /**
     * Disposes the Writer document.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Adds spin button into text and retrieves it's control model.
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        XControlShape aShape = FormTools.createControlShape(
            xTextDoc,3000,4500,15000,10000,"SpinButton");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);
        oObj = aShape.getControl();
        log.println( "creating a new environment for OButtonModel object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );
        tEnv.addObjRelation("OBJNAME", "com.sun.star.form.component.SpinButton");
        PropertyValue prop = new PropertyValue();
        prop.Name = "HelpText";
        prop.Value = "new Help Text since XPropertyAccess";
        tEnv.addObjRelation("XPropertyAccess.propertyToChange", prop);
        tEnv.addObjRelation("XPropertyContainer.propertyNotRemovable", "HelpText");

        System.out.println("Implementation name: "+util.utils.getImplName(oObj));
        return tEnv;
    } // finish method getTestEnvironment

}
