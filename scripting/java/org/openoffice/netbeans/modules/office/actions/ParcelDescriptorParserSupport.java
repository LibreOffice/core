package org.openoffice.netbeans.modules.office.actions;

import java.io.*;
import java.util.*;
import javax.swing.event.ChangeListener;
import javax.swing.event.ChangeEvent;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.openide.filesystems.*;
import org.openide.xml.XMLUtil;

public class ParcelDescriptorParserSupport
    implements ParcelDescriptorParserCookie, FileChangeListener
{
    private FileObject fo;
    private Document document;
    private Set listeners;

    public ParcelDescriptorParserSupport(FileObject fo)
    {
        this.fo = fo;
        fo.addFileChangeListener(this);
    }

    private synchronized void parseFile()
    {
        File file = FileUtil.toFile(fo);
        InputSource is;

        try {
            is = new InputSource(new FileInputStream(file));
        }
        catch (FileNotFoundException fnfe) {
            System.out.println("Couldn't find file: " + file.getName());
            return;
        }

        document = null;
        try {
            document = XMLUtil.parse(is, false, false, null, null);
        }
        catch (IOException ioe) {
            System.out.println("IO Error parsing file: " + file.getName());
        }
        catch (SAXException se) {
            System.out.println("Sax Error parsing file: " + file.getName());
        }
    }

    public synchronized NodeList getScriptElements()
    {
        if (document == null)
            parseFile();

        if (document != null)
            return document.getElementsByTagName("script");
        return null;
    }

    public void addChangeListener(ChangeListener cl) {
        if (listeners == null)
            listeners = new HashSet();

        listeners.add(cl);
    }

    public void removeChangeListener(ChangeListener cl) {
        if (listeners == null)
            return;

        listeners.remove(cl);
    }

    public void fileChanged(FileEvent fe) {
        parseFile();

        if (listeners != null) {
            Iterator iter = listeners.iterator();

            while (iter.hasNext())
                ((ChangeListener)iter.next()).stateChanged(new ChangeEvent(this));
        }
    }

    public void fileAttributeChanged(FileAttributeEvent fe) {}
    public void fileDataCreated(FileEvent fe) {}
    public void fileDeleted(FileEvent fe) {}
    public void fileFolderCreated(FileEvent fe) {}
    public void fileRenamed(FileRenameEvent fe) {}
}
