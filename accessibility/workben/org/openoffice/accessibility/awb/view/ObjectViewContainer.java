/*************************************************************************
 *
 *  $RCSfile: ObjectViewContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2003/09/19 09:21:42 $
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
import java.awt.Component;
import java.awt.Font;
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
import javax.swing.SwingUtilities;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.awb.view.ObjectView;



/** This container of specialized object views displays information about
    one accessible object.
    In this it plays several roles:
    1. Object container.
    2. Accessibility event dispatcher.
    3. Object view class registration manager.
    4. Swing widget.
*/
public class ObjectViewContainer
    extends JPanel
    implements XAccessibleEventListener
{
    public ObjectViewContainer ()
    {
        maFont = new Font ("Dialog", Font.PLAIN, 11);
        maViewTemplates = new Vector ();
        maViewBorder = BorderFactory.createBevelBorder (BevelBorder.RAISED);
        GridBagLayout aLayout = new GridBagLayout ();
        setLayout (aLayout);
        //        maLayoutManager = new LayoutManager (this);
        maLayoutManager = null;

        RegisterView (ContextView.class);
        RegisterView (ComponentView.class);
        RegisterView (ParentView.class);
        RegisterView (StateSetView.class);
        RegisterView (FocusView.class);
        RegisterView (TextView.class);
        RegisterView (EditableTextView.class);
        RegisterView (TableView.class);
        RegisterView (SelectionView.class);
        RegisterView (ServiceInterfaceView.class);
        RegisterView (EventMonitorView.class);

        mxContext = null;

        //        addMouseListener (maLayoutManager);
        //        addMouseMotionListener (maLayoutManager);
    }



    /** Remove all existing views and create new ones according to the
        interfaces supported by the given object.
    */
    public synchronized void SetObject (XAccessibleContext xContext)
    {
        // Call Destroy at all views to give them a chance to release their
        // resources.
        int n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).Destroy();
        // Remove existing views.
        removeAll ();

        mxContext = xContext;

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
                        aCreateMethod.invoke (
                            null, new Object[] {this, xContext});
                    Add (aView);
                }
            }
            catch (NoSuchMethodException e)
            {System.err.println ("Caught exception while creating view "
                + i + " : " + e);}
            catch (IllegalAccessException e)
            {System.err.println ("Caught exception while creating view "
                + i + " : " + e);}
            catch (InvocationTargetException e)
            {System.err.println ("Caught exception while creating view "
                + i + " : " + e);}
        }

        UpdateLayoutManager ();

        // Now set the object at all views.
        n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).SetObject (xContext);

        setPreferredSize (getLayout().preferredLayoutSize (this));
        ((GridBagLayout) getLayout()).invalidateLayout(this);
        validate();
    }




    /** Add the given class to the list of classes which will be
        instantiated the next time an accessible object is set.
    */
    public void RegisterView (Class aObjectViewClass)
    {
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


    /** Return a font that should be used for widgets in the views.
    */
    public Font GetViewFont ()
    {
        return maFont;
    }

    public Color GetErrorColor ()
    {
        return new Color (255,80,50);
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
        if (getComponentCount() > 1000)
        {
            Component aComponent = getComponent (getComponentCount()-1);
            GridBagLayout aLayout = (GridBagLayout)getLayout();
            GridBagConstraints aConstraints = aLayout.getConstraints (aComponent);
            aConstraints.weighty = 1;
            aLayout.setConstraints (aComponent, aConstraints);
        }
    }




    /** Put the event just received into the event queue which will deliver
        it soon asynchronuously to the DispatchEvent method.
    */
    public void notifyEvent (final AccessibleEventObject aEvent)
    {
        SwingUtilities.invokeLater(
            new Runnable()
            {
                public void run()
                {
                    DispatchEvent (aEvent);
                }
            }
        );
    }




    /** Forward accessibility events to all views without them being
        registered as event listeners each on their own.
    */
    private void DispatchEvent (AccessibleEventObject aEvent)
    {
        int n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).notifyEvent (aEvent);
    }



    /** When the object is disposed that is displayed by the views of this
        container then tell all views about this.
    */
    public void disposing (EventObject aEvent)
    {
        mxContext = null;
       SwingUtilities.invokeLater(
            new Runnable()
            {
                public void run()
                {
                    SetObject (null);
                }
            }
            );
    }




    /// The current accessible context display by the views.
    private XAccessibleContext mxContext;

    /// Observe this tree for selection changes and notify them to all
    /// children.
    private JTree maTree;
    private Border maViewBorder;
    /// List of view templates which are instantiated when new object is set.
    private Vector maViewTemplates;
    private Font maFont;
    private LayoutManager maLayoutManager;
}
