import java.awt.event.ActionListener;
import javax.swing.*;
import java.awt.*;
import java.util.*;

import com.sun.star.awt.XTopWindowListener;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.accessibility.*;
import com.sun.star.awt.XExtendedToolkit;

/** This class is used as a thread and registers or unregsiters a listener
    given the constructor at all nodes of a tree of accessibility objects.
*/
public class RegistrationThread
    implements Runnable
{
    /** Start a new thread that adds or removes the given listener at all
        accessible objects in the sub-tree rooted in the given accessible
        object.
        @param aListener
            The listener that is added or removed.
        @param xRoot
            The root of the sub-tree of accessibility objects.
        @param bRegister
            This flag decides whether to add or remove the listener.
    */
    public RegistrationThread (
        EventListenerProxy aListener,
        XAccessibleContext xRoot,
        boolean bRegister,
        boolean bShowMessages)
    {
        maListener = aListener;
        mxRoot = xRoot;
        mbRegister = bRegister;
        mbShowMessages = bShowMessages;

        if (mxRoot != null)
        {
            if (mbShowMessages)
                MessageArea.println ("starting to register at " + mxRoot.getAccessibleName());
            new Thread (this, "RegistrationThread").start();
        }
    }



    public void run ()
    {
        System.out.println ("starting registration");
        long nNodeCount = traverseTree (mxRoot);
        System.out.println ("ending registration");
        if (mbShowMessages)
        {
            if ( ! mbRegister)
                MessageArea.print ("un");
            MessageArea.println ("registered at " + nNodeCount
                + " objects in accessibility tree of " + mxRoot.getAccessibleName());
        }
    }




    /** Register this object as listener for accessibility events at all nodes
        of the given tree.
        @param xRoot
            The root node of the tree at which to register.
    */
    public long traverseTree (XAccessibleContext xRoot)
    {
        long nNodeCount = 0;
        if (xRoot != null)
        {
            // Register the root node.
            XAccessibleEventBroadcaster xBroadcaster =
                (XAccessibleEventBroadcaster) UnoRuntime.queryInterface (
                    XAccessibleEventBroadcaster.class,
                    xRoot);
            if (xBroadcaster != null)
            {
                if (mbRegister)
                    xBroadcaster.addEventListener (maListener);
                else
                    xBroadcaster.removeEventListener (maListener);
                nNodeCount += 1;
            }

            // Call this method recursively to register all sub-trees.
            try
            {
                int nChildCount = xRoot.getAccessibleChildCount();
                for (int i=0; i<nChildCount; i++)
                {
                    XAccessible xChild = xRoot.getAccessibleChild (i);
                    if (xChild != null)
                        nNodeCount += traverseTree (xChild.getAccessibleContext());
                }
            }
            catch (com.sun.star.lang.IndexOutOfBoundsException aException)
            {
                // The set of children has changed since our last call to
                // getAccesibleChildCount().  Don't try any further on this
                // sub-tree.
            }
            catch (com.sun.star.lang.DisposedException aException)
            {
                // The child has been destroyed since our last call to
                // getAccesibleChildCount().  That is OK. Don't try any
                // further on this sub-tree.
            }
        }
        return nNodeCount;
    }

    private EventListenerProxy maListener;
    private XAccessibleContext mxRoot;
    private boolean mbRegister;
    private boolean mbShowMessages;
}
