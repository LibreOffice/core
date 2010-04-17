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

package org.openoffice.idesupport.xml;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Iterator;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import com.sun.star.script.framework.container.XMLParserFactory;

public class Manifest {

    private Document document = null;
    private boolean baseElementsExist = false;

    public Manifest(InputStream inputStream) throws IOException {
        document = XMLParserFactory.getParser().parse(inputStream);
    }

    public void add(String entry) {
        add(entry, "");
    }

    private void add(String entry, String type) {
        Element root, el;

        ensureBaseElementsExist();

        try {
            root = (Element)
                document.getElementsByTagName("manifest:manifest").item(0);

            el = document.createElement("manifest:file-entry");
            el.setAttribute("manifest:media-type", type);
            el.setAttribute("manifest:full-path", entry);
            // System.out.println("added: " + el.toString());
            root.appendChild(el);
        }
        catch (Exception e) {
            System.err.println("Error adding entry: " + e.getMessage());
        }
    }

    private void ensureBaseElementsExist() {
        if (baseElementsExist == false) {
            baseElementsExist = true;
            add("Scripts/", "application/script-parcel");
        }
    }

    public void remove(String entry) {
        Element root, el;
        int len;

        try {
            root = (Element)
                document.getElementsByTagName("manifest:manifest").item(0);

            NodeList nl = root.getElementsByTagName("manifest:file-entry");
            if (nl == null || (len = nl.getLength()) == 0)
                return;

            ArrayList list = new ArrayList();
            for (int i = 0; i < len; i++) {
                el = (Element)nl.item(i);
                if (el.getAttribute("manifest:full-path").startsWith(entry)) {
                    // System.out.println("found: " + el.toString());
                    list.add(el);
                }
            }

            Iterator iter = list.iterator();
            while (iter.hasNext())
                root.removeChild((Element)iter.next());

            // System.out.println("and after root is: " + root.toString());
        }
        catch (Exception e) {
            System.err.println("Error removing entry: " + e.getMessage());
        }
    }

    public InputStream getInputStream() throws IOException {
        InputStream result = null;
        ByteArrayOutputStream out = null;

        try {
            out = new ByteArrayOutputStream();
            write(out);
            result = new ByteArrayInputStream(out.toByteArray());
            // result = replaceNewlines(out.toByteArray());
        }
        finally {
            if (out != null)
                out.close();
        }

        return result;
    }

    private InputStream replaceNewlines(byte[] bytes) throws IOException {
        InputStream result;
        ByteArrayOutputStream out;
        BufferedReader reader;

        reader = new BufferedReader(new InputStreamReader(
            new ByteArrayInputStream(bytes)));
        out = new ByteArrayOutputStream();

        int previous = reader.read();
        out.write(previous);
        int current;

        while ((current = reader.read()) != -1) {
            if (((char)current == '\n' || (char)current == ' ') &&
                (char)previous == '\n')
                continue;
            else {
                out.write(current);
                previous = current;
            }
        }
        result = new ByteArrayInputStream(out.toByteArray());

        return result;
    }

    public void write(OutputStream out) throws IOException {
        XMLParserFactory.getParser().write(document, out);
    }
}
