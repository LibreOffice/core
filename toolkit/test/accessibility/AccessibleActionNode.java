import javax.swing.JOptionPane;
import drafts.com.sun.star.accessibility.XAccessibleAction;

/**
    Base class for all tree nodes.
 */
class AccessibleActionNode
    extends StringNode
{
    public AccessibleActionNode (String aDisplayObject,
        AccessibleTreeNode aParent,
        int nActionIndex)
    {
        super (aDisplayObject, aParent);
        mnActionIndex = nActionIndex;
    }

    public String[] getActions ()
    {
        return new String[] {"Perform Action"};
    }

    /** perform action */
    public void performAction (int nIndex)
    {
        if (nIndex != 0)
            return;
        boolean bResult = false;
        if (getParent() instanceof AccTreeNode)
            try
            {
                bResult = AccessibleActionHandler.getAction(
                    (AccTreeNode)getParent()).doAccessibleAction (
                        mnActionIndex);
            }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
        }

        JOptionPane.showMessageDialog (null,
            "performed action " + mnActionIndex
            + (bResult?" with":" without") + " success",
            "Action " + mnActionIndex,
            JOptionPane.INFORMATION_MESSAGE);
    }

    private int mnActionIndex;
}
