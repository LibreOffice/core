import com.sun.star.lang.IndexOutOfBoundsException;
import java.util.Vector;

/** The VectorNode class is a simple container whose list of children is
    managed entirely by its owner.
*/
class VectorNode
    extends StringNode
{
    private Vector maChildren;

    public VectorNode (String sDisplayObject, AccessibleTreeNode aParent)
    {
        super (sDisplayObject, aParent);

        maChildren = new Vector ();
    }

    public void addChild (AccessibleTreeNode aChild)
    {
        maChildren.add (aChild);
    }

    public int getChildCount ()
    {
        return maChildren.size();
    }

    public AccessibleTreeNode getChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        return (AccessibleTreeNode)maChildren.elementAt (nIndex);
    }

    public boolean removeChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        return maChildren.remove (nIndex) != null;
    }

    public int indexOf (AccessibleTreeNode aNode)
    {
        return maChildren.indexOf (aNode);
    }

    public boolean isLeaf()
    {
        return maChildren.isEmpty();
    }
}
