package org.openoffice.idesupport.ui;

import java.io.File;
import java.util.Vector;
import java.util.Enumeration;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.AbstractButton;
import javax.swing.ImageIcon;
import javax.swing.border.LineBorder;

import java.awt.BorderLayout;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import org.w3c.dom.Document;

import org.openoffice.idesupport.xml.ParcelDescriptor;
import org.openoffice.idesupport.zip.ParcelZipper;

public class ConfigurePanel extends JPanel {

    private File basedir;
    private Vector classpath;

    private MethodPanel methodPanel;
    private ScriptPanel scriptPanel;
    private ParcelDescriptor descriptor = ParcelDescriptor.getParcelDescriptor();

    public ConfigurePanel(String basedir, Vector classpath, Document doc) {
        this.basedir = new File(basedir);
        this.classpath = classpath;
        initUI(doc);
    }

    public Document getConfiguration() throws Exception {
        Enumeration scripts = scriptPanel.getScriptEntries();
        return descriptor.generate(scripts);
    }

    private void initUI(Document doc) {
        JPanel leftPanel = new JPanel();
        JPanel methodButtons = initMethodButtons();
        methodPanel = new MethodPanel(basedir, classpath);
        leftPanel.setLayout(new BorderLayout());
        leftPanel.add(methodPanel, BorderLayout.CENTER);

        JPanel rightPanel = new JPanel();
        JPanel scriptButtons = initScriptButtons();
        scriptPanel = new ScriptPanel(descriptor.parse(doc));
        rightPanel.setLayout(new BorderLayout());
        rightPanel.add(scriptPanel, BorderLayout.CENTER);
        rightPanel.add(scriptButtons, BorderLayout.SOUTH);

        setLayout(new GridBagLayout());
        setPreferredSize(new java.awt.Dimension(700, 300));
        setBorder(LineBorder.createBlackLineBorder());

        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.ipadx = 40;
        gbc.anchor = java.awt.GridBagConstraints.WEST;
        gbc.insets = new Insets(10, 5, 5, 5);
        gbc.weightx = 0.75;
        add(leftPanel, gbc);

        gbc = new java.awt.GridBagConstraints();
        gbc.gridx = 1;
        gbc.gridy = 0;
        add(methodButtons, gbc);

        gbc = new java.awt.GridBagConstraints();
        gbc.gridx = 2;
        gbc.gridy = 0;
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.anchor = java.awt.GridBagConstraints.EAST;
        gbc.insets = new Insets(10, 5, 5, 5);
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        add(rightPanel, gbc);
    }

    private JPanel initMethodButtons() {
        JPanel panel = new JPanel();
        panel.setLayout(new GridBagLayout());
        ImageIcon icon = new ImageIcon(getClass().getResource("/org/openoffice/idesupport/ui/add.gif"));
        JButton addButton = new JButton("Add", icon);
        addButton.setHorizontalTextPosition(AbstractButton.LEFT);

        addButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.addScriptEntries(methodPanel.getSelectedEntries());
                }
            }
        );

        GridBagConstraints gbc = new java.awt.GridBagConstraints();
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(5, 5, 5, 5);
        panel.add(addButton, gbc);

        JPanel dummyPanel = new JPanel();
        gbc = new java.awt.GridBagConstraints();
        gbc.gridwidth = java.awt.GridBagConstraints.REMAINDER;
        gbc.gridheight = java.awt.GridBagConstraints.REMAINDER;
        gbc.fill = java.awt.GridBagConstraints.BOTH;
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        panel.add(dummyPanel, gbc);

        return panel;
    }

    private JPanel initScriptButtons() {
        JPanel panel = new JPanel();
        JButton removeButton = new JButton("Remove");
        JButton removeAllButton = new JButton("Remove All");

        removeButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.removeSelectedRows();
                }
            }
        );

        removeAllButton.addActionListener(
            new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    scriptPanel.removeAllRows();
                }
            }
        );

        panel.add(removeButton);
        panel.add(removeAllButton);

        return panel;
    }
}
