/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PlainSourceView.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.script.framework.provider.beanshell;

import javax.swing.JTextArea;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.text.Document;
import javax.swing.event.DocumentListener;
import javax.swing.event.DocumentEvent;

import java.awt.FlowLayout;
import java.awt.Graphics;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.Dimension;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

public class PlainSourceView extends JScrollPane
    implements ScriptSourceView, DocumentListener {

    private ScriptSourceModel model;
    private JTextArea ta;
    private GlyphGutter gg;
    private int linecount;
    private boolean isModified = false;

    public PlainSourceView(ScriptSourceModel model) {
        this.model = model;
        initUI();
        model.setView(this);
    }

    public void clear() {
        ta.setText("");
    }

    public void update() {
        /* Remove ourselves as a DocumentListener while loading the source
           so we don't get a storm of DocumentEvents during loading */
        ta.getDocument().removeDocumentListener(this);

        if (isModified == false)
        {
            int pos = ta.getCaretPosition();
            ta.setText(model.getText());
            try {
                ta.setCaretPosition(pos);
            }
            catch (IllegalArgumentException iae) {
                // do nothing and allow JTextArea to set it's own position
            }
        }

        // scroll to currentPosition of the model
        try {
            int line = ta.getLineStartOffset(model.getCurrentPosition());
            Rectangle rect = ta.modelToView(line);
            ta.scrollRectToVisible(rect);
        }
        catch (Exception e) {
            // couldn't scroll to line, do nothing
        }

        gg.repaint();

        // Add back the listener
        ta.getDocument().addDocumentListener(this);
    }

    public boolean isModified() {
        return isModified;
    }

    public void setModified(boolean value) {
        isModified = value;
    }

    private void initUI() {
        ta = new JTextArea();
        ta.setRows(15);
        ta.setColumns(40);
        ta.setLineWrap(false);
        ta.insert(model.getText(), 0);
        linecount = ta.getLineCount();

        gg = new GlyphGutter(this);

        setViewportView(ta);
        setRowHeaderView(gg);

        ta.getDocument().addDocumentListener(this);
    }

    /* Implementation of DocumentListener interface */
    public void insertUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void removeUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void changedUpdate(DocumentEvent e) {
        doChanged(e);
    }

    /* If the number of lines in the JTextArea has changed then update the
       GlyphGutter */
    public void doChanged(DocumentEvent e) {
        isModified = true;

        if (linecount != ta.getLineCount()) {
            gg.update();
            linecount = ta.getLineCount();
        }
    }

    public String getText() {
        return ta.getText();
    }

    public JTextArea getTextArea() {
        return ta;
    }

    public int getCurrentPosition() {
        return model.getCurrentPosition();
    }
}

class GlyphGutter extends JComponent {

    private PlainSourceView view;
    private final String DUMMY_STRING = "99";

    GlyphGutter(PlainSourceView view) {
        this.view = view;
        update();
    }

    public void update() {
        JTextArea textArea = view.getTextArea();
        Font font = textArea.getFont();
        setFont(font);

        FontMetrics metrics = getFontMetrics(font);
        int h = metrics.getHeight();
        int lineCount = textArea.getLineCount() + 1;

        String dummy = Integer.toString(lineCount);
        if (dummy.length() < 2) {
            dummy = DUMMY_STRING;
        }

        Dimension d = new Dimension();
        d.width = metrics.stringWidth(dummy) + 16;
        d.height = lineCount * h + 100;
        setPreferredSize(d);
        setSize(d);
    }

    public void paintComponent(Graphics g) {
        JTextArea textArea = view.getTextArea();

        Font font = textArea.getFont();
        g.setFont(font);

        FontMetrics metrics = getFontMetrics(font);
        Rectangle clip = g.getClipBounds();

        g.setColor(getBackground());
        g.fillRect(clip.x, clip.y, clip.width, clip.height);

        int ascent = metrics.getMaxAscent();
        int h = metrics.getHeight();
        int lineCount = textArea.getLineCount() + 1;

        int startLine = clip.y / h;
        int endLine = (clip.y + clip.height) / h + 1;
        int width = getWidth();
        if (endLine > lineCount) {
            endLine = lineCount;
        }

        for (int i = startLine; i < endLine; i++) {
            String text;
            text = Integer.toString(i + 1) + " ";
            int w = metrics.stringWidth(text);
            int y = i * h;
            g.setColor(Color.blue);
            g.drawString(text, 0, y + ascent);
            int x = width - ascent;

            // if currentPosition is not -1 then a red arrow will be drawn
            if (i == view.getCurrentPosition()) {
                drawArrow(g, ascent, x, y);
            }
        }
    }

    private void drawArrow(Graphics g, int ascent, int x, int y) {
        Polygon arrow = new Polygon();
        int dx = x;
        y += ascent - 10;
        int dy = y;
        arrow.addPoint(dx, dy + 3);
        arrow.addPoint(dx + 5, dy + 3);
        for (x = dx + 5; x <= dx + 10; x++, y++) {
            arrow.addPoint(x, y);
        }
        for (x = dx + 9; x >= dx + 5; x--, y++) {
            arrow.addPoint(x, y);
        }
        arrow.addPoint(dx + 5, dy + 7);
        arrow.addPoint(dx, dy + 7);

        g.setColor(Color.red);
        g.fillPolygon(arrow);
        g.setColor(Color.black);
        g.drawPolygon(arrow);
    }
};
