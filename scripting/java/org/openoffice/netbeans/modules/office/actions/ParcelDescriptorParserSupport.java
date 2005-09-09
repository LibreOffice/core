/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParcelDescriptorParserSupport.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:12:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
