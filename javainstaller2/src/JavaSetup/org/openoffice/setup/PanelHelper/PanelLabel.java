package org.openoffice.setup.PanelHelper;

import java.awt.Color;
import java.awt.Font;
import javax.swing.JTextArea;
import javax.swing.UIManager;

public class PanelLabel extends JTextArea {

    static private Color BackgroundColor;
    static private Color TextColor;
    static private Font  TextFont;

    public PanelLabel() {
    }

    public PanelLabel(String text, int rows, int columns) {
        super(text, rows, columns);
        init(true);
    }

    public PanelLabel(String text, boolean multiline) {
        super(text);
        init(multiline);
    }

    public PanelLabel(String text) {
        super(text);
        init(false);
    }

    private void init(boolean multiline) {
        setEditable(false);
        setBackground(BackgroundColor);
        setForeground(TextColor);
        setFont(TextFont);

        if (multiline) {
            setLineWrap(true);
            setWrapStyleWord(true);
        }
    }

    static {
        BackgroundColor = (Color)UIManager.get("Label.background");
        TextColor       = (Color)UIManager.get("Label.foreground");
        TextFont        = ((Font)UIManager.get("Label.font")).deriveFont(Font.PLAIN);
    }
}
