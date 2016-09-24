/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Exception;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

import com.sun.star.beans.*;
import com.sun.star.script.browse.XBrowseNode;
import com.sun.star.script.browse.BrowseNodeTypes;
import com.sun.star.script.browse.XBrowseNodeFactory;
import com.sun.star.script.browse.BrowseNodeFactoryViewTypes;
import com.sun.star.script.provider.XScriptContext;
import com.sun.star.script.provider.XScript;
import com.sun.star.script.provider.XScriptProvider;
import com.sun.star.script.provider.XScriptProviderFactory;

public class ScriptSelector {

    private static final int BIG_GAP = 10;
    private static final int MED_GAP = 5;

    private static final String MSP_FACTORY =
        "/singletons/com.sun.star.script.provider." +
        "theMasterScriptProviderFactory";

    private static final String BROWSE_FACTORY =
        "/singletons/com.sun.star.script.browse.theBrowseNodeFactory";

    private ScriptSelectorPanel selectorPanel;

    public void show(final XScriptContext ctxt)
    {
        try {
            XBrowseNode root = getRootNode(ctxt);

            Object obj = ctxt.getComponentContext().getValueByName(MSP_FACTORY);

            XScriptProviderFactory fac = UnoRuntime.queryInterface(XScriptProviderFactory.class, obj);

            final XScriptProvider msp =
                fac.createScriptProvider(new Any(new Type(), null));

            final JFrame client = new JFrame("Script");

            selectorPanel = new ScriptSelectorPanel(root);

            final JButton runButton, closeButton;

            runButton = new JButton("Run");
            runButton.setEnabled(false);

            closeButton = new JButton("Close");

            JPanel northButtons =
                new JPanel(new GridLayout(2, 1, MED_GAP, MED_GAP));

            northButtons.add(runButton);
            northButtons.add(closeButton);

            JPanel southButtons =
                new JPanel(new GridLayout(3, 1, MED_GAP, MED_GAP));

            selectorPanel.tree.addTreeSelectionListener(
                new TreeSelectionListener() {
                    public void valueChanged(TreeSelectionEvent e) {
                        XBrowseNode xbn = selectorPanel.getSelection();
                        XPropertySet props = UnoRuntime.queryInterface(XPropertySet.class, xbn);

                        if (xbn != null &&
                            xbn.getType() == BrowseNodeTypes.SCRIPT)
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
                }
            };

            runButton.addActionListener(listener);
            closeButton.addActionListener(listener);

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
        }
        catch (com.sun.star.uno.RuntimeException rue) {
            rue.printStackTrace();
        }
        catch (java.lang.Exception e) {
            e.printStackTrace();
        }
    }

    public void show(final XScriptContext ctxt,
        final com.sun.star.awt.MouseEvent e)
    {
        show(ctxt);
    }

    public void show(final XScriptContext ctxt,
        final com.sun.star.awt.ActionEvent e)
    {
        show(ctxt);
    }

    private XBrowseNode getRootNode(XScriptContext ctxt) {

        XBrowseNode result = null;

        XComponentContext xcc = ctxt.getComponentContext();
        XBrowseNodeFactory xBrowseFac = UnoRuntime.queryInterface(
            XBrowseNodeFactory.class, xcc.getValueByName(BROWSE_FACTORY));

        result = UnoRuntime.queryInterface(
           XBrowseNode.class, xBrowseFac.createView(
               BrowseNodeFactoryViewTypes.MACROSELECTOR ) );

        return result;
    }
}

class ScriptSelectorPanel extends JPanel {

    private final XBrowseNode myrootnode;
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
                @Override
                public String toString() {
                    return ((XBrowseNode)getUserObject()).getName();
                }
            };
        initNodes(myrootnode, top);
        DefaultTreeModel treeModel = new DefaultTreeModel(top);
        tree = new JTree(treeModel);

        tree.setCellRenderer(new ScriptTreeRenderer());

        tree.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);

        tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                XBrowseNode xbn = getSelection();
                XPropertySet props = UnoRuntime.queryInterface(
                    XPropertySet.class, xbn);

                if (xbn == null) {
                    textField.setText("");
                    return;
                }

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
    }





    private void initNodes(XBrowseNode parent, DefaultMutableTreeNode top) {
        if ( parent == null || !parent.hasChildNodes() )
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
                            @Override
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

    private final ImageIcon sofficeIcon;
    private final ImageIcon scriptIcon;
    private final ImageIcon containerIcon;

    public ScriptTreeRenderer() {
        sofficeIcon = new ImageIcon(getClass().getResource("soffice.gif"));
        scriptIcon = new ImageIcon(getClass().getResource("script.gif"));
        containerIcon = new ImageIcon(getClass().getResource("container.gif"));
    }

    @Override
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
