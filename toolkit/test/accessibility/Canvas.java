import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.TreeSelectionListener;
import javax.swing.event.TreeSelectionEvent;

import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;

/** This canvas displays accessible objects graphically.  Each accessible
    object with graphical representation is represented by an
    AccessibleObject object and has to be added by the
    <member>addAccessible</member> member function.

    <p>The canvas listens to selection events of the associated JTree and
    highlights the first selected node of that tree.</p>
*/
class Canvas
    extends JPanel
    implements MouseListener, MouseMotionListener, TreeSelectionListener, Scrollable
{
    public MessageInterface maMessageDisplay;
    public final int nMaximumWidth = 800;
    public final int nMaximumHeight = 800;

    public static boolean bPaintText = false;

    public Canvas ()
    {
        super (true);
        maObjects = new java.util.HashMap ();
        maNodes = new Vector ();
        maObjectList = new Vector ();
        maContexts = new Vector ();
        addMouseListener (this);
        addMouseMotionListener (this);
        maBoundingBox = new Rectangle (0,0,100,100);
        //        setPreferredSize (new Dimension (nMaximumWidth,nMaximumHeight));
        //        setSize (nMaximumWidth,nMaximumHeight);
        maTree = null;
        mnHOffset = 0;
        mnVOffset = 0;
        mnScale = 1;
    }

    public void setTree (JTree aTree)
    {
        if (maTree != null)
            maTree.removeTreeSelectionListener (this);
        maTree = aTree;
        if (maTree != null)
            maTree.addTreeSelectionListener (this);
    }

    public void addNode (AccTreeNode aNode)
    {
        if (maNodes.indexOf (aNode) == -1)
        {
            maNodes.add (aNode);

            AccessibleObject aObject = (AccessibleObject) maObjects.get (aNode);
            if (aObject == null)
            {
                aObject = new AccessibleObject (aNode);
                // Update bounding box that includes all objects.
                if (maObjects.size() == 0)
                    maBoundingBox = aObject.getBBox();
                else
                    maBoundingBox = maBoundingBox.union (aObject.getBBox());

                maObjects.put (aNode, aObject);
                maObjectList.add (aObject);

            }
            repaint ();
        }
    }

    public void removeNode (AccTreeNode aNode)
    {
        int i = maNodes.indexOf (aNode);
        if( i != -1 )
        {
            Object aObject = maObjects.get(aNode);
            maObjectList.remove (aObject);
            maObjects.remove (aObject);
            maNodes.remove (aNode);
            repaint ();
        }
    }

    public void updateNode (AccTreeNode aNode)
    {
        int i = maNodes.indexOf (aNode);
        System.out.println ("updating node " + i);
        if (i != -1)
            ((AccessibleObject)maObjects.get(aNode)).update();
    }

    public void clear ()
    {
        while (maNodes.size() > 0)
            removeNode ((AccTreeNode)maNodes.elementAt(0));

        maNodes.clear();
        maObjects.clear();
        maObjectList.clear();
    }

    public boolean getShowDescriptions ()
    {
        return mbShowDescriptions;
    }

    public void setShowDescriptions (boolean bNewValue)
    {
        mbShowDescriptions = bNewValue;
        repaint ();
    }

    public boolean getShowNames ()
    {
        return mbShowNames;
    }

    public void setShowNames (boolean bNewValue)
    {
        mbShowNames = bNewValue;
        repaint ();
    }

    public boolean getAntialiasing ()
    {
        return mbAntialiasing;
    }

    public void setAntialiasing (boolean bNewValue)
    {
        mbAntialiasing = bNewValue;
        repaint ();
    }

    public void paintComponent (Graphics g)
    {
        super.paintComponent (g);

        Graphics2D g2 = (Graphics2D)g;
        if (mbAntialiasing)
            g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);
        else
            g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_OFF);

        setupTransformation ();

        // Draw the screen representation to give a hint of the location of the
        // accessible object on the screen.
        Dimension aScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
        // Fill the screen rectangle.
        g.setColor (new Color (250,240,230));
        g.fillRect (
            (int)(mnHOffset+0.5),
            (int)(mnVOffset+0.5),
            (int)(mnScale*aScreenSize.getWidth()),
            (int)(mnScale*aScreenSize.getHeight()));
        // Draw a frame arround the screen rectangle to increase its visibility.
        g.setColor (Color.BLACK);
        g.drawRect (
            (int)(mnHOffset+0.5),
            (int)(mnVOffset+0.5),
            (int)(mnScale*aScreenSize.getWidth()),
            (int)(mnScale*aScreenSize.getHeight()));

        synchronized (maObjectList)
        {
            int nCount = maObjectList.size();
            for (int i=0; i<nCount; i++)
            {
                AccessibleObject aAccessibleObject = (AccessibleObject)maObjectList.elementAt(i);
                aAccessibleObject.paint (
                    g2,
                    mnHOffset, mnVOffset, mnScale,
                    mbShowDescriptions, mbShowNames);
            }
        }

        // Paint highlighted frame around active object as the last thing.
        if (maActiveObject != null)
            maActiveObject.paint_highlight (
                g,
                mnHOffset, mnVOffset, mnScale);
    }

    /** Set up the transformation so that the graphical display can show a
        centered representation of the whole screen.
    */
    private void setupTransformation ()
    {
        Dimension aScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension aWidgetSize = getSize();
        if ((aScreenSize.getWidth() > 0) && (aScreenSize.getHeight() > 0))
        {
            // Calculate the scales that would map the screen onto the
            // widget in both of the coordinate axes and select the smaller
            // of the two: it maps the screen onto the widget in both axes
            // at the same time.
            double nHScale = (aWidgetSize.getWidth() - 10) / aScreenSize.getWidth();
            double nVScale = (aWidgetSize.getHeight() - 10) / aScreenSize.getHeight();
            if (nHScale < nVScale)
                mnScale = nHScale;
            else
                mnScale = nVScale;

            // Calculate offsets that center the scaled screen inside the widget.
            mnHOffset = (aWidgetSize.getWidth() - mnScale*aScreenSize.getWidth()) / 2.0;
            mnVOffset = (aWidgetSize.getHeight() - mnScale*aScreenSize.getHeight()) / 2.0;
        }
        else
        {
            // In case of a degenerate (not yet initialized?) screen size
            // use some meaningless default values.
            mnScale = 1;
            mnHOffset = 0;
            mnVOffset = 0;
        }
    }



    /**  Call getAccessibleAt to determine accessible object under mouse.
    */
    public void mouseClicked (MouseEvent e)
    {
        /*        FindAccessibleObjectUnderMouse (e);
        // Because we have no access (at the moment) to the root node of the
        // accessibility tree we use the first accessible object inserted
        // into the canvas instead.
        com.sun.star.awt.Point aPosition = new com.sun.star.awt.Point (
            (int)((e.getX() + mnHOffset) / mnScale),
                    (int)((e.getY() + mnVOffset) / mnScale));
        if (maObjects.size() > 0 && maActiveObject != null)
        {
            // Get component interface of object which is to be queried
            // about accessible object at mouse position.
            XAccessibleComponent xComponent = maActiveObject.getComponent();
            if (xComponent != null)
            {
                XAccessible xAccessible = xComponent.getAccessibleAt (aPosition);
                if (xAccessible != null)
                {
                    XAccessibleContext xContext2 = xAccessible.getAccessibleContext();
                    maMessageDisplay.message ("accesssible at "
                        + aPosition.X + "," + aPosition.Y
                        + " is " + xContext2.getAccessibleName());
                    return;
                }
            }

        }
        maMessageDisplay.message ("no object found at"
            + aPosition.X + "," + aPosition.Y);
        */
    }

    public void mousePressed (MouseEvent e)
    {
        AccessibleObject aObjectUnderMouse = FindAccessibleObjectUnderMouse (e);
        highlightObject (aObjectUnderMouse);
        if ((e.getModifiers() & InputEvent.CTRL_MASK) != 0)
        {
            maTree.expandPath (aObjectUnderMouse.getPath());
        }
    }

    public void mouseReleased (MouseEvent e)
    {
    }

    public void mouseEntered (MouseEvent e)
    {
    }

    public void mouseExited (MouseEvent e)
    {
        // Deselect currently active object.
        if (maActiveObject != null)
        {
            maActiveObject.unhighlight ();
            maActiveObject = null;
            repaint ();
        }
    }

    public void mouseDragged (MouseEvent e)
    {
    }

    public void mouseMoved (MouseEvent e)
    {
        if ((e.getModifiers() & InputEvent.SHIFT_MASK) != 0)
            highlightObject (FindAccessibleObjectUnderMouse (e));
    }

    protected AccessibleObject FindAccessibleObjectUnderMouse (MouseEvent e)
    {
        int nObjects = maObjects.size();
        AccessibleObject aObjectUnderMouse = null;
        int nCount = maObjectList.size();
        for (int i=nCount-1; i>=0; --i)
        {
            AccessibleObject aObject = (AccessibleObject)maObjectList.elementAt(i);
            if (aObject != null)
                if (aObject.contains (e.getX(),e.getY()))
                {
                    aObjectUnderMouse = aObject;
                    break;
                }
        }
        return aObjectUnderMouse;
    }

    protected boolean highlightObject (AccessibleObject aNewActiveObject)
    {
        if (aNewActiveObject != maActiveObject)
        {
            if (maActiveObject != null)
                maActiveObject.unhighlight();

            maActiveObject = aNewActiveObject;
            if (maActiveObject != null)
            {
                if (maTree != null)
                {
                    maTree.scrollPathToVisible (maActiveObject.getPath());
                    maTree.setSelectionPath (maActiveObject.getPath());
                    maTree.repaint ();
                }
                maActiveObject.highlight ();
                repaint ();
            }
            return true;
        }
        else
            return false;
    }

    /** Called when the selection of the tree changes.  Highlight the
        corresponding graphical representation of the first selected object.
    */
    public void valueChanged (javax.swing.event.TreeSelectionEvent event)
    {
        TreePath aPath = event.getPath();
        Object aObject = aPath.getLastPathComponent();
        if (aObject instanceof AccTreeNode)
        {
            AccessibleObject aAccessibleObject = (AccessibleObject)maObjects.get ((AccTreeNode)aObject);
            if (highlightObject (aAccessibleObject))
                repaint();
        }
    }


    public Dimension getPreferredScrollableViewportSize ()
    {
        return new Dimension (nMaximumWidth,nMaximumHeight);
    }

    public int getScrollableBlockIncrement (Rectangle visibleRect, int orientation, int direction)
    {
        return 25;
    }

    public boolean getScrollableTracksViewportHeight ()
    {
        return false;
    }

    public boolean getScrollableTracksViewportWidth ()
    {
        return false;
    }

    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction)
    {
        return 1;
    }

    protected int
        mnXAnchor,
        mnYAnchor,
        maResizeFlag;
    private double
        mnHOffset,
        mnVOffset,
        mnScale;
    protected AccessibleObject
        maActiveObject;
    protected java.util.HashMap
        maObjects;
    protected Vector
        maObjectList,
        maContexts,
        maNodes;
    protected Rectangle
        maBoundingBox;
    protected JTree
        maTree;
    protected boolean
        mbShowDescriptions = true,
        mbShowNames = true,
        mbAntialiasing = true;
}
