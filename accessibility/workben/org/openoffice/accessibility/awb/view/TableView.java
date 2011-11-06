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
