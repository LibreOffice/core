package org.openoffice.setup;

import javax.swing.JPanel;

public abstract class PanelController {

    private SetupFrame frame;
    private JPanel panel;
    private String name;
    private String next;
    private String prev;

    private PanelController () {
    }

    public PanelController (String name, JPanel panel) {
        this.name  = name;
        this.panel = panel;
    }

    public final JPanel getPanel () {
        return this.panel;
    }

    public final void setPanel (JPanel panel) {
        this.panel = panel;
    }

    public final String getName () {
        return this.name;
    }

    public final void setName (String name) {
        this.name = name;
    }

    final void setSetupFrame (SetupFrame frame) {
        this.frame = frame;
    }

    public final SetupFrame getSetupFrame () {
        return this.frame;
    }

    public String getNext () {
        return null;
    }

    public String getDialogText () {
        return null;
    }

    public String getPrevious () {
        return null;
    }

    public void beforeShow () {
    }

    public void duringShow () {
    }

    public boolean afterShow (boolean nextButtonPressed) {
        boolean repeatDialog = false;
        return repeatDialog;
    }

    public abstract String getHelpFileName();

}
