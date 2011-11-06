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
