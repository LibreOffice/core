package org.openoffice.accessibility.awb.view;

import java.awt.BorderLayout;
import javax.swing.JFrame;
import com.sun.star.accessibility.XAccessibleContext;


/** Top level window that creates a single object view container.  This
    container shows information about a specific accessible object and is
    not affected by the selection of the accessbility tree widget.
*/
public class ObjectViewContainerWindow
    extends JFrame
{
    public ObjectViewContainerWindow (XAccessibleContext xContext)
    {
        setSize (new java.awt.Dimension (300,600));

        maContainer = new ObjectViewContainer ();
        maContainer.SetObject (xContext);
        getContentPane().add (maContainer, BorderLayout.CENTER);

        pack ();
        setVisible (true);
    }

    /** Set the object that is displayed in this window.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        maContainer.SetObject (xContext);
    }

    private ObjectViewContainer maContainer;
}
