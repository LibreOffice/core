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
    implements MouseListener, MouseMotionListener, TreeSelectionListener
{
    public MessageInterface maMessageDisplay;
    public final int nMaximumWidth = 1000;
    public final int nMaximumHeight = 1000;

    public static boolean bPaintText = false;

    public Canvas (MessageInterface aMessageDisplay)
    {
        super (true);
        maObjects = new java.util.HashMap ();
        maNodes = new Vector ();
        maObjectList = new Vector ();
        maContexts = new Vector ();
        addMouseListener (this);
        addMouseMotionListener (this);
        maBoundingBox = new Rectangle (0,0,100,100);
        setPreferredSize (maBoundingBox.getSize());
        setSize (nMaximumWidth,nMaximumHeight);
        maMessageDisplay = aMessageDisplay;
        maTree = null;
        mnXOffset = 0;
        mnYOffset = 0;
        mnScaleFactor = 1;
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
        Graphics2D g2 = (Graphics2D)g;
        if (mbAntialiasing)
            g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);
        else
            g2.setRenderingHint (RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_OFF);

        Rectangle r = g.getClipBounds();
        g.clearRect (r.x,r.y,r.width,r.height);

        // Recalculate scale and offset so that all accessible objects fit
        // into the area specified by nMaximum(Width,Height)
        double nXScale = 1,
            nYScale = 1;
        int nWidth = (int)maBoundingBox.getWidth() + 50,
            nHeight = (int)maBoundingBox.getWidth() + 50;
        mnXOffset = (int)-maBoundingBox.getX() + 25;
        mnYOffset = (int)-maBoundingBox.getY() + 25;
        if (nWidth > nMaximumWidth)
            nXScale = 1.0 * nMaximumWidth / nWidth;
        if (nHeight > nMaximumHeight)
            nYScale = 1.0 * nMaximumHeight / nHeight;
        if (nXScale < nYScale)
            mnScaleFactor = nXScale;
        else
            mnScaleFactor = nYScale;

        //        synchronized (maObjectList)
        //        {
            int nCount = maObjectList.size();
            for (int i=0; i<nCount; i++)
            {
                AccessibleObject aAccessibleObject = (AccessibleObject)maObjectList.elementAt(i);
                aAccessibleObject.paint (
                    g,
                    mnXOffset, mnYOffset, mnScaleFactor,
                    mbShowDescriptions, mbShowNames);
            }
            //        }

        // Paint highlighted frame around active object as the last thing.
        if (maActiveObject != null)
            maActiveObject.paint_highlight (
                g,
                mnXOffset, mnYOffset, mnScaleFactor);
    }

    /**  Call getAccessibleAt to determine accessible object under mouse.
    */
    public void mouseClicked (MouseEvent e)
    {
        FindAccessibleObjectUnderMouse (e);
        // Because we have no access (at the moment) to the root node of the
        // accessibility tree we use the first accessible object inserted
        // into the canvas instead.
        com.sun.star.awt.Point aPosition = new com.sun.star.awt.Point (
            (int)((e.getX() + mnXOffset) / mnScaleFactor),
                    (int)((e.getY() + mnYOffset) / mnScaleFactor));
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
    }

    public void mousePressed (MouseEvent e)
    {
        FindAccessibleObjectUnderMouse (e);
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
            FindAccessibleObjectUnderMouse (e);
    }

    protected void FindAccessibleObjectUnderMouse (MouseEvent e)
    {
        int nObjects = maObjects.size();
        AccessibleObject aNewActiveObject = null;
        int nCount = maObjectList.size();
        for (int i=nCount-1; i>=0; --i)
        {
            AccessibleObject aObject = (AccessibleObject)maObjectList.elementAt(i);
            if (aObject != null)
                if (aObject.contains (e.getX(),e.getY()))
                {
                    aNewActiveObject = aObject;
                    break;
                }
        }
        if (highlightObject (aNewActiveObject))
        {
            if (maActiveObject != null && maTree != null)
            {
                maTree.scrollPathToVisible (maActiveObject.getPath());
                maTree.setSelectionPath (maActiveObject.getPath());
                maTree.repaint ();
            }

            repaint ();
        }
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
                maActiveObject.highlight ();
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

    protected int
        mnXAnchor,
        mnYAnchor,
        maResizeFlag,
        mnXOffset,
        mnYOffset;
    protected double
        mnScaleFactor;
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
