/*************************************************************************
 *
 *  $RCSfile: TableView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
