import java.util.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;

import drafts.com.sun.star.script.framework.runtime.XScriptContext;
import bsh.*;

public class OOBeanShellDebugger implements OOScriptDebugger, ActionListener, DocumentListener {

    private JFrame frame;
    private JTextArea ta;
    private GlyphGutter gg;
    private XScriptContext context;
    private int currentPosition = -1;
    private ArrayList breakpoints;
    private int linecount;
    private BufferedReader bufReader;
    private Interpreter sessionInterpreter;
    private Thread execThread = null;
    private String filename = null;

    public void go(XScriptContext context, String filename) {
        this.context = context;
        this.breakpoints = new ArrayList();
        initUI();

        if (filename != null && filename != "") {
            try {
                loadFile(filename);
                this.filename = filename;
            }
            catch (IOException ioe) {
                JOptionPane.showMessageDialog(frame,
                    "Error loading file: " + ioe.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    public void go(XScriptContext context, InputStream in) {
        this.context = context;
        this.breakpoints = new ArrayList();
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

    public void loadFile(String filename) throws IOException {
        FileInputStream fis = new FileInputStream(filename);
        loadFile(fis);
    }

    public void loadFile(InputStream in) throws IOException {
        ta.getDocument().removeDocumentListener(this);

        byte[] contents = new byte[1024];
        int len = 0, pos = 0;

        while ((len = in.read(contents, 0, 1024)) != -1) {
            ta.insert(new String(contents, 0, len), pos);
            pos += len;
        }

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
        String[] labels = {"Run", /* "Stop", */ "Clear", "Save", "Close"};
        JPanel p = new JPanel();
        p.setLayout(new FlowLayout());

        for (int i = 0; i < labels.length; i++) {
            JButton b = new JButton(labels[i]);
            b.addActionListener(this);
            p.add(b);
        }

        frame.getContentPane().add(sp, "Center");
        frame.getContentPane().add(p, "South");
        frame.pack();
        frame.show();
    }

    public void insertUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void removeUpdate(DocumentEvent e) {
        doChanged(e);
    }

    public void changedUpdate(DocumentEvent e) {
        doChanged(e);
    }

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

                currentPosition = -1;
                gg.repaint();

                try {
                    interpreter.set("context", context);
                    interpreter.eval(ta.getText());
                }
                catch (bsh.EvalError err) {
                    currentPosition = err.getErrorLineNumber() - 1;
                    try {
                        int line = ta.getLineStartOffset(currentPosition);
                        Rectangle rect = ta.modelToView(line);
                        ta.scrollRectToVisible(rect);
                    }
                    catch (Exception e) {
                        System.err.println("error: " + e.getMessage());
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

    private void stopExecution() {
        if (execThread != null) {
            execThread.interrupt();
            execThread = null;
        }

        ta.setEditable(true);
        if (bufReader != null) {
            try {
                bufReader.close();
            }
            catch (IOException ioe) {}
        }
        bufReader = null;
        currentPosition = -1;
        gg.repaint();
    }

    private void startSession() {
        /* Reader reader = new StringReader(ta.getText());
        bsh.Parser parser = new bsh.Parser(reader);
        boolean eof = false;

        try {
            Interpreter interpreter = new Interpreter();
            interpreter.getNameSpace().clear();
            interpreter.set("context", context);

            CallStack callstack = new CallStack();
            callstack.push(interpreter.getNameSpace());

            bsh.SimpleNode node = null;
            while(!eof) {
                    eof = parser.Line();
                    node = parser.popNode();
                    System.out.println("Executing: " + node.getText());
                    node.eval(callstack, interpreter);
            }
        }
        catch(Exception e) {
            e.printStackTrace();
        } */

        /* try {
            sessionInterpreter = new Interpreter();
            sessionInterpreter.getNameSpace().clear();
            sessionInterpreter.set("context", context);
        }
        catch (bsh.EvalError err) {
            JOptionPane.showMessageDialog(frame,
                "Error at line " + String.valueOf(err.getErrorLineNumber()) +
                "\n\n: " + err.getErrorText(),
                "Error", JOptionPane.ERROR_MESSAGE);
            stopExecution();
        }

        ta.setEditable(false);
        Reader reader = new StringReader(ta.getText());
        bufReader = new BufferedReader(reader);
        currentPosition = 0;
        gg.repaint(); */
    }

    private void doStep() {
        String line = null;

        try {
            line = bufReader.readLine();
        }
        catch (IOException ioe) {
            JOptionPane.showMessageDialog(frame,
                "Error reading line " + currentPosition +
                "\n\n: " + ioe.getMessage(),
                "Error", JOptionPane.ERROR_MESSAGE);
            stopExecution();
        }

        if (line != null) {
            try {
                sessionInterpreter.eval(line);
                currentPosition++;
                gg.repaint();
            }
            catch (bsh.EvalError err) {
                gg.repaint();

                JOptionPane.showMessageDialog(frame,
                    "Error at line " + String.valueOf(err.getErrorLineNumber()) +
                    "\n\n: " + err.getErrorText(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
        else
            stopExecution();
    }

    private void promptForSaveName() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new javax.swing.filechooser.FileFilter() {
            public boolean accept(File f) {
                if (f.isDirectory() || f.getName().endsWith(".bsh"))
                    return true;
                return false;
            }

            public String getDescription() {
                return ("BeanShell files: *.bsh");
            }
        });

        int ret = chooser.showSaveDialog(frame);

        if (ret == JFileChooser.APPROVE_OPTION) {
            filename = chooser.getSelectedFile().getAbsolutePath();
            if (!filename.endsWith(".bsh"))
                filename += ".bsh";
        }

    }

    private void saveTextArea() {
        if (filename == null)
            promptForSaveName();

        if (filename != null) {
            try {
                File f = new File(filename);
                FileOutputStream fos = new FileOutputStream(f);
                String s = ta.getText();
                fos.write(s.getBytes(), 0, s.length());
            }
            catch (IOException ioe) {
                JOptionPane.showMessageDialog(frame,
                    "Error saving file: " + ioe.getMessage(),
                    "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getActionCommand().equals("Run"))
            startExecution();
        else if (e.getActionCommand().equals("Stop"))
            stopExecution();
        else if (e.getActionCommand().equals("Close"))
            frame.dispose();
        else if (e.getActionCommand().equals("Save"))
            saveTextArea();
        else if (e.getActionCommand().equals("Clear"))
            ta.setText("");
    }

    public JTextArea getTextArea() {
        return ta;
    }

    public int getCurrentPosition() {
        return currentPosition;
    }

    public void toggleBreakPoint(int line) {
        Integer lineObj = new Integer(line);
        int idx = breakpoints.indexOf(lineObj);
        if (idx != -1)
            breakpoints.remove(lineObj);
        else
            breakpoints.add(lineObj);
    }

    public boolean isBreakPoint(int line) {
        if (breakpoints.contains(new Integer(line)))
            return true;
        return false;
    }
}

class GlyphGutter extends JComponent implements MouseListener {

    private OOBeanShellDebugger debugger;
    private boolean isError = false;

    public void mouseEntered(MouseEvent e) {
    }
    public void mousePressed(MouseEvent e) {
    }
    public void mouseClicked(MouseEvent e) {
        if (e.getComponent() == this &&
          (e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0) {
            int x = e.getX();
            int y = e.getY();
            Font font = debugger.getTextArea().getFont();
            FontMetrics metrics = getFontMetrics(font);
            int h = metrics.getHeight();
            int line = y/h;
            debugger.toggleBreakPoint(line + 1);
        }
    }
    public void mouseExited(MouseEvent e) {
    }
    public void mouseReleased(MouseEvent e) {
    }

    GlyphGutter(OOBeanShellDebugger debugger) {
        this.debugger = debugger;
        // addMouseListener(this);
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
            dummy = "99";
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
        if (endLine > lineCount) endLine = lineCount;

        for (int i = startLine; i < endLine; i++) {
            String text;
            text = Integer.toString(i + 1) + " ";
            int w = metrics.stringWidth(text);
            int y = i * h;
            g.setColor(Color.blue);
            g.drawString(text, 0, y + ascent);
            int x = width - ascent;

            if (debugger.isBreakPoint(i + 1))
                drawBreakPoint(g, ascent, x, y);

            if (i == debugger.getCurrentPosition())
                drawArrow(g, ascent, x, y);
        }
    }

    private void drawBreakPoint(Graphics g, int ascent, int x, int y) {
        g.setColor(new Color(0x80, 0x00, 0x00));
        int dy = y + ascent - 9;
        g.fillOval(x, dy, 9, 9);
        g.drawOval(x, dy, 8, 8);
        g.drawOval(x, dy, 9, 9);
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

