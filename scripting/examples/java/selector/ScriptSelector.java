import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XModel;
import com.sun.star.container.XEnumeration;
import com.sun.star.beans.*;
import com.sun.star.util.XMacroExpander;

import com.sun.star.lib.uno.helper.PropertySet;

import drafts.com.sun.star.script.framework.browse.XBrowseNode;
import drafts.com.sun.star.script.framework.browse.BrowseNodeTypes;
import drafts.com.sun.star.script.framework.runtime.XScriptContext;

public class ScriptSelector extends JFrame implements ActionListener {

    private XBrowseNode myrootnode = null;
    private JButton runButton, editButton, deleteButton;
    private JTextField textField;

    public ScriptSelector(XScriptContext ctxt)
    {
        //super("Script Selector Prototype");
        // myrootnode = new RootBrowseNode(ctxt);

        try {
            String serviceName = "drafts.com.sun.star.script." +
                "framework.provider.MasterScriptProvider";

            XComponentContext xcc = ctxt.getComponentContext();
            XMultiComponentFactory xmcf = xcc.getServiceManager();

            Object serviceObj =
                xmcf.createInstanceWithContext(serviceName, xcc);

            myrootnode = (XBrowseNode)UnoRuntime.queryInterface(
                XBrowseNode.class, serviceObj);
        }
        catch (Exception e) {
            e.printStackTrace();
            return;
        }

        initUI();
    }

    public static void go(XScriptContext ctxt)
    {
        try {
           ScriptSelector client = new ScriptSelector(ctxt);
           client.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
           client.show();
        }
        catch (com.sun.star.uno.RuntimeException rue) {
            rue.printStackTrace();
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
        }
    }

    private void initUI() {
        JPanel panel = new JPanel();
        panel.setLayout(new BorderLayout());

        DefaultMutableTreeNode top = new DefaultMutableTreeNode(myrootnode);
        initNodes(myrootnode, top);
        final JTree tree = new JTree(top);

        tree.setCellRenderer(new ScriptTreeRenderer());

        tree.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                    tree.getLastSelectedPathComponent();

                if (node == null) return;

                XBrowseNode xbn = (XBrowseNode)node.getUserObject();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xbn);

                checkEnabled(props, "Deletable", deleteButton);
                checkEnabled(props, "Editable", editButton);

                String str = xbn.getName();
                if (xbn.getType() == BrowseNodeTypes.SCRIPT && props != null)
                {
                    try {
                        str = AnyConverter.toString(
                            props.getPropertyValue("URI"));
                    }
                    catch (Exception ignore) {
                        // default will be used
                    }
                }
                textField.setText(str);

                if (xbn.getType() == BrowseNodeTypes.SCRIPT)
                {
                    runButton.setEnabled(true);
                }
                else
                {
                    runButton.setEnabled(false);
                }
            }

            private void checkEnabled(XPropertySet props, String name,
                JButton button)
            {
                boolean enable = false;

                try
                {
                    if (props != null)
                    {
                        Object o = props.getPropertyValue(name);
                        enable = AnyConverter.toBoolean(
                            props.getPropertyValue(name));
                    }
                }
                catch (com.sun.star.lang.IllegalArgumentException iae)
                {
                    // leave enable set to false
                }
                catch (com.sun.star.beans.UnknownPropertyException upe)
                {
                    // leave enable set to false
                }
                catch (com.sun.star.lang.WrappedTargetException wte)
                {
                    // leave enable set to false
                }

                button.setEnabled(enable);
            }
        });

        JScrollPane scroller = new JScrollPane(tree);
        panel.add(scroller, BorderLayout.CENTER);

        // JTable table = new JTable(new ScriptTableModel(myrootnode));
        // JScrollPane tableScroller = new JScrollPane(table);
        // panel.add(tableScroller, BorderLayout.NORTH);

        JPanel lowerPanel = new JPanel();
        textField = new JTextField();
        lowerPanel.setLayout(new BorderLayout());
        lowerPanel.add(textField, BorderLayout.NORTH);

        JPanel buttonPanel = new JPanel();
        runButton = new JButton("Run");
        runButton.addActionListener(this);
        runButton.setEnabled(false);

        editButton = new JButton("Edit");
        editButton.addActionListener(this);
        editButton.setEnabled(false);

        deleteButton = new JButton("Delete");
        deleteButton.addActionListener(this);
        deleteButton.setEnabled(false);

        buttonPanel.add(runButton);
        buttonPanel.add(editButton);
        buttonPanel.add(deleteButton);
        lowerPanel.add(buttonPanel, BorderLayout.SOUTH);

        panel.add(lowerPanel, BorderLayout.SOUTH);

        getContentPane().add(panel);
        setSize(600, 300);
    }

    public void actionPerformed(ActionEvent event) {
        if (event.getSource() == runButton) {
            String uri = textField.getText();

            // XScript script = msp.getScript(uri);
            // script.invoke();
        }
        else if (event.getSource() == editButton) {
        }
        else if (event.getSource() == deleteButton) {
        }
    }

    private void initNodes(XBrowseNode parent, DefaultMutableTreeNode top) {
        if ( parent == null || parent.hasChildNodes() == false )
        {
            return;
        }
        System.err.println("initNodes parent = " + parent.getName() );
        XBrowseNode[] children = parent.getChildNodes();

        try {
        if (children != null) {
            for (int i = 0; i < children.length; i++) {
                if ( children[i] != null )
                {
                    System.err.println("initNodes child is " + children[ i ].getName() );
                }
                else
                {
                    System.err.println("initNodes child " + i + " is null " );
                    continue;
                }
                DefaultMutableTreeNode newNode =
                    new DefaultMutableTreeNode(children[i]) {
                    public String toString(){ return ( (XBrowseNode)getUserObject() ).getName(); } };
                top.add(newNode);
                initNodes(children[i], newNode);
            }
        }
        } catch (java.lang.Exception e) { e.printStackTrace(); }
    }
}

class RootBrowseNode implements XBrowseNode {

    private Collection nodes = new ArrayList();

    public RootBrowseNode(XScriptContext ctxt) {

        Object serviceObj = ctxt.getComponentContext().getValueByName(
            "/singletons/com.sun.star.util.theMacroExpander");

        try {
            XMacroExpander me = (XMacroExpander) AnyConverter.toObject(
                new Type(XMacroExpander.class), serviceObj);

            MyMasterScriptProvider msp;

            msp = new MyMasterScriptProvider(ctxt, me.expandMacros(
                "${$SYSBINDIR/bootstraprc::BaseInstallation}/share"), "share");
            nodes.add(msp);

            msp = new MyMasterScriptProvider(ctxt, me.expandMacros(
                "${$SYSBINDIR/bootstraprc::UserInstallation}/user"), "user");
            nodes.add(msp);

            XEnumeration docs =
                ctxt.getDesktop().getComponents().createEnumeration();

            while (docs.hasMoreElements())
            {
                Object o = docs.nextElement();
                XComponent comp = (XComponent)AnyConverter.toObject(
                    new Type(XComponent.class), o);

                XModel doc =
                    (XModel)UnoRuntime.queryInterface(XModel.class, comp);

                msp = new MyMasterScriptProvider(doc, ctxt);
                nodes.add(msp);
            }
        }
        catch (Exception e) {
            System.err.println("Error creating RootNode: " + e.getMessage());
        }
    }

    public String getName() {
        return "soffice";
    }

    public XBrowseNode[] getChildNodes() {
        return (XBrowseNode[])nodes.toArray(new XBrowseNode[0]);
    }

    public boolean hasChildNodes() {
        return true;
    }

    public short getType() {
        return BrowseNodeTypes.ROOT;
    }

    public String toString() {
        return getName();
    }
}

class MyMasterScriptProvider implements XBrowseNode {

    private Collection nodes = new ArrayList();
    private XModel model;
    private String path;
    private String name;
    private XComponentContext xcc;
    private XMultiComponentFactory xmcf;

    public MyMasterScriptProvider(XScriptContext ctxt, String path, String name)
    {
        this.path = path;
        this.name = name;

        xcc = ctxt.getComponentContext();
        xmcf = xcc.getServiceManager();

        String[] languages = {"Java", "BeanShell", "JavaScript"};

        for (int i = 0; i < languages.length; i++)
        {
            createScriptProvider(languages[i]);
        }
    }

    public MyMasterScriptProvider(XModel model, XScriptContext ctxt)
    {
        this.model = model;
        this.name = model.getURL();
        this.name = this.name.substring(this.name.lastIndexOf("/") + 1);

        xcc = ctxt.getComponentContext();
        xmcf = xcc.getServiceManager();

        String[] languages = {"Java", "BeanShell", "JavaScript"};

        for (int i = 0; i < languages.length; i++)
        {
            createScriptProvider(languages[i]);
        }
    }

    private void createScriptProvider(String language) {

        Any[] args = new Any[1];

        if (path != null)
        {
            args[0] = new Any(new Type(String.class), path);
        }
        else if (model != null)
        {
            System.out.println("creating property set for: " + model.getURL());
            XPropertySet props = new InvocationPropertySet(model);
            System.out.println("property set created");
            args[0] = new Any(new Type(XPropertySet.class), props);
            System.out.println("and stuffed into Any");
        }

        try {
            String serviceName = "drafts.com.sun.star.script." +
                "framework.provider.ScriptProviderFor" + language;

            System.out.println("now createInstance");
            Object serviceObj = xmcf.createInstanceWithArgumentsAndContext(
                serviceName, args, xcc);

            XBrowseNode xbn = (XBrowseNode)UnoRuntime.queryInterface(
                XBrowseNode.class, serviceObj);

            nodes.add(xbn);
        }
        catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
            System.err.println("Error creating SPFor" + language + ": " + e);
        }
    }

    public String getName() {
        return name;
    }

    public XBrowseNode[] getChildNodes() {
        return (XBrowseNode[])nodes.toArray(new XBrowseNode[0]);
    }

    public boolean hasChildNodes() {
        return true;
    }

    public short getType() {
        return BrowseNodeTypes.CONTAINER;
    }

    public String toString() {
        return getName();
    }

    public class InvocationPropertySet extends PropertySet {
        public XModel model;
        public int id = 7;
        public String uri;

        public InvocationPropertySet(XModel model) {
            this.model = model;
            this.uri = model.getURL();

            registerProperty("SCRIPTING_DOC_REF", new Type(XModel.class),
                (short)0, "model");
            registerProperty("SCRIPTING_DOC_STORAGE_ID", new Type(int.class),
                (short)0, "id");
            registerProperty("SCRIPTING_DOC_URI", new Type(String.class),
                (short)0, "uri");
        }
    }
}

class ScriptTreeRenderer extends DefaultTreeCellRenderer {

    private ImageIcon sofficeIcon;
    private ImageIcon scriptIcon;
    private ImageIcon containerIcon;

    public ScriptTreeRenderer() {
        sofficeIcon = new ImageIcon(getClass().getResource("soffice.gif"));
        scriptIcon = new ImageIcon(getClass().getResource("script.gif"));
        containerIcon = new ImageIcon(getClass().getResource("container.gif"));
    }

    public Component getTreeCellRendererComponent(
                        JTree tree,
                        Object value,
                        boolean sel,
                        boolean expanded,
                        boolean leaf,
                        int row,
                        boolean hasFocus) {

        super.getTreeCellRendererComponent(
                        tree, value, sel,
                        expanded, leaf, row,
                        hasFocus);

        DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
        XBrowseNode xbn = (XBrowseNode)node.getUserObject();
        if (xbn.getType() == BrowseNodeTypes.SCRIPT) {
            setIcon(scriptIcon);
        }
        else if(xbn.getType() == BrowseNodeTypes.CONTAINER) {
            setIcon(containerIcon);
        }
        else if(xbn.getType() == BrowseNodeTypes.ROOT) {
            setIcon(sofficeIcon);
        }

        return this;
    }
}

class ScriptTableModel extends AbstractTableModel {
    final String[] columnNames = {"Name", "Full Name"};
    private Vector scripts;

    public ScriptTableModel(XBrowseNode myrootnode) {
        scripts = new Vector();
        addScriptNodes(myrootnode);
    }

    private void addScriptNodes(XBrowseNode root) {
        XBrowseNode[] nodes = root.getChildNodes();

        if (nodes != null) {
            for (int i = 0; i < nodes.length; i++) {
                if (nodes[i].getType() == BrowseNodeTypes.SCRIPT) {
                    scripts.addElement(nodes[i]);
                }
                else {
                    addScriptNodes(nodes[i]);
                }
            }
        }
    }

    public int getColumnCount() {
        return columnNames.length;
    }

    public int getRowCount() {
        return scripts.size();
    }

    public String getColumnName(int col) {
        return columnNames[col];
    }

    public Object getValueAt(int row, int col) {
        String result = "";

        XBrowseNode xbn = (XBrowseNode)scripts.elementAt(row);

        if (col == 0)
            result = xbn.getName();
        else if (col == 1)
            result = xbn.getName();

        return result;
    }

    public boolean isCellEditable(int row, int col) {
        return false;
    }

    public void setValueAt(Object value, int row, int col) {
        return;
    }
}
