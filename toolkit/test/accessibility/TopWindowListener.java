import com.sun.star.awt.XTopWindowListener;
import com.sun.star.awt.XWindow;
import drafts.com.sun.star.awt.XExtendedToolkit;
import drafts.com.sun.star.accessibility.XAccessible;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

/** Listen for top window events and create or delete children of the tree
    model accordingly.
*/
class TopWindowListener
    implements XTopWindowListener
{
    TopWindowListener (AccessibilityTreeModel aModel, SimpleOffice aOffice)
    {
        maModel = aModel;
        maOffice = aOffice;
    }

    // XTopWindowListener
    public void windowOpened (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window opened: " + aEvent.Source);
        if (maModel != null)
        {
            Object aObject = maModel.getRoot();
            if (aObject instanceof VectorNode)
            {
                VectorNode aRoot = (VectorNode) aObject;
                InformationWriter aIW = new InformationWriter();
                aIW.showInterfaces ((XInterface)aEvent.Source);
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
                    {
                        XAccessible xRoot = maOffice.getAccessibleRoot (xAccessible);
                        aRoot.addChild (
                            AccessibilityTreeModel.createDefaultNode (xRoot, aRoot));
                    }
                }
            }
        }
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
    public void windowClosing (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window closing: " + aEvent);
    }
    public void windowClosed (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window closed: " + aEvent);
    }
    public void windowMinimized (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window minimized: " + aEvent);
    }
    public void windowNormalized (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window normalized: " + aEvent);
    }
    public void windowActivated (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window actived: " + aEvent);
    }
    public void windowDeactivated (final com.sun.star.lang.EventObject aEvent) throws RuntimeException
    {
        System.out.println ("Top window deactived: " + aEvent);
    }

    // XEventListener
    public void disposing (com.sun.star.lang.EventObject aEvent)
    {
        System.out.println ("broadcaster disposed");
    }

    private AccessibilityTreeModel
        maModel;
    private SimpleOffice
        maOffice;
}
