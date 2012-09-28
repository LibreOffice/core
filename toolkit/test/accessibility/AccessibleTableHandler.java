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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleTable;


class AccessibleTableHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleTable xTable =
            UnoRuntime.queryInterface (
            XAccessibleTable.class, xContext);
        if (xTable != null)
            return new AccessibleTableHandler (xTable);
        else
            return null;
    }

    public AccessibleTableHandler ()
    {
    }

    public AccessibleTableHandler (XAccessibleTable xTable)
    {
        if (xTable != null)
            maChildList.setSize (4);
    }

    protected static XAccessibleTable getTable(Object aObject)
    {
        return UnoRuntime.queryInterface (
            XAccessibleTable.class, aObject);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        XAccessibleTable xTable = null;
        if (aParent instanceof AccTreeNode)
            xTable = ((AccTreeNode)aParent).getTable();
        try
        {
            if( xTable != null )
            {
                switch( nIndex )
                {
                    case 0:
                        aChild = new StringNode ("# table rows: " + xTable.getAccessibleRowCount(), aParent);
                        break;
                    case 1:
                        aChild = new StringNode ("# table columns: " + xTable.getAccessibleColumnCount(), aParent);
                        break;
                    case 2:
                        {
                            String sText = "selected rows: ";
                            int[] aSelected = xTable.getSelectedAccessibleRows();
                            for( int i=0; i < aSelected.length; i++ )
                            {
                                sText += aSelected[i];
                                sText += " ";
                            }
                            aChild = new StringNode (sText, aParent);
                        }
                        break;
                    case 3:
                        {
                            String sText = "selected columns: ";
                            int[] aSelected = xTable.getSelectedAccessibleColumns();
                            for( int i=0; i < aSelected.length; i++ )
                            {
                                sText += aSelected[i];
                                sText += " ";
                            }
                            aChild = new StringNode (sText, aParent);
                        }
                        break;
                    default:
                        aChild = new StringNode ("unknown child index " + nIndex, aParent);
                }
            }
        }
        catch (Exception e)
        {
            // Return empty child.
        }

        return aChild;
    }
}
