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

import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeSelectionEvent;
import ov.ObjectViewContainer;

/** This class manages the GUI of the work bench.
    @see AccessibilityTreeModel
        for the implementation of the tree view on the left side which also
        manages the registration of accessibility listeners.
    @see Canvas
        for the graphical view of the accessible objects.
*/
public class AccessibilityWorkBench
    extends JFrame
    implements ActionListener, XTerminateListener, TreeSelectionListener

{
    public static final String msVersion = "v1.7.2";
    public String msOptionsFileName = ".AWBrc";

    public static void main (String args[])
    {
        int nPortNumber = 5678;

        for (int i=0; i<args.length; i++)
        {
            if (args[i].equals ("-h") || args[i].equals ("--help") || args[i].equals ("-?"))
            {
                System.out.println ("usage: AccessibilityWorkBench <option>*");
                System.out.println ("options:");
                System.out.println ("   -p <port-number>   Port on which to connect to StarOffice.");
                System.out.println ("                      Defaults to 5678.");
                System.exit (0);
            }
            else if (args[i].equals ("-p"))
            {
                nPortNumber = Integer.parseInt (args[++i]);
            }
        }

        saWorkBench = new AccessibilityWorkBench (nPortNumber);
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
    private AccessibilityWorkBench (int nPortNumber)
    {
        mbInitialized = false;

        Layout ();

        MessageArea.println (System.getProperty ("os.name") + " / "
            + System.getProperty ("os.arch") + " / "
            + System.getProperty ("os.version"));
        MessageArea.println ("Using port " + nPortNumber);
        office = new SimpleOffice (nPortNumber);
        info = new InformationWriter ();

        maAccessibilityTree.getComponent().addTreeSelectionListener (this);

        addWindowListener (new WindowAdapter ()
            { public void windowClosing (WindowEvent e)
                  { System.exit(0); }
            });

        initialize ();
    }




    /** Create and arrange the widgets of the GUI.
    */
    public void Layout  ()
    {
        setSize (new Dimension (8000,600));

        JScrollPane aScrollPane;
        GridBagConstraints constraints;

        // Create new layout.
        GridBagLayout aLayout = new GridBagLayout ();
        getContentPane().setLayout (aLayout);

        //  Accessible Tree.
        maAccessibilityTree = new AccessibilityTree ();
        //        maAccessibilityTree.getComponent().setMinimumSize (new Dimension (250,300));
        JScrollPane aTreeScrollPane = new JScrollPane(
            maAccessibilityTree.getComponent(),
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        aTreeScrollPane.setPreferredSize (new Dimension (400,300));

        // Object view shows details about the currently selected accessible
        // object.
        maObjectViewContainer = new ObjectViewContainer ();
        //        maObjectViewContainer.setPreferredSize (new Dimension (300,100));
        JScrollPane aObjectViewContainerScrollPane = new JScrollPane(
            maObjectViewContainer,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        aObjectViewContainerScrollPane.setPreferredSize (new Dimension (400,300));

        // Split pane for tree view and object view.
        JSplitPane aLeftViewSplitPane = new JSplitPane (
            JSplitPane.VERTICAL_SPLIT,
            aTreeScrollPane,
            aObjectViewContainerScrollPane
            );
        aLeftViewSplitPane.setDividerLocation (300);

        //  Canvas.
        maCanvas = new Canvas ();
        maCanvas.setTree (maAccessibilityTree.getComponent());
        maAccessibilityTree.SetCanvas (maCanvas);
        JScrollPane aScrolledCanvas = new JScrollPane(maCanvas,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        aScrolledCanvas.getViewport().setBackground (Color.RED);
        aScrolledCanvas.setPreferredSize (new Dimension(600,400));

        // Split pane for tree view and canvas.
        JSplitPane aViewSplitPane = new JSplitPane (
            JSplitPane.HORIZONTAL_SPLIT,
            aLeftViewSplitPane,
            aScrolledCanvas
            );
        aViewSplitPane.setOneTouchExpandable(true);
        aViewSplitPane.setDividerLocation (400);

        //  Text output area.
        maMessageArea = MessageArea.Instance ();
        //        maMessageArea.setPreferredSize (new Dimension (300,50));

        // Split pane for the two views and the message area.
        JSplitPane aSplitPane = new JSplitPane (JSplitPane.VERTICAL_SPLIT,
            aViewSplitPane, maMessageArea);
        aSplitPane.setOneTouchExpandable(true);
        addGridElement (aViewSplitPane, 0,0, 2,1, 3,3,
            GridBagConstraints.CENTER, GridBagConstraints.BOTH);

        // Button bar.
        maButtonBar = new JPanel();
        GridBagLayout aButtonLayout = new GridBagLayout ();
        maButtonBar.setLayout (new FlowLayout());
        addGridElement (maButtonBar, 0,3, 2,1, 1,0,
            GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL);

        //  Buttons.
        aConnectButton = createButton ("Connect", "connect");
        aUpdateButton = createButton ("Update", "update");
        aShapesButton = createButton ("Expand Shapes", "shapes");
        aExpandButton = createButton ("Expand All", "expand");
        aQuitButton = createButton ("Quit", "quit");
        UpdateButtonStates ();

        Options.Instance().Load (msOptionsFileName);

        setJMenuBar (CreateMenuBar ());

        setTitle("Accessibility Workbench " + msVersion);

        pack ();
        setVisible (true);
        validate ();
        repaint();
    }




    /** Shortcut method for adding an object to a GridBagLayout.
    */
    void addGridElement (JComponent object,
        int x, int y, int width, int height, int weightx, int weighty,
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
    JMenuBar CreateMenuBar ()
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
        JRadioButtonMenuItem aRadioButton = new JRadioButtonMenuItem ("Whole Screen");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("200%");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("100%");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("50%");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("25%");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);
        aRadioButton = new JRadioButtonMenuItem ("10%");
        aGroup.add (aRadioButton);
        aViewMenu.add (aRadioButton);
        aRadioButton.addActionListener (this);

        // Options menu.
        JMenu aOptionsMenu = new JMenu ("Options");
        maMenuBar.add (aOptionsMenu);
        JCheckBoxMenuItem aCBItem;
        aCBItem = new JCheckBoxMenuItem ("Show Descriptions", maCanvas.getShowDescriptions());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Show Names", maCanvas.getShowNames());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Show Text", maCanvas.getShowText());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new JCheckBoxMenuItem ("Antialiased Rendering", maCanvas.getAntialiasing());
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
    protected void initialize ()
    {
        maCanvas.clear();

        AccessibilityTreeModel aModel = null;
        aModel = new AccessibilityTreeModel (createTreeModelRoot());

        aModel.setCanvas (maCanvas);
        maAccessibilityTree.getComponent().setModel (aModel);

        if (office != null)
        {
            // Add terminate listener.
            if (office.getDesktop() != null)
                office.getDesktop().addTerminateListener (this);

            XExtendedToolkit xToolkit = office.getExtendedToolkit();
            // Remove old top window listener.
            if (maTopWindowListener != null)
                xToolkit.removeTopWindowListener (maQueuedTopWindowListener);
            // Add top window listener.
            if (xToolkit != null)
            {
                MessageArea.println ("registering at extended toolkit");
                maTopWindowListener = new TopWindowListener (aModel, office);
                maQueuedTopWindowListener = new QueuedTopWindowListener (maTopWindowListener);
                xToolkit.addTopWindowListener (maQueuedTopWindowListener);
                maTopWindowListener.Initialize ();
            }
            else
                maTopWindowListener = null;
        }

        mbInitialized = true;
        UpdateButtonStates ();
    }




    /** Update the states of the buttons according to the internal state of
        the AWB.
    */
    protected void UpdateButtonStates ()
    {
        aConnectButton.setEnabled (mbInitialized);
        aQuitButton.setEnabled (mbInitialized);
        aUpdateButton.setEnabled (mbInitialized);
        aExpandButton.setEnabled (mbInitialized);
        aShapesButton.setEnabled (mbInitialized);
    }



    /** Callback for GUI actions from the buttons.
    */
    public void actionPerformed (java.awt.event.ActionEvent e)
    {
        if (e.getActionCommand().equals("connect"))
        {
            office.connect();
            initialize ();
        }
        else if (e.getActionCommand().equals("quit"))
        {
            AccessibilityTreeModel aModel = (AccessibilityTreeModel)maAccessibilityTree.getComponent().getModel();
            aModel.clear();
            System.exit (0);
        }
        else if (e.getActionCommand().equals("update"))
        {
            initialize ();
        }
        else if (e.getActionCommand().equals("shapes"))
        {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
            maAccessibilityTree.expandShapes();
            setCursor (aCursor);
        }
        else if (e.getActionCommand().equals("expand"))
        {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
            maAccessibilityTree.expandAll();
            setCursor (aCursor);
        }
        else if (e.getActionCommand().equals ("Quit"))
        {
            System.out.println ("exiting");
            System.exit (0);
        }
        else if (e.getActionCommand().equals ("Show Descriptions"))
        {
            maCanvas.setShowDescriptions ( ! maCanvas.getShowDescriptions());
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("Show Names"))
        {
            maCanvas.setShowNames ( ! maCanvas.getShowNames());
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("Antialiased Rendering"))
        {
            maCanvas.setAntialiasing ( ! maCanvas.getAntialiasing());
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("Help"))
        {
            HelpWindow.Instance().loadFile ("help.html");
        }
        else if (e.getActionCommand().equals ("News"))
        {
            try{
                HelpWindow.Instance().loadFile ("news.html");
            } catch (Exception ex) {}
        }
        else if (e.getActionCommand().equals ("About"))
        {
            HelpWindow.Instance().loadFile ("about.html");
        }
        else if (e.getActionCommand().equals ("Whole Screen"))
        {
            maCanvas.setZoomMode (Canvas.WHOLE_SCREEN);
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("200%"))
        {
            maCanvas.setZoomMode (200);
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("100%"))
        {
            maCanvas.setZoomMode (100);
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("50%"))
        {
            maCanvas.setZoomMode (50);
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("25%"))
        {
            maCanvas.setZoomMode (25);
            Options.Instance().Save (msOptionsFileName);
        }
        else if (e.getActionCommand().equals ("10%"))
        {
            maCanvas.setZoomMode (10);
            Options.Instance().Save (msOptionsFileName);
        }
        else
        {
            System.err.println("unknown command " + e.getActionCommand());
        }
    }




    /** Create an AccessibilityTreeModel root which contains the documents
        (top windows) that are present at the moment.
    */
    private AccessibleTreeNode createTreeModelRoot()
    {
        // create root node
        VectorNode aRoot = new VectorNode ("Accessibility Tree", null);
        if (maTopWindowListener != null)
            maTopWindowListener.Initialize ();
        return aRoot;
    }


    // TreeSelectionListener
    public void valueChanged (TreeSelectionEvent aEvent)
    {
        TreePath aPath = aEvent.getPath();
        Object aObject = aPath.getLastPathComponent();
        if (aObject instanceof AccTreeNode)
        {
            AccTreeNode aNode = (AccTreeNode) aObject;
            XAccessibleContext xContext = aNode.getContext();
            maObjectViewContainer.SetObject (xContext);
        }
    }




    // XEventListener
    public void disposing( com.sun.star.lang.EventObject aSourceObj )
    {
        XFrame xFrame = (XFrame)UnoRuntime.queryInterface(XFrame.class, aSourceObj.Source);

        if( xFrame != null )
            System.out.println("frame disposed");
        else
            System.out.println("controller disposed");
    }




    // XTerminateListener
    public void queryTermination (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Terminate Event : " + aEvent);
    }




    // XTerminateListener
    public void notifyTermination (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Notifiy Termination Event : " + aEvent);
    }



    /// The Singleton Workbench object.
    private static AccessibilityWorkBench
        saWorkBench = null;

    protected SimpleOffice
        office;
    protected InformationWriter
        info;

    private XModel
        mxModel;
    private JPanel
        maMainPanel,
        maButtonBar;
    private Canvas
        maCanvas;
    private AccessibilityTree
        maAccessibilityTree;
    private ObjectViewContainer
        maObjectViewContainer;
    private JScrollPane
        maScrollPane;
    private MessageArea
        maMessageArea;
    private JButton
        aConnectButton,
        aQuitButton,
        aUpdateButton,
        aExpandButton,
        aShapesButton;
    private JMenuBar
        maMenuBar;
    private String
        msMessage;
    private boolean
        mbInitialized;
    private TopWindowListener
        maTopWindowListener;
    private QueuedTopWindowListener
        maQueuedTopWindowListener;
}
