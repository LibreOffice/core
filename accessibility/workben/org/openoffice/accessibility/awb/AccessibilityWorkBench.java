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

package org.openoffice.accessibility.awb;

import java.awt.Cursor;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeWillExpandListener;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.misc.MessageArea;
import org.openoffice.accessibility.misc.Options;
import org.openoffice.accessibility.misc.OfficeConnection;
import org.openoffice.accessibility.misc.SimpleOffice;
import org.openoffice.accessibility.awb.canvas.Canvas;
import org.openoffice.accessibility.awb.tree.AccessibilityTree;
import org.openoffice.accessibility.awb.tree.AccessibilityModel;
import org.openoffice.accessibility.awb.tree.DynamicAccessibilityModel;
import org.openoffice.accessibility.awb.view.ObjectViewContainer;
import org.openoffice.accessibility.awb.view.ObjectViewContainerWindow;



/** This class manages the GUI of the work bench.
    @see AccessibilityTreeModel
        for the implementation of the tree view on the left side which also
        manages the registration of accessibility listeners.
    @see Canvas
        for the graphical view of the accessible objects.
*/
public class AccessibilityWorkBench
            extends JFrame
            implements XTerminateListener,
                       ActionListener,
                       TreeSelectionListener
{
    public static final String msVersion = "v1.9";
    public String msOptionsFileName = ".AWBrc";

    public static void main (String args[])
    {
        String sPipeName = System.getenv( "USER" );

        for (int i=0; i<args.length; i++)
        {
            if (args[i].equals ("-h") || args[i].equals ("--help") || args[i].equals ("-?"))
            {
                System.out.println ("usage: AccessibilityWorkBench <option>*");
                System.out.println ("options:");
                System.out.println (" -p <pipe-name>    name of the pipe to use to connect to OpenOffice.org.");
                System.out.println ("                   Defaults to $USER.");
                System.exit (0);
            }
            else if (args[i].equals ("-p"))
            {
                sPipeName = args[++i];
            }
        }

        saWorkBench = new AccessibilityWorkBench (sPipeName);
    }




    /** Return the one instance of the AccessibilityWorkBench
        @return
            Returns null when the AccessibilityWorkBench could not be
            created successfully.
    */
    public static AccessibilityWorkBench Instance ()
    {
        return saWorkBench;
    }



    /** Create an accessibility work bench that listens at the specified
        port to Office applications.
    */
    private AccessibilityWorkBench (String sPipeName)
    {
        mbInitialized = false;

        OfficeConnection.SetPipeName (sPipeName);
        Options.Instance().Load (msOptionsFileName);
        Layout ();

        MessageArea.println (System.getProperty ("os.name") + " / "
            + System.getProperty ("os.arch") + " / "
            + System.getProperty ("os.version"));
        MessageArea.println ("Using pipe name " + sPipeName);

        maTree.addTreeSelectionListener (this);

        addWindowListener (new WindowAdapter ()
            { public void windowClosing (WindowEvent e) {Quit();} }
            );

        OfficeConnection.Instance().AddConnectionListener (this);
        Initialize ();
    }




    /** Create and arrange the widgets of the GUI.
    */
    public void Layout ()
    {
        setSize (new java.awt.Dimension (800,600));

        JScrollPane aScrollPane;
        GridBagConstraints constraints;

        // Create new layout.
        java.awt.GridBagLayout aLayout = new java.awt.GridBagLayout ();
        getContentPane().setLayout (aLayout);

        //  Accessible Tree.
        javax.swing.tree.TreeModel treeModel = new DynamicAccessibilityModel();
        maTree = new AccessibilityTree(treeModel);
        // Add the model as tree listeners to be able to populate/clear the
        // child lists on demand.
        maTree.addTreeExpansionListener((TreeExpansionListener) treeModel);
        maTree.addTreeWillExpandListener((TreeWillExpandListener) treeModel);

        JScrollPane aTreeScrollPane = new JScrollPane(
            maTree,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        aTreeScrollPane.setPreferredSize (new java.awt.Dimension (400,300));

        // Object view shows details about the currently selected accessible
        // object.
        maObjectViewContainer = new ObjectViewContainer ();
        JScrollPane aObjectViewContainerScrollPane = new JScrollPane(
            maObjectViewContainer,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        aObjectViewContainerScrollPane.setPreferredSize (
            new java.awt.Dimension (400,300));
        JButton aCornerButton = new JButton ("CreateNewViewWindow");
        aCornerButton.addActionListener (this);
        aObjectViewContainerScrollPane.setCorner (
            JScrollPane.LOWER_RIGHT_CORNER,
            aCornerButton);

        // Split pane for tree view and object view.
        JSplitPane aLeftViewSplitPane = new JSplitPane (
            JSplitPane.VERTICAL_SPLIT,
            aTreeScrollPane,
            aObjectViewContainerScrollPane
            );
        aLeftViewSplitPane.setDividerLocation (300);
        aLeftViewSplitPane.setContinuousLayout (true);

        //  Canvas.
        maCanvas = new Canvas ();
        maCanvas.SetTree (maTree);
        JScrollPane aScrolledCanvas = new JScrollPane(maCanvas,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        aScrolledCanvas.getViewport().setBackground (java.awt.Color.RED);
        aScrolledCanvas.setPreferredSize (new java.awt.Dimension(600,400));

        // Split pane for tree view and canvas.
        JSplitPane aViewSplitPane = new JSplitPane (
            JSplitPane.HORIZONTAL_SPLIT,
            aLeftViewSplitPane,
            aScrolledCanvas
            );
        aViewSplitPane.setOneTouchExpandable(true);
        aViewSplitPane.setDividerLocation (400);
        aViewSplitPane.setContinuousLayout (true);

        // Split pane for the three views at the top and the message area.
        MessageArea.Instance().setPreferredSize (new java.awt.Dimension(600,50));
        JSplitPane aSplitPane = new JSplitPane (
            JSplitPane.VERTICAL_SPLIT,
            aViewSplitPane,
            MessageArea.Instance());
        aSplitPane.setOneTouchExpandable(true);
        aSplitPane.setContinuousLayout (true);
        addGridElement (aSplitPane, 0,0, 2,1, 3,3,
            GridBagConstraints.CENTER, GridBagConstraints.BOTH);

        // Button bar.
        maButtonBar = new javax.swing.JPanel();
        java.awt.GridBagLayout aButtonLayout = new java.awt.GridBagLayout ();
        maButtonBar.setLayout (new java.awt.FlowLayout());
        addGridElement (maButtonBar, 0,3, 2,1, 1,0,
            GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL);

        //  Buttons.
        //        maConnectButton = createButton ("Connect", "connect");
        //        maUpdateButton = createButton ("Update", "update");
        //        maShapesButton = createButton ("Expand Shapes", "shapes");
        maExpandButton = createButton ("Expand All", "expand");
        maQuitButton = createButton ("Quit", "quit");
        UpdateButtonStates ();

        setJMenuBar (CreateMenuBar ());

        setTitle("Accessibility Workbench " + msVersion);

        setVisible (true);
        pack ();
        aSplitPane.setDividerLocation (1.0);
        validate ();
        repaint();
    }




    /** Shortcut method for adding an object to a GridBagLayout.
    */
    void addGridElement (JComponent object,
        int x, int y,
        int width, int height,
        int weightx, int weighty,
        int anchor, int fill)
    {
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = x;
        constraints.gridy = y;
        constraints.gridwidth = width;
        constraints.gridheight = height;
        constraints.weightx = weightx;
        constraints.weighty = weighty;
        constraints.anchor = anchor;
        constraints.fill = fill;
        getContentPane().add (object, constraints);
    }




    /** Create a new button and place at the right most position into the
        button bar.
    */
    public JButton createButton (String title, String command)
    {
        JButton aButton = new JButton (title);
        aButton.setEnabled (false);
        aButton.setActionCommand (command);
        aButton.addActionListener (this);

        maButtonBar.add (aButton);
        return aButton;
    }




    /** Create a menu bar for the application.
        @return
            Returns the new menu bar.  The returned reference is also
            remembered in the data member <member>maMenuBar</member>.
    */
    javax.swing.JMenuBar CreateMenuBar()
    {
        // Menu bar.
        maMenuBar = new JMenuBar ();

        // File menu.
        JMenu aFileMenu = new JMenu ("File");
        maMenuBar.add (aFileMenu);
        JMenuItem aItem;
        aItem = new JMenuItem ("Quit");
        aFileMenu.add (aItem);
        aItem.addActionListener (this);

        // View menu.
        JMenu aViewMenu = new JMenu ("View");
        maMenuBar.add (aViewMenu);
        ButtonGroup aGroup = new ButtonGroup ();
        int nZoomMode = Options.GetInteger ("ZoomMode", Canvas.WHOLE_SCREEN);
        JRadioButtonMenuItem aRadioButton = new JRadioButtonMenuItem (
            "Whole Screen", nZoomMode==Canvas.WHOLE_SCREEN);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("200%", nZoomMode==200);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("100%", nZoomMode==100);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("50%", nZoomMode==50);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("25%", nZoomMode==25);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("10%", nZoomMode==10);
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);

        // Options menu.
        JMenu aOptionsMenu = new JMenu ("Options");
        maMenuBar.add (aOptionsMenu);
        JCheckBoxMenuItem aCBItem;
        aCBItem = new JCheckBoxMenuItem ("Show Descriptions",
                                         Options.GetBoolean("ShowDescriptions"));
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Show Names",
                                         Options.GetBoolean ("ShowNames"));
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Show Text",
                                         Options.GetBoolean ("ShowText"));
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Antialiased Rendering",
                                         Options.GetBoolean ("Antialiasing"));
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        // Help menu.
        JMenu aHelpMenu = new JMenu ("Help");
        maMenuBar.add (aHelpMenu);

        aItem = new JMenuItem ("Help");
        aHelpMenu.add (aItem);
        aItem.addActionListener (this);

        aItem = new JMenuItem ("News");
        aHelpMenu.add (aItem);
        aItem.addActionListener (this);

        aItem = new JMenuItem ("About");
        aHelpMenu.add (aItem);
        aItem.addActionListener (this);

        return maMenuBar;
    }




    /** Initialize the AWB.  This includes clearing the canvas, add
        listeners, creation of a new tree model for the tree list box and
        the update of the button states.

        This method may be called any number of times.  Note that all
        actions will be carried out every time.  The main purpose of a
        second call is that of a re-initialization after a reconnect.
    */
    protected void Initialize ()
    {
        maCanvas.SetTree (maTree);

        SimpleOffice aOffice = SimpleOffice.Instance ();
        if (aOffice != null)
        {
            // Add terminate listener.
            if (aOffice.GetDesktop() != null)
                aOffice.GetDesktop().addTerminateListener (this);

        }

        mbInitialized = true;
        UpdateButtonStates ();
    }




    /** Update the states of the buttons according to the internal state of
        the AWB.
    */
    protected void UpdateButtonStates ()
    {
        //        maConnectButton.setEnabled (mbInitialized);
        maQuitButton.setEnabled (true);
        //        maUpdateButton.setEnabled (mbInitialized);
        maExpandButton.setEnabled (mbInitialized);
        //        maShapesButton.setEnabled (mbInitialized);
    }



    /** Callback for GUI actions from the buttons.
    */
    public void actionPerformed (ActionEvent aEvent)
    {
        String sCommand = aEvent.getActionCommand();
        if (sCommand.equals("connect"))
        {
            SimpleOffice.Clear();
            Initialize ();
        }
        else if (sCommand.equals("quit"))
        {
            Quit ();
        }
        else if (sCommand.equals("update"))
        {
//          maTree.Dispose();
            Initialize ();
        }
        else if (sCommand.equals("shapes"))
        {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
            //            maTree.expandShapes();
            setCursor (aCursor);
        }
        else if (sCommand.equals("expand"))
        {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));

            for (int i=0; i<maTree.getRowCount(); i++)
                maTree.expandRow (i);
            //            maAccessibilityTree.expandAll();
            setCursor (aCursor);
        }
        else if (sCommand.equals ("Quit"))
        {
            System.out.println ("exiting");
            System.exit (0);
        }
        else if (sCommand.equals ("Show Descriptions"))
        {
            Options.SetBoolean ("ShowDescriptions",
                ((JCheckBoxMenuItem)aEvent.getSource()).getState());
            maCanvas.repaint();
        }
        else if (sCommand.equals ("Show Names"))
        {
            Options.SetBoolean ("ShowNames",
                ((JCheckBoxMenuItem)aEvent.getSource()).getState());
            maCanvas.repaint();
        }
        else if (sCommand.equals ("Show Text"))
        {
            Options.SetBoolean ("ShowText",
                ((JCheckBoxMenuItem)aEvent.getSource()).getState());
            maCanvas.repaint();
        }
        else if (sCommand.equals ("Antialiased Rendering"))
        {
            Options.SetBoolean ("Antialiasing",
                ((JCheckBoxMenuItem)aEvent.getSource()).getState());
            maCanvas.repaint();
        }
        else if (sCommand.equals ("Help"))
        {
            HelpWindow.Instance().loadFile ("help.html");
        }
        else if (sCommand.equals ("News"))
        {
            try{
                HelpWindow.Instance().loadFile ("news.html");
            } catch (Exception ex) {}
        }
        else if (sCommand.equals ("About"))
        {
            HelpWindow.Instance().loadFile ("about.html");
        }
        else if (sCommand.equals ("Whole Screen"))
        {
            Options.SetInteger ("ZoomMode", Canvas.WHOLE_SCREEN);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("200%"))
        {
            Options.SetInteger ("ZoomMode", 200);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("100%"))
        {
            Options.SetInteger ("ZoomMode", 100);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("50%"))
        {
            Options.SetInteger ("ZoomMode", 50);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("25%"))
        {
            Options.SetInteger ("ZoomMode", 25);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("10%"))
        {
            Options.SetInteger ("ZoomMode", 10);
            maCanvas.repaint();
        }
        else if (sCommand.equals ("<connected>"))
        {
            Connected ();
        }
        else if (sCommand.equals ("CreateNewViewWindow"))
        {
            TreePath aSelectionPath = maTree.getSelectionPath();
            if (aSelectionPath != null)
            {
                javax.swing.tree.TreeNode aSelectedNode =
                    (javax.swing.tree.TreeNode)aSelectionPath.getLastPathComponent();
                if (aSelectedNode instanceof XAccessible) {
                   new ObjectViewContainerWindow (((XAccessible) aSelectedNode).getAccessibleContext());
                }
            }
        }
        else
        {
            System.err.println("unknown command " + sCommand);
        }
    }




    /** TreeSelectionListener
        Tell the object view and the canvas about the selected object.
    */
    public void valueChanged (TreeSelectionEvent aEvent) {

        if (aEvent.isAddedPath()) {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));

            javax.swing.tree.TreePath aPath = aEvent.getPath();
            maTree.scrollPathToVisible (aPath);
            Object aObject = aPath.getLastPathComponent();
            implSetCurrentObject( aObject );
            if (aObject instanceof XAccessible)
            {
                if (maObjectViewContainer != null)
                    maObjectViewContainer.SetObject( ((XAccessible)aObject).getAccessibleContext() );
            }
            if (maCanvas != null)
                maCanvas.SelectObject ((TreeNode) aObject);
            setCursor (aCursor);
        } else {
            implSetCurrentObject( aEvent.getPath().getLastPathComponent() );
            if (maObjectViewContainer != null)
                maObjectViewContainer.SetObject (null);
            if (maCanvas != null)
                maCanvas.SelectObject (null);
        }
    }


    private void implSetCurrentObject( Object i_object )
    {
        if ( maObjectViewContainer == null )
            return;
        if ( maCurrentObject != null )
        {
            AccessibilityModel.removeEventListener( (TreeNode)maCurrentObject, maObjectViewContainer );
        }
        maCurrentObject = i_object;
        if ( maCurrentObject != null )
        {
            AccessibilityModel.addEventListener( (TreeNode)maCurrentObject, maObjectViewContainer );
        }
    }

    // XEventListener
    public void disposing (EventObject aSourceObj)
    {
        XFrame xFrame = (XFrame)UnoRuntime.queryInterface(
            XFrame.class, aSourceObj.Source);

        if( xFrame != null )
            System.out.println("frame disposed");
        else
            System.out.println("controller disposed");
    }




    // XTerminateListener
    public void queryTermination(final EventObject aEvent) throws com.sun.star.frame.TerminationVetoException
    {
        System.out.println ("Terminate Event : " + aEvent);
    }




    // XTerminateListener
    public void notifyTermination(final EventObject aEvent)
    {
        System.out.println ("Notifiy Termination Event : " + aEvent);
    }


    /** Called after the AWB is connected to an Office application.
    */
    private void Connected ()
    {
        // Clear the tree and by expanding the root node initiate the
        // scanning and insertion of nodes for the top-level windows.
//        maTree.Clear();
//        maTree.collapseRow (0);
//        maTree.expandRow (0);

        // Register the top window listener.
        XExtendedToolkit xToolkit =
            SimpleOffice.Instance().GetExtendedToolkit();
        if (xToolkit != null)
        {
            maTree.setToolkit(xToolkit);
        }
    }


    /** Called when shutting down the AWB tool.
    */
    private void Quit ()
    {
//        maTree.Dispose();
        System.exit (0);
    }

    /// The Singleton Workbench object.
    private static AccessibilityWorkBench
        saWorkBench = null;

    private JPanel maMainPanel;
    private JPanel maButtonBar;
    private Canvas maCanvas;
    private AccessibilityTree maTree;
    private ObjectViewContainer maObjectViewContainer;
    private JButton
        maConnectButton,
        maQuitButton,
        maUpdateButton,
        maExpandButton,
        maShapesButton;
    private JMenuBar maMenuBar;
    private boolean mbInitialized;
    private Object maCurrentObject = null;
}
