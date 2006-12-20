package org.openoffice.setup.Panel;

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupActionListener;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public class UninstallationCompleted extends JPanel {

    public static final String ACTION_DETAILS   = "ActionDetails";
    public static final int BUTTON_DETAILS  = 5;
    private JButton mDetailsButton;
    private PanelLabel varLabel;
    private PanelTitle titleBox;

    public UninstallationCompleted() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText = ResourceManager.getString("String_UninstallationCompleted1");
        titleBox = new PanelTitle(titleText);
        add(titleBox, BorderLayout.NORTH);

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new java.awt.BorderLayout());

        String dialogText = ResourceManager.getString("String_UninstallationCompleted2");
        varLabel = new PanelLabel(dialogText, true);
        String text2 = ResourceManager.getString("String_InstallationCompleted3");
        PanelLabel label2 = new PanelLabel(text2);

        mDetailsButton = new JButton();
        String buttonText = ResourceManager.getString("String_InstallationCompleted_Button");
        mDetailsButton.setText(buttonText);
        mDetailsButton.setEnabled(true);

        JPanel ButtonPanel = new JPanel();
        ButtonPanel.setLayout(new BorderLayout());
        ButtonPanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        ButtonPanel.setPreferredSize(new Dimension(120, 44));
        ButtonPanel.add(mDetailsButton, BorderLayout.NORTH);

        contentPanel.add(varLabel, BorderLayout.NORTH);
        contentPanel.add(ButtonPanel, BorderLayout.EAST);
        contentPanel.add(label2, BorderLayout.CENTER);

        add(contentPanel, BorderLayout.CENTER);
    }

    public void setDetailsButtonActionCommand(String actionCommand) {
        mDetailsButton.setActionCommand(actionCommand);
    }

    public void addDetailsButtonActionListener(SetupActionListener actionListener) {
        mDetailsButton.addActionListener(actionListener);
    }

    public void setTitleText(String s) {
        titleBox.setTitle(s);
    }

    public void setDialogText(String text) {
        varLabel.setText(text);
    }

}
