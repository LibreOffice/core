import java.awt.Color;
import java.awt.Font;
import java.awt.Dimension;
import javax.swing.JTextArea;
import javax.swing.JScrollPane;
import java.util.Vector;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleStateSet;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

/** Display textual information for a given accessible object.  This
    includes the names of that object of its ancestors as well as some
    information retrieved from the XAccessibleContext and
    XAccessibleComponent interfaces.
*/
class TextualDisplay
    extends JScrollPane
    implements IAccessibleObjectDisplay
{
    /** Create a new scroll pane that contains a text widget which display
        information about given accessible objects.
    */
    public TextualDisplay ()
    {
        // Create a text widget for displaying the text information...
        maText = new JTextArea (80,10);
        maText.setBackground (new Color (250,240,230));
        maText.setFont (new Font ("Courier", Font.PLAIN, 11));

        // ...and set-up the scroll pane to show this widget.
        setViewportView (maText);
        setVerticalScrollBarPolicy (JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        setHorizontalScrollBarPolicy (JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    }




    /** Set the accessible object to display.  Call this method e.g. when a
        new object has been focused.
    */
    public synchronized void setAccessibleObject (XAccessibleContext xContext)
    {
        // First clear the display area.
        msTextContent = new String ();

        if (xContext != null)
        {
            String sIndentation = showParents (xContext);
            showContextInfo (xContext, sIndentation);
            showComponentInfo (xContext, sIndentation);
        }

        maText.setText (msTextContent);
    }




    public synchronized void updateAccessibleObject (XAccessibleContext xContext)
    {
        setAccessibleObject (xContext);
    }



    /** Show some of the information available over the given object's
        XAccessibleContext interface.
    */
    private void showContextInfo (XAccessibleContext xContext, String sIndentation)
    {
        // Show the description.
        msTextContent += sIndentation + "Description: "
            + xContext.getAccessibleDescription() + "\n";

        showStates (xContext, sIndentation);
    }




    /** Show a list of all of the the given object's states.  Use the
        NameConverter class to transform the numerical state ids into human
        readable names.
        @param xContext
            The accessible context for which to show the state names.
    */
    private void showStates (XAccessibleContext xContext, String sIndentation)
    {
        // Get the state set object...
        XAccessibleStateSet xStateSet = xContext.getAccessibleStateSet();
        // ...and retrieve an array of numerical ids.
        short aStates[] = xStateSet.getStates();

        // Iterate over the array and print the names of the states.
        msTextContent += sIndentation + "States     : ";
        for (int i=0; i<aStates.length; i++)
        {
            if (i > 0)
                msTextContent += ", ";
            msTextContent += NameProvider.getStateName(aStates[i]);
        }
        msTextContent += "\n";
    }




    /** When the given object supports the XAccessibleComponent interface then
        show its size and location on the screen.
    */
    private void showComponentInfo (XAccessibleContext xContext, String sIndentation)
    {
        // Try to cast the given accessible context to the
        // XAccessibleComponent interface.
        XAccessibleComponent xComponent =
            (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, xContext);
        if (xComponent != null)
        {
            Point aLocation = xComponent.getLocationOnScreen();
            msTextContent += sIndentation + "Position   : "
                + aLocation.X + ", " + aLocation.Y + "\n";

            Size aSize = xComponent.getSize();
            msTextContent += sIndentation + "Size       : "
                + aSize.Width + ", " + aSize.Height + "\n";
        }
    }





    /** Print the names of the given object and its parents and return an
        indentation string that can be used to print further information
        about the object.
    */
    private String showParents (XAccessibleContext xContext)
    {
        // Create the path from the given object to its tree's root.
        Vector aPathToRoot = new Vector();
        while (xContext != null)
        {
            aPathToRoot.add (xContext);
            // Go up the hierarchy one level to the object's parent.
            try
            {
                XAccessible xParent = xContext.getAccessibleParent();
                if (xParent != null)
                    xContext = xParent.getAccessibleContext();
                else
                    xContext = null;
            }
            catch (Exception e)
            {
                System.err.println ("caught exception " + e + " while getting path to root");
            }
        }

        // Print the path in the accessibility tree from the given context to
        // the root.
        String sIndentation = new String ();
        for (int i=aPathToRoot.size()-1; i>=0; i--)
        {
            XAccessibleContext xParentContext = (XAccessibleContext)aPathToRoot.get(i);
            String sParentName = xParentContext.getAccessibleName();
            if (sParentName.length() == 0)
                sParentName = "<unnamed> / Role "
                    + NameProvider.getRoleName(xParentContext.getAccessibleRole());
            msTextContent += sIndentation + sParentName + "\n";
            sIndentation += msIndentation;
        }

        return sIndentation;
    }



    /// The text widget that is used for the actual text display.
    private JTextArea maText;

    /// The indentation with which an object's child is indented.
    private final String msIndentation = new String("  ");

    /// The text content displayed by this object.
    private String msTextContent = new String ();
}
