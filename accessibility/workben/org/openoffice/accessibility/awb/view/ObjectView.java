package org.openoffice.accessibility.awb.view;

import javax.swing.JPanel;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;

/** This is the base class for all object views that can be placed inside an
    object view container.

    <p>When provided with a new accessible object the container will call
    the Create method to create a new instance when certain conditions are
    met.  It then calls SetObject to pass the object to the instance.
    Finally it calls Update.</p>

    <p>The SetObject and Update methods may be called for a new object
    without calling Create first.  In this way an existing instance is
    recycled.</p>
*/
abstract public class ObjectView
    extends JPanel
{
    /** This factory method creates a new instance of the (derived) class
        when the given accessible object supports all necessary features.
        In the ususal case this will be the support of a specific
        accessibility interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        return null;
    }

    public ObjectView (ObjectViewContainer aContainer)
    {
        maContainer = aContainer;
        mxContext = null;
    }

    /** Call this when you want the object to be destroyed.  Release all
        resources when called.
    */
    public void Destroy ()
    {
    }

    /** Tell the view to display information for a new accessible object.
        @param xObject
            The given object may be null.  A typical behaviour in this case
            would be to display a blank area.  But is also possible to show
            information about the last object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        mxContext = xContext;
        Update ();
    }


    /** This is a request of a repaint with the current state of the current
        object.  The current object may or may not be the same as the one
        when Update() was called the last time.
    */
    public void Update ()
    {
    }


    /** Return a string that is used as a title of an enclosing frame.
    */
    abstract public String GetTitle ();


    public ObjectViewContainer GetContainer ()
    {
        return maContainer;
    }


    /** Implement this method if you are interested in accessible events.
    */
    public void notifyEvent (AccessibleEventObject aEvent)
    {}

    /// Reference to the current object to display information about.
    protected XAccessibleContext mxContext;

    protected ObjectViewContainer maContainer;
}
