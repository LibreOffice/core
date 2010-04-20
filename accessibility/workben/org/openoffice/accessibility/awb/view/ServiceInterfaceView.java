package org.openoffice.accessibility.awb.view;

import java.awt.GridLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.lang.Integer;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

import org.openoffice.accessibility.misc.NameProvider;


/** Show all supported services and interfaces.
*/
public class ServiceInterfaceView
    extends ObjectView
{
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        if (xContext != null)
            return new ServiceInterfaceView (aContainer);
        else
            return null;
    }




    public ServiceInterfaceView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        maImplementationNameRoot = new DefaultMutableTreeNode ("Implementation Name");
        maServiceRoot = new DefaultMutableTreeNode ("Supported Services");
        maInterfaceRoot = new DefaultMutableTreeNode ("Supported Interfaces");
        maTree = new JTree (new DefaultMutableTreeNode[]
            {maServiceRoot,maInterfaceRoot});
        JScrollPane aScrollPane = new JScrollPane (
            maTree,
            JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
            JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        setMinimumSize (new Dimension(300,200));
        setLayout (new GridLayout (1,1));
        add (aScrollPane);
    }




    public void Update ()
    {
        DefaultTreeModel aModel = (DefaultTreeModel)maTree.getModel();

        // Clear old tree.
        DefaultMutableTreeNode aRoot =(DefaultMutableTreeNode)aModel.getRoot();
        aRoot.removeAllChildren();

        // Create the new tree.
        CreateImplementationNameTree ();
        CreateServiceTree ();
        CreateInterfaceTree ();
        aRoot.add (maImplementationNameRoot);
        aRoot.add (maServiceRoot);
        aRoot.add (maInterfaceRoot);
        aModel.setRoot (aRoot);

        // Expand whole tree.
        for (int i=0; i<maTree.getRowCount(); i++)
            maTree.expandRow (i);
    }

    private void CreateImplementationNameTree ()
    {
        XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(
            XServiceInfo.class, mxContext);
        maImplementationNameRoot.removeAllChildren();
        if (xServiceInfo != null)
        {
            maImplementationNameRoot.add (
                new DefaultMutableTreeNode (
                    (xServiceInfo!=null
                        ? xServiceInfo.getImplementationName()
                        : "<XServiceInfo not supported>")));
        }
    }

    private void CreateServiceTree ()
    {
        XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(
            XServiceInfo.class, mxContext);
        maServiceRoot.removeAllChildren();
        if (xServiceInfo != null)
        {
            String[] aServiceNames = xServiceInfo.getSupportedServiceNames();
            int nCount = aServiceNames.length;
            for (int i=0; i<nCount; i++)
                maServiceRoot.add (
                    new DefaultMutableTreeNode (aServiceNames[i]));
        }
        else
            maServiceRoot.add (
                new DefaultMutableTreeNode("XServiceInfo not supported"));
    }

    private void CreateInterfaceTree ()
    {
        XTypeProvider xTypeProvider = (XTypeProvider)UnoRuntime.queryInterface(
            XTypeProvider.class, mxContext);
        maInterfaceRoot.removeAllChildren();
        if (xTypeProvider != null)
        {
            Type[] aTypes = xTypeProvider.getTypes();
            int nCount = aTypes.length;
            for (int i=0; i<nCount; i++)
                maInterfaceRoot.add (
                    new DefaultMutableTreeNode (aTypes[i].getTypeName()));
        }
        else
            maInterfaceRoot.add (
                new DefaultMutableTreeNode("XTypeProvider not supported"));
    }

    public String GetTitle ()
    {
        return ("Supported Services and Interfaces");
    }


    private JTree maTree;
    private DefaultMutableTreeNode maImplementationNameRoot;
    private DefaultMutableTreeNode maServiceRoot;
    private DefaultMutableTreeNode maInterfaceRoot;
}
