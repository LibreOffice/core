/*************************************************************************
 *
 *  $RCSfile: Manifest.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:45:54 $
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

import com.sun.star.script.framework.browse.XMLParserFactory;

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
