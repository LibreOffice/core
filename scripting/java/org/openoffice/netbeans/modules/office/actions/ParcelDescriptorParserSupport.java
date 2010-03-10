/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
