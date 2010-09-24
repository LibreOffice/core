/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.netbeans.modules.office.actions;

import java.io.File;
import java.io.IOException;
import java.util.Hashtable;
import java.util.List;
import java.util.ArrayList;
import java.util.Enumeration;

import javax.swing.JMenuItem;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.awt.Actions;
import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.*;
import org.openide.awt.JMenuPlus;

import org.openoffice.idesupport.SVersionRCFile;
import org.openoffice.idesupport.OfficeInstallation;
import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.LocalOffice;

import org.openoffice.netbeans.modules.office.utils.NagDialog;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;

public class DeployParcelAction extends CookieAction implements Presenter.Popup {

    private static final String BROWSE_LABEL = "Office Document...";
    private static final String DEPLOY_LABEL = "Deploy To";

    public String getName () {
        return DEPLOY_LABEL;
    }

    public HelpCtx getHelpCtx () {
        return HelpCtx.DEFAULT_HELP;
    }

    public JMenuItem getPopupPresenter() {
        JMenuPlus menu = new JMenuPlus(DEPLOY_LABEL);
        JMenuItem item, user, share;
        final OfficeInstallation oi = OfficeSettings.getDefault().getOfficeDirectory();

        ActionListener listener = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                JMenuItem choice = (JMenuItem)e.getSource();
                String label = choice.getText();

                Node[] nodes = getActivatedNodes();
                final ParcelCookie parcelCookie =
                    (ParcelCookie)nodes[0].getCookie(ParcelCookie.class);

                File target = new File(oi.getPath(File.separator + label +
                    File.separator + "Scripts"));

                File langdir = new File(target, parcelCookie.getLanguage());

                if (!langdir.exists()) {
                    boolean response = askIfCreateDirectory(langdir);
                    if (response == false) {
                        return;
                    }
                }

                deploy(target);
            }
        };

        user = new JMenuItem("user");
        user.addActionListener(listener);

        share = new JMenuItem("share");
        share.addActionListener(listener);

        item = new JMenuPlus(oi.getName());
        item.add(user);
        item.add(share);
        menu.add(item);

        menu.addSeparator();
        item = new JMenuItem(BROWSE_LABEL);
        item.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                File target = getTargetFile();
                if (target == null)
                    return;
                deploy(target);
            }
        });
        menu.add(item);

        return menu;
    }

    protected int mode () {
        return MODE_ONE;
    }

    protected Class[] cookieClasses () {
        return new Class[] { ParcelCookie.class };
    }

    protected void performAction (Node[] activatedNodes) {
        // do nothing, should not happen
    }

    private void deploy(final File target) {
        Node[] nodes = getActivatedNodes();
        final ParcelCookie parcelCookie =
            (ParcelCookie)nodes[0].getCookie(ParcelCookie.class);

        RequestProcessor.getDefault().post(new Runnable() {
            public void run() {
                boolean result = parcelCookie.deploy(target);

                if (result == true && target.isDirectory()) {
                    showNagDialog();
                    // refreshOffice((String)versions.get(label));
                }
            }
        });
    }

    private boolean askIfCreateDirectory(File directory) {
        String message = directory.getAbsolutePath() + " does not exist. " +
            "Do you want to create it now?";

        NotifyDescriptor d = new NotifyDescriptor.Confirmation(
            message, NotifyDescriptor.OK_CANCEL_OPTION);
        TopManager.getDefault().notify(d);

        if (d.getValue() == NotifyDescriptor.CANCEL_OPTION)
            return false;

        boolean result;
        try {
            result = directory.mkdirs();
        }
        catch (SecurityException se) {
            result = false;
        }

        if (result == false) {
            String tmp = "Error creating: " + directory.getAbsolutePath();
            NotifyDescriptor d2 = new NotifyDescriptor.Message(
                tmp, NotifyDescriptor.ERROR_MESSAGE);
            TopManager.getDefault().notify(d2);
        }
        return result;
    }

    private void refreshOffice(String path) {
        ClassLoader syscl = TopManager.getDefault().currentClassLoader();
        LocalOffice office = LocalOffice.create(syscl, path, 8100);
        office.refreshStorage("file://" + path + "/program/../user");
        office.disconnect();
    }

    private void showNagDialog() {
        String message = "If you currently have Office running you will " +
            "need to click on the Tools/Scripting Add-on's/Refresh All Scripts " +
            " menu item in Office so that the scripts in this parcel can be detected.";

        OfficeSettings settings = OfficeSettings.getDefault();

        if (settings.getWarnAfterDirDeploy() == true) {
            NagDialog warning = NagDialog.createInformationDialog(
                message, "Show this message in future", true);

            warning.show();

            if (warning.getState() == false)
                settings.setWarnAfterDirDeploy(false);
        }
    }

    private File getTargetFile() {
        File target = null;

        JFileChooser chooser = new JFileChooser();
        chooser.setDialogTitle("Deploy Parcel To Office Document");
        chooser.setApproveButtonText("Deploy");
        chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
        chooser.setFileFilter(new FileFilter() {
            public boolean accept(File file) {
                if (file.isDirectory() ||
                    file.getName().endsWith(".sxw") ||
                    file.getName().endsWith(".sxc") ||
                    file.getName().endsWith(".sxd") ||
                    file.getName().endsWith(".sxi"))
                    return true;
                return false;
            }

            public String getDescription() {
                return "Office Documents";
            }
        });

        int result = chooser.showDialog(null, null);

        if (result == JFileChooser.APPROVE_OPTION) {
            target = chooser.getSelectedFile();
        }
        return target;
    }
}
