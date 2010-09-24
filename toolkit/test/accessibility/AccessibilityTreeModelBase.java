import javax.swing.tree.TreeModel;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelEvent;
import java.util.Vector;

public class AccessibilityTreeModelBase
    implements TreeModel
{
    public AccessibilityTreeModelBase ()
    {
        setRoot (null);
        maTMListeners = new Vector();
    }

    public synchronized void addTreeModelListener(TreeModelListener l)
    {
        maTMListeners.add(l);
    }

    public synchronized void removeTreeModelListener(TreeModelListener l)
    {
        maTMListeners.remove(l);
    }

    public synchronized int getChildCount(Object aParent)
    {
        return (aParent instanceof AccessibleTreeNode) ?
            ((AccessibleTreeNode)aParent).getChildCount() : 0;
    }

    public synchronized Object getChild (Object aParent, int nIndex)
    {
        Object aChild = null;
        try
        {
            if (aParent != null && aParent instanceof AccessibleTreeNode)
                aChild = ((AccessibleTreeNode)aParent).getChild(nIndex);
            else
                System.out.println ("getChild called for unknown parent node");
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            aChild = ("no child " + nIndex + " from " + aParent + ": " + e);
        }
        return aChild;
    }

    public synchronized Object getChildNoCreate (Object aParent, int nIndex)
    {
        Object aChild = null;
        try
        {
            if (aParent != null && aParent instanceof AccessibleTreeNode)
                aChild = ((AccessibleTreeNode)aParent).getChildNoCreate(nIndex);
            else
                System.out.println ("getChild called for unknown parent node");
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        { }
        return aChild;
    }

    /** iterate over all children and look for child */
    public synchronized int getIndexOfChild (Object aParent, Object aChild)
    {
        int nIndex = -1;
        try
        {
            if ((aParent instanceof AccessibleTreeNode) && (aChild instanceof AccessibleTreeNode))
            {
                AccessibleTreeNode aParentNode = (AccessibleTreeNode) aParent;
                AccessibleTreeNode aChildNode = (AccessibleTreeNode) aChild;

                int nChildCount = aParentNode.getChildCount();
                for( int i = 0; i < nChildCount; i++ )
                {
                    if (aChildNode.equals (aParentNode.getChild (i)))
                    {
                        nIndex = i;
                        break;
                    }
                }
            }
        }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
            // Return -1 by falling through.
        }

        // not found?
        return nIndex;
    }

    public boolean isLeaf (Object aNode)
    {
        return (aNode instanceof AccessibleTreeNode) ?
            ((AccessibleTreeNode)aNode).isLeaf() : true;
    }



    public synchronized Object getRoot()
    {
        return maRoot;
    }

    public void valueForPathChanged(TreePath path, Object newValue)
    { }

    protected synchronized void setRoot (AccessibleTreeNode aRoot)
    {
        maRoot = aRoot;
    }


    // The list of TreeModelListener objects.
    protected Vector maTMListeners;

    // The root node of the tree.  Use setRoot to change it.
    private AccessibleTreeNode maRoot = null;
}
