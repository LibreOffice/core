/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JToolBar;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.table.DefaultTableModel;

import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;
import org.openoffice.test.vcl.client.VclHook;
import org.openoffice.test.vcl.client.CommandCaller.WinInfoReceiver;

/**
 * Use the application to inspect vcl widgets in the openoffice.
 *
 */
public class VclInspector extends JFrame implements WinInfoReceiver {

    /**
     *
     */
    private static final long serialVersionUID = 1L;

    private JTable winInfoEditor = null;

    public VclInspector(){
        super("VCL Inspector");
    }


    private void init() {
        try {
            UIManager.getInstalledLookAndFeels();
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e1) {
        }

        setSize(800, 600);

        JToolBar toolBar = new JToolBar("Tools");
        JButton button = new JButton("Inspect");
        button.setToolTipText("Inspect VCL controls.");
        button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                inspect();
              }
        });
        toolBar.add(button);

        add(toolBar, BorderLayout.PAGE_START);
        String col[] = {"Type", "ID", "ToolTip"};
        DefaultTableModel model = new DefaultTableModel(null ,col);
        winInfoEditor = new JTable(model);
        winInfoEditor.getColumnModel().getColumn(0).setMaxWidth(60);
        winInfoEditor.getColumnModel().getColumn(1).setMaxWidth(100);
        JScrollPane pane1 = new JScrollPane(winInfoEditor);

        add(pane1, BorderLayout.CENTER);

        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                VclHook.getCommunicationManager().stop();
                System.exit(0);
            };

        });
    }

    public void open() {
        init();
        setVisible(true);
    }



    public void addWinInfo(final SmartId id, final long type, final String t) {
        final String tooltip = t.replaceAll("%.*%.*:", "");
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                ((DefaultTableModel)winInfoEditor.getModel()).addRow(new Object[]{type, id, tooltip});
            }
        });

    }

    public void onStartReceiving() {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                ((DefaultTableModel)winInfoEditor.getModel()).setRowCount(0);
            }
        });
    }


    public void inspect() {
        VclHook.invokeCommand(Constant.RC_DisplayHid, new Object[]{Boolean.TRUE});
    }

    public static void main(String[] args) {
        VclInspector inspector = new VclInspector();
        VclHook.getCommandCaller().setWinInfoReceiver(inspector);
        inspector.open();
    }


    public void onFinishReceiving() {

    }
}
