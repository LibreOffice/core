import SimpleOffice;
import InformationWriter;
import Print;
import Canvas;
import AccessibilityTree;

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
import com.sun.star.uno.UnoRuntime;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;
import drafts.com.sun.star.accessibility.XAccessibleExtendedComponent;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;


import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;


public class AccessibilityWorkBench
    extends JFrame
    implements ActionListener,
        Print,
        MessageInterface,
        XEventListener,
        XFrameActionListener,
        XPropertyChangeListener
{
    public String msFileName;
    /*WinFilename
        //        = "file:///d|/tmp/impress-test-document.sxi";
        = "file:///d|/tmp/writer-test-document.sxw";
        //        = "file:///d|/tmp/calc-test-document.sxc";
    final public String sUnxFilename
        = "file:///tmp/impress-test-document.sxi";
        //= "file:///tmp/draw-test-document.sxd";
    */

    public static void main (String args[])
    {
        int nPortNumber = 5678;
        String sFileName = "file:///tmp/impress-test-document.sxi";

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

        new AccessibilityWorkBench (nPortNumber, sFileName);
    }




    public  AccessibilityWorkBench (int nPortNumber, String sFileName)
    {
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
        setSize (new Dimension (1024,768));
        maMainPanel = new JPanel();
        this.getContentPane().add (maMainPanel);
        GridBagLayout aLayout = new GridBagLayout ();
        JScrollPane aScrollPane;

        //  Text output area.
        maOutputArea = new JTextArea (5,50);
        maScrollPane = new JScrollPane(maOutputArea,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = 1;
        constraints.gridy = 1;
        constraints.gridwidth = 1;
        constraints.gridheight = 1;
        constraints.weightx = 3;
        constraints.weighty = 1;
        constraints.fill = GridBagConstraints.BOTH;
        aLayout.addLayoutComponent (maScrollPane, constraints);
        maMainPanel.add (maScrollPane);

        //  Message output area.
        maMessageArea = new JTextArea (5,20);
        constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 2;
        constraints.gridwidth = 2;
        constraints.gridheight = 1;
        constraints.weightx = 3;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.BOTH;
        aLayout.addLayoutComponent (maMessageArea, constraints);
        maMainPanel.add (maMessageArea);

        //  Canvas.
        maCanvas = new Canvas (this, maTree);
        maCanvas.setPreferredSize (new Dimension (1050,1050));
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
        aLayout.addLayoutComponent (aScrollPane, constraints);
        maMainPanel.add (aScrollPane);

        //  Accessible Tree.
        maTree = new AccessibilityTree (this, maCanvas);
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
        aLayout.addLayoutComponent (aScrollPane, constraints);
        maMainPanel.add (aScrollPane);

        // Button bar.
        maButtonBar = new JPanel();
        GridBagLayout aButtonLayout = new GridBagLayout ();
        maButtonBar.setLayout (aLayout);
        constraints = new GridBagConstraints ();
        constraints.gridx = 0;
        constraints.gridy = 3;
        constraints.gridwidth = 3;
        constraints.weightx = 1;
        constraints.anchor = GridBagConstraints.WEST;
        constraints.fill = GridBagConstraints.BOTH;
        aLayout.setConstraints (maButtonBar, constraints);
        maMainPanel.add (maButtonBar);

        //  Buttons.
        aConnectButton = createButton ("Connect", "connect");
        aLoadButton = createButton ("Load", "load");
        aRunButton = createButton ("Run", "run");
        aUpdateButton = createButton ("Update", "update");
        aQuitButton = createButton ("Quit", "quit");

        maMainPanel.setLayout (aLayout);
        getContentPane().add ("Center", maMainPanel);
        maMainPanel.setVisible (true);
        setVisible (true);
        setTitle("Accessibility Workbench");
    }



    /** Create a new button and place at the right most position into the
        button bar.
    */
    public JButton createButton (String title, String command)
    {
        JButton aButton = new JButton (title);
        aButton.setActionCommand (command);
        aButton.addActionListener (this);
        GridBagConstraints constraints = new GridBagConstraints ();
        constraints.gridx = maButtonBar.getComponentCount();
        constraints.gridy = 0;
        GridBagLayout aLayout = (GridBagLayout)maButtonBar.getLayout();

        aLayout.setConstraints (aButton, constraints);
        maButtonBar.add (aButton);
        return aButton;
    }


    protected void initialize ()
    {
        // Clear the accessibility tree.
        maTree.clear ();

        // Delete the graphical representations.
        maCanvas.clear ();

        // Add entries for open documents to tree.
        addOpenDocumentsToTree ();
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
                XWindow xWindow = office.getCurrentWindow (mxModel);
                XAccessible xRoot = office.getAccessibleRoot (xWindow);
                maTree.addDocument (xRoot, msFileName);
            }
        }
        else if (e.getActionCommand().equals("run"))
        {
            run (mxModel);
        }
        else if (e.getActionCommand().equals("update"))
        {
            initialize ();
            run (mxModel);
        }
        else
        {
            System.err.println("unknown command " + e.getActionCommand());
        }
    }




    public void test (XModel xModel)
    {
        println ("Test:");
        try
        {
            if (xModel == null)
            {
                XDrawView xView = office.getCurrentView ();
                if (xView == null)
                    println ("no current view");
                else
                    xModel = office.getModel (xView);
            }
            info.showProperties (xModel);

            XWindow xWindow = office.getCurrentWindow (xModel);
            if (xWindow != null)
            {
                println ("current window:");
                com.sun.star.awt.Rectangle aRectangle = xWindow.getPosSize();
                println (aRectangle.X + " " + aRectangle.Y + " "
                    + aRectangle.Width + " " + aRectangle.Height);
            }
            else
                println ("Can't get window");

            XPropertySet xSet = (XPropertySet) UnoRuntime.queryInterface (
                XPropertySet.class, xModel);
            if (xSet != null)
            {
                com.sun.star.awt.Rectangle aRectangle =
                    (com.sun.star.awt.Rectangle) xSet.getPropertyValue (
                        "VisibleArea");
                println (aRectangle.X + " " + aRectangle.Y + " "
                    + aRectangle.Width + " " + aRectangle.Height);
            }
            else
                println ("model does not support XPropertySet");
        }
        catch (Exception e)
        {
            System.out.println ("caught exception in test: " + e);
        }
        println ("Test finished.");
    }




    /** Get the accessibility tree for the specified model.
    */
    protected  void run (XModel xModel)
    {
        try
        {
            addListeners (xModel);
            XWindow xWindow = office.getCurrentWindow (xModel);
            XAccessible xRoot = office.getAccessibleRoot (xWindow);
            if (xRoot != null)
            {
                println ("window is accessible");
            }
            else
            {
                println ("window is not accessible.");
                return;
            }

            message ("creating accessibility tree");
            maTree.createTree (xRoot);
            maTree.expandShapes ();
        }
        catch (Exception e)
        {
            System.out.println ("caught exception in run: " + e);
        }
    }





    /** Experimental. Add list of currently open (and named) documents to
        the tree widget.
    */
    public void addOpenDocumentsToTree ()
    {
        try
        {
            XDesktop xDesktop = office.getDesktop();
            if (xDesktop == null)
            {
                println ("can't get desktop to retrieve open documents");
                return;
            }

            XEnumerationAccess xEA = xDesktop.getComponents();
            if (xEA == null)
            {
                println ("Can't get list of components from desktop");
                return;
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
                    XAccessible xRoot = office.getAccessibleRoot (xWindow);
                    maTree.addDocument (xRoot, xModel.getURL());
                }
                else
                    println ("can't cast component to model");
            }
            println ("finished getting named documents");
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while getting document names: " + e);
        }
    }




    /** Add various listeners to the model and other Office objects.
    */
    protected void addListeners (XModel xModel)
    {

        com.sun.star.document.XEventBroadcaster xBr =
            (com.sun.star.document.XEventBroadcaster)UnoRuntime.queryInterface(
                com.sun.star.document.XEventBroadcaster.class, xModel);
        if( xBr != null )
            xBr.addEventListener (this);

        XController xController = xModel.getCurrentController();
        if (xController != null)
        {
            XFrame xFrame = xController.getFrame();
            if (xFrame != null)
            {
                xFrame.addFrameActionListener( this );
                System.out.println("[DONE]");
            }
            connectListener( xController );
        }
    }



    public boolean connectListener( XController xController )
    {
        System.out.println("connecting to controller...");

        XServiceInfo oObj = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, xController);
        String[] names = oObj.getSupportedServiceNames();
        for (int i=0;i<names.length;i++)
        {
           System.out.println("Supported Service is "+names[i]);
        }

        System.out.print("add property listener             ");

        XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xController);

        try
        {

            xPropSet.addPropertyChangeListener ("", this);

            System.out.println("[DONE]");

        }
        catch (Exception exp)
        {
            System.out.println("[FAILED]");
            System.exit(0);
        }

        System.out.print("add dispose listener             ");

        XComponent xComponent = (XComponent)UnoRuntime.queryInterface(XComponent.class, xController);
        //      com.sun.star.lang.XEventListener xEventListener = (com.sun.star.lang.XEventListener)UnoRuntime.queryInterface(com.sun.star.lang.XEventListener.class, this);

        xComponent.addEventListener (this);

        System.out.println("[DONE]");

        return true;
    }

    public void propertyChange( final PropertyChangeEvent evt ) throws RuntimeException
    {
        System.out.println("property " + evt.PropertyName + "(" + evt.PropertyHandle + ") changed!");

        if( evt.PropertyHandle == 3 )
        {
            System.out.print("old value: ");
            com.sun.star.awt.Rectangle aRect;
            aRect = (com.sun.star.awt.Rectangle)evt.OldValue;
            System.out.println( "old value: (" + aRect.X + "," + aRect.Y + "," + aRect.Width + "," + aRect.Height + ")" );

            aRect = (com.sun.star.awt.Rectangle)evt.NewValue;
            System.out.println( "new value: (" + aRect.X + "," + aRect.Y + "," + aRect.Width + "," + aRect.Height + ")" );
        }
    }

    public void frameAction( final FrameActionEvent aEvent ) throws RuntimeException
    {
        System.out.println("frame action event " + aEvent.Action );

        if( aEvent.Action == FrameAction.COMPONENT_REATTACHED )
        {
            System.out.println("FrameAction.COMPONENT_REATTACHED");
            XController xController = aEvent.Frame.getController();
            connectListener( xController );
        }
    }


    // XEventListener
    public void disposing( com.sun.star.lang.EventObject aSourceObj )
    {
        XFrame xFrame = (XFrame)UnoRuntime.queryInterface(XFrame.class, aSourceObj.Source);

        if( xFrame != null )
        {
            System.out.println("frame disposed");
            System.exit(0);
        }
        else
        {
            System.out.println("controller disposed");

        }
    }

    // XEventListener
    public void notifyEvent (final com.sun.star.document.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Event : " + aEvent.EventName + " at " + aEvent.Source);
    }


    /** Write message into message area.
    */
    public void message (String message)
    {
        maMessageArea.setText (message);
        System.out.println (message);

        // Show the new message string immediately.
        maMessageArea.paintImmediately (maMessageArea.getVisibleRect());
    }




    public  void print (String text)
    {
        maOutputArea.append (text);
    }




    public  void println (String text)
    {
        maOutputArea.append (text + "\n");
        JScrollBar aBar = maScrollPane.getVerticalScrollBar();
        aBar.setValue (aBar.getMaximum());
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
        aRunButton;
}
