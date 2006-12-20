package org.openoffice.setup.Panel;

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Insets;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public class Prologue extends JPanel {

    public Prologue() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));

        String titleText    = ResourceManager.getString("String_Prologue1");
        PanelTitle titleBox = new PanelTitle(titleText);
        add(titleBox, BorderLayout.NORTH);


        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new java.awt.BorderLayout());

            String text1 = ResourceManager.getString("String_Prologue2");
            PanelLabel label1 = new PanelLabel(text1, true);
            String text2 = ResourceManager.getString("String_Prologue3");
            PanelLabel label2 = new PanelLabel(text2);

        contentPanel.add(label1, BorderLayout.NORTH);
        contentPanel.add(label2, BorderLayout.CENTER);

        add(contentPanel, BorderLayout.CENTER);
    }
}
