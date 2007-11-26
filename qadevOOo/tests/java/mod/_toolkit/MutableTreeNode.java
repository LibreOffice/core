/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MutableTreeNode.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 17:17:55 $
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
import com.sun.star.ucb.CommandAbortedException;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import ifc.awt.tree._XMutableTreeNode.XMutableTreeNodeCreator;

import java.io.PrintWriter;
import lib.Status;
import lib.StatusException;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.PropertyName;
import util.utils;


public class MutableTreeNode extends TestCase {
    private static XTextDocument xTextDoc;
    private static XInterface oObj = null;
    private static XMutableTreeDataModel mXTreeDataModel;
    private static XMultiServiceFactory mxMSF;
    private static PrintWriter log;
    private static boolean debug = false;

    /**
     * Creates StarOffice Writer document.
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        this.log = log;
        debug = tParam.getBool(PropertyName.DEBUG_IS_ACTIVE);
        mxMSF = (XMultiServiceFactory) tParam.getMSF();
//        log.println("creating a textdocument");
//        xTextDoc = WriterTools.createTextDoc(mxMSF);
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
        XMutableTreeNode xNode;

        try {
            mXTreeDataModel = (XMutableTreeDataModel) UnoRuntime.queryInterface(XMutableTreeDataModel.class,
                mxMSF.createInstance("com.sun.star.awt.tree.MutableTreeDataModel"));
        } catch (com.sun.star.uno.Exception ex) {
            throw new StatusException(Status.failed("ERROR: could not create instance of" +
                " 'com.sun.star.awt.tree.MutableTreeDataModel'"));
        }

        xNode = mXTreeDataModel.createNode("UnoTreeControl", false);

        String sDisplayValue = "UnoTreeControl";
        String sExpandedGraphicURL = "private:graphicrepository/sd/res/triangle_down.png";
        String sCollapsedGraphicURL = "private:graphicrepository/sd/res/triangle_right.png";
        String sNodeGraphicURL = "private:graphicrepository/sw/imglst/nc20010.png";

        xNode.setDisplayValue( sDisplayValue);
        xNode.setDataValue(sDisplayValue);
        xNode.setExpandedGraphicURL(sExpandedGraphicURL);
        xNode.setCollapsedGraphicURL(sCollapsedGraphicURL);
        xNode.setNodeGraphicURL(sNodeGraphicURL);
        xNode.setHasChildrenOnDemand(true);

        fillNode(xNode);

        TestEnvironment tEnv = new TestEnvironment(xNode);

        tEnv.addObjRelation("OBJNAME", "toolkit.MutableTreeDataModel");
        log.println("ImplementationName: " + utils.getImplName(oObj));

        tEnv.addObjRelation("XTreeNode_DisplayValue", sDisplayValue);
        tEnv.addObjRelation("XTreeNode_ExpandedGraphicURL", sExpandedGraphicURL);
        tEnv.addObjRelation("XTreeNode_CollapsedGraphicURL", sCollapsedGraphicURL);
        tEnv.addObjRelation("XTreeNode_NodeGraphicURL", sNodeGraphicURL);

        tEnv.addObjRelation("XMutableTreeNode_NodeToAppend",
                            mXTreeDataModel.createNode("XMutableTreeNode_NodeToAppend", true));

        tEnv.addObjRelation("XMutableTreeNodeCreator", new XMutableTreeNodeCreator(){
            public XMutableTreeNode createNode(String name){
                return mXTreeDataModel.createNode(name, true);
            }
        });

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
} // finish class UnoControlListBoxModel
