package ov;

import java.awt.Color;
import java.awt.Component;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.BorderFactory;
import javax.swing.border.Border;
import javax.swing.border.BevelBorder;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.uno.UnoRuntime;

public class ObjectViewContainer
    extends JPanel
{
    public ObjectViewContainer ()
    {
        maViewBorder = BorderFactory.createBevelBorder (BevelBorder.RAISED);
        setLayout (new GridBagLayout ());
    }



    /** Remove all existing views and create new ones according to the
        interfaces supported by the given object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        // Call Destroy at all views to give them a chance to release their
        // resources.
        int n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).Destroy();
        // Remove existing views.
        removeAll ();

        // Add new views.
        Add (ContextView.Create(xContext));
        Add (StateSetView.Create(xContext));
        Add (FocusView.Create(xContext));
        Add (SelectionView.Create(xContext));
        Add (TextView.Create(xContext));

        UpdateLayoutManager ();

        // Now set the object at all views.
        n = getComponentCount();
        for (int i=0; i<n; i++)
            ((ObjectView)getComponent(i)).SetObject (xContext);

        setPreferredSize (getLayout().preferredLayoutSize (this));
    }



    /** Add an object view and place it below all previously added views.
        @param aView
            This argument may be null.  In this case nothing happens.
    */
    public void Add (ObjectView aView)
    {
        if (aView != null)
        {
            GridBagConstraints constraints = new GridBagConstraints ();
            constraints.gridx = 0;
            constraints.gridy = getComponentCount();
            constraints.gridwidth = 1;
            constraints.gridheight = 1;
            constraints.weightx = 1;
            constraints.weighty = 0;
            constraints.ipadx = 2;
            constraints.ipady = 5;
            constraints.insets = new Insets (5,5,5,5);
            constraints.anchor = GridBagConstraints.NORTH;
            constraints.fill = GridBagConstraints.HORIZONTAL;

            aView.setBorder (
                BorderFactory.createTitledBorder (
                    maViewBorder, aView.GetTitle()));

            add (aView, constraints);
        }
    }

    /** Update the layout manager by setting the vertical weight of the
        bottom entry to 1 and so make it strech to over the available
        space.

    */
    private void UpdateLayoutManager ()
    {
        // Adapt the layout manager.
        Component aComponent = getComponent (getComponentCount()-1);
        GridBagLayout aLayout = (GridBagLayout)getLayout();
        GridBagConstraints aConstraints = aLayout.getConstraints (aComponent);
        aConstraints.weighty = 1;
        aLayout.setConstraints (aComponent, aConstraints);
    }

    /// Observe this tree for selection changes and notify them to all
    /// children.
    private JTree maTree;
    private Border maViewBorder;
}
