package org.openoffice.setup.Panel;

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.io.File;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.border.EmptyBorder;

public class UninstallationImminent extends JPanel {

    private String infoText;
    private JEditorPane ProductInformation;

    public UninstallationImminent() {
        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titletext = ResourceManager.getString("String_UninstallationImminent1");
        PanelTitle titlebox = new PanelTitle(titletext);
        add(titlebox, BorderLayout.NORTH);

        JPanel contentpanel = new JPanel();
        contentpanel.setLayout(new java.awt.BorderLayout());

        String text1 = ResourceManager.getString("String_UninstallationImminent2");
        PanelLabel label1 = new PanelLabel(text1);

        ProductInformation = new JEditorPane("text/html", getInfoText());
        ProductInformation.setEditable(false);

        JScrollPane ProductPane = new JScrollPane(ProductInformation);
        ProductPane.setPreferredSize(new Dimension(250, 145));
        ProductPane.setBorder(new EmptyBorder(10, 0, 10, 0));

        contentpanel.add(label1, BorderLayout.NORTH);
        contentpanel.add(ProductPane, BorderLayout.CENTER);

        add(contentpanel, BorderLayout.CENTER);
    }

    public void setInfoText(String text) {
        infoText = text;
        updateInfoText();
    }

    public String getInfoText() {
        return infoText;
    }

    public void updateInfoText() {
        ProductInformation.setText(infoText);
    }

}
