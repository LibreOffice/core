import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.document.XEventListener;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawView;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XFrameActionListener;
import com.sun.star.frame.FrameActionEvent;
import com.sun.star.frame.FrameAction;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XTerminateListener;
import com.sun.star.uno.UnoRuntime;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;

import drafts.com.sun.star.awt.XExtendedToolkit;

import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.io.*;

/** This class manages the GUI of the work bench.
    @see AccessibilityTreeModel
        for the implementation of the tree view on the left side which also
        manages the registration of accessibility listeners.
    @see Canvas
        for the graphical view of the accessible objects.
*/
public class AccessibilityWorkBench
    extends JFrame
    implements ActionListener,
        XTerminateListener
{
    public static final String msVersion = "v1.7";
    public String msOptionsFileName = ".AWBrc";

    public static void main (String args[])
    {
        int nPortNumber = 5678;
        String sFileName
            = "file:///tmp/impress-test-document.sxi";
        //            = "file:///tmp/draw-test-document.sxd";

        for (int i=0; i<args.length; i++)
        {
            if (args[i].equals ("-h") || args[i].equals ("--help") || args[i].equals ("-?"))
            {
                System.out.println ("usage: AccessibilityWorkBench <option>*");
                System.out.println ("options:");
                System.out.println ("   -p <port-number>   Port on which to connect to StarOffice.");
                System.out.println ("                      Defaults to 5678.");
                System.out.println ("   -f <file-name-URL> URL of document file which is loaded when");
                System.out.println ("                      clicking on the Load button.  Don't forget");
                System.out.println ("                      the file:// prefix!");
                System.exit (0);
            }
            else if (args[i].equals ("-p"))
            {
                nPortNumber = Integer.parseInt (args[++i]);
            }
            else if (args[i].equals ("-f"))
            {
                sFileName = args[++i];
            }
        }

        aWorkBench = new AccessibilityWorkBench (nPortNumber);
    }


    /** the Singleton Workbench object */
    private static AccessibilityWorkBench aWorkBench;

    /** access to the Singleton Workbench object */
    public static AccessibilityWorkBench get()
    {
        return aWorkBench;
    }


    public  AccessibilityWorkBench (int nPortNumber)
    {
        mbInitialized = false;

        Layout ();
        EventLogger.Instance();

        MessageArea.println (System.getProperty ("os.name") + " / "
            + System.getProperty ("os.arch") + " / "
            + System.getProperty ("os.version"));
        MessageArea.println ("Using port " + nPortNumber);
        office = new SimpleOffice (nPortNumber);
        info = new InformationWriter ();

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
        JScrollPane aScrollPane;
        GridBagConstraints constraints;

        // Create new layout.
        GridBagLayout aLayout = new GridBagLayout ();
        getContentPane().setLayout (aLayout);

        //  Accessible Tree.
        maTree = new AccessibilityTree ();
        maTree.setMinimumSize (new Dimension (250,300));
        //        maTree.setPreferredSize (new Dimension (300,500));
        JScrollPane aTreeScrollPane = new JScrollPane(maTree,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

        //  Canvas.
        maCanvas = new Canvas ();
        maCanvas.setTree (maTree);
        maTree.SetCanvas (maCanvas);
        maCanvas.setPreferredSize (new Dimension (1000,1000));

        // Split pane for tree view and canvas.
        JSplitPane aViewSplitPane = new JSplitPane (JSplitPane.HORIZONTAL_SPLIT,
                                                    aTreeScrollPane, maCanvas);
        aViewSplitPane.setOneTouchExpandable(true);
        aViewSplitPane.setDividerLocation (aTreeScrollPane.getPreferredSize().width);

        //  Text output area.
        maMessageArea = MessageArea.Instance ();
        maMessageArea.setMinimumSize (new Dimension (50,50));
        maMessageArea.setPreferredSize (new Dimension (300,100));

        // Split pane for the two views and the message area.
        JSplitPane aSplitPane = new JSplitPane (JSplitPane.VERTICAL_SPLIT,
            aViewSplitPane, maMessageArea);
        aSplitPane.setOneTouchExpandable(true);
        addGridElement (aSplitPane, 0,0, 2,1, 3,3,
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

        LoadOptions();

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

        aItem = new JMenuItem ("About");
        aHelpMenu.add (aItem);
        aItem.addActionListener (this);

        return maMenuBar;
    }




    protected void LoadOptions ()
    {
        try
        {
            File aOptionsFile = new File (
                System.getProperty ("user.home"),
                msOptionsFileName);
            StreamTokenizer aTokenizer = new StreamTokenizer (new FileReader (aOptionsFile));
            aTokenizer.eolIsSignificant (true);
            while (true)
            {
                if (aTokenizer.nextToken() != StreamTokenizer.TT_WORD)
                {
                    if (aTokenizer.ttype != StreamTokenizer.TT_EOF)
                        System.out.println ("unexpected token in options file: "
                            + aTokenizer.toString()
                            + " instead of option name");
                    break;
                }
                String sOptionName = aTokenizer.sval;

                if (aTokenizer.nextToken() != '=')
                {
                    System.out.println ("unexpected token in options file: "
                        + aTokenizer.toString()
                        + " instead of =");
                    break;
                }

                String sValue = null;
                Number nValue = null;
                switch (aTokenizer.nextToken())
                {
                    case StreamTokenizer.TT_WORD:
                        sValue = aTokenizer.sval;
                        break;
                    case StreamTokenizer.TT_NUMBER:
                        nValue = new Double (aTokenizer.nval);
                        break;
                }

                if (aTokenizer.nextToken() == StreamTokenizer.TT_EOF)
                    break;
                if (aTokenizer.ttype != StreamTokenizer.TT_EOL)
                {
                    System.out.println ("unexpected token in options file: " + aTokenizer.toString()
                        + " instead of newline");
                    break;
                }

                if (sOptionName.compareTo ("ShowDescriptions") == 0)
                    maCanvas.setShowDescriptions (sValue.compareTo ("true")==0);
                else if (sOptionName.compareTo ("ShowNames") == 0)
                    maCanvas.setShowNames (sValue.compareTo ("true")==0);
                else if (sOptionName.compareTo ("ShowText") == 0)
                    maCanvas.setShowText (sValue.compareTo ("true")==0);
                else if (sOptionName.compareTo ("Antialiasing") == 0)
                    maCanvas.setAntialiasing (sValue.compareTo ("true")==0);
                else
                    System.out.println ("option " + sOptionName + " unknown");

            }
        }
        catch (java.io.FileNotFoundException e)
            {}
        catch (Exception e)
        {
            System.out.println ("caught exception while loading options file : " + e);
        }
    }

    protected void SaveOptions ()
    {
        try
        {
            File aOptionsFile = new File (
                System.getProperty ("user.home"),
                msOptionsFileName);
            PrintWriter aOut = new PrintWriter (new FileWriter (aOptionsFile));
            aOut.println ("ShowDescriptions = " + maCanvas.getShowDescriptions());
            aOut.println ("ShowNames = " + maCanvas.getShowNames());
            aOut.println ("ShowText = " + maCanvas.getShowText());
            aOut.println ("Antialiasing = " + maCanvas.getAntialiasing());
            aOut.close();
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while writing options file : " + e);
        }
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
        maTree.setModel (aModel);


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
            AccessibilityTreeModel aModel = (AccessibilityTreeModel)maTree.getModel();
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
            maTree.expandShapes();
            setCursor (aCursor);
        }
        else if (e.getActionCommand().equals("expand"))
        {
            Cursor aCursor = getCursor();
            setCursor (new Cursor (Cursor.WAIT_CURSOR));
            maTree.expandAll();
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
            SaveOptions ();
        }
        else if (e.getActionCommand().equals ("Show Names"))
        {
            maCanvas.setShowNames ( ! maCanvas.getShowNames());
            SaveOptions ();
        }
        else if (e.getActionCommand().equals ("Antialiased Rendering"))
        {
            maCanvas.setAntialiasing ( ! maCanvas.getAntialiasing());
            SaveOptions ();
        }
        else if (e.getActionCommand().equals ("Help"))
        {
            HelpWindow.Instance().loadURL ("file://help.html");
        }
        else if (e.getActionCommand().equals ("About"))
        {
            HelpWindow.Instance().loadURL ("file://about.html");
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
        maTree;
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
