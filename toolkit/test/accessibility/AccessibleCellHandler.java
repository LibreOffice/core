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
import com.sun.star.accessibility.XAccessible;


class AccessibleCellHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        AccessibleCellHandler aCellHandler = null;
        if (xContext != null)
        {
            XAccessible xParent = xContext.getAccessibleParent();
            if (xParent != null)
            {
                XAccessibleTable xTable =
                    UnoRuntime.queryInterface (
                    XAccessibleTable.class, xParent.getAccessibleContext());
                if (xTable != null)
                    aCellHandler = new AccessibleCellHandler (xTable);
            }
        }
        return aCellHandler;

    }

    public AccessibleCellHandler ()
    {
    }

    public AccessibleCellHandler (XAccessibleTable xTable)
    {
        if (xTable != null)
            maChildList.setSize (8);
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
        XAccessibleContext xContext = null;
        AccessibleTreeNode aGrandParent = aParent.getParent();
        if (aGrandParent instanceof AccTreeNode)
        {
            xTable = ((AccTreeNode)aGrandParent).getTable();
            xContext = ((AccTreeNode)aGrandParent).getContext();
        }
        if (aParent instanceof AccTreeNode)
        {
            xContext = ((AccTreeNode)aParent).getContext();
        }
        try
        {
            if( xTable != null && xContext != null )
            {
                switch( nIndex )
                {
                    case 0:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );

                            aChild = new StringNode ("# table row: " + nRow, aParent);
                        }
                        break;
                    case 1:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nCol = xTable.getAccessibleColumn( nChild );

                            aChild = new StringNode ("# table column: " + nCol, aParent);
                        }
                        break;
                    case 2:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );
                            int nCol = xTable.getAccessibleColumn( nChild );
                            int nExt = xTable.getAccessibleRowExtentAt( nRow, nCol );

                            aChild = new StringNode ("# table row extend: " + nExt, aParent);
                        }
                        break;
                     case 3:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );
                            int nCol = xTable.getAccessibleColumn( nChild );
                            int nExt = xTable.getAccessibleColumnExtentAt( nRow, nCol );

                            aChild = new StringNode ("# table column extend: " + nExt, aParent);
                        }
                        break;
                     case 4:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );
                            int nCol = xTable.getAccessibleColumn( nChild );
                            XAccessible xChild =
                                xTable.getAccessibleCellAt( nRow, nCol );

                            aChild = new StringNode ("# cell name retrieved from table: " + xChild.getAccessibleContext().getAccessibleName(), aParent);
                        }
                        break;
                     case 5:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );
                            int nCol = xTable.getAccessibleColumn( nChild );
                            boolean bSelected =
                                xTable.isAccessibleSelected( nRow, nCol );

                            aChild = new StringNode ("cell is selected: " + bSelected, aParent);
                        }
                        break;
                     case 6:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nRow = xTable.getAccessibleRow( nChild );
                            boolean bSelected =
                                xTable.isAccessibleRowSelected( nRow );

                            aChild = new StringNode ("table row is selected: " + bSelected, aParent);
                        }
                        break;
                     case 7:
                        {
                            int nChild = xContext.getAccessibleIndexInParent();
                            int nCol = xTable.getAccessibleColumn( nChild );
                            boolean bSelected =
                                xTable.isAccessibleColumnSelected( nCol );

                            aChild = new StringNode ("table column is selected: " + bSelected, aParent);
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
