package org.openoffice.idesupport.ui;

import java.io.File;
import java.util.Vector;
import java.util.ArrayList;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JList;
import javax.swing.JLabel;
import java.awt.BorderLayout;

import org.openoffice.idesupport.ScriptEntry;
import org.openoffice.idesupport.DefaultScriptClassLoader;
import org.openoffice.idesupport.zip.ParcelZipper;

public class MethodPanel extends JPanel {

    private File basedir;
    private Vector classpath;
    private final static String FIRST_PARAM = "drafts.com.sun.star.script.framework.XScriptContext";

    private JList list;
    private Vector values = new Vector(11);

    public MethodPanel(File basedir, Vector classpath) {
    this.basedir = basedir;
        this.classpath = classpath;
        initValues();
        initUI();
    }

    public ScriptEntry[] getSelectedEntries() {
        Object[] selections = list.getSelectedValues();
        ScriptEntry[] entries = new ScriptEntry[selections.length];

        for (int i = 0; i < selections.length; i++) {
            entries[i] = (ScriptEntry)selections[i];
        }

        return entries;
    }

    private void initUI() {
        JLabel label = new JLabel("Available Methods:");
        list = new JList(values);
        JScrollPane pane = new JScrollPane(list);
        label.setLabelFor(pane);

        BorderLayout layout = new BorderLayout();
        setLayout(layout);
        layout.setVgap(5);

        add(label, BorderLayout.NORTH);
        add(pane, BorderLayout.CENTER);
    }

    private void initValues() {
        String[] classNames;
        String parcelName;

        if (basedir == null || basedir.exists() == false ||
            basedir.isDirectory() == false)
            return;

        classNames = findClassNames();
        if (classNames == null || classNames.length == 0)
            return;

        parcelName = basedir.getName();
        if (parcelName.equals(ParcelZipper.CONTENTS_DIRNAME))
            parcelName = basedir.getParentFile().getName();

        DefaultScriptClassLoader classloader =
            new DefaultScriptClassLoader(classpath);

        for (int i = 0; i < classNames.length; i++)
        {
            try
            {
                Class clazz = classloader.loadClass(classNames[i]);
                Method[] methods = clazz.getDeclaredMethods();
                for (int k = 0; k < methods.length; k++)
                {
                    if (Modifier.isPublic(methods[k].getModifiers()))
                    {
                        Class[] params = methods[k].getParameterTypes();
                        if(params.length > 0)
                        {
                            if(params[0].getName().equals(FIRST_PARAM))
                            {
                                ScriptEntry entry =
                                new ScriptEntry(classNames[i] + "." + methods[k].getName(),
                                                parcelName);
                                values.addElement(entry);
                            }
                        }
                    }
                }
            }
            catch (ClassNotFoundException e)
            {
                System.err.println("Class Not Found Exception...");
                continue;
            }
            catch (NoClassDefFoundError nc)
            {
                System.err.println("No Class Definition Found...");
                continue;
            }
        }
    }

    private ArrayList findFiles(File basedir, String suffix) {
        ArrayList result = new ArrayList();
        File[] children = basedir.listFiles();

        for (int i = 0; i < children.length; i++) {
            if (children[i].isDirectory())
                result.addAll(findFiles(children[i], suffix));
            else if (children[i].getName().endsWith(suffix))
                    result.add(children[i]);
        }
        return result;
    }

    private String[] findClassNames()
    {
        ArrayList classFiles = findFiles(basedir, ".class");
        if(classFiles == null || classFiles.size() == 0)
            return null;

        ArrayList javaFiles = findFiles(basedir, ".java");
        if(javaFiles == null || javaFiles.size() == 0)
            return null;

        ArrayList result = new ArrayList();
        for (int i = 0; i < classFiles.size(); i++)
        {
            File classFile = (File)classFiles.get(i);
            String className = classFile.getName();
            className = className.substring(0, className.lastIndexOf(".class"));
            boolean finished = false;


            for (int j = 0; j < javaFiles.size() && finished == false; j++)
            {
                File javaFile = (File)javaFiles.get(j);
                String javaName = javaFile.getName();
                javaName = javaName.substring(0, javaName.lastIndexOf(".java"));


                if (javaName.equals(className))
                {
                    String path = classFile.getAbsolutePath();
                    path = path.substring(basedir.getAbsolutePath().length() + 1);
                    path = path.replace(File.separatorChar, '.');
                    path = path.substring(0, path.lastIndexOf(".class"));

                    result.add(path);
                    javaFiles.remove(j);
                    finished = true;
                }
            }
        }
        return (String[])result.toArray(new String[0]);
    }
}
