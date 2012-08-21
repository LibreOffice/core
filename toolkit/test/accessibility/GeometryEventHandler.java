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

import com.sun.star.accessibility.AccessibleEventObject;
import java.io.PrintStream;
import java.util.LinkedList;

class GeometryEventHandler
    extends EventHandler
{
    public GeometryEventHandler (AccessibleEventObject aEvent, AccessibilityTreeModel aTreeModel)
    {
        super (aEvent, aTreeModel);
    }

    public void PrintOldAndNew (PrintStream out)
    {
        out.println ("   children not relevant");
    }

    public void Process ()
    {
        AccTreeNode aNode = maTreeModel.updateNode (mxEventSource,
            AccessibleComponentHandler.class,
            AccessibleExtendedComponentHandler.class);

        // Update the graphical representation of aNode in the Canvas.
        Canvas aCanvas = maTreeModel.getCanvas();
        if (aCanvas != null)
        {
            // Iterate over all nodes in the sub-tree rooted in aNode.
            LinkedList aShapeQueue = new LinkedList();
            aShapeQueue.addLast (aNode);
            while (aShapeQueue.size() > 0)
            {
                // Remove the first node from the queue and update its
                // graphical representation.
                AccTreeNode aShapeNode = (AccTreeNode) aShapeQueue.getFirst();
                aShapeQueue.removeFirst();
                aCanvas.updateNodeGeometry (aShapeNode);

                // Add the node's children to the queue.
                int nChildCount = maTreeModel.getChildCount (aShapeNode);
                for (int i=0; i<nChildCount; i++)
                {
                    Object aTreeNode = maTreeModel.getChildNoCreate (aShapeNode, i);
                    if (aTreeNode instanceof AccTreeNode)
                        aShapeQueue.addLast (aTreeNode);
                }
            }
            aCanvas.repaint ();
        }
    }
}
