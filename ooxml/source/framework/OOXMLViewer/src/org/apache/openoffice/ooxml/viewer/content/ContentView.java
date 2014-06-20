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

package org.apache.openoffice.ooxml.viewer.content;

import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeModel;

import org.apache.openoffice.ooxml.framework.part.OOXMLPackage;
import org.apache.openoffice.ooxml.viewer.DetailViewManager;

/** Overview of the individual parts/streams in an OOXML file.
 */
@SuppressWarnings("serial")
public class ContentView
    extends JTree
{
    public ContentView (
        final DetailViewManager aDetailViewManager,
        final OOXMLPackage aPackage)
    {
        TreeModel aModel = null;
        switch(aPackage.getOfficeDocumentPart().getContentType())
        {
            case PmlDocument:
                aModel = CreateTreeForPresentationModel(new PresentationImporter().importModel(
                    aPackage.getOfficeDocumentPart()));
                break;
            //case SmlDocument:
            //case WmlDocument:
            default:
                break;
        }
        if (aModel != null)
            setModel(aModel);
    }






    private TreeModel CreateTreeForPresentationModel (final PresentationModel aModel)
    {
        final DefaultMutableTreeNode aRootNode = new DefaultMutableTreeNode(
            "presentation");
        final DefaultTreeModel aTreeModel = new DefaultTreeModel(aRootNode);

        for (final Slide aSlide : aModel.GetSlideManager().GetSlides())
        {
                DefaultMutableTreeNode aNode = new DefaultMutableTreeNode(
                    aSlide.toString());
                aRootNode.add(aNode);
        }
        return aTreeModel;
    }
}
