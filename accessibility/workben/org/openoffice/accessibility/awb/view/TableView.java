/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.accessibility.awb.view;

import java.lang.Integer;
import java.lang.StringBuffer;

import javax.swing.JLabel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleTable;
import com.sun.star.uno.UnoRuntime;



/** The <type>ContextView</type> class displays information accessible over
    the <type>XAccessibleContext</type> interface.  This includes name,
    description, and role.
*/
public class TableView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (UnoRuntime.queryInterface(
            XAccessibleTable.class, xContext) != null)
            return new TableView (aContainer);
        else
            return null;
    }

    public TableView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);

        maRowCountLabel = aLayout.AddLabeledEntry ("Row Count: ");
        maColumnCountLabel = aLayout.AddLabeledEntry ("Column Count: ");
        maCellCountLabel = aLayout.AddLabeledEntry ("Cell Count: ");
        maSelectedRowsLabel = aLayout.AddLabeledEntry ("Selected Rows: ");
        maSelectedColumnsLabel = aLayout.AddLabeledEntry ("Selected Columns: ");
    }


    public void SetObject (XAccessibleContext xContext)
    {
        mxTable = (XAccessibleTable)UnoRuntime.queryInterface(
            XAccessibleTable.class, xContext);
        super.SetObject (xContext);
    }


    public void Update ()
    {
        if (mxTable == null)
        {
            maRowCountLabel.setText ("<null object>");
            maColumnCountLabel.setText ("<null object>");
            maCellCountLabel.setText ("<null object>");
            maSelectedRowsLabel.setText ("<null object>");
            maSelectedColumnsLabel.setText ("<null object>");
        }
        else
        {
            int nRowCount = mxTable.getAccessibleRowCount();
            int nColumnCount = mxTable.getAccessibleColumnCount();
            maRowCountLabel.setText (Integer.toString (nRowCount));
            maColumnCountLabel.setText (Integer.toString (nColumnCount));
            maCellCountLabel.setText (Integer.toString (nRowCount*nColumnCount));

            StringBuffer sList = new StringBuffer();
            int[] aSelected = mxTable.getSelectedAccessibleRows();
            boolean bFirst = true;
            for (int i=0; i<aSelected.length; i++)
            {
                if ( ! bFirst)
                {
                    sList.append (", ");
                    bFirst = false;
                }
                sList.append (Integer.toString(aSelected[i]));
            }
            maSelectedRowsLabel.setText (sList.toString());
            sList = new StringBuffer();
            aSelected = mxTable.getSelectedAccessibleColumns();
            bFirst = true;
            for (int i=0; i<aSelected.length; i++)
            {
                if ( ! bFirst)
                {
                    sList.append (", ");
                    bFirst = false;
                }
                sList.append (Integer.toString(aSelected[i]));
            }
            maSelectedColumnsLabel.setText (sList.toString());
        }
    }




    public String GetTitle ()
    {
        return ("Table");
    }




    /** Listen for changes regarding displayed values.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            case AccessibleEventId.TABLE_MODEL_CHANGED :
            case AccessibleEventId.SELECTION_CHANGED:
                Update ();
        }
    }

    private XAccessibleTable mxTable;
    private JLabel
        maRowCountLabel,
        maColumnCountLabel,
        maCellCountLabel,
        maSelectedRowsLabel,
        maSelectedColumnsLabel;
}
