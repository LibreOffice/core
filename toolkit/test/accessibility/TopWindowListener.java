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

import com.sun.star.awt.XWindow;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.uno.UnoRuntime;

/** Listen for top window events and create or delete children of the tree
    model accordingly.
*/
class TopWindowListener
{
    TopWindowListener (AccessibilityTreeModel aModel, SimpleOffice aOffice)
    {
        maModel = aModel;
        maOffice = aOffice;
    }




    /** Use this function to initially fill the accessibility object tree
        view with nodes for top level windows.
    */
    public void Initialize ()
    {
        XExtendedToolkit xToolkit = maOffice.getExtendedToolkit();
        if (xToolkit != null)
        {
            maModel.lock ();
            int nTopWindowCount = xToolkit.getTopWindowCount();
            MessageArea.println ("There are " + nTopWindowCount + " top windows.");
            for (int i=0; i<nTopWindowCount; i++)
            {
                try
                {
                    XAccessible xAccessible = maOffice.getAccessibleObject(
                        xToolkit.getTopWindow (i));
                    // Uncomment the following line to get the real root of
                    //   the accessible tree that xAccessible belongs to.
                    // xAccessible = maOffice.getAccessibleRoot(xAccessible);
                    AddTopLevelNode (xAccessible);
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception: " + e);
                    e.printStackTrace();
                }
            }
            maModel.unlock ((AccessibleTreeNode)maModel.getRoot());
        }
    }



    /** Add a new top level node which, to be exact, will be placed on the
        second layer of the tree.
        @param xNewTopLevelObject
            The accessible object of the new top level window.
    */
    private void AddTopLevelNode (XAccessible xNewTopLevelObject)
    {
        System.out.println ("adding top level window");
        if (xNewTopLevelObject != null)
        {
            XAccessibleContext xContext = xNewTopLevelObject.getAccessibleContext();
            if (xContext == null)
                System.out.println ("top level window not accessible");
            else
            {
                if ( ! FilterTopLevelNode (xContext))
                {
                    Object aRootObject = maModel.getRoot();
                    if (aRootObject instanceof VectorNode)
                    {
                        VectorNode aRoot = (VectorNode) aRootObject;
                        AccessibleTreeNode aNode =
                            NodeFactory.Instance().createDefaultNode (xNewTopLevelObject, aRoot);
                        aRoot.addChild (aNode);
                        maModel.fireTreeNodesInserted (maModel.createEvent (aRoot, aNode));
                    }
                }
            }
        }
    }

    /** Ignore windows that have no accessible name, i.e. do not represent
        document windows.
        @return
            Returns <true/> when the given object should not be displayed,
            i.e. filtered out.
    */
    private boolean FilterTopLevelNode (XAccessibleContext xContext)
    {
        // No filtering at the moment.
        return false;
        //        return xContext.getAccessibleName().length() == 0;
    }




    /** Remove an existing top level node from the tree.
        @param xNewTopLevelObject
            The accessible object to remove.
    */
    private void RemoveTopLevelNode (XAccessible xTopLevelObject)
    {
        Object aObject = maModel.getRoot();
        if (aObject instanceof VectorNode && xTopLevelObject != null)
        {
            System.out.println ("removing node " + xTopLevelObject);
            VectorNode aRoot = (VectorNode) aObject;
            maModel.removeNode (xTopLevelObject.getAccessibleContext());
        }
    }





    /**  This method exists for debugging.  It prints a list of all top
         level windows.
    */
    private void ShowAllTopLevelWindows ()
    {
        XExtendedToolkit xToolkit = maOffice.getExtendedToolkit();
        if (xToolkit != null)
        {
            int nTopWindowCount = xToolkit.getTopWindowCount();
            for (int i=0; i<nTopWindowCount; i++)
            {
                try
                {
                    System.out.println (i + " : " + xToolkit.getTopWindow (i));
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception; " + e);
                }
            }
        }
    }




    // XTopWindowListener
    public void windowOpened (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        if (maModel != null)
        {
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(
                XWindow.class, aEvent.Source);
            if (xWindow == null)
                System.out.println ("event source is no XWindow");
            else
            {
                XAccessible xAccessible = maOffice.getAccessibleObject(xWindow);
                if (xAccessible == null)
                    System.out.println ("event source is no XAccessible");
                else
                    AddTopLevelNode (xAccessible);
            }
        }
    }




    public void windowClosed (final com.sun.star.lang.EventObject aEvent)
        throws RuntimeException
    {
        if (maModel != null)
        {
            XWindow xWindow = (XWindow) UnoRuntime.queryInterface(
                XWindow.class, aEvent.Source);
            if (xWindow == null)
                System.out.println ("event source is no XWindow");
            else
            {
                XAccessible xAccessible = maOffice.getAccessibleObject(xWindow);
                if (xAccessible == null)
                    System.out.println ("event source is no XAccessible");
                else
                    RemoveTopLevelNode (xAccessible);
            }
        }
    }

    public void disposing (final com.sun.star.lang.EventObject aEvent)
    {
        System.out.println ("Top window disposed: " + aEvent);
    }




    private AccessibilityTreeModel
        maModel;
    private SimpleOffice
        maOffice;
}
