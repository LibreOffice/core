import java.util.Vector;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import drafts.com.sun.star.accessibility.XAccessibleContext;

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

    public Canvas (MessageInterface aMessageDisplay, JTree aTree)
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
        maTree = aTree;
        mnXOffset = 0;
        mnYOffset = 0;
        mnScaleFactor = 1;
    }

    public void addAccessible (AccessibleObject aObject)
    {
        if( maObjects.indexOf( aObject ) == -1 )
        {
            maObjects.add (aObject);
            maContexts.add (aObject.getContext());
        }
        maBoundingBox = maBoundingBox.union (aObject.getBBox());
    }

    public void removeAccessible (AccessibleObject aObject)
    {
        maObjects.remove (aObject);
        maContexts.remove (aObject.getContext());
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
        if (maBoundingBox.getWidth() > nMaximumWidth)
            nXScale = 1.0 * nMaximumWidth / maBoundingBox.getWidth();
        if (maBoundingBox.getHeight() > nMaximumHeight)
            nYScale = 1.0 * nMaximumHeight / maBoundingBox.getHeight();
        if (nXScale < nYScale)
            mnScaleFactor = nXScale;
        else
            mnScaleFactor = nYScale;

        int n = maObjects.size();
        for (int i=0; i<n; i++)
            ((AccessibleObject)(maObjects.elementAt(i))).paint (
                g,
                (int)-maBoundingBox.getX(),
                (int)-maBoundingBox.getY(),
                mnScaleFactor);
    }

    public void mouseClicked (MouseEvent e)
    {
    }

    public void mousePressed (MouseEvent e)
    {
        mnXAnchor = e.getX();
        mnYAnchor = e.getY();
        int n = maObjects.size();
        for (int i=0; i<n; i++)
        {
            AccessibleObject aObject = (AccessibleObject)(maObjects.elementAt(i));
            if (aObject != null && aObject.contains (e.getX(),e.getY()))
            {
                maActiveObject = aObject;
                maResizeFlag = aObject.getResizeFlag (e.getX(),e.getY());
            }
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
        int n = maObjects.size();
        for (int i=0; i<n; i++)
        {
            AccessibleObject aObject = (AccessibleObject)(maObjects.elementAt(i));
            if (aObject != null)
                aObject.deselect ();
        }
        repaint ();
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
        maActiveObject = null;
        for (int i=0; i<maObjects.size(); i++)
        {
            AccessibleObject aObject = (AccessibleObject)(maObjects.elementAt(i));
            if (aObject != null)
            {
                aObject.deselect ();
                if (aObject != null)
                    if (aObject.contains (e.getX(),e.getY()))
                    {
                        maActiveObject = aObject;
                    }
            }
        }
        if (maActiveObject != null)
        {
            maActiveObject.select ();
            maMessageDisplay.message ("mouse moved to " + e.getX() + "," + e.getY() + ": "
                +maActiveObject.toString());
            System.out.println ("path: " + maActiveObject.getPath());

            if (maTree != null)
            {
                maTree.scrollPathToVisible (maActiveObject.getPath());
                maTree.repaint ();
            }
        }
        repaint ();
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
