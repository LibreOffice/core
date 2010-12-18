package org.openoffice.accessibility.awb.canvas;

import java.awt.Dimension;
import java.awt.event.InputEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.Iterator;
import javax.swing.tree.TreePath;


/** Observe the mouse and highlight shapes of the canvas when clicked.
*/
public class MouseObserver
    implements MouseListener,
               MouseMotionListener
{
    public MouseObserver (Canvas aCanvas)
    {
        maCanvas = aCanvas;
        maCanvas.addMouseListener (this);
        maCanvas.addMouseMotionListener (this);
    }


    public void SetTree (javax.swing.JTree aTree)
    {
        maTree = aTree;
    }

    public void mouseClicked (MouseEvent e)
    {}

    public void mousePressed (MouseEvent e)
    {
        CanvasShape aObjectUnderMouse = FindCanvasShapeUnderMouse (e);
        maTree.clearSelection();
        if (aObjectUnderMouse != null)
        {
            TreePath aPath = aObjectUnderMouse.getNodePath();
            if ((e.getModifiers() & InputEvent.CTRL_MASK) != 0)
                maTree.expandPath (aPath);
            // Selecting the entry will eventually highlight the shape.
            maTree.setSelectionPath (aPath);
            maTree.makeVisible (aPath);
        }
    }

    public void mouseReleased (MouseEvent e)
    {}

    public void mouseEntered (MouseEvent e)
    {}

    public void mouseExited (MouseEvent e)
    {}

    public void mouseDragged (MouseEvent e)
    {
    }

    public void mouseMoved (MouseEvent e)
    {
        if ((e.getModifiers() & InputEvent.SHIFT_MASK) != 0)
            maCanvas.HighlightObject (FindCanvasShapeUnderMouse (e));
    }


    /** Search for the smallest shape that contains the mouse position.
    */
    protected CanvasShape FindCanvasShapeUnderMouse (MouseEvent e)
    {
        Dimension aSmallestSize = null;
        Iterator maShapeIterator = maCanvas.GetShapeIterator();
        CanvasShape aShapeUnderMouse = null;
        while (maShapeIterator.hasNext())
        {
            CanvasShape aShape = (CanvasShape)maShapeIterator.next();
            if (aShape != null)
                if (aShape.Contains (e.getX(),e.getY()))
                {
                    if (aShapeUnderMouse == null)
                    {
                        aSmallestSize = aShape.GetSize();
                        aShapeUnderMouse = aShape;
                    }
                    else
                    {
                        Dimension aSize = aShape.GetSize();
                        if (aSize.getWidth()<aSmallestSize.getWidth()
                            || aSize.getHeight()<aSmallestSize.getHeight())
                        {
                            aSmallestSize = aSize;
                            aShapeUnderMouse = aShape;
                        }
                    }
                }
        }
        return aShapeUnderMouse;
    }

    private Canvas maCanvas;
    private javax.swing.JTree maTree;
}
