/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/

package org.apache.openoffice.ooxml.viewer;

import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;

import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import org.apache.openoffice.ooxml.framework.part.ContentType;
import org.apache.openoffice.ooxml.framework.part.OOXMLPackage;
import org.apache.openoffice.ooxml.framework.part.Part;
import org.apache.openoffice.ooxml.framework.part.PartName;

/** Overview of the individual parts/streams in an OOXML file.
 */
@SuppressWarnings("serial")
public class StreamView
    extends JTree
    implements TreeSelectionListener, MouseMotionListener
{
    /** Create a new PartsView object for the given OOXML file.
     *  When the user clicks on one part entry then the DetailViewManager is
     *  called and asked to display the part.
     */
    StreamView (
        final DetailViewManager aDetailViewManager,
        final OOXMLPackage aPackage)
    {
        maDetailViewManager = aDetailViewManager;
        maOOXMLPackage = aPackage;
        getSelectionModel().addTreeSelectionListener(this);

        // Listen for motion events so that the tooltip can be set according to
        // the entry under the mouse pointer.
        addMouseMotionListener(this);

        // Create a tree model for the streams in the (zip) file, set the model
        // asynchronously at the JTree and finally expand all nodes.
        final TreeModel aModel = CreateTreeModel(aPackage);
        SwingUtilities.invokeLater(new Runnable()
        {
            public void run()
            {
                setModel(aModel);
                for (int nIndex=0; nIndex<getRowCount(); ++nIndex)
                {
                    expandRow(nIndex);
                }
            }
        });

        setToolTipText("hallo");
    }




    private TreeModel CreateTreeModel (final OOXMLPackage aPackage)
    {
        final DefaultMutableTreeNode aRootNode = new DefaultMutableTreeNode(
            aPackage.getFileName());
        final DefaultTreeModel aModel = new DefaultTreeModel(aRootNode);
        try
        {
            for (final String sStreamName : aPackage.listStreamNames())
            {
                DefaultMutableTreeNode aNode = aRootNode;
                for (final String sPart : sStreamName.split("/"))
                {
                    DefaultMutableTreeNode aChild = GetChildNodeForName(aNode, sPart);
                    if (aChild == null)
                    {
                        aChild = new DefaultMutableTreeNode(sPart);
                        aNode.add(aChild);
                    }

                    aNode = aChild;
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        return aModel;
    }




    public void ShowInitialPart ()
    {
        final Part aPart = maOOXMLPackage.getOfficeDocumentPart();
        MakePartVisible(aPart);
        maDetailViewManager.ShowPart(aPart);
    }




    private DefaultMutableTreeNode GetChildNodeForName (final TreeNode aNode, final String sName)
    {
        for (int nIndex=0; nIndex<aNode.getChildCount(); ++nIndex)
        {
            final TreeNode aChild = aNode.getChildAt(nIndex);
            if (aChild.toString().equals(sName))
            {
                return (DefaultMutableTreeNode)aChild;
            }
        }
        return null;
    }




    /** Callback for clicks on the part view.
     */
    @Override
    public void valueChanged (final TreeSelectionEvent aEvent)
    {
        final TreePath aPath = aEvent.getNewLeadSelectionPath();
        if (aPath != null)
        {
            final String sPath = GetPackagePathForTreePath(aPath);
            final PartName aName = new PartName(sPath);
            final ContentType eType = maOOXMLPackage.getPart(
                aName).getContentType();
            maDetailViewManager.ShowPart(
                aName,
                eType);
        }
    }




    @Override
    public void mouseDragged (final MouseEvent aEvent)
    {
    }




    @Override
    public void mouseMoved (final MouseEvent aEvent)
    {
        final int nRow = getRowForLocation(aEvent.getX(), aEvent.getY());
        if (nRow >= 0)
        {
            final String sPath = GetPackagePathForTreePath(getPathForRow(nRow));
            SetPartUnderMouse(sPath);
        }
    }




    private String GetPackagePathForTreePath (final TreePath aPath)
    {
        final StringBuffer sPath = new StringBuffer("");
        final Object aNodes[] = aPath.getPath();
        for (int nIndex=1; nIndex<aNodes.length; ++nIndex)
        {
            sPath.append("/");
            sPath.append(aNodes[nIndex].toString());
        }
        return sPath.toString();
    }




    private void SetPartUnderMouse (final String sPartPath)
    {
        if (msPartPathUnderMouse==null
            || ! msPartPathUnderMouse.equals(sPartPath))
        {
            msPartPathUnderMouse = sPartPath;

            final ContentType eType = maOOXMLPackage.getPart(
                new PartName(msPartPathUnderMouse)).getContentType();
            String sToolTipText = eType.toString();
            if ( ! eType.GetLongName().isEmpty())
                sToolTipText += " (" + eType.GetLongName() + ")";
            setToolTipText(sToolTipText);
        }
    }




    private void MakePartVisible (final Part aPart)
    {
        final String[] aPathParts = aPart.getPartName().GetFullname().substring(1).split("/");
        final TreeNode[] aTreeNodePath = new TreeNode[aPathParts.length+1];
        TreeNode aNode = (TreeNode)getModel().getRoot();
        int nDepth = 0;
        aTreeNodePath[nDepth++] = aNode;
        for (final String sPathPart : aPathParts)
        {
            boolean bFoundChild = false;
            for (int nIndex=0; nIndex<aNode.getChildCount(); ++nIndex)
            {
                final TreeNode aChildNode = aNode.getChildAt(nIndex);
                final String sChildName = aChildNode.toString();
                if (sChildName.equals(sPathPart))
                {
                    aNode = aChildNode;
                    aTreeNodePath[nDepth++] = aNode;
                    bFoundChild = true;
                    break;
                }
            }
            if ( ! bFoundChild)
                return;
        }

        SelectNode(new TreePath(aTreeNodePath));
    }




    private void SelectNode (final TreePath aTreePath)
    {
//        getSelectionModel().setSelectionPath(aTreePath);
//        scrollPathToVisible(aTreePath);
    }




    private final DetailViewManager maDetailViewManager;
    private final OOXMLPackage maOOXMLPackage;
    private String msPartPathUnderMouse;
}
