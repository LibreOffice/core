package com.sun.star.script.framework.provider.beanshell;

import javax.swing.JFrame;
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

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import drafts.com.sun.star.script.framework.runtime.XScriptContext;
import bsh.Interpreter;

public class ScriptEditorForBeanShell
    implements ActionListener, DocumentListener {

    private JFrame frame;
    private JTextArea ta;
    private GlyphGutter gg;
    private XScriptContext context;
    private int currentPosition = -1;
    private int linecount;
    private Interpreter sessionInterpreter;
    private Thread execThread = null;
    private String filename = null;

    public void edit(XScriptContext context, String path) {
        go(context, path);
    }

    /* Entry point for script execution */
    public void go(XScriptContext context, String filename) {
        if (filename != null && filename != "") {
            try {
                FileInputStream fis = new FileInputStream(filename);
                this.filename = filename;
                go(context, fis);
            }
            catch (IOException ioe) {
                JOptionPane.showMessageDialog(frame,
                    "Error loading file: " + ioe.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    /* Entry point for script execution */
    public void go(XScriptContext context, InputStream in) {
        this.context = context;
        initUI();

        if (in != null) {
            try {
                loadFile(in);
            }
            catch (IOException ioe) {
                JOptionPane.showMessageDialog(frame,
                    "Error loading stream: " + ioe.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    public void loadFile(InputStream in) throws IOException {

        /* Remove ourselves as a DocumentListener while loading the file
           so we don't get a storm of DocumentEvents during loading */
        ta.getDocument().removeDocumentListener(this);

        byte[] contents = new byte[1024];
        int len = 0, pos = 0;

        while ((len = in.read(contents, 0, 1024)) != -1) {
            ta.insert(new String(contents, 0, len), pos);
            pos += len;
        }

        try {
            in.close();
        }
        catch (IOException ignore) {
        }

        /* Update the GlyphGutter and add back the DocumentListener */
        gg.update();
        ta.getDocument().addDocumentListener(this);
    }

    private void initUI() {
        frame = new JFrame("BeanShell Debug Window");
        ta = new JTextArea();
        ta.setRows(15);
        ta.setColumns(40);
        ta.setLineWrap(false);
        linecount = ta.getLineCount();

        gg = new GlyphGutter(this);

        final JScrollPane sp = new JScrollPane();
        sp.setViewportView(ta);
        sp.setRowHeaderView(gg);

        ta.getDocument().addDocumentListener(this);
        String[] labels = {"Run", "Clear", "Save", "Close"};
        JPanel p = new JPanel();
        p.setLayout(new FlowLayout());

        for (int i = 0; i < labels.length; i++) {
            JButton b = new JButton(labels[i]);
            b.addActionListener(this);
            p.add(b);

            if (labels[i].equals("Save") && filename == null) {
                b.setEnabled(false);
            }
        }

        frame.getContentPane().add(sp, "Center");
        frame.getContentPane().add(p, "South");
        frame.pack();
        frame.show();
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
        if (linecount != ta.getLineCount()) {
            gg.update();
            linecount = ta.getLineCount();
        }
    }

    private void startExecution() {
        execThread = new Thread() {
            public void run() {
                Interpreter interpreter = new Interpreter();
                interpreter.getNameSpace().clear();

                // reset position and repaint gutter so no red arrow appears
                currentPosition = -1;
                gg.repaint();

                try {
                    interpreter.set("context", context);
                    interpreter.eval(ta.getText());
                }
                catch (bsh.EvalError err) {
                    currentPosition = err.getErrorLineNumber() - 1;
                    try {
                        // scroll to line of the error
                        int line = ta.getLineStartOffset(currentPosition);
                        Rectangle rect = ta.modelToView(line);
                        ta.scrollRectToVisible(rect);
                    }
                    catch (Exception e) {
                        // couldn't scroll to line, do nothing
                    }
                    gg.repaint();

                    JOptionPane.showMessageDialog(frame, "Error at line " +
                        String.valueOf(err.getErrorLineNumber()) +
                        "\n\n: " + err.getErrorText(),
                        "Error", JOptionPane.ERROR_MESSAGE);
                }
                catch (Exception e) {
                    JOptionPane.showMessageDialog(frame,
                        "Error: " + e.getMessage(),
                        "Error", JOptionPane.ERROR_MESSAGE);
                }
            }
        };
        execThread.start();
    }

    private void promptForSaveName() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new javax.swing.filechooser.FileFilter() {
            public boolean accept(File f) {
                if (f.isDirectory() || f.getName().endsWith(".bsh")) {
                    return true;
                }
                return false;
            }

            public String getDescription() {
                return ("BeanShell files: *.bsh");
            }
        });

        int ret = chooser.showSaveDialog(frame);

        if (ret == JFileChooser.APPROVE_OPTION) {
            filename = chooser.getSelectedFile().getAbsolutePath();
            if (!filename.endsWith(".bsh")) {
                filename += ".bsh";
            }
        }

    }

    private void saveTextArea() {
        if (filename == null) {
            promptForSaveName();
        }

        FileOutputStream fos = null;
        if (filename != null) {
            try {
                File f = new File(filename);
                fos = new FileOutputStream(f);
                String s = ta.getText();
                fos.write(s.getBytes(), 0, s.length());
            }
            catch (IOException ioe) {
                JOptionPane.showMessageDialog(frame,
                    "Error saving file: " + ioe.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
            finally {
                if (fos != null) {
                    try {
                        fos.close();
                    }
                    catch (IOException ignore) {
                    }
                }
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand().equals("Run")) {
            startExecution();
        }
        else if (e.getActionCommand().equals("Close")) {
            frame.dispose();
        }
        else if (e.getActionCommand().equals("Save")) {
            saveTextArea();
        }
        else if (e.getActionCommand().equals("Clear")) {
            ta.setText("");
        }
    }

    public JTextArea getTextArea() {
        return ta;
    }

    public int getCurrentPosition() {
        return currentPosition;
    }
}

class GlyphGutter extends JComponent {

    private ScriptEditorForBeanShell debugger;
    private final String DUMMY_STRING = "99";

    GlyphGutter(ScriptEditorForBeanShell debugger) {
        this.debugger = debugger;
        update();
    }

    public void update() {
        JTextArea textArea = debugger.getTextArea();
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
        JTextArea textArea = debugger.getTextArea();

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
            if (i == debugger.getCurrentPosition()) {
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

