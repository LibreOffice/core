package org.openoffice.netbeans.modules.office.actions;

import java.awt.event.ActionEvent;
import java.io.*;
import java.util.*;
import javax.swing.JMenuItem;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.awt.Actions;
import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.*;

import org.openoffice.idesupport.SVersionRCFile;
import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.idesupport.LocalOffice;

import org.openoffice.netbeans.modules.office.utils.NagDialog;
import org.openoffice.netbeans.modules.office.options.OfficeSettings;

public class DeployParcelAction extends CookieAction implements Presenter.Popup {
    public String getName () {
        return "Deploy To";
    }

    public HelpCtx getHelpCtx () {
        return HelpCtx.DEFAULT_HELP;
    }

    public JMenuItem getPopupPresenter() {
        return new LocationsMenu (this, new LocationsModel (this), true);
    }

    protected int mode () {
        return MODE_ALL;
    }

    protected Class[] cookieClasses () {
        return new Class[] { ParcelCookie.class };
    }

    protected void performAction (Node[] activatedNodes) {
        // do nothing, should not happen
    }

    /** Special submenu which notifies model when it is added as a component.
    */
    private static final class LocationsMenu extends Actions.SubMenu {

        private final LocationsModel model;

        LocationsMenu (SystemAction action, LocationsModel model, boolean popup) {
            super (action, model, popup);
            this.model = model;
        }

        public void addNotify () {
            model.addNotify ();
            super.addNotify ();
        }
    }

    /** Model to use for the submenu.
    */
    private static final class LocationsModel implements Actions.SubMenuModel {
        private List labels = null;
        private Hashtable versions = null;
        private final NodeAction action;
        private String BROWSE_LABEL = "Office Document...";

        LocationsModel (NodeAction action) {
            this.action = action;
        }

        public int getCount () {
            if (labels == null)
                return 0;
            return labels.size();
        }

        public String getLabel (int index) {
            return (String)labels.get(index);
        }

        public HelpCtx getHelpCtx (int index) {
            return HelpCtx.DEFAULT_HELP; // NOI18N
        }

        public void performActionAt (final int index) {
            final String label = getLabel(index);
            final File source;
            final File target;

            if (label.equals(BROWSE_LABEL)) {
                target = getTargetFile();
                if (target == null)
                    return;
            }
            else {
                target = new File((String)versions.get(label) +
                    File.separator + "user" + File.separator + "Scripts" +
                    File.separator + "java");
                if (!target.exists()) {
                    boolean response = askIfCreateDirectory(target);
                    if (response == false) {
                        return;
                    }
                }
            }

            Node[] nodes = action.getActivatedNodes();
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

        void addNotify () {
            SVersionRCFile rcfile = new SVersionRCFile();

            try {
                versions = rcfile.getVersions();
            }
            catch (IOException ioe) {
                return;
            }

            Enumeration enum = versions.keys();
            labels = new ArrayList();

            while (enum.hasMoreElements()) {
                String s = (String)enum.nextElement();
                // System.out.println("Adding: " + s);
                labels.add(s);
            }

            labels.add(null);
            labels.add(BROWSE_LABEL);

            // IDE will not show the submenu if there is only one item
            // so add a blank item which will not be shown
            if (labels.size() == 1)
                labels.add(null);
        }

        private boolean askIfCreateDirectory(File directory) {
            String message = "Your Office installation does not have a " +
                "directory for scripts written in java. Do you want to " +
                "create one now?";

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
                "need to click on the Tools/Refresh Scripts(java) menu item " +
                "in Office so that the scripts in this parcel can be " +
                "detected.";

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

        public synchronized void addChangeListener (ChangeListener l) {
        }

        public synchronized void removeChangeListener (ChangeListener l) {
        }
    }
}
