/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.script.framework.provider.beanshell;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.InputEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.KeyStroke;
import javax.swing.UIManager;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.text.BadLocationException;
import javax.swing.undo.CompoundEdit;
import javax.swing.undo.UndoManager;
import java.util.List;
import java.util.ArrayList;

public class PlainSourceView extends JScrollPane implements
    ScriptSourceView, DocumentListener {

    private final ScriptSourceModel model;
    private JTextArea ta;
    private GlyphGutter gg;
    private int linecount;
    private boolean isModified = false;
    private static final String undoKey = "Undo";
    private static final String redoKey = "Redo";
    private CompoundEdit compoundEdit = null;
    private static final int noLimit = -1;
    UndoManager undoManager;
    private List<UnsavedChangesListener> unsavedListener = new ArrayList<UnsavedChangesListener>();

    public PlainSourceView(ScriptSourceModel model) {
        this.model = model;
        initUI();
        model.setView(this);
    }

    public void undo(){
        if(compoundEdit!=null){
            compoundEdit.end();
            undoManager.addEdit(compoundEdit);
            compoundEdit = null;
        }
        if(undoManager.canUndo()){
            undoManager.undo();
        }
        // check if it's the last undoable change
        if(undoManager.canUndo() == false){
            setModified(false);
        }
    }
    public void redo(){
        if(undoManager.canRedo()){
            undoManager.redo();
        }
    }
    public void clear() {
        ta.setText("");
    }

    public void update() {
        /* Remove ourselves as a DocumentListener while loading the source
           so we don't get a storm of DocumentEvents during loading */
        ta.getDocument().removeDocumentListener(this);

        if (!isModified) {
            int pos = ta.getCaretPosition();
            ta.setText(model.getText());

            try {
                ta.setCaretPosition(pos);
            } catch (IllegalArgumentException iae) {
                // do nothing and allow JTextArea to set its own position
            }
        }

        // scroll to currentPosition of the model
        try {
            int line = ta.getLineStartOffset(model.getCurrentPosition());
            Rectangle rect = ta.modelToView(line);
            if (rect != null) {
                ta.scrollRectToVisible(rect);
            }
        } catch (BadLocationException e) {
            // couldn't scroll to line, do nothing
        }

        gg.repaint();

        // Add back the listener
        ta.getDocument().addDocumentListener(this);
    }

    public boolean isModified() {
        return isModified;
    }

    private void notifyListeners (boolean isUnsaved) {
        for (UnsavedChangesListener listener : unsavedListener) {
            listener.onUnsavedChanges(isUnsaved);
        }
    }

    public void setModified(boolean value) {
        if(value != isModified) {
            notifyListeners(value);
            isModified = value;
        }
    }

    private void initUI() {
        try{
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        }
        catch(Exception e){
                // What to do here
        }
        ta = new JTextArea();
        ta.setTabSize(4);
        ta.setRows(15);
        ta.setColumns(40);
        ta.setLineWrap(false);
        ta.insert(model.getText(), 0);
        ta.setFont(new Font("Monospaced", ta.getFont().getStyle(), ta.getFont().getSize()));
        undoManager = new UndoManager();
        undoManager.setLimit(noLimit);
        ta.getDocument().addUndoableEditListener(new UndoableEditListener(){
            @Override
            public void undoableEditHappened(UndoableEditEvent editEvent) {
                if(compoundEdit == null){
                    compoundEdit = new CompoundEdit();
                }
                compoundEdit.addEdit(editEvent.getEdit());
            }
        });

        ta.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_Z, InputEvent.CTRL_MASK), undoKey);
        ta.getInputMap().put(KeyStroke.getKeyStroke(KeyEvent.VK_Y, InputEvent.CTRL_MASK), redoKey);

        ta.addKeyListener(new KeyAdapter(){
            @Override
            public void keyReleased(KeyEvent ke){
                if(ke.getKeyCode() == KeyEvent.VK_SPACE || ke.getKeyCode() == KeyEvent.VK_ENTER){
                    compoundEdit.end();
                    undoManager.addEdit(compoundEdit);
                    compoundEdit = null;
                }
            }
        });

        ta.getActionMap().put(undoKey, new AbstractAction(undoKey){
            @Override
            public void actionPerformed(ActionEvent event) {
                undo();
            }
        });

        ta.getActionMap().put(redoKey, new AbstractAction(redoKey){
            @Override
            public void actionPerformed(ActionEvent event) {
                redo();
            }
        });

        linecount = ta.getLineCount();

        gg = new GlyphGutter(this);

        setViewportView(ta);
        setRowHeaderView(gg);

        ta.getDocument().addDocumentListener(this);
    }

    /* Implementation of DocumentListener interface */
    public void insertUpdate(DocumentEvent e) {
        doChanged();
    }

    public void removeUpdate(DocumentEvent e) {
        doChanged();
    }

    public void changedUpdate(DocumentEvent e) {
        doChanged();
    }

    /* If the number of lines in the JTextArea has changed then update the
       GlyphGutter */
    private void doChanged() {
        setModified(true);

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

    public void addListener(UnsavedChangesListener toAdd) {
        unsavedListener.add(toAdd);
    }
}

class GlyphGutter extends JComponent {

    private final PlainSourceView view;
    private static final String DUMMY_STRING = "99";

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

    @Override
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
}

