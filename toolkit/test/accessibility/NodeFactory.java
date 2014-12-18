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

import com.sun.star.accessibility.*;
import tools.NameProvider;

/** This singleton class creates nodes for given accessible objects.
*/
class NodeFactory
{
    public synchronized static NodeFactory Instance ()
    {
        if (maInstance == null)
        {
            maInstance = new NodeFactory();
        }
        return maInstance;
    }

    private NodeFactory ()
    {
    }


    /** add default handlers based on the supported interfaces */
    private void addDefaultHandlers (AccTreeNode aNode, XAccessibleContext xContext)
    {
        if (false)
        {
            // Slow but complete version: try each handler type separately.
            aNode.addHandler (maContextHandler.createHandler (xContext));
            aNode.addHandler (maTextHandler.createHandler (xContext));
            aNode.addHandler (maEditableTextHandler.createHandler (xContext));
            aNode.addHandler (maComponentHandler.createHandler (xContext));
            aNode.addHandler (maExtendedComponentHandler.createHandler (xContext));
            aNode.addHandler (maActionHandler.createHandler (xContext));
            aNode.addHandler (maImageHandler.createHandler (xContext));
            aNode.addHandler (maTableHandler.createHandler (xContext));
            aNode.addHandler (maCellHandler.createHandler (xContext));
            aNode.addHandler (maHypertextHandler.createHandler (xContext));
            aNode.addHandler (maHyperlinkHandler.createHandler (xContext));
            aNode.addHandler (maSelectionHandler.createHandler (xContext));
            aNode.addHandler (maRelationHandler.createHandler (xContext));
            aNode.addHandler (maUNOHandler.createHandler (xContext));
            aNode.addHandler (maTreeHandler.createHandler (xContext));
        }
        else
        {
            // Exploit dependencies between interfaces.
            NodeHandler aHandler;
            aNode.addHandler (maContextHandler.createHandler (xContext));

            aHandler = maTextHandler.createHandler (xContext);
            if (aHandler != null)
            {
                aNode.addHandler (aHandler);
                aNode.addHandler (maEditableTextHandler.createHandler (xContext));
                aNode.addHandler (maHypertextHandler.createHandler (xContext));
                aNode.addHandler (maHyperlinkHandler.createHandler (xContext));
            }
            aHandler = maComponentHandler.createHandler (xContext);
            if (aHandler != null)
            {
                aNode.addHandler (aHandler);
                aNode.addHandler (maExtendedComponentHandler.createHandler (xContext));
            }
            aNode.addHandler (maActionHandler.createHandler (xContext));
            aNode.addHandler (maImageHandler.createHandler (xContext));
            aNode.addHandler (maTableHandler.createHandler (xContext));
            aNode.addHandler (maRelationHandler.createHandler (xContext));
            aNode.addHandler (maCellHandler.createHandler (xContext));
            aNode.addHandler (maSelectionHandler.createHandler (xContext));
            aNode.addHandler (maUNOHandler.createHandler (xContext));
            aNode.addHandler (maTreeHandler.createHandler (xContext));
        }
    }

    /** create a node with the default handlers */
    public AccTreeNode createDefaultNode (XAccessible xAccessible, AccessibleTreeNode aParent)
    {
        // default: aObject + aDisplay
        String sDisplay;

        // if we are accessible, we use the context + name instead
        XAccessibleContext xContext = null;
        if (xAccessible != null)
            xContext = xAccessible.getAccessibleContext();
        if (xContext != null)
        {
            sDisplay = xContext.getAccessibleName();
            if (sDisplay.length()==0)
            {
                sDisplay = "<no name> Role: "
                    + NameProvider.getRoleName (
                        xContext.getAccessibleRole());
            }
        }
        else
            sDisplay = "not accessible";


        // create node, and add default handlers
        AccTreeNode aNode = new AccTreeNode (xContext, sDisplay, aParent);
        addDefaultHandlers (aNode, xContext);

        return aNode;
    }

    private static NodeFactory maInstance = null;

    // default handlers
    private final NodeHandler maContextHandler = new AccessibleContextHandler();
    private final NodeHandler maTextHandler = new AccessibleTextHandler();
    private final NodeHandler maEditableTextHandler = new AccessibleEditableTextHandler();
    private final NodeHandler maComponentHandler = new AccessibleComponentHandler();
    private final NodeHandler maExtendedComponentHandler = new AccessibleExtendedComponentHandler();
    private final NodeHandler maActionHandler = new AccessibleActionHandler();
    private final NodeHandler maImageHandler = new AccessibleImageHandler();
    private final NodeHandler maTableHandler = new AccessibleTableHandler();
    private final NodeHandler maCellHandler = new AccessibleCellHandler();
    private final NodeHandler maHypertextHandler = new AccessibleHypertextHandler();
    private final NodeHandler maHyperlinkHandler = new AccessibleHyperlinkHandler();
    private final NodeHandler maSelectionHandler = new AccessibleSelectionHandler();
    private final NodeHandler maRelationHandler = new AccessibleRelationHandler();
    private final NodeHandler maTreeHandler = new AccessibleTreeHandler();
    private final NodeHandler maUNOHandler = new AccessibleUNOHandler();
}
