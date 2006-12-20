package org.openoffice.setup.PanelHelper;

import java.awt.FlowLayout;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;

public class PanelTitle extends Box {

    private JLabel TitleLabel;

    public PanelTitle() {
         super(BoxLayout.PAGE_AXIS);
    }

    public PanelTitle(String title, String subtitle, int rows, int columns) {
        super(BoxLayout.PAGE_AXIS);
        init(title, subtitle, rows, columns);
    }

    public PanelTitle(String title, String subtitle) {
        super(BoxLayout.PAGE_AXIS);
        init(title, subtitle, 0, 0);
    }

    public PanelTitle(String title) {
        super (BoxLayout.PAGE_AXIS);
        init(title, null, 0, 0);
    }

    public void addVerticalStrut(int strut) {
        add(createVerticalStrut(strut));
    }

    public void setTitle(String title) {
        TitleLabel.setText(title);
    }

    // public void setSubtitle(String subtitle) {
    //     SubtitleLabel.setText(subtitle);
    // }

    private void init(String title, String subtitle, int rows, int columns) {

        TitleLabel = new JLabel(title);
        JPanel TitlePanel = new JPanel();
        TitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
        TitlePanel.add(TitleLabel);

        add(createVerticalStrut(10));
        add(TitlePanel);
        add(createVerticalStrut(10));
        add(new JSeparator());
        add(createVerticalStrut(20));

        if (subtitle != null) {
            PanelLabel SubtitleLabel = null;
            if ( rows > 0 ) {
                SubtitleLabel = new PanelLabel(subtitle, rows, columns );
            } else {
                SubtitleLabel = new PanelLabel(subtitle);
            }
            // PanelLabel SubtitleLabel = new PanelLabel(subtitle, true);
            JPanel    SubtitlePanel = new JPanel();
            SubtitlePanel.setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
            SubtitlePanel.add(SubtitleLabel);

            add(SubtitlePanel);
        }
    }
}