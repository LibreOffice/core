/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoTreeControl.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package mod._toolkit;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.awt.tree.XTreeControl;
import com.sun.star.awt.tree.XTreeNode;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.PropertyName;
import util.SOfficeFactory;
import util.UITools;
import util.utils;


public class UnoTreeControl extends TestCase {
    private static XTextDocument xTextDoc;
    private static XMutableTreeDataModel mXTreeDataModel;
    private static XMultiServiceFactory mxMSF;
    private static PrintWriter log;
    private static boolean debug = false;

    protected void initialize(TestParameters Param, PrintWriter log) {
        this.log = log;
        debug = Param.getBool(PropertyName.DEBUG_IS_ACTIVE);

        SOfficeFactory SOF = SOfficeFactory.getFactory(
            (XMultiServiceFactory) Param.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
//        log.println("    disposing xTextDoc ");
//
//        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {
        String sTreeControlName = "UnoTreeControl-Test";
        mxMSF = (XMultiServiceFactory) Param.getMSF();
        XInterface oObj = null;
        XMutableTreeNode xNode = null;

        try {

            mXTreeDataModel = (XMutableTreeDataModel )
                UnoRuntime.queryInterface(XMutableTreeDataModel.class,
                mxMSF.createInstance("com.sun.star.awt.tree.MutableTreeDataModel"));

            xNode = mXTreeDataModel.createNode("UnoTreeControl", false);

            xNode.setDataValue( "UnoTreeControl");
            xNode.setExpandedGraphicURL( "private:graphicrepository/sd/res/triangle_down.png");
            xNode.setCollapsedGraphicURL( "private:graphicrepository/sd/res/triangle_right.png");

            fillNode(xNode);

            mXTreeDataModel.setRoot(xNode);

            XControlModel xDialogModel = (XControlModel)
                UnoRuntime.queryInterface(XControlModel.class,
                mxMSF.createInstance("com.sun.star.awt.UnoControlDialogModel"));

            XPropertySet xDialogPropertySet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xDialogModel);
            xDialogPropertySet.setPropertyValue( "PositionX",   new Integer(50) );
            xDialogPropertySet.setPropertyValue( "PositionY",   new Integer(50) );
            xDialogPropertySet.setPropertyValue( "Width",       new Integer(256) );
            xDialogPropertySet.setPropertyValue( "Height",      new Integer(256) );
            xDialogPropertySet.setPropertyValue( "Title",       "Tree Control Test");

            XMultiServiceFactory xDialogMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xDialogModel);

            XControlModel  xTreeControlModel = (XControlModel)
                UnoRuntime.queryInterface(XControlModel.class,
                xDialogMSF.createInstance("com.sun.star.awt.tree.TreeControlModel"));

            XPropertySet XTreeControlModelSet = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTreeControlModel);

            XTreeControlModelSet.setPropertyValue( "SelectionType",com.sun.star.view.SelectionType.NONE);
            XTreeControlModelSet.setPropertyValue( "PositionX",     new Integer(3 ));
            XTreeControlModelSet.setPropertyValue( "PositionY",     new Integer(3 ));
            XTreeControlModelSet.setPropertyValue( "Width",         new Integer(253));
            XTreeControlModelSet.setPropertyValue( "Height",        new Integer(253) );
            XTreeControlModelSet.setPropertyValue( "DataModel",     mXTreeDataModel );
            XTreeControlModelSet.setPropertyValue( "ShowsRootHandles",new Boolean (false));
            XTreeControlModelSet.setPropertyValue( "ShowsHandles",  new Boolean (false));
            XTreeControlModelSet.setPropertyValue( "RootDisplayed", new Boolean (true));
            XTreeControlModelSet.setPropertyValue( "Editable",      new Boolean (true ));

            XNameContainer xDialogModelContainer = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, xDialogModel);

            xDialogModelContainer.insertByName( sTreeControlName, xTreeControlModel);

            XControl xDialogControl = (XControl)
            UnoRuntime.queryInterface(XControl.class,
                mxMSF.createInstance("com.sun.star.awt.UnoControlDialog"));

            xDialogControl.setModel( xDialogModel );

            XToolkit xToolkit = (XToolkit) UnoRuntime.queryInterface(XToolkit.class,
                        mxMSF.createInstance("com.sun.star.awt.Toolkit" ));

            xDialogControl.createPeer( xToolkit, null );

            // get the peers of the sub controls from the dialog peer container
            XControlContainer xDialogContainer = (XControlContainer)
            UnoRuntime.queryInterface(XControlContainer.class ,xDialogControl);

            XTreeControl xTreeControl = (XTreeControl)
            UnoRuntime.queryInterface(XTreeControl.class, xDialogContainer.getControl( sTreeControlName ));

            xTreeControl.expandNode(xNode);
            oObj = xTreeControl;

            XDialog xDialog = (XDialog) UnoRuntime.queryInterface(XDialog.class, xDialogControl);

            execurteDialog aDialog = new execurteDialog(xDialog);

            aDialog.start();

//            xDialog.execute();

        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XTreeControl_Node", xNode);

        //com.sun.star.view.XSelectionSupplier
        try {

            System.out.println("count of children: " + xNode.getChildCount());
            tEnv.addObjRelation("Selections", new Object[]{xNode.getChildAt(0), xNode});
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("ERROR: could not add object relation 'Selections' because 'xNode.getChildAt(1) failed: " +
                        ex.toString());
        }

        tEnv.addObjRelation("Comparer",
                            new Comparator() {
            public int compare(Object o1, Object o2) {
                XMutableTreeNode xNode1 = (XMutableTreeNode) UnoRuntime.queryInterface(
                                        XMutableTreeNode.class, o1);
                XTreeNode xNode2a = null;
                try {
                    xNode2a = (XTreeNode) AnyConverter.toObject(new Type(XTreeNode.class), o2);
                } catch (com.sun.star.lang.IllegalArgumentException ex) {
                    ex.printStackTrace();
                }

                XMutableTreeNode xNode2 = (XMutableTreeNode) UnoRuntime.queryInterface(
                                        XMutableTreeNode.class, xNode2a);

                if (((String) xNode1.getDataValue()).equals((String)xNode2.getDataValue())) {
                    return 0;
                }

                return -1;
            }
        });
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        //this.getAccessibleContext(xNode);

        return tEnv;
    } // finish method getTestEnvironment

    private void fillNode( XMutableTreeNode xNode ){

        if( xNode.getChildCount() == 0 )
        {
            String sParentPath = (String) xNode.getDataValue();

            String officeUserPath = utils.getOfficeUserPath(mxMSF);
            Object fileacc = null;
            try {
                fileacc = mxMSF.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            } catch (com.sun.star.uno.Exception ex) {
                ex.printStackTrace();
            }
            XSimpleFileAccess sA = (XSimpleFileAccess)
                            UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);


            dirlist(officeUserPath, xNode);
        }
    }

    private void dirlist(String dir, XMutableTreeNode xNode){

        Object fileacc = null;
        try {
            fileacc = mxMSF.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }
        XSimpleFileAccess sfa = (XSimpleFileAccess)
                        UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
        XMutableTreeNode xChildNode = null;
        try {
            xChildNode = mXTreeDataModel.createNode(dir.substring(dir.lastIndexOf("/")+1, dir.length()), sfa.isFolder(dir));
            xChildNode.setDataValue(dir);
            boolean test = sfa.isFolder(dir);
            if (sfa.isFolder(dir)){
                xChildNode.setExpandedGraphicURL( "private:graphicrepository/sd/res/triangle_down.png");
                xChildNode.setCollapsedGraphicURL("private:graphicrepository/sd/res/triangle_right.png");
                String[] children = sfa.getFolderContents(dir, true);
                if (children != null){
                    for (int i=0; i<children.length; i++) {
                        // Get filename of file or directory
                        String filename = children[i];
                        dirlist( filename , xChildNode);
                    }
                }
            }
            else{
                xChildNode.setNodeGraphicURL( "private:graphicrepository/sw/imglst/nc20010.png");
            }
        } catch (CommandAbortedException ex) {
            ex.printStackTrace();
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace();
        }

        try {
            xNode.appendChild( xChildNode );
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace();
        }
    }

    private XAccessibleContext getAccessibleContext(XMutableTreeNode xNode ){

        UITools oDocUITools = new UITools(this.mxMSF, this.xTextDoc);

        XWindow xDialogWindow = null;
        try {

            xDialogWindow = oDocUITools.getActiveTopWindow();
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        UITools oDialog = new UITools(mxMSF, xDialogWindow);

        oDialog.printAccessibleTree(log, debug);

        return null;

    }

    private class execurteDialog extends Thread{
        private XDialog mXDialog;

        public execurteDialog(XDialog xDialog){
            mXDialog = xDialog;
        }

        public void run() {
            mXDialog.endExecute();
        }
    }


} // finish class UnoControlRadioButton
