package ov;

import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Shape;

import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;

import java.awt.geom.Rectangle2D;
import java.awt.geom.AffineTransform;


import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.Border;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;

import tools.NameProvider;

public class StateSetView
    extends ListeningObjectView
    implements MouseListener
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
        maStates = null;
        mnViewMode = SHOW_ALL_STATES;
        setPreferredSize (new Dimension(300,90));
        setMinimumSize (new Dimension(200,80));
        addMouseListener (this);
    }

    public void paintChildren (Graphics g)
    {
        synchronized (g)
        {
            super.paintChildren (g);

            // Calculcate the are inside the border.
            Insets aInsets = getInsets ();
            Dimension aSize = getSize();
            Rectangle aWidgetArea = new Rectangle (
                aInsets.left,
                aInsets.top,
                aSize.width-aInsets.left-aInsets.right,
                aSize.height-aInsets.top-aInsets.bottom);

            switch (mnViewMode)
            {
                case SHOW_ALL_STATES :
                    PaintAllStates ((Graphics2D)g, aWidgetArea);
                    break;
                case SHOW_SET_STATES :
                    PaintSetStates ((Graphics2D)g, aWidgetArea);
                    break;
            }
        }
    }

    private void SetViewMode (int nViewMode)
    {
        mnViewMode = nViewMode;
        switch (mnViewMode)
        {
            case SHOW_SET_STATES :
                maStates = new JLabel ();
                add (maStates, BorderLayout.CENTER);
                Update();
                break;
            case SHOW_ALL_STATES :
                if (maStates != null)
                {
                    remove (maStates);
                    maStates = null;
                }
                repaint();
                break;
        }
    }

    private void PaintSetStates (Graphics2D g, Rectangle aWidgetArea)
    {
    }

    private void PaintAllStates (Graphics2D g, Rectangle aWidgetArea)
    {
        Color aTextColor = g.getColor();

        g.setRenderingHint (
            RenderingHints.KEY_ANTIALIASING,
            RenderingHints.VALUE_ANTIALIAS_ON);

        XAccessibleStateSet xStateSet = mxContext.getAccessibleStateSet();
        if (xStateSet != null)
        {
            short aStates[] = xStateSet.getStates ();
            final int nMaxStateIndex = AccessibleStateType.MANAGES_DESCENDANTS;
            int nStateWidth = (aWidgetArea.width-12) / (nMaxStateIndex+1);
            AffineTransform aTransform = g.getTransform ();
            g.setColor (aTextColor);
            int y = aWidgetArea.y+aWidgetArea.height - 12;
            double nTextRotation = -0.9;//-java.lang.Math.PI/2;
            double nScale = 0.6;

            // Create a shape for the boxes.
            int nBoxWidth = nStateWidth-2;
            if (nBoxWidth > 8)
                nBoxWidth = 8;
            Rectangle aCheckBox = new Rectangle (-nBoxWidth/2,0,nBoxWidth,nBoxWidth);

            for (short i=0; i<=nMaxStateIndex; i++)
            {
                int x = nStateWidth + i * nStateWidth;
                String sStateName = NameProvider.getStateName (i);
                boolean bStateSet = xStateSet.contains (i);
                g.setTransform (aTransform);
                g.translate (x,y);
                if (bStateSet)
                {
                    g.setColor (Color.GREEN);
                    g.fill (aCheckBox);
                    g.setColor (aTextColor);
                }
                g.draw (aCheckBox);
                g.rotate (nTextRotation);
                g.scale (nScale, nScale);
                g.translate (2,-2);
                g.drawString (sStateName, 0,0);
            }
        }
    }


    synchronized public void Update ()
    {
        if (mnViewMode == SHOW_SET_STATES)
        {
            XAccessibleStateSet xStateSet = mxContext.getAccessibleStateSet();
            if (xStateSet != null)
            {
                String sStates = new String ();
                short aStates[] = xStateSet.getStates();
                for (int i=0; i<aStates.length; i++)
                {
                    if (i > 0)
                        sStates = sStates + ", ";
                    sStates = sStates + NameProvider.getStateName(aStates[i]);
                }
                maStates.setText (sStates);
            }
        }
    }

    public String GetTitle ()
    {
        return ("StateSet");
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        if (aEvent.EventId == AccessibleEventId.STATE_CHANGED)
            Update();
    }

    public void mouseClicked(MouseEvent e)
    {
        switch (mnViewMode)
        {
            case SHOW_SET_STATES :
                SetViewMode (SHOW_ALL_STATES);
                break;
            case SHOW_ALL_STATES :
                SetViewMode (SHOW_SET_STATES);
                break;
        }
    }
    public void mouseEntered (MouseEvent e) {}
    public void mouseExited (MouseEvent e) {}
    public void mousePressed (MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}

    private JLabel maStates;
    private int mnViewMode;
    private final int SHOW_SET_STATES = 0;
    private final int SHOW_ALL_STATES = 1;
}
