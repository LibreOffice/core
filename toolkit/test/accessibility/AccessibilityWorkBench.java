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

public class AccessibilityWorkBench
    extends JFrame
    implements ActionListener,
        Print,
        MessageInterface,
        XTerminateListener
{
    public static final String msVersion = "v1.5";
    public String msFileName;
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

        aWorkBench = new AccessibilityWorkBench (nPortNumber, sFileName);
    }


    /** the Singleton Workbench object */
    private static AccessibilityWorkBench aWorkBench;

    /** access to the Singleton Workbench object */
    public static AccessibilityWorkBench get()
    {
        return aWorkBench;
    }


    public  AccessibilityWorkBench (int nPortNumber, String sFileName)
    {
        mbInitialized = false;

        msFileName = sFileName;

        Layout ();

        println (System.getProperty ("os.name") + " / "
            + System.getProperty ("os.arch") + " / "
            + System.getProperty ("os.version"));
        println ("Using port " + nPortNumber + " and document file name " + msFileName);
        office = new SimpleOffice (this, nPortNumber);
        info = new InformationWriter (this);

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
        maTree = new AccessibilityTree ((MessageInterface)this, (Print)this);
        aScrollPane = new JScrollPane(maTree,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 2;
        constraints.weightx = 2;
        constraints.weighty = 1;
        constraints.fill = GridBagConstraints.BOTH;
        getContentPane().add (aScrollPane, constraints);

        //  Canvas.
        maCanvas = new Canvas (this);
        maCanvas.setTree (maTree);
        maTree.SetCanvas (maCanvas);
        maCanvas.setPreferredSize (new Dimension (1000,800));
        aScrollPane = new JScrollPane(maCanvas,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS/*AS_NEEDED*/,
            JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS/*AS_NEEDED*/);
        constraints = new GridBagConstraints ();
        constraints.gridx = 1;
        constraints.gridy = 0;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = 3;
        constraints.weighty = 3;
        constraints.fill = GridBagConstraints.BOTH;
        getContentPane().add (aScrollPane, constraints);

        //  Text output area.
        maOutputArea = new JTextArea (5,50);
        maScrollPane = new JScrollPane(maOutputArea,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        constraints = new GridBagConstraints ();
        constraints.gridx = 1;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = 3;
        constraints.weighty = 1;
        constraints.fill = GridBagConstraints.BOTH;
        getContentPane().add (maScrollPane, constraints);

        //  Message output area.
        maMessageArea = new JTextArea (5,20);
        constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 2;
        constraints.gridwidth = 2;
        constraints.gridheight = 1;
        constraints.weightx = 3;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.HORIZONTAL;
        getContentPane().add (maMessageArea, constraints);

        // Button bar.
        maButtonBar = new JPanel();
        GridBagLayout aButtonLayout = new GridBagLayout ();
        maButtonBar.setLayout (new FlowLayout());
        constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 3;
        constraints.gridwidth = 2;
        constraints.weightx = 1;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.fill = GridBagConstraints.BOTH;
        getContentPane().add (maButtonBar, constraints);

        //  Buttons.
        aConnectButton = createButton ("Connect", "connect");
        aLoadButton = createButton ("Load", "load");
        aUpdateButton = createButton ("Update", "update");
        aShapesButton = createButton ("Expand Shapes", "shapes");
        aExpandButton = createButton ("Expand All", "expand");
        aTextButton = createButton("Text", "text");
        aQuitButton = createButton ("Quit", "quit");
        UpdateButtonStates ();

        LoadOptions();

        // Menu bar.
        maMenuBar = new MenuBar ();
        setMenuBar (maMenuBar);

        // File menu.
        Menu aFileMenu = new Menu ("File");
        maMenuBar.add (aFileMenu);
        MenuItem aItem;
        aItem = new MenuItem ("Quit");
        aFileMenu.add (aItem);
        aItem.addActionListener (this);

        // Options menu.
        Menu aOptionsMenu = new Menu ("Options");
        maMenuBar.add (aOptionsMenu);
        CheckboxMenuItem aCBItem;
        aCBItem = new CheckboxMenuItem ("Show Descriptions", maCanvas.getShowDescriptions());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new CheckboxMenuItem ("Show Names", maCanvas.getShowNames());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        aCBItem = new CheckboxMenuItem ("Antialiased Rendering", maCanvas.getAntialiasing());
        aOptionsMenu.add (aCBItem);
        aCBItem.addActionListener (this);

        setTitle("Accessibility Workbench " + msVersion);

        pack ();
        setVisible (true);
        repaint();
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
                        System.out.println ("unexpected token in options file: " + aTokenizer.toString()
                            + " instead of option name");
                    break;
                }
                String sOptionName = aTokenizer.sval;

                if (aTokenizer.nextToken() != '=')
                {
                    System.out.println ("unexpected token in options file: " + aTokenizer.toString()
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

                System.out.print ("option value " + sOptionName + " is set to ");
                if (sValue != null)
                    System.out.println ("string " + sValue);
                else if (nValue != null)
                    System.out.println ("number " + nValue);
                else
                    System.out.println ("nothing");

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
                else if (sOptionName.compareTo ("Antialiasing") == 0)
                    maCanvas.setAntialiasing (sValue.compareTo ("true")==0);
                else
                    System.out.println ("option " + sOptionName + " unknown");

            }
        }
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
        System.out.println ("creating new tree model");
        aModel = new AccessibilityTreeModel (createTreeModelRoot(), this, this);
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
                xToolkit.removeTopWindowListener (maTopWindowListener);
            // Add top window listener.
            if (xToolkit != null)
            {
                println ("registering at extended toolkit");
                maTopWindowListener = new TopWindowListener (aModel, office);
                xToolkit.addTopWindowListener (maTopWindowListener);
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
        aLoadButton.setEnabled (mbInitialized);
        aUpdateButton.setEnabled (mbInitialized);
        aExpandButton.setEnabled (mbInitialized);
        aShapesButton.setEnabled (mbInitialized);
        aTextButton.setEnabled (mbInitialized);
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
        else if (e.getActionCommand().equals("load"))
        {
            print ("Loading file " + msFileName);
            mxModel = office.loadDocument (msFileName);
            if (mxModel == null)
                println (": could not be loaded");
            else
            {
                println (".");
                initialize();
            }
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
        else if (e.getActionCommand().equals("text"))
        {
            Canvas.bPaintText = ! Canvas.bPaintText;
            maCanvas.repaint ();
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
        /*
        try
        {
            XDesktop xDesktop = office.getDesktop();
            if (xDesktop == null)
            {
                return new StringNode ("ERROR: Can't connect. (No desktop)", null);
            }

            XEnumerationAccess xEA = xDesktop.getComponents();
            if (xEA == null)
            {
                return new StringNode ("ERROR: Can't get components", null);
            }
            XEnumeration xE = xEA.createEnumeration();
            while (xE.hasMoreElements())
            {
                XComponent xComponent = (XComponent) UnoRuntime.queryInterface(
                    XComponent.class, xE.nextElement());
                XModel xModel = (XModel) UnoRuntime.queryInterface(
                    XModel.class, xComponent);
                if (xModel != null)
                {
                    println (xModel.getURL());
                    XWindow xWindow = office.getCurrentWindow (xModel);
                    XAccessible xRoot = office.getAccessibleRoot (
                        office.getAccessibleObject(xWindow));

                    // create document node
                    aRoot.addChild (
                        AccessibilityTreeModel.createDefaultNode (xRoot, aRoot));
                }
            }
            println ("finished getting named documents");
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while getting document names: " + e);
            e.printStackTrace();
        }
        */
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


    /** Write message into message area.
    */
    public void message (String message)
    {
        msMessage = message;
        maMessageArea.setText (msMessage);
        //        System.out.println (message);

        // Show the new message string immediately.
        maMessageArea.paintImmediately (maMessageArea.getVisibleRect());
    }




    public void message_append (String message)
    {
        msMessage += message;
        message (msMessage);
    }




    public  void print (String text)
    {
        maOutputArea.append (text);
        maOutputArea.paintImmediately (maOutputArea.getVisibleRect());
    }




    public  void println (String text)
    {
        maOutputArea.append (text + "\n");
        JScrollBar aBar = maScrollPane.getVerticalScrollBar();
        aBar.setValue (aBar.getMaximum());
        maOutputArea.paintImmediately (maOutputArea.getVisibleRect());
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
    private JTextArea
        maOutputArea,
        maMessageArea;
    private JButton
        aConnectButton,
        aQuitButton,
        aLoadButton,
        aUpdateButton,
        aExpandButton,
        aShapesButton,
        aTextButton;
    private MenuBar
        maMenuBar;
    private String
        msMessage;
    private boolean
        mbInitialized;
    private TopWindowListener
        maTopWindowListener;
}
