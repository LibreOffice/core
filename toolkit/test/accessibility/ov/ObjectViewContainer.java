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
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

import java.util.Vector;

import java.lang.reflect.Method;
import java.lang.NoSuchMethodException;
import java.lang.IllegalAccessException;
import java.lang.reflect.InvocationTargetException;

import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.BorderFactory;
import javax.swing.border.Border;
import javax.swing.border.BevelBorder;

import com.sun.star.accessibility.XAccessibleContext;


public class ObjectViewContainer
    extends JPanel
{
    public ObjectViewContainer ()
    {
        maViewTemplates = new Vector ();
        maViewBorder = BorderFactory.createBevelBorder (BevelBorder.RAISED);
        setLayout (new GridBagLayout ());

        System.out.println ("ObjectViewContainer");
        RegisterView (ContextView.class);
        //        RegisterView (StateSetView.class);
        RegisterView (FocusView.class);
        RegisterView (TextView.class);
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
            try
            {
                Class aViewClass = (Class)maViewTemplates.elementAt (i);
                Method aCreateMethod = aViewClass.getDeclaredMethod (
                    "Create", new Class[] {
                        ObjectViewContainer.class,
                        XAccessibleContext.class});
                if (aCreateMethod != null)
                {
                    ObjectView aView = (ObjectView)
                        aCreateMethod.invoke (null, new Object[] {this, xContext});
                    Add (aView);
                }
            }
            catch (NoSuchMethodException e)
            {System.err.println ("Caught exception while creating view " + i + " : " + e);}
            catch (IllegalAccessException e)
            {System.err.println ("Caught exception while creating view " + i + " : " + e);}
            catch (InvocationTargetException e)
            {System.err.println ("Caught exception while creating view " + i + " : " + e);}
        }

        UpdateLayoutManager ();

        // Now set the object at all views.
        n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).SetObject (xContext);

        setPreferredSize (getLayout().preferredLayoutSize (this));
    }


    /** Add the given class to the list of classes which will be
        instantiated the next time an accessible object is set.
    */
    public void RegisterView (Class aObjectViewClass)
    {
        System.out.println ("registering " + aObjectViewClass);
        maViewTemplates.addElement (aObjectViewClass);
    }

    /** Replace one view class with another.
    */
    public void ReplaceView (Class aObjectViewClass, Class aSubstitution)
    {
        int nIndex = maViewTemplates.indexOf (aObjectViewClass);
        if (nIndex >= 0)
            maViewTemplates.setElementAt (aSubstitution, nIndex);
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

    /// Observe this tree for selection changes and notify them to all
    /// children.
    private JTree maTree;
    private Border maViewBorder;
    /// List of view templates which are instantiated when new object is set.
    private Vector maViewTemplates;
}
