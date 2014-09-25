/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.libreoffice.example.java_scripts;

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

import com.sun.star.script.browse.XBrowseNode;
import com.sun.star.script.browse.BrowseNodeTypes;
import com.sun.star.script.browse.XBrowseNodeFactory;
import com.sun.star.script.browse.BrowseNodeFactoryViewTypes;
import com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.provider.XScript;
import com.sun.star.script.provider.XScriptProvider;

public class ScriptSelector {

    private static final int BIG_GAP = 10;
    private static final int MED_GAP = 5;

    private ScriptSelectorPanel selectorPanel;

    public ScriptSelector() {
    }

    public void showOrganizer(final XScriptContext ctxt) {
        try {
            XBrowseNode root = getRootNode(ctxt);

            final XScriptProvider msp =
                (XScriptProvider)UnoRuntime.queryInterface(
                    XScriptProvider.class, root);

            final JFrame client = new JFrame("Script");

            selectorPanel = new ScriptSelectorPanel(root);

            final JButton runButton, closeButton, createButton,
                  editButton, deleteButton;

            runButton = new JButton("Run");
            runButton.setEnabled(false);

            closeButton = new JButton("Close");

            editButton = new JButton("Edit");
            editButton.setEnabled(false);

            JPanel northButtons =
                new JPanel(new GridLayout(2, 1, MED_GAP, MED_GAP));

            northButtons.add(runButton);
            northButtons.add(closeButton);

            createButton = new JButton("Create");
            createButton.setEnabled(false);

            deleteButton = new JButton("Delete");
            deleteButton.setEnabled(false);

            JPanel southButtons =
                new JPanel(new GridLayout(3, 1, MED_GAP, MED_GAP));

            southButtons.add(editButton);
            southButtons.add(createButton);
            southButtons.add(deleteButton);

            selectorPanel.tree.addTreeSelectionListener(
            new TreeSelectionListener() {
                public void valueChanged(TreeSelectionEvent e) {
                    XBrowseNode xbn = selectorPanel.getSelection();
                    XPropertySet props = (XPropertySet)
                                         UnoRuntime.queryInterface(XPropertySet.class, xbn);

                    checkEnabled(props, "Creatable", createButton);
                    checkEnabled(props, "Deletable", deleteButton);
                    checkEnabled(props, "Editable", editButton);

                    if (xbn != null &&
                        xbn.getType() == BrowseNodeTypes.SCRIPT) {
                        runButton.setEnabled(true);
                    } else {
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
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    } else if (event.getSource() == closeButton) {
                        client.dispose();
                    } else if (event.getSource() == editButton) {
                        DefaultMutableTreeNode node =
                            (DefaultMutableTreeNode)
                            selectorPanel.tree.getLastSelectedPathComponent();

                        if (node == null) return;

                        showEditor(ctxt, node);
                    } else if (event.getSource() == createButton) {
                        DefaultMutableTreeNode node =
                            (DefaultMutableTreeNode)
                            selectorPanel.tree.getLastSelectedPathComponent();

                        if (node == null) return;

                        doCreate(ctxt, node);
                    } else if (event.getSource() == deleteButton) {
                        DefaultMutableTreeNode node =
                            (DefaultMutableTreeNode)
                            selectorPanel.tree.getLastSelectedPathComponent();

                        if (node == null) return;

                        doDelete(ctxt, node);
                    }
                }
            };

            runButton.addActionListener(listener);
            closeButton.addActionListener(listener);
            createButton.addActionListener(listener);
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
            client.setSize(500, 350);

            // set the x and y locations so that the frame is in the
            // centre of the screen
            Dimension d = client.getToolkit().getScreenSize();

            int x = (int)((d.getWidth() - client.getWidth()) / 2);
            int y = (int)((d.getHeight() - client.getHeight()) / 2);

            client.setLocation(x, y);

            client.show();
        } catch (com.sun.star.uno.RuntimeException rue) {
            rue.printStackTrace();
        } catch (java.lang.Exception e) {
            e.printStackTrace();
        }
    }

    public void showOrganizer(final XScriptContext ctxt,
                              final com.sun.star.awt.MouseEvent e) {
        showOrganizer(ctxt);
    }

    public void showOrganizer(final XScriptContext ctxt,
                              final com.sun.star.awt.ActionEvent e) {
        showOrganizer(ctxt);
    }

    private void doDelete(
        XScriptContext ctxt, DefaultMutableTreeNode node) {
        Object obj = node.getUserObject();
        XInvocation inv =
            (XInvocation)UnoRuntime.queryInterface(
                XInvocation.class, obj);
        Object[] args = new Object[] { ctxt };

        try {
            Object result = inv.invoke("Deletable", args,
                                       new short[1][0], new Object[1][0]);

            if (result != null && AnyConverter.toBoolean(result) == true) {
                selectorPanel.removeNode(node);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void doCreate(
        XScriptContext ctxt, DefaultMutableTreeNode node) {
        Object obj = node.getUserObject();
        XInvocation inv =
            (XInvocation)UnoRuntime.queryInterface(
                XInvocation.class, obj);
        Object[] args = new Object[] { ctxt };

        try {
            Object result = inv.invoke("Creatable", args,
                                       new short[1][0], new Object[1][0]);

            if (result != null) {
                XBrowseNode xbn = (XBrowseNode)
                                  AnyConverter.toObject(new Type(XBrowseNode.class), result);
                selectorPanel.addNode(node, xbn);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void showEditor(
        XScriptContext ctxt, DefaultMutableTreeNode node) {
        Object obj = node.getUserObject();
        XInvocation inv =
            (XInvocation)UnoRuntime.queryInterface(
                XInvocation.class, obj);
        Object[] args = new Object[] { ctxt };

        try {
            inv.invoke("Editable", args,
                       new short[1][0], new Object[1][0]);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void checkEnabled(XPropertySet props, String name,
                              JButton button) {
        boolean enable = false;

        try {
            if (props != null) {
                Object o = props.getPropertyValue(name);
                enable = AnyConverter.toBoolean(
                             props.getPropertyValue(name));
            }
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            // leave enable set to false
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            // leave enable set to false
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            // leave enable set to false
        }

        button.setEnabled(enable);
    }

    private XBrowseNode getRootNode(XScriptContext ctxt) {

        XBrowseNode result = null;


        XComponentContext xcc = ctxt.getComponentContext();
        XMultiComponentFactory xmcf = xcc.getServiceManager();
        XBrowseNodeFactory xBrowseFac = (XBrowseNodeFactory)
                                        UnoRuntime.queryInterface(XBrowseNodeFactory.class, xcc.getValueByName(
                                                "/singletons/com.sun.star.script.browse.theBrowseNodeFactory"));


        result = (XBrowseNode)UnoRuntime.queryInterface(
                     XBrowseNode.class, xBrowseFac.createView(
                         BrowseNodeFactoryViewTypes.MACROORGANIZER));
        return result;
    }
}

class ScriptSelectorPanel extends JPanel {

    private XBrowseNode myrootnode = null;
    public JTextField textField;
    public JTree tree;
    public DefaultTreeModel treeModel;

    public ScriptSelectorPanel(XBrowseNode root) {
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
        treeModel = new DefaultTreeModel(top);
        tree = new JTree(treeModel);

        tree.setCellRenderer(new ScriptTreeRenderer());

        tree.getSelectionModel().setSelectionMode
        (TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                XBrowseNode xbn = getSelection();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(
                                         XPropertySet.class, xbn);

                if (xbn == null) {
                    textField.setText("");
                    return;
                }

                String str = xbn.getName();

                if (xbn.getType() == BrowseNodeTypes.SCRIPT && props != null) {
                    try {
                        str = AnyConverter.toString(
                                  props.getPropertyValue("URI"));
                    } catch (Exception ignore) {
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
    }

    public void removeNode(DefaultMutableTreeNode node) {
        MutableTreeNode parent = (MutableTreeNode)(node.getParent());

        if (parent != null) {
            treeModel.removeNodeFromParent(node);
        }
    }

    public void addNode(DefaultMutableTreeNode parent, XBrowseNode xbn) {
        DefaultMutableTreeNode newNode =
        new DefaultMutableTreeNode(xbn) {
            public String toString() {
                return ((XBrowseNode)getUserObject()).getName();
            }
        };

        treeModel.insertNodeInto(newNode, parent, parent.getChildCount());
        tree.scrollPathToVisible(new TreePath(newNode.getPath()));
    }

    private void initNodes(XBrowseNode parent, DefaultMutableTreeNode top) {
        if (parent == null || parent.hasChildNodes() == false) {
            return;
        }

        XBrowseNode[] children = parent.getChildNodes();

        try {
            if (children != null) {
                for (int i = 0; i < children.length; i++) {
                    if (children[i] == null) {
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
        } catch (java.lang.Exception e) {
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
        } else if (xbn.getType() == BrowseNodeTypes.CONTAINER) {
            setIcon(containerIcon);
        } else if (xbn.getType() == BrowseNodeTypes.ROOT) {
            setIcon(sofficeIcon);
        }

        return this;
    }
}
