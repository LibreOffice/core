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

import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.TreeSet;

import javax.swing.JTree;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import org.apache.openoffice.ooxml.framework.part.ContentType;
import org.apache.openoffice.ooxml.framework.part.IReferenceProvider;
import org.apache.openoffice.ooxml.framework.part.OOXMLPackage;
import org.apache.openoffice.ooxml.framework.part.Part;
import org.apache.openoffice.ooxml.framework.part.PartName;

@SuppressWarnings("serial")
public class PartsView
    extends JTree
    implements TreeSelectionListener
{
    public PartsView (
        final DetailViewManager aDetailViewManager,
        final OOXMLPackage aPackage)
    {
        maPackage = aPackage;
        maDetailViewManager = aDetailViewManager;

        Initialize();

        getSelectionModel().addTreeSelectionListener(this);
    }



    private final void Initialize ()
    {
        final DefaultMutableTreeNode aRootNode = new DefaultMutableTreeNode(
            "top level relations");
        final TreeModel aModel = new DefaultTreeModel(aRootNode);

        final Queue<IReferenceProvider> aWorklist = new LinkedList<IReferenceProvider>();
        aWorklist.add(maPackage);
        final Set<PartName> aProcessedPartNames = new TreeSet<PartName>();
        CreateChildren(aRootNode, aProcessedPartNames, maPackage);
        setModel(aModel);
    }




    private void CreateChildren (
        final DefaultMutableTreeNode aNode,
        final Set<PartName> aProcessedPartNames,
        final IReferenceProvider aReferences)
    {
        for (final PartName aTarget : aReferences.getRelatedParts().getAllTargets())
        {
            Part aPart = maPackage.getPart(aTarget);
            final DefaultMutableTreeNode aRelationNode;
            if ( ! aProcessedPartNames.contains(aPart.getPartName()))
            {
                aProcessedPartNames.add(aPart.getPartName());
                aRelationNode = new DefaultMutableTreeNode(aPart.getPartName().GetFullname());
                CreateChildren(aRelationNode, aProcessedPartNames, aPart);
            }
            else
            {
                aRelationNode = new DefaultMutableTreeNode(aPart.getPartName().GetFullname() + "...");
            }
            aNode.add(aRelationNode);
        }
    }




    /** Callback for clicks on the part view.
     */
    @Override
    public void valueChanged (final TreeSelectionEvent aEvent)
    {
        final TreePath aPath = aEvent.getNewLeadSelectionPath();
        if (aPath != null)
        {
            final PartName aName= GetPackagePathForTreePath(aPath);
            final ContentType eType = maPackage.getPart(
                aName).getContentType();
            maDetailViewManager.ShowPart(
                aName,
                eType);
        }
    }





    private PartName GetPackagePathForTreePath (final TreePath aPath)
    {
        final PartName aName = new PartName(aPath.getLastPathComponent().toString());
        return aName;
    }





    private final OOXMLPackage maPackage;
    private final DetailViewManager maDetailViewManager;
}
