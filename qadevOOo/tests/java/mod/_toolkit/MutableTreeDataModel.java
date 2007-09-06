/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MutableTreeDataModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:58:54 $
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
package mod._toolkit;

import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.WriterTools;
import util.utils;


public class MutableTreeDataModel extends TestCase {
    private static XTextDocument xTextDoc;
    private static XInterface oObj = null;

    /**
     * Creates StarOffice Writer document.
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(
            (XMultiServiceFactory) tParam.getMSF());
    }

    /**
     * Disposes StarOffice Writer document.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {

        try {
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
                "com.sun.star.awt.tree.MutableTreeDataModel");
        } catch (Exception e) {
        }

        log.println(
            "creating a new environment for MutableTreeDataModel object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME", "toolkit.MutableTreeDataModel");
        log.println("ImplementationName: " + utils.getImplName(oObj));

        tEnv.addObjRelation("XTreeDataModelListenerEvent", new XTreeDataModelListenerEvent());

        return tEnv;
    } // finish method getTestEnvironment

    public class XTreeDataModelListenerEvent implements ifc.awt.tree._XTreeDataModel.XTreeDataModelListenerEvent{

        public void fireEvent(){

            XMutableTreeDataModel xModel = (XMutableTreeDataModel) UnoRuntime.queryInterface(XMutableTreeDataModel.class, oObj);
            XMutableTreeNode node = xModel.createNode("EventNode", true);
            try {
                xModel.setRoot(node);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not preform event: " + ex.toString());
                ex.printStackTrace();
            }

        }
    }
} // finish class UnoControlListBoxModel
