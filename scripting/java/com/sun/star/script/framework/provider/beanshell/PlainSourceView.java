/*************************************************************************
*
*  $RCSfile: PlainSourceView.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: rt $ $Date: 2004-01-05 13:13:49 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
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

        if (model.isModified()) {
            if (this.isModified()) {
                // ouch, contention, prompt for what to do
            }
            else {
                ta.insert(model.getText(), 0);
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
        return this.isModified;
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
        this.isModified = true;

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
