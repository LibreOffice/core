/*************************************************************************
 *
 *  $RCSfile: ParentView.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: af $ $Date: 2003/06/13 16:30:36 $
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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.lang.Integer;
import javax.swing.JLabel;
import javax.swing.JTextField;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.lang.IndexOutOfBoundsException;

import org.openoffice.accessibility.misc.NameProvider;


/** Show informations related to the parent/child relationship.
*/
public class ParentView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (xContext != null)
            return new ParentView (aContainer);
        else
            return null;
    }

    public ParentView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        ViewGridLayout aLayout = new ViewGridLayout (this);
        maParentLabel = aLayout.AddLabeledEntry ("Has parent: ");
        maIndexLabel = aLayout.AddLabeledEntry ("Index in parent: ");
        maValidLabel = aLayout.AddLabeledEntry ("Parent/Child relationship valid: ");
        maChildrenLabel = aLayout.AddLabeledEntry ("Child count: ");
    }

    public void Update ()
    {
        if (mxContext == null)
        {
            maParentLabel.setText ("<null object>");
            maIndexLabel.setText ("<null object>");
            maValidLabel.setText ("<null object>");
            maChildrenLabel.setText ("<null object>");
        }
        else
        {
            XAccessible xParent = mxContext.getAccessibleParent();
            int nIndex = mxContext.getAccessibleIndexInParent();
            maIndexLabel.setText (Integer.toString(nIndex));
            if (xParent != null)
            {
                maParentLabel.setText ("yes");
                XAccessibleContext xParentContext =
                    xParent.getAccessibleContext();
                if (xParentContext != null)
                {
                    try
                    {
                        XAccessible xChild =
                            xParentContext.getAccessibleChild(nIndex);
                        if (xChild != mxContext)
                            maValidLabel.setText ("yes");
                        else
                        {
                            maValidLabel.setText ("no");
                            maValidLabel.setBackground (GetContainer().GetErrorColor());
                        }
                    }
                    catch (IndexOutOfBoundsException e)
                    {
                        maValidLabel.setText ("no: invalid index in parent");
                        maValidLabel.setBackground (GetContainer().GetErrorColor());
                    }
                }
                else
                {
                    maValidLabel.setText ("no: parent has no context");
                    maValidLabel.setBackground (GetContainer().GetErrorColor());
                }
            }
            else
                maParentLabel.setText ("no");
            maChildrenLabel.setText (Integer.toString(mxContext.getAccessibleChildCount()));
        }
    }

    public String GetTitle ()
    {
        return ("Parent");
    }


    /** Listen for changes regarding displayed values.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {
        switch (aEvent.EventId)
        {
            default:
                Update ();
        }
    }


    private JLabel
        maParentLabel,
        maIndexLabel,
        maValidLabel,
        maChildrenLabel;
}
