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

package org.openoffice.idesupport.xml;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

import java.util.ArrayList;
import java.util.Iterator;

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

            ArrayList<Element> list = new ArrayList<Element>();
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
