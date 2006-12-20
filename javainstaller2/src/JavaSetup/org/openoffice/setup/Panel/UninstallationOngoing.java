package org.openoffice.setup.Panel;

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupActionListener;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Insets;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.border.EmptyBorder;

public class UninstallationOngoing extends JPanel {

    private PanelLabel currentProgress;
    private JProgressBar progressBar;
    private JButton mStopButton;

    public UninstallationOngoing() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText = ResourceManager.getString("String_UninstallationOngoing1");
        PanelTitle titlebox = new PanelTitle(titleText);
        titlebox.addVerticalStrut(20);
        add(titlebox, BorderLayout.NORTH);

        Container contentbox = Box.createVerticalBox();

        // String progressText = ResourceManager.getString("String_UninstallationOngoing2");
        String progressText = "";
        currentProgress = new PanelLabel(progressText);

        Container innerbox = Box.createHorizontalBox();

        progressBar = new JProgressBar(0, 100);
        mStopButton = new JButton();
        String progressButtonText = ResourceManager.getString("String_InstallationOngoing3");
        mStopButton.setText(progressButtonText);
        mStopButton.setEnabled(true);

        innerbox.add(progressBar);
        innerbox.add(Box.createHorizontalStrut(10));
        innerbox.add(mStopButton);

        contentbox.add(currentProgress);
        contentbox.add(Box.createVerticalStrut(10));
        contentbox.add(innerbox);
        contentbox.add(Box.createVerticalStrut(20));

        add(contentbox, BorderLayout.SOUTH);
    }

    public void setProgressText(String s) {
        currentProgress.setText(s);
    }

    public void setProgressValue(int i) {
        progressBar.setValue(i);
    }

    public void setStopButtonActionCommand(String actionCommand) {
        mStopButton.setActionCommand(actionCommand);
    }

    public void addStopButtonActionListener(SetupActionListener actionListener) {
        mStopButton.addActionListener(actionListener);
    }

}
