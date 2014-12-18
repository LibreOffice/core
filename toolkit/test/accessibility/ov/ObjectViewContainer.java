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

package ov;

import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.util.ArrayList;

import javax.swing.BorderFactory;
import javax.swing.JPanel;
import javax.swing.border.BevelBorder;
import javax.swing.border.Border;

import com.sun.star.accessibility.XAccessibleContext;


public class ObjectViewContainer
    extends JPanel
{
    private static interface IViewFactory {
        ObjectView Create (
                ObjectViewContainer aContainer,
                XAccessibleContext xContext);
    }

    public ObjectViewContainer ()
    {
        maViewTemplates = new ArrayList<IViewFactory>();
        maViewBorder = BorderFactory.createBevelBorder (BevelBorder.RAISED);
        setLayout (new GridBagLayout ());

        maViewTemplates.add(new IViewFactory() {
            public ObjectView Create(ObjectViewContainer aContainer,
                    XAccessibleContext xContext) {
                return ContextView.Create(aContainer, xContext);
            }
        });
        maViewTemplates.add(new IViewFactory() {
            public ObjectView Create(ObjectViewContainer aContainer,
                    XAccessibleContext xContext) {
                return FocusView.Create(aContainer, xContext);
            }
        });
        maViewTemplates.add(new IViewFactory() {
            public ObjectView Create(ObjectViewContainer aContainer,
                    XAccessibleContext xContext) {
                return TextView.Create(aContainer, xContext);
            }
        });
    }

    /** Remove all existing views and create new ones according to the
        interfaces supported by the given object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        // Call Destroy at all views to give them a chance to release their
        // resources.
        int n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).Destroy();
        // Remove existing views.
        removeAll ();

        // Add new views.
        for (int i=0; i<maViewTemplates.size(); i++)
        {
            IViewFactory aViewFactory = maViewTemplates.get(i);
            ObjectView aView = aViewFactory.Create(this, xContext);
            Add (aView);
        }

        UpdateLayoutManager ();

        // Now set the object at all views.
        n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).SetObject (xContext);

        setPreferredSize (getLayout().preferredLayoutSize (this));
    }


    /** Add an object view and place it below all previously added views.
        @param aView
            This argument may be null.  In this case nothing happens.
    */
    private void Add (ObjectView aView)
    {
        if (aView != null)
        {
            GridBagConstraints constraints = new GridBagConstraints ();
            constraints.gridx = 0;
            constraints.gridy = getComponentCount();
            constraints.gridwidth = 1;
            constraints.gridheight = 1;
            constraints.weightx = 1;
            constraints.weighty = 0;
            constraints.ipadx = 2;
            constraints.ipady = 5;
            constraints.insets = new Insets (5,5,5,5);
            constraints.anchor = GridBagConstraints.NORTH;
            constraints.fill = GridBagConstraints.HORIZONTAL;

            aView.setBorder (
                BorderFactory.createTitledBorder (
                    maViewBorder, aView.GetTitle()));

            add (aView, constraints);
        }
    }

    /** Update the layout manager by setting the vertical weight of the
        bottom entry to 1 and so make it strech to over the available
        space.

    */
    private void UpdateLayoutManager ()
    {
        // Adapt the layout manager.
        if (getComponentCount() > 0)
        {
            Component aComponent = getComponent (getComponentCount()-1);
            GridBagLayout aLayout = (GridBagLayout)getLayout();
            GridBagConstraints aConstraints = aLayout.getConstraints (aComponent);
            aConstraints.weighty = 1;
            aLayout.setConstraints (aComponent, aConstraints);
        }
    }

    private final Border maViewBorder;
    /// List of view templates which are instantiated when new object is set.
    private final ArrayList<IViewFactory> maViewTemplates;
}
