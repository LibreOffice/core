package ov;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.geom.Rectangle2D;

import java.awt.BorderLayout;

import javax.swing.JPanel;

import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.AccessibleEventObject;

public class StateSetView
    extends ListeningObjectView
{
    /** Create a FocusView when the given object supports the
        XAccessibleComponent interface.
    */
    static public ObjectView Create (XAccessibleContext xContext)
    {
        if (xContext != null)
            return new StateSetView();
        else
            return null;
    }

    public StateSetView ()
    {
        mxCanvas = new JPanel();
        add (mxCanvas, BorderLayout.CENTER);
    }

    public void paintChildren (Graphics g)
    {
        synchronized (g)
        {
            System.out.println ("Paint");

            Dimension aSize = mxCanvas.getSize();

            Graphics2D g2 = (Graphics2D)g;
            Shape aWidgetArea = g.getClip();

            g2.setColor (new Color (250,240,230));
            g2.fill (aWidgetArea);

            super.paintChildren (g);
        }
    }

    synchronized public void Update ()
    {
    }

    public String GetTitle ()
    {
        return ("StateSet");
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        System.out.println (aEvent);
    }

    private JPanel mxCanvas;
}
