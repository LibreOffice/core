import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;

import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleComponent;

/** This canvas displays accessible objects graphically.  Each accessible
    object with graphical representation is represented by an
    AccessibleObject object and has to be added by the
    <member>addAccessible</member> member function.
*/
class Canvas
    extends JPanel
    implements MouseListener, MouseMotionListener
{
    public MessageInterface maMessageDisplay;
    public final int nMaximumWidth = 1000;
    public final int nMaximumHeight = 1000;

    public static boolean bPaintText = false;

    public Canvas (MessageInterface aMessageDisplay)
    {
        super (true);
        maObjects = new Vector ();
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
        maTree = aTree;
    }

    public void addAccessible (AccessibleObject aObject)
    {
        // Update bounding box that includes all objects.
        if (maObjects.size() == 0)
            maBoundingBox = aObject.getBBox();
        else
            maBoundingBox = maBoundingBox.union (aObject.getBBox());

        if( maObjects.indexOf( aObject ) == -1 )
        {
            maObjects.add (aObject);
            maContexts.add (aObject.getContext());
        }
        repaint ();
    }

    public void removeAccessible (AccessibleObject aObject)
    {
        maObjects.remove (aObject);
        maContexts.remove (aObject.getContext());
        repaint ();
    }


    public void addContext(XAccessibleContext xContext, TreePath aPath)
    {
        if( maContexts.indexOf( xContext ) == -1 )
            addAccessible( new AccessibleObject( xContext, aPath ) );
    }

    public void removeContext(XAccessibleContext xContext)
    {
        int i = maContexts.indexOf( xContext );
        if( i != -1 )
            removeAccessible( (AccessibleObject)maObjects.elementAt( i ) );
    }

    public void updateContext(XAccessibleContext aContext)
    {
        int i = maContexts.indexOf( aContext );
        if( i != -1 )
            ((AccessibleObject)maObjects.elementAt( i )).update();
    }

    public void clear ()
    {
        maObjects.clear();
        maContexts.clear();
    }

    public void paintComponent (Graphics g)
    {
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

        int n = maObjects.size();
        for (int i=0; i<n; i++)
        {
            AccessibleObject aAccessibleObject = (AccessibleObject)maObjects.elementAt(i);
            if ( ! aAccessibleObject.isSelected())
                aAccessibleObject.paint (
                    g,
                    mnXOffset, mnYOffset, mnScaleFactor);
        }
        if (maActiveObject != null)
            maActiveObject.paint (
                    g,
                    mnXOffset, mnYOffset, mnScaleFactor);
    }

    public void mouseClicked (MouseEvent e)
    {
        // Because we have no access (at the moment) to the root node of the
        // accessibility tree we use the first accessible object inserted
        // into the canvas instead.
        com.sun.star.awt.Point aPosition = new com.sun.star.awt.Point (
            (int)((e.getX() + mnXOffset) / mnScaleFactor),
                    (int)((e.getY() + mnYOffset) / mnScaleFactor));
        if (maObjects.size() > 0)
        {
            // Get component interface of object which is to be queried
            // about accessible object at mouse position.
            XAccessibleContext xContext = maActiveObject.getContext();
            XAccessibleComponent xComponent =
                (XAccessibleComponent) UnoRuntime.queryInterface (
                    XAccessibleComponent.class, xContext);
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
            maActiveObject.deselect ();
            maActiveObject = null;
            repaint ();
        }
    }

    public void mouseDragged (MouseEvent e)
    {
        if (maActiveObject != null)
        {
            int dx = e.getX() - mnXAnchor;
            int dy = e.getY() - mnYAnchor;

            if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0)
            {
                maActiveObject.translate (dx,dy);
            }
            else if ((e.getModifiers() & MouseEvent.BUTTON2_MASK) != 0)
            {
                maActiveObject.resize (maResizeFlag, dx,dy);
            }

            mnXAnchor = e.getX();
            mnYAnchor = e.getY();

            repaint ();
        }
    }

    public void mouseMoved (MouseEvent e)
    {
        int nObjects = maObjects.size();
        AccessibleObject aNewActiveObject = null;
        for (int i=nObjects-1; i>=0; i--)
        {
            AccessibleObject aObject = (AccessibleObject)(maObjects.elementAt(i));
            if (aObject != null)
                if (aObject.contains (e.getX(),e.getY()))
                {
                    aNewActiveObject = aObject;
                    break;
                }
        }
        if (aNewActiveObject != maActiveObject)
        {
            if (maActiveObject != null)
                maActiveObject.deselect();

            maActiveObject = aNewActiveObject;
            if (maActiveObject != null)
            {
                maActiveObject.select ();

                if (maTree != null)
                {
                    maTree.scrollPathToVisible (maActiveObject.getPath());
                    maTree.setSelectionPath (maActiveObject.getPath());
                    maTree.repaint ();
                }
            }

            repaint ();
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
    protected Vector
        maObjects,
        maContexts;
    protected Rectangle
        maBoundingBox;
    protected JTree
        maTree;
}
