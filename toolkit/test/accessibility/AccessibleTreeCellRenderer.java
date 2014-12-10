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

import java.awt.Color;
import java.awt.Component;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;


public class AccessibleTreeCellRenderer
    extends DefaultTreeCellRenderer
{
    private Color
        maDefaultColor,
        maChangedColor;
    private ArrayList<Boolean>
        maChangedLines;



    public AccessibleTreeCellRenderer ()
    {
        maDefaultColor = Color.black;
        maChangedColor = Color.red;
        maChangedLines = new ArrayList<Boolean> ();
    }

    @Override
    public Component getTreeCellRendererComponent (
        JTree tree,
        Object value,
        boolean sel,
        boolean expanded,
        boolean leaf,
        int row,
        boolean hasFocus)
    {
        super.getTreeCellRendererComponent(
            tree, value, sel,
            expanded, leaf, row,
            hasFocus);

        if (maChangedLines.size()<=row || maChangedLines.get (row) == null)
            setTextNonSelectionColor (maDefaultColor);
        else
            setTextNonSelectionColor (maChangedColor);

        return this;
    }

    /** Tell the cell renderer that no changes shall be displayed anymore.
    */
    public void clearAllChanges ()
    {
        maChangedLines.clear();
    }

    /** Inform the cell renderer of a new changed line which to paint
        highlighted when asked to paint it the next time.
    */
    private void addChangedLine (int nRow)
    {
        while (maChangedLines.size() <= nRow) {
            maChangedLines.add(null);
        }
        nRow -= 1; // row index is one to large for some reason.
        maChangedLines.set (nRow, true);
    }

    /** Inform the cell renderer of a set of changed line which to paint
        highlighted when asked to paint them the next time.
        @param aChangedNodes
            The set of changed nodes.  Each entry is a TreePath.
        @param aTree
            The JTree that is used to transform the given TreePath objects
            into rows.
    */
    public void addChangedNodes (List<TreePath> aChangedNodes, JTree aTree)
    {
        for (int i=0; i<aChangedNodes.size(); i++)
        {
            TreePath aPath = aChangedNodes.get (i);
            int nRow = aTree.getRowForPath (aPath);
            addChangedLine (nRow);
        }
    }

}

