import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.table.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.beans.*;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XModel;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XDispatchHelper;
import com.sun.star.frame.XDispatch;
import com.sun.star.util.XURLTransformer;
import com.sun.star.beans.*;
import com.sun.star.script.XInvocation;

import com.sun.star.lib.uno.helper.PropertySet;

import drafts.com.sun.star.script.browse.XBrowseNode;
import drafts.com.sun.star.script.browse.BrowseNodeTypes;
import drafts.com.sun.star.script.provider.XScriptContext;
import drafts.com.sun.star.script.provider.XScript;
import drafts.com.sun.star.script.provider.XScriptProvider;

public class ScriptSelector {

    private static final int BIG_GAP = 10;
    private static final int MED_GAP = 5;

    public static String go(final XScriptContext ctxt)
    {
        String result = "";

        try {
            XBrowseNode root = getRootNode(ctxt);

            final ScriptSelectorPanel selectorPanel =
                new ScriptSelectorPanel(root);

            final JOptionPane optionPane = new JOptionPane(
                 selectorPanel,
                 JOptionPane.PLAIN_MESSAGE,
                 JOptionPane.OK_CANCEL_OPTION);

            final JDialog dialog = new JDialog();
            dialog.setModal(true);
            dialog.setContentPane(optionPane);
            dialog.setDefaultCloseOperation(
                JDialog.DO_NOTHING_ON_CLOSE);

            optionPane.addPropertyChangeListener(
                new PropertyChangeListener() {
                    public void propertyChange(java.beans.PropertyChangeEvent e)
                    {
                        String prop = e.getPropertyName();

                        if (dialog.isVisible()
                            && (e.getSource() == optionPane)
                            && (prop.equals(JOptionPane.VALUE_PROPERTY))) {
                                dialog.setVisible(false);
                        }
                    }
                }
            );

            dialog.pack();
            dialog.setSize(375, 350);
            dialog.setVisible(true);

            int value = ((Integer)optionPane.getValue()).intValue();
            if (value == JOptionPane.YES_OPTION) {
                result = selectorPanel.textField.getText();
            }
        }
        catch (com.sun.star.uno.RuntimeException rue) {
            rue.printStackTrace();
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    public static void showOrganizer(final XScriptContext ctxt)
    {
        try {
            XBrowseNode root = getRootNode(ctxt);

            final XScriptProvider msp =
                (XScriptProvider)UnoRuntime.queryInterface(
                 XScriptProvider.class, root);

            final JFrame client = new JFrame("Script");

            final ScriptSelectorPanel selectorPanel =
                new ScriptSelectorPanel(root);

            final JButton runButton, closeButton, assignButton,
                          editButton, deleteButton;

            runButton = new JButton("Run");
            runButton.setEnabled(false);

            closeButton = new JButton("Close");

            editButton = new JButton("Edit");
            editButton.setEnabled(false);

            JPanel northButtons =
                new JPanel(new GridLayout(2, 1, MED_GAP, MED_GAP));

            // northButtons.add(runButton);
            northButtons.add(editButton);
            northButtons.add(closeButton);

            assignButton = new JButton("Assign");
            assignButton.setEnabled(false);

            deleteButton = new JButton("Delete");
            deleteButton.setEnabled(false);

            JPanel southButtons =
                new JPanel(new GridLayout(3, 1, MED_GAP, MED_GAP));

            // southButtons.add(assignButton);
            // southButtons.add(editButton);
            // southButtons.add(deleteButton);

            selectorPanel.tree.addTreeSelectionListener(
                new TreeSelectionListener() {
                    public void valueChanged(TreeSelectionEvent e) {
                        XBrowseNode xbn = selectorPanel.getSelection();
                        XPropertySet props = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class, xbn);

                        checkEnabled(props, "Deletable", deleteButton);
                        checkEnabled(props, "Editable", editButton);

                        try {
                            if (props != null) {
                                String str = AnyConverter.toString(
                                    props.getPropertyValue("URI"));
                                if (str.indexOf("language=Basic") != -1) {
                                    editButton.setEnabled(true);
                                }
                            }
                        }
                        catch (Exception ignore) {
                            editButton.setEnabled(false);
                        }

                        if (xbn.getType() == BrowseNodeTypes.SCRIPT)
                        {
                            runButton.setEnabled(true);
                        }
                        else
                        {
                            runButton.setEnabled(false);
                        }
                    }
                }
            );

            ActionListener listener = new ActionListener() {
                public void actionPerformed(ActionEvent event) {
                    if (event.getSource() == runButton) {
                        String uri = selectorPanel.textField.getText();

                        try {
                            XScript script = msp.getScript(uri);

                            Object[][] out = new Object[1][0];
                            out[0] = new Object[0];

                            short[][] num = new short[1][0];
                            num[0] = new short[0];

                            script.invoke(new Object[0], num, out);
                        }
                        catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    else if (event.getSource() == closeButton) {
                        client.dispose();
                    }
                    else if (event.getSource() == editButton) {
                        String uri = selectorPanel.textField.getText();

                        if (uri.indexOf("language=Basic") != -1) {
                            showBasicEditor(ctxt);
                        }
                        else {
                            DefaultMutableTreeNode node =
                              (DefaultMutableTreeNode)
                              selectorPanel.tree.getLastSelectedPathComponent();

                            if (node == null) return;

                            showEditor(ctxt, node);
                        }
                    }
                    else if (event.getSource() == assignButton) {
                    }
                    else if (event.getSource() == deleteButton) {
                    }
                }
            };

            runButton.addActionListener(listener);
            closeButton.addActionListener(listener);
            assignButton.addActionListener(listener);
            editButton.addActionListener(listener);
            deleteButton.addActionListener(listener);

            JPanel buttonPanel = new JPanel(new BorderLayout());
            buttonPanel.add(northButtons, BorderLayout.NORTH);
            buttonPanel.add(southButtons, BorderLayout.SOUTH);

            JPanel mainPanel = new JPanel(new BorderLayout(MED_GAP, MED_GAP));
            mainPanel.setBorder(
                new EmptyBorder(BIG_GAP, BIG_GAP, BIG_GAP, BIG_GAP));
            mainPanel.add(selectorPanel, BorderLayout.CENTER);
            mainPanel.add(buttonPanel, BorderLayout.EAST);

            client.getContentPane().add(mainPanel);
            client.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
            client.setSize(375, 350);

            client.show();
        }
        catch (com.sun.star.uno.RuntimeException rue) {
            rue.printStackTrace();
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
        }
    }

    private static void showEditor(
        XScriptContext ctxt, DefaultMutableTreeNode node)
    {
        Object obj = node.getUserObject();
        XInvocation inv =
            (XInvocation)UnoRuntime.queryInterface(
            XInvocation.class, obj);
        Object[] args = new Object[] { ctxt };
        try {
            inv.invoke("Editable", args,
                new short[0][0], new Object[0][0]);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void showBasicEditor(XScriptContext ctxt) {
        XComponentContext xcc = ctxt.getComponentContext();
        XModel doc = ctxt.getDocument();

        System.err.println("let's start the Basic IDE");

        try {
            XDispatchProvider xProvider = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class,
                    doc.getCurrentController().getFrame());

            Object obj = xcc.getServiceManager().createInstanceWithContext(
                "com.sun.star.frame.DispatchHelper", xcc);

            XDispatchHelper xDispatchHelper =
                (XDispatchHelper)UnoRuntime.queryInterface(
                    XDispatchHelper.class, obj);

            xDispatchHelper.executeDispatch(xProvider,
                ".uno:BasicIDEAppear", "", 0,
                new com.sun.star.beans.PropertyValue[0]);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void checkEnabled(XPropertySet props, String name,
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

    private static XBrowseNode getRootNode(XScriptContext ctxt) {

        XBrowseNode result = null;

        try {
            String serviceName = "drafts.com.sun.star.script." +
                "provider.MasterScriptProvider";

            XComponentContext xcc = ctxt.getComponentContext();
            XMultiComponentFactory xmcf = xcc.getServiceManager();

            Any[] args = new Any[1];
            args[0] = new Any(new Type(XModel.class), ctxt.getDocument());

            Object serviceObj = xmcf.createInstanceWithArgumentsAndContext(
                serviceName, args, xcc);

            result = (XBrowseNode)UnoRuntime.queryInterface(
                XBrowseNode.class, serviceObj);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }
}

class ScriptSelectorPanel extends JPanel {

    private XBrowseNode myrootnode = null;
    public JTextField textField;
    public JTree tree;

    public ScriptSelectorPanel(XBrowseNode root)
    {
        this.myrootnode = root;
        initUI();
    }

    public XBrowseNode getSelection() {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)
            tree.getLastSelectedPathComponent();

        if (node == null) {
            return null;
        }

        return (XBrowseNode)node.getUserObject();
    }

    private void initUI() {
        setLayout(new BorderLayout());

        DefaultMutableTreeNode top =
            new DefaultMutableTreeNode(myrootnode) {
                public String toString() {
                    return ((XBrowseNode)getUserObject()).getName();
                }
            };
        initNodes(myrootnode, top);
        tree = new JTree(top);

        tree.setCellRenderer(new ScriptTreeRenderer());

        tree.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                XBrowseNode xbn = getSelection();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xbn);

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
            }
        });

        JScrollPane scroller = new JScrollPane(tree);
        add(scroller, BorderLayout.CENTER);

        textField = new JTextField();
        add(textField, BorderLayout.SOUTH);
        setSize(600, 300);
    }

    private void initNodes(XBrowseNode parent, DefaultMutableTreeNode top) {
        if ( parent == null || parent.hasChildNodes() == false )
        {
            return;
        }

        XBrowseNode[] children = parent.getChildNodes();

        try {
            if (children != null) {
                for (int i = 0; i < children.length; i++) {
                    if ( children[i] == null )
                    {
                        continue;
                    }
                    DefaultMutableTreeNode newNode =
                        new DefaultMutableTreeNode(children[i]) {
                            public String toString() {
                                return ((XBrowseNode)getUserObject()).getName();
                            }
                        };
                    top.add(newNode);
                    initNodes(children[i], newNode);
                }
            }
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
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
