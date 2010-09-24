
import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleTable;


class AccessibleTableHandler extends NodeHandler
{
    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        XAccessibleTable xTable =
            (XAccessibleTable) UnoRuntime.queryInterface (
                XAccessibleTable.class, xContext);
        if (xTable != null)
            return new AccessibleTableHandler (xTable);
        else
            return null;
    }

    public AccessibleTableHandler ()
    {
    }

    public AccessibleTableHandler (XAccessibleTable xTable)
    {
        if (xTable != null)
            maChildList.setSize (4);
    }

    protected static XAccessibleTable getTable(Object aObject)
    {
        return (XAccessibleTable) UnoRuntime.queryInterface (
            XAccessibleTable.class, aObject);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        AccessibleTreeNode aChild = null;
        XAccessibleTable xTable = null;
        if (aParent instanceof AccTreeNode)
            xTable = ((AccTreeNode)aParent).getTable();
        try
        {
            if( xTable != null )
            {
                switch( nIndex )
                {
                    case 0:
                        aChild = new StringNode ("# table rows: " + xTable.getAccessibleRowCount(), aParent);
                        break;
                    case 1:
                        aChild = new StringNode ("# table columns: " + xTable.getAccessibleColumnCount(), aParent);
                        break;
                    case 2:
                        {
                            String sText = "selected rows: ";
                            int[] aSelected = xTable.getSelectedAccessibleRows();
                            for( int i=0; i < aSelected.length; i++ )
                            {
                                sText += aSelected[i];
                                sText += " ";
                            }
                            aChild = new StringNode (sText, aParent);
                        }
                        break;
                    case 3:
                        {
                            String sText = "selected columns: ";
                            int[] aSelected = xTable.getSelectedAccessibleColumns();
                            for( int i=0; i < aSelected.length; i++ )
                            {
                                sText += aSelected[i];
                                sText += " ";
                            }
                            aChild = new StringNode (sText, aParent);
                        }
                        break;
                    default:
                        aChild = new StringNode ("unknown child index " + nIndex, aParent);
                }
            }
        }
        catch (Exception e)
        {
            // Return empty child.
        }

        return aChild;
    }
}
