import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;
import javax.swing.JTree;
import java.awt.Color;
import java.awt.Component;
import java.util.Vector;


public class AccessibleTreeCellRenderer
    extends DefaultTreeCellRenderer
{
    public Color
        maDefaultColor,
        maChangedColor;
    protected Vector
        maChangedLines;



    public AccessibleTreeCellRenderer ()
    {
        maDefaultColor = Color.black;
        maChangedColor = Color.red;
        maChangedLines = new Vector ();
    }

    public Component getTreeCellRendererComponent (
        JTree tree,
        Object value,
        boolean sel,
        boolean expanded,
        boolean leaf,
        int row,
        boolean hasFocus)
    {
        super.getTreeCellRendererComponent(
            tree, value, sel,
            expanded, leaf, row,
            hasFocus);

        if (maChangedLines.size()<=row || maChangedLines.elementAt (row) == null)
            setTextNonSelectionColor (maDefaultColor);
        else
            setTextNonSelectionColor (maChangedColor);

        return this;
    }

    /** Tell the cell renderer that no changes shall be displayed anymore.
    */
    public void clearAllChanges ()
    {
        maChangedLines.clear();
    }

    /** Inform the cell renderer of a new changed line which to paint
        highlighted when asked to paint it the next time.
    */
    public void addChangedLine (int nRow)
    {
        if (maChangedLines.size() <= nRow)
            maChangedLines.setSize (nRow+1);
        nRow -= 1; // row index is one to large for some reason.
        maChangedLines.set (nRow, new Boolean (true));
    }

    /** Inform the cell renderer of a set of changed line which to paint
        highlighted when asked to paint them the next time.
        @param aChangedNodes
            The set of changed nodes.  Each entry is a TreePath.
        @param aTree
            The JTree that is used to transform the given TreePath objects
            into rows.
    */
    public void addChangedNodes (Vector aChangedNodes, JTree aTree)
    {
        for (int i=0; i<aChangedNodes.size(); i++)
        {
            TreePath aPath = (TreePath)aChangedNodes.elementAt (i);
            int nRow = aTree.getRowForPath (aPath);
            addChangedLine (nRow);
        }
    }

}

