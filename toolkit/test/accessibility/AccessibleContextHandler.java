import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XIndexAccess;
import java.util.HashMap;

class AccessibleContextHandler
    extends NodeHandler
{
    protected int nChildrenCount;

    private static String maStateNames[] = {
        "INVALID",              // 0
        "ACTIVE",
        "ARMED",
        "BUSY",
        "CHECKED",
        "COLLAPSED",
        "DEFUNC",
        "EDITABLE",
        "ENABLED",
        "EXPANDABLE",
        "EXPANDED",             // 10
        "FOCUSABLE",
        "FOCUSED",
        "HORIZONTAL",
        "ICONIFIED",
        "MODAL",
        "MULTILINE",
        "MULTISELECTABLE",
        "OPAQUE",
        "PRESSED",
        "RESIZABLE",            // 20
        "SELECTABLE",
        "SELECTED",
        "SENSITIVE",
        "SHOWING",
        "SINGLE_LINE",
        "STALE",
        "TRANSIENT",
        "VERTICAL",
        "VISIBLE",
        "MANAGES_DESCENDANT",   // 30
    };
    private static HashMap maRoleMap = new HashMap();
    static
    {
        maRoleMap.put (new Integer(0),"UNKNOWN");
        maRoleMap.put (new Integer(1), "ALERT");
        maRoleMap.put (new Integer(2), "AWTCOMPONENT");
        maRoleMap.put (new Integer(3), "COLUMNHEADER");
        maRoleMap.put (new Integer(4), "CANVAS");
        maRoleMap.put (new Integer(5), "CHECKBOX");
        maRoleMap.put (new Integer(6), "COLORCHOOSER");
        maRoleMap.put (new Integer(7), "COMBOBOX");
        maRoleMap.put (new Integer(8), "DATEEDITOR");
        maRoleMap.put (new Integer(9), "DESKTOPICON");
        maRoleMap.put (new Integer(10), "DESKTOPPANE");
        maRoleMap.put (new Integer(11), "DIRECTORYPANE");
        maRoleMap.put (new Integer(12), "DIALOG");
        maRoleMap.put (new Integer(13), "FILECHOOSER");
        maRoleMap.put (new Integer(14), "FILLER");
        maRoleMap.put (new Integer(15), "FONTCHOOSER");
        maRoleMap.put (new Integer(16), "FRAME");
        maRoleMap.put (new Integer(17), "GLASSPANE");
        maRoleMap.put (new Integer(18), "GROUPBOX");
        maRoleMap.put (new Integer(19), "HYPERLINK");
        maRoleMap.put (new Integer(20), "ICON");
        maRoleMap.put (new Integer(21), "INTERNALFRAME");
        maRoleMap.put (new Integer(22), "LABEL");
        maRoleMap.put (new Integer(23), "LAYEREDPANE");
        maRoleMap.put (new Integer(24), "LIST");
        maRoleMap.put (new Integer(25), "LISTITEM");
        maRoleMap.put (new Integer(26), "MENU");
        maRoleMap.put (new Integer(27), "MENUBAR");
        maRoleMap.put (new Integer(28), "MENUITEM");
        maRoleMap.put (new Integer(29), "OPTIONPANE");
        maRoleMap.put (new Integer(30), "PAGETAB");
        maRoleMap.put (new Integer(31), "PAGETABLIST");
        maRoleMap.put (new Integer(32), "PANEL");
        maRoleMap.put (new Integer(33), "PASSWORDTEXT");
        maRoleMap.put (new Integer(34), "POPUPMENU");
        maRoleMap.put (new Integer(35), "PUSHBUTTON");
        maRoleMap.put (new Integer(36), "PROGRESSBAR");
        maRoleMap.put (new Integer(37), "RADIOBUTTON");
        maRoleMap.put (new Integer(38), "ROWHEADER");
        maRoleMap.put (new Integer(39), "ROOTPANE");
        maRoleMap.put (new Integer(40), "SCROLLBAR");
        maRoleMap.put (new Integer(41), "SCROLLPANE");
        maRoleMap.put (new Integer(42), "SEPARATOR");
        maRoleMap.put (new Integer(43), "SLIDER");
        maRoleMap.put (new Integer(44), "SPINBOX");
        maRoleMap.put (new Integer(45), "SPLITPANE");
        maRoleMap.put (new Integer(46), "STATUSBAR");
        maRoleMap.put (new Integer(47), "SWINGCOMPONENT");
        maRoleMap.put (new Integer(48), "TABLE");
        maRoleMap.put (new Integer(49), "TEXT");
        maRoleMap.put (new Integer(50), "TOGGLEBUTTON");
        maRoleMap.put (new Integer(51), "TOOLBAR");
        maRoleMap.put (new Integer(52), "TOOLTIP");
        maRoleMap.put (new Integer(53), "TREE");
        maRoleMap.put (new Integer(54), "VIEWPORT");
        maRoleMap.put (new Integer(55), "WINDOW");
        maRoleMap.put (new Integer(100), "DOCUMENT");
        maRoleMap.put (new Integer(101), "HEADER");
        maRoleMap.put (new Integer(102), "FOOTER");
        maRoleMap.put (new Integer(103), "PARAGRAPH");
        maRoleMap.put (new Integer(104), "HEADING");
        maRoleMap.put (new Integer(105), "TABLE_CELL");
        maRoleMap.put (new Integer(106), "TEXT_FRAME");
        maRoleMap.put (new Integer(107), "GRAPHIC");
        maRoleMap.put (new Integer(108), "EMBEDDED_OBJECT");
        maRoleMap.put (new Integer(109), "ENDNOTE");
        maRoleMap.put (new Integer(110), "FOOTNOTE");
        maRoleMap.put (new Integer(111), "SHAPE");
    }
    public static String GetRoleName (int nRole)
    {
        return (String)maRoleMap.get (new Integer(nRole));
    }


    public NodeHandler createHandler (XAccessibleContext xContext)
    {
        if (xContext != null)
            return new AccessibleContextHandler (xContext);
        else
            return null;
    }

    public AccessibleContextHandler ()
    {
        super ();
    }

    public AccessibleContextHandler (XAccessibleContext xContext)
    {
        super();
        if (xContext != null)
            maChildList.setSize (4);
    }

    public AccessibleTreeNode createChild (AccessibleTreeNode aParent, int nIndex)
    {
        XAccessibleContext xContext = null;
        if (aParent instanceof AccTreeNode)
            xContext = ((AccTreeNode)aParent).getContext();

        String sChild = new String();
        if (xContext != null)
        {
            switch( nIndex )
            {
                case 0:
                    sChild = "Description: " +
                        xContext.getAccessibleDescription();
                    break;
                case 1:
                    int nRole = xContext.getAccessibleRole();
                    sChild = "Role: " + nRole + " (" + GetRoleName (nRole) + ")";
                    break;
                case 2:
                    XAccessible xParent = xContext.getAccessibleParent();
                    sChild = "Has parent: " + (xParent!=null ? "yes" : "no");
                    /*                    if (xParent != ((AccTreeNode)aParent).getAccessible())
                    {
                        sChild += " but that is inconsistent"
                            + "#" + xParent + " # " + ((AccTreeNode)aParent).getAccessible();
                    }
                    */
                    break;
                case 3:
                    sChild = "";
                    XAccessibleStateSet xStateSet =
                        xContext.getAccessibleStateSet();
                    if (xStateSet != null)
                    {
                        for (short i=0; i<=30; i++)
                        {
                            if (xStateSet.contains (i))
                            {
                                if (sChild.compareTo ("") != 0)
                                    sChild += ", ";
                                sChild += maStateNames[i];
                            }
                        }
                    }
                    else
                        sChild += "no state set";
                    sChild = "State set: " + sChild;

                    /*                case 3:
                    sChild = "Child count: " + xContext.getAccessibleChildCount();
                    break;*/
            }
        }
        return new StringNode (sChild, aParent);
    }
}
