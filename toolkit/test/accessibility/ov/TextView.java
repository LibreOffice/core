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

package ov;

import java.awt.Color;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JLabel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.uno.UnoRuntime;

public class TextView
    extends ListeningObjectView
{
    /** Create a TextView when the given object supports the
        XAccessibleText interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleText xText = (XAccessibleText)UnoRuntime.queryInterface(
                XAccessibleText.class, xContext);
        if (xText != null)
            return new TextView (aContainer);
        else
            return null;
    }


    public TextView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        setLayout (new GridBagLayout());
        GridBagConstraints aConstraints = new GridBagConstraints ();

        JLabel aLabel = new JLabel ("Text:");
        aConstraints.gridy = 0;
        aConstraints.weightx = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (aLabel, aConstraints);

        maTextLabel = new JLabel ("");
        aConstraints.gridx = 1;
        aConstraints.fill = GridBagConstraints.NONE;
        aConstraints.anchor = GridBagConstraints.WEST;
        add (maTextLabel, aConstraints);

        aLabel = new JLabel ("Caret position:");
        aConstraints.gridx = 0;
        aConstraints.gridy = 1;
        aConstraints.weightx = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (aLabel, aConstraints);

        maCaretPositionLabel = new JLabel ("");
        aConstraints.gridx = 1;
        aConstraints.fill = GridBagConstraints.NONE;
        aConstraints.anchor = GridBagConstraints.WEST;
        add (maCaretPositionLabel, aConstraints);
    }


    /** Additionally to the context store a reference to the
        XAccessibleText interface.
    */
    public void SetObject (XAccessibleContext xObject)
    {
        mxText = (XAccessibleText)UnoRuntime.queryInterface(
            XAccessibleText.class, xObject);
        super.SetObject (xObject);
    }

    synchronized public void Destroy ()
    {
        super.Destroy();
    }

    synchronized public void Update ()
    {
        if (mxText == null)
        {
            maTextLabel.setText ("<null object>");
            maCaretPositionLabel.setText ("<null object>");
        }
        else
        {
            maTextLabel.setText (mxText.getText());
            maCaretPositionLabel.setText (Integer.toString(mxText.getCaretPosition()));
        }
    }

    public String GetTitle ()
    {
        return ("Text");
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        System.out.println (aEvent);
        switch (aEvent.EventId)
        {
            case AccessibleEventId.TEXT_CHANGED :
            case AccessibleEventId.CARET_CHANGED :
                Update ();
                break;
        }
    }

    private JLabel
        maTextLabel,
        maCaretPositionLabel;
    private XAccessibleText mxText;
}
