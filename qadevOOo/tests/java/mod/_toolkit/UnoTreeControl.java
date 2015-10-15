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
package mod._toolkit;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.awt.tree.XTreeControl;
import com.sun.star.awt.tree.XTreeNode;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;
import java.util.Comparator;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.utils;


public class UnoTreeControl extends TestCase {
    private static XMutableTreeDataModel mXTreeDataModel;
    private static XMultiServiceFactory mxMSF;

    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
            Param.getMSF());

        log.println("creating a textdocument");
         SOF.createTextDoc(null);
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {
        String sTreeControlName = "UnoTreeControl-Test";
        mxMSF = Param.getMSF();
        XInterface oObj = null;
        XMutableTreeNode xNode = null;

        try {

            mXTreeDataModel = UnoRuntime.queryInterface(XMutableTreeDataModel.class,
            mxMSF.createInstance("com.sun.star.awt.tree.MutableTreeDataModel"));

            xNode = mXTreeDataModel.createNode("UnoTreeControl", false);

            xNode.setDataValue( "UnoTreeControl");
            xNode.setExpandedGraphicURL( "private:graphicrepository/sd/res/triangle_down.png");
            xNode.setCollapsedGraphicURL( "private:graphicrepository/sd/res/triangle_right.png");

            fillNode(xNode);

            mXTreeDataModel.setRoot(xNode);

            XControlModel xDialogModel = UnoRuntime.queryInterface(XControlModel.class,
            mxMSF.createInstance("com.sun.star.awt.UnoControlDialogModel"));

            XPropertySet xDialogPropertySet = UnoRuntime.queryInterface(XPropertySet.class, xDialogModel);
            xDialogPropertySet.setPropertyValue( "PositionX",   Integer.valueOf(50) );
            xDialogPropertySet.setPropertyValue( "PositionY",   Integer.valueOf(50) );
            xDialogPropertySet.setPropertyValue( "Width",       Integer.valueOf(256) );
            xDialogPropertySet.setPropertyValue( "Height",      Integer.valueOf(256) );
            xDialogPropertySet.setPropertyValue( "Title",       "Tree Control Test");

            XMultiServiceFactory xDialogMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xDialogModel);

            XControlModel  xTreeControlModel = UnoRuntime.queryInterface(XControlModel.class,
            xDialogMSF.createInstance("com.sun.star.awt.tree.TreeControlModel"));

            XPropertySet XTreeControlModelSet = UnoRuntime.queryInterface(XPropertySet.class, xTreeControlModel);

            XTreeControlModelSet.setPropertyValue( "SelectionType",com.sun.star.view.SelectionType.NONE);
            XTreeControlModelSet.setPropertyValue( "PositionX",     Integer.valueOf(3 ));
            XTreeControlModelSet.setPropertyValue( "PositionY",     Integer.valueOf(3 ));
            XTreeControlModelSet.setPropertyValue( "Width",         Integer.valueOf(253));
            XTreeControlModelSet.setPropertyValue( "Height",        Integer.valueOf(253) );
            XTreeControlModelSet.setPropertyValue( "DataModel",     mXTreeDataModel );
            XTreeControlModelSet.setPropertyValue( "ShowsRootHandles", Boolean.FALSE);
            XTreeControlModelSet.setPropertyValue( "ShowsHandles",  Boolean.FALSE);
            XTreeControlModelSet.setPropertyValue( "RootDisplayed", Boolean.TRUE);
            XTreeControlModelSet.setPropertyValue( "Editable",      Boolean.TRUE);

            XNameContainer xDialogModelContainer = UnoRuntime.queryInterface(XNameContainer.class, xDialogModel);

            xDialogModelContainer.insertByName( sTreeControlName, xTreeControlModel);

            XControl xDialogControl = UnoRuntime.queryInterface(XControl.class,
                mxMSF.createInstance("com.sun.star.awt.UnoControlDialog"));

            xDialogControl.setModel( xDialogModel );

            XToolkit xToolkit = UnoRuntime.queryInterface(XToolkit.class,
                        mxMSF.createInstance("com.sun.star.awt.Toolkit" ));

            xDialogControl.createPeer( xToolkit, null );

            // get the peers of the sub controls from the dialog peer container
            XControlContainer xDialogContainer = UnoRuntime.queryInterface(XControlContainer.class ,xDialogControl);

            XTreeControl xTreeControl = UnoRuntime.queryInterface(XTreeControl.class, xDialogContainer.getControl( sTreeControlName ));

            xTreeControl.expandNode(xNode);
            oObj = xTreeControl;

            XDialog xDialog = UnoRuntime.queryInterface(XDialog.class, xDialogControl);

            execurteDialog aDialog = new execurteDialog(xDialog);

            aDialog.start();

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
                            new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XMutableTreeNode xNode1 = UnoRuntime.queryInterface(
                                        XMutableTreeNode.class, o1);
                XTreeNode xNode2a = null;
                try {
                    xNode2a = (XTreeNode) AnyConverter.toObject(new Type(XTreeNode.class), o2);
                } catch (com.sun.star.lang.IllegalArgumentException ex) {
                    ex.printStackTrace();
                }

                XMutableTreeNode xNode2 = UnoRuntime.queryInterface(
                                        XMutableTreeNode.class, xNode2a);

                if (((String) xNode1.getDataValue()).equals(xNode2.getDataValue())) {
                    return 0;
                }

                return -1;
            }
        });
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

    private void fillNode( XMutableTreeNode xNode ){

        if( xNode.getChildCount() == 0 )
        {
            xNode.getDataValue();

            String officeUserPath = utils.getOfficeUserPath(mxMSF);
            Object fileacc = null;
            try {
                fileacc = mxMSF.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            } catch (com.sun.star.uno.Exception ex) {
                ex.printStackTrace();
            }
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
        XSimpleFileAccess sfa = UnoRuntime.queryInterface(XSimpleFileAccess.class,fileacc);
        XMutableTreeNode xChildNode = null;
        try {
            xChildNode = mXTreeDataModel.createNode(dir.substring(dir.lastIndexOf("/")+1, dir.length()), sfa.isFolder(dir));
            xChildNode.setDataValue(dir);
            sfa.isFolder(dir);
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

    private static class execurteDialog extends Thread{
        private final XDialog mXDialog;

        public execurteDialog(XDialog xDialog){
            mXDialog = xDialog;
        }

        @Override
        public void run() {
            mXDialog.endExecute();
        }
    }


} // finish class UnoControlRadioButton
